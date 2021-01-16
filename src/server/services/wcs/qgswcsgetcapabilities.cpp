/***************************************************************************
                              qgswcsgecapabilities.cpp
                              -------------------------
  begin                : January 16 , 2017
  copyright            : (C) 2013 by René-Luc D'Hont  ( parts from qgswcsserver )
                         (C) 2017 by David Marteau
  email                : rldhont at 3liz dot com
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
#include "qgswcsutils.h"
#include "qgsserverprojectutils.h"
#include "qgswcsgetcapabilities.h"

#include "qgsproject.h"
#include "qgsrasterlayer.h"

namespace QgsWcs
{

  /**
   * Output WCS  GetCapabilities response
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

    //wcs:WCS_Capabilities element
    QDomElement wcsCapabilitiesElement = doc.createElement( QStringLiteral( "WCS_Capabilities" )/*wcs:WCS_Capabilities*/ );
    wcsCapabilitiesElement.setAttribute( QStringLiteral( "xmlns" ), WCS_NAMESPACE );
    wcsCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:xsi" ), QStringLiteral( "http://www.w3.org/2001/XMLSchema-instance" ) );
    wcsCapabilitiesElement.setAttribute( QStringLiteral( "xsi:schemaLocation" ), WCS_NAMESPACE + " http://schemas.opengis.net/wcs/1.0.0/wcsCapabilities.xsd" );
    wcsCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:gml" ), GML_NAMESPACE );
    wcsCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:xlink" ), QStringLiteral( "http://www.w3.org/1999/xlink" ) );
    wcsCapabilitiesElement.setAttribute( QStringLiteral( "version" ), implementationVersion() );
    wcsCapabilitiesElement.setAttribute( QStringLiteral( "updateSequence" ), QStringLiteral( "0" ) );
    doc.appendChild( wcsCapabilitiesElement );

    //INSERT Service
    wcsCapabilitiesElement.appendChild( getServiceElement( doc, project ) );

    //wcs:Capability element
    QDomElement capabilityElement = doc.createElement( QStringLiteral( "Capability" )/*wcs:Capability*/ );
    wcsCapabilitiesElement.appendChild( capabilityElement );

    //wcs:Request element
    QDomElement requestElement = doc.createElement( QStringLiteral( "Request" )/*wcs:Request*/ );
    capabilityElement.appendChild( requestElement );

    //wcs:GetCapabilities
    QDomElement getCapabilitiesElement = doc.createElement( QStringLiteral( "GetCapabilities" )/*wcs:GetCapabilities*/ );
    requestElement.appendChild( getCapabilitiesElement );

    QDomElement dcpTypeElement = doc.createElement( QStringLiteral( "DCPType" )/*wcs:DCPType*/ );
    getCapabilitiesElement.appendChild( dcpTypeElement );
    QDomElement httpElement = doc.createElement( QStringLiteral( "HTTP" )/*wcs:HTTP*/ );
    dcpTypeElement.appendChild( httpElement );

    //Prepare url
    QString hrefString = serviceUrl( request, project, *serverIface->serverSettings() );

    QDomElement getElement = doc.createElement( QStringLiteral( "Get" )/*wcs:Get*/ );
    httpElement.appendChild( getElement );
    QDomElement onlineResourceElement = doc.createElement( QStringLiteral( "OnlineResource" )/*wcs:OnlineResource*/ );
    onlineResourceElement.setAttribute( QStringLiteral( "xlink:type" ), QStringLiteral( "simple" ) );
    onlineResourceElement.setAttribute( QStringLiteral( "xlink:href" ), hrefString );
    getElement.appendChild( onlineResourceElement );

    QDomElement getCapabilitiesDhcTypePostElement = dcpTypeElement.cloneNode().toElement();//this is the same as for 'GetCapabilities'
    getCapabilitiesDhcTypePostElement.firstChild().firstChild().toElement().setTagName( QStringLiteral( "Post" ) );
    getCapabilitiesElement.appendChild( getCapabilitiesDhcTypePostElement );

    QDomElement describeCoverageElement = getCapabilitiesElement.cloneNode().toElement();//this is the same as 'GetCapabilities'
    describeCoverageElement.setTagName( QStringLiteral( "DescribeCoverage" ) );
    requestElement.appendChild( describeCoverageElement );

    QDomElement getCoverageElement = getCapabilitiesElement.cloneNode().toElement();//this is the same as 'GetCapabilities'
    getCoverageElement.setTagName( QStringLiteral( "GetCoverage" ) );
    requestElement.appendChild( getCoverageElement );

    //INSERT ContentMetadata
    wcsCapabilitiesElement.appendChild( getContentMetadataElement( doc, serverIface, project ) );

    return doc;

  }

  QDomElement getServiceElement( QDomDocument &doc, const QgsProject *project )
  {
    //Service element
    QDomElement serviceElem = doc.createElement( QStringLiteral( "Service" ) );

    //Service name
    QDomElement nameElem = doc.createElement( QStringLiteral( "name" ) );
    QDomText nameText = doc.createTextNode( "WCS" );
    nameElem.appendChild( nameText );
    serviceElem.appendChild( nameElem );

    QString title = QgsServerProjectUtils::owsServiceTitle( *project );
    if ( !title.isEmpty() )
    {
      QDomElement titleElem = doc.createElement( QStringLiteral( "label" ) );
      QDomText titleText = doc.createTextNode( title );
      titleElem.appendChild( titleText );
      serviceElem.appendChild( titleElem );
    }

    QString abstract = QgsServerProjectUtils::owsServiceAbstract( *project );
    if ( !abstract.isEmpty() )
    {
      QDomElement abstractElem = doc.createElement( QStringLiteral( "description" ) );
      QDomText abstractText = doc.createCDATASection( abstract );
      abstractElem.appendChild( abstractText );
      serviceElem.appendChild( abstractElem );
    }

    QStringList keywords = QgsServerProjectUtils::owsServiceKeywords( *project );
    if ( !keywords.isEmpty() )
    {
      QDomElement keywordsElem = doc.createElement( QStringLiteral( "keywords" ) );
      for ( int i = 0; i < keywords.size(); ++i )
      {
        QDomElement keywordElem = doc.createElement( QStringLiteral( "keyword" ) );
        QDomText keywordText = doc.createTextNode( keywords.at( i ) );
        keywordElem.appendChild( keywordText );
        keywordsElem.appendChild( keywordElem );
      }
      serviceElem.appendChild( keywordsElem );
    }


    QString contactPerson = QgsServerProjectUtils::owsServiceContactPerson( *project );
    QString contactOrganization = QgsServerProjectUtils::owsServiceContactOrganization( *project );
    QString contactPosition = QgsServerProjectUtils::owsServiceContactPosition( *project );
    QString contactMail = QgsServerProjectUtils::owsServiceContactMail( *project );
    QString contactPhone = QgsServerProjectUtils::owsServiceContactPhone( *project );
    QString onlineResource = QgsServerProjectUtils::owsServiceOnlineResource( *project );
    if ( !contactPerson.isEmpty() ||
         !contactOrganization.isEmpty() ||
         !contactPosition.isEmpty() ||
         !contactMail.isEmpty() ||
         !contactPhone.isEmpty() ||
         !onlineResource.isEmpty() )
    {
      QDomElement responsiblePartyElem = doc.createElement( QStringLiteral( "responsibleParty" ) );
      if ( !contactPerson.isEmpty() )
      {
        QDomElement contactPersonElem = doc.createElement( QStringLiteral( "individualName" ) );
        QDomText contactPersonText = doc.createTextNode( contactPerson );
        contactPersonElem.appendChild( contactPersonText );
        responsiblePartyElem.appendChild( contactPersonElem );
      }
      if ( !contactOrganization.isEmpty() )
      {
        QDomElement contactOrganizationElem = doc.createElement( QStringLiteral( "organisationName" ) );
        QDomText contactOrganizationText = doc.createTextNode( contactOrganization );
        contactOrganizationElem.appendChild( contactOrganizationText );
        responsiblePartyElem.appendChild( contactOrganizationElem );
      }
      if ( !contactPosition.isEmpty() )
      {
        QDomElement contactPositionElem = doc.createElement( QStringLiteral( "positionName" ) );
        QDomText contactPositionText = doc.createTextNode( contactPosition );
        contactPositionElem.appendChild( contactPositionText );
        responsiblePartyElem.appendChild( contactPositionElem );
      }
      if ( !contactMail.isEmpty() ||
           !contactPhone.isEmpty() ||
           !onlineResource.isEmpty() )
      {
        QDomElement contactInfoElem = doc.createElement( QStringLiteral( "contactInfo" ) );
        if ( !contactMail.isEmpty() )
        {
          QDomElement contactAddressElem = doc.createElement( QStringLiteral( "address" ) );
          QDomElement contactAddressMailElem = doc.createElement( QStringLiteral( "electronicMailAddress" ) );
          QDomText contactAddressMailText = doc.createTextNode( contactMail );
          contactAddressMailElem.appendChild( contactAddressMailText );
          contactAddressElem.appendChild( contactAddressMailElem );
          contactInfoElem.appendChild( contactAddressElem );
        }
        if ( !contactPhone.isEmpty() )
        {
          QDomElement contactPhoneElem = doc.createElement( QStringLiteral( "phone" ) );
          QDomElement contactVoiceElem = doc.createElement( QStringLiteral( "voice" ) );
          QDomText contactVoiceText = doc.createTextNode( contactPhone );
          contactVoiceElem.appendChild( contactVoiceText );
          contactPhoneElem.appendChild( contactVoiceElem );
          contactInfoElem.appendChild( contactPhoneElem );
        }
        if ( !onlineResource.isEmpty() )
        {
          QDomElement onlineResourceElem = doc.createElement( QStringLiteral( "onlineResource" ) );
          onlineResourceElem.setAttribute( QStringLiteral( "xmlns:xlink" ), QStringLiteral( "http://www.w3.org/1999/xlink" ) );
          onlineResourceElem.setAttribute( QStringLiteral( "xlink:type" ), QStringLiteral( "simple" ) );
          onlineResourceElem.setAttribute( QStringLiteral( "xlink:href" ), onlineResource );
          contactInfoElem.appendChild( onlineResourceElem );
        }
        responsiblePartyElem.appendChild( contactInfoElem );
      }
      serviceElem.appendChild( responsiblePartyElem );
    }

    QDomElement feesElem = doc.createElement( QStringLiteral( "fees" ) );
    QDomText feesText = doc.createTextNode( QStringLiteral( "None" ) ); // default value if fees are unknown
    QString fees = QgsServerProjectUtils::owsServiceFees( *project );
    if ( !fees.isEmpty() )
    {
      feesText = doc.createTextNode( fees );
    }
    feesElem.appendChild( feesText );
    serviceElem.appendChild( feesElem );

    QDomElement accessConstraintsElem = doc.createElement( QStringLiteral( "accessConstraints" ) );
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

  QDomElement getContentMetadataElement( QDomDocument &doc, QgsServerInterface *serverIface, const QgsProject *project )
  {
#ifdef HAVE_SERVER_PYTHON_PLUGINS
    QgsAccessControl *accessControl = serverIface->accessControls();
#else
    ( void )serverIface;
#endif
    /*
     * Adding layer list in ContentMetadata
     */
    QDomElement contentMetadataElement = doc.createElement( QStringLiteral( "ContentMetadata" )/*wcs:ContentMetadata*/ );

    QStringList wcsLayersId = QgsServerProjectUtils::wcsLayerIds( *project );
    for ( int i = 0; i < wcsLayersId.size(); ++i )
    {
      QgsMapLayer *layer = project->mapLayer( wcsLayersId.at( i ) );
      if ( !layer )
      {
        continue;
      }
      if ( layer->type() != QgsMapLayerType::RasterLayer )
      {
        continue;
      }
#ifdef HAVE_SERVER_PYTHON_PLUGINS
      if ( !accessControl->layerReadPermission( layer ) )
      {
        continue;
      }
#endif

      QgsRasterLayer *rLayer = qobject_cast<QgsRasterLayer *>( layer );
      QDomElement layerElem = getCoverageOffering( doc, const_cast<QgsRasterLayer *>( rLayer ), project, true );

      contentMetadataElement.appendChild( layerElem );
    }

    //End
    return contentMetadataElement;
  }

} // namespace QgsWcs



