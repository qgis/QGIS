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
#include "qgsgeometry.h"
#include "qgsvectordataprovider.h"
#include "qgsogcutils.h"
#include "qgswfstransaction.h"

namespace QgsWfs
{
  namespace
  {
    void addTransactionResult( QDomDocument &responseDoc, QDomElement &responseElem, const QString &status,
                               const QString &locator, const QString &message );


    QgsFeatureIds getFeatureIdsFromFilter( const QDomElement &filterElem, QgsVectorLayer *layer );

  }


  void writeTransaction( QgsServerInterface *serverIface, const QString &version,
                         const QgsServerRequest &request, QgsServerResponse &response )

  {
    QDomDocument doc = createTransactionDocument( serverIface, version, request );

    response.setHeader( "Content-Type", "text/xml; charset=utf-8" );
    response.write( doc.toByteArray() );
  }


  QDomDocument createTransactionDocument( QgsServerInterface *serverIface, const QString &version,
                                          const QgsServerRequest &request )
  {
    Q_UNUSED( version );

    QDomDocument doc;

    QgsWfsProjectParser *configParser = getConfigParser( serverIface );
#ifdef HAVE_SERVER_PYTHON_PLUGINS
    QgsAccessControl *accessControl = serverIface->accessControls();
#endif
    const QString requestBody = request.getParameter( QStringLiteral( "REQUEST_BODY" ) );

    QString errorMsg;
    if ( !doc.setContent( requestBody, true, &errorMsg ) )
    {
      throw QgsRequestNotWellFormedException( errorMsg );
    }

    QDomElement docElem = doc.documentElement();
    QDomNodeList docChildNodes = docElem.childNodes();

    // Re-organize the transaction document
    QDomDocument mDoc;
    QDomElement mDocElem = mDoc.createElement( QStringLiteral( "myTransactionDocument" ) );
    mDocElem.setAttribute( QStringLiteral( "xmlns" ), QGS_NAMESPACE );
    mDocElem.setAttribute( QStringLiteral( "xmlns:wfs" ), WFS_NAMESPACE );
    mDocElem.setAttribute( QStringLiteral( "xmlns:gml" ), GML_NAMESPACE );
    mDocElem.setAttribute( QStringLiteral( "xmlns:ogc" ), OGC_NAMESPACE );
    mDocElem.setAttribute( QStringLiteral( "xmlns:qgs" ), QGS_NAMESPACE );
    mDocElem.setAttribute( QStringLiteral( "xmlns:xsi" ), QStringLiteral( "http://www.w3.org/2001/XMLSchema-instance" ) );
    mDoc.appendChild( mDocElem );

    QDomElement actionElem;
    QString actionName;
    QDomElement typeNameElem;
    QString typeName;

    for ( int i = docChildNodes.count(); 0 < i; --i )
    {
      actionElem = docChildNodes.at( i - 1 ).toElement();
      actionName = actionElem.localName();

      if ( actionName == QLatin1String( "Insert" ) )
      {
        QDomElement featureElem = actionElem.firstChild().toElement();
        typeName = featureElem.localName();
      }
      else if ( actionName == QLatin1String( "Update" ) )
      {
        typeName = actionElem.attribute( QStringLiteral( "typeName" ) );
      }
      else if ( actionName == QLatin1String( "Delete" ) )
      {
        typeName = actionElem.attribute( QStringLiteral( "typeName" ) );
      }

      if ( typeName.contains( QLatin1String( ":" ) ) )
        typeName = typeName.section( QStringLiteral( ":" ), 1, 1 );

      QDomNodeList typeNameList = mDocElem.elementsByTagName( typeName );
      if ( typeNameList.count() == 0 )
      {
        typeNameElem = mDoc.createElement( typeName );
        mDocElem.appendChild( typeNameElem );
      }
      else
        typeNameElem = typeNameList.at( 0 ).toElement();

      typeNameElem.appendChild( actionElem );
    }

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

    // Store the created feature id for WFS
    QStringList insertResults;
    // Get the WFS layers id
    QStringList wfsLayersId = configParser->wfsLayers();;

    QList<QgsMapLayer *> layerList;
    QgsMapLayer *currentLayer = nullptr;

    // Loop through the layer transaction elements
    docChildNodes = mDocElem.childNodes();
    for ( int i = 0; i < docChildNodes.count(); ++i )
    {
      // Get the vector layer
      typeNameElem = docChildNodes.at( i ).toElement();
      typeName = typeNameElem.tagName();

      layerList = configParser->mapLayerFromTypeName( typeName );
      // Could be empty!
      if ( layerList.count() > 0 )
      {
        currentLayer = layerList.at( 0 );
      }
      else
      {
        throw QgsRequestNotWellFormedException( QStringLiteral( "Wrong TypeName: %1" ).arg( typeName ) );
      }

      QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( currentLayer );
      // it's a vectorlayer and defined by the administrator as a WFS layer
      if ( layer && wfsLayersId.contains( layer->id() ) )
      {
#ifdef HAVE_SERVER_PYTHON_PLUGINS
        if ( actionName == QLatin1String( "Insert" ) )
        {
          if ( !accessControl->layerInsertPermission( layer ) )
          {
            throw QgsSecurityAccessException( QStringLiteral( "Feature insert permission denied" ) );
          }
        }
        else if ( actionName == QLatin1String( "Update" ) )
        {
          if ( !accessControl->layerUpdatePermission( layer ) )
          {
            throw QgsSecurityAccessException( QStringLiteral( "Feature update permission denied" ) );
          }
        }
        else if ( actionName == QLatin1String( "Delete" ) )
        {
          if ( !accessControl->layerDeletePermission( layer ) )
          {
            throw QgsSecurityAccessException( QStringLiteral( "Feature delete permission denied" ) );
          }
        }
#endif

        // Get the provider and it's capabilities
        QgsVectorDataProvider *provider = layer->dataProvider();
        if ( !provider )
        {
          continue;
        }

        int cap = provider->capabilities();

        // Start the update transaction
        layer->startEditing();
        if ( ( cap & QgsVectorDataProvider::ChangeAttributeValues ) && ( cap & QgsVectorDataProvider::ChangeGeometries ) )
        {
          // Loop through the update elements for this layer
          QDomNodeList upNodeList = typeNameElem.elementsByTagNameNS( WFS_NAMESPACE, QStringLiteral( "Update" ) );
          for ( int j = 0; j < upNodeList.count(); ++j )
          {
            if ( !configParser->wfstUpdateLayers().contains( layer->id() ) )
            {
              //no wfs permissions to do updates
              QString errorMsg = "No permissions to do WFS updates on layer '" + layer->name() + "'";
              QgsMessageLog::logMessage( errorMsg, QStringLiteral( "Server" ), QgsMessageLog::CRITICAL );
              addTransactionResult( resp, respElem, QStringLiteral( "FAILED" ), QStringLiteral( "Update" ), errorMsg );
              return resp;
            }

            actionElem = upNodeList.at( j ).toElement();

            // Get the Feature Ids for this filter on the layer
            QDomElement filterElem = actionElem.elementsByTagName( QStringLiteral( "Filter" ) ).at( 0 ).toElement();
            QgsFeatureIds fids = getFeatureIdsFromFilter( filterElem, layer );

            // Loop through the property elements
            // Store properties and the geometry element
            QDomNodeList propertyNodeList = actionElem.elementsByTagName( QStringLiteral( "Property" ) );
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

            // Update the features
            QgsFields fields = provider->fields();
            QMap<QString, int> fieldMap = provider->fieldNameMap();
            QMap<QString, int>::const_iterator fieldMapIt;
            QString fieldName;
            bool conversionSuccess;

            QgsFeatureIds::const_iterator fidIt = fids.constBegin();
            for ( ; fidIt != fids.constEnd(); ++fidIt )
            {
#ifdef HAVE_SERVER_PYTHON_PLUGINS
              QgsFeatureIterator fit = layer->getFeatures( QgsFeatureRequest( *fidIt ) );
              QgsFeature feature;
              while ( fit.nextFeature( feature ) )
              {
                if ( !accessControl->allowToEdit( layer, feature ) )
                {
                  throw QgsSecurityAccessException( QStringLiteral( "Feature modify permission denied" ) );
                }
              }
#endif

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
                if ( field.type() == 2 )
                  layer->changeAttributeValue( *fidIt, fieldMapIt.value(), it.value().toInt( &conversionSuccess ) );
                else if ( field.type() == 6 )
                  layer->changeAttributeValue( *fidIt, fieldMapIt.value(), it.value().toDouble( &conversionSuccess ) );
                else
                  layer->changeAttributeValue( *fidIt, fieldMapIt.value(), it.value() );
              }

              if ( !geometryElem.isNull() )
              {
                QgsGeometry g = QgsOgcUtils::geometryFromGML( geometryElem );
                if ( !layer->changeGeometry( *fidIt, g ) )
                {
                  throw QgsRequestNotWellFormedException( QStringLiteral( "Error in change geometry" ) );
                }
              }

#ifdef HAVE_SERVER_PYTHON_PLUGINS
              fit = layer->getFeatures( QgsFeatureRequest( *fidIt ) );
              while ( fit.nextFeature( feature ) )
              {
                if ( !accessControl->allowToEdit( layer, feature ) )
                {
                  layer->rollBack();
                  throw QgsSecurityAccessException( QStringLiteral( "Feature modify permission denied" ) );
                }
              }
#endif
            }
          }
        }
        // Commit the changes of the update elements
        if ( !layer->commitChanges() )
        {
          addTransactionResult( resp, respElem, QStringLiteral( "PARTIAL" ), QStringLiteral( "Update" ), layer->commitErrors().join( QStringLiteral( "\n  " ) ) );
          return resp;
        }
        // Start the delete transaction
        layer->startEditing();
        if ( ( cap & QgsVectorDataProvider::DeleteFeatures ) )
        {
          // Loop through the delete elements
          QDomNodeList delNodeList = typeNameElem.elementsByTagNameNS( WFS_NAMESPACE, QStringLiteral( "Delete" ) );
          for ( int j = 0; j < delNodeList.count(); ++j )
          {
            if ( !configParser->wfstDeleteLayers().contains( layer->id() ) )
            {
              //no wfs permissions to do updates
              QString errorMsg = "No permissions to do WFS deletes on layer '" + layer->name() + "'";
              QgsMessageLog::logMessage( errorMsg, QStringLiteral( "Server" ), QgsMessageLog::CRITICAL );
              addTransactionResult( resp, respElem, QStringLiteral( "FAILED" ), QStringLiteral( "Delete" ), errorMsg );
              return resp;
            }

            actionElem = delNodeList.at( j ).toElement();
            QDomElement filterElem = actionElem.firstChild().toElement();
            // Get Feature Ids for the Filter element
            QgsFeatureIds fids = getFeatureIdsFromFilter( filterElem, layer );

#ifdef HAVE_SERVER_PYTHON_PLUGINS
            QgsFeatureIds::const_iterator fidIt = fids.constBegin();
            for ( ; fidIt != fids.constEnd(); ++fidIt )
            {
              QgsFeatureIterator fit = layer->getFeatures( QgsFeatureRequest( *fidIt ) );
              QgsFeature feature;
              while ( fit.nextFeature( feature ) )
              {
                if ( !accessControl->allowToEdit( layer, feature ) )
                {
                  throw QgsSecurityAccessException( QStringLiteral( "Feature modify permission denied" ) );
                }
              }
            }
#endif

            layer->selectByIds( fids );
            layer->deleteSelectedFeatures();
          }
        }
        // Commit the changes of the delete elements
        if ( !layer->commitChanges() )
        {
          addTransactionResult( resp, respElem, QStringLiteral( "PARTIAL" ), QStringLiteral( "Delete" ), layer->commitErrors().join( QStringLiteral( "\n  " ) ) );
          return resp;
        }

        // Store the inserted features
        QgsFeatureList inFeatList;
        if ( cap & QgsVectorDataProvider::AddFeatures )
        {
          // Get Layer Field Information
          QgsFields fields = provider->fields();
          QMap<QString, int> fieldMap = provider->fieldNameMap();
          QMap<QString, int>::const_iterator fieldMapIt;

          // Loop through the insert elements
          QDomNodeList inNodeList = typeNameElem.elementsByTagNameNS( WFS_NAMESPACE, QStringLiteral( "Insert" ) );
          for ( int j = 0; j < inNodeList.count(); ++j )
          {
            if ( !configParser->wfstInsertLayers().contains( layer->id() ) )
            {
              //no wfs permissions to do updates
              QString errorMsg = "No permissions to do WFS inserts on layer '" + layer->name() + "'";
              QgsMessageLog::logMessage( errorMsg, QStringLiteral( "Server" ), QgsMessageLog::CRITICAL );
              addTransactionResult( resp, respElem, QStringLiteral( "FAILED" ), QStringLiteral( "Insert" ), errorMsg );
              return resp;
            }

            actionElem = inNodeList.at( j ).toElement();
            // Loop through the feature element
            QDomNodeList featNodes = actionElem.childNodes();
            for ( int l = 0; l < featNodes.count(); l++ )
            {
              // Add the feature to the layer
              // and store it to put it's Feature Id in the response
              inFeatList << QgsFeature( fields );

              // Create feature for this layer
              QDomElement featureElem = featNodes.at( l ).toElement();

              QDomNode currentAttributeChild = featureElem.firstChild();

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
                      inFeatList.last().setAttribute( fieldMapIt.value(), attrValue.toInt() );
                    else if ( attrType == QVariant::Double )
                      inFeatList.last().setAttribute( fieldMapIt.value(), attrValue.toDouble() );
                    else
                      inFeatList.last().setAttribute( fieldMapIt.value(), attrValue );
                  }
                  else //a geometry attribute
                  {
                    QgsGeometry g = QgsOgcUtils::geometryFromGML( currentAttributeElement );
                    inFeatList.last().setGeometry( g );
                  }
                }
                currentAttributeChild = currentAttributeChild.nextSibling();
              }
            }
          }
        }
#ifdef HAVE_SERVER_PYTHON_PLUGINS
        QgsFeatureList::iterator featureIt = inFeatList.begin();
        while ( featureIt != inFeatList.end() )
        {
          if ( !accessControl->allowToEdit( layer, *featureIt ) )
          {
            throw QgsSecurityAccessException( QStringLiteral( "Feature modify permission denied" ) );
          }
          featureIt++;
        }
#endif

        // add the features
        if ( !provider->addFeatures( inFeatList ) )
        {
          addTransactionResult( resp, respElem, QStringLiteral( "Partial" ), QStringLiteral( "Insert" ), layer->commitErrors().join( QStringLiteral( "\n  " ) ) );
          if ( provider->hasErrors() )
          {
            provider->clearErrors();
          }
          return resp;
        }
        // Get the Feature Ids of the inserted feature
        for ( int j = 0; j < inFeatList.size(); j++ )
        {
          insertResults << typeName + "." + QString::number( inFeatList[j].id() );
        }
      }
    }

    // Put the Feature Ids of the inserted feature
    if ( !insertResults.isEmpty() )
    {
      Q_FOREACH ( const QString &fidStr, insertResults )
      {
        QDomElement irElem = doc.createElement( QStringLiteral( "InsertResult" ) );
        QDomElement fiElem = doc.createElement( QStringLiteral( "ogc:FeatureId" ) );
        fiElem.setAttribute( QStringLiteral( "fid" ), fidStr );
        irElem.appendChild( fiElem );
        respElem.appendChild( irElem );
      }
    }

    // Set the transaction reposne for success
    QDomElement trElem = doc.createElement( QStringLiteral( "TransactionResult" ) );
    QDomElement stElem = doc.createElement( QStringLiteral( "Status" ) );
    QDomElement successElem = doc.createElement( QStringLiteral( "SUCCESS" ) );
    stElem.appendChild( successElem );
    trElem.appendChild( stElem );
    respElem.appendChild( trElem );

    return resp;
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

      QDomElement locElem = responseDoc.createElement( QStringLiteral( "Locator" ) );
      locElem.appendChild( responseDoc.createTextNode( locator ) );
      trElem.appendChild( locElem );

      QDomElement mesElem = responseDoc.createElement( QStringLiteral( "Message" ) );
      mesElem.appendChild( responseDoc.createTextNode( message ) );
      trElem.appendChild( mesElem );
    }

    QgsFeatureIds getFeatureIdsFromFilter( const QDomElement &filterElem, QgsVectorLayer *layer )
    {
      QgsFeatureIds fids;

      QgsVectorDataProvider *provider = layer->dataProvider();
      QDomNodeList fidNodes = filterElem.elementsByTagName( QStringLiteral( "FeatureId" ) );

      if ( fidNodes.size() != 0 )
      {
        QDomElement fidElem;
        QString fid;
        bool conversionSuccess;
        for ( int i = 0; i < fidNodes.size(); ++i )
        {
          fidElem = fidNodes.at( i ).toElement();
          fid = fidElem.attribute( QStringLiteral( "fid" ) );
          if ( fid.contains( QLatin1String( "." ) ) )
            fid = fid.section( QStringLiteral( "." ), 1, 1 );
          fids.insert( fid.toLongLong( &conversionSuccess ) );
        }
      }
      else
      {
        std::shared_ptr<QgsExpression> filter( QgsOgcUtils::expressionFromOgcFilter( filterElem ) );
        if ( filter )
        {
          if ( filter->hasParserError() )
          {
            throw QgsRequestNotWellFormedException( filter->parserErrorString() );
          }
          QgsFeature feature;
          QgsFields fields = provider->fields();
          QgsFeatureIterator fit = layer->getFeatures();
          QgsExpressionContext context = QgsExpressionContextUtils::createFeatureBasedContext( feature, fields );

          while ( fit.nextFeature( feature ) )
          {
            context.setFeature( feature );
            QVariant res = filter->evaluate( &context );
            if ( filter->hasEvalError() )
            {
              throw QgsRequestNotWellFormedException( filter->evalErrorString() );
            }
            if ( res.toInt() != 0 )
            {
              fids.insert( feature.id() );
            }
          }
        }
      }

      return fids;
    }


  }



} // samespace QgsWfs


