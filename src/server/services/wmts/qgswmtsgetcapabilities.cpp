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
#include "qgswmtsutils.h"
#include "qgsserverprojectutils.h"
#include "qgswmtsgetcapabilities.h"

#include "qgsproject.h"
#include "qgsexception.h"
#include "qgscoordinatereferencesystem.h"

namespace QgsWmts
{
  namespace
  {
    void appendLayerElements( QDomDocument &doc, QDomElement &contentsElement,
                              QList< layerDef > wmtsLayers, QList< tileMatrixSetDef > tmsList,
                              const QgsProject *project );

    void appendTileMatrixSetElements( QDomDocument &doc, QDomElement &contentsElement,
                                      QList< tileMatrixSetDef > tmsList );
  }

  /**
   * Output WMTS  GetCapabilities response
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

    //wmts:Capabilities element
    QDomElement wmtsCapabilitiesElement = doc.createElement( QStringLiteral( "Capabilities" )/*wmts:Capabilities*/ );
    wmtsCapabilitiesElement.setAttribute( QStringLiteral( "xmlns" ), WMTS_NAMESPACE );
    wmtsCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:gml" ), GML_NAMESPACE );
    wmtsCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:ows" ), OWS_NAMESPACE );
    wmtsCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:xlink" ), QStringLiteral( "http://www.w3.org/1999/xlink" ) );
    wmtsCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:xsi" ), QStringLiteral( "http://www.w3.org/2001/XMLSchema-instance" ) );
    wmtsCapabilitiesElement.setAttribute( QStringLiteral( "xsi:schemaLocation" ), WMTS_NAMESPACE + " http://schemas.opengis.net/wmts/1.0/wmtsGetCapabilities_response.xsd" );
    wmtsCapabilitiesElement.setAttribute( QStringLiteral( "version" ), implementationVersion() );
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
    QDomElement serviceElem = doc.createElement( QStringLiteral( "ows:ServiceIdentification" ) );

    //Service type
    QDomElement typeElem = doc.createElement( QStringLiteral( "ows:ServiceType" ) );
    QDomText typeText = doc.createTextNode( QStringLiteral( "OGC WMTS" ) );
    typeElem.appendChild( typeText );
    serviceElem.appendChild( typeElem );

    //Service type version
    QDomElement typeVersionElem = doc.createElement( QStringLiteral( "ows:ServiceTypeVersion" ) );
    QDomText typeVersionText = doc.createTextNode( implementationVersion() );
    typeVersionElem.appendChild( typeVersionText );
    serviceElem.appendChild( typeVersionElem );

    QString title = QgsServerProjectUtils::owsServiceTitle( *project );
    if ( !title.isEmpty() )
    {
      QDomElement titleElem = doc.createElement( QStringLiteral( "ows:Title" ) );
      QDomText titleText = doc.createTextNode( title );
      titleElem.appendChild( titleText );
      serviceElem.appendChild( titleElem );
    }

    QString abstract = QgsServerProjectUtils::owsServiceAbstract( *project );
    if ( !abstract.isEmpty() )
    {
      QDomElement abstractElem = doc.createElement( QStringLiteral( "ows:Abstract" ) );
      QDomText abstractText = doc.createCDATASection( abstract );
      abstractElem.appendChild( abstractText );
      serviceElem.appendChild( abstractElem );
    }

    QStringList keywords = QgsServerProjectUtils::owsServiceKeywords( *project );
    if ( !keywords.isEmpty() )
    {
      QDomElement keywordsElem = doc.createElement( QStringLiteral( "ows:Keywords" ) );
      for ( const QString &k : keywords )
      {
        QDomElement keywordElem = doc.createElement( QStringLiteral( "ows:Keyword" ) );
        QDomText keywordText = doc.createTextNode( k );
        keywordElem.appendChild( keywordText );
        keywordsElem.appendChild( keywordElem );
      }
      serviceElem.appendChild( keywordsElem );
    }

    QDomElement feesElem = doc.createElement( QStringLiteral( "ows:Fees" ) );
    QDomText feesText = doc.createTextNode( QStringLiteral( "None" ) ); // default value if fees are unknown
    QString fees = QgsServerProjectUtils::owsServiceFees( *project );
    if ( !fees.isEmpty() )
    {
      feesText = doc.createTextNode( fees );
    }
    feesElem.appendChild( feesText );
    serviceElem.appendChild( feesElem );

    QDomElement accessConstraintsElem = doc.createElement( QStringLiteral( "ows:AccessConstraints" ) );
    QDomText accessConstraintsText = doc.createTextNode( QStringLiteral( "None" ) ); // default value if access constraints are unknown
    QString accessConstraints = QgsServerProjectUtils::owsServiceAccessConstraints( *project );
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
    QDomElement serviceElem = doc.createElement( QStringLiteral( "ows:ServiceProvider" ) );

    QString contactOrganization = QgsServerProjectUtils::owsServiceContactOrganization( *project );
    if ( !contactOrganization.isEmpty() )
    {
      QDomElement contactOrganizationElem = doc.createElement( QStringLiteral( "ows:ProviderName" ) );
      QDomText contactOrganizationText = doc.createTextNode( contactOrganization );
      contactOrganizationElem.appendChild( contactOrganizationText );
      serviceElem.appendChild( contactOrganizationElem );
    }

    QString onlineResource = QgsServerProjectUtils::owsServiceOnlineResource( *project );
    if ( !onlineResource.isEmpty() )
    {
      QDomElement onlineResourceElem = doc.createElement( QStringLiteral( "ows:ProviderSite" ) );
      onlineResourceElem.setAttribute( QStringLiteral( "xlink:href" ), onlineResource );
      serviceElem.appendChild( onlineResourceElem );
    }

    //Contact information
    QString contactPerson = QgsServerProjectUtils::owsServiceContactPerson( *project );
    QString contactPosition = QgsServerProjectUtils::owsServiceContactPosition( *project );
    QString contactMail = QgsServerProjectUtils::owsServiceContactMail( *project );
    QString contactPhone = QgsServerProjectUtils::owsServiceContactPhone( *project );
    if ( !contactPerson.isEmpty() ||
         !contactPosition.isEmpty() ||
         !contactMail.isEmpty() ||
         !contactPhone.isEmpty() )
    {
      QDomElement serviceContactElem = doc.createElement( QStringLiteral( "ows:ServiceContact" ) );
      if ( !contactPerson.isEmpty() )
      {
        QDomElement contactPersonElem = doc.createElement( QStringLiteral( "ows:IndividualName" ) );
        QDomText contactPersonText = doc.createTextNode( contactPerson );
        contactPersonElem.appendChild( contactPersonText );
        serviceContactElem.appendChild( contactPersonElem );
      }
      if ( !contactPosition.isEmpty() )
      {
        QDomElement contactPositionElem = doc.createElement( QStringLiteral( "ows:PositionName" ) );
        QDomText contactPositionText = doc.createTextNode( contactPosition );
        contactPositionElem.appendChild( contactPositionText );
        serviceContactElem.appendChild( contactPositionElem );
      }
      if ( !contactMail.isEmpty() ||
           !contactPhone.isEmpty() )
      {
        QDomElement contactInfoElem = doc.createElement( QStringLiteral( "ows:ContactInfo" ) );
        if ( !contactMail.isEmpty() )
        {
          QDomElement contactAddressElem = doc.createElement( QStringLiteral( "ows:Address" ) );
          QDomElement contactAddressMailElem = doc.createElement( QStringLiteral( "ows:ElectronicMailAddress" ) );
          QDomText contactAddressMailText = doc.createTextNode( contactMail );
          contactAddressMailElem.appendChild( contactAddressMailText );
          contactAddressElem.appendChild( contactAddressMailElem );
          contactInfoElem.appendChild( contactAddressElem );
        }
        if ( !contactPhone.isEmpty() )
        {
          QDomElement contactPhoneElem = doc.createElement( QStringLiteral( "ows:Phone" ) );
          QDomElement contactVoiceElem = doc.createElement( QStringLiteral( "ows:Voice" ) );
          QDomText contactVoiceText = doc.createTextNode( contactPhone );
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
    QDomElement operationsMetadataElement = doc.createElement( QStringLiteral( "ows:OperationsMetadata" )/*ows:OperationsMetadata*/ );

    //ows:Operation element with name GetCapabilities
    QDomElement getCapabilitiesElement = doc.createElement( QStringLiteral( "ows:Operation" )/*ows:Operation*/ );
    getCapabilitiesElement.setAttribute( QStringLiteral( "name" ), QStringLiteral( "GetCapabilities" ) );
    operationsMetadataElement.appendChild( getCapabilitiesElement );

    //ows:DCP
    QDomElement dcpElement = doc.createElement( QStringLiteral( "ows:DCP" )/*ows:DCP*/ );
    getCapabilitiesElement.appendChild( dcpElement );
    QDomElement httpElement = doc.createElement( QStringLiteral( "ows:HTTP" )/*ows:HTTP*/ );
    dcpElement.appendChild( httpElement );

    // Get service URL
    const QUrl href = serviceUrl( request, project, *settings );

    //href needs to be a prefix
    QString hrefString = href.toString();
    hrefString.append( href.hasQuery() ? '&' : '?' );

    //ows:Get
    QDomElement getElement = doc.createElement( QStringLiteral( "ows:Get" )/*ows:Get*/ );
    getElement.setAttribute( QStringLiteral( "xlink:href" ), hrefString );
    QDomElement constraintElement = doc.createElement( QStringLiteral( "ows:Constraint" )/*ows:Constraint*/ );
    constraintElement.setAttribute( QStringLiteral( "name" ), QStringLiteral( "GetEncoding" ) );
    QDomElement allowedValuesElement = doc.createElement( QStringLiteral( "ows:AllowedValues" )/*ows:AllowedValues*/ );
    QDomElement valueElement = doc.createElement( QStringLiteral( "ows:Value" )/*ows:Value*/ );
    QDomText valueText = doc.createTextNode( QStringLiteral( "KVP" ) );
    valueElement.appendChild( valueText );
    allowedValuesElement.appendChild( valueElement );
    constraintElement.appendChild( allowedValuesElement );
    getElement.appendChild( constraintElement );
    httpElement.appendChild( getElement );

    //ows:Operation element with name GetTile
    QDomElement getTileElement = getCapabilitiesElement.cloneNode().toElement();//this is the same as 'GetCapabilities'
    getTileElement.setAttribute( QStringLiteral( "name" ), QStringLiteral( "GetTile" ) );
    operationsMetadataElement.appendChild( getTileElement );

    //ows:Operation element with name GetFeatureInfo
    QDomElement getFeatureInfoElement = getCapabilitiesElement.cloneNode().toElement();//this is the same as 'GetCapabilities'
    getFeatureInfoElement.setAttribute( QStringLiteral( "name" ), QStringLiteral( "GetFeatureInfo" ) );
    operationsMetadataElement.appendChild( getFeatureInfoElement );

    // End
    return operationsMetadataElement;
  }

  QDomElement getContentsElement( QDomDocument &doc, QgsServerInterface *serverIface, const QgsProject *project )
  {
    /*
     * Adding layer list in ContentMetadata
     */
    QDomElement contentsElement = doc.createElement( QStringLiteral( "Contents" )/*wmts:Contents*/ );

    QList< tileMatrixSetDef > tmsList = getTileMatrixSetList( project );
    if ( !tmsList.isEmpty() )
    {
      // get layer list
      QList< layerDef > wmtsLayers = getWmtsLayerList( serverIface, project );
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
    void appendLayerElements( QDomDocument &doc, QDomElement &contentsElement,
                              QList< layerDef > wmtsLayers, QList< tileMatrixSetDef > tmsList,
                              const QgsProject *project )
    {
      QgsCoordinateReferenceSystem wgs84 = QgsCoordinateReferenceSystem::fromOgcWmsCrs( geoEpsgCrsAuthId() );
      // Define InfoFormat helper
      std::function < void ( QDomElement &, const QString & ) > appendInfoFormat = [&doc]( QDomElement & elem, const QString & format )
      {
        QDomElement formatElem = doc.createElement( QStringLiteral( "InfoFormat" )/*wmts:InfoFormat*/ );
        formatElem.appendChild( doc.createTextNode( format ) );
        elem.appendChild( formatElem );
      };

      for ( const layerDef &wmtsLayer : wmtsLayers )
      {
        if ( wmtsLayer.id.isEmpty() )
          continue;

        QDomElement layerElem = doc.createElement( QStringLiteral( "Layer" ) );

        QDomElement layerIdElem = doc.createElement( QStringLiteral( "ows:Identifier" ) );
        QDomText layerIdText = doc.createTextNode( wmtsLayer.id );
        layerIdElem.appendChild( layerIdText );
        layerElem.appendChild( layerIdElem );

        if ( !wmtsLayer.title.isEmpty() )
        {
          // Layer title
          QDomElement layerTitleElem = doc.createElement( QStringLiteral( "ows:Title" ) );
          QDomText layerTitleText = doc.createTextNode( wmtsLayer.title );
          layerTitleElem.appendChild( layerTitleText );
          layerElem.appendChild( layerTitleElem );
        }

        if ( !wmtsLayer.abstract.isEmpty() )
        {
          // Layer abstract
          QDomElement layerAbstElem = doc.createElement( QStringLiteral( "ows:Abstract" ) );
          QDomText layerAbstText = doc.createTextNode( project->title() );
          layerAbstElem.appendChild( layerAbstText );
          layerElem.appendChild( layerAbstElem );
        }

        // WGS84 bounding box
        int wgs84precision = 6;
        QDomElement wgs84BBoxElement = doc.createElement( QStringLiteral( "ows:WGS84BoundingBox" ) );
        QDomElement wgs84LowerCornerElement = doc.createElement( QStringLiteral( "ows:LowerCorner" ) );
        QDomText wgs84LowerCornerText = doc.createTextNode( qgsDoubleToString( QgsServerProjectUtils::floorWithPrecision( wmtsLayer.wgs84BoundingRect.xMinimum(), wgs84precision ), wgs84precision ) + ' ' + qgsDoubleToString( QgsServerProjectUtils::floorWithPrecision( wmtsLayer.wgs84BoundingRect.yMinimum(), wgs84precision ), wgs84precision ) );
        wgs84LowerCornerElement.appendChild( wgs84LowerCornerText );
        wgs84BBoxElement.appendChild( wgs84LowerCornerElement );
        QDomElement wgs84UpperCornerElement = doc.createElement( QStringLiteral( "ows:UpperCorner" ) );
        QDomText wgs84UpperCornerText = doc.createTextNode( qgsDoubleToString( QgsServerProjectUtils::ceilWithPrecision( wmtsLayer.wgs84BoundingRect.xMaximum(), wgs84precision ), wgs84precision ) + ' ' + qgsDoubleToString( QgsServerProjectUtils::ceilWithPrecision( wmtsLayer.wgs84BoundingRect.yMaximum(), wgs84precision ), wgs84precision ) );
        wgs84UpperCornerElement.appendChild( wgs84UpperCornerText );
        wgs84BBoxElement.appendChild( wgs84UpperCornerElement );
        layerElem.appendChild( wgs84BBoxElement );

        // Other bounding boxes
        for ( const tileMatrixSetDef &tms : tmsList )
        {
          if ( tms.ref == QLatin1String( "EPSG:4326" ) )
            continue;

          QgsRectangle rect;
          QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( tms.ref );
          QgsCoordinateTransform exGeoTransform( wgs84, crs, project );
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

          QDomElement bboxElement = doc.createElement( QStringLiteral( "ows:BoundingBox" ) );
          bboxElement.setAttribute( QStringLiteral( "crs" ), tms.ref );
          QDomElement lowerCornerElement = doc.createElement( QStringLiteral( "ows:LowerCorner" ) );
          QDomText lowerCornerText = doc.createTextNode( qgsDoubleToString( QgsServerProjectUtils::floorWithPrecision( rect.xMinimum(), precision ), precision ) + ' ' + qgsDoubleToString( QgsServerProjectUtils::floorWithPrecision( rect.yMinimum(), precision ), precision ) );
          lowerCornerElement.appendChild( lowerCornerText );
          bboxElement.appendChild( lowerCornerElement );
          QDomElement upperCornerElement = doc.createElement( QStringLiteral( "ows:UpperCorner" ) );
          QDomText upperCornerText = doc.createTextNode( qgsDoubleToString( QgsServerProjectUtils::ceilWithPrecision( rect.xMaximum(), precision ), precision ) + ' ' + qgsDoubleToString( QgsServerProjectUtils::ceilWithPrecision( rect.yMaximum(), precision ), precision ) );
          upperCornerElement.appendChild( upperCornerText );
          bboxElement.appendChild( upperCornerElement );
          layerElem.appendChild( bboxElement );
        }

        // Layer Style
        QDomElement layerStyleElem = doc.createElement( QStringLiteral( "Style" ) );
        layerStyleElem.setAttribute( QStringLiteral( "isDefault" ), QStringLiteral( "true" ) );
        QDomElement layerStyleIdElem = doc.createElement( QStringLiteral( "ows:Identifier" ) );
        QDomText layerStyleIdText = doc.createTextNode( QStringLiteral( "default" ) );
        layerStyleIdElem.appendChild( layerStyleIdText );
        layerStyleElem.appendChild( layerStyleIdElem );
        QDomElement layerStyleTitleElem = doc.createElement( QStringLiteral( "ows:Title" ) );
        QDomText layerStyleTitleText = doc.createTextNode( QStringLiteral( "default" ) );
        layerStyleTitleElem.appendChild( layerStyleTitleText );
        layerStyleElem.appendChild( layerStyleTitleElem );
        layerElem.appendChild( layerStyleElem );

        for ( const QString &format : wmtsLayer.formats )
        {
          QDomElement layerFormatElem = doc.createElement( QStringLiteral( "Format" ) );
          QDomText layerFormatText = doc.createTextNode( format );
          layerFormatElem.appendChild( layerFormatText );
          layerElem.appendChild( layerFormatElem );
        }

        if ( wmtsLayer.queryable )
        {
          appendInfoFormat( layerElem, QStringLiteral( "text/plain" ) );
          appendInfoFormat( layerElem, QStringLiteral( "text/html" ) );
          appendInfoFormat( layerElem, QStringLiteral( "text/xml" ) );
          appendInfoFormat( layerElem, QStringLiteral( "application/vnd.ogc.gml" ) );
          appendInfoFormat( layerElem, QStringLiteral( "application/vnd.ogc.gml/3.1.1" ) );
        }

        for ( const tileMatrixSetDef &tms : tmsList )
        {
          tileMatrixSetLinkDef tmsl = getLayerTileMatrixSetLink( wmtsLayer, tms, project );
          if ( tmsl.ref.isEmpty() || tmsl.ref != tms.ref )
          {
            continue;
          }

          //wmts:TileMatrixSetLink
          QDomElement tmslElement = doc.createElement( QStringLiteral( "TileMatrixSetLink" )/*wmts:TileMatrixSetLink*/ );

          QDomElement identifierElem = doc.createElement( QStringLiteral( "TileMatrixSet" ) );
          QDomText identifierText = doc.createTextNode( tms.ref );
          identifierElem.appendChild( identifierText );
          tmslElement.appendChild( identifierElem );

          //wmts:TileMatrixSetLimits
          QDomElement tmsLimitsElement = doc.createElement( QStringLiteral( "TileMatrixSetLimits" )/*wmts:TileMatrixSetLimits*/ );
          for ( int tmIdx : tmsl.tileMatrixLimits.keys() )
          {
            QDomElement tmLimitsElement = doc.createElement( QStringLiteral( "TileMatrixLimits" )/*wmts:TileMatrixLimits*/ );

            QDomElement tmIdentifierElem = doc.createElement( QStringLiteral( "TileMatrix" ) );
            QDomText tmIdentifierText = doc.createTextNode( QString::number( tmIdx ) );
            tmIdentifierElem.appendChild( tmIdentifierText );
            tmLimitsElement.appendChild( tmIdentifierElem );

            tileMatrixLimitDef tml = tmsl.tileMatrixLimits[tmIdx];

            QDomElement minTileColElem = doc.createElement( QStringLiteral( "MinTileCol" ) );
            QDomText minTileColText = doc.createTextNode( QString::number( tml.minCol ) );
            minTileColElem.appendChild( minTileColText );
            tmLimitsElement.appendChild( minTileColElem );

            QDomElement maxTileColElem = doc.createElement( QStringLiteral( "MaxTileCol" ) );
            QDomText maxTileColText = doc.createTextNode( QString::number( tml.maxCol ) );
            maxTileColElem.appendChild( maxTileColText );
            tmLimitsElement.appendChild( maxTileColElem );

            QDomElement minTileRowElem = doc.createElement( QStringLiteral( "MinTileRow" ) );
            QDomText minTileRowText = doc.createTextNode( QString::number( tml.minRow ) );
            minTileRowElem.appendChild( minTileRowText );
            tmLimitsElement.appendChild( minTileRowElem );

            QDomElement maxTileRowElem = doc.createElement( QStringLiteral( "MaxTileRow" ) );
            QDomText maxTileRowText = doc.createTextNode( QString::number( tml.maxRow ) );
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

    void appendTileMatrixSetElements( QDomDocument &doc, QDomElement &contentsElement,
                                      QList< tileMatrixSetDef > tmsList )
    {
      for ( const tileMatrixSetDef &tms : tmsList )
      {
        //wmts:TileMatrixSet
        QDomElement tmsElement = doc.createElement( QStringLiteral( "TileMatrixSet" )/*wmts:TileMatrixSet*/ );

        QDomElement identifierElem = doc.createElement( QStringLiteral( "ows:Identifier" ) );
        QDomText identifierText = doc.createTextNode( tms.ref );
        identifierElem.appendChild( identifierText );
        tmsElement.appendChild( identifierElem );

        QDomElement crsElem = doc.createElement( QStringLiteral( "ows:SupportedCRS" ) );
        QDomText crsText = doc.createTextNode( tms.ref );
        crsElem.appendChild( crsText );
        tmsElement.appendChild( crsElem );

        QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( tms.ref );
        int precision = 3;
        if ( crs.isGeographic() )
        {
          precision = 6;
        }

        //wmts:TileMatrix
        int tmIdx = 0;
        for ( const tileMatrixDef &tm : tms.tileMatrixList )
        {
          QDomElement tmElement = doc.createElement( QStringLiteral( "TileMatrix" )/*wmts:TileMatrix*/ );

          QDomElement tmIdentifierElem = doc.createElement( QStringLiteral( "ows:Identifier" ) );
          QDomText tmIdentifierText = doc.createTextNode( QString::number( tmIdx ) );
          tmIdentifierElem.appendChild( tmIdentifierText );
          tmElement.appendChild( tmIdentifierElem );

          QDomElement tmScaleDenomElem = doc.createElement( QStringLiteral( "ScaleDenominator" ) );
          QDomText tmScaleDenomText = doc.createTextNode( qgsDoubleToString( tm.scaleDenominator, 6 ) );
          tmScaleDenomElem.appendChild( tmScaleDenomText );
          tmElement.appendChild( tmScaleDenomElem );

          QDomElement tmTopLeftCornerElem = doc.createElement( QStringLiteral( "TopLeftCorner" ) );
          if ( tms.hasAxisInverted )
          {
            QDomText tmTopLeftCornerText = doc.createTextNode( qgsDoubleToString( QgsServerProjectUtils::ceilWithPrecision( tm.top, precision ), precision ) + ' ' + qgsDoubleToString( QgsServerProjectUtils::floorWithPrecision( tm.left, precision ), precision ) );
            tmTopLeftCornerElem.appendChild( tmTopLeftCornerText );
          }
          else
          {
            QDomText tmTopLeftCornerText = doc.createTextNode( qgsDoubleToString( QgsServerProjectUtils::floorWithPrecision( tm.left, precision ), precision ) + ' ' + qgsDoubleToString( QgsServerProjectUtils::ceilWithPrecision( tm.top, precision ), precision ) );
            tmTopLeftCornerElem.appendChild( tmTopLeftCornerText );
          }
          tmElement.appendChild( tmTopLeftCornerElem );

          QDomElement tmTileWidthElem = doc.createElement( QStringLiteral( "TileWidth" ) );
          QDomText tmTileWidthText = doc.createTextNode( QString::number( 256 ) );
          tmTileWidthElem.appendChild( tmTileWidthText );
          tmElement.appendChild( tmTileWidthElem );

          QDomElement tmTileHeightElem = doc.createElement( QStringLiteral( "TileHeight" ) );
          QDomText tmTileHeightText = doc.createTextNode( QString::number( 256 ) );
          tmTileHeightElem.appendChild( tmTileHeightText );
          tmElement.appendChild( tmTileHeightElem );

          QDomElement tmColElem = doc.createElement( QStringLiteral( "MatrixWidth" ) );
          QDomText tmColText = doc.createTextNode( QString::number( tm.col ) );
          tmColElem.appendChild( tmColText );
          tmElement.appendChild( tmColElem );

          QDomElement tmRowElem = doc.createElement( QStringLiteral( "MatrixHeight" ) );
          QDomText tmRowText = doc.createTextNode( QString::number( tm.row ) );
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



