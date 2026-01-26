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
#include "qgswfsgetcapabilities_1_0_0.h"

#include "qgscoordinatereferencesystem.h"
#include "qgsproject.h"
#include "qgsserverprojectutils.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgswfsutils.h"

namespace QgsWfs
{
  namespace v1_0_0
  {

    /**
     * Output WFS  GetCapabilities response
     */
    void writeGetCapabilities( QgsServerInterface *serverIface, const QgsProject *project, const QString &version, const QgsServerRequest &request, QgsServerResponse &response )
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
      response.setHeader( u"Content-Type"_s, u"text/xml; charset=utf-8"_s );
      response.write( capabilitiesDocument->toByteArray() );
    }


    QDomDocument createGetCapabilitiesDocument( QgsServerInterface *serverIface, const QgsProject *project, const QString &version, const QgsServerRequest &request )
    {
      Q_UNUSED( version )

      QDomDocument doc;

      //wfs:WFS_Capabilities element
      QDomElement wfsCapabilitiesElement = doc.createElement( u"WFS_Capabilities"_s /*wms:WFS_Capabilities*/ );
      wfsCapabilitiesElement.setAttribute( u"xmlns"_s, WFS_NAMESPACE );
      wfsCapabilitiesElement.setAttribute( u"xmlns:xsi"_s, u"http://www.w3.org/2001/XMLSchema-instance"_s );
      wfsCapabilitiesElement.setAttribute( u"xsi:schemaLocation"_s, WFS_NAMESPACE + " http://schemas.opengis.net/wfs/1.0.0/WFS-capabilities.xsd" );
      wfsCapabilitiesElement.setAttribute( u"xmlns:ogc"_s, OGC_NAMESPACE );
      wfsCapabilitiesElement.setAttribute( u"xmlns:gml"_s, GML_NAMESPACE );
      wfsCapabilitiesElement.setAttribute( u"xmlns:ows"_s, u"http://www.opengis.net/ows"_s );
      wfsCapabilitiesElement.setAttribute( u"xmlns:xlink"_s, u"http://www.w3.org/1999/xlink"_s );
      wfsCapabilitiesElement.setAttribute( u"version"_s, u"1.0.0"_s );
      wfsCapabilitiesElement.setAttribute( u"updateSequence"_s, u"0"_s );
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
      QDomElement filterCapabilitiesElement = doc.createElement( u"ogc:Filter_Capabilities"_s /*ogc:Filter_Capabilities*/ );
      wfsCapabilitiesElement.appendChild( filterCapabilitiesElement );
      QDomElement spatialCapabilitiesElement = doc.createElement( u"ogc:Spatial_Capabilities"_s /*ogc:Spatial_Capabilities*/ );
      filterCapabilitiesElement.appendChild( spatialCapabilitiesElement );
      QDomElement spatialOperatorsElement = doc.createElement( u"ogc:Spatial_Operators"_s /*ogc:Spatial_Operators*/ );
      spatialCapabilitiesElement.appendChild( spatialOperatorsElement );
      spatialOperatorsElement.appendChild( doc.createElement( u"ogc:BBOX"_s /*ogc:BBOX*/ ) );
      spatialOperatorsElement.appendChild( doc.createElement( u"ogc:Disjoint"_s /*ogc:Disjoint*/ ) );
      spatialOperatorsElement.appendChild( doc.createElement( u"ogc:Intersect"_s /*ogc:Intersects*/ ) );
      spatialOperatorsElement.appendChild( doc.createElement( u"ogc:Touches"_s /*ogc:Touches*/ ) );
      spatialOperatorsElement.appendChild( doc.createElement( u"ogc:Crosses"_s /*ogc:Crosses*/ ) );
      spatialOperatorsElement.appendChild( doc.createElement( u"ogc:Contains"_s /*ogc:Contains*/ ) );
      spatialOperatorsElement.appendChild( doc.createElement( u"ogc:Overlaps"_s /*ogc:Overlaps*/ ) );
      spatialOperatorsElement.appendChild( doc.createElement( u"ogc:Within"_s /*ogc:Within*/ ) );
      QDomElement scalarCapabilitiesElement = doc.createElement( u"ogc:Scalar_Capabilities"_s /*ogc:Scalar_Capabilities*/ );
      filterCapabilitiesElement.appendChild( scalarCapabilitiesElement );
      QDomElement comparisonOperatorsElement = doc.createElement( u"ogc:Comparison_Operators"_s /*ogc:Comparison_Operators*/ );
      scalarCapabilitiesElement.appendChild( comparisonOperatorsElement );
      comparisonOperatorsElement.appendChild( doc.createElement( u"ogc:Simple_Comparisons"_s /*ogc:Simple_Comparisons*/ ) );
      comparisonOperatorsElement.appendChild( doc.createElement( u"ogc:Between"_s /*ogc:Between*/ ) );
      comparisonOperatorsElement.appendChild( doc.createElement( u"ogc:Like"_s /*ogc:Like*/ ) );

      return doc;
    }

    QDomElement getServiceElement( QDomDocument &doc, const QgsProject *project )
    {
      //Service element
      QDomElement serviceElem = doc.createElement( u"Service"_s );

      //Service name
      QDomElement nameElem = doc.createElement( u"Name"_s );
      const QDomText nameText = doc.createTextNode( "WFS" );
      nameElem.appendChild( nameText );
      serviceElem.appendChild( nameElem );

      const QString title = QgsServerProjectUtils::owsServiceTitle( *project );
      QDomElement titleElem = doc.createElement( u"Title"_s );
      const QDomText titleText = doc.createTextNode( title );
      titleElem.appendChild( titleText );
      serviceElem.appendChild( titleElem );

      const QString abstract = QgsServerProjectUtils::owsServiceAbstract( *project );
      if ( !abstract.isEmpty() )
      {
        QDomElement abstractElem = doc.createElement( u"Abstract"_s );
        const QDomText abstractText = doc.createCDATASection( abstract );
        abstractElem.appendChild( abstractText );
        serviceElem.appendChild( abstractElem );
      }

      const QStringList keywords = QgsServerProjectUtils::owsServiceKeywords( *project );
      if ( !keywords.isEmpty() && !keywords.join( ", "_L1 ).isEmpty() )
      {
        QDomElement keywordsElem = doc.createElement( u"Keywords"_s );
        const QDomText keywordsText = doc.createTextNode( keywords.join( ", "_L1 ) );
        keywordsElem.appendChild( keywordsText );
        serviceElem.appendChild( keywordsElem );
      }

      QDomElement onlineResourceElem = doc.createElement( u"OnlineResource"_s );
      const QString onlineResource = QgsServerProjectUtils::owsServiceOnlineResource( *project );
      if ( !onlineResource.isEmpty() )
      {
        const QDomText onlineResourceText = doc.createTextNode( onlineResource );
        onlineResourceElem.appendChild( onlineResourceText );
      }
      serviceElem.appendChild( onlineResourceElem );

      const QString fees = QgsServerProjectUtils::owsServiceFees( *project );
      if ( !fees.isEmpty() )
      {
        QDomElement feesElem = doc.createElement( u"Fees"_s );
        const QDomText feesText = doc.createTextNode( fees );
        feesElem.appendChild( feesText );
        serviceElem.appendChild( feesElem );
      }

      const QString accessConstraints = QgsServerProjectUtils::owsServiceAccessConstraints( *project );
      if ( !accessConstraints.isEmpty() )
      {
        QDomElement accessConstraintsElem = doc.createElement( u"AccessConstraints"_s );
        const QDomText accessConstraintsText = doc.createTextNode( accessConstraints );
        accessConstraintsElem.appendChild( accessConstraintsText );
        serviceElem.appendChild( accessConstraintsElem );
      }

      return serviceElem;
    }

    QDomElement getCapabilityElement( QDomDocument &doc, const QgsProject *project, const QgsServerRequest &request, const QgsServerSettings *settings )
    {
      //wfs:Capability element
      QDomElement capabilityElement = doc.createElement( u"Capability"_s /*wfs:Capability*/ );

      //wfs:Request element
      QDomElement requestElement = doc.createElement( u"Request"_s /*wfs:Request*/ );
      capabilityElement.appendChild( requestElement );
      //wfs:GetCapabilities
      QDomElement getCapabilitiesElement = doc.createElement( u"GetCapabilities"_s /*wfs:GetCapabilities*/ );
      requestElement.appendChild( getCapabilitiesElement );

      QDomElement dcpTypeElement = doc.createElement( u"DCPType"_s /*wfs:DCPType*/ );
      getCapabilitiesElement.appendChild( dcpTypeElement );
      QDomElement httpElement = doc.createElement( u"HTTP"_s /*wfs:HTTP*/ );
      dcpTypeElement.appendChild( httpElement );

      //Prepare url
      const QString hrefString = serviceUrl( request, project, *settings );

      //only Get supported for the moment
      QDomElement getElement = doc.createElement( u"Get"_s /*wfs:Get*/ );
      httpElement.appendChild( getElement );
      getElement.setAttribute( u"onlineResource"_s, hrefString );
      const QDomElement getCapabilitiesDhcTypePostElement = dcpTypeElement.cloneNode().toElement(); //this is the same as for 'GetCapabilities'
      getCapabilitiesDhcTypePostElement.firstChild().firstChild().toElement().setTagName( u"Post"_s );
      getCapabilitiesElement.appendChild( getCapabilitiesDhcTypePostElement );

      //wfs:DescribeFeatureType
      QDomElement describeFeatureTypeElement = doc.createElement( u"DescribeFeatureType"_s /*wfs:DescribeFeatureType*/ );
      requestElement.appendChild( describeFeatureTypeElement );
      QDomElement schemaDescriptionLanguageElement = doc.createElement( u"SchemaDescriptionLanguage"_s /*wfs:SchemaDescriptionLanguage*/ );
      describeFeatureTypeElement.appendChild( schemaDescriptionLanguageElement );
      const QDomElement xmlSchemaElement = doc.createElement( u"XMLSCHEMA"_s /*wfs:XMLSCHEMA*/ );
      schemaDescriptionLanguageElement.appendChild( xmlSchemaElement );
      const QDomElement describeFeatureTypeDhcTypeElement = dcpTypeElement.cloneNode().toElement(); //this is the same as for 'GetCapabilities'
      describeFeatureTypeElement.appendChild( describeFeatureTypeDhcTypeElement );
      const QDomElement describeFeatureTypeDhcTypePostElement = dcpTypeElement.cloneNode().toElement(); //this is the same as for 'GetCapabilities'
      describeFeatureTypeDhcTypePostElement.firstChild().firstChild().toElement().setTagName( u"Post"_s );
      describeFeatureTypeElement.appendChild( describeFeatureTypeDhcTypePostElement );

      //wfs:GetFeature
      QDomElement getFeatureElement = doc.createElement( u"GetFeature"_s /*wfs:GetFeature*/ );
      requestElement.appendChild( getFeatureElement );
      QDomElement getFeatureFormatElement = doc.createElement( u"ResultFormat"_s ); /*wfs:ResultFormat*/
      getFeatureElement.appendChild( getFeatureFormatElement );
      const QDomElement gmlFormatElement = doc.createElement( u"GML2"_s ); /*wfs:GML2*/
      getFeatureFormatElement.appendChild( gmlFormatElement );
      const QDomElement gml3FormatElement = doc.createElement( u"GML3"_s ); /*wfs:GML3*/
      getFeatureFormatElement.appendChild( gml3FormatElement );
      const QDomElement geojsonFormatElement = doc.createElement( u"GeoJSON"_s ); /*wfs:GeoJSON*/
      getFeatureFormatElement.appendChild( geojsonFormatElement );
      const QDomElement getFeatureDhcTypeGetElement = dcpTypeElement.cloneNode().toElement(); //this is the same as for 'GetCapabilities'
      getFeatureElement.appendChild( getFeatureDhcTypeGetElement );
      const QDomElement getFeatureDhcTypePostElement = dcpTypeElement.cloneNode().toElement(); //this is the same as for 'GetCapabilities'
      getFeatureDhcTypePostElement.firstChild().firstChild().toElement().setTagName( u"Post"_s );
      getFeatureElement.appendChild( getFeatureDhcTypePostElement );

      //wfs:Transaction
      QDomElement transactionElement = doc.createElement( u"Transaction"_s /*wfs:Transaction*/ );
      requestElement.appendChild( transactionElement );
      const QDomElement transactionDhcTypeElement = dcpTypeElement.cloneNode().toElement(); //this is the same as for 'GetCapabilities'
      transactionDhcTypeElement.firstChild().firstChild().toElement().setTagName( u"Post"_s );
      transactionElement.appendChild( transactionDhcTypeElement );

      return capabilityElement;
    }

    QDomElement getFeatureTypeListElement( QDomDocument &doc, QgsServerInterface *serverIface, const QgsProject *project )
    {
#ifdef HAVE_SERVER_PYTHON_PLUGINS
      QgsAccessControl *accessControl = serverIface->accessControls();
#else
      ( void ) serverIface;
#endif

      //wfs:FeatureTypeList element
      QDomElement featureTypeListElement = doc.createElement( u"FeatureTypeList"_s /*wfs:FeatureTypeList*/ );
      //wfs:Operations element
      QDomElement operationsElement = doc.createElement( u"Operations"_s /*wfs:Operations*/ );
      featureTypeListElement.appendChild( operationsElement );
      //wfs:Query element
      const QDomElement queryElement = doc.createElement( u"Query"_s /*wfs:Query*/ );
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
        if ( layer->type() != Qgis::LayerType::Vector )
        {
          continue;
        }
#ifdef HAVE_SERVER_PYTHON_PLUGINS
        if ( accessControl && !accessControl->layerReadPermission( layer ) )
        {
          continue;
        }
#endif
        QDomElement layerElem = doc.createElement( u"FeatureType"_s );

        //create Name
        QDomElement nameElem = doc.createElement( u"Name"_s );
        QString typeName = layer->name();
        if ( !layer->serverProperties()->shortName().isEmpty() )
          typeName = layer->serverProperties()->shortName();
        typeName = typeName.replace( " "_L1, "_"_L1 );
        const QDomText nameText = doc.createTextNode( typeName );
        nameElem.appendChild( nameText );
        layerElem.appendChild( nameElem );

        //create Title
        QDomElement titleElem = doc.createElement( u"Title"_s );
        QString title = layer->serverProperties()->title();
        if ( title.isEmpty() )
        {
          title = layer->name();
        }
        const QDomText titleText = doc.createTextNode( title );
        titleElem.appendChild( titleText );
        layerElem.appendChild( titleElem );

        //create Abstract
        const QString abstract = layer->serverProperties()->abstract();
        if ( !abstract.isEmpty() )
        {
          QDomElement abstractElem = doc.createElement( u"Abstract"_s );
          const QDomText abstractText = doc.createTextNode( abstract );
          abstractElem.appendChild( abstractText );
          layerElem.appendChild( abstractElem );
        }

        //create keywords
        const QString keywords = layer->serverProperties()->keywordList();
        if ( !keywords.isEmpty() )
        {
          QDomElement keywordsElem = doc.createElement( u"Keywords"_s );
          const QDomText keywordsText = doc.createTextNode( keywords );
          keywordsElem.appendChild( keywordsText );
          layerElem.appendChild( keywordsElem );
        }

        //create SRS
        QDomElement srsElem = doc.createElement( u"SRS"_s );
        const QDomText srsText = doc.createTextNode( layer->crs().authid() );
        srsElem.appendChild( srsText );
        layerElem.appendChild( srsElem );

        // Define precision
        int precision = 3;
        if ( layer->crs().isGeographic() )
        {
          precision = 6;
        }

        //create LatLongBoundingBox
        const QgsRectangle layerExtent = layer->extent();
        QDomElement bBoxElement = doc.createElement( u"LatLongBoundingBox"_s );
        bBoxElement.setAttribute( u"minx"_s, qgsDoubleToString( QgsServerProjectUtils::floorWithPrecision( layerExtent.xMinimum(), precision ), precision ) );
        bBoxElement.setAttribute( u"miny"_s, qgsDoubleToString( QgsServerProjectUtils::floorWithPrecision( layerExtent.yMinimum(), precision ), precision ) );
        bBoxElement.setAttribute( u"maxx"_s, qgsDoubleToString( QgsServerProjectUtils::ceilWithPrecision( layerExtent.xMaximum(), precision ), precision ) );
        bBoxElement.setAttribute( u"maxy"_s, qgsDoubleToString( QgsServerProjectUtils::ceilWithPrecision( layerExtent.yMaximum(), precision ), precision ) );
        layerElem.appendChild( bBoxElement );

        // layer metadata URL
        const QList<QgsMapLayerServerProperties::MetadataUrl> urls = layer->serverProperties()->metadataUrls();
        for ( const QgsMapLayerServerProperties::MetadataUrl &url : urls )
        {
          QDomElement metaUrlElem = doc.createElement( u"MetadataURL"_s );
          const QString metadataUrlType = url.type;
          metaUrlElem.setAttribute( u"type"_s, metadataUrlType );
          const QString metadataUrlFormat = url.format;
          if ( metadataUrlFormat == "text/xml"_L1 )
          {
            metaUrlElem.setAttribute( u"format"_s, u"XML"_s );
          }
          else
          {
            metaUrlElem.setAttribute( u"format"_s, u"TXT"_s );
          }
          const QDomText metaUrlText = doc.createTextNode( url.url );
          metaUrlElem.appendChild( metaUrlText );
          layerElem.appendChild( metaUrlElem );
        }

        //wfs:Operations element
        QDomElement operationsElement = doc.createElement( u"Operations"_s /*wfs:Operations*/ );
        //wfs:Query element
        const QDomElement queryElement = doc.createElement( u"Query"_s /*wfs:Query*/ );
        operationsElement.appendChild( queryElement );
        if ( wfstUpdateLayersId.contains( layer->id() ) || wfstInsertLayersId.contains( layer->id() ) || wfstDeleteLayersId.contains( layer->id() ) )
        {
          QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
          QgsVectorDataProvider *provider = vlayer->dataProvider();
          if ( ( provider->capabilities() & Qgis::VectorProviderCapability::AddFeatures ) && wfstInsertLayersId.contains( layer->id() ) )
          {
            //wfs:Insert element
            const QDomElement insertElement = doc.createElement( u"Insert"_s /*wfs:Insert*/ );
            operationsElement.appendChild( insertElement );
          }
          if ( ( provider->capabilities() & Qgis::VectorProviderCapability::ChangeAttributeValues ) && ( provider->capabilities() & Qgis::VectorProviderCapability::ChangeGeometries ) && wfstUpdateLayersId.contains( layer->id() ) )
          {
            //wfs:Update element
            const QDomElement updateElement = doc.createElement( u"Update"_s /*wfs:Update*/ );
            operationsElement.appendChild( updateElement );
          }
          if ( ( provider->capabilities() & Qgis::VectorProviderCapability::DeleteFeatures ) && wfstDeleteLayersId.contains( layer->id() ) )
          {
            //wfs:Delete element
            const QDomElement deleteElement = doc.createElement( u"Delete"_s /*wfs:Delete*/ );
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
