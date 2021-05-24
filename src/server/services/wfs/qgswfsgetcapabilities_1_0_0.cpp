/***************************************************************************
                              qgswfsgecapabilities_1_0_0.cpp
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
#include "qgswfsgetcapabilities_1_0_0.h"

#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgscoordinatereferencesystem.h"

namespace QgsWfs
{
  namespace v1_0_0
  {

    /**
     * Output WFS  GetCapabilities response
     */
    void writeGetCapabilities( QgsServerInterface *serverIface, const QgsProject *project, const QString &version,
                               const QgsServerRequest &request, QgsServerResponse &response )
    {
#ifdef HAVE_SERVER_PYTHON_PLUGINS
      QgsAccessControl *accessControl = serverIface->accessControls();
#endif
      QDomDocument doc;
      const QDomDocument *capabilitiesDocument = nullptr;

#ifdef HAVE_SERVER_PYTHON_PLUGINS
      QgsServerCacheManager *cacheManager = serverIface->cacheManager();
      if ( cacheManager && cacheManager->getCachedDocument( &doc, project, request, accessControl ) )
      {
        capabilitiesDocument = &doc;
      }
      else //capabilities xml not in cache. Create a new one
      {
        doc = createGetCapabilitiesDocument( serverIface, project, version, request );

        if ( cacheManager )
        {
          cacheManager->setCachedDocument( &doc, project, request, accessControl );
        }
        capabilitiesDocument = &doc;
      }
#else
      doc = createGetCapabilitiesDocument( serverIface, project, version, request );
      capabilitiesDocument = &doc;
#endif
      response.setHeader( QStringLiteral( "Content-Type" ), QStringLiteral( "text/xml; charset=utf-8" ) );
      response.write( capabilitiesDocument->toByteArray() );
    }


    QDomDocument createGetCapabilitiesDocument( QgsServerInterface *serverIface, const QgsProject *project, const QString &version,
        const QgsServerRequest &request )
    {
      Q_UNUSED( version )

      QDomDocument doc;

      //wfs:WFS_Capabilities element
      QDomElement wfsCapabilitiesElement = doc.createElement( QStringLiteral( "WFS_Capabilities" )/*wms:WFS_Capabilities*/ );
      wfsCapabilitiesElement.setAttribute( QStringLiteral( "xmlns" ), WFS_NAMESPACE );
      wfsCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:xsi" ), QStringLiteral( "http://www.w3.org/2001/XMLSchema-instance" ) );
      wfsCapabilitiesElement.setAttribute( QStringLiteral( "xsi:schemaLocation" ), WFS_NAMESPACE + " http://schemas.opengis.net/wfs/1.0.0/WFS-capabilities.xsd" );
      wfsCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:ogc" ), OGC_NAMESPACE );
      wfsCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:gml" ), GML_NAMESPACE );
      wfsCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:ows" ), QStringLiteral( "http://www.opengis.net/ows" ) );
      wfsCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:xlink" ), QStringLiteral( "http://www.w3.org/1999/xlink" ) );
      wfsCapabilitiesElement.setAttribute( QStringLiteral( "version" ), QStringLiteral( "1.0.0" ) );
      wfsCapabilitiesElement.setAttribute( QStringLiteral( "updateSequence" ), QStringLiteral( "0" ) );
      doc.appendChild( wfsCapabilitiesElement );

      //wfs:Service
      wfsCapabilitiesElement.appendChild( getServiceElement( doc, project ) );

      //wfs:Capability
      wfsCapabilitiesElement.appendChild( getCapabilityElement( doc, project, request, serverIface->serverSettings() ) );

      //wfs:FeatureTypeList
      wfsCapabilitiesElement.appendChild( getFeatureTypeListElement( doc, serverIface, project ) );

      /*
       * Adding ogc:Filter_Capabilities in wfsCapabilitiesElement
       */
      //ogc:Filter_Capabilities element
      QDomElement filterCapabilitiesElement = doc.createElement( QStringLiteral( "ogc:Filter_Capabilities" )/*ogc:Filter_Capabilities*/ );
      wfsCapabilitiesElement.appendChild( filterCapabilitiesElement );
      QDomElement spatialCapabilitiesElement = doc.createElement( QStringLiteral( "ogc:Spatial_Capabilities" )/*ogc:Spatial_Capabilities*/ );
      filterCapabilitiesElement.appendChild( spatialCapabilitiesElement );
      QDomElement spatialOperatorsElement = doc.createElement( QStringLiteral( "ogc:Spatial_Operators" )/*ogc:Spatial_Operators*/ );
      spatialCapabilitiesElement.appendChild( spatialOperatorsElement );
      spatialOperatorsElement.appendChild( doc.createElement( QStringLiteral( "ogc:BBOX" )/*ogc:BBOX*/ ) );
      spatialOperatorsElement.appendChild( doc.createElement( QStringLiteral( "ogc:Disjoint" )/*ogc:Disjoint*/ ) );
      spatialOperatorsElement.appendChild( doc.createElement( QStringLiteral( "ogc:Intersect" )/*ogc:Intersects*/ ) );
      spatialOperatorsElement.appendChild( doc.createElement( QStringLiteral( "ogc:Touches" )/*ogc:Touches*/ ) );
      spatialOperatorsElement.appendChild( doc.createElement( QStringLiteral( "ogc:Crosses" )/*ogc:Crosses*/ ) );
      spatialOperatorsElement.appendChild( doc.createElement( QStringLiteral( "ogc:Contains" )/*ogc:Contains*/ ) );
      spatialOperatorsElement.appendChild( doc.createElement( QStringLiteral( "ogc:Overlaps" )/*ogc:Overlaps*/ ) );
      spatialOperatorsElement.appendChild( doc.createElement( QStringLiteral( "ogc:Within" )/*ogc:Within*/ ) );
      QDomElement scalarCapabilitiesElement = doc.createElement( QStringLiteral( "ogc:Scalar_Capabilities" )/*ogc:Scalar_Capabilities*/ );
      filterCapabilitiesElement.appendChild( scalarCapabilitiesElement );
      QDomElement comparisonOperatorsElement = doc.createElement( QStringLiteral( "ogc:Comparison_Operators" )/*ogc:Comparison_Operators*/ );
      scalarCapabilitiesElement.appendChild( comparisonOperatorsElement );
      comparisonOperatorsElement.appendChild( doc.createElement( QStringLiteral( "ogc:Simple_Comparisons" )/*ogc:Simple_Comparisons*/ ) );
      comparisonOperatorsElement.appendChild( doc.createElement( QStringLiteral( "ogc:Between" )/*ogc:Between*/ ) );
      comparisonOperatorsElement.appendChild( doc.createElement( QStringLiteral( "ogc:Like" )/*ogc:Like*/ ) );

      return doc;

    }

    QDomElement getServiceElement( QDomDocument &doc, const QgsProject *project )
    {
      //Service element
      QDomElement serviceElem = doc.createElement( QStringLiteral( "Service" ) );

      //Service name
      QDomElement nameElem = doc.createElement( QStringLiteral( "Name" ) );
      QDomText nameText = doc.createTextNode( "WFS" );
      nameElem.appendChild( nameText );
      serviceElem.appendChild( nameElem );

      const QString title = QgsServerProjectUtils::owsServiceTitle( *project );
      if ( !title.isEmpty() )
      {
        QDomElement titleElem = doc.createElement( QStringLiteral( "Title" ) );
        QDomText titleText = doc.createTextNode( title );
        titleElem.appendChild( titleText );
        serviceElem.appendChild( titleElem );
      }

      const QString abstract = QgsServerProjectUtils::owsServiceAbstract( *project );
      if ( !abstract.isEmpty() )
      {
        QDomElement abstractElem = doc.createElement( QStringLiteral( "Abstract" ) );
        QDomText abstractText = doc.createCDATASection( abstract );
        abstractElem.appendChild( abstractText );
        serviceElem.appendChild( abstractElem );
      }

      const QStringList keywords = QgsServerProjectUtils::owsServiceKeywords( *project );
      if ( !keywords.isEmpty() && !keywords.join( QLatin1String( ", " ) ).isEmpty() )
      {
        QDomElement keywordsElem = doc.createElement( QStringLiteral( "Keywords" ) );
        QDomText keywordsText = doc.createTextNode( keywords.join( QLatin1String( ", " ) ) );
        keywordsElem.appendChild( keywordsText );
        serviceElem.appendChild( keywordsElem );
      }

      QDomElement onlineResourceElem = doc.createElement( QStringLiteral( "OnlineResource" ) );
      const QString onlineResource = QgsServerProjectUtils::owsServiceOnlineResource( *project );
      if ( !onlineResource.isEmpty() )
      {
        QDomText onlineResourceText = doc.createTextNode( onlineResource );
        onlineResourceElem.appendChild( onlineResourceText );
      }
      serviceElem.appendChild( onlineResourceElem );

      const QString fees = QgsServerProjectUtils::owsServiceFees( *project );
      if ( !fees.isEmpty() )
      {
        QDomElement feesElem = doc.createElement( QStringLiteral( "Fees" ) );
        QDomText feesText = doc.createTextNode( fees );
        feesElem.appendChild( feesText );
        serviceElem.appendChild( feesElem );
      }

      const QString accessConstraints = QgsServerProjectUtils::owsServiceAccessConstraints( *project );
      if ( !accessConstraints.isEmpty() )
      {
        QDomElement accessConstraintsElem = doc.createElement( QStringLiteral( "AccessConstraints" ) );
        QDomText accessConstraintsText = doc.createTextNode( accessConstraints );
        accessConstraintsElem.appendChild( accessConstraintsText );
        serviceElem.appendChild( accessConstraintsElem );
      }

      return serviceElem;

    }

    QDomElement getCapabilityElement( QDomDocument &doc, const QgsProject *project, const QgsServerRequest &request, const QgsServerSettings *settings )
    {
      //wfs:Capability element
      QDomElement capabilityElement = doc.createElement( QStringLiteral( "Capability" )/*wfs:Capability*/ );

      //wfs:Request element
      QDomElement requestElement = doc.createElement( QStringLiteral( "Request" )/*wfs:Request*/ );
      capabilityElement.appendChild( requestElement );
      //wfs:GetCapabilities
      QDomElement getCapabilitiesElement = doc.createElement( QStringLiteral( "GetCapabilities" )/*wfs:GetCapabilities*/ );
      requestElement.appendChild( getCapabilitiesElement );

      QDomElement dcpTypeElement = doc.createElement( QStringLiteral( "DCPType" )/*wfs:DCPType*/ );
      getCapabilitiesElement.appendChild( dcpTypeElement );
      QDomElement httpElement = doc.createElement( QStringLiteral( "HTTP" )/*wfs:HTTP*/ );
      dcpTypeElement.appendChild( httpElement );

      //Prepare url
      QString hrefString = serviceUrl( request, project, *settings );

      //only Get supported for the moment
      QDomElement getElement = doc.createElement( QStringLiteral( "Get" )/*wfs:Get*/ );
      httpElement.appendChild( getElement );
      getElement.setAttribute( QStringLiteral( "onlineResource" ), hrefString );
      QDomElement getCapabilitiesDhcTypePostElement = dcpTypeElement.cloneNode().toElement();//this is the same as for 'GetCapabilities'
      getCapabilitiesDhcTypePostElement.firstChild().firstChild().toElement().setTagName( QStringLiteral( "Post" ) );
      getCapabilitiesElement.appendChild( getCapabilitiesDhcTypePostElement );

      //wfs:DescribeFeatureType
      QDomElement describeFeatureTypeElement = doc.createElement( QStringLiteral( "DescribeFeatureType" )/*wfs:DescribeFeatureType*/ );
      requestElement.appendChild( describeFeatureTypeElement );
      QDomElement schemaDescriptionLanguageElement = doc.createElement( QStringLiteral( "SchemaDescriptionLanguage" )/*wfs:SchemaDescriptionLanguage*/ );
      describeFeatureTypeElement.appendChild( schemaDescriptionLanguageElement );
      QDomElement xmlSchemaElement = doc.createElement( QStringLiteral( "XMLSCHEMA" )/*wfs:XMLSCHEMA*/ );
      schemaDescriptionLanguageElement.appendChild( xmlSchemaElement );
      QDomElement describeFeatureTypeDhcTypeElement = dcpTypeElement.cloneNode().toElement();//this is the same as for 'GetCapabilities'
      describeFeatureTypeElement.appendChild( describeFeatureTypeDhcTypeElement );
      QDomElement describeFeatureTypeDhcTypePostElement = dcpTypeElement.cloneNode().toElement();//this is the same as for 'GetCapabilities'
      describeFeatureTypeDhcTypePostElement.firstChild().firstChild().toElement().setTagName( QStringLiteral( "Post" ) );
      describeFeatureTypeElement.appendChild( describeFeatureTypeDhcTypePostElement );

      //wfs:GetFeature
      QDomElement getFeatureElement = doc.createElement( QStringLiteral( "GetFeature" )/*wfs:GetFeature*/ );
      requestElement.appendChild( getFeatureElement );
      QDomElement getFeatureFormatElement = doc.createElement( QStringLiteral( "ResultFormat" ) );/*wfs:ResultFormat*/
      getFeatureElement.appendChild( getFeatureFormatElement );
      QDomElement gmlFormatElement = doc.createElement( QStringLiteral( "GML2" ) );/*wfs:GML2*/
      getFeatureFormatElement.appendChild( gmlFormatElement );
      QDomElement gml3FormatElement = doc.createElement( QStringLiteral( "GML3" ) );/*wfs:GML3*/
      getFeatureFormatElement.appendChild( gml3FormatElement );
      QDomElement geojsonFormatElement = doc.createElement( QStringLiteral( "GeoJSON" ) );/*wfs:GeoJSON*/
      getFeatureFormatElement.appendChild( geojsonFormatElement );
      QDomElement getFeatureDhcTypeGetElement = dcpTypeElement.cloneNode().toElement();//this is the same as for 'GetCapabilities'
      getFeatureElement.appendChild( getFeatureDhcTypeGetElement );
      QDomElement getFeatureDhcTypePostElement = dcpTypeElement.cloneNode().toElement();//this is the same as for 'GetCapabilities'
      getFeatureDhcTypePostElement.firstChild().firstChild().toElement().setTagName( QStringLiteral( "Post" ) );
      getFeatureElement.appendChild( getFeatureDhcTypePostElement );

      //wfs:Transaction
      QDomElement transactionElement = doc.createElement( QStringLiteral( "Transaction" )/*wfs:Transaction*/ );
      requestElement.appendChild( transactionElement );
      QDomElement transactionDhcTypeElement = dcpTypeElement.cloneNode().toElement();//this is the same as for 'GetCapabilities'
      transactionDhcTypeElement.firstChild().firstChild().toElement().setTagName( QStringLiteral( "Post" ) );
      transactionElement.appendChild( transactionDhcTypeElement );

      return capabilityElement;
    }

    QDomElement getFeatureTypeListElement( QDomDocument &doc, QgsServerInterface *serverIface, const QgsProject *project )
    {
#ifdef HAVE_SERVER_PYTHON_PLUGINS
      QgsAccessControl *accessControl = serverIface->accessControls();
#else
      ( void )serverIface;
#endif

      //wfs:FeatureTypeList element
      QDomElement featureTypeListElement = doc.createElement( QStringLiteral( "FeatureTypeList" )/*wfs:FeatureTypeList*/ );
      //wfs:Operations element
      QDomElement operationsElement = doc.createElement( QStringLiteral( "Operations" )/*wfs:Operations*/ );
      featureTypeListElement.appendChild( operationsElement );
      //wfs:Query element
      QDomElement queryElement = doc.createElement( QStringLiteral( "Query" )/*wfs:Query*/ );
      operationsElement.appendChild( queryElement );

      const QStringList wfsLayerIds = QgsServerProjectUtils::wfsLayerIds( *project );
      const QStringList wfstUpdateLayersId = QgsServerProjectUtils::wfstUpdateLayerIds( *project );
      const QStringList wfstInsertLayersId = QgsServerProjectUtils::wfstInsertLayerIds( *project );
      const QStringList wfstDeleteLayersId = QgsServerProjectUtils::wfstDeleteLayerIds( *project );
      for ( const QString &wfsLayerId : wfsLayerIds )
      {
        QgsMapLayer *layer = project->mapLayer( wfsLayerId );
        if ( !layer )
        {
          continue;
        }
        if ( layer->type() != QgsMapLayerType::VectorLayer )
        {
          continue;
        }
#ifdef HAVE_SERVER_PYTHON_PLUGINS
        if ( accessControl && !accessControl->layerReadPermission( layer ) )
        {
          continue;
        }
#endif
        QDomElement layerElem = doc.createElement( QStringLiteral( "FeatureType" ) );

        //create Name
        QDomElement nameElem = doc.createElement( QStringLiteral( "Name" ) );
        QString typeName = layer->name();
        if ( !layer->shortName().isEmpty() )
          typeName = layer->shortName();
        typeName = typeName.replace( QLatin1String( " " ), QLatin1String( "_" ) );
        QDomText nameText = doc.createTextNode( typeName );
        nameElem.appendChild( nameText );
        layerElem.appendChild( nameElem );

        //create Title
        QDomElement titleElem = doc.createElement( QStringLiteral( "Title" ) );
        QString title = layer->title();
        if ( title.isEmpty() )
        {
          title = layer->name();
        }
        QDomText titleText = doc.createTextNode( title );
        titleElem.appendChild( titleText );
        layerElem.appendChild( titleElem );

        //create Abstract
        QString abstract = layer->abstract();
        if ( !abstract.isEmpty() )
        {
          QDomElement abstractElem = doc.createElement( QStringLiteral( "Abstract" ) );
          QDomText abstractText = doc.createTextNode( abstract );
          abstractElem.appendChild( abstractText );
          layerElem.appendChild( abstractElem );
        }

        //create keywords
        QString keywords = layer->keywordList();
        if ( !keywords.isEmpty() )
        {
          QDomElement keywordsElem = doc.createElement( QStringLiteral( "Keywords" ) );
          QDomText keywordsText = doc.createTextNode( keywords );
          keywordsElem.appendChild( keywordsText );
          layerElem.appendChild( keywordsElem );
        }

        //create SRS
        QDomElement srsElem = doc.createElement( QStringLiteral( "SRS" ) );
        QDomText srsText = doc.createTextNode( layer->crs().authid() );
        srsElem.appendChild( srsText );
        layerElem.appendChild( srsElem );

        // Define precision
        int precision = 3;
        if ( layer->crs().isGeographic() )
        {
          precision = 6;
        }

        //create LatLongBoundingBox
        QgsRectangle layerExtent = layer->extent();
        QDomElement bBoxElement = doc.createElement( QStringLiteral( "LatLongBoundingBox" ) );
        bBoxElement.setAttribute( QStringLiteral( "minx" ), qgsDoubleToString( QgsServerProjectUtils::floorWithPrecision( layerExtent.xMinimum(), precision ), precision ) );
        bBoxElement.setAttribute( QStringLiteral( "miny" ), qgsDoubleToString( QgsServerProjectUtils::floorWithPrecision( layerExtent.yMinimum(), precision ), precision ) );
        bBoxElement.setAttribute( QStringLiteral( "maxx" ), qgsDoubleToString( QgsServerProjectUtils::ceilWithPrecision( layerExtent.xMaximum(), precision ), precision ) );
        bBoxElement.setAttribute( QStringLiteral( "maxy" ), qgsDoubleToString( QgsServerProjectUtils::ceilWithPrecision( layerExtent.yMaximum(), precision ), precision ) );
        layerElem.appendChild( bBoxElement );

        // layer metadata URL
        QString metadataUrl = layer->metadataUrl();
        if ( !metadataUrl.isEmpty() )
        {
          QDomElement metaUrlElem = doc.createElement( QStringLiteral( "MetadataURL" ) );
          QString metadataUrlType = layer->metadataUrlType();
          metaUrlElem.setAttribute( QStringLiteral( "type" ), metadataUrlType );
          QString metadataUrlFormat = layer->metadataUrlFormat();
          if ( metadataUrlFormat == QLatin1String( "text/xml" ) )
          {
            metaUrlElem.setAttribute( QStringLiteral( "format" ), QStringLiteral( "XML" ) );
          }
          else
          {
            metaUrlElem.setAttribute( QStringLiteral( "format" ), QStringLiteral( "TXT" ) );
          }
          QDomText metaUrlText = doc.createTextNode( metadataUrl );
          metaUrlElem.appendChild( metaUrlText );
          layerElem.appendChild( metaUrlElem );
        }

        //wfs:Operations element
        QDomElement operationsElement = doc.createElement( QStringLiteral( "Operations" )/*wfs:Operations*/ );
        //wfs:Query element
        QDomElement queryElement = doc.createElement( QStringLiteral( "Query" )/*wfs:Query*/ );
        operationsElement.appendChild( queryElement );
        if ( wfstUpdateLayersId.contains( layer->id() ) ||
             wfstInsertLayersId.contains( layer->id() ) ||
             wfstDeleteLayersId.contains( layer->id() ) )
        {
          QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
          QgsVectorDataProvider *provider = vlayer->dataProvider();
          if ( ( provider->capabilities() & QgsVectorDataProvider::AddFeatures ) && wfstInsertLayersId.contains( layer->id() ) )
          {
            //wfs:Insert element
            QDomElement insertElement = doc.createElement( QStringLiteral( "Insert" )/*wfs:Insert*/ );
            operationsElement.appendChild( insertElement );
          }
          if ( ( provider->capabilities() & QgsVectorDataProvider::ChangeAttributeValues ) &&
               ( provider->capabilities() & QgsVectorDataProvider::ChangeGeometries ) &&
               wfstUpdateLayersId.contains( layer->id() ) )
          {
            //wfs:Update element
            QDomElement updateElement = doc.createElement( QStringLiteral( "Update" )/*wfs:Update*/ );
            operationsElement.appendChild( updateElement );
          }
          if ( ( provider->capabilities() & QgsVectorDataProvider::DeleteFeatures ) && wfstDeleteLayersId.contains( layer->id() ) )
          {
            //wfs:Delete element
            QDomElement deleteElement = doc.createElement( QStringLiteral( "Delete" )/*wfs:Delete*/ );
            operationsElement.appendChild( deleteElement );
          }
        }

        layerElem.appendChild( operationsElement );

        featureTypeListElement.appendChild( layerElem );
      }

      return featureTypeListElement;
    }

  } // namespace v1_0_0
} // namespace QgsWfs



