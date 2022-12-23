/***************************************************************************
                              qgswmsgetmap.h
                              -------------------------
  begin                : December 20 , 2016
  copyright            : (C) 2007 by Marco Hugentobler  (original code)
                         (C) 2014 by Alessandro Pasotti (original code)
                         (C) 2016 by David Marteau
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
#include "qgswmsutils.h"
#include "qgswmsgetcapabilities.h"
#include "qgsserverprojectutils.h"

#include "qgslayoutmanager.h"
#include "qgslayoutatlas.h"
#include "qgsprintlayout.h"
#include "qgslayoutitemmap.h"
#include "qgslayoutitemlabel.h"
#include "qgslayoutitemhtml.h"
#include "qgslayoutframe.h"
#include "qgslayoutpagecollection.h"

#include "qgsmaplayerstylemanager.h"
#include "qgsmaplayertemporalproperties.h"

#include "qgsexception.h"
#include "qgsexpressionnodeimpl.h"
#include "qgsvectorlayer.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterlayer.h"
#include "qgsrasterrenderer.h"
#include "qgsmaplayerserverproperties.h"

namespace QgsWms
{
  namespace
  {

    void appendLayerProjectSettings( QDomDocument &doc, QDomElement &layerElem, QgsMapLayer *currentLayer );

    void appendDrawingOrder( QDomDocument &doc, QDomElement &parentElem, QgsServerInterface *serverIface,
                             const QgsProject *project );

    void appendLayerWgs84BoundingRect( QDomDocument &doc, QDomElement &layerElement, const QgsRectangle &wgs84BoundingRect );

    void appendLayerCrsExtents( QDomDocument &doc, QDomElement &layerElement, const QMap<QString, QgsRectangle> &crsExtents );

    void appendCrsElementToLayer( QDomDocument &doc, QDomElement &layerElement, const QDomElement &precedingElement,
                                  const QString &crsText );

    void appendCrsElementsToLayer( QDomDocument &doc, QDomElement &layerElement,
                                   const QStringList &crsList, const QStringList &constrainedCrsList );

    void appendLayerStyles( QDomDocument &doc, QDomElement &layerElem, const QgsWmsLayerInfos &layerInfos,
                            const QgsProject *project, const QgsWmsRequest &request, const QgsServerSettings *settings );

    void appendLayersFromTreeGroup( QDomDocument &doc,
                                    QDomElement &parentLayer,
                                    QgsServerInterface *serverIface,
                                    const QgsProject *project,
                                    const QgsWmsRequest &request,
                                    const QgsLayerTreeGroup *layerTreeGroup,
                                    const QMap< QString, QgsWmsLayerInfos > &wmsLayerInfos,
                                    bool projectSettings );

    void addKeywordListElement( const QgsProject *project, QDomDocument &doc, QDomElement &parent );
  }

  void writeGetCapabilities( QgsServerInterface *serverIface,
                             const QgsProject *project,
                             const QgsWmsRequest &request,
                             QgsServerResponse &response,
                             bool projectSettings )
  {
#ifdef HAVE_SERVER_PYTHON_PLUGINS
    QgsAccessControl *accessControl = serverIface->accessControls();
#endif

    QDomDocument doc;
    const QDomDocument *capabilitiesDocument = nullptr;

    // Data for WMS capabilities server memory cache
    QString configFilePath = serverIface->configFilePath();
    QgsCapabilitiesCache *capabilitiesCache = serverIface->capabilitiesCache();
    QStringList cacheKeyList;
    cacheKeyList << ( projectSettings ? QStringLiteral( "projectSettings" ) : request.wmsParameters().version() );
    cacheKeyList << QgsServerProjectUtils::serviceUrl( request.serverParameters().service(), request, *serverIface->serverSettings() );
    bool cache = true;

#ifdef HAVE_SERVER_PYTHON_PLUGINS
    if ( accessControl )
      cache = accessControl->fillCacheKey( cacheKeyList );
#endif
    QString cacheKey = cacheKeyList.join( '-' );

#ifdef HAVE_SERVER_PYTHON_PLUGINS
    QgsServerCacheManager *cacheManager = serverIface->cacheManager();
    if ( cacheManager && cacheManager->getCachedDocument( &doc, project, request, accessControl ) )
    {
      capabilitiesDocument = &doc;
    }
#endif
    if ( !capabilitiesDocument && cache ) //capabilities xml not in cache plugins
    {
      capabilitiesDocument = capabilitiesCache->searchCapabilitiesDocument( configFilePath, cacheKey );
    }

    if ( !capabilitiesDocument ) //capabilities xml not in cache. Create a new one
    {
      QgsMessageLog::logMessage( QStringLiteral( "WMS capabilities document not found in cache" ), QStringLiteral( "Server" ) );

      doc = getCapabilities( serverIface, project, request, projectSettings );

#ifdef HAVE_SERVER_PYTHON_PLUGINS
      if ( cacheManager &&
           cacheManager->setCachedDocument( &doc, project, request, accessControl ) )
      {
        capabilitiesDocument = &doc;
      }
#endif

      // cppcheck-suppress identicalInnerCondition
      if ( !capabilitiesDocument )
      {
        capabilitiesCache->insertCapabilitiesDocument( configFilePath, cacheKey, &doc );
        capabilitiesDocument = capabilitiesCache->searchCapabilitiesDocument( configFilePath, cacheKey );
      }
      if ( !capabilitiesDocument )
      {
        capabilitiesDocument = &doc;
      }
      else
      {
        QgsMessageLog::logMessage( QStringLiteral( "Set WMS capabilities document in cache" ), QStringLiteral( "Server" ) );
      }
    }
    else
    {
      QgsMessageLog::logMessage( QStringLiteral( "Found WMS capabilities document in cache" ), QStringLiteral( "Server" ) );
    }

    response.setHeader( QStringLiteral( "Content-Type" ), QStringLiteral( "text/xml; charset=utf-8" ) );
    response.write( capabilitiesDocument->toByteArray() );
  }

  QDomDocument getCapabilities( QgsServerInterface *serverIface, const QgsProject *project,
                                const QgsWmsRequest &request,
                                bool projectSettings )
  {
    QDomDocument doc;
    QDomElement wmsCapabilitiesElement;

    // Get service URL
    QUrl href = serviceUrl( request, project, *serverIface->serverSettings() );

    //href needs to be a prefix
    QString hrefString = href.toString();
    hrefString.append( href.hasQuery() ? "&" : "?" );

    // XML declaration
    QDomProcessingInstruction xmlDeclaration = doc.createProcessingInstruction( QStringLiteral( "xml" ),
        QStringLiteral( "version=\"1.0\" encoding=\"utf-8\"" ) );

    // Append format helper
    std::function < void ( QDomElement &, const QString & ) > appendFormat = [&doc]( QDomElement & elem, const QString & format )
    {
      QDomElement formatElem = doc.createElement( QStringLiteral( "Format" )/*wms:Format*/ );
      formatElem.appendChild( doc.createTextNode( format ) );
      elem.appendChild( formatElem );
    };

    if ( request.wmsParameters().version() == QLatin1String( "1.1.1" ) )
    {
      doc = QDomDocument( QStringLiteral( "WMT_MS_Capabilities SYSTEM 'http://schemas.opengis.net/wms/1.1.1/WMS_MS_Capabilities.dtd'" ) );  //WMS 1.1.1 needs DOCTYPE  "SYSTEM http://schemas.opengis.net/wms/1.1.1/WMS_MS_Capabilities.dtd"
      doc.appendChild( xmlDeclaration );
      wmsCapabilitiesElement = doc.createElement( QStringLiteral( "WMT_MS_Capabilities" )/*wms:WMS_Capabilities*/ );
    }
    else // 1.3.0 as default
    {
      doc.appendChild( xmlDeclaration );
      wmsCapabilitiesElement = doc.createElement( QStringLiteral( "WMS_Capabilities" )/*wms:WMS_Capabilities*/ );
      wmsCapabilitiesElement.setAttribute( QStringLiteral( "xmlns" ), QStringLiteral( "http://www.opengis.net/wms" ) );
      wmsCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:sld" ), QStringLiteral( "http://www.opengis.net/sld" ) );
      wmsCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:qgs" ), QStringLiteral( "http://www.qgis.org/wms" ) );
      wmsCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:xsi" ), QStringLiteral( "http://www.w3.org/2001/XMLSchema-instance" ) );
      QString schemaLocation = QStringLiteral( "http://www.opengis.net/wms" );
      schemaLocation += QLatin1String( " http://schemas.opengis.net/wms/1.3.0/capabilities_1_3_0.xsd" );
      schemaLocation += QLatin1String( " http://www.opengis.net/sld" );
      schemaLocation += QLatin1String( " http://schemas.opengis.net/sld/1.1.0/sld_capabilities.xsd" );

      if ( QgsServerProjectUtils::wmsInspireActivate( *project ) )
      {
        wmsCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:inspire_common" ), QStringLiteral( "http://inspire.ec.europa.eu/schemas/common/1.0" ) );
        wmsCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:inspire_vs" ), QStringLiteral( "http://inspire.ec.europa.eu/schemas/inspire_vs/1.0" ) );
        schemaLocation += QLatin1String( " http://inspire.ec.europa.eu/schemas/inspire_vs/1.0" );
        schemaLocation += QLatin1String( " http://inspire.ec.europa.eu/schemas/inspire_vs/1.0/inspire_vs.xsd" );
      }

      schemaLocation += QLatin1String( " http://www.qgis.org/wms" );
      schemaLocation += " " + hrefString + "SERVICE=WMS&REQUEST=GetSchemaExtension";

      wmsCapabilitiesElement.setAttribute( QStringLiteral( "xsi:schemaLocation" ), schemaLocation );
    }
    wmsCapabilitiesElement.setAttribute( QStringLiteral( "version" ), request.wmsParameters().version() );
    doc.appendChild( wmsCapabilitiesElement );

    //INSERT Service
    wmsCapabilitiesElement.appendChild( getServiceElement( doc, project, request, serverIface->serverSettings() ) );

    //wms:Capability element
    QDomElement capabilityElement = getCapabilityElement( doc, project, request, projectSettings, serverIface );
    wmsCapabilitiesElement.appendChild( capabilityElement );

    if ( projectSettings )
    {
      //Insert <ComposerTemplate> elements derived from wms:_ExtendedCapabilities
      capabilityElement.appendChild( getComposerTemplatesElement( doc, project ) );

      //WFS layers
      capabilityElement.appendChild( getWFSLayersElement( doc, project ) );
    }

    capabilityElement.appendChild(
      getLayersAndStylesCapabilitiesElement( doc, serverIface, project, request, projectSettings )
    );

    if ( projectSettings )
    {
      appendDrawingOrder( doc, capabilityElement, serverIface, project );
    }

    return doc;
  }

  QDomElement getServiceElement( QDomDocument &doc, const QgsProject *project,
                                 const QgsWmsRequest &request, const QgsServerSettings *serverSettings )
  {
    //Service element
    QDomElement serviceElem = doc.createElement( QStringLiteral( "Service" ) );

    //Service name
    QDomElement nameElem = doc.createElement( QStringLiteral( "Name" ) );
    QDomText nameText = doc.createTextNode( QStringLiteral( "WMS" ) );
    nameElem.appendChild( nameText );
    serviceElem.appendChild( nameElem );

    // Service title
    QDomElement titleElem = doc.createElement( QStringLiteral( "Title" ) );
    QDomText titleText = doc.createTextNode( QgsServerProjectUtils::owsServiceTitle( *project ) );
    titleElem.appendChild( titleText );
    serviceElem.appendChild( titleElem );

    QString abstract = QgsServerProjectUtils::owsServiceAbstract( *project );
    if ( !abstract.isEmpty() )
    {
      QDomElement abstractElem = doc.createElement( QStringLiteral( "Abstract" ) );
      QDomText abstractText = doc.createCDATASection( abstract );
      abstractElem.appendChild( abstractText );
      serviceElem.appendChild( abstractElem );
    }

    addKeywordListElement( project, doc, serviceElem );

    QString onlineResource = QgsServerProjectUtils::owsServiceOnlineResource( *project );
    if ( onlineResource.isEmpty() )
    {
      onlineResource = serviceUrl( request, project, *serverSettings ).toString();
    }
    QDomElement onlineResourceElem = doc.createElement( QStringLiteral( "OnlineResource" ) );
    onlineResourceElem.setAttribute( QStringLiteral( "xmlns:xlink" ), QStringLiteral( "http://www.w3.org/1999/xlink" ) );
    onlineResourceElem.setAttribute( QStringLiteral( "xlink:type" ), QStringLiteral( "simple" ) );
    onlineResourceElem.setAttribute( QStringLiteral( "xlink:href" ), onlineResource );
    serviceElem.appendChild( onlineResourceElem );

    QString contactPerson = QgsServerProjectUtils::owsServiceContactPerson( *project );
    QString contactOrganization = QgsServerProjectUtils::owsServiceContactOrganization( *project );
    QString contactPosition = QgsServerProjectUtils::owsServiceContactPosition( *project );
    QString contactMail = QgsServerProjectUtils::owsServiceContactMail( *project );
    QString contactPhone = QgsServerProjectUtils::owsServiceContactPhone( *project );
    if ( !contactPerson.isEmpty() ||
         !contactOrganization.isEmpty() ||
         !contactPosition.isEmpty() ||
         !contactMail.isEmpty() ||
         !contactPhone.isEmpty() )
    {
      //Contact information
      QDomElement contactInfoElem = doc.createElement( QStringLiteral( "ContactInformation" ) );

      //Contact person primary
      if ( !contactPerson.isEmpty() ||
           !contactOrganization.isEmpty() )
      {
        QDomElement contactPersonPrimaryElem = doc.createElement( QStringLiteral( "ContactPersonPrimary" ) );

        QDomText contactPersonText;
        if ( !contactPerson.isEmpty() )
        {
          contactPersonText = doc.createTextNode( contactPerson );
        }
        else
        {
          contactPersonText = doc.createTextNode( QStringLiteral( "unknown" ) );
        }
        QDomElement contactPersonElem = doc.createElement( QStringLiteral( "ContactPerson" ) );
        contactPersonElem.appendChild( contactPersonText );
        contactPersonPrimaryElem.appendChild( contactPersonElem );

        QDomText contactOrganizationText;
        if ( !contactOrganization.isEmpty() )
        {
          contactOrganizationText = doc.createTextNode( contactOrganization );
        }
        else
        {
          contactOrganizationText = doc.createTextNode( QStringLiteral( "unknown" ) );
        }
        QDomElement contactOrganizationElem = doc.createElement( QStringLiteral( "ContactOrganization" ) );
        contactOrganizationElem.appendChild( contactOrganizationText );
        contactPersonPrimaryElem.appendChild( contactOrganizationElem );

        contactInfoElem.appendChild( contactPersonPrimaryElem );
      }

      if ( !contactPosition.isEmpty() )
      {
        QDomElement contactPositionElem = doc.createElement( QStringLiteral( "ContactPosition" ) );
        QDomText contactPositionText = doc.createTextNode( contactPosition );
        contactPositionElem.appendChild( contactPositionText );
        contactInfoElem.appendChild( contactPositionElem );
      }

      if ( !contactPhone.isEmpty() )
      {
        QDomElement phoneElem = doc.createElement( QStringLiteral( "ContactVoiceTelephone" ) );
        QDomText phoneText = doc.createTextNode( contactPhone );
        phoneElem.appendChild( phoneText );
        contactInfoElem.appendChild( phoneElem );
      }

      if ( !contactMail.isEmpty() )
      {
        QDomElement mailElem = doc.createElement( QStringLiteral( "ContactElectronicMailAddress" ) );
        QDomText mailText = doc.createTextNode( contactMail );
        mailElem.appendChild( mailText );
        contactInfoElem.appendChild( mailElem );
      }

      serviceElem.appendChild( contactInfoElem );
    }

    QDomElement feesElem = doc.createElement( QStringLiteral( "Fees" ) );
    QDomText feesText = doc.createTextNode( QStringLiteral( "None" ) ); // default value if fees are unknown
    QString fees = QgsServerProjectUtils::owsServiceFees( *project );
    if ( !fees.isEmpty() )
    {
      feesText = doc.createTextNode( fees );
    }
    feesElem.appendChild( feesText );
    serviceElem.appendChild( feesElem );

    QDomElement accessConstraintsElem = doc.createElement( QStringLiteral( "AccessConstraints" ) );
    QDomText accessConstraintsText = doc.createTextNode( QStringLiteral( "None" ) ); // default value if access constraints are unknown
    QString accessConstraints = QgsServerProjectUtils::owsServiceAccessConstraints( *project );
    if ( !accessConstraints.isEmpty() )
    {
      accessConstraintsText = doc.createTextNode( accessConstraints );
    }
    accessConstraintsElem.appendChild( accessConstraintsText );
    serviceElem.appendChild( accessConstraintsElem );

    if ( request.wmsParameters().version() == QLatin1String( "1.3.0" ) )
    {
      int maxWidth = QgsServerProjectUtils::wmsMaxWidth( *project );
      if ( maxWidth > 0 )
      {
        QDomElement maxWidthElem = doc.createElement( QStringLiteral( "MaxWidth" ) );
        QDomText maxWidthText = doc.createTextNode( QString::number( maxWidth ) );
        maxWidthElem.appendChild( maxWidthText );
        serviceElem.appendChild( maxWidthElem );
      }

      int maxHeight = QgsServerProjectUtils::wmsMaxHeight( *project );
      if ( maxHeight > 0 )
      {
        QDomElement maxHeightElem = doc.createElement( QStringLiteral( "MaxHeight" ) );
        QDomText maxHeightText = doc.createTextNode( QString::number( maxHeight ) );
        maxHeightElem.appendChild( maxHeightText );
        serviceElem.appendChild( maxHeightElem );
      }
    }

    return serviceElem;
  }

  QDomElement getCapabilityElement( QDomDocument &doc, const QgsProject *project,
                                    const QgsWmsRequest &request,
                                    bool projectSettings, QgsServerInterface *serverIface )
  {
    const QString version = request.wmsParameters().version();

    // Get service URL
    QUrl href = serviceUrl( request, project, *serverIface->serverSettings() );

    //href needs to be a prefix
    QString hrefString = href.toString();
    hrefString.append( href.hasQuery() ? "&" : "?" );

    QDomElement capabilityElem = doc.createElement( QStringLiteral( "Capability" )/*wms:Capability*/ );

    //wms:Request element
    QDomElement requestElem = doc.createElement( QStringLiteral( "Request" )/*wms:Request*/ );
    capabilityElem.appendChild( requestElem );

    QDomElement dcpTypeElem = doc.createElement( QStringLiteral( "DCPType" )/*wms:DCPType*/ );
    QDomElement httpElem = doc.createElement( QStringLiteral( "HTTP" )/*wms:HTTP*/ );
    dcpTypeElem.appendChild( httpElem );

    // Append format helper
    std::function < void ( QDomElement &, const QString & ) > appendFormat = [&doc]( QDomElement & elem, const QString & format )
    {
      QDomElement formatElem = doc.createElement( QStringLiteral( "Format" )/*wms:Format*/ );
      formatElem.appendChild( doc.createTextNode( format ) );
      elem.appendChild( formatElem );
    };

    QDomElement elem;

    //wms:GetCapabilities
    elem = doc.createElement( QStringLiteral( "GetCapabilities" )/*wms:GetCapabilities*/ );
    appendFormat( elem, ( version == QLatin1String( "1.1.1" ) ? "application/vnd.ogc.wms_xml" : "text/xml" ) );
    elem.appendChild( dcpTypeElem );
    requestElem.appendChild( elem );

    //only Get supported for the moment
    QDomElement getElem = doc.createElement( QStringLiteral( "Get" )/*wms:Get*/ );
    httpElem.appendChild( getElem );
    QDomElement olResourceElem = doc.createElement( QStringLiteral( "OnlineResource" )/*wms:OnlineResource*/ );
    olResourceElem.setAttribute( QStringLiteral( "xmlns:xlink" ), QStringLiteral( "http://www.w3.org/1999/xlink" ) );
    olResourceElem.setAttribute( QStringLiteral( "xlink:type" ), QStringLiteral( "simple" ) );
    olResourceElem.setAttribute( QStringLiteral( "xlink:href" ), hrefString );
    getElem.appendChild( olResourceElem );

    //wms:GetMap
    elem = doc.createElement( QStringLiteral( "GetMap" )/*wms:GetMap*/ );
    appendFormat( elem, QStringLiteral( "image/jpeg" ) );
    appendFormat( elem, QStringLiteral( "image/png" ) );
    appendFormat( elem, QStringLiteral( "image/png; mode=16bit" ) );
    appendFormat( elem, QStringLiteral( "image/png; mode=8bit" ) );
    appendFormat( elem, QStringLiteral( "image/png; mode=1bit" ) );
    appendFormat( elem, QStringLiteral( "application/dxf" ) );
    elem.appendChild( dcpTypeElem.cloneNode().toElement() ); //this is the same as for 'GetCapabilities'
    requestElem.appendChild( elem );

    //wms:GetFeatureInfo
    elem = doc.createElement( QStringLiteral( "GetFeatureInfo" ) );
    appendFormat( elem, QStringLiteral( "text/plain" ) );
    appendFormat( elem, QStringLiteral( "text/html" ) );
    appendFormat( elem, QStringLiteral( "text/xml" ) );
    appendFormat( elem, QStringLiteral( "application/vnd.ogc.gml" ) );
    appendFormat( elem, QStringLiteral( "application/vnd.ogc.gml/3.1.1" ) );
    appendFormat( elem, QStringLiteral( "application/json" ) );
    appendFormat( elem, QStringLiteral( "application/geo+json" ) );
    elem.appendChild( dcpTypeElem.cloneNode().toElement() ); //this is the same as for 'GetCapabilities'
    requestElem.appendChild( elem );

    //wms:GetLegendGraphic
    elem = doc.createElement( ( version == QLatin1String( "1.1.1" ) ? "GetLegendGraphic" : "sld:GetLegendGraphic" )/*wms:GetLegendGraphic*/ );
    appendFormat( elem, QStringLiteral( "image/jpeg" ) );
    appendFormat( elem, QStringLiteral( "image/png" ) );
    appendFormat( elem, QStringLiteral( "application/json" ) );
    elem.appendChild( dcpTypeElem.cloneNode().toElement() ); //this is the same as for 'GetCapabilities'
    requestElem.appendChild( elem );

    //wms:DescribeLayer
    elem = doc.createElement( ( version == QLatin1String( "1.1.1" ) ? "DescribeLayer" : "sld:DescribeLayer" )/*wms:GetLegendGraphic*/ );
    appendFormat( elem, QStringLiteral( "text/xml" ) );
    elem.appendChild( dcpTypeElem.cloneNode().toElement() ); //this is the same as for 'GetCapabilities'
    requestElem.appendChild( elem );

    //wms:GetStyles
    elem = doc.createElement( ( version == QLatin1String( "1.1.1" ) ? "GetStyles" : "qgs:GetStyles" )/*wms:GetStyles*/ );
    appendFormat( elem, QStringLiteral( "text/xml" ) );
    elem.appendChild( dcpTypeElem.cloneNode().toElement() ); //this is the same as for 'GetCapabilities'
    requestElem.appendChild( elem );

    if ( ( !serverIface->serverSettings() || !serverIface->serverSettings()->getPrintDisabled() ) &&
         projectSettings ) //remove composer templates from GetCapabilities in the long term
    {
      //wms:GetPrint
      elem = doc.createElement( QStringLiteral( "GetPrint" ) /*wms:GetPrint*/ );
      appendFormat( elem, QStringLiteral( "svg" ) );
      appendFormat( elem, QStringLiteral( "png" ) );
      appendFormat( elem, QStringLiteral( "pdf" ) );
      elem.appendChild( dcpTypeElem.cloneNode().toElement() ); //this is the same as for 'GetCapabilities'
      requestElem.appendChild( elem );
    }

    //Exception element is mandatory
    elem = doc.createElement( QStringLiteral( "Exception" ) );
    appendFormat( elem, ( version == QLatin1String( "1.1.1" ) ? "application/vnd.ogc.se_xml" : "XML" ) );
    capabilityElem.appendChild( elem );

    //UserDefinedSymbolization element
    if ( version == QLatin1String( "1.3.0" ) )
    {
      elem = doc.createElement( QStringLiteral( "sld:UserDefinedSymbolization" ) );
      elem.setAttribute( QStringLiteral( "SupportSLD" ), QStringLiteral( "1" ) );
      elem.setAttribute( QStringLiteral( "UserLayer" ), QStringLiteral( "0" ) );
      elem.setAttribute( QStringLiteral( "UserStyle" ), QStringLiteral( "1" ) );
      elem.setAttribute( QStringLiteral( "RemoteWFS" ), QStringLiteral( "0" ) );
      elem.setAttribute( QStringLiteral( "InlineFeature" ), QStringLiteral( "0" ) );
      elem.setAttribute( QStringLiteral( "RemoteWCS" ), QStringLiteral( "0" ) );
      capabilityElem.appendChild( elem );

      if ( QgsServerProjectUtils::wmsInspireActivate( *project ) )
      {
        capabilityElem.appendChild( getInspireCapabilitiesElement( doc, project ) );
      }
    }

    return capabilityElem;
  }

  QDomElement getInspireCapabilitiesElement( QDomDocument &doc, const QgsProject *project )
  {
    QDomElement inspireCapabilitiesElem;

    if ( !QgsServerProjectUtils::wmsInspireActivate( *project ) )
      return inspireCapabilitiesElem;

    inspireCapabilitiesElem = doc.createElement( QStringLiteral( "inspire_vs:ExtendedCapabilities" ) );

    QString inspireMetadataUrl = QgsServerProjectUtils::wmsInspireMetadataUrl( *project );
    // inspire scenario 1
    if ( !inspireMetadataUrl.isEmpty() )
    {
      QDomElement inspireCommonMetadataUrlElem = doc.createElement( QStringLiteral( "inspire_common:MetadataUrl" ) );
      inspireCommonMetadataUrlElem.setAttribute( QStringLiteral( "xsi:type" ), QStringLiteral( "inspire_common:resourceLocatorType" ) );

      QDomElement inspireCommonMetadataUrlUrlElem = doc.createElement( QStringLiteral( "inspire_common:URL" ) );
      inspireCommonMetadataUrlUrlElem.appendChild( doc.createTextNode( inspireMetadataUrl ) );
      inspireCommonMetadataUrlElem.appendChild( inspireCommonMetadataUrlUrlElem );

      QString inspireMetadataUrlType = QgsServerProjectUtils::wmsInspireMetadataUrlType( *project );
      if ( !inspireMetadataUrlType.isNull() )
      {
        QDomElement inspireCommonMetadataUrlMediaTypeElem = doc.createElement( QStringLiteral( "inspire_common:MediaType" ) );
        inspireCommonMetadataUrlMediaTypeElem.appendChild( doc.createTextNode( inspireMetadataUrlType ) );
        inspireCommonMetadataUrlElem.appendChild( inspireCommonMetadataUrlMediaTypeElem );
      }

      inspireCapabilitiesElem.appendChild( inspireCommonMetadataUrlElem );
    }
    else
    {
      QDomElement inspireCommonResourceTypeElem = doc.createElement( QStringLiteral( "inspire_common:ResourceType" ) );
      inspireCommonResourceTypeElem.appendChild( doc.createTextNode( QStringLiteral( "service" ) ) );
      inspireCapabilitiesElem.appendChild( inspireCommonResourceTypeElem );

      QDomElement inspireCommonSpatialDataServiceTypeElem = doc.createElement( QStringLiteral( "inspire_common:SpatialDataServiceType" ) );
      inspireCommonSpatialDataServiceTypeElem.appendChild( doc.createTextNode( QStringLiteral( "view" ) ) );
      inspireCapabilitiesElem.appendChild( inspireCommonSpatialDataServiceTypeElem );

      QString inspireTemporalReference = QgsServerProjectUtils::wmsInspireTemporalReference( *project );
      if ( !inspireTemporalReference.isNull() )
      {
        QDomElement inspireCommonTemporalReferenceElem = doc.createElement( QStringLiteral( "inspire_common:TemporalReference" ) );
        QDomElement inspireCommonDateOfLastRevisionElem = doc.createElement( QStringLiteral( "inspire_common:DateOfLastRevision" ) );
        inspireCommonDateOfLastRevisionElem.appendChild( doc.createTextNode( inspireTemporalReference ) );
        inspireCommonTemporalReferenceElem.appendChild( inspireCommonDateOfLastRevisionElem );
        inspireCapabilitiesElem.appendChild( inspireCommonTemporalReferenceElem );
      }

      QDomElement inspireCommonMetadataPointOfContactElem = doc.createElement( QStringLiteral( "inspire_common:MetadataPointOfContact" ) );

      QString contactOrganization = QgsServerProjectUtils::owsServiceContactOrganization( *project );
      QDomElement inspireCommonOrganisationNameElem = doc.createElement( QStringLiteral( "inspire_common:OrganisationName" ) );
      if ( !contactOrganization.isNull() )
      {
        inspireCommonOrganisationNameElem.appendChild( doc.createTextNode( contactOrganization ) );
      }
      inspireCommonMetadataPointOfContactElem.appendChild( inspireCommonOrganisationNameElem );

      QString contactMail = QgsServerProjectUtils::owsServiceContactMail( *project );
      QDomElement inspireCommonEmailAddressElem = doc.createElement( QStringLiteral( "inspire_common:EmailAddress" ) );
      if ( !contactMail.isNull() )
      {
        inspireCommonEmailAddressElem.appendChild( doc.createTextNode( contactMail ) );
      }
      inspireCommonMetadataPointOfContactElem.appendChild( inspireCommonEmailAddressElem );

      inspireCapabilitiesElem.appendChild( inspireCommonMetadataPointOfContactElem );

      QString inspireMetadataDate = QgsServerProjectUtils::wmsInspireMetadataDate( *project );
      if ( !inspireMetadataDate.isNull() )
      {
        QDomElement inspireCommonMetadataDateElem = doc.createElement( QStringLiteral( "inspire_common:MetadataDate" ) );
        inspireCommonMetadataDateElem.appendChild( doc.createTextNode( inspireMetadataDate ) );
        inspireCapabilitiesElem.appendChild( inspireCommonMetadataDateElem );
      }
    }

    // Supported languages
    QDomElement inspireCommonSupportedLanguagesElem = doc.createElement( QStringLiteral( "inspire_common:SupportedLanguages" ) );
    inspireCommonSupportedLanguagesElem.setAttribute( QStringLiteral( "xsi:type" ), QStringLiteral( "inspire_common:supportedLanguagesType" ) );

    QDomElement inspireCommonLanguageElem = doc.createElement( QStringLiteral( "inspire_common:Language" ) );
    inspireCommonLanguageElem.appendChild( doc.createTextNode( QgsServerProjectUtils::wmsInspireLanguage( *project ) ) );

    QDomElement inspireCommonDefaultLanguageElem = doc.createElement( QStringLiteral( "inspire_common:DefaultLanguage" ) );
    inspireCommonDefaultLanguageElem.appendChild( inspireCommonLanguageElem );
    inspireCommonSupportedLanguagesElem.appendChild( inspireCommonDefaultLanguageElem );

#if 0
    /* Supported language has to be different from default one */
    QDomElement inspireCommonSupportedLanguageElem = doc.createElement( "inspire_common:SupportedLanguage" );
    inspireCommonSupportedLanguageElem.appendChild( inspireCommonLanguageElem.cloneNode().toElement() );
    inspireCommonSupportedLanguagesElem.appendChild( inspireCommonSupportedLanguageElem );
#endif

    inspireCapabilitiesElem.appendChild( inspireCommonSupportedLanguagesElem );

    QDomElement inspireCommonResponseLanguageElem = doc.createElement( QStringLiteral( "inspire_common:ResponseLanguage" ) );
    inspireCommonResponseLanguageElem.appendChild( inspireCommonLanguageElem.cloneNode().toElement() );
    inspireCapabilitiesElem.appendChild( inspireCommonResponseLanguageElem );

    return inspireCapabilitiesElem;
  }

  QDomElement getComposerTemplatesElement( QDomDocument &doc, const QgsProject *project )
  {
    QList< QgsPrintLayout * > projectComposers = project->layoutManager()->printLayouts();
    if ( projectComposers.size() == 0 )
      return QDomElement();

    QStringList restrictedComposers = QgsServerProjectUtils::wmsRestrictedComposers( *project );

    QDomElement composerTemplatesElem = doc.createElement( QStringLiteral( "ComposerTemplates" ) );
    QList<QgsPrintLayout *>::const_iterator cIt = projectComposers.constBegin();
    for ( ; cIt != projectComposers.constEnd(); ++cIt )
    {
      QgsPrintLayout *layout = *cIt;
      if ( restrictedComposers.contains( layout->name() ) )
        continue;

      // Check that we have at least one page
      if ( layout->pageCollection()->pageCount() < 1 )
        continue;

      // Get width and height from first page of the collection
      QgsLayoutSize layoutSize( layout->pageCollection()->page( 0 )->sizeWithUnits() );
      QgsLayoutMeasurement width( layout->convertFromLayoutUnits( layoutSize.width(), QgsUnitTypes::LayoutUnit::LayoutMillimeters ) );
      QgsLayoutMeasurement height( layout->convertFromLayoutUnits( layoutSize.height(), QgsUnitTypes::LayoutUnit::LayoutMillimeters ) );

      QDomElement composerTemplateElem = doc.createElement( QStringLiteral( "ComposerTemplate" ) );
      composerTemplateElem.setAttribute( QStringLiteral( "name" ), layout->name() );

      //get paper width and height in mm from composition
      composerTemplateElem.setAttribute( QStringLiteral( "width" ), width.length() );
      composerTemplateElem.setAttribute( QStringLiteral( "height" ), height.length() );

      //atlas enabled and atlas covering layer
      QgsLayoutAtlas *atlas = layout->atlas();
      if ( atlas && atlas->enabled() )
      {
        composerTemplateElem.setAttribute( QStringLiteral( "atlasEnabled" ), QStringLiteral( "1" ) );
        QgsVectorLayer *cLayer = atlas->coverageLayer();
        if ( cLayer )
        {
          QString layerName = cLayer->shortName();
          if ( QgsServerProjectUtils::wmsUseLayerIds( *project ) )
          {
            layerName = cLayer->id();
          }
          else if ( layerName.isEmpty() )
          {
            layerName = cLayer->name();
          }
          composerTemplateElem.setAttribute( QStringLiteral( "atlasCoverageLayer" ), layerName );
        }
      }

      //add available composer maps and their size in mm
      QList<QgsLayoutItemMap *> layoutMapList;
      layout->layoutItems<QgsLayoutItemMap>( layoutMapList );
      QList<QgsLayoutItemMap *>::const_iterator cmIt = layoutMapList.constBegin();
      // Add map id
      int mapId = 0;
      for ( ; cmIt != layoutMapList.constEnd(); ++cmIt )
      {
        const QgsLayoutItemMap *composerMap = *cmIt;

        QDomElement composerMapElem = doc.createElement( QStringLiteral( "ComposerMap" ) );
        composerMapElem.setAttribute( QStringLiteral( "name" ), QStringLiteral( "map%1" ).arg( mapId ) );
        composerMapElem.setAttribute( QStringLiteral( "itemName" ), composerMap->displayName() );
        mapId++;
        composerMapElem.setAttribute( QStringLiteral( "width" ), composerMap->rect().width() );
        composerMapElem.setAttribute( QStringLiteral( "height" ), composerMap->rect().height() );
        composerTemplateElem.appendChild( composerMapElem );
      }

      //add available composer labels
      QList<QgsLayoutItemLabel *> composerLabelList;
      layout->layoutItems<QgsLayoutItemLabel>( composerLabelList );
      QList<QgsLayoutItemLabel *>::const_iterator clIt = composerLabelList.constBegin();
      for ( ; clIt != composerLabelList.constEnd(); ++clIt )
      {
        QgsLayoutItemLabel *composerLabel = *clIt;
        QString id = composerLabel->id();
        if ( id.isEmpty() )
          continue;

        QDomElement composerLabelElem = doc.createElement( QStringLiteral( "ComposerLabel" ) );
        composerLabelElem.setAttribute( QStringLiteral( "name" ), id );
        composerTemplateElem.appendChild( composerLabelElem );
      }

      //add available composer HTML
      QList<QgsLayoutItemHtml *> composerHtmlList;
      layout->layoutObjects<QgsLayoutItemHtml>( composerHtmlList );
      QList<QgsLayoutItemHtml *>::const_iterator chIt = composerHtmlList.constBegin();
      for ( ; chIt != composerHtmlList.constEnd(); ++chIt )
      {
        QgsLayoutItemHtml *composerHtml = *chIt;
        if ( composerHtml->frameCount() == 0 )
          continue;

        QString id = composerHtml->frame( 0 )->id();
        if ( id.isEmpty() )
          continue;

        QDomElement composerHtmlElem = doc.createElement( QStringLiteral( "ComposerHtml" ) );
        composerHtmlElem.setAttribute( QStringLiteral( "name" ), id );
        composerTemplateElem.appendChild( composerHtmlElem );
      }

      composerTemplatesElem.appendChild( composerTemplateElem );
    }

    if ( composerTemplatesElem.childNodes().size() == 0 )
      return QDomElement();

    return composerTemplatesElem;
  }

  QDomElement getWFSLayersElement( QDomDocument &doc, const QgsProject *project )
  {
    QStringList wfsLayerIds = QgsServerProjectUtils::wfsLayerIds( *project );
    if ( wfsLayerIds.size() == 0 )
      return QDomElement();

    QDomElement wfsLayersElem = doc.createElement( QStringLiteral( "WFSLayers" ) );
    for ( int i = 0; i < wfsLayerIds.size(); ++i )
    {
      QgsMapLayer *layer = project->mapLayer( wfsLayerIds.at( i ) );
      if ( ! layer || layer->type() != QgsMapLayerType::VectorLayer )
      {
        continue;
      }

      QDomElement wfsLayerElem = doc.createElement( QStringLiteral( "WFSLayer" ) );
      if ( QgsServerProjectUtils::wmsUseLayerIds( *project ) )
      {
        wfsLayerElem.setAttribute( QStringLiteral( "name" ), layer->id() );
      }
      else
      {
        wfsLayerElem.setAttribute( QStringLiteral( "name" ), layer->name() );
      }
      wfsLayersElem.appendChild( wfsLayerElem );
    }

    return wfsLayersElem;
  }

  void handleLayersFromTreeGroup( QDomDocument &doc,
                                  QDomElement &parentLayer,
                                  QgsServerInterface *serverIface,
                                  const QgsProject *project,
                                  const QgsWmsRequest &request,
                                  const QgsLayerTreeGroup *layerTreeGroup,
                                  const QMap< QString, QgsWmsLayerInfos > &wmsLayerInfos,
                                  bool projectSettings )
  {
    const auto layerIds = layerTreeGroup->findLayerIds();

    parentLayer.setAttribute(
      QStringLiteral( "queryable" ),
      hasQueryableLayers( layerIds, wmsLayerInfos ) ? QStringLiteral( "1" ) : QStringLiteral( "0" )
    );

    const QgsRectangle wgs84BoundingRect = combineWgs84BoundingRect( layerIds, wmsLayerInfos );
    QMap<QString, QgsRectangle> crsExtents = combineCrsExtents( layerIds, wmsLayerInfos );

    appendCrsElementsToLayer( doc, parentLayer, crsExtents.keys(), QStringList() );
    appendLayerWgs84BoundingRect( doc, parentLayer, wgs84BoundingRect );
    appendLayerCrsExtents( doc, parentLayer, crsExtents );

    appendLayersFromTreeGroup( doc, parentLayer, serverIface, project, request, layerTreeGroup, wmsLayerInfos, projectSettings );
  }

  QDomElement getLayersAndStylesCapabilitiesElement( QDomDocument &doc, QgsServerInterface *serverIface,
      const QgsProject *project,
      const QgsWmsRequest &request, bool projectSettings )
  {
    const QgsLayerTree *projectLayerTreeRoot = project->layerTreeRoot();

    QDomElement layerParentElem = doc.createElement( QStringLiteral( "Layer" ) );

    // Root Layer name
    QString rootLayerName = QgsServerProjectUtils::wmsRootName( *project );
    if ( rootLayerName.isEmpty() && !project->title().isEmpty() )
    {
      rootLayerName = project->title();
    }

    if ( !rootLayerName.isEmpty() )
    {
      QDomElement layerParentNameElem = doc.createElement( QStringLiteral( "Name" ) );
      QDomText layerParentNameText = doc.createTextNode( rootLayerName );
      layerParentNameElem.appendChild( layerParentNameText );
      layerParentElem.appendChild( layerParentNameElem );
    }

    // Root Layer title
    QDomElement layerParentTitleElem = doc.createElement( QStringLiteral( "Title" ) );
    QDomText layerParentTitleText = doc.createTextNode( QgsServerProjectUtils::owsServiceTitle( *project ) );
    layerParentTitleElem.appendChild( layerParentTitleText );
    layerParentElem.appendChild( layerParentTitleElem );

    // Root Layer abstract
    const QString rootLayerAbstract = QgsServerProjectUtils::owsServiceAbstract( *project );
    if ( !rootLayerAbstract.isEmpty() )
    {
      QDomElement layerParentAbstElem = doc.createElement( QStringLiteral( "Abstract" ) );
      QDomText layerParentAbstText = doc.createCDATASection( rootLayerAbstract );
      layerParentAbstElem.appendChild( layerParentAbstText );
      layerParentElem.appendChild( layerParentAbstElem );
    }

    // Keyword list
    addKeywordListElement( project, doc, layerParentElem );

    // Root Layer tree name
    if ( projectSettings )
    {
      QDomElement treeNameElem = doc.createElement( QStringLiteral( "TreeName" ) );
      QDomText treeNameText = doc.createTextNode( project->title() );
      treeNameElem.appendChild( treeNameText );
      layerParentElem.appendChild( treeNameElem );
    }

    // Instantiate CRS's from the project's crs list
    // This will prevent us to re-instantiate all the crs's each
    // time we will need to rebuild a bounding box.
    auto outputCrsList = QList<QgsCoordinateReferenceSystem>();
    for ( const QString &crsDef :  QgsServerProjectUtils::wmsOutputCrsList( *project ) )
    {
      const auto crs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( crsDef );
      if ( crs.isValid() )
      {
        outputCrsList.append( crs );
      }
    }

    // Get WMS layer infos
    const QMap< QString, QgsWmsLayerInfos > wmsLayerInfos = QgsWmsLayerInfos::buildWmsLayerInfos( serverIface, project, outputCrsList );

    const QgsRectangle wmsExtent = QgsServerProjectUtils::wmsExtent( *project );

    if ( !wmsExtent.isEmpty() )
    {
      const QgsCoordinateReferenceSystem wgs84 = QgsCoordinateReferenceSystem::fromOgcWmsCrs( geoEpsgCrsAuthId() );

      // Get WMS WGS84 bounding rectangle
      QgsRectangle wmsWgs84BoundingRect;
      try
      {
        wmsWgs84BoundingRect = QgsWmsLayerInfos::transformExtent(
                                 wmsExtent, project->crs(), wgs84, project->transformContext(), true
                               );
      }
      catch ( QgsCsException &cse )
      {
        QgsMessageLog::logMessage(
          QStringLiteral( "Error transforming extent: %1" ).arg( cse.what() ),
          QStringLiteral( "Server" ),
          Qgis::MessageLevel::Warning
        );
      }

      // Get WMS extents in output CRSes
      QMap< QString, QgsRectangle > wmsCrsExtents;
      try
      {
        wmsCrsExtents = QgsWmsLayerInfos::transformExtentToCrsList(
                          wmsExtent, project->crs(), outputCrsList, project->transformContext()
                        );
      }
      catch ( QgsCsException &cse )
      {
        QgsMessageLog::logMessage( QStringLiteral( "Error transforming extent: %1" ).arg( cse.what() ), QStringLiteral( "Server" ), Qgis::MessageLevel::Warning );
      }

      layerParentElem.setAttribute(
        QStringLiteral( "queryable" ),
        hasQueryableLayers( projectLayerTreeRoot->findLayerIds(), wmsLayerInfos ) ? QStringLiteral( "1" ) : QStringLiteral( "0" )
      );

      appendCrsElementsToLayer( doc, layerParentElem, wmsCrsExtents.keys(), QStringList() );
      appendLayerWgs84BoundingRect( doc, layerParentElem, wmsWgs84BoundingRect );
      appendLayerCrsExtents( doc, layerParentElem, wmsCrsExtents );

      appendLayersFromTreeGroup( doc, layerParentElem, serverIface, project, request, projectLayerTreeRoot, wmsLayerInfos, projectSettings );
    }
    else
    {
      handleLayersFromTreeGroup( doc, layerParentElem, serverIface, project, request, projectLayerTreeRoot, wmsLayerInfos, projectSettings );
    }

    return layerParentElem;
  }

  namespace
  {

    void appendLayersFromTreeGroup( QDomDocument &doc,
                                    QDomElement &parentLayer,
                                    QgsServerInterface *serverIface,
                                    const QgsProject *project,
                                    const QgsWmsRequest &request,
                                    const QgsLayerTreeGroup *layerTreeGroup,
                                    const QMap< QString, QgsWmsLayerInfos > &wmsLayerInfos,
                                    bool projectSettings )
    {
      const QString version = request.wmsParameters().version();

      bool siaFormat = QgsServerProjectUtils::wmsInfoFormatSia2045( *project );
      const QStringList restrictedLayers = QgsServerProjectUtils::wmsRestrictedLayers( *project );

      QList< QgsLayerTreeNode * > layerTreeGroupChildren = layerTreeGroup->children();
      for ( int i = 0; i < layerTreeGroupChildren.size(); ++i )
      {
        QgsLayerTreeNode *treeNode = layerTreeGroupChildren.at( i );
        QDomElement layerElem = doc.createElement( QStringLiteral( "Layer" ) );

        if ( projectSettings )
        {
          layerElem.setAttribute( QStringLiteral( "visible" ), treeNode->isVisible() );
          layerElem.setAttribute( QStringLiteral( "visibilityChecked" ), treeNode->itemVisibilityChecked() );
          layerElem.setAttribute( QStringLiteral( "expanded" ), treeNode->isExpanded() );
        }

        if ( treeNode->nodeType() == QgsLayerTreeNode::NodeGroup )
        {
          QgsLayerTreeGroup *treeGroupChild = static_cast<QgsLayerTreeGroup *>( treeNode );

          QString name = treeGroupChild->name();
          if ( restrictedLayers.contains( name ) ) //unpublished group
          {
            continue;
          }

          if ( projectSettings )
          {
            layerElem.setAttribute( QStringLiteral( "mutuallyExclusive" ), treeGroupChild->isMutuallyExclusive() );
          }

          QString shortName = treeGroupChild->customProperty( QStringLiteral( "wmsShortName" ) ).toString();
          QString title = treeGroupChild->customProperty( QStringLiteral( "wmsTitle" ) ).toString();

          QDomElement nameElem = doc.createElement( QStringLiteral( "Name" ) );
          QDomText nameText;
          if ( !shortName.isEmpty() )
            nameText = doc.createTextNode( shortName );
          else
            nameText = doc.createTextNode( name );
          nameElem.appendChild( nameText );
          layerElem.appendChild( nameElem );

          QDomElement titleElem = doc.createElement( QStringLiteral( "Title" ) );
          QDomText titleText;
          if ( !title.isEmpty() )
            titleText = doc.createTextNode( title );
          else
            titleText = doc.createTextNode( name );
          titleElem.appendChild( titleText );
          layerElem.appendChild( titleElem );

          QString abstract = treeGroupChild->customProperty( QStringLiteral( "wmsAbstract" ) ).toString();
          if ( !abstract.isEmpty() )
          {
            QDomElement abstractElem = doc.createElement( QStringLiteral( "Abstract" ) );
            QDomText abstractText = doc.createTextNode( abstract );
            abstractElem.appendChild( abstractText );
            layerElem.appendChild( abstractElem );
          }

          // Layer tree name
          if ( projectSettings )
          {
            QDomElement treeNameElem = doc.createElement( QStringLiteral( "TreeName" ) );
            QDomText treeNameText = doc.createTextNode( name );
            treeNameElem.appendChild( treeNameText );
            layerElem.appendChild( treeNameElem );
          }

          handleLayersFromTreeGroup( doc, layerElem, serverIface, project, request, treeGroupChild, wmsLayerInfos, projectSettings );
        }
        else
        {
          QgsLayerTreeLayer *treeLayer = static_cast<QgsLayerTreeLayer *>( treeNode );
          QgsMapLayer *l = treeLayer->layer();
          if ( !wmsLayerInfos.contains( treeLayer->layerId() ) ) //unpublished layer
          {
            continue;
          }

          const QgsWmsLayerInfos &layerInfos = wmsLayerInfos[ treeLayer->layerId() ];

          layerElem.setAttribute(
            QStringLiteral( "queryable" ),
            layerInfos.queryable ? QStringLiteral( "1" ) : QStringLiteral( "0" )
          );

          QDomElement nameElem = doc.createElement( QStringLiteral( "Name" ) );
          QDomText nameText = doc.createTextNode( layerInfos.name );
          nameElem.appendChild( nameText );
          layerElem.appendChild( nameElem );

          QDomElement titleElem = doc.createElement( QStringLiteral( "Title" ) );
          QDomText titleText = doc.createTextNode( layerInfos.title );
          titleElem.appendChild( titleText );
          layerElem.appendChild( titleElem );

          if ( ! layerInfos.abstract.isEmpty() )
          {
            QDomElement abstractElem = doc.createElement( QStringLiteral( "Abstract" ) );
            QDomText abstractText = doc.createTextNode( layerInfos.abstract );
            abstractElem.appendChild( abstractText );
            layerElem.appendChild( abstractElem );
          }

          //keyword list
          if ( ! layerInfos.keywords.isEmpty() )
          {
            QDomElement keywordListElem = doc.createElement( QStringLiteral( "KeywordList" ) );
            for ( const QString &keyword : std::as_const( layerInfos.keywords ) )
            {
              QDomElement keywordElem = doc.createElement( QStringLiteral( "Keyword" ) );
              QDomText keywordText = doc.createTextNode( keyword.trimmed() );
              keywordElem.appendChild( keywordText );
              if ( siaFormat )
              {
                keywordElem.setAttribute( QStringLiteral( "vocabulary" ), QStringLiteral( "SIA_Geo405" ) );
              }
              keywordListElem.appendChild( keywordElem );
            }
            layerElem.appendChild( keywordListElem );
          }

          // Append not null Bounding rectangles
          if ( ! layerInfos.wgs84BoundingRect.isNull() )
          {
            appendCrsElementsToLayer( doc, layerElem, layerInfos.crsExtents.keys(), QStringList() );

            appendLayerWgs84BoundingRect( doc, layerElem, layerInfos.wgs84BoundingRect );

            appendLayerCrsExtents( doc, layerElem, layerInfos.crsExtents );
          }

          // add details about supported styles of the layer
          appendLayerStyles( doc, layerElem, layerInfos, project, request, serverIface->serverSettings() );

          //min/max scale denominatorScaleBasedVisibility
          if ( layerInfos.hasScaleBasedVisibility )
          {
            if ( version == QLatin1String( "1.1.1" ) )
            {
              double OGC_PX_M = 0.00028; // OGC reference pixel size in meter, also used by qgis
              double SCALE_TO_SCALEHINT = OGC_PX_M * M_SQRT2;

              QDomElement scaleHintElem = doc.createElement( QStringLiteral( "ScaleHint" ) );
              scaleHintElem.setAttribute( QStringLiteral( "min" ), QString::number( layerInfos.maxScale * SCALE_TO_SCALEHINT ) );
              scaleHintElem.setAttribute( QStringLiteral( "max" ), QString::number( layerInfos.minScale * SCALE_TO_SCALEHINT ) );
              layerElem.appendChild( scaleHintElem );
            }
            else
            {
              QDomElement minScaleElem = doc.createElement( QStringLiteral( "MinScaleDenominator" ) );
              QDomText minScaleText = doc.createTextNode( QString::number( layerInfos.maxScale ) );
              minScaleElem.appendChild( minScaleText );
              layerElem.appendChild( minScaleElem );

              QDomElement maxScaleElem = doc.createElement( QStringLiteral( "MaxScaleDenominator" ) );
              QDomText maxScaleText = doc.createTextNode( QString::number( layerInfos.minScale ) );
              maxScaleElem.appendChild( maxScaleText );
              layerElem.appendChild( maxScaleElem );
            }
          }

          // layer data URL
          if ( !layerInfos.dataUrl.isEmpty() )
          {
            QDomElement dataUrlElem = doc.createElement( QStringLiteral( "DataURL" ) );
            QDomElement dataUrlFormatElem = doc.createElement( QStringLiteral( "Format" ) );
            QString dataUrlFormat = l->dataUrlFormat();
            QDomText dataUrlFormatText = doc.createTextNode( dataUrlFormat );
            dataUrlFormatElem.appendChild( dataUrlFormatText );
            dataUrlElem.appendChild( dataUrlFormatElem );
            QDomElement dataORElem = doc.createElement( QStringLiteral( "OnlineResource" ) );
            dataORElem.setAttribute( QStringLiteral( "xmlns:xlink" ), QStringLiteral( "http://www.w3.org/1999/xlink" ) );
            dataORElem.setAttribute( QStringLiteral( "xlink:type" ), QStringLiteral( "simple" ) );
            dataORElem.setAttribute( QStringLiteral( "xlink:href" ), layerInfos.dataUrl );
            dataUrlElem.appendChild( dataORElem );
            layerElem.appendChild( dataUrlElem );
          }

          // layer attribution
          if ( ! layerInfos.attribution.isEmpty() )
          {
            QDomElement attribElem = doc.createElement( QStringLiteral( "Attribution" ) );
            QDomElement attribTitleElem = doc.createElement( QStringLiteral( "Title" ) );
            QDomText attribText = doc.createTextNode( layerInfos.attribution );
            attribTitleElem.appendChild( attribText );
            attribElem.appendChild( attribTitleElem );
            if ( ! layerInfos.attributionUrl.isEmpty() )
            {
              QDomElement attribORElem = doc.createElement( QStringLiteral( "OnlineResource" ) );
              attribORElem.setAttribute( QStringLiteral( "xmlns:xlink" ), QStringLiteral( "http://www.w3.org/1999/xlink" ) );
              attribORElem.setAttribute( QStringLiteral( "xlink:type" ), QStringLiteral( "simple" ) );
              attribORElem.setAttribute( QStringLiteral( "xlink:href" ), layerInfos.attributionUrl );
              attribElem.appendChild( attribORElem );
            }
            layerElem.appendChild( attribElem );
          }

          // layer metadata URL
          for ( const QgsMapLayerServerProperties::MetadataUrl &metadataUrl : std::as_const( layerInfos.metadataUrls ) )
          {
            QDomElement metaUrlElem = doc.createElement( QStringLiteral( "MetadataURL" ) );
            const QString metadataUrlType = metadataUrl.type;
            if ( version == QLatin1String( "1.1.1" ) )
            {
              metaUrlElem.setAttribute( QStringLiteral( "type" ), metadataUrlType );
            }
            else if ( metadataUrlType == QLatin1String( "FGDC" ) )
            {
              metaUrlElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "FGDC:1998" ) );
            }
            else if ( metadataUrlType == QLatin1String( "TC211" ) )
            {
              metaUrlElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "ISO19115:2003" ) );
            }
            else
            {
              metaUrlElem.setAttribute( QStringLiteral( "type" ), metadataUrlType );
            }
            const QString metadataUrlFormat = metadataUrl.format;
            if ( !metadataUrlFormat.isEmpty() )
            {
              QDomElement metaUrlFormatElem = doc.createElement( QStringLiteral( "Format" ) );
              QDomText metaUrlFormatText = doc.createTextNode( metadataUrlFormat );
              metaUrlFormatElem.appendChild( metaUrlFormatText );
              metaUrlElem.appendChild( metaUrlFormatElem );
            }
            QDomElement metaUrlORElem = doc.createElement( QStringLiteral( "OnlineResource" ) );
            metaUrlORElem.setAttribute( QStringLiteral( "xmlns:xlink" ), QStringLiteral( "http://www.w3.org/1999/xlink" ) );
            metaUrlORElem.setAttribute( QStringLiteral( "xlink:type" ), QStringLiteral( "simple" ) );
            metaUrlORElem.setAttribute( QStringLiteral( "xlink:href" ), metadataUrl.url );
            metaUrlElem.appendChild( metaUrlORElem );
            layerElem.appendChild( metaUrlElem );
          }

          bool timeDimensionAdded { false };

          // Add dimensions
          if ( l->type() == QgsMapLayerType::VectorLayer )
          {
            QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( l );
            QgsMapLayerServerProperties *serverProperties = static_cast<QgsMapLayerServerProperties *>( vl->serverProperties() );
            const QList<QgsMapLayerServerProperties::WmsDimensionInfo> wmsDims = serverProperties->wmsDimensions();
            for ( const  QgsMapLayerServerProperties::WmsDimensionInfo &dim : wmsDims )
            {

              int fieldIndex = vl->fields().indexOf( dim.fieldName );
              // Check field index
              if ( fieldIndex == -1 )
              {
                continue;
              }
              // get unique values
              QSet<QVariant> uniqueValues = vl->uniqueValues( fieldIndex );

              // get unique values from endfield name if define
              if ( !dim.endFieldName.isEmpty() )
              {
                int endFieldIndex = vl->fields().indexOf( dim.endFieldName );
                // Check end field index
                if ( endFieldIndex == -1 )
                {
                  continue;
                }
                uniqueValues.unite( vl->uniqueValues( endFieldIndex ) );
              }
              // sort unique values
              QList<QVariant> values = qgis::setToList( uniqueValues );
              std::sort( values.begin(), values.end() );

              QDomElement dimElem = doc.createElement( QStringLiteral( "Dimension" ) );
              dimElem.setAttribute( QStringLiteral( "name" ), dim.name );

              if ( dim.name.toUpper() == QLatin1String( "TIME" ) )
              {
                timeDimensionAdded = true;
              }

              if ( !dim.units.isEmpty() )
              {
                dimElem.setAttribute( QStringLiteral( "units" ), dim.units );
              }
              if ( !dim.unitSymbol.isEmpty() )
              {
                dimElem.setAttribute( QStringLiteral( "unitSymbol" ), dim.unitSymbol );
              }
              if ( dim.defaultDisplayType == QgsMapLayerServerProperties::WmsDimensionInfo::MinValue )
              {
                dimElem.setAttribute( QStringLiteral( "default" ), values.first().toString() );
              }
              else if ( dim.defaultDisplayType == QgsMapLayerServerProperties::WmsDimensionInfo::MaxValue )
              {
                dimElem.setAttribute( QStringLiteral( "default" ), values.last().toString() );
              }
              else if ( dim.defaultDisplayType == QgsMapLayerServerProperties::WmsDimensionInfo::ReferenceValue )
              {
                dimElem.setAttribute( QStringLiteral( "default" ), dim.referenceValue.toString() );
              }
              dimElem.setAttribute( QStringLiteral( "multipleValues" ), QStringLiteral( "1" ) );
              dimElem.setAttribute( QStringLiteral( "nearestValue" ), QStringLiteral( "0" ) );
              if ( projectSettings )
              {
                dimElem.setAttribute( QStringLiteral( "fieldName" ), dim.fieldName );
                dimElem.setAttribute( QStringLiteral( "endFieldName" ), dim.endFieldName );
              }
              // values list
              QStringList strValues;
              for ( const QVariant &v : values )
              {
                strValues << v.toString();
              }
              QDomText dimValuesText = doc.createTextNode( strValues.join( QLatin1String( ", " ) ) );
              dimElem.appendChild( dimValuesText );
              layerElem.appendChild( dimElem );
            }
          }

          // Add WMS time dimension if not already added
          if ( ! timeDimensionAdded
               && l->temporalProperties()
               && l->temporalProperties()->isActive() )
          {

            QDomElement dimElem = doc.createElement( QStringLiteral( "Dimension" ) );
            dimElem.setAttribute( QStringLiteral( "name" ), QStringLiteral( "TIME" ) );
            dimElem.setAttribute( QStringLiteral( "units" ), QStringLiteral( "ISO8601" ) );

            // TODO: set "default" (reference value)

            // Add all values
            const QList<QgsDateTimeRange> allRanges { l->temporalProperties()->allTemporalRanges( l ) };

            // Apparently, for vectors allTemporalRanges is always empty :/
            // there is no way to know the type of range or the individual instants

            bool isDateList { true };
            bool isInstantList { true };

            QList<QDateTime> values;
            for ( const auto &r : std::as_const( allRanges ) )
            {
              if ( r.isInstant() )
              {
                if ( r.begin().time() != QTime( 0, 0, 0, 0 ) )
                {
                  isDateList = false;
                }
                values.append( r.begin() );
              }
              else
              {
                isInstantList = false;
                break;
              }
            }

            // Only list individual values for list of instants,
            // otherwise only the extent will be shown
            if ( isInstantList )
            {
              // values list
              QStringList strValues;
              for ( const auto &v : values )
              {
                if ( isDateList )
                {
                  strValues << v.date().toString( Qt::DateFormat::ISODate );
                }
                else
                {
                  strValues << v.toString( Qt::DateFormat::ISODate );
                }
              }
              QDomText dimValuesText = doc.createTextNode( strValues.join( QChar( ',' ) ) );
              dimElem.appendChild( dimValuesText );
            }

            layerElem.appendChild( dimElem );

            QDomElement timeExtentElem = doc.createElement( QStringLiteral( "Extent" ) );
            timeExtentElem.setAttribute( QStringLiteral( "name" ), QStringLiteral( "TIME" ) );

            const QgsDateTimeRange timeExtent { l->temporalProperties()->calculateTemporalExtent( l ) };
            QString extent;
            if ( isDateList )
            {
              extent = QStringLiteral( "%1/%2" ).arg( timeExtent.begin().date().toString( Qt::DateFormat::ISODate ), timeExtent.end().date().toString( Qt::DateFormat::ISODate ) );
            }
            else
            {
              extent = QStringLiteral( "%1/%2" ).arg( timeExtent.begin().toString( Qt::DateFormat::ISODate ), timeExtent.end().toString( Qt::DateFormat::ISODate ) );
            }
            QDomText extentValueText = doc.createTextNode( extent );
            timeExtentElem.appendChild( extentValueText );
            layerElem.appendChild( timeExtentElem );

          }

          if ( projectSettings )
          {
            appendLayerProjectSettings( doc, layerElem, l );
          }
        }

        parentLayer.appendChild( layerElem );
      }
    }

    void appendLayerStyles( QDomDocument &doc, QDomElement &layerElem, const QgsWmsLayerInfos &layerInfos,
                            const QgsProject *project, const QgsWmsRequest &request, const QgsServerSettings *settings )
    {
      // Get service URL
      QUrl href = serviceUrl( request, project, *settings );

      //href needs to be a prefix
      QString hrefString = href.toString();
      hrefString.append( href.hasQuery() ? "&" : "?" );
      for ( const QString &styleName : std::as_const( layerInfos.styles ) )
      {
        QDomElement styleElem = doc.createElement( QStringLiteral( "Style" ) );
        QDomElement styleNameElem = doc.createElement( QStringLiteral( "Name" ) );
        QDomText styleNameText = doc.createTextNode( styleName );
        styleNameElem.appendChild( styleNameText );
        QDomElement styleTitleElem = doc.createElement( QStringLiteral( "Title" ) );
        QDomText styleTitleText = doc.createTextNode( styleName );
        styleTitleElem.appendChild( styleTitleText );
        styleElem.appendChild( styleNameElem );
        styleElem.appendChild( styleTitleElem );

        // QString LegendURL for explicit layerbased GetLegendGraphic request
        QDomElement getLayerLegendGraphicElem = doc.createElement( QStringLiteral( "LegendURL" ) );

        QString customHrefString = layerInfos.legendUrl;

        QStringList getLayerLegendGraphicFormats;
        if ( !customHrefString.isEmpty() )
        {
          getLayerLegendGraphicFormats << layerInfos.legendUrlFormat;
        }
        else
        {
          getLayerLegendGraphicFormats << QStringLiteral( "image/png" ); // << "jpeg" << "image/jpeg"
        }

        for ( const QString &getLayerLegendGraphicFormat : std::as_const( getLayerLegendGraphicFormats ) )
        {
          QDomElement getLayerLegendGraphicFormatElem = doc.createElement( QStringLiteral( "Format" ) );
          QDomText getLayerLegendGraphicFormatText = doc.createTextNode( getLayerLegendGraphicFormat );
          getLayerLegendGraphicFormatElem.appendChild( getLayerLegendGraphicFormatText );
          getLayerLegendGraphicElem.appendChild( getLayerLegendGraphicFormatElem );
        }

        // no parameters on custom hrefUrl, because should link directly to graphic
        if ( customHrefString.isEmpty() )
        {
          QUrl mapUrl( hrefString );
          QUrlQuery mapUrlQuery( mapUrl.query() );
          mapUrlQuery.addQueryItem( QStringLiteral( "SERVICE" ), QStringLiteral( "WMS" ) );
          mapUrlQuery.addQueryItem( QStringLiteral( "VERSION" ), request.wmsParameters().version() );
          mapUrlQuery.addQueryItem( QStringLiteral( "REQUEST" ), QStringLiteral( "GetLegendGraphic" ) );
          mapUrlQuery.addQueryItem( QStringLiteral( "LAYER" ), layerInfos.name );
          mapUrlQuery.addQueryItem( QStringLiteral( "FORMAT" ), QStringLiteral( "image/png" ) );
          mapUrlQuery.addQueryItem( QStringLiteral( "STYLE" ), styleName );
          if ( request.wmsParameters().version() == QLatin1String( "1.3.0" ) )
          {
            mapUrlQuery.addQueryItem( QStringLiteral( "SLD_VERSION" ), QStringLiteral( "1.1.0" ) );
          }
          mapUrl.setQuery( mapUrlQuery );
          customHrefString = mapUrl.toString();
        }

        QDomElement getLayerLegendGraphicORElem = doc.createElement( QStringLiteral( "OnlineResource" ) );
        getLayerLegendGraphicORElem.setAttribute( QStringLiteral( "xmlns:xlink" ), QStringLiteral( "http://www.w3.org/1999/xlink" ) );
        getLayerLegendGraphicORElem.setAttribute( QStringLiteral( "xlink:type" ), QStringLiteral( "simple" ) );
        getLayerLegendGraphicORElem.setAttribute( QStringLiteral( "xlink:href" ), customHrefString );
        getLayerLegendGraphicElem.appendChild( getLayerLegendGraphicORElem );
        styleElem.appendChild( getLayerLegendGraphicElem );

        layerElem.appendChild( styleElem );
      }
    }

    void appendCrsElementsToLayer( QDomDocument &doc, QDomElement &layerElement,
                                   const QStringList &crsList, const QStringList &constrainedCrsList )
    {
      if ( layerElement.isNull() )
      {
        return;
      }

      const QString version = doc.documentElement().attribute( QStringLiteral( "version" ) );

      //insert the CRS elements after the title element to be in accordance with the WMS 1.3 specification
      QDomElement titleElement = layerElement.firstChildElement( QStringLiteral( "Title" ) );
      QDomElement abstractElement = layerElement.firstChildElement( QStringLiteral( "Abstract" ) );
      QDomElement keywordListElement = layerElement.firstChildElement( QStringLiteral( "KeywordList" ) );
      QDomElement CRSPrecedingElement = !keywordListElement.isNull() ? keywordListElement : !abstractElement.isNull() ? abstractElement : titleElement;

      if ( CRSPrecedingElement.isNull() )
      {
        // keyword list element is never empty
        const QDomElement keyElement = layerElement.firstChildElement( QStringLiteral( "KeywordList" ) );
        CRSPrecedingElement = keyElement;
      }

      //In case the number of advertised CRS is constrained
      if ( !constrainedCrsList.isEmpty() )
      {
        for ( int i = constrainedCrsList.size() - 1; i >= 0; --i )
        {
          appendCrsElementToLayer( doc, layerElement, CRSPrecedingElement, constrainedCrsList.at( i ) );
        }
      }
      else //no crs constraint
      {
        for ( const QString &crs : crsList )
        {
          appendCrsElementToLayer( doc, layerElement, CRSPrecedingElement, crs );
        }
      }

      // Support for CRS:84 is mandatory (equals EPSG:4326 with reversed axis)
      // https://github.com/opengeospatial/ets-wms13/blob/47155399c09b200cb21382874fdb21d5fae4ab6e/src/site/markdown/index.md
      if ( version == QLatin1String( "1.3.0" ) )
      {
        appendCrsElementToLayer( doc, layerElement, CRSPrecedingElement, QString( "CRS:84" ) );
      }
    }

    void appendCrsElementToLayer( QDomDocument &doc, QDomElement &layerElement, const QDomElement &precedingElement,
                                  const QString &crsText )
    {
      if ( crsText.isEmpty() )
        return;
      const QString version = doc.documentElement().attribute( QStringLiteral( "version" ) );
      QDomElement crsElement = doc.createElement( version == QLatin1String( "1.1.1" ) ? "SRS" : "CRS" );
      QDomText crsTextNode = doc.createTextNode( crsText );
      crsElement.appendChild( crsTextNode );
      layerElement.insertAfter( crsElement, precedingElement );
    }

    void appendLayerWgs84BoundingRect( QDomDocument &doc, QDomElement &layerElem, const QgsRectangle &wgs84BoundingRect )
    {
      //LatLonBoundingBox / Ex_GeographicBounding box is optional
      if ( wgs84BoundingRect.isNull() )
      {
        return;
      }

      //Ex_GeographicBoundingBox
      QDomElement ExGeoBBoxElement;
      const int wgs84precision = 6;
      const QString version = doc.documentElement().attribute( QStringLiteral( "version" ) );
      if ( version == QLatin1String( "1.1.1" ) ) // WMS Version 1.1.1
      {
        ExGeoBBoxElement = doc.createElement( QStringLiteral( "LatLonBoundingBox" ) );
        ExGeoBBoxElement.setAttribute( QStringLiteral( "minx" ), qgsDoubleToString( QgsServerProjectUtils::floorWithPrecision( wgs84BoundingRect.xMinimum(), wgs84precision ), wgs84precision ) );
        ExGeoBBoxElement.setAttribute( QStringLiteral( "miny" ), qgsDoubleToString( QgsServerProjectUtils::floorWithPrecision( wgs84BoundingRect.yMinimum(), wgs84precision ), wgs84precision ) );
        ExGeoBBoxElement.setAttribute( QStringLiteral( "maxx" ), qgsDoubleToString( QgsServerProjectUtils::ceilWithPrecision( wgs84BoundingRect.xMaximum(), wgs84precision ), wgs84precision ) );
        ExGeoBBoxElement.setAttribute( QStringLiteral( "maxy" ), qgsDoubleToString( QgsServerProjectUtils::ceilWithPrecision( wgs84BoundingRect.yMaximum(), wgs84precision ), wgs84precision ) );
      }
      else // WMS Version 1.3.0
      {
        ExGeoBBoxElement = doc.createElement( QStringLiteral( "EX_GeographicBoundingBox" ) );
        QDomElement wBoundLongitudeElement = doc.createElement( QStringLiteral( "westBoundLongitude" ) );
        QDomText wBoundLongitudeText = doc.createTextNode( qgsDoubleToString( QgsServerProjectUtils::floorWithPrecision( wgs84BoundingRect.xMinimum(), wgs84precision ), wgs84precision ) );
        wBoundLongitudeElement.appendChild( wBoundLongitudeText );
        ExGeoBBoxElement.appendChild( wBoundLongitudeElement );
        QDomElement eBoundLongitudeElement = doc.createElement( QStringLiteral( "eastBoundLongitude" ) );
        QDomText eBoundLongitudeText = doc.createTextNode( qgsDoubleToString( QgsServerProjectUtils::ceilWithPrecision( wgs84BoundingRect.xMaximum(), wgs84precision ), wgs84precision ) );
        eBoundLongitudeElement.appendChild( eBoundLongitudeText );
        ExGeoBBoxElement.appendChild( eBoundLongitudeElement );
        QDomElement sBoundLatitudeElement = doc.createElement( QStringLiteral( "southBoundLatitude" ) );
        QDomText sBoundLatitudeText = doc.createTextNode( qgsDoubleToString( QgsServerProjectUtils::floorWithPrecision( wgs84BoundingRect.yMinimum(), wgs84precision ), wgs84precision ) );
        sBoundLatitudeElement.appendChild( sBoundLatitudeText );
        ExGeoBBoxElement.appendChild( sBoundLatitudeElement );
        QDomElement nBoundLatitudeElement = doc.createElement( QStringLiteral( "northBoundLatitude" ) );
        QDomText nBoundLatitudeText = doc.createTextNode( qgsDoubleToString( QgsServerProjectUtils::ceilWithPrecision( wgs84BoundingRect.yMaximum(), wgs84precision ), wgs84precision ) );
        nBoundLatitudeElement.appendChild( nBoundLatitudeText );
        ExGeoBBoxElement.appendChild( nBoundLatitudeElement );
      }

      const QDomElement lastCRSElem = layerElem.lastChildElement( version == QLatin1String( "1.1.1" ) ? "SRS" : "CRS" );
      if ( !lastCRSElem.isNull() )
      {
        layerElem.insertAfter( ExGeoBBoxElement, lastCRSElem );
      }
      else
      {
        layerElem.appendChild( ExGeoBBoxElement );
      }
    }

    void appendLayerCrsExtents( QDomDocument &doc, QDomElement &layerElem, const QMap<QString, QgsRectangle> &crsExtents )
    {
      const QString version = doc.documentElement().attribute( QStringLiteral( "version" ) );

      const auto &keys =  crsExtents.keys();
      for ( const QString &crsText : std::as_const( keys ) )
      {
        QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( crsText );
        QgsRectangle crsExtent( crsExtents[ crsText ] );

        if ( crsExtent.isNull() )
        {
          continue;
        }

        int precision = 3;
        if ( crs.isGeographic() )
        {
          precision = 6;
        }

        //BoundingBox element
        QDomElement bBoxElement = doc.createElement( QStringLiteral( "BoundingBox" ) );
        if ( crs.isValid() )
        {
          bBoxElement.setAttribute( version == QLatin1String( "1.1.1" ) ? "SRS" : "CRS", crs.authid() );
        }

        if ( version != QLatin1String( "1.1.1" ) && crs.hasAxisInverted() )
        {
          crsExtent.invert();
        }

        bBoxElement.setAttribute( QStringLiteral( "minx" ), qgsDoubleToString( QgsServerProjectUtils::floorWithPrecision( crsExtent.xMinimum(), precision ), precision ) );
        bBoxElement.setAttribute( QStringLiteral( "miny" ), qgsDoubleToString( QgsServerProjectUtils::floorWithPrecision( crsExtent.yMinimum(), precision ), precision ) );
        bBoxElement.setAttribute( QStringLiteral( "maxx" ), qgsDoubleToString( QgsServerProjectUtils::ceilWithPrecision( crsExtent.xMaximum(), precision ), precision ) );
        bBoxElement.setAttribute( QStringLiteral( "maxy" ), qgsDoubleToString( QgsServerProjectUtils::ceilWithPrecision( crsExtent.yMaximum(), precision ), precision ) );

        QDomElement lastBBoxElem = layerElem.lastChildElement( QStringLiteral( "BoundingBox" ) );
        if ( !lastBBoxElem.isNull() )
        {
          layerElem.insertAfter( bBoxElement, lastBBoxElem );
        }
        else
        {
          lastBBoxElem = layerElem.lastChildElement( version == QLatin1String( "1.1.1" ) ? "LatLonBoundingBox" : "EX_GeographicBoundingBox" );
          if ( !lastBBoxElem.isNull() )
          {
            layerElem.insertAfter( bBoxElement, lastBBoxElem );
          }
          else
          {
            layerElem.appendChild( bBoxElement );
          }
        }
      }
    }

    void appendDrawingOrder( QDomDocument &doc, QDomElement &parentElem, QgsServerInterface *serverIface,
                             const QgsProject *project )
    {
#ifdef HAVE_SERVER_PYTHON_PLUGINS
      QgsAccessControl *accessControl = serverIface->accessControls();
#else
      ( void )serverIface;
#endif
      bool useLayerIds = QgsServerProjectUtils::wmsUseLayerIds( *project );
      QStringList restrictedLayers = QgsServerProjectUtils::wmsRestrictedLayers( *project );

      QStringList layerList;

      const QgsLayerTree *projectLayerTreeRoot = project->layerTreeRoot();
      QList< QgsMapLayer * > projectLayerOrder = projectLayerTreeRoot->layerOrder();
      for ( int i = 0; i < projectLayerOrder.size(); ++i )
      {
        QgsMapLayer *l = projectLayerOrder.at( i );

        if ( restrictedLayers.contains( l->name() ) ) //unpublished layer
        {
          continue;
        }
#ifdef HAVE_SERVER_PYTHON_PLUGINS
        if ( accessControl && !accessControl->layerReadPermission( l ) )
        {
          continue;
        }
#endif
        QString wmsName = l->name();
        if ( useLayerIds )
        {
          wmsName = l->id();
        }
        else if ( !l->shortName().isEmpty() )
        {
          wmsName = l->shortName();
        }

        layerList <<  wmsName;
      }

      if ( !layerList.isEmpty() )
      {
        QStringList reversedList;
        reversedList.reserve( layerList.size() );
        for ( int i = layerList.size() - 1; i >= 0; --i )
          reversedList << layerList[ i ];

        QDomElement layerDrawingOrderElem = doc.createElement( QStringLiteral( "LayerDrawingOrder" ) );
        QDomText drawingOrderText = doc.createTextNode( reversedList.join( ',' ) );
        layerDrawingOrderElem.appendChild( drawingOrderText );
        parentElem.appendChild( layerDrawingOrderElem );
      }
    }

    void appendLayerProjectSettings( QDomDocument &doc, QDomElement &layerElem, QgsMapLayer *currentLayer )
    {
      if ( !currentLayer )
      {
        return;
      }

      // Layer tree name
      QDomElement treeNameElem = doc.createElement( QStringLiteral( "TreeName" ) );
      QDomText treeNameText = doc.createTextNode( currentLayer->name() );
      treeNameElem.appendChild( treeNameText );
      layerElem.appendChild( treeNameElem );

      switch ( currentLayer->type() )
      {
        case QgsMapLayerType::VectorLayer:
        {
          QgsVectorLayer *vLayer = static_cast<QgsVectorLayer *>( currentLayer );

          int displayFieldIdx = -1;
          QString displayField = QStringLiteral( "maptip" );
          QgsExpression exp( vLayer->displayExpression() );
          if ( exp.isField() )
          {
            displayField = static_cast<const QgsExpressionNodeColumnRef *>( exp.rootNode() )->name();
            displayFieldIdx = vLayer->fields().lookupField( displayField );
          }

          //attributes
          QDomElement attributesElem = doc.createElement( QStringLiteral( "Attributes" ) );
          const QgsFields layerFields = vLayer->fields();
          for ( int idx = 0; idx < layerFields.count(); ++idx )
          {
            QgsField field = layerFields.at( idx );
            if ( field.configurationFlags().testFlag( QgsField::ConfigurationFlag::HideFromWms ) )
            {
              continue;
            }
            // field alias in case of displayField
            if ( idx == displayFieldIdx )
            {
              displayField = vLayer->attributeDisplayName( idx );
            }
            QDomElement attributeElem = doc.createElement( QStringLiteral( "Attribute" ) );
            attributeElem.setAttribute( QStringLiteral( "name" ), field.name() );
            attributeElem.setAttribute( QStringLiteral( "type" ), QVariant::typeToName( field.type() ) );
            attributeElem.setAttribute( QStringLiteral( "typeName" ), field.typeName() );
            QString alias = field.alias();
            if ( !alias.isEmpty() )
            {
              attributeElem.setAttribute( QStringLiteral( "alias" ), alias );
            }

            //edit type to text
            attributeElem.setAttribute( QStringLiteral( "editType" ), vLayer->editorWidgetSetup( idx ).type() );
            attributeElem.setAttribute( QStringLiteral( "comment" ), field.comment() );
            attributeElem.setAttribute( QStringLiteral( "length" ), field.length() );
            attributeElem.setAttribute( QStringLiteral( "precision" ), field.precision() );
            attributesElem.appendChild( attributeElem );
          }

          //displayfield
          layerElem.setAttribute( QStringLiteral( "displayField" ), displayField );

          //primary key
          QgsAttributeList pkAttributes = vLayer->primaryKeyAttributes();
          if ( pkAttributes.size() > 0 )
          {
            QDomElement pkElem = doc.createElement( QStringLiteral( "PrimaryKey" ) );
            QgsAttributeList::const_iterator pkIt = pkAttributes.constBegin();
            for ( ; pkIt != pkAttributes.constEnd(); ++pkIt )
            {
              QDomElement pkAttributeElem = doc.createElement( QStringLiteral( "PrimaryKeyAttribute" ) );
              QDomText pkAttName = doc.createTextNode( layerFields.at( *pkIt ).name() );
              pkAttributeElem.appendChild( pkAttName );
              pkElem.appendChild( pkAttributeElem );
            }
            layerElem.appendChild( pkElem );
          }

          //geometry type
          layerElem.setAttribute( QStringLiteral( "geometryType" ), QgsWkbTypes::displayString( vLayer->wkbType() ) );

          //opacity
          layerElem.setAttribute( QStringLiteral( "opacity" ), QString::number( vLayer->opacity() ) );

          layerElem.appendChild( attributesElem );
          break;
        }

        case QgsMapLayerType::RasterLayer:
        {
          const QgsDataProvider *provider = currentLayer->dataProvider();
          if ( provider && provider->name() == "wms" )
          {
            //advertise as web map background layer
            QVariant wmsBackgroundLayer = currentLayer->customProperty( QStringLiteral( "WMSBackgroundLayer" ), false );
            QDomElement wmsBackgroundLayerElem = doc.createElement( "WMSBackgroundLayer" );
            QDomText wmsBackgroundLayerText = doc.createTextNode( wmsBackgroundLayer.toBool() ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
            wmsBackgroundLayerElem.appendChild( wmsBackgroundLayerText );
            layerElem.appendChild( wmsBackgroundLayerElem );

            //publish datasource
            QVariant wmsPublishDataSourceUrl = currentLayer->customProperty( QStringLiteral( "WMSPublishDataSourceUrl" ), false );
            if ( wmsPublishDataSourceUrl.toBool() )
            {
              bool tiled = qobject_cast< const QgsRasterDataProvider * >( provider )
                           ? !qobject_cast< const QgsRasterDataProvider * >( provider )->nativeResolutions().isEmpty()
                           : false;

              QDomElement dataSourceElem = doc.createElement( tiled ? QStringLiteral( "WMTSDataSource" ) : QStringLiteral( "WMSDataSource" ) );
              QDomText dataSourceUri = doc.createTextNode( provider->dataSourceUri() );
              dataSourceElem.appendChild( dataSourceUri );
              layerElem.appendChild( dataSourceElem );
            }
          }

          QVariant wmsPrintLayer = currentLayer->customProperty( QStringLiteral( "WMSPrintLayer" ) );
          if ( wmsPrintLayer.isValid() )
          {
            QDomElement wmsPrintLayerElem = doc.createElement( "WMSPrintLayer" );
            QDomText wmsPrintLayerText = doc.createTextNode( wmsPrintLayer.toString() );
            wmsPrintLayerElem.appendChild( wmsPrintLayerText );
            layerElem.appendChild( wmsPrintLayerElem );
          }

          //opacity
          QgsRasterLayer *rl = static_cast<QgsRasterLayer *>( currentLayer );
          QgsRasterRenderer *rasterRenderer = rl->renderer();
          if ( rasterRenderer )
          {
            layerElem.setAttribute( QStringLiteral( "opacity" ), QString::number( rasterRenderer->opacity() ) );
          }
          break;
        }

        case QgsMapLayerType::MeshLayer:
        case QgsMapLayerType::VectorTileLayer:
        case QgsMapLayerType::PluginLayer:
        case QgsMapLayerType::AnnotationLayer:
        case QgsMapLayerType::PointCloudLayer:
        case QgsMapLayerType::GroupLayer:
          break;
      }
    }

    void addKeywordListElement( const QgsProject *project, QDomDocument &doc, QDomElement &parent )
    {
      bool sia2045 = QgsServerProjectUtils::wmsInfoFormatSia2045( *project );

      QDomElement keywordsElem = doc.createElement( QStringLiteral( "KeywordList" ) );
      //add default keyword
      QDomElement keywordElem = doc.createElement( QStringLiteral( "Keyword" ) );
      keywordElem.setAttribute( QStringLiteral( "vocabulary" ), QStringLiteral( "ISO" ) );
      QDomText keywordText = doc.createTextNode( QStringLiteral( "infoMapAccessService" ) );
      keywordElem.appendChild( keywordText );
      keywordsElem.appendChild( keywordElem );
      parent.appendChild( keywordsElem );
      QStringList keywords = QgsServerProjectUtils::owsServiceKeywords( *project );
      for ( const QString &keyword : std::as_const( keywords ) )
      {
        if ( !keyword.isEmpty() )
        {
          keywordElem = doc.createElement( QStringLiteral( "Keyword" ) );
          keywordText = doc.createTextNode( keyword );
          keywordElem.appendChild( keywordText );
          if ( sia2045 )
          {
            keywordElem.setAttribute( QStringLiteral( "vocabulary" ), QStringLiteral( "SIA_Geo405" ) );
          }
          keywordsElem.appendChild( keywordElem );
        }
      }
      parent.appendChild( keywordsElem );
    }
  }

  bool hasQueryableLayers( const QStringList &layerIds, const QMap< QString, QgsWmsLayerInfos > &wmsLayerInfos )
  {
    for ( const QString &id : std::as_const( layerIds ) )
    {
      if ( !wmsLayerInfos.contains( id ) )
      {
        continue;
      }
      if ( wmsLayerInfos[id].queryable )
      {
        return true;
      }
    }
    return false;
  }

  QgsRectangle combineWgs84BoundingRect( const QStringList &layerIds, const QMap< QString, QgsWmsLayerInfos > &wmsLayerInfos )
  {
    QgsRectangle combined;
    bool empty = true;

    for ( const QString &id : std::as_const( layerIds ) )
    {
      if ( !wmsLayerInfos.contains( id ) )
      {
        continue;
      }

      QgsRectangle rect = wmsLayerInfos[ id ].wgs84BoundingRect;
      if ( rect.isNull() )
      {
        continue;
      }

      if ( rect.isEmpty() )
      {
        continue;
      }

      if ( empty )
      {
        combined = rect;
        empty = false;
      }
      else
      {
        combined.combineExtentWith( rect );
      }
    }

    return combined;
  }

  QMap<QString, QgsRectangle> combineCrsExtents( const QStringList &layerIds, const QMap< QString, QgsWmsLayerInfos > &wmsLayerInfos )
  {
    QMap<QString, QgsRectangle> combined;

    for ( const QString &id : std::as_const( layerIds ) )
    {
      if ( !wmsLayerInfos.contains( id ) )
      {
        continue;
      }

      const QgsWmsLayerInfos &layerInfos = wmsLayerInfos[ id ];
      const auto keys = layerInfos.crsExtents.keys();
      for ( const QString &crs : std::as_const( keys ) )
      {
        const QgsRectangle rect = layerInfos.crsExtents[ crs ];
        if ( rect.isNull() )
        {
          continue;
        }

        if ( rect.isEmpty() )
        {
          continue;
        }

        if ( !combined.contains( crs ) )
        {
          combined[ crs ] = rect;
        }
        else
        {
          combined[ crs ].combineExtentWith( rect );
        }
      }
    }

    return combined;
  }

} // namespace QgsWms
