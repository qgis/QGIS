/***************************************************************************
                              qgswmtsgecapabilities.cpp
                              -------------------------
  begin                : July 23 , 2017
  copyright            : (C) 2018 by René-Luc D'Hont
  email                : rldhont at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgswmtsgetcapabilities.h"

#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransform.h"
#include "qgsexception.h"
#include "qgsproject.h"
#include "qgsserverprojectutils.h"
#include "qgswmtsutils.h"

namespace QgsWmts
{
  namespace
  {
    void appendLayerElements( QDomDocument &doc, QDomElement &contentsElement, QList<layerDef> wmtsLayers, QList<tileMatrixSetDef> tmsList, const QgsProject *project );

    void appendTileMatrixSetElements( QDomDocument &doc, QDomElement &contentsElement, QList<tileMatrixSetDef> tmsList );
  } // namespace

  /**
   * Output WMTS  GetCapabilities response
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

    //wmts:Capabilities element
    QDomElement wmtsCapabilitiesElement = doc.createElement( u"Capabilities"_s /*wmts:Capabilities*/ );
    wmtsCapabilitiesElement.setAttribute( u"xmlns"_s, WMTS_NAMESPACE );
    wmtsCapabilitiesElement.setAttribute( u"xmlns:gml"_s, GML_NAMESPACE );
    wmtsCapabilitiesElement.setAttribute( u"xmlns:ows"_s, OWS_NAMESPACE );
    wmtsCapabilitiesElement.setAttribute( u"xmlns:xlink"_s, u"http://www.w3.org/1999/xlink"_s );
    wmtsCapabilitiesElement.setAttribute( u"xmlns:xsi"_s, u"http://www.w3.org/2001/XMLSchema-instance"_s );
    wmtsCapabilitiesElement.setAttribute( u"xsi:schemaLocation"_s, WMTS_NAMESPACE + " http://schemas.opengis.net/wmts/1.0/wmtsGetCapabilities_response.xsd" );
    wmtsCapabilitiesElement.setAttribute( u"version"_s, implementationVersion() );
    doc.appendChild( wmtsCapabilitiesElement );

    //INSERT ServiceIdentification
    wmtsCapabilitiesElement.appendChild( getServiceIdentificationElement( doc, project ) );

    //INSERT ServiceProvider
    wmtsCapabilitiesElement.appendChild( getServiceProviderElement( doc, project ) );

    //INSERT OperationsMetadata
    wmtsCapabilitiesElement.appendChild( getOperationsMetadataElement( doc, project, request, serverIface->serverSettings() ) );

    //INSERT Contents
    wmtsCapabilitiesElement.appendChild( getContentsElement( doc, serverIface, project ) );

    return doc;
  }

  QDomElement getServiceIdentificationElement( QDomDocument &doc, const QgsProject *project )
  {
    //Service identification
    QDomElement serviceElem = doc.createElement( u"ows:ServiceIdentification"_s );

    //Service type
    QDomElement typeElem = doc.createElement( u"ows:ServiceType"_s );
    const QDomText typeText = doc.createTextNode( u"OGC WMTS"_s );
    typeElem.appendChild( typeText );
    serviceElem.appendChild( typeElem );

    //Service type version
    QDomElement typeVersionElem = doc.createElement( u"ows:ServiceTypeVersion"_s );
    const QDomText typeVersionText = doc.createTextNode( implementationVersion() );
    typeVersionElem.appendChild( typeVersionText );
    serviceElem.appendChild( typeVersionElem );

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
    if ( !keywords.isEmpty() )
    {
      QDomElement keywordsElem = doc.createElement( u"ows:Keywords"_s );
      for ( const QString &k : keywords )
      {
        QDomElement keywordElem = doc.createElement( u"ows:Keyword"_s );
        const QDomText keywordText = doc.createTextNode( k );
        keywordElem.appendChild( keywordText );
        keywordsElem.appendChild( keywordElem );
      }
      serviceElem.appendChild( keywordsElem );
    }

    QDomElement feesElem = doc.createElement( u"ows:Fees"_s );
    QDomText feesText = doc.createTextNode( u"None"_s ); // default value if fees are unknown
    const QString fees = QgsServerProjectUtils::owsServiceFees( *project );
    if ( !fees.isEmpty() )
    {
      feesText = doc.createTextNode( fees );
    }
    feesElem.appendChild( feesText );
    serviceElem.appendChild( feesElem );

    QDomElement accessConstraintsElem = doc.createElement( u"ows:AccessConstraints"_s );
    QDomText accessConstraintsText = doc.createTextNode( u"None"_s ); // default value if access constraints are unknown
    const QString accessConstraints = QgsServerProjectUtils::owsServiceAccessConstraints( *project );
    if ( !accessConstraints.isEmpty() )
    {
      accessConstraintsText = doc.createTextNode( accessConstraints );
    }
    accessConstraintsElem.appendChild( accessConstraintsText );
    serviceElem.appendChild( accessConstraintsElem );

    //End
    return serviceElem;
  }

  QDomElement getServiceProviderElement( QDomDocument &doc, const QgsProject *project )
  {
    //Service provider
    QDomElement serviceElem = doc.createElement( u"ows:ServiceProvider"_s );

    const QString contactOrganization = QgsServerProjectUtils::owsServiceContactOrganization( *project );
    if ( !contactOrganization.isEmpty() )
    {
      QDomElement contactOrganizationElem = doc.createElement( u"ows:ProviderName"_s );
      const QDomText contactOrganizationText = doc.createTextNode( contactOrganization );
      contactOrganizationElem.appendChild( contactOrganizationText );
      serviceElem.appendChild( contactOrganizationElem );
    }

    const QString onlineResource = QgsServerProjectUtils::owsServiceOnlineResource( *project );
    if ( !onlineResource.isEmpty() )
    {
      QDomElement onlineResourceElem = doc.createElement( u"ows:ProviderSite"_s );
      onlineResourceElem.setAttribute( u"xlink:href"_s, onlineResource );
      serviceElem.appendChild( onlineResourceElem );
    }

    //Contact information
    const QString contactPerson = QgsServerProjectUtils::owsServiceContactPerson( *project );
    const QString contactPosition = QgsServerProjectUtils::owsServiceContactPosition( *project );
    const QString contactMail = QgsServerProjectUtils::owsServiceContactMail( *project );
    const QString contactPhone = QgsServerProjectUtils::owsServiceContactPhone( *project );
    if ( !contactPerson.isEmpty() || !contactPosition.isEmpty() || !contactMail.isEmpty() || !contactPhone.isEmpty() )
    {
      QDomElement serviceContactElem = doc.createElement( u"ows:ServiceContact"_s );
      if ( !contactPerson.isEmpty() )
      {
        QDomElement contactPersonElem = doc.createElement( u"ows:IndividualName"_s );
        const QDomText contactPersonText = doc.createTextNode( contactPerson );
        contactPersonElem.appendChild( contactPersonText );
        serviceContactElem.appendChild( contactPersonElem );
      }
      if ( !contactPosition.isEmpty() )
      {
        QDomElement contactPositionElem = doc.createElement( u"ows:PositionName"_s );
        const QDomText contactPositionText = doc.createTextNode( contactPosition );
        contactPositionElem.appendChild( contactPositionText );
        serviceContactElem.appendChild( contactPositionElem );
      }
      if ( !contactMail.isEmpty() || !contactPhone.isEmpty() )
      {
        QDomElement contactInfoElem = doc.createElement( u"ows:ContactInfo"_s );
        if ( !contactMail.isEmpty() )
        {
          QDomElement contactAddressElem = doc.createElement( u"ows:Address"_s );
          QDomElement contactAddressMailElem = doc.createElement( u"ows:ElectronicMailAddress"_s );
          const QDomText contactAddressMailText = doc.createTextNode( contactMail );
          contactAddressMailElem.appendChild( contactAddressMailText );
          contactAddressElem.appendChild( contactAddressMailElem );
          contactInfoElem.appendChild( contactAddressElem );
        }
        if ( !contactPhone.isEmpty() )
        {
          QDomElement contactPhoneElem = doc.createElement( u"ows:Phone"_s );
          QDomElement contactVoiceElem = doc.createElement( u"ows:Voice"_s );
          const QDomText contactVoiceText = doc.createTextNode( contactPhone );
          contactVoiceElem.appendChild( contactVoiceText );
          contactPhoneElem.appendChild( contactVoiceElem );
          contactInfoElem.appendChild( contactPhoneElem );
        }
        serviceContactElem.appendChild( contactInfoElem );
      }
      serviceElem.appendChild( serviceContactElem );
    }

    //End
    return serviceElem;
  }

  QDomElement getOperationsMetadataElement( QDomDocument &doc, const QgsProject *project, const QgsServerRequest &request, const QgsServerSettings *settings )
  {
    //ows:OperationsMetadata element
    QDomElement operationsMetadataElement = doc.createElement( u"ows:OperationsMetadata"_s /*ows:OperationsMetadata*/ );

    //ows:Operation element with name GetCapabilities
    QDomElement getCapabilitiesElement = doc.createElement( u"ows:Operation"_s /*ows:Operation*/ );
    getCapabilitiesElement.setAttribute( u"name"_s, u"GetCapabilities"_s );
    operationsMetadataElement.appendChild( getCapabilitiesElement );

    //ows:DCP
    QDomElement dcpElement = doc.createElement( u"ows:DCP"_s /*ows:DCP*/ );
    getCapabilitiesElement.appendChild( dcpElement );
    QDomElement httpElement = doc.createElement( u"ows:HTTP"_s /*ows:HTTP*/ );
    dcpElement.appendChild( httpElement );

    // Get service URL
    const QUrl href = serviceUrl( request, project, *settings );

    //href needs to be a prefix
    QString hrefString = href.toString();
    hrefString.append( href.hasQuery() ? '&' : '?' );

    //ows:Get
    QDomElement getElement = doc.createElement( u"ows:Get"_s /*ows:Get*/ );
    getElement.setAttribute( u"xlink:href"_s, hrefString );
    QDomElement constraintElement = doc.createElement( u"ows:Constraint"_s /*ows:Constraint*/ );
    constraintElement.setAttribute( u"name"_s, u"GetEncoding"_s );
    QDomElement allowedValuesElement = doc.createElement( u"ows:AllowedValues"_s /*ows:AllowedValues*/ );
    QDomElement valueElement = doc.createElement( u"ows:Value"_s /*ows:Value*/ );
    const QDomText valueText = doc.createTextNode( u"KVP"_s );
    valueElement.appendChild( valueText );
    allowedValuesElement.appendChild( valueElement );
    constraintElement.appendChild( allowedValuesElement );
    getElement.appendChild( constraintElement );
    httpElement.appendChild( getElement );

    //ows:Operation element with name GetTile
    QDomElement getTileElement = getCapabilitiesElement.cloneNode().toElement(); //this is the same as 'GetCapabilities'
    getTileElement.setAttribute( u"name"_s, u"GetTile"_s );
    operationsMetadataElement.appendChild( getTileElement );

    //ows:Operation element with name GetFeatureInfo
    QDomElement getFeatureInfoElement = getCapabilitiesElement.cloneNode().toElement(); //this is the same as 'GetCapabilities'
    getFeatureInfoElement.setAttribute( u"name"_s, u"GetFeatureInfo"_s );
    operationsMetadataElement.appendChild( getFeatureInfoElement );

    // End
    return operationsMetadataElement;
  }

  QDomElement getContentsElement( QDomDocument &doc, QgsServerInterface *serverIface, const QgsProject *project )
  {
    /*
     * Adding layer list in ContentMetadata
     */
    QDomElement contentsElement = doc.createElement( u"Contents"_s /*wmts:Contents*/ );

    const QList<tileMatrixSetDef> tmsList = getTileMatrixSetList( project );
    if ( !tmsList.isEmpty() )
    {
      // get layer list
      const QList<layerDef> wmtsLayers = getWmtsLayerList( serverIface, project );
      if ( !wmtsLayers.isEmpty() )
      {
        appendLayerElements( doc, contentsElement, wmtsLayers, tmsList, project );
      }

      appendTileMatrixSetElements( doc, contentsElement, tmsList );
    }

    //End
    return contentsElement;
  }
  namespace
  {
    void appendLayerElements( QDomDocument &doc, QDomElement &contentsElement, QList<layerDef> wmtsLayers, QList<tileMatrixSetDef> tmsList, const QgsProject *project )
    {
      const QgsCoordinateReferenceSystem wgs84 = QgsCoordinateReferenceSystem::fromOgcWmsCrs( Qgis::geographicCrsAuthId() );
      // Define InfoFormat helper
      const std::function<void( QDomElement &, const QString & )> appendInfoFormat = [&doc]( QDomElement &elem, const QString &format ) {
        QDomElement formatElem = doc.createElement( u"InfoFormat"_s /*wmts:InfoFormat*/ );
        formatElem.appendChild( doc.createTextNode( format ) );
        elem.appendChild( formatElem );
      };

      for ( const layerDef &wmtsLayer : wmtsLayers )
      {
        if ( wmtsLayer.id.isEmpty() )
          continue;

        QDomElement layerElem = doc.createElement( u"Layer"_s );

        QDomElement layerIdElem = doc.createElement( u"ows:Identifier"_s );
        const QDomText layerIdText = doc.createTextNode( wmtsLayer.id );
        layerIdElem.appendChild( layerIdText );
        layerElem.appendChild( layerIdElem );

        if ( !wmtsLayer.title.isEmpty() )
        {
          // Layer title
          QDomElement layerTitleElem = doc.createElement( u"ows:Title"_s );
          const QDomText layerTitleText = doc.createTextNode( wmtsLayer.title );
          layerTitleElem.appendChild( layerTitleText );
          layerElem.appendChild( layerTitleElem );
        }

        if ( !wmtsLayer.abstract.isEmpty() )
        {
          // Layer abstract
          QDomElement layerAbstElem = doc.createElement( u"ows:Abstract"_s );
          const QDomText layerAbstText = doc.createTextNode( project->title() );
          layerAbstElem.appendChild( layerAbstText );
          layerElem.appendChild( layerAbstElem );
        }

        // WGS84 bounding box
        const int wgs84precision = 6;
        QDomElement wgs84BBoxElement = doc.createElement( u"ows:WGS84BoundingBox"_s );
        QDomElement wgs84LowerCornerElement = doc.createElement( u"ows:LowerCorner"_s );
        const QDomText wgs84LowerCornerText = doc.createTextNode( qgsDoubleToString( QgsServerProjectUtils::floorWithPrecision( wmtsLayer.wgs84BoundingRect.xMinimum(), wgs84precision ), wgs84precision ) + ' ' + qgsDoubleToString( QgsServerProjectUtils::floorWithPrecision( wmtsLayer.wgs84BoundingRect.yMinimum(), wgs84precision ), wgs84precision ) );
        wgs84LowerCornerElement.appendChild( wgs84LowerCornerText );
        wgs84BBoxElement.appendChild( wgs84LowerCornerElement );
        QDomElement wgs84UpperCornerElement = doc.createElement( u"ows:UpperCorner"_s );
        const QDomText wgs84UpperCornerText = doc.createTextNode( qgsDoubleToString( QgsServerProjectUtils::ceilWithPrecision( wmtsLayer.wgs84BoundingRect.xMaximum(), wgs84precision ), wgs84precision ) + ' ' + qgsDoubleToString( QgsServerProjectUtils::ceilWithPrecision( wmtsLayer.wgs84BoundingRect.yMaximum(), wgs84precision ), wgs84precision ) );
        wgs84UpperCornerElement.appendChild( wgs84UpperCornerText );
        wgs84BBoxElement.appendChild( wgs84UpperCornerElement );
        layerElem.appendChild( wgs84BBoxElement );

        // Other bounding boxes
        for ( const tileMatrixSetDef &tms : tmsList )
        {
          if ( tms.ref == "EPSG:4326"_L1 )
            continue;

          QgsRectangle rect;
          const QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( tms.ref );
          const QgsCoordinateTransform exGeoTransform( wgs84, crs, project );
          try
          {
            rect = exGeoTransform.transformBoundingBox( wmtsLayer.wgs84BoundingRect );
          }
          catch ( const QgsCsException & )
          {
            continue;
          }

          int precision = 3;
          if ( crs.isGeographic() )
          {
            precision = 6;
          }

          QDomElement bboxElement = doc.createElement( u"ows:BoundingBox"_s );
          bboxElement.setAttribute( u"crs"_s, tms.ref );

          // lower corner
          double firstCoord = rect.xMinimum();
          double secondCoord = rect.yMinimum();

          if ( crs.hasAxisInverted() )
          {
            std::swap( firstCoord, secondCoord );
          }

          QString firstCoordStr = qgsDoubleToString( QgsServerProjectUtils::floorWithPrecision( firstCoord, precision ), precision );
          QString secondCoordStr = qgsDoubleToString( QgsServerProjectUtils::floorWithPrecision( secondCoord, precision ), precision );
          const QDomText lowerCornerText = doc.createTextNode( QString( "%1 %2" ).arg( firstCoordStr, secondCoordStr ) );

          QDomElement lowerCornerElement = doc.createElement( u"ows:LowerCorner"_s );
          lowerCornerElement.appendChild( lowerCornerText );
          bboxElement.appendChild( lowerCornerElement );

          // upper corner
          firstCoord = rect.xMaximum();
          secondCoord = rect.yMaximum();

          if ( crs.hasAxisInverted() )
          {
            std::swap( firstCoord, secondCoord );
          }

          firstCoordStr = qgsDoubleToString( QgsServerProjectUtils::ceilWithPrecision( firstCoord, precision ), precision );
          secondCoordStr = qgsDoubleToString( QgsServerProjectUtils::ceilWithPrecision( secondCoord, precision ), precision );
          const QDomText upperCornerText = doc.createTextNode( QString( "%1 %2" ).arg( firstCoordStr, secondCoordStr ) );

          QDomElement upperCornerElement = doc.createElement( u"ows:UpperCorner"_s );
          upperCornerElement.appendChild( upperCornerText );
          bboxElement.appendChild( upperCornerElement );

          // update layer element
          layerElem.appendChild( bboxElement );
        }

        // Layer Style
        QDomElement layerStyleElem = doc.createElement( u"Style"_s );
        layerStyleElem.setAttribute( u"isDefault"_s, u"true"_s );
        QDomElement layerStyleIdElem = doc.createElement( u"ows:Identifier"_s );
        const QDomText layerStyleIdText = doc.createTextNode( u"default"_s );
        layerStyleIdElem.appendChild( layerStyleIdText );
        layerStyleElem.appendChild( layerStyleIdElem );
        QDomElement layerStyleTitleElem = doc.createElement( u"ows:Title"_s );
        const QDomText layerStyleTitleText = doc.createTextNode( u"default"_s );
        layerStyleTitleElem.appendChild( layerStyleTitleText );
        layerStyleElem.appendChild( layerStyleTitleElem );
        layerElem.appendChild( layerStyleElem );

        for ( const QString &format : wmtsLayer.formats )
        {
          QDomElement layerFormatElem = doc.createElement( u"Format"_s );
          const QDomText layerFormatText = doc.createTextNode( format );
          layerFormatElem.appendChild( layerFormatText );
          layerElem.appendChild( layerFormatElem );
        }

        if ( wmtsLayer.queryable )
        {
          appendInfoFormat( layerElem, u"text/plain"_s );
          appendInfoFormat( layerElem, u"text/html"_s );
          appendInfoFormat( layerElem, u"text/xml"_s );
          appendInfoFormat( layerElem, u"application/vnd.ogc.gml"_s );
          appendInfoFormat( layerElem, u"application/vnd.ogc.gml/3.1.1"_s );
        }

        for ( const tileMatrixSetDef &tms : tmsList )
        {
          tileMatrixSetLinkDef tmsl = getLayerTileMatrixSetLink( wmtsLayer, tms, project );
          if ( tmsl.ref.isEmpty() || tmsl.ref != tms.ref )
          {
            continue;
          }

          //wmts:TileMatrixSetLink
          QDomElement tmslElement = doc.createElement( u"TileMatrixSetLink"_s /*wmts:TileMatrixSetLink*/ );

          QDomElement identifierElem = doc.createElement( u"TileMatrixSet"_s );
          const QDomText identifierText = doc.createTextNode( tms.ref );
          identifierElem.appendChild( identifierText );
          tmslElement.appendChild( identifierElem );

          //wmts:TileMatrixSetLimits
          QDomElement tmsLimitsElement = doc.createElement( u"TileMatrixSetLimits"_s /*wmts:TileMatrixSetLimits*/ );

          for ( auto it = tmsl.tileMatrixLimits.constBegin(); it != tmsl.tileMatrixLimits.constEnd(); it++ )
          {
            QDomElement tmLimitsElement = doc.createElement( u"TileMatrixLimits"_s /*wmts:TileMatrixLimits*/ );

            QDomElement tmIdentifierElem = doc.createElement( u"TileMatrix"_s );
            const QDomText tmIdentifierText = doc.createTextNode( QString::number( it.key() ) );
            tmIdentifierElem.appendChild( tmIdentifierText );
            tmLimitsElement.appendChild( tmIdentifierElem );

            const tileMatrixLimitDef tml = it.value();

            QDomElement minTileColElem = doc.createElement( u"MinTileCol"_s );
            const QDomText minTileColText = doc.createTextNode( QString::number( tml.minCol ) );
            minTileColElem.appendChild( minTileColText );
            tmLimitsElement.appendChild( minTileColElem );

            QDomElement maxTileColElem = doc.createElement( u"MaxTileCol"_s );
            const QDomText maxTileColText = doc.createTextNode( QString::number( tml.maxCol ) );
            maxTileColElem.appendChild( maxTileColText );
            tmLimitsElement.appendChild( maxTileColElem );

            QDomElement minTileRowElem = doc.createElement( u"MinTileRow"_s );
            const QDomText minTileRowText = doc.createTextNode( QString::number( tml.minRow ) );
            minTileRowElem.appendChild( minTileRowText );
            tmLimitsElement.appendChild( minTileRowElem );

            QDomElement maxTileRowElem = doc.createElement( u"MaxTileRow"_s );
            const QDomText maxTileRowText = doc.createTextNode( QString::number( tml.maxRow ) );
            maxTileRowElem.appendChild( maxTileRowText );
            tmLimitsElement.appendChild( maxTileRowElem );

            tmsLimitsElement.appendChild( tmLimitsElement );
          }
          tmslElement.appendChild( tmsLimitsElement );

          layerElem.appendChild( tmslElement );
        }

        contentsElement.appendChild( layerElem );
      }
    }

    void appendTileMatrixSetElements( QDomDocument &doc, QDomElement &contentsElement, QList<tileMatrixSetDef> tmsList )
    {
      for ( const tileMatrixSetDef &tms : tmsList )
      {
        //wmts:TileMatrixSet
        QDomElement tmsElement = doc.createElement( u"TileMatrixSet"_s /*wmts:TileMatrixSet*/ );

        QDomElement identifierElem = doc.createElement( u"ows:Identifier"_s );
        const QDomText identifierText = doc.createTextNode( tms.ref );
        identifierElem.appendChild( identifierText );
        tmsElement.appendChild( identifierElem );

        QDomElement crsElem = doc.createElement( u"ows:SupportedCRS"_s );
        const QDomText crsText = doc.createTextNode( tms.ref );
        crsElem.appendChild( crsText );
        tmsElement.appendChild( crsElem );

        const QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( tms.ref );
        int precision = 3;
        if ( crs.isGeographic() )
        {
          precision = 6;
        }

        //wmts:TileMatrix
        int tmIdx = 0;
        for ( const tileMatrixDef &tm : tms.tileMatrixList )
        {
          QDomElement tmElement = doc.createElement( u"TileMatrix"_s /*wmts:TileMatrix*/ );

          QDomElement tmIdentifierElem = doc.createElement( u"ows:Identifier"_s );
          const QDomText tmIdentifierText = doc.createTextNode( QString::number( tmIdx ) );
          tmIdentifierElem.appendChild( tmIdentifierText );
          tmElement.appendChild( tmIdentifierElem );

          QDomElement tmScaleDenomElem = doc.createElement( u"ScaleDenominator"_s );
          const QDomText tmScaleDenomText = doc.createTextNode( qgsDoubleToString( tm.scaleDenominator, 6 ) );
          tmScaleDenomElem.appendChild( tmScaleDenomText );
          tmElement.appendChild( tmScaleDenomElem );

          QDomElement tmTopLeftCornerElem = doc.createElement( u"TopLeftCorner"_s );
          if ( tms.hasAxisInverted )
          {
            const QDomText tmTopLeftCornerText = doc.createTextNode( qgsDoubleToString( QgsServerProjectUtils::ceilWithPrecision( tm.top, precision ), precision ) + ' ' + qgsDoubleToString( QgsServerProjectUtils::floorWithPrecision( tm.left, precision ), precision ) );
            tmTopLeftCornerElem.appendChild( tmTopLeftCornerText );
          }
          else
          {
            const QDomText tmTopLeftCornerText = doc.createTextNode( qgsDoubleToString( QgsServerProjectUtils::floorWithPrecision( tm.left, precision ), precision ) + ' ' + qgsDoubleToString( QgsServerProjectUtils::ceilWithPrecision( tm.top, precision ), precision ) );
            tmTopLeftCornerElem.appendChild( tmTopLeftCornerText );
          }
          tmElement.appendChild( tmTopLeftCornerElem );

          QDomElement tmTileWidthElem = doc.createElement( u"TileWidth"_s );
          const QDomText tmTileWidthText = doc.createTextNode( QString::number( 256 ) );
          tmTileWidthElem.appendChild( tmTileWidthText );
          tmElement.appendChild( tmTileWidthElem );

          QDomElement tmTileHeightElem = doc.createElement( u"TileHeight"_s );
          const QDomText tmTileHeightText = doc.createTextNode( QString::number( 256 ) );
          tmTileHeightElem.appendChild( tmTileHeightText );
          tmElement.appendChild( tmTileHeightElem );

          QDomElement tmColElem = doc.createElement( u"MatrixWidth"_s );
          const QDomText tmColText = doc.createTextNode( QString::number( tm.col ) );
          tmColElem.appendChild( tmColText );
          tmElement.appendChild( tmColElem );

          QDomElement tmRowElem = doc.createElement( u"MatrixHeight"_s );
          const QDomText tmRowText = doc.createTextNode( QString::number( tm.row ) );
          tmRowElem.appendChild( tmRowText );
          tmElement.appendChild( tmRowElem );

          tmsElement.appendChild( tmElement );
          ++tmIdx;
        }

        contentsElement.appendChild( tmsElement );
      }
    }

  } // namespace

} // namespace QgsWmts
