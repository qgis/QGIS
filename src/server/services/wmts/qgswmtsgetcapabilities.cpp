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
#include "qgsmapserviceexception.h"
#include "qgscoordinatereferencesystem.h"
#include "qgslayertree.h"
#include "qgslayertreemodel.h"
#include "qgslayertreemodellegendnode.h"

#include <QStringList>

namespace QgsWmts
{

  /**
   * Output WMTS  GetCapabilities response
   */
  void writeGetCapabilities( QgsServerInterface *serverIface, const QgsProject *project, const QString &version,
                             const QgsServerRequest &request, QgsServerResponse &response )
  {
    QStringList cacheKeyList;
    bool cache = true;

    QgsAccessControl *accessControl = serverIface->accessControls();
    if ( accessControl )
      cache = accessControl->fillCacheKey( cacheKeyList );

    QDomDocument doc;
    QString cacheKey = cacheKeyList.join( QStringLiteral( "-" ) );
    const QDomDocument *capabilitiesDocument = nullptr;

    QgsServerCacheManager *cacheManager = serverIface->cacheManager();
    if ( cacheManager && cache )
    {
      QByteArray content = cacheManager->getCachedDocument( project, request, cacheKey );
      if ( !content.isEmpty() && doc.setContent( content ) )
      {
        doc = doc.cloneNode().toDocument();
        capabilitiesDocument = &doc;
      }
    }

    if ( !capabilitiesDocument ) //capabilities xml not in cache. Create a new one
    {
      doc = createGetCapabilitiesDocument( serverIface, project, version, request );

      if ( cache && cacheManager )
      {
        if ( cacheManager->setCachedDocument( &doc, project, request, cacheKey ) )
        {
          QByteArray content = cacheManager->getCachedDocument( project, request, cacheKey );
          if ( !content.isEmpty() && doc.setContent( content ) )
          {
            doc = doc.cloneNode().toDocument();
            capabilitiesDocument = &doc;
          }
        }
      }
      if ( !capabilitiesDocument )
      {
        doc = doc.cloneNode().toDocument();
        capabilitiesDocument = &doc;
      }
    }

    response.setHeader( "Content-Type", "text/xml; charset=utf-8" );
    response.write( capabilitiesDocument->toByteArray() );
  }


  QDomDocument createGetCapabilitiesDocument( QgsServerInterface *serverIface, const QgsProject *project, const QString &version,
      const QgsServerRequest &request )
  {
    Q_UNUSED( version );

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
    wmtsCapabilitiesElement.appendChild( getOperationsMetadataElement( doc, project, request ) );

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
    QDomText typeText = doc.createTextNode( "OGC WMTS" );
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
      for ( int i = 0; i < keywords.size(); ++i )
      {
        QDomElement keywordElem = doc.createElement( QStringLiteral( "ows:Keyword" ) );
        QDomText keywordText = doc.createTextNode( keywords.at( i ) );
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

    //Contact informations
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

  QDomElement getOperationsMetadataElement( QDomDocument &doc, const QgsProject *project, const QgsServerRequest &request )
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

    //Prepare url
    QString hrefString = serviceUrl( request, project );

    //ows:Get
    QDomElement getElement = doc.createElement( QStringLiteral( "ows:Get" )/*ows:Get*/ );
    getElement.setAttribute( QStringLiteral( "xlink:href" ), hrefString );
    httpElement.appendChild( getElement );

    //ows:Operation element with name GetTile
    QDomElement getTileElement = getCapabilitiesElement.cloneNode().toElement();//this is the same as 'GetCapabilities'
    getTileElement.setAttribute( QStringLiteral( "name" ), QStringLiteral( "GetTile" ) );
    operationsMetadataElement.appendChild( getTileElement );

    //ows:Operation element with name GetFeatureInfo
    /*QDomElement getFeatureInfoElement = getCapabilitiesElement.cloneNode().toElement();//this is the same as 'GetCapabilities'
    getFeatureInfoElement.setAttribute( QStringLiteral( "name" ), QStringLiteral( "GetFeatureInfo" ) );
    operationsMetadataElement.appendChild( getFeatureInfoElement );*/

    // End
    return operationsMetadataElement;
  }

  QDomElement getContentsElement( QDomDocument &doc, QgsServerInterface *serverIface, const QgsProject *project )
  {
#ifdef HAVE_SERVER_PYTHON_PLUGINS
    QgsAccessControl *accessControl = serverIface->accessControls();
#endif
    /*
     * Adding layer list in ContentMetadata
     */
    QDomElement contentsElement = doc.createElement( QStringLiteral( "Contents" )/*wmts:Contents*/ );

    QList< tileMatrixSet > tmsList = getTileMatrixSetList( project );
    if ( !tmsList.isEmpty() )
    {
      QList< layerDef > wmtsLayers;
      QgsCoordinateReferenceSystem wgs84 = QgsCoordinateReferenceSystem::fromOgcWmsCrs( GEO_EPSG_CRS_AUTHID );
      QList<tileMatrixSet>::iterator tmsIt = tmsList.begin();

      // WMTS Project configuration
      bool wmtsProject = project->readBoolEntry( QStringLiteral( "WMTSLayers" ), QStringLiteral( "Project" ) );

      if ( wmtsProject )
      {
        layerDef pLayer;

        // Root Layer name
        QString rootLayerName = QgsServerProjectUtils::wmsRootName( *project );
        if ( rootLayerName.isEmpty() && !project->title().isEmpty() )
        {
          rootLayerName = project->title();
        }
        pLayer.id = rootLayerName;

        if ( !project->title().isEmpty() )
        {
          pLayer.title = project->title();
          pLayer.abstract = project->title();
        }

        //transform the project native CRS into WGS84
        QgsRectangle projRect = QgsServerProjectUtils::wmsExtent( *project );
        QgsCoordinateReferenceSystem projCrs = project->crs();
        Q_NOWARN_DEPRECATED_PUSH
        QgsCoordinateTransform exGeoTransform( projCrs, wgs84 );
        Q_NOWARN_DEPRECATED_POP
        try
        {
          pLayer.wgs84BoundingRect = exGeoTransform.transformBoundingBox( projRect );
        }
        catch ( const QgsCsException & )
        {
          pLayer.wgs84BoundingRect = QgsRectangle( -180, -90, 180, 90 );
        }

        // Formats
        bool wmtsPngProject = project->readBoolEntry( QStringLiteral( "WMTSPngLayers" ), QStringLiteral( "Project" ) );
        if ( wmtsPngProject )
          pLayer.formats << QStringLiteral( "image/png" );
        bool wmtsJpegProject = project->readBoolEntry( QStringLiteral( "WMTSJpegLayers" ), QStringLiteral( "Project" ) );
        if ( wmtsJpegProject )
          pLayer.formats << QStringLiteral( "image/jpeg" );

        wmtsLayers.append( pLayer );
      }

      QStringList wmtsGroupNameList = project->readListEntry( QStringLiteral( "WMTSLayers" ), QStringLiteral( "Group" ) );
      if ( !wmtsGroupNameList.isEmpty() )
      {
        QgsLayerTreeGroup *treeRoot = project->layerTreeRoot();

        QStringList wmtsPngGroupNameList = project->readListEntry( QStringLiteral( "WMTSPngLayers" ), QStringLiteral( "Group" ) );
        QStringList wmtsJpegGroupNameList = project->readListEntry( QStringLiteral( "WMTSJpegLayers" ), QStringLiteral( "Group" ) );

        Q_FOREACH ( QString gName, wmtsGroupNameList )
        {
          QgsLayerTreeGroup *treeGroup = treeRoot->findGroup( gName );
          if ( !treeGroup )
          {
            continue;
          }

          layerDef pLayer;
          pLayer.id = treeGroup->customProperty( QStringLiteral( "wmsShortName" ) ).toString();
          if ( pLayer.id.isEmpty() )
            pLayer.id = gName;

          pLayer.title = treeGroup->customProperty( QStringLiteral( "wmsTitle" ) ).toString();
          if ( pLayer.title.isEmpty() )
            pLayer.title = gName;

          pLayer.abstract = treeGroup->customProperty( QStringLiteral( "wmsAbstract" ) ).toString();

          for ( QgsLayerTreeLayer *layer : treeGroup->findLayers() )
          {
            QgsMapLayer *l = layer->layer();
            //transform the layer native CRS into WGS84
            QgsRectangle wgs84BoundingRect;
            QgsCoordinateReferenceSystem layerCrs = l->crs();
            Q_NOWARN_DEPRECATED_PUSH
            QgsCoordinateTransform exGeoTransform( layerCrs, wgs84 );
            Q_NOWARN_DEPRECATED_POP
            try
            {
              wgs84BoundingRect.combineExtentWith( exGeoTransform.transformBoundingBox( l->extent() ) );
            }
            catch ( const QgsCsException & )
            {
              wgs84BoundingRect.combineExtentWith( QgsRectangle( -180, -90, 180, 90 ) );
            }
          }

          // Formats
          if ( wmtsPngGroupNameList.contains( gName ) )
            pLayer.formats << QStringLiteral( "image/png" );
          if ( wmtsJpegGroupNameList.contains( gName ) )
            pLayer.formats << QStringLiteral( "image/jpeg" );

          wmtsLayers.append( pLayer );
        }
      }

      QStringList wmtsLayerIdList = project->readListEntry( QStringLiteral( "WMTSLayers" ), QStringLiteral( "Layer" ) );
      QStringList wmtsPngLayerIdList = project->readListEntry( QStringLiteral( "WMTSPngLayers" ), QStringLiteral( "Layer" ) );
      QStringList wmtsJpegLayerIdList = project->readListEntry( QStringLiteral( "WMTSJpegLayers" ), QStringLiteral( "Layer" ) );

      Q_FOREACH ( QString lId, wmtsLayerIdList )
      {
        QgsMapLayer *l = project->mapLayer( lId );
        if ( !l )
        {
          continue;
        }
#ifdef HAVE_SERVER_PYTHON_PLUGINS
        if ( !accessControl->layerReadPermission( l ) )
        {
          continue;
        }
#endif

        layerDef pLayer;
        pLayer.id = l->name();
        if ( !l->shortName().isEmpty() )
          pLayer.id = l->shortName();
        pLayer.id = pLayer.id.replace( ' ', '_' );

        pLayer.title = l->title();
        pLayer.abstract = l->abstract();

        //transform the layer native CRS into WGS84
        QgsCoordinateReferenceSystem layerCrs = l->crs();
        Q_NOWARN_DEPRECATED_PUSH
        QgsCoordinateTransform exGeoTransform( layerCrs, wgs84 );
        Q_NOWARN_DEPRECATED_POP
        try
        {
          pLayer.wgs84BoundingRect = exGeoTransform.transformBoundingBox( l->extent() );
        }
        catch ( const QgsCsException & )
        {
          pLayer.wgs84BoundingRect = QgsRectangle( -180, -90, 180, 90 );
        }

        // Formats
        if ( wmtsPngLayerIdList.contains( lId ) )
          pLayer.formats << QStringLiteral( "image/png" );
        if ( wmtsJpegLayerIdList.contains( lId ) )
          pLayer.formats << QStringLiteral( "image/jpeg" );

        wmtsLayers.append( pLayer );
      }

      Q_FOREACH ( layerDef wmtsLayer, wmtsLayers )
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

        QDomElement wgs84BBoxElement = doc.createElement( QStringLiteral( "ows:WGS84BoundingBox" ) );
        QDomElement wgs84LowerCornerElement = doc.createElement( QStringLiteral( "LowerCorner" ) );
        QDomText wgs84LowerCornerText = doc.createTextNode( qgsDoubleToString( wmtsLayer.wgs84BoundingRect.xMinimum(), 6 ) + ' ' + qgsDoubleToString( wmtsLayer.wgs84BoundingRect.yMinimum(), 6 ) );
        wgs84LowerCornerElement.appendChild( wgs84LowerCornerText );
        wgs84BBoxElement.appendChild( wgs84LowerCornerElement );
        QDomElement wgs84UpperCornerElement = doc.createElement( QStringLiteral( "UpperCorner" ) );
        QDomText wgs84UpperCornerText = doc.createTextNode( qgsDoubleToString( wmtsLayer.wgs84BoundingRect.xMaximum(), 6 ) + ' ' + qgsDoubleToString( wmtsLayer.wgs84BoundingRect.yMaximum(), 6 ) );
        wgs84UpperCornerElement.appendChild( wgs84UpperCornerText );
        wgs84BBoxElement.appendChild( wgs84UpperCornerElement );
        layerElem.appendChild( wgs84BBoxElement );

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

        Q_FOREACH ( QString format, wmtsLayer.formats )
        {
          QDomElement layerFormatElem = doc.createElement( QStringLiteral( "Format" ) );
          QDomText layerFormatText = doc.createTextNode( format );
          layerFormatElem.appendChild( layerFormatText );
          layerElem.appendChild( layerFormatElem );
        }

        tmsIt = tmsList.begin();
        for ( ; tmsIt != tmsList.end(); ++tmsIt )
        {
          tileMatrixSet &tms = *tmsIt;

          //wmts:TileMatrixSetLink
          QDomElement tmslElement = doc.createElement( QStringLiteral( "TileMatrixSetLink" )/*wmts:TileMatrixSetLink*/ );

          QDomElement identifierElem = doc.createElement( QStringLiteral( "TileMatrixSet" ) );
          QDomText identifierText = doc.createTextNode( tms.ref );
          identifierElem.appendChild( identifierText );
          tmslElement.appendChild( identifierElem );

          layerElem.appendChild( tmslElement );
        }

        contentsElement.appendChild( layerElem );
      }

      tmsIt = tmsList.begin();
      for ( ; tmsIt != tmsList.end(); ++tmsIt )
      {
        tileMatrixSet &tms = *tmsIt;

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

        //wmts:TileMatrix
        int tmIdx = 0;
        QList<tileMatrix>::iterator tmIt = tms.tileMatrixList.begin();
        for ( ; tmIt != tms.tileMatrixList.end(); ++tmIt )
        {
          tileMatrix &tm = *tmIt;

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
          QDomText tmTopLeftCornerText = doc.createTextNode( qgsDoubleToString( tm.left, 6 ) + ' ' + qgsDoubleToString( tm.top, 6 ) );
          tmTopLeftCornerElem.appendChild( tmTopLeftCornerText );
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

    //End
    return contentsElement;
  }

} // namespace QgsWmts



