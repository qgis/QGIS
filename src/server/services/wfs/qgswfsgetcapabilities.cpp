/***************************************************************************
                              qgswfsgecapabilities.cpp
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
#include "qgswfsgetcapabilities.h"

#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransform.h"
#include "qgsexception.h"
#include "qgsproject.h"
#include "qgsserverprojectutils.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgswfsutils.h"

namespace QgsWfs
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
    wfsCapabilitiesElement.setAttribute( u"xsi:schemaLocation"_s, WFS_NAMESPACE + " http://schemas.opengis.net/wfs/1.1.0/wfs.xsd" );
    wfsCapabilitiesElement.setAttribute( u"xmlns:ogc"_s, OGC_NAMESPACE );
    wfsCapabilitiesElement.setAttribute( u"xmlns:gml"_s, GML_NAMESPACE );
    wfsCapabilitiesElement.setAttribute( u"xmlns:ows"_s, u"http://www.opengis.net/ows"_s );
    wfsCapabilitiesElement.setAttribute( u"xmlns:xlink"_s, u"http://www.w3.org/1999/xlink"_s );
    wfsCapabilitiesElement.setAttribute( u"version"_s, implementationVersion() );
    wfsCapabilitiesElement.setAttribute( u"updateSequence"_s, u"0"_s );
    doc.appendChild( wfsCapabilitiesElement );

    //ows:ServiceIdentification
    wfsCapabilitiesElement.appendChild( getServiceIdentificationElement( doc, project ) );

    //ows:ServiceProvider
    wfsCapabilitiesElement.appendChild( getServiceProviderElement( doc, project ) );

    //wfs:OperationsMetadata
    wfsCapabilitiesElement.appendChild( getOperationsMetadataElement( doc, project, request, serverIface->serverSettings() ) );

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
    //GeometryOperands
    QStringList geometryOperands;
    geometryOperands << u"gml:Point"_s << u"gml:LineString"_s << u"gml:Polygon"_s
                     << u"gml:Envelope"_s;
    QDomElement geometryOperandsElem = doc.createElement( u"ogc:GeometryOperands"_s );
    for ( const QString &geometryOperand : geometryOperands )
    {
      QDomElement geometryOperandElem = doc.createElement( u"ogc:GeometryOperand"_s );
      const QDomText geometryOperandText = doc.createTextNode( geometryOperand );
      geometryOperandElem.appendChild( geometryOperandText );
      geometryOperandsElem.appendChild( geometryOperandElem );
    }
    spatialCapabilitiesElement.appendChild( geometryOperandsElem );
    //SpatialOperators
    QStringList spatialOperators;
    spatialOperators << u"Equals"_s << u"Disjoint"_s << u"Touches"_s
                     << u"Within"_s << u"Overlaps"_s << u"Crosses"_s
                     << u"Intersects"_s << u"Contains"_s << u"BBOX"_s;
    QDomElement spatialOperatorsElem = doc.createElement( u"ogc:SpatialOperators"_s );
    for ( const QString &spatialOperator : spatialOperators )
    {
      QDomElement spatialOperatorElem = doc.createElement( u"ogc:SpatialOperator"_s );
      spatialOperatorElem.setAttribute( u"name"_s, spatialOperator );
      spatialOperatorsElem.appendChild( spatialOperatorElem );
    }
    spatialCapabilitiesElement.appendChild( spatialOperatorsElem );
    QDomElement scalarCapabilitiesElement = doc.createElement( u"ogc:Scalar_Capabilities"_s /*ogc:Scalar_Capabilities*/ );
    filterCapabilitiesElement.appendChild( scalarCapabilitiesElement );
    const QDomElement logicalOperatorsElement = doc.createElement( u"ogc:LogicalOperators"_s );
    scalarCapabilitiesElement.appendChild( logicalOperatorsElement );
    // ComparisonOperators
    QStringList comparisonOperators;
    comparisonOperators << u"LessThan"_s << u"GreaterThan"_s
                        << u"LessThanEqualTo"_s << u"GreaterThanEqualTo"_s
                        << u"EqualTo"_s << u"Like"_s << u"Between"_s;
    QDomElement comparisonOperatorsElem = doc.createElement( u"ogc:ComparisonOperators"_s );
    for ( const QString &comparisonOperator : comparisonOperators )
    {
      QDomElement comparisonOperatorElem = doc.createElement( u"ogc:ComparisonOperator"_s );
      const QDomText comparisonOperatorText = doc.createTextNode( comparisonOperator );
      comparisonOperatorElem.appendChild( comparisonOperatorText );
      comparisonOperatorsElem.appendChild( comparisonOperatorElem );
    }
    scalarCapabilitiesElement.appendChild( comparisonOperatorsElem );

    QDomElement idCapabilitiesElement = doc.createElement( u"ogc:Id_Capabilities"_s );
    const QDomElement fidElem = doc.createElement( u"ogc:FID"_s );
    idCapabilitiesElement.appendChild( fidElem );
    filterCapabilitiesElement.appendChild( idCapabilitiesElement );

    return doc;
  }

  QDomElement getServiceIdentificationElement( QDomDocument &doc, const QgsProject *project )
  {
    //Service element
    QDomElement serviceElem = doc.createElement( u"ows:ServiceIdentification"_s );

    const QString title = QgsServerProjectUtils::owsServiceTitle( *project );
    QDomElement titleElem = doc.createElement( u"ows:Title"_s );
    const QDomText titleText = doc.createTextNode( title );
    titleElem.appendChild( titleText );
    serviceElem.appendChild( titleElem );

    const QString abstract = QgsServerProjectUtils::owsServiceAbstract( *project );
    if ( !abstract.isEmpty() )
    {
      QDomElement abstractElem = doc.createElement( u"ows:Abstract"_s );
      const QDomText abstractText = doc.createCDATASection( abstract );
      abstractElem.appendChild( abstractText );
      serviceElem.appendChild( abstractElem );
    }

    const QStringList keywords = QgsServerProjectUtils::owsServiceKeywords( *project );
    if ( !keywords.isEmpty() && !keywords.join( ", "_L1 ).isEmpty() )
    {
      QDomElement keywordsElem = doc.createElement( u"ows:Keywords"_s );
      for ( const QString &keyword : keywords )
      {
        if ( !keyword.isEmpty() )
        {
          QDomElement keywordElem = doc.createElement( u"ows:Keyword"_s );
          const QDomText keywordText = doc.createTextNode( keyword );
          keywordElem.appendChild( keywordText );
          keywordsElem.appendChild( keywordElem );
        }
      }
      serviceElem.appendChild( keywordsElem );
    }

    //Service type
    QDomElement serviceTypeElem = doc.createElement( u"ows:ServiceType"_s );
    const QDomText serviceTypeText = doc.createTextNode( "WFS" );
    serviceTypeElem.appendChild( serviceTypeText );
    serviceElem.appendChild( serviceTypeElem );

    //Service type version
    QDomElement serviceTypeVersionElem = doc.createElement( u"ows:ServiceTypeVersion"_s );
    const QDomText serviceTypeVersionText = doc.createTextNode( "1.1.0" );
    serviceTypeVersionElem.appendChild( serviceTypeVersionText );
    serviceElem.appendChild( serviceTypeVersionElem );

    QDomElement feesElem = doc.createElement( u"ows:Fees"_s );
    QDomText feesText = doc.createTextNode( "None" );
    const QString fees = QgsServerProjectUtils::owsServiceFees( *project );
    if ( !fees.isEmpty() )
    {
      feesText = doc.createTextNode( fees );
    }
    feesElem.appendChild( feesText );
    serviceElem.appendChild( feesElem );

    QDomElement accessConstraintsElem = doc.createElement( u"ows:AccessConstraints"_s );
    const QString accessConstraints = QgsServerProjectUtils::owsServiceAccessConstraints( *project );
    QDomText accessConstraintsText = doc.createTextNode( "None" );
    if ( !accessConstraints.isEmpty() )
    {
      accessConstraintsText = doc.createTextNode( accessConstraints );
    }
    accessConstraintsElem.appendChild( accessConstraintsText );
    serviceElem.appendChild( accessConstraintsElem );

    return serviceElem;
  }

  QDomElement getServiceProviderElement( QDomDocument &doc, const QgsProject *project )
  {
    //Service element
    QDomElement serviceElem = doc.createElement( u"ows:ServiceProvider"_s );

    //ProviderName
    const QString contactOrganization = QgsServerProjectUtils::owsServiceContactOrganization( *project );
    if ( !contactOrganization.isEmpty() )
    {
      QDomElement providerNameElem = doc.createElement( u"ows:ProviderName"_s );
      const QDomText providerNameText = doc.createTextNode( contactOrganization );
      providerNameElem.appendChild( providerNameText );
      serviceElem.appendChild( providerNameElem );
    }

    const QString contactPerson = QgsServerProjectUtils::owsServiceContactPerson( *project );
    const QString contactPosition = QgsServerProjectUtils::owsServiceContactPosition( *project );
    if ( !contactPerson.isEmpty() || !contactPosition.isEmpty() )
    {
      //Contact information
      QDomElement serviceContactElem = doc.createElement( u"ows:ServiceContact"_s );

      if ( !contactPerson.isEmpty() )
      {
        QDomElement individualNameElem = doc.createElement( u"ows:IndividualName"_s );
        const QDomText individualNameText = doc.createTextNode( contactPerson );
        individualNameElem.appendChild( individualNameText );
        serviceContactElem.appendChild( individualNameElem );
      }

      if ( !contactPosition.isEmpty() )
      {
        QDomElement positionNameElem = doc.createElement( u"ows:PositionName"_s );
        const QDomText positionNameText = doc.createTextNode( contactPosition );
        positionNameElem.appendChild( positionNameText );
        serviceContactElem.appendChild( positionNameElem );
      }

      const QString contactMail = QgsServerProjectUtils::owsServiceContactMail( *project );
      const QString contactPhone = QgsServerProjectUtils::owsServiceContactPhone( *project );
      const QString onlineResource = QgsServerProjectUtils::owsServiceOnlineResource( *project );
      if ( !contactMail.isEmpty() || !contactPhone.isEmpty() || !onlineResource.isEmpty() )
      {
        //Contact information
        QDomElement contactInfoElem = doc.createElement( u"ows:ContactInfo"_s );

        if ( !contactPhone.isEmpty() )
        {
          QDomElement phoneElem = doc.createElement( u"ows:Phone"_s );
          QDomElement voiceElem = doc.createElement( u"ows:Voice"_s );
          const QDomText voiceText = doc.createTextNode( contactPhone );
          voiceElem.appendChild( voiceText );
          phoneElem.appendChild( voiceElem );
          contactInfoElem.appendChild( phoneElem );
        }

        if ( !contactMail.isEmpty() )
        {
          QDomElement addressElem = doc.createElement( u"ows:Address"_s );
          QDomElement mailElem = doc.createElement( u"ows:ElectronicMailAddress"_s );
          const QDomText mailText = doc.createTextNode( contactMail );
          mailElem.appendChild( mailText );
          addressElem.appendChild( mailElem );
          contactInfoElem.appendChild( addressElem );
        }

        if ( !onlineResource.isEmpty() )
        {
          QDomElement onlineResourceElem = doc.createElement( u"ows:OnlineResource"_s );
          onlineResourceElem.setAttribute( "xlink:href", onlineResource );
          contactInfoElem.appendChild( onlineResourceElem );
        }
      }

      QDomElement roleElem = doc.createElement( u"ows:Role"_s );
      const QDomText roleText = doc.createTextNode( "PointOfContact" );
      roleElem.appendChild( roleText );
      serviceContactElem.appendChild( roleElem );

      serviceElem.appendChild( serviceContactElem );
    }

    return serviceElem;
  }

  QDomElement getParameterElement( QDomDocument &doc, const QString &name, const QStringList &values )
  {
    QDomElement parameterElement = doc.createElement( u"ows:Parameter"_s );
    parameterElement.setAttribute( u"name"_s, name );

    for ( const QString &v : values )
    {
      QDomElement valueElement = doc.createElement( u"ows:Value"_s );
      const QDomText valueText = doc.createTextNode( v );
      valueElement.appendChild( valueText );
      parameterElement.appendChild( valueElement );
    }

    return parameterElement;
  }

  QDomElement getOperationsMetadataElement( QDomDocument &doc, const QgsProject *project, const QgsServerRequest &request, const QgsServerSettings *settings )
  {
    QDomElement oprationsElement = doc.createElement( u"ows:OperationsMetadata"_s );

    // Prepare url
    const QString hrefString = serviceUrl( request, project, *settings );

    QDomElement operationElement = doc.createElement( u"ows:Operation"_s );
    QDomElement dcpElement = doc.createElement( u"ows:DCP"_s );
    QDomElement httpElement = doc.createElement( u"ows:HTTP"_s );
    QDomElement getElement = doc.createElement( u"ows:Get"_s );
    getElement.setAttribute( u"xlink:href"_s, hrefString );
    httpElement.appendChild( getElement );

    QDomElement postElement = doc.createElement( u"ows:Post"_s );
    postElement.setAttribute( u"xlink:href"_s, hrefString );
    httpElement.appendChild( postElement );

    dcpElement.appendChild( httpElement );
    operationElement.appendChild( dcpElement );

    // GetCapabilities
    QDomElement getCapabilitiesElement = operationElement.cloneNode().toElement();
    getCapabilitiesElement.setAttribute( u"name"_s, u"GetCapabilities"_s );
    // GetCapabilities service
    const QDomElement serviceParameterElement = getParameterElement( doc, u"service"_s, QStringList() << u"WFS"_s );
    getCapabilitiesElement.appendChild( serviceParameterElement );
    // GetCapabilities AcceptVersions
    const QDomElement acceptVersionsParameterElement = getParameterElement( doc, u"AcceptVersions"_s, QStringList() << u"1.1.0"_s << u"1.0.0"_s );
    getCapabilitiesElement.appendChild( acceptVersionsParameterElement );
    // GetCapabilities AcceptFormats
    const QDomElement acceptFormatsParameterElement = getParameterElement( doc, u"AcceptFormats"_s, QStringList() << u"text/xml"_s );
    getCapabilitiesElement.appendChild( acceptFormatsParameterElement );
    // Add
    oprationsElement.appendChild( getCapabilitiesElement );

    // DescribeFeatureType
    QDomElement describeFeatureTypeElement = operationElement.cloneNode().toElement();
    describeFeatureTypeElement.setAttribute( u"name"_s, u"DescribeFeatureType"_s );
    // DescribeFeatureType outputFormat
    const QDomElement dftOutputFormatParameterElement = getParameterElement( doc, u"outputFormat"_s, QStringList() << u"XMLSCHEMA"_s << u"text/xml; subtype=gml/2.1.2"_s << u"text/xml; subtype=gml/3.1.1"_s );
    describeFeatureTypeElement.appendChild( dftOutputFormatParameterElement );
    // Add
    oprationsElement.appendChild( describeFeatureTypeElement );

    // GetFeature
    QDomElement getFeatureElement = operationElement.cloneNode().toElement();
    getFeatureElement.setAttribute( u"name"_s, u"GetFeature"_s );
    // GetFeature outputFormat
    const QDomElement gfOutputFormatParameterElement = getParameterElement( doc, u"outputFormat"_s, QStringList() << u"text/xml; subtype=gml/2.1.2"_s << u"text/xml; subtype=gml/3.1.1"_s << u"application/vnd.geo+json"_s );
    getFeatureElement.appendChild( gfOutputFormatParameterElement );
    // GetFeature resultType
    const QDomElement resultTypeParameterElement = getParameterElement( doc, u"resultType"_s, QStringList() << u"results"_s << u"hits"_s );
    getFeatureElement.appendChild( resultTypeParameterElement );
    // Add
    oprationsElement.appendChild( getFeatureElement );

    // Transaction
    QDomElement transactionElement = operationElement.cloneNode().toElement();
    transactionElement.setAttribute( u"name"_s, u"Transaction"_s );
    // GetFeature inputFormat
    const QDomElement inputFormatParameterElement = getParameterElement( doc, u"inputFormat"_s, QStringList() << u"text/xml; subtype=gml/2.1.2"_s << u"text/xml; subtype=gml/3.1.1"_s << u"application/vnd.geo+json"_s );
    transactionElement.appendChild( inputFormatParameterElement );
    // Add
    oprationsElement.appendChild( transactionElement );

    return oprationsElement;
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
    QDomElement operationElement = doc.createElement( u"Operation"_s );
    const QDomText queryText = doc.createTextNode( "Query" );
    operationElement.appendChild( queryText );
    operationsElement.appendChild( operationElement );

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
      const QDomText nameText = doc.createTextNode( layer->serverProperties()->wfsTypeName() );
      nameElem.appendChild( nameText );
      layerElem.appendChild( nameElem );

      //create Title
      QDomElement titleElem = doc.createElement( u"Title"_s );
      QString title = layer->serverProperties()->wfsTitle();
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
        QDomElement keywordsElem = doc.createElement( u"ows:Keywords"_s );
        for ( const QString &keyword : keywords.split( ',' ) )
        {
          if ( !keyword.trimmed().isEmpty() )
          {
            QDomElement keywordElem = doc.createElement( u"ows:Keyword"_s );
            const QDomText keywordText = doc.createTextNode( keyword.trimmed() );
            keywordElem.appendChild( keywordText );
            keywordsElem.appendChild( keywordElem );
          }
        }
        layerElem.appendChild( keywordsElem );
      }

      //create DefaultSRS element
      const QString defaultSrs = layer->crs().toOgcUrn();
      QDomElement srsElem = doc.createElement( u"DefaultSRS"_s );
      const QDomText srsText = doc.createTextNode( defaultSrs );
      srsElem.appendChild( srsText );
      layerElem.appendChild( srsElem );

      //create OtherSRS elements
      const QStringList outputCrsList = QgsServerProjectUtils::wmsOutputCrsListAsOgcUrn( *project );
      for ( const QString &crs : outputCrsList )
      {
        if ( crs == defaultSrs )
          continue;
        QDomElement otherSrsElem = doc.createElement( u"OtherSRS"_s );
        const QDomText otherSrsText = doc.createTextNode( crs );
        otherSrsElem.appendChild( otherSrsText );
        layerElem.appendChild( otherSrsElem );
      }

      //wfs:Operations element
      QDomElement operationsElement = doc.createElement( u"Operations"_s /*wfs:Operations*/ );
      //wfs:Query element
      QDomElement operationElement = doc.createElement( u"Operation"_s );
      const QDomText queryText = doc.createTextNode( u"Query"_s );
      operationElement.appendChild( queryText );
      operationsElement.appendChild( operationElement );

      if ( wfstUpdateLayersId.contains( layer->id() ) || wfstInsertLayersId.contains( layer->id() ) || wfstDeleteLayersId.contains( layer->id() ) )
      {
        QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( layer );
        QgsVectorDataProvider *provider = vlayer->dataProvider();
        if ( ( provider->capabilities() & Qgis::VectorProviderCapability::AddFeatures ) && wfstInsertLayersId.contains( layer->id() ) )
        {
          //wfs:Insert element
          QDomElement operationElement = doc.createElement( u"Operation"_s );
          const QDomText insertText = doc.createTextNode( u"Insert"_s /*wfs:Insert*/ );
          operationElement.appendChild( insertText );
          operationsElement.appendChild( operationElement );
        }

        if ( ( provider->capabilities() & Qgis::VectorProviderCapability::ChangeAttributeValues ) && ( !layer->isSpatial() || provider->capabilities() & Qgis::VectorProviderCapability::ChangeGeometries ) && wfstUpdateLayersId.contains( layer->id() ) )
        {
          //wfs:Update element
          QDomElement operationElement = doc.createElement( u"Operation"_s );
          const QDomText updateText = doc.createTextNode( u"Update"_s /*wfs:Update*/ );
          operationElement.appendChild( updateText );
          operationsElement.appendChild( operationElement );
        }

        if ( ( provider->capabilities() & Qgis::VectorProviderCapability::DeleteFeatures ) && wfstDeleteLayersId.contains( layer->id() ) )
        {
          //wfs:Delete element
          QDomElement operationElement = doc.createElement( u"Operation"_s );
          const QDomText deleteText = doc.createTextNode( u"Delete"_s /*wfs:Delete*/ );
          operationElement.appendChild( deleteText );
          operationsElement.appendChild( operationElement );
        }
      }

      layerElem.appendChild( operationsElement );

      //create WGS84BoundingBox
      const QgsRectangle layerExtent = layer->extent();
      //transform the layers native CRS into WGS84
      const QgsCoordinateReferenceSystem wgs84 = QgsCoordinateReferenceSystem::fromOgcWmsCrs( Qgis::geographicCrsAuthId() );
      const int wgs84precision = 6;
      QgsRectangle wgs84BoundingRect( 0, 0, 0, 0 );
      if ( !layerExtent.isNull() )
      {
        const QgsCoordinateTransform exGeoTransform( layer->crs(), wgs84, project );
        try
        {
          wgs84BoundingRect = exGeoTransform.transformBoundingBox( layerExtent );
        }
        catch ( const QgsCsException & )
        {
          wgs84BoundingRect = QgsRectangle( 0, 0, 0, 0 );
        }
      }

      //create WGS84BoundingBox element
      QDomElement bBoxElement = doc.createElement( u"ows:WGS84BoundingBox"_s );
      bBoxElement.setAttribute( u"dimensions"_s, u"2"_s );
      QDomElement lCornerElement = doc.createElement( u"ows:LowerCorner"_s );
      const QDomText lCornerText = doc.createTextNode( qgsDoubleToString( QgsServerProjectUtils::floorWithPrecision( wgs84BoundingRect.xMinimum(), wgs84precision ), wgs84precision ) + " " + qgsDoubleToString( QgsServerProjectUtils::floorWithPrecision( wgs84BoundingRect.yMinimum(), wgs84precision ), wgs84precision ) );
      lCornerElement.appendChild( lCornerText );
      bBoxElement.appendChild( lCornerElement );
      QDomElement uCornerElement = doc.createElement( u"ows:UpperCorner"_s );
      const QDomText uCornerText = doc.createTextNode( qgsDoubleToString( QgsServerProjectUtils::ceilWithPrecision( wgs84BoundingRect.xMaximum(), wgs84precision ), wgs84precision ) + " " + qgsDoubleToString( QgsServerProjectUtils::ceilWithPrecision( wgs84BoundingRect.yMaximum(), wgs84precision ), wgs84precision ) );
      uCornerElement.appendChild( uCornerText );
      bBoxElement.appendChild( uCornerElement );
      layerElem.appendChild( bBoxElement );

      // layer metadata URL
      const QList<QgsMapLayerServerProperties::MetadataUrl> urls = layer->serverProperties()->metadataUrls();
      for ( const QgsMapLayerServerProperties::MetadataUrl &url : urls )
      {
        QDomElement metaUrlElem = doc.createElement( u"MetadataURL"_s );
        const QString metadataUrlType = url.type;
        metaUrlElem.setAttribute( u"type"_s, metadataUrlType );
        const QString metadataUrlFormat = url.format;
        metaUrlElem.setAttribute( u"format"_s, metadataUrlFormat );
        const QDomText metaUrlText = doc.createTextNode( url.url );
        metaUrlElem.appendChild( metaUrlText );
        layerElem.appendChild( metaUrlElem );
      }

      featureTypeListElement.appendChild( layerElem );
    }

    return featureTypeListElement;
  }

} // namespace QgsWfs
