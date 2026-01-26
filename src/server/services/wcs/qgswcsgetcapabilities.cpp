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
#include "qgswcsgetcapabilities.h"

#include "qgsproject.h"
#include "qgsrasterlayer.h"
#include "qgsserverprojectutils.h"
#include "qgswcsutils.h"

namespace QgsWcs
{

  /**
   * Output WCS  GetCapabilities response
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

    //wcs:WCS_Capabilities element
    QDomElement wcsCapabilitiesElement = doc.createElement( u"WCS_Capabilities"_s /*wcs:WCS_Capabilities*/ );
    wcsCapabilitiesElement.setAttribute( u"xmlns"_s, WCS_NAMESPACE );
    wcsCapabilitiesElement.setAttribute( u"xmlns:xsi"_s, u"http://www.w3.org/2001/XMLSchema-instance"_s );
    wcsCapabilitiesElement.setAttribute( u"xsi:schemaLocation"_s, WCS_NAMESPACE + " http://schemas.opengis.net/wcs/1.0.0/wcsCapabilities.xsd" );
    wcsCapabilitiesElement.setAttribute( u"xmlns:gml"_s, GML_NAMESPACE );
    wcsCapabilitiesElement.setAttribute( u"xmlns:xlink"_s, u"http://www.w3.org/1999/xlink"_s );
    wcsCapabilitiesElement.setAttribute( u"version"_s, implementationVersion() );
    wcsCapabilitiesElement.setAttribute( u"updateSequence"_s, u"0"_s );
    doc.appendChild( wcsCapabilitiesElement );

    //INSERT Service
    wcsCapabilitiesElement.appendChild( getServiceElement( doc, project ) );

    //wcs:Capability element
    QDomElement capabilityElement = doc.createElement( u"Capability"_s /*wcs:Capability*/ );
    wcsCapabilitiesElement.appendChild( capabilityElement );

    //wcs:Request element
    QDomElement requestElement = doc.createElement( u"Request"_s /*wcs:Request*/ );
    capabilityElement.appendChild( requestElement );

    //wcs:GetCapabilities
    QDomElement getCapabilitiesElement = doc.createElement( u"GetCapabilities"_s /*wcs:GetCapabilities*/ );
    requestElement.appendChild( getCapabilitiesElement );

    QDomElement dcpTypeElement = doc.createElement( u"DCPType"_s /*wcs:DCPType*/ );
    getCapabilitiesElement.appendChild( dcpTypeElement );
    QDomElement httpElement = doc.createElement( u"HTTP"_s /*wcs:HTTP*/ );
    dcpTypeElement.appendChild( httpElement );

    //Prepare url
    const QString hrefString = serviceUrl( request, project, *serverIface->serverSettings() );

    QDomElement getElement = doc.createElement( u"Get"_s /*wcs:Get*/ );
    httpElement.appendChild( getElement );
    QDomElement onlineResourceElement = doc.createElement( u"OnlineResource"_s /*wcs:OnlineResource*/ );
    onlineResourceElement.setAttribute( u"xlink:type"_s, u"simple"_s );
    onlineResourceElement.setAttribute( u"xlink:href"_s, hrefString );
    getElement.appendChild( onlineResourceElement );

    const QDomElement getCapabilitiesDhcTypePostElement = dcpTypeElement.cloneNode().toElement(); //this is the same as for 'GetCapabilities'
    getCapabilitiesDhcTypePostElement.firstChild().firstChild().toElement().setTagName( u"Post"_s );
    getCapabilitiesElement.appendChild( getCapabilitiesDhcTypePostElement );

    QDomElement describeCoverageElement = getCapabilitiesElement.cloneNode().toElement(); //this is the same as 'GetCapabilities'
    describeCoverageElement.setTagName( u"DescribeCoverage"_s );
    requestElement.appendChild( describeCoverageElement );

    QDomElement getCoverageElement = getCapabilitiesElement.cloneNode().toElement(); //this is the same as 'GetCapabilities'
    getCoverageElement.setTagName( u"GetCoverage"_s );
    requestElement.appendChild( getCoverageElement );

    //INSERT ContentMetadata
    wcsCapabilitiesElement.appendChild( getContentMetadataElement( doc, serverIface, project ) );

    return doc;
  }

  QDomElement getServiceElement( QDomDocument &doc, const QgsProject *project )
  {
    //Service element
    QDomElement serviceElem = doc.createElement( u"Service"_s );

    //Service name
    QDomElement nameElem = doc.createElement( u"name"_s );
    const QDomText nameText = doc.createTextNode( "WCS" );
    nameElem.appendChild( nameText );
    serviceElem.appendChild( nameElem );

    const QString title = QgsServerProjectUtils::owsServiceTitle( *project );
    QDomElement titleElem = doc.createElement( u"label"_s );
    const QDomText titleText = doc.createTextNode( title );
    titleElem.appendChild( titleText );
    serviceElem.appendChild( titleElem );

    const QString abstract = QgsServerProjectUtils::owsServiceAbstract( *project );
    if ( !abstract.isEmpty() )
    {
      QDomElement abstractElem = doc.createElement( u"description"_s );
      const QDomText abstractText = doc.createCDATASection( abstract );
      abstractElem.appendChild( abstractText );
      serviceElem.appendChild( abstractElem );
    }

    const QStringList keywords = QgsServerProjectUtils::owsServiceKeywords( *project );
    if ( !keywords.isEmpty() )
    {
      QDomElement keywordsElem = doc.createElement( u"keywords"_s );
      for ( int i = 0; i < keywords.size(); ++i )
      {
        QDomElement keywordElem = doc.createElement( u"keyword"_s );
        const QDomText keywordText = doc.createTextNode( keywords.at( i ) );
        keywordElem.appendChild( keywordText );
        keywordsElem.appendChild( keywordElem );
      }
      serviceElem.appendChild( keywordsElem );
    }


    const QString contactPerson = QgsServerProjectUtils::owsServiceContactPerson( *project );
    const QString contactOrganization = QgsServerProjectUtils::owsServiceContactOrganization( *project );
    const QString contactPosition = QgsServerProjectUtils::owsServiceContactPosition( *project );
    const QString contactMail = QgsServerProjectUtils::owsServiceContactMail( *project );
    const QString contactPhone = QgsServerProjectUtils::owsServiceContactPhone( *project );
    const QString onlineResource = QgsServerProjectUtils::owsServiceOnlineResource( *project );
    if ( !contactPerson.isEmpty() || !contactOrganization.isEmpty() || !contactPosition.isEmpty() || !contactMail.isEmpty() || !contactPhone.isEmpty() || !onlineResource.isEmpty() )
    {
      QDomElement responsiblePartyElem = doc.createElement( u"responsibleParty"_s );
      if ( !contactPerson.isEmpty() )
      {
        QDomElement contactPersonElem = doc.createElement( u"individualName"_s );
        const QDomText contactPersonText = doc.createTextNode( contactPerson );
        contactPersonElem.appendChild( contactPersonText );
        responsiblePartyElem.appendChild( contactPersonElem );
      }
      if ( !contactOrganization.isEmpty() )
      {
        QDomElement contactOrganizationElem = doc.createElement( u"organisationName"_s );
        const QDomText contactOrganizationText = doc.createTextNode( contactOrganization );
        contactOrganizationElem.appendChild( contactOrganizationText );
        responsiblePartyElem.appendChild( contactOrganizationElem );
      }
      if ( !contactPosition.isEmpty() )
      {
        QDomElement contactPositionElem = doc.createElement( u"positionName"_s );
        const QDomText contactPositionText = doc.createTextNode( contactPosition );
        contactPositionElem.appendChild( contactPositionText );
        responsiblePartyElem.appendChild( contactPositionElem );
      }
      if ( !contactMail.isEmpty() || !contactPhone.isEmpty() || !onlineResource.isEmpty() )
      {
        QDomElement contactInfoElem = doc.createElement( u"contactInfo"_s );
        if ( !contactMail.isEmpty() )
        {
          QDomElement contactAddressElem = doc.createElement( u"address"_s );
          QDomElement contactAddressMailElem = doc.createElement( u"electronicMailAddress"_s );
          const QDomText contactAddressMailText = doc.createTextNode( contactMail );
          contactAddressMailElem.appendChild( contactAddressMailText );
          contactAddressElem.appendChild( contactAddressMailElem );
          contactInfoElem.appendChild( contactAddressElem );
        }
        if ( !contactPhone.isEmpty() )
        {
          QDomElement contactPhoneElem = doc.createElement( u"phone"_s );
          QDomElement contactVoiceElem = doc.createElement( u"voice"_s );
          const QDomText contactVoiceText = doc.createTextNode( contactPhone );
          contactVoiceElem.appendChild( contactVoiceText );
          contactPhoneElem.appendChild( contactVoiceElem );
          contactInfoElem.appendChild( contactPhoneElem );
        }
        if ( !onlineResource.isEmpty() )
        {
          QDomElement onlineResourceElem = doc.createElement( u"onlineResource"_s );
          onlineResourceElem.setAttribute( u"xmlns:xlink"_s, u"http://www.w3.org/1999/xlink"_s );
          onlineResourceElem.setAttribute( u"xlink:type"_s, u"simple"_s );
          onlineResourceElem.setAttribute( u"xlink:href"_s, onlineResource );
          contactInfoElem.appendChild( onlineResourceElem );
        }
        responsiblePartyElem.appendChild( contactInfoElem );
      }
      serviceElem.appendChild( responsiblePartyElem );
    }

    QDomElement feesElem = doc.createElement( u"fees"_s );
    QDomText feesText = doc.createTextNode( u"None"_s ); // default value if fees are unknown
    const QString fees = QgsServerProjectUtils::owsServiceFees( *project );
    if ( !fees.isEmpty() )
    {
      feesText = doc.createTextNode( fees );
    }
    feesElem.appendChild( feesText );
    serviceElem.appendChild( feesElem );

    QDomElement accessConstraintsElem = doc.createElement( u"accessConstraints"_s );
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

  QDomElement getContentMetadataElement( QDomDocument &doc, QgsServerInterface *serverIface, const QgsProject *project )
  {
#ifdef HAVE_SERVER_PYTHON_PLUGINS
    QgsAccessControl *accessControl = serverIface->accessControls();
#else
    ( void ) serverIface;
#endif
    /*
     * Adding layer list in ContentMetadata
     */
    QDomElement contentMetadataElement = doc.createElement( u"ContentMetadata"_s /*wcs:ContentMetadata*/ );

    const QStringList wcsLayersId = QgsServerProjectUtils::wcsLayerIds( *project );
    for ( int i = 0; i < wcsLayersId.size(); ++i )
    {
      QgsMapLayer *layer = project->mapLayer( wcsLayersId.at( i ) );
      if ( !layer )
      {
        continue;
      }
      if ( layer->type() != Qgis::LayerType::Raster )
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
      const QDomElement layerElem = getCoverageOffering( doc, const_cast<QgsRasterLayer *>( rLayer ), project, true );

      contentMetadataElement.appendChild( layerElem );
    }

    //End
    return contentMetadataElement;
  }

} // namespace QgsWcs
