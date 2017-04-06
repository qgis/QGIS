/***************************************************************************
                              qgswfstransaction.cpp
                              -------------------------
  begin                : December 20 , 2016
  copyright            : (C) 2007 by Marco Hugentobler  (original code)
                         (C) 2014 by Alessandro Pasotti (original code)
                         (C) 2017 by David Marteau
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
                         a dot pasotti at itopen dot it
                         david dot marteau at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgswfsutils.h"
#include "qgsserverprojectutils.h"
#include "qgsfields.h"
#include "qgsexpression.h"
#include "qgsgeometry.h"
#include "qgsmaplayer.h"
#include "qgsfeatureiterator.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsfilterrestorer.h"
#include "qgsogcutils.h"
#include "qgswfstransaction.h"

namespace QgsWfs
{
  namespace
  {
    void addTransactionResult( QDomDocument &responseDoc, QDomElement &responseElem, const QString &status,
                               const QString &locator, const QString &message );
  }


  void writeTransaction( QgsServerInterface *serverIface, const QgsProject *project,
                         const QString &version, const QgsServerRequest &request,
                         QgsServerResponse &response )

  {
    QDomDocument doc = createTransactionDocument( serverIface, project, version, request );

    response.setHeader( "Content-Type", "text/xml; charset=utf-8" );
    response.write( doc.toByteArray() );
  }

  QDomDocument createTransactionDocument( QgsServerInterface *serverIface, const QgsProject *project,
                                          const QString &version, const QgsServerRequest &request )
  {
    Q_UNUSED( version );

    QgsServerRequest::Parameters parameters = request.parameters();
    transactionRequest aRequest;

    QDomDocument doc;
    QString errorMsg;

    if ( doc.setContent( parameters.value( QStringLiteral( "REQUEST_BODY" ) ), true, &errorMsg ) )
    {
      QDomElement docElem = doc.documentElement();
      aRequest = parseTransactionRequestBody( docElem );
    }

    int actionCount = aRequest.inserts.size() + aRequest.updates.size() + aRequest.deletes.size();
    if ( actionCount == 0 )
    {
      throw QgsRequestNotWellFormedException( QStringLiteral( "No actions found" ) );
    }

    performTransaction( aRequest, serverIface, project );

    // It's time to make the transaction
    // Create the response document
    QDomDocument resp;
    //wfs:WFS_TransactionRespone element
    QDomElement respElem = resp.createElement( QStringLiteral( "WFS_TransactionResponse" )/*wfs:WFS_TransactionResponse*/ );
    respElem.setAttribute( QStringLiteral( "xmlns" ), WFS_NAMESPACE );
    respElem.setAttribute( QStringLiteral( "xmlns:xsi" ), QStringLiteral( "http://www.w3.org/2001/XMLSchema-instance" ) );
    respElem.setAttribute( QStringLiteral( "xsi:schemaLocation" ), WFS_NAMESPACE + " http://schemas.opengis.net/wfs/1.0.0/wfs.xsd" );
    respElem.setAttribute( QStringLiteral( "xmlns:ogc" ), OGC_NAMESPACE );
    respElem.setAttribute( QStringLiteral( "version" ), QStringLiteral( "1.0.0" ) );
    resp.appendChild( respElem );

    int errorCount = 0;
    QStringList errorLocators;
    QStringList errorMessages;

    QList<transactionUpdate>::iterator tuIt = aRequest.updates.begin();
    for ( ; tuIt != aRequest.updates.end(); ++tuIt )
    {
      transactionUpdate &action = *tuIt;
      if ( action.error )
      {
        errorCount += 1;
        if ( action.handle.isEmpty() )
        {
          errorLocators << QStringLiteral( "Update:%1" ).arg( action.typeName );
        }
        else
        {
          errorLocators << action.handle;
        }
        errorMessages << action.errorMsg;
      }
    }

    QList<transactionDelete>::iterator tdIt = aRequest.deletes.begin();
    for ( ; tdIt != aRequest.deletes.end(); ++tdIt )
    {
      transactionDelete &action = *tdIt;
      if ( action.error )
      {
        errorCount += 1;
        if ( action.handle.isEmpty() )
        {
          errorLocators << QStringLiteral( "Update:%1" ).arg( action.typeName );
        }
        else
        {
          errorLocators << action.handle;
        }
        errorMessages << action.errorMsg;
      }
    }

    QList<transactionInsert>::iterator tiIt = aRequest.inserts.begin();
    for ( ; tiIt != aRequest.inserts.end(); ++tiIt )
    {
      transactionInsert &action = *tiIt;
      if ( action.error )
      {
        errorCount += 1;
        if ( action.handle.isEmpty() )
        {
          errorLocators << QStringLiteral( "Update:%1" ).arg( action.typeName );
        }
        else
        {
          errorLocators << action.handle;
        }
        errorMessages << action.errorMsg;
      }
      else
      {
        QStringList::const_iterator fidIt = action.insertFeatureIds.constBegin();
        for ( ; fidIt != action.insertFeatureIds.constEnd(); ++fidIt )
        {
          QString fidStr = *fidIt;
          QDomElement irElem = doc.createElement( QStringLiteral( "InsertResult" ) );
          if ( !action.handle.isEmpty() )
          {
            irElem.setAttribute( QStringLiteral( "handle" ), action.handle );
          }
          QDomElement fiElem = doc.createElement( QStringLiteral( "ogc:FeatureId" ) );
          fiElem.setAttribute( QStringLiteral( "fid" ), fidStr );
          irElem.appendChild( fiElem );
          respElem.appendChild( irElem );
        }
      }
    }

    // addTransactionResult
    if ( errorCount == 0 )
    {
      addTransactionResult( resp, respElem, QStringLiteral( "SUCCESS" ), QString(), QString() );
    }
    else
    {
      QString locator = errorLocators.join( QStringLiteral( "; " ) );
      QString message = errorMessages.join( QStringLiteral( "; " ) );
      if ( errorCount != actionCount )
      {
        addTransactionResult( resp, respElem, QStringLiteral( "PARTIAL" ), locator, message );
      }
      else
      {
        addTransactionResult( resp, respElem, QStringLiteral( "ERROR" ), locator, message );
      }
    }
    return resp;
  }

  void performTransaction( transactionRequest &aRequest, QgsServerInterface *serverIface, const QgsProject *project )
  {
    // store typeName
    QStringList typeNameList;

    QList<transactionInsert>::iterator tiIt = aRequest.inserts.begin();
    for ( ; tiIt != aRequest.inserts.end(); ++tiIt )
    {
      QString name = ( *tiIt ).typeName;
      if ( !typeNameList.contains( name ) )
        typeNameList << name;
    }
    QList<transactionUpdate>::iterator tuIt = aRequest.updates.begin();
    for ( ; tuIt != aRequest.updates.end(); ++tuIt )
    {
      QString name = ( *tuIt ).typeName;
      if ( !typeNameList.contains( name ) )
        typeNameList << name;
    }
    QList<transactionDelete>::iterator tdIt = aRequest.deletes.begin();
    for ( ; tdIt != aRequest.deletes.end(); ++tdIt )
    {
      QString name = ( *tdIt ).typeName;
      if ( !typeNameList.contains( name ) )
        typeNameList << name;
    }

    // get access controls
    QgsAccessControl *accessControl = serverIface->accessControls();

    //scoped pointer to restore all original layer filters (subsetStrings) when pointer goes out of scope
    //there's LOTS of potential exit paths here, so we avoid having to restore the filters manually
    std::unique_ptr< QgsOWSServerFilterRestorer > filterRestorer( new QgsOWSServerFilterRestorer( accessControl ) );

    // get layers
    QStringList wfsLayerIds = QgsServerProjectUtils::wfsLayerIds( *project );
    QStringList wfstUpdateLayerIds = QgsServerProjectUtils::wfstUpdateLayerIds( *project );
    QStringList wfstDeleteLayerIds = QgsServerProjectUtils::wfstDeleteLayerIds( *project );
    QStringList wfstInsertLayerIds = QgsServerProjectUtils::wfstInsertLayerIds( *project );
    QMap<QString, QgsVectorLayer *> mapLayerMap;
    for ( int i = 0; i < wfsLayerIds.size(); ++i )
    {
      QgsMapLayer *layer = project->mapLayer( wfsLayerIds.at( i ) );
      if ( layer->type() != QgsMapLayer::LayerType::VectorLayer )
      {
        continue;
      }

      QString name = layer->name();
      if ( !layer->shortName().isEmpty() )
        name = layer->shortName();
      name = name.replace( QLatin1String( " " ), QLatin1String( "_" ) );

      if ( !typeNameList.contains( name ) )
      {
        continue;
      }

      // get vector layer
      QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
      if ( !vlayer )
      {
        throw QgsRequestNotWellFormedException( QStringLiteral( "Layer error on '%1'" ).arg( name ) );
      }

      //get provider
      QgsVectorDataProvider *provider = vlayer->dataProvider();
      if ( !provider )
      {
        throw QgsRequestNotWellFormedException( QStringLiteral( "Provider error on layer '%1'" ).arg( name ) );
      }

      // get provider capabilities
      int cap = provider->capabilities();
      if ( !( cap & QgsVectorDataProvider::ChangeAttributeValues ) && !( cap & QgsVectorDataProvider::ChangeGeometries )
           && !( cap & QgsVectorDataProvider::DeleteFeatures ) && !( cap & QgsVectorDataProvider::AddFeatures ) )
      {
        throw QgsRequestNotWellFormedException( QStringLiteral( "No capabilities to do WFS changes on layer '%1'" ).arg( name ) );
      }

      if ( !wfstUpdateLayerIds.contains( vlayer->id() )
           && !wfstDeleteLayerIds.contains( vlayer->id() )
           && !wfstInsertLayerIds.contains( vlayer->id() ) )
      {
        throw QgsSecurityAccessException( QStringLiteral( "No permissions to do WFS changes on layer '%1'" ).arg( name ) );
      }
      if ( accessControl && !accessControl->layerUpdatePermission( vlayer )
           && !accessControl->layerDeletePermission( vlayer ) && !accessControl->layerInsertPermission( vlayer ) )
      {
        throw QgsSecurityAccessException( QStringLiteral( "No permissions to do WFS changes on layer '%1'" ).arg( name ) );
      }

      if ( accessControl )
      {
        QgsOWSServerFilterRestorer::applyAccessControlLayerFilters( accessControl, vlayer, filterRestorer->originalFilters() );
      }

      // store layers
      mapLayerMap[name] = vlayer;
    }

    // perform updates
    tuIt = aRequest.updates.begin();
    for ( ; tuIt != aRequest.updates.end(); ++tuIt )
    {
      transactionUpdate &action = *tuIt;
      QString typeName = action.typeName;

      if ( !mapLayerMap.keys().contains( typeName ) )
      {
        action.error = true;
        action.errorMsg = QStringLiteral( "TypeName '%1' unknown" ).arg( typeName );
        continue;
      }

      // get vector layer
      QgsVectorLayer *vlayer = mapLayerMap[typeName];

      // verifying specific permissions
      if ( !wfstUpdateLayerIds.contains( vlayer->id() ) )
      {
        action.error = true;
        action.errorMsg = QStringLiteral( "No permissions to do WFS updates on layer '%1'" ).arg( typeName );
        continue;
      }
      if ( accessControl && !accessControl->layerUpdatePermission( vlayer ) )
      {
        action.error = true;
        action.errorMsg = QStringLiteral( "No permissions to do WFS updates on layer '%1'" ).arg( typeName );
        continue;
      }

      //get provider
      QgsVectorDataProvider *provider = vlayer->dataProvider();

      // verifying specific capabilities
      int cap = provider->capabilities();
      if ( !( cap & QgsVectorDataProvider::ChangeAttributeValues ) || !( cap & QgsVectorDataProvider::ChangeGeometries ) )
      {
        action.error = true;
        action.errorMsg = QStringLiteral( "No capabilities to do WFS updates on layer '%1'" ).arg( typeName );
        continue;
      }
      // start editing
      vlayer->startEditing();

      // update request
      QgsFeatureRequest featureRequest = action.featureRequest;

      // expression context
      QgsExpressionContext expressionContext;
      expressionContext << QgsExpressionContextUtils::globalScope()
                        << QgsExpressionContextUtils::projectScope( project )
                        << QgsExpressionContextUtils::layerScope( vlayer );
      featureRequest.setExpressionContext( expressionContext );

      if ( accessControl )
      {
        accessControl->filterFeatures( vlayer, featureRequest );
      }

      // get iterator
      QgsFeatureIterator fit = vlayer->getFeatures( featureRequest );
      QgsFeature feature;
      // get action properties
      QMap<QString, QString> propertyMap = action.propertyMap;
      QDomElement geometryElem = action.geometryElement;
      // get field information
      QgsFields fields = provider->fields();
      QMap<QString, int> fieldMap = provider->fieldNameMap();
      QMap<QString, int>::const_iterator fieldMapIt;
      QString fieldName;
      bool conversionSuccess;
      // Update the features
      while ( fit.nextFeature( feature ) )
      {
        if ( accessControl && !accessControl->allowToEdit( vlayer, feature ) )
        {
          action.error = true;
          action.errorMsg = QStringLiteral( "Feature modify permission denied on layer '%1'" ).arg( typeName );
          vlayer->rollBack();
          break;
        }
        QMap< QString, QString >::const_iterator it = propertyMap.constBegin();
        for ( ; it != propertyMap.constEnd(); ++it )
        {
          fieldName = it.key();
          fieldMapIt = fieldMap.find( fieldName );
          if ( fieldMapIt == fieldMap.constEnd() )
          {
            continue;
          }
          QgsField field = fields.at( fieldMapIt.value() );
          QVariant value = it.value();
          if ( field.type() == 2 )
          {
            value = it.value().toInt( &conversionSuccess );
            if ( !conversionSuccess )
            {
              action.error = true;
              action.errorMsg = QStringLiteral( "Property conversion error on layer '%1'" ).arg( typeName );
              vlayer->rollBack();
              break;
            }
          }
          else if ( field.type() == 6 )
          {
            value = it.value().toDouble( &conversionSuccess );
            if ( !conversionSuccess )
            {
              action.error = true;
              action.errorMsg = QStringLiteral( "Property conversion error on layer '%1'" ).arg( typeName );
              vlayer->rollBack();
              break;
            }
          }
          vlayer->changeAttributeValue( feature.id(), fieldMapIt.value(), value );
        }
        if ( action.error )
        {
          break;
        }

        if ( !geometryElem.isNull() )
        {
          QgsGeometry g = QgsOgcUtils::geometryFromGML( geometryElem );
          if ( g.isNull() )
          {
            action.error = true;
            action.errorMsg = QStringLiteral( "Geometry from GML error on layer '%1'" ).arg( typeName );
            vlayer->rollBack();
            break;
          }
          if ( !vlayer->changeGeometry( feature.id(), g ) )
          {
            action.error = true;
            action.errorMsg = QStringLiteral( "Error in change geometry on layer '%1'" ).arg( typeName );
            vlayer->rollBack();
            break;
          }
        }
      }
      if ( action.error )
      {
        continue;
      }
      // verifying changes
      if ( accessControl )
      {
        fit = vlayer->getFeatures( featureRequest );
        while ( fit.nextFeature( feature ) )
        {
          if ( accessControl && !accessControl->allowToEdit( vlayer, feature ) )
          {
            action.error = true;
            action.errorMsg = QStringLiteral( "Feature modify permission denied on layer '%1'" ).arg( typeName );
            vlayer->rollBack();
            break;
          }
        }
      }
      if ( action.error )
      {
        continue;
      }

      // Commit the changes of the update elements
      if ( !vlayer->commitChanges() )
      {
        action.error = true;
        action.errorMsg = QStringLiteral( "Error commiting updates: %1" ).arg( vlayer->commitErrors().join( QStringLiteral( "; " ) ) );
        vlayer->rollBack();
        continue;
      }
      // all the changes are OK!
      action.error = false;

    }

    // perform deletes
    tdIt = aRequest.deletes.begin();
    for ( ; tdIt != aRequest.deletes.end(); ++tdIt )
    {
      transactionDelete &action = *tdIt;
      QString typeName = action.typeName;

      if ( !mapLayerMap.keys().contains( typeName ) )
      {
        action.error = true;
        action.errorMsg = QStringLiteral( "TypeName '%1' unknown" ).arg( typeName );
        continue;
      }

      // get vector layer
      QgsVectorLayer *vlayer = mapLayerMap[typeName];

      // verifying specific permissions
      if ( !wfstDeleteLayerIds.contains( vlayer->id() ) )
      {
        action.error = true;
        action.errorMsg = QStringLiteral( "No permissions to do WFS deletes on layer '%1'" ).arg( typeName );
        continue;
      }
      if ( accessControl && !accessControl->layerDeletePermission( vlayer ) )
      {
        action.error = true;
        action.errorMsg = QStringLiteral( "No permissions to do WFS deletes on layer '%1'" ).arg( typeName );
        continue;
      }

      //get provider
      QgsVectorDataProvider *provider = vlayer->dataProvider();

      // verifying specific capabilities
      int cap = provider->capabilities();
      if ( !( cap & QgsVectorDataProvider::DeleteFeatures ) )
      {
        action.error = true;
        action.errorMsg = QStringLiteral( "No capabilities to do WFS deletes on layer '%1'" ).arg( typeName );
        continue;
      }
      // start editing
      vlayer->startEditing();

      // update request
      QgsFeatureRequest featureRequest = action.featureRequest;

      // expression context
      QgsExpressionContext expressionContext;
      expressionContext << QgsExpressionContextUtils::globalScope()
                        << QgsExpressionContextUtils::projectScope( project )
                        << QgsExpressionContextUtils::layerScope( vlayer );
      featureRequest.setExpressionContext( expressionContext );

      // get iterator
      QgsFeatureIterator fit = vlayer->getFeatures( featureRequest );
      QgsFeature feature;
      // get deleted fids
      QgsFeatureIds fids;
      while ( fit.nextFeature( feature ) )
      {
        if ( accessControl && !accessControl->allowToEdit( vlayer, feature ) )
        {
          action.error = true;
          action.errorMsg = QStringLiteral( "Feature modify permission denied" );
          vlayer->rollBack();
          break;
        }
        fids << feature.id();
      }
      if ( action.error )
      {
        continue;
      }
      // delete features
      if ( !vlayer->deleteFeatures( fids ) )
      {
        action.error = true;
        action.errorMsg = QStringLiteral( "Delete features failed on layer '%1'" ).arg( typeName );
        vlayer->rollBack();
        continue;
      }

      // Commit the changes of the update elements
      if ( !vlayer->commitChanges() )
      {
        action.error = true;
        action.errorMsg = QStringLiteral( "Error commiting deletes: %1" ).arg( vlayer->commitErrors().join( QStringLiteral( "; " ) ) );
        vlayer->rollBack();
        continue;
      }
      // all the changes are OK!
      action.error = false;
    }

    // perform inserts
    tiIt = aRequest.inserts.begin();
    for ( ; tiIt != aRequest.inserts.end(); ++tiIt )
    {
      transactionInsert &action = *tiIt;
      QString typeName = action.typeName;

      if ( !mapLayerMap.keys().contains( typeName ) )
      {
        action.error = true;
        action.errorMsg = QStringLiteral( "TypeName '%1' unknown" ).arg( typeName );
        continue;
      }

      // get vector layer
      QgsVectorLayer *vlayer = mapLayerMap[typeName];

      // verifying specific permissions
      if ( !wfstInsertLayerIds.contains( vlayer->id() ) )
      {
        action.error = true;
        action.errorMsg = QStringLiteral( "No permissions to do WFS inserts on layer '%1'" ).arg( typeName );
        continue;
      }
      if ( accessControl && !accessControl->layerDeletePermission( vlayer ) )
      {
        action.error = true;
        action.errorMsg = QStringLiteral( "No permissions to do WFS inserts on layer '%1'" ).arg( typeName );
        continue;
      }

      //get provider
      QgsVectorDataProvider *provider = vlayer->dataProvider();

      // verifying specific capabilities
      int cap = provider->capabilities();
      if ( !( cap & QgsVectorDataProvider::AddFeatures ) )
      {
        action.error = true;
        action.errorMsg = QStringLiteral( "No capabilities to do WFS inserts on layer '%1'" ).arg( typeName );
        continue;
      }

      // start editing
      vlayer->startEditing();

      // get inserting features
      QgsFeatureList featureList;
      try
      {
        featureList = featuresFromGML( action.featureNodeList, provider );
      }
      catch ( QgsOgcServiceException &ex )
      {
        action.error = true;
        action.errorMsg = QStringLiteral( "%1 '%2'" ).arg( ex.message() ).arg( typeName );
        continue;
      }
      // control features
      if ( accessControl )
      {
        QgsFeatureList::iterator featureIt = featureList.begin();
        while ( featureIt != featureList.end() )
        {
          if ( !accessControl->allowToEdit( vlayer, *featureIt ) )
          {
            action.error = true;
            action.errorMsg = QStringLiteral( "Feature modify permission denied on layer '%1'" ).arg( typeName );
            vlayer->rollBack();
            break;
          }
          featureIt++;
        }
      }
      if ( action.error )
      {
        continue;
      }

      // perform add features
      if ( !vlayer->addFeatures( featureList ) )
      {
        action.error = true;
        action.errorMsg = QStringLiteral( "Insert features failed on layer '%1'" ).arg( typeName );
        vlayer->rollBack();
        continue;
      }

      // Commit the changes of the update elements
      if ( !vlayer->commitChanges() )
      {
        action.error = true;
        action.errorMsg = QStringLiteral( "Error commiting inserts: %1" ).arg( vlayer->commitErrors().join( QStringLiteral( "; " ) ) );
        vlayer->rollBack();
        continue;
      }
      // all changes are OK!
      action.error = false;

      // Get the Feature Ids of the inserted feature
      for ( int j = 0; j < featureList.size(); j++ )
      {
        action.insertFeatureIds << typeName + "." + QString::number( featureList[j].id() );
      }
    }

    //force restoration of original layer filters
    filterRestorer.reset();
  }

  QgsFeatureList featuresFromGML( QDomNodeList featureNodeList, QgsVectorDataProvider *provider )
  {
    // Store the inserted features
    QgsFeatureList featList;

    // Get Layer Field Information
    QgsFields fields = provider->fields();
    QMap<QString, int> fieldMap = provider->fieldNameMap();
    QMap<QString, int>::const_iterator fieldMapIt;

    for ( int i = 0; i < featureNodeList.count(); i++ )
    {
      QgsFeature feat( fields );

      QDomElement featureElem = featureNodeList.at( i ).toElement();
      QDomNode currentAttributeChild = featureElem.firstChild();
      bool conversionSuccess = true;

      while ( !currentAttributeChild.isNull() )
      {
        QDomElement currentAttributeElement = currentAttributeChild.toElement();
        QString attrName = currentAttributeElement.localName();

        if ( attrName != QLatin1String( "boundedBy" ) )
        {
          if ( attrName != QLatin1String( "geometry" ) ) //a normal attribute
          {
            fieldMapIt = fieldMap.find( attrName );
            if ( fieldMapIt == fieldMap.constEnd() )
            {
              continue;
            }

            QgsField field = fields.at( fieldMapIt.value() );
            QString attrValue = currentAttributeElement.text();
            int attrType = field.type();

            QgsMessageLog::logMessage( QStringLiteral( "attr: name=%1 idx=%2 value=%3" ).arg( attrName ).arg( fieldMapIt.value() ).arg( attrValue ) );

            if ( attrType == QVariant::Int )
              feat.setAttribute( fieldMapIt.value(), attrValue.toInt( &conversionSuccess ) );
            else if ( attrType == QVariant::Double )
              feat.setAttribute( fieldMapIt.value(), attrValue.toDouble( &conversionSuccess ) );
            else
              feat.setAttribute( fieldMapIt.value(), attrValue );

            if ( !conversionSuccess )
            {
              throw QgsRequestNotWellFormedException( QStringLiteral( "Property conversion error on layer insert" ) );
            }
          }
          else //a geometry attribute
          {
            QgsGeometry g = QgsOgcUtils::geometryFromGML( currentAttributeElement );
            if ( g.isNull() )
            {
              throw QgsRequestNotWellFormedException( QStringLiteral( "Geometry from GML error on layer insert" ) );
            }
            feat.setGeometry( g );
          }
        }
        currentAttributeChild = currentAttributeChild.nextSibling();
      }
      // update fetaure list
      featList << feat;
    }
    return featList;
  }

  transactionRequest parseTransactionRequestBody( QDomElement &docElem )
  {
    transactionRequest request;

    QDomNodeList docChildNodes = docElem.childNodes();

    QDomElement actionElem;
    QString actionName;

    for ( int i = docChildNodes.count(); 0 < i; --i )
    {
      actionElem = docChildNodes.at( i - 1 ).toElement();
      actionName = actionElem.localName();

      if ( actionName == QLatin1String( "Insert" ) )
      {
        transactionInsert action = parseInsertActionElement( actionElem );
        request.inserts.append( action );
      }
      else if ( actionName == QLatin1String( "Update" ) )
      {
        transactionUpdate action = parseUpdateActionElement( actionElem );
        request.updates.append( action );
      }
      else if ( actionName == QLatin1String( "Delete" ) )
      {
        transactionDelete action = parseDeleteActionElement( actionElem );
        request.deletes.append( action );
      }
    }

    return request;
  }

  transactionDelete parseDeleteActionElement( QDomElement &actionElem )
  {
    QString typeName = actionElem.attribute( QStringLiteral( "typeName" ) );
    if ( typeName.contains( QLatin1String( ":" ) ) )
      typeName = typeName.section( QStringLiteral( ":" ), 1, 1 );

    QDomElement filterElem = actionElem.firstChild().toElement();
    if ( filterElem.tagName() != QLatin1String( "Filter" ) )
    {
      throw QgsRequestNotWellFormedException( QStringLiteral( "Delete action element first child is not Filter" ) );
    }

    QgsFeatureRequest featureRequest = parseFilterElement( typeName, filterElem );

    transactionDelete action;
    action.typeName = typeName;
    action.featureRequest = featureRequest;
    action.error = false;

    if ( actionElem.hasAttribute( QStringLiteral( "handle" ) ) )
    {
      action.handle = actionElem.attribute( QStringLiteral( "handle" ) );
    }

    return action;
  }

  transactionUpdate parseUpdateActionElement( QDomElement &actionElem )
  {
    QString typeName = actionElem.attribute( QStringLiteral( "typeName" ) );
    if ( typeName.contains( QLatin1String( ":" ) ) )
      typeName = typeName.section( QStringLiteral( ":" ), 1, 1 );

    QDomNodeList propertyNodeList = actionElem.elementsByTagName( QStringLiteral( "Property" ) );
    if ( propertyNodeList.size() != 1 )
    {
      throw QgsRequestNotWellFormedException( QStringLiteral( "Update action element must have one or more Property element" ) );
    }

    QMap<QString, QString> propertyMap;
    QDomElement propertyElem;
    QDomElement nameElem;
    QDomElement valueElem;
    QDomElement geometryElem;

    for ( int l = 0; l < propertyNodeList.count(); ++l )
    {
      propertyElem = propertyNodeList.at( l ).toElement();
      nameElem = propertyElem.elementsByTagName( QStringLiteral( "Name" ) ).at( 0 ).toElement();
      valueElem = propertyElem.elementsByTagName( QStringLiteral( "Value" ) ).at( 0 ).toElement();
      if ( nameElem.text() != QLatin1String( "geometry" ) )
      {
        propertyMap.insert( nameElem.text(), valueElem.text() );
      }
      else
      {
        geometryElem = valueElem;
      }
    }

    QDomNodeList filterNodeList = actionElem.elementsByTagName( QStringLiteral( "Filter" ) );
    QgsFeatureRequest featureRequest;
    if ( filterNodeList.size() != 1 )
    {
      QDomElement filterElem = filterNodeList.at( 0 ).toElement();
      featureRequest = parseFilterElement( typeName, filterElem );
    }

    transactionUpdate action;
    action.typeName = typeName;
    action.propertyMap = propertyMap;
    action.geometryElement = geometryElem;
    action.featureRequest = featureRequest;
    action.error = false;

    if ( actionElem.hasAttribute( QStringLiteral( "handle" ) ) )
    {
      action.handle = actionElem.attribute( QStringLiteral( "handle" ) );
    }

    return action;
  }

  transactionInsert parseInsertActionElement( QDomElement &actionElem )
  {
    QDomNodeList featureNodeList = actionElem.childNodes();
    if ( featureNodeList.size() != 1 )
    {
      throw QgsRequestNotWellFormedException( QStringLiteral( "Insert action element must have one or more child node" ) );
    }

    QString typeName;
    for ( int i = 0; i < featureNodeList.count(); ++i )
    {
      QString tempTypeName = featureNodeList.at( i ).toElement().localName();
      if ( tempTypeName.contains( QLatin1String( ":" ) ) )
        tempTypeName = tempTypeName.section( QStringLiteral( ":" ), 1, 1 );

      if ( typeName.isEmpty() )
      {
        typeName = tempTypeName;
      }
      else if ( tempTypeName != typeName )
      {
        throw QgsRequestNotWellFormedException( QStringLiteral( "Insert action element must have one typename features" ) );
      }
    }

    transactionInsert action;
    action.typeName = typeName;
    action.featureNodeList = featureNodeList;
    action.error = false;

    if ( actionElem.hasAttribute( QStringLiteral( "handle" ) ) )
    {
      action.handle = actionElem.attribute( QStringLiteral( "handle" ) );
    }

    return action;
  }

  namespace
  {

    void addTransactionResult( QDomDocument &responseDoc, QDomElement &responseElem, const QString &status,
                               const QString &locator, const QString &message )
    {
      QDomElement trElem = responseDoc.createElement( QStringLiteral( "TransactionResult" ) );
      QDomElement stElem = responseDoc.createElement( QStringLiteral( "Status" ) );
      QDomElement successElem = responseDoc.createElement( status );
      stElem.appendChild( successElem );
      trElem.appendChild( stElem );
      responseElem.appendChild( trElem );

      if ( !locator.isEmpty() )
      {
        QDomElement locElem = responseDoc.createElement( QStringLiteral( "Locator" ) );
        locElem.appendChild( responseDoc.createTextNode( locator ) );
        trElem.appendChild( locElem );
      }

      if ( !message.isEmpty() )
      {
        QDomElement mesElem = responseDoc.createElement( QStringLiteral( "Message" ) );
        mesElem.appendChild( responseDoc.createTextNode( message ) );
        trElem.appendChild( mesElem );
      }
    }

  }

} // samespace QgsWfs


