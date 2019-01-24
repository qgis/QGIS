/***************************************************************************
                              qgswfstransaction.cpp
                              -------------------------
  begin                : December 20 , 2016
  copyright            : (C) 2007 by Marco Hugentobler  (original code)
                         (C) 2012 by René-Luc D'Hont    (original code)
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
#include "qgsproject.h"


namespace QgsWfs
{
  namespace
  {
    void addTransactionResult( QDomDocument &responseDoc, QDomElement &resultsElem,
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
    else
    {
      aRequest = parseTransactionParameters( parameters );
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
    //wfs:TransactionRespone element
    QDomElement respElem = resp.createElement( QStringLiteral( "TransactionResponse" )/*wfs:TransactionResponse*/ );
    respElem.setAttribute( QStringLiteral( "xmlns" ), WFS_NAMESPACE );
    respElem.setAttribute( QStringLiteral( "xmlns:xsi" ), QStringLiteral( "http://www.w3.org/2001/XMLSchema-instance" ) );
    respElem.setAttribute( QStringLiteral( "xsi:schemaLocation" ), WFS_NAMESPACE + " http://schemas.opengis.net/wfs/1.1.0/wfs.xsd" );
    respElem.setAttribute( QStringLiteral( "xmlns:ogc" ), OGC_NAMESPACE );
    respElem.setAttribute( QStringLiteral( "version" ), QStringLiteral( "1.1.0" ) );
    resp.appendChild( respElem );

    int totalInserted = 0;
    int totalUpdated = 0;
    int totalDeleted = 0;
    int errorCount = 0;

    //wfs:TransactionResults element
    QDomElement trsElem = doc.createElement( QStringLiteral( "TransactionResults" ) );

    //wfs:InsertResults element
    QDomElement irsElem = doc.createElement( QStringLiteral( "InsertResults" ) );
    QList<transactionInsert>::iterator tiIt = aRequest.inserts.begin();
    for ( ; tiIt != aRequest.inserts.end(); ++tiIt )
    {
      transactionInsert &action = *tiIt;
      if ( action.error )
      {
        errorCount += 1;
        QString locator = action.handle;
        if ( locator.isEmpty() )
        {
          locator = QStringLiteral( "Insert:%1" ).arg( action.typeName );
        }
        addTransactionResult( resp, trsElem, locator, action.errorMsg );
      }
      else
      {
        QStringList::const_iterator fidIt = action.insertFeatureIds.constBegin();
        for ( ; fidIt != action.insertFeatureIds.constEnd(); ++fidIt )
        {
          QString fidStr = *fidIt;
          QDomElement irElem = doc.createElement( QStringLiteral( "Feature" ) );
          if ( !action.handle.isEmpty() )
          {
            irElem.setAttribute( QStringLiteral( "handle" ), action.handle );
          }
          QDomElement fiElem = doc.createElement( QStringLiteral( "ogc:FeatureId" ) );
          fiElem.setAttribute( QStringLiteral( "fid" ), fidStr );
          irElem.appendChild( fiElem );
          irsElem.appendChild( irElem );
        }
      }
      totalInserted += action.insertFeatureIds.count();
    }

    QList<transactionUpdate>::iterator tuIt = aRequest.updates.begin();
    for ( ; tuIt != aRequest.updates.end(); ++tuIt )
    {
      transactionUpdate &action = *tuIt;
      if ( action.error )
      {
        errorCount += 1;
        QString locator = action.handle;
        if ( locator.isEmpty() )
        {
          locator = QStringLiteral( "Update:%1" ).arg( action.typeName );
        }
        addTransactionResult( resp, trsElem, locator, action.errorMsg );
      }
      totalUpdated += action.totalUpdated;
    }

    QList<transactionDelete>::iterator tdIt = aRequest.deletes.begin();
    for ( ; tdIt != aRequest.deletes.end(); ++tdIt )
    {
      transactionDelete &action = *tdIt;
      if ( action.error )
      {
        errorCount += 1;
        QString locator = action.handle;
        if ( locator.isEmpty() )
        {
          locator = QStringLiteral( "Delete:%1" ).arg( action.typeName );
        }
        addTransactionResult( resp, trsElem, locator, action.errorMsg );
      }
      totalDeleted += action.totalDeleted;
    }

    //wfs:TransactionSummary element
    QDomElement summaryElem = doc.createElement( QStringLiteral( "TransactionSummary" ) );
    if ( aRequest.inserts.size() > 0 )
    {
      QDomElement totalInsertedElem = doc.createElement( QStringLiteral( "TotalInserted" ) );
      totalInsertedElem.appendChild( doc.createTextNode( QString::number( totalInserted ) ) );
      summaryElem.appendChild( totalInsertedElem );
    }
    if ( aRequest.updates.size() > 0 )
    {
      QDomElement totalUpdatedElem = doc.createElement( QStringLiteral( "TotalUpdated" ) );
      totalUpdatedElem.appendChild( doc.createTextNode( QString::number( totalUpdated ) ) );
      summaryElem.appendChild( totalUpdatedElem );
    }
    if ( aRequest.deletes.size() > 0 )
    {
      QDomElement totalDeletedElem = doc.createElement( QStringLiteral( "TotalDeleted" ) );
      totalDeletedElem.appendChild( doc.createTextNode( QString::number( totalDeleted ) ) );
      summaryElem.appendChild( totalDeletedElem );
    }
    respElem.appendChild( summaryElem );

    // add TransactionResults
    if ( errorCount > 0 && trsElem.hasChildNodes() )
    {
      respElem.appendChild( trsElem );
    }

    // add InsertResults
    if ( aRequest.inserts.size() > 0 && irsElem.hasChildNodes() )
    {
      respElem.appendChild( irsElem );
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
    std::unique_ptr< QgsOWSServerFilterRestorer > filterRestorer( new QgsOWSServerFilterRestorer() );

    // get layers
    QStringList wfsLayerIds = QgsServerProjectUtils::wfsLayerIds( *project );
    QStringList wfstUpdateLayerIds = QgsServerProjectUtils::wfstUpdateLayerIds( *project );
    QStringList wfstDeleteLayerIds = QgsServerProjectUtils::wfstDeleteLayerIds( *project );
    QStringList wfstInsertLayerIds = QgsServerProjectUtils::wfstInsertLayerIds( *project );
    QMap<QString, QgsVectorLayer *> mapLayerMap;
    for ( int i = 0; i < wfsLayerIds.size(); ++i )
    {
      QgsMapLayer *layer = project->mapLayer( wfsLayerIds.at( i ) );
      if ( !layer )
      {
        continue;
      }
      if ( layer->type() != QgsMapLayer::LayerType::VectorLayer )
      {
        continue;
      }

      QString name = layerTypeName( layer );

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
      int totalUpdated = 0;
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
          if ( value.isNull() )
          {
            if ( field.constraints().constraints() & QgsFieldConstraints::Constraint::ConstraintNotNull )
            {
              action.error = true;
              action.errorMsg = QStringLiteral( "NOT NULL constraint error on layer '%1', field '%2'" ).arg( typeName, field.name() );
              vlayer->rollBack();
              break;
            }
          }
          else  // Not NULL
          {
            if ( field.type() == QVariant::Type::Int )
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
            else if ( field.type() == QVariant::Type::Double )
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
            else if ( field.type() == QVariant::Type::LongLong )
            {
              value = it.value().toLongLong( &conversionSuccess );
              if ( !conversionSuccess )
              {
                action.error = true;
                action.errorMsg = QStringLiteral( "Property conversion error on layer '%1'" ).arg( typeName );
                vlayer->rollBack();
                break;
              }
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
        totalUpdated += 1;
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
        action.errorMsg = QStringLiteral( "Error committing updates: %1" ).arg( vlayer->commitErrors().join( QStringLiteral( "; " ) ) );
        vlayer->rollBack();
        continue;
      }
      // all the changes are OK!
      action.totalUpdated = totalUpdated;
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
        action.errorMsg = QStringLiteral( "Error committing deletes: %1" ).arg( vlayer->commitErrors().join( QStringLiteral( "; " ) ) );
        vlayer->rollBack();
        continue;
      }
      // all the changes are OK!
      action.totalDeleted = fids.count();
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
      if ( !provider->addFeatures( featureList ) )
      {
        action.error = true;
        action.errorMsg = QStringLiteral( "Insert features failed on layer '%1'" ).arg( typeName );
        if ( provider ->hasErrors() )
        {
          provider->clearErrors();
        }
        vlayer->rollBack();
        continue;
      }

      // Commit the changes of the update elements
      if ( !vlayer->commitChanges() )
      {
        action.error = true;
        action.errorMsg = QStringLiteral( "Error committing inserts: %1" ).arg( vlayer->commitErrors().join( QStringLiteral( "; " ) ) );
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
      // update feature list
      featList << feat;
    }
    return featList;
  }

  transactionRequest parseTransactionParameters( QgsServerRequest::Parameters parameters )
  {
    if ( !parameters.contains( QStringLiteral( "OPERATION" ) ) )
    {
      throw QgsRequestNotWellFormedException( QStringLiteral( "OPERATION parameter is mandatory" ) );
    }
    if ( parameters.value( QStringLiteral( "OPERATION" ) ).toUpper() != QStringLiteral( "DELETE" ) )
    {
      throw QgsRequestNotWellFormedException( QStringLiteral( "Only DELETE value is defined for OPERATION parameter" ) );
    }

    // Verifying parameters mutually exclusive
    if ( ( parameters.contains( QStringLiteral( "FEATUREID" ) )
           && ( parameters.contains( QStringLiteral( "FILTER" ) ) || parameters.contains( QStringLiteral( "BBOX" ) ) ) )
         || ( parameters.contains( QStringLiteral( "FILTER" ) )
              && ( parameters.contains( QStringLiteral( "FEATUREID" ) ) || parameters.contains( QStringLiteral( "BBOX" ) ) ) )
         || ( parameters.contains( QStringLiteral( "BBOX" ) )
              && ( parameters.contains( QStringLiteral( "FEATUREID" ) ) || parameters.contains( QStringLiteral( "FILTER" ) ) ) )
       )
    {
      throw QgsRequestNotWellFormedException( QStringLiteral( "FEATUREID FILTER and BBOX parameters are mutually exclusive" ) );
    }

    transactionRequest request;

    QStringList typeNameList;
    // parse FEATUREID
    if ( parameters.contains( QStringLiteral( "FEATUREID" ) ) )
    {
      QStringList fidList = parameters.value( QStringLiteral( "FEATUREID" ) ).split( ',' );

      QMap<QString, QgsFeatureIds> fidsMap;

      QStringList::const_iterator fidIt = fidList.constBegin();
      for ( ; fidIt != fidList.constEnd(); ++fidIt )
      {
        // Get FeatureID
        QString fid = *fidIt;
        fid = fid.trimmed();
        // testing typename in the WFS featureID
        if ( !fid.contains( '.' ) )
        {
          throw QgsRequestNotWellFormedException( QStringLiteral( "FEATUREID has to have TYPENAME in the values" ) );
        }

        QString typeName = fid.section( '.', 0, 0 );
        fid = fid.section( '.', 1, 1 );
        if ( !typeNameList.contains( typeName ) )
        {
          typeNameList << typeName;
        }

        QgsFeatureIds fids;
        if ( fidsMap.contains( typeName ) )
        {
          fids = fidsMap.value( typeName );
        }
        fids.insert( fid.toInt() );
        fidsMap.insert( typeName, fids );
      }

      QMap<QString, QgsFeatureIds>::const_iterator fidsMapIt = fidsMap.constBegin();
      while ( fidsMapIt != fidsMap.constEnd() )
      {
        transactionDelete action;
        action.typeName = fidsMapIt.key();

        QgsFeatureIds fids = fidsMapIt.value();
        action.featureRequest = QgsFeatureRequest( fids );

        request.deletes.append( action );
      }
      return request;
    }

    if ( !parameters.contains( QStringLiteral( "TYPENAME" ) ) )
    {
      throw QgsRequestNotWellFormedException( QStringLiteral( "TYPENAME is mandatory except if FEATUREID is used" ) );
    }

    typeNameList = parameters.value( QStringLiteral( "TYPENAME" ) ).split( ',' );

    // Create actions based on TypeName
    QStringList::const_iterator typeNameIt = typeNameList.constBegin();
    for ( ; typeNameIt != typeNameList.constEnd(); ++typeNameIt )
    {
      QString typeName = *typeNameIt;
      typeName = typeName.trimmed();

      transactionDelete action;
      action.typeName = typeName;

      request.deletes.append( action );
    }

    // Manage extra parameter exp_filter
    if ( parameters.contains( QStringLiteral( "EXP_FILTER" ) ) )
    {
      QString expFilterName = parameters.value( QStringLiteral( "EXP_FILTER" ) );
      QStringList expFilterList;
      QRegExp rx( "\\(([^()]+)\\)" );
      if ( rx.indexIn( expFilterName, 0 ) == -1 )
      {
        expFilterList << expFilterName;
      }
      else
      {
        int pos = 0;
        while ( ( pos = rx.indexIn( expFilterName, pos ) ) != -1 )
        {
          expFilterList << rx.cap( 1 );
          pos += rx.matchedLength();
        }
      }

      // Verifying the 1:1 mapping between TYPENAME and EXP_FILTER but without exception
      if ( request.deletes.size() == expFilterList.size() )
      {
        // set feature request filter expression based on filter element
        QList<transactionDelete>::iterator dIt = request.deletes.begin();
        QStringList::const_iterator expFilterIt = expFilterList.constBegin();
        for ( ; dIt != request.deletes.end(); ++dIt )
        {
          transactionDelete &action = *dIt;
          // Get Filter for this typeName
          QString expFilter;
          if ( expFilterIt != expFilterList.constEnd() )
          {
            expFilter = *expFilterIt;
          }
          std::shared_ptr<QgsExpression> filter( new QgsExpression( expFilter ) );
          if ( filter )
          {
            if ( filter->hasParserError() )
            {
              QgsMessageLog::logMessage( filter->parserErrorString() );
            }
            else
            {
              if ( filter->needsGeometry() )
              {
                action.featureRequest.setFlags( QgsFeatureRequest::NoFlags );
              }
              action.featureRequest.setFilterExpression( filter->expression() );
            }
          }
        }
      }
      else
      {
        QgsMessageLog::logMessage( "There has to be a 1:1 mapping between each element in a TYPENAME and the EXP_FILTER list" );
      }
    }

    if ( parameters.contains( QStringLiteral( "BBOX" ) ) )
    {
      // get bbox value
      QString bbox = parameters.value( QStringLiteral( "BBOX" ) );
      if ( bbox.isEmpty() )
      {
        throw QgsRequestNotWellFormedException( QStringLiteral( "BBOX parameter is empty" ) );
      }

      // get bbox corners
      QStringList corners = bbox.split( ',' );
      if ( corners.size() != 4 )
      {
        throw QgsRequestNotWellFormedException( QStringLiteral( "BBOX has to be composed of 4 elements: '%1'" ).arg( bbox ) );
      }

      // convert corners to double
      double d[4];
      bool ok;
      for ( int i = 0; i < 4; i++ )
      {
        corners[i].replace( ' ', '+' );
        d[i] = corners[i].toDouble( &ok );
        if ( !ok )
        {
          throw QgsRequestNotWellFormedException( QStringLiteral( "BBOX has to be composed of 4 double: '%1'" ).arg( bbox ) );
        }
      }
      // create extent
      QgsRectangle extent( d[0], d[1], d[2], d[3] );

      // set feature request filter rectangle
      QList<transactionDelete>::iterator dIt = request.deletes.begin();
      for ( ; dIt != request.deletes.end(); ++dIt )
      {
        transactionDelete &action = *dIt;
        action.featureRequest.setFilterRect( extent );
      }
      return request;
    }
    else if ( parameters.contains( QStringLiteral( "FILTER" ) ) )
    {
      QString filterName = parameters.value( QStringLiteral( "FILTER" ) );
      QStringList filterList;
      QRegExp rx( "\\(([^()]+)\\)" );
      if ( rx.indexIn( filterName, 0 ) == -1 )
      {
        filterList << filterName;
      }
      else
      {
        int pos = 0;
        while ( ( pos = rx.indexIn( filterName, pos ) ) != -1 )
        {
          filterList << rx.cap( 1 );
          pos += rx.matchedLength();
        }
      }

      // Verifying the 1:1 mapping between TYPENAME and FILTER
      if ( request.deletes.size() != filterList.size() )
      {
        throw QgsRequestNotWellFormedException( QStringLiteral( "There has to be a 1:1 mapping between each element in a TYPENAME and the FILTER list" ) );
      }

      // set feature request filter expression based on filter element
      QList<transactionDelete>::iterator dIt = request.deletes.begin();
      QStringList::const_iterator filterIt = filterList.constBegin();
      for ( ; dIt != request.deletes.end(); ++dIt )
      {
        transactionDelete &action = *dIt;

        // Get Filter for this typeName
        QDomDocument filter;
        if ( filterIt != filterList.constEnd() )
        {
          QString errorMsg;
          if ( !filter.setContent( *filterIt, true, &errorMsg ) )
          {
            throw QgsRequestNotWellFormedException( QStringLiteral( "error message: %1. The XML string was: %2" ).arg( errorMsg, *filterIt ) );
          }
        }

        QDomElement filterElem = filter.firstChildElement();
        action.featureRequest = parseFilterElement( action.typeName, filterElem );

        if ( filterIt != filterList.constEnd() )
        {
          ++filterIt;
        }
      }
      return request;
    }

    return request;
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
    if ( typeName.contains( ':' ) )
      typeName = typeName.section( ':', 1, 1 );

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
    if ( typeName.contains( ':' ) )
      typeName = typeName.section( ':', 1, 1 );

    QDomNodeList propertyNodeList = actionElem.elementsByTagName( QStringLiteral( "Property" ) );
    if ( propertyNodeList.isEmpty() )
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
    if ( filterNodeList.size() != 0 )
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
      if ( tempTypeName.contains( ':' ) )
        tempTypeName = tempTypeName.section( ':', 1, 1 );

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

    void addTransactionResult( QDomDocument &responseDoc, QDomElement &resultsElem,
                               const QString &locator, const QString &message )
    {
      QDomElement trElem = responseDoc.createElement( QStringLiteral( "Action" ) );
      resultsElem.appendChild( trElem );

      if ( !locator.isEmpty() )
      {
        trElem.setAttribute( QStringLiteral( "locator" ), locator );
      }

      if ( !message.isEmpty() )
      {
        QDomElement mesElem = responseDoc.createElement( QStringLiteral( "Message" ) );
        mesElem.appendChild( responseDoc.createTextNode( message ) );
        trElem.appendChild( mesElem );
      }
    }

  }

} // namespace QgsWfs


