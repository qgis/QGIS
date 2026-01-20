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

#include "qgswmsgetcapabilities.h"

#include "qgsexception.h"
#include "qgsexpressionnodeimpl.h"
#include "qgslayoutatlas.h"
#include "qgslayoutframe.h"
#include "qgslayoutitemhtml.h"
#include "qgslayoutitemlabel.h"
#include "qgslayoutitemmap.h"
#include "qgslayoutmanager.h"
#include "qgslayoutpagecollection.h"
#include "qgsmaplayerserverproperties.h"
#include "qgsmaplayertemporalproperties.h"
#include "qgsprintlayout.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterlayer.h"
#include "qgsrasterrenderer.h"
#include "qgsserverprojectutils.h"
#include "qgsvectorlayer.h"
#include "qgswmsutils.h"

namespace QgsWms
{
  namespace
  {

    void appendLayerProjectSettings( QDomDocument &doc, QDomElement &layerElem, QgsMapLayer *currentLayer );

    void appendDrawingOrder( QDomDocument &doc, QDomElement &parentElem, QgsServerInterface *serverIface, const QgsProject *project );

    void appendLayerWgs84BoundingRect( QDomDocument &doc, QDomElement &layerElement, const QgsRectangle &wgs84BoundingRect );

    void appendLayerCrsExtents( QDomDocument &doc, QDomElement &layerElement, const QMap<QString, QgsRectangle> &crsExtents );

    void appendCrsElementToLayer( QDomDocument &doc, QDomElement &layerElement, const QDomElement &precedingElement, const QString &crsText );

    void appendCrsElementsToLayer( QDomDocument &doc, QDomElement &layerElement, const QStringList &crsList, const QStringList &constrainedCrsList );

    void appendLayerStyles( QDomDocument &doc, QDomElement &layerElem, const QgsWmsLayerInfos &layerInfos, const QgsProject *project, const QgsWmsRequest &request, const QgsServerSettings *settings );

    void appendLayersFromTreeGroup( QDomDocument &doc, QDomElement &parentLayer, QgsServerInterface *serverIface, const QgsProject *project, const QgsWmsRequest &request, const QgsLayerTreeGroup *layerTreeGroup, const QMap<QString, QgsWmsLayerInfos> &wmsLayerInfos, bool projectSettings, QList<QgsDateTimeRange> &parentDateRanges );

    void addKeywordListElement( const QgsProject *project, QDomDocument &doc, QDomElement &parent );
  } // namespace

  void writeGetCapabilities( QgsServerInterface *serverIface, const QgsProject *project, const QgsWmsRequest &request, QgsServerResponse &response, bool projectSettings )
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
    cacheKeyList << ( projectSettings ? u"projectSettings"_s : request.wmsParameters().version() );
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
      QgsMessageLog::logMessage( u"WMS capabilities document not found in cache"_s, u"Server"_s );

      doc = getCapabilities( serverIface, project, request, projectSettings );

#ifdef HAVE_SERVER_PYTHON_PLUGINS
      if ( cacheManager && cacheManager->setCachedDocument( &doc, project, request, accessControl ) )
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
        QgsMessageLog::logMessage( u"Set WMS capabilities document in cache"_s, u"Server"_s );
      }
    }
    else
    {
      QgsMessageLog::logMessage( u"Found WMS capabilities document in cache"_s, u"Server"_s );
    }

    response.setHeader( u"Content-Type"_s, u"text/xml; charset=utf-8"_s );
    response.write( capabilitiesDocument->toByteArray() );
  }

  QDomDocument getCapabilities( QgsServerInterface *serverIface, const QgsProject *project, const QgsWmsRequest &request, bool projectSettings )
  {
    QDomDocument doc;
    QDomElement wmsCapabilitiesElement;

    // Get service URL
    QUrl href = serviceUrl( request, project, *serverIface->serverSettings() );

    //href needs to be a prefix
    QString hrefString = href.toString();
    hrefString.append( href.hasQuery() ? "&" : "?" );

    // XML declaration
    QDomProcessingInstruction xmlDeclaration = doc.createProcessingInstruction( u"xml"_s, u"version=\"1.0\" encoding=\"utf-8\""_s );

    // Append format helper
    std::function<void( QDomElement &, const QString & )> appendFormat = [&doc]( QDomElement &elem, const QString &format ) {
      QDomElement formatElem = doc.createElement( u"Format"_s /*wms:Format*/ );
      formatElem.appendChild( doc.createTextNode( format ) );
      elem.appendChild( formatElem );
    };

    if ( request.wmsParameters().version() == "1.1.1"_L1 )
    {
      doc = QDomDocument( u"WMT_MS_Capabilities SYSTEM 'http://schemas.opengis.net/wms/1.1.1/WMS_MS_Capabilities.dtd'"_s ); //WMS 1.1.1 needs DOCTYPE  "SYSTEM http://schemas.opengis.net/wms/1.1.1/WMS_MS_Capabilities.dtd"
      doc.appendChild( xmlDeclaration );
      wmsCapabilitiesElement = doc.createElement( u"WMT_MS_Capabilities"_s /*wms:WMS_Capabilities*/ );
    }
    else // 1.3.0 as default
    {
      doc.appendChild( xmlDeclaration );
      wmsCapabilitiesElement = doc.createElement( u"WMS_Capabilities"_s /*wms:WMS_Capabilities*/ );
      wmsCapabilitiesElement.setAttribute( u"xmlns"_s, u"http://www.opengis.net/wms"_s );
      wmsCapabilitiesElement.setAttribute( u"xmlns:sld"_s, u"http://www.opengis.net/sld"_s );
      wmsCapabilitiesElement.setAttribute( u"xmlns:qgs"_s, u"http://www.qgis.org/wms"_s );
      wmsCapabilitiesElement.setAttribute( u"xmlns:xsi"_s, u"http://www.w3.org/2001/XMLSchema-instance"_s );
      QString schemaLocation = u"http://www.opengis.net/wms"_s;
      schemaLocation += " http://schemas.opengis.net/wms/1.3.0/capabilities_1_3_0.xsd"_L1;
      schemaLocation += " http://www.opengis.net/sld"_L1;
      schemaLocation += " http://schemas.opengis.net/sld/1.1.0/sld_capabilities.xsd"_L1;

      if ( QgsServerProjectUtils::wmsInspireActivate( *project ) )
      {
        wmsCapabilitiesElement.setAttribute( u"xmlns:inspire_common"_s, u"http://inspire.ec.europa.eu/schemas/common/1.0"_s );
        wmsCapabilitiesElement.setAttribute( u"xmlns:inspire_vs"_s, u"http://inspire.ec.europa.eu/schemas/inspire_vs/1.0"_s );
        schemaLocation += " http://inspire.ec.europa.eu/schemas/inspire_vs/1.0"_L1;
        schemaLocation += " http://inspire.ec.europa.eu/schemas/inspire_vs/1.0/inspire_vs.xsd"_L1;
      }

      schemaLocation += " http://www.qgis.org/wms"_L1;
      schemaLocation += " " + hrefString + "SERVICE=WMS&REQUEST=GetSchemaExtension";

      wmsCapabilitiesElement.setAttribute( u"xsi:schemaLocation"_s, schemaLocation );
    }
    wmsCapabilitiesElement.setAttribute( u"version"_s, request.wmsParameters().version() );
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

  QDomElement getServiceElement( QDomDocument &doc, const QgsProject *project, const QgsWmsRequest &request, const QgsServerSettings *serverSettings )
  {
    //Service element
    QDomElement serviceElem = doc.createElement( u"Service"_s );

    //Service name
    QDomElement nameElem = doc.createElement( u"Name"_s );
    QDomText nameText = doc.createTextNode( u"WMS"_s );
    nameElem.appendChild( nameText );
    serviceElem.appendChild( nameElem );

    // Service title
    QDomElement titleElem = doc.createElement( u"Title"_s );
    QDomText titleText = doc.createTextNode( QgsServerProjectUtils::owsServiceTitle( *project ) );
    titleElem.appendChild( titleText );
    serviceElem.appendChild( titleElem );

    QString abstract = QgsServerProjectUtils::owsServiceAbstract( *project );
    if ( !abstract.isEmpty() )
    {
      QDomElement abstractElem = doc.createElement( u"Abstract"_s );
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
    QDomElement onlineResourceElem = doc.createElement( u"OnlineResource"_s );
    onlineResourceElem.setAttribute( u"xmlns:xlink"_s, u"http://www.w3.org/1999/xlink"_s );
    onlineResourceElem.setAttribute( u"xlink:type"_s, u"simple"_s );
    onlineResourceElem.setAttribute( u"xlink:href"_s, onlineResource );
    serviceElem.appendChild( onlineResourceElem );

    QString contactPerson = QgsServerProjectUtils::owsServiceContactPerson( *project );
    QString contactOrganization = QgsServerProjectUtils::owsServiceContactOrganization( *project );
    QString contactPosition = QgsServerProjectUtils::owsServiceContactPosition( *project );
    QString contactMail = QgsServerProjectUtils::owsServiceContactMail( *project );
    QString contactPhone = QgsServerProjectUtils::owsServiceContactPhone( *project );
    if ( !contactPerson.isEmpty() || !contactOrganization.isEmpty() || !contactPosition.isEmpty() || !contactMail.isEmpty() || !contactPhone.isEmpty() )
    {
      //Contact information
      QDomElement contactInfoElem = doc.createElement( u"ContactInformation"_s );

      //Contact person primary
      if ( !contactPerson.isEmpty() || !contactOrganization.isEmpty() )
      {
        QDomElement contactPersonPrimaryElem = doc.createElement( u"ContactPersonPrimary"_s );

        QDomText contactPersonText;
        if ( !contactPerson.isEmpty() )
        {
          contactPersonText = doc.createTextNode( contactPerson );
        }
        else
        {
          contactPersonText = doc.createTextNode( u"unknown"_s );
        }
        QDomElement contactPersonElem = doc.createElement( u"ContactPerson"_s );
        contactPersonElem.appendChild( contactPersonText );
        contactPersonPrimaryElem.appendChild( contactPersonElem );

        QDomText contactOrganizationText;
        if ( !contactOrganization.isEmpty() )
        {
          contactOrganizationText = doc.createTextNode( contactOrganization );
        }
        else
        {
          contactOrganizationText = doc.createTextNode( u"unknown"_s );
        }
        QDomElement contactOrganizationElem = doc.createElement( u"ContactOrganization"_s );
        contactOrganizationElem.appendChild( contactOrganizationText );
        contactPersonPrimaryElem.appendChild( contactOrganizationElem );

        contactInfoElem.appendChild( contactPersonPrimaryElem );
      }

      if ( !contactPosition.isEmpty() )
      {
        QDomElement contactPositionElem = doc.createElement( u"ContactPosition"_s );
        QDomText contactPositionText = doc.createTextNode( contactPosition );
        contactPositionElem.appendChild( contactPositionText );
        contactInfoElem.appendChild( contactPositionElem );
      }

      if ( !contactPhone.isEmpty() )
      {
        QDomElement phoneElem = doc.createElement( u"ContactVoiceTelephone"_s );
        QDomText phoneText = doc.createTextNode( contactPhone );
        phoneElem.appendChild( phoneText );
        contactInfoElem.appendChild( phoneElem );
      }

      if ( !contactMail.isEmpty() )
      {
        QDomElement mailElem = doc.createElement( u"ContactElectronicMailAddress"_s );
        QDomText mailText = doc.createTextNode( contactMail );
        mailElem.appendChild( mailText );
        contactInfoElem.appendChild( mailElem );
      }

      serviceElem.appendChild( contactInfoElem );
    }

    QDomElement feesElem = doc.createElement( u"Fees"_s );
    QDomText feesText = doc.createTextNode( u"None"_s ); // default value if fees are unknown
    QString fees = QgsServerProjectUtils::owsServiceFees( *project );
    if ( !fees.isEmpty() )
    {
      feesText = doc.createTextNode( fees );
    }
    feesElem.appendChild( feesText );
    serviceElem.appendChild( feesElem );

    QDomElement accessConstraintsElem = doc.createElement( u"AccessConstraints"_s );
    QDomText accessConstraintsText = doc.createTextNode( u"None"_s ); // default value if access constraints are unknown
    QString accessConstraints = QgsServerProjectUtils::owsServiceAccessConstraints( *project );
    if ( !accessConstraints.isEmpty() )
    {
      accessConstraintsText = doc.createTextNode( accessConstraints );
    }
    accessConstraintsElem.appendChild( accessConstraintsText );
    serviceElem.appendChild( accessConstraintsElem );

    if ( request.wmsParameters().version() == "1.3.0"_L1 )
    {
      int maxWidth = QgsServerProjectUtils::wmsMaxWidth( *project );
      if ( maxWidth > 0 )
      {
        QDomElement maxWidthElem = doc.createElement( u"MaxWidth"_s );
        QDomText maxWidthText = doc.createTextNode( QString::number( maxWidth ) );
        maxWidthElem.appendChild( maxWidthText );
        serviceElem.appendChild( maxWidthElem );
      }

      int maxHeight = QgsServerProjectUtils::wmsMaxHeight( *project );
      if ( maxHeight > 0 )
      {
        QDomElement maxHeightElem = doc.createElement( u"MaxHeight"_s );
        QDomText maxHeightText = doc.createTextNode( QString::number( maxHeight ) );
        maxHeightElem.appendChild( maxHeightText );
        serviceElem.appendChild( maxHeightElem );
      }
    }

    return serviceElem;
  }

  QDomElement getCapabilityElement( QDomDocument &doc, const QgsProject *project, const QgsWmsRequest &request, bool projectSettings, QgsServerInterface *serverIface )
  {
    const QString version = request.wmsParameters().version();

    // Get service URL
    QUrl href = serviceUrl( request, project, *serverIface->serverSettings() );

    //href needs to be a prefix
    QString hrefString = href.toString();
    hrefString.append( href.hasQuery() ? "&" : "?" );

    QDomElement capabilityElem = doc.createElement( u"Capability"_s /*wms:Capability*/ );

    //wms:Request element
    QDomElement requestElem = doc.createElement( u"Request"_s /*wms:Request*/ );
    capabilityElem.appendChild( requestElem );

    QDomElement dcpTypeElem = doc.createElement( u"DCPType"_s /*wms:DCPType*/ );
    QDomElement httpElem = doc.createElement( u"HTTP"_s /*wms:HTTP*/ );
    dcpTypeElem.appendChild( httpElem );

    // Append format helper
    std::function<void( QDomElement &, const QString & )> appendFormat = [&doc]( QDomElement &elem, const QString &format ) {
      QDomElement formatElem = doc.createElement( u"Format"_s /*wms:Format*/ );
      formatElem.appendChild( doc.createTextNode( format ) );
      elem.appendChild( formatElem );
    };

    QDomElement elem;

    //wms:GetCapabilities
    elem = doc.createElement( u"GetCapabilities"_s /*wms:GetCapabilities*/ );
    appendFormat( elem, ( version == "1.1.1"_L1 ? "application/vnd.ogc.wms_xml" : "text/xml" ) );
    elem.appendChild( dcpTypeElem );
    requestElem.appendChild( elem );

    //only Get supported for the moment
    QDomElement getElem = doc.createElement( u"Get"_s /*wms:Get*/ );
    httpElem.appendChild( getElem );
    QDomElement olResourceElem = doc.createElement( u"OnlineResource"_s /*wms:OnlineResource*/ );
    olResourceElem.setAttribute( u"xmlns:xlink"_s, u"http://www.w3.org/1999/xlink"_s );
    olResourceElem.setAttribute( u"xlink:type"_s, u"simple"_s );
    olResourceElem.setAttribute( u"xlink:href"_s, hrefString );
    getElem.appendChild( olResourceElem );

    //wms:GetMap
    elem = doc.createElement( u"GetMap"_s /*wms:GetMap*/ );
    appendFormat( elem, u"image/png"_s ); //QGIS Desktop uses first advertised format as default, png supports transparency
    appendFormat( elem, u"image/png; mode=16bit"_s );
    appendFormat( elem, u"image/png; mode=8bit"_s );
    appendFormat( elem, u"image/png; mode=1bit"_s );
    appendFormat( elem, u"image/jpeg"_s );
    appendFormat( elem, u"application/dxf"_s );
    appendFormat( elem, u"application/pdf"_s );
    elem.appendChild( dcpTypeElem.cloneNode().toElement() ); //this is the same as for 'GetCapabilities'
    requestElem.appendChild( elem );

    //wms:GetFeatureInfo
    elem = doc.createElement( u"GetFeatureInfo"_s );
    appendFormat( elem, u"text/plain"_s );
    appendFormat( elem, u"text/html"_s );
    appendFormat( elem, u"text/xml"_s );
    appendFormat( elem, u"application/vnd.ogc.gml"_s );
    appendFormat( elem, u"application/vnd.ogc.gml/3.1.1"_s );
    appendFormat( elem, u"application/json"_s );
    appendFormat( elem, u"application/geo+json"_s );
    elem.appendChild( dcpTypeElem.cloneNode().toElement() ); //this is the same as for 'GetCapabilities'
    requestElem.appendChild( elem );

    //wms:GetLegendGraphic
    elem = doc.createElement( ( version == "1.1.1"_L1 ? "GetLegendGraphic" : "sld:GetLegendGraphic" ) /*wms:GetLegendGraphic*/ );
    appendFormat( elem, u"image/jpeg"_s );
    appendFormat( elem, u"image/png"_s );
    appendFormat( elem, u"application/json"_s );
    elem.appendChild( dcpTypeElem.cloneNode().toElement() ); //this is the same as for 'GetCapabilities'
    requestElem.appendChild( elem );

    //wms:DescribeLayer
    elem = doc.createElement( ( version == "1.1.1"_L1 ? "DescribeLayer" : "sld:DescribeLayer" ) /*wms:GetLegendGraphic*/ );
    appendFormat( elem, u"text/xml"_s );
    elem.appendChild( dcpTypeElem.cloneNode().toElement() ); //this is the same as for 'GetCapabilities'
    requestElem.appendChild( elem );

    //wms:GetStyles
    elem = doc.createElement( ( version == "1.1.1"_L1 ? "GetStyles" : "qgs:GetStyles" ) /*wms:GetStyles*/ );
    appendFormat( elem, u"text/xml"_s );
    elem.appendChild( dcpTypeElem.cloneNode().toElement() ); //this is the same as for 'GetCapabilities'
    requestElem.appendChild( elem );

    if ( ( !serverIface->serverSettings() || !serverIface->serverSettings()->getPrintDisabled() ) && projectSettings ) //remove composer templates from GetCapabilities in the long term
    {
      //wms:GetPrint
      elem = doc.createElement( u"GetPrint"_s /*wms:GetPrint*/ );
      appendFormat( elem, u"svg"_s );
      appendFormat( elem, u"png"_s );
      appendFormat( elem, u"pdf"_s );
      elem.appendChild( dcpTypeElem.cloneNode().toElement() ); //this is the same as for 'GetCapabilities'
      requestElem.appendChild( elem );
    }

    //Exception element is mandatory
    elem = doc.createElement( u"Exception"_s );
    appendFormat( elem, ( version == "1.1.1"_L1 ? "application/vnd.ogc.se_xml" : "XML" ) );
    capabilityElem.appendChild( elem );

    //UserDefinedSymbolization element
    if ( version == "1.3.0"_L1 )
    {
      elem = doc.createElement( u"sld:UserDefinedSymbolization"_s );
      elem.setAttribute( u"SupportSLD"_s, u"1"_s );
      elem.setAttribute( u"UserLayer"_s, u"0"_s );
      elem.setAttribute( u"UserStyle"_s, u"1"_s );
      elem.setAttribute( u"RemoteWFS"_s, u"0"_s );
      elem.setAttribute( u"InlineFeature"_s, u"0"_s );
      elem.setAttribute( u"RemoteWCS"_s, u"0"_s );
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

    inspireCapabilitiesElem = doc.createElement( u"inspire_vs:ExtendedCapabilities"_s );

    QString inspireMetadataUrl = QgsServerProjectUtils::wmsInspireMetadataUrl( *project );
    // inspire scenario 1
    if ( !inspireMetadataUrl.isEmpty() )
    {
      QDomElement inspireCommonMetadataUrlElem = doc.createElement( u"inspire_common:MetadataUrl"_s );
      inspireCommonMetadataUrlElem.setAttribute( u"xsi:type"_s, u"inspire_common:resourceLocatorType"_s );

      QDomElement inspireCommonMetadataUrlUrlElem = doc.createElement( u"inspire_common:URL"_s );
      inspireCommonMetadataUrlUrlElem.appendChild( doc.createTextNode( inspireMetadataUrl ) );
      inspireCommonMetadataUrlElem.appendChild( inspireCommonMetadataUrlUrlElem );

      QString inspireMetadataUrlType = QgsServerProjectUtils::wmsInspireMetadataUrlType( *project );
      if ( !inspireMetadataUrlType.isNull() )
      {
        QDomElement inspireCommonMetadataUrlMediaTypeElem = doc.createElement( u"inspire_common:MediaType"_s );
        inspireCommonMetadataUrlMediaTypeElem.appendChild( doc.createTextNode( inspireMetadataUrlType ) );
        inspireCommonMetadataUrlElem.appendChild( inspireCommonMetadataUrlMediaTypeElem );
      }

      inspireCapabilitiesElem.appendChild( inspireCommonMetadataUrlElem );
    }
    else
    {
      QDomElement inspireCommonResourceTypeElem = doc.createElement( u"inspire_common:ResourceType"_s );
      inspireCommonResourceTypeElem.appendChild( doc.createTextNode( u"service"_s ) );
      inspireCapabilitiesElem.appendChild( inspireCommonResourceTypeElem );

      QDomElement inspireCommonSpatialDataServiceTypeElem = doc.createElement( u"inspire_common:SpatialDataServiceType"_s );
      inspireCommonSpatialDataServiceTypeElem.appendChild( doc.createTextNode( u"view"_s ) );
      inspireCapabilitiesElem.appendChild( inspireCommonSpatialDataServiceTypeElem );

      QString inspireTemporalReference = QgsServerProjectUtils::wmsInspireTemporalReference( *project );
      if ( !inspireTemporalReference.isNull() )
      {
        QDomElement inspireCommonTemporalReferenceElem = doc.createElement( u"inspire_common:TemporalReference"_s );
        QDomElement inspireCommonDateOfLastRevisionElem = doc.createElement( u"inspire_common:DateOfLastRevision"_s );
        inspireCommonDateOfLastRevisionElem.appendChild( doc.createTextNode( inspireTemporalReference ) );
        inspireCommonTemporalReferenceElem.appendChild( inspireCommonDateOfLastRevisionElem );
        inspireCapabilitiesElem.appendChild( inspireCommonTemporalReferenceElem );
      }

      QDomElement inspireCommonMetadataPointOfContactElem = doc.createElement( u"inspire_common:MetadataPointOfContact"_s );

      QString contactOrganization = QgsServerProjectUtils::owsServiceContactOrganization( *project );
      QDomElement inspireCommonOrganisationNameElem = doc.createElement( u"inspire_common:OrganisationName"_s );
      if ( !contactOrganization.isNull() )
      {
        inspireCommonOrganisationNameElem.appendChild( doc.createTextNode( contactOrganization ) );
      }
      inspireCommonMetadataPointOfContactElem.appendChild( inspireCommonOrganisationNameElem );

      QString contactMail = QgsServerProjectUtils::owsServiceContactMail( *project );
      QDomElement inspireCommonEmailAddressElem = doc.createElement( u"inspire_common:EmailAddress"_s );
      if ( !contactMail.isNull() )
      {
        inspireCommonEmailAddressElem.appendChild( doc.createTextNode( contactMail ) );
      }
      inspireCommonMetadataPointOfContactElem.appendChild( inspireCommonEmailAddressElem );

      inspireCapabilitiesElem.appendChild( inspireCommonMetadataPointOfContactElem );

      QString inspireMetadataDate = QgsServerProjectUtils::wmsInspireMetadataDate( *project );
      if ( !inspireMetadataDate.isNull() )
      {
        QDomElement inspireCommonMetadataDateElem = doc.createElement( u"inspire_common:MetadataDate"_s );
        inspireCommonMetadataDateElem.appendChild( doc.createTextNode( inspireMetadataDate ) );
        inspireCapabilitiesElem.appendChild( inspireCommonMetadataDateElem );
      }
    }

    // Supported languages
    QDomElement inspireCommonSupportedLanguagesElem = doc.createElement( u"inspire_common:SupportedLanguages"_s );
    inspireCommonSupportedLanguagesElem.setAttribute( u"xsi:type"_s, u"inspire_common:supportedLanguagesType"_s );

    QDomElement inspireCommonLanguageElem = doc.createElement( u"inspire_common:Language"_s );
    inspireCommonLanguageElem.appendChild( doc.createTextNode( QgsServerProjectUtils::wmsInspireLanguage( *project ) ) );

    QDomElement inspireCommonDefaultLanguageElem = doc.createElement( u"inspire_common:DefaultLanguage"_s );
    inspireCommonDefaultLanguageElem.appendChild( inspireCommonLanguageElem );
    inspireCommonSupportedLanguagesElem.appendChild( inspireCommonDefaultLanguageElem );

#if 0
    /* Supported language has to be different from default one */
    QDomElement inspireCommonSupportedLanguageElem = doc.createElement( "inspire_common:SupportedLanguage" );
    inspireCommonSupportedLanguageElem.appendChild( inspireCommonLanguageElem.cloneNode().toElement() );
    inspireCommonSupportedLanguagesElem.appendChild( inspireCommonSupportedLanguageElem );
#endif

    inspireCapabilitiesElem.appendChild( inspireCommonSupportedLanguagesElem );

    QDomElement inspireCommonResponseLanguageElem = doc.createElement( u"inspire_common:ResponseLanguage"_s );
    inspireCommonResponseLanguageElem.appendChild( inspireCommonLanguageElem.cloneNode().toElement() );
    inspireCapabilitiesElem.appendChild( inspireCommonResponseLanguageElem );

    return inspireCapabilitiesElem;
  }

  QDomElement getComposerTemplatesElement( QDomDocument &doc, const QgsProject *project )
  {
    QList<QgsPrintLayout *> projectComposers = project->layoutManager()->printLayouts();
    if ( projectComposers.size() == 0 )
      return QDomElement();

    QStringList restrictedComposers = QgsServerProjectUtils::wmsRestrictedComposers( *project );

    QDomElement composerTemplatesElem = doc.createElement( u"ComposerTemplates"_s );
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
      QgsLayoutMeasurement width( layout->convertFromLayoutUnits( layoutSize.width(), Qgis::LayoutUnit::Millimeters ) );
      QgsLayoutMeasurement height( layout->convertFromLayoutUnits( layoutSize.height(), Qgis::LayoutUnit::Millimeters ) );

      QDomElement composerTemplateElem = doc.createElement( u"ComposerTemplate"_s );
      composerTemplateElem.setAttribute( u"name"_s, layout->name() );

      //get paper width and height in mm from composition
      composerTemplateElem.setAttribute( u"width"_s, width.length() );
      composerTemplateElem.setAttribute( u"height"_s, height.length() );

      //atlas enabled and atlas covering layer
      QgsLayoutAtlas *atlas = layout->atlas();
      if ( atlas && atlas->enabled() )
      {
        composerTemplateElem.setAttribute( u"atlasEnabled"_s, u"1"_s );
        QgsVectorLayer *cLayer = atlas->coverageLayer();
        if ( cLayer )
        {
          QString layerName = cLayer->serverProperties()->shortName();
          if ( QgsServerProjectUtils::wmsUseLayerIds( *project ) )
          {
            layerName = cLayer->id();
          }
          else if ( layerName.isEmpty() )
          {
            layerName = cLayer->name();
          }
          composerTemplateElem.setAttribute( u"atlasCoverageLayer"_s, layerName );
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

        QDomElement composerMapElem = doc.createElement( u"ComposerMap"_s );
        composerMapElem.setAttribute( u"name"_s, u"map%1"_s.arg( mapId ) );
        composerMapElem.setAttribute( u"itemName"_s, composerMap->displayName() );
        mapId++;
        composerMapElem.setAttribute( u"width"_s, composerMap->rect().width() );
        composerMapElem.setAttribute( u"height"_s, composerMap->rect().height() );
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

        QDomElement composerLabelElem = doc.createElement( u"ComposerLabel"_s );
        composerLabelElem.setAttribute( u"name"_s, id );
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

        QDomElement composerHtmlElem = doc.createElement( u"ComposerHtml"_s );
        composerHtmlElem.setAttribute( u"name"_s, id );
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

    QDomElement wfsLayersElem = doc.createElement( u"WFSLayers"_s );
    for ( int i = 0; i < wfsLayerIds.size(); ++i )
    {
      QgsMapLayer *layer = project->mapLayer( wfsLayerIds.at( i ) );
      if ( !layer || layer->type() != Qgis::LayerType::Vector )
      {
        continue;
      }

      QDomElement wfsLayerElem = doc.createElement( u"WFSLayer"_s );
      if ( QgsServerProjectUtils::wmsUseLayerIds( *project ) )
      {
        wfsLayerElem.setAttribute( u"name"_s, layer->id() );
      }
      else
      {
        wfsLayerElem.setAttribute( u"name"_s, layer->name() );
      }
      wfsLayersElem.appendChild( wfsLayerElem );
    }

    return wfsLayersElem;
  }

  void handleLayersFromTreeGroup( QDomDocument &doc, QDomElement &parentLayer, QgsServerInterface *serverIface, const QgsProject *project, const QgsWmsRequest &request, const QgsLayerTreeGroup *layerTreeGroup, const QMap<QString, QgsWmsLayerInfos> &wmsLayerInfos, bool projectSettings, QList<QgsDateTimeRange> &parentDateRanges )
  {
    const auto layerIds = layerTreeGroup->findLayerIds();

    parentLayer.setAttribute(
      u"queryable"_s,
      hasQueryableLayers( layerIds, wmsLayerInfos ) ? u"1"_s : u"0"_s
    );

    const QgsRectangle wgs84BoundingRect = combineWgs84BoundingRect( layerIds, wmsLayerInfos );
    QMap<QString, QgsRectangle> crsExtents = combineCrsExtents( layerIds, wmsLayerInfos );

    appendCrsElementsToLayer( doc, parentLayer, crsExtents.keys(), QStringList() );
    appendLayerWgs84BoundingRect( doc, parentLayer, wgs84BoundingRect );
    appendLayerCrsExtents( doc, parentLayer, crsExtents );

    appendLayersFromTreeGroup( doc, parentLayer, serverIface, project, request, layerTreeGroup, wmsLayerInfos, projectSettings, parentDateRanges );
  }

  QDomElement getLayersAndStylesCapabilitiesElement( QDomDocument &doc, QgsServerInterface *serverIface, const QgsProject *project, const QgsWmsRequest &request, bool projectSettings )
  {
    const QgsLayerTree *projectLayerTreeRoot = project->layerTreeRoot();

    QDomElement layerParentElem = doc.createElement( u"Layer"_s );

    // Root Layer name
    QString rootLayerName = QgsServerProjectUtils::wmsRootName( *project );
    if ( rootLayerName.isEmpty() && !project->title().isEmpty() )
    {
      rootLayerName = project->title();
    }

    if ( !rootLayerName.isEmpty() )
    {
      QDomElement layerParentNameElem = doc.createElement( u"Name"_s );
      QDomText layerParentNameText = doc.createTextNode( rootLayerName );
      layerParentNameElem.appendChild( layerParentNameText );
      layerParentElem.appendChild( layerParentNameElem );
    }

    // Root Layer title
    QDomElement layerParentTitleElem = doc.createElement( u"Title"_s );
    QDomText layerParentTitleText = doc.createTextNode( QgsServerProjectUtils::owsServiceTitle( *project ) );
    layerParentTitleElem.appendChild( layerParentTitleText );
    layerParentElem.appendChild( layerParentTitleElem );

    // Root Layer abstract
    const QString rootLayerAbstract = QgsServerProjectUtils::owsServiceAbstract( *project );
    if ( !rootLayerAbstract.isEmpty() )
    {
      QDomElement layerParentAbstElem = doc.createElement( u"Abstract"_s );
      QDomText layerParentAbstText = doc.createCDATASection( rootLayerAbstract );
      layerParentAbstElem.appendChild( layerParentAbstText );
      layerParentElem.appendChild( layerParentAbstElem );
    }

    // Keyword list
    addKeywordListElement( project, doc, layerParentElem );

    // Root Layer tree name
    if ( projectSettings )
    {
      QDomElement treeNameElem = doc.createElement( u"TreeName"_s );
      QDomText treeNameText = doc.createTextNode( project->title() );
      treeNameElem.appendChild( treeNameText );
      layerParentElem.appendChild( treeNameElem );
    }

    // Instantiate CRS's from the project's crs list
    // This will prevent us to re-instantiate all the crs's each
    // time we will need to rebuild a bounding box.
    auto outputCrsList = QList<QgsCoordinateReferenceSystem>();
    for ( const QString &crsDef : QgsServerProjectUtils::wmsOutputCrsList( *project ) )
    {
      const auto crs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( crsDef );
      if ( crs.isValid() )
      {
        outputCrsList.append( crs );
      }
    }

    // Get WMS layer infos
    const QMap<QString, QgsWmsLayerInfos> wmsLayerInfos = QgsWmsLayerInfos::buildWmsLayerInfos( serverIface, project, outputCrsList );

    const QgsRectangle wmsExtent = QgsServerProjectUtils::wmsExtent( *project );

    if ( !wmsExtent.isEmpty() )
    {
      const QgsCoordinateReferenceSystem wgs84 = QgsCoordinateReferenceSystem::fromOgcWmsCrs( Qgis::geographicCrsAuthId() );

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
          u"Error transforming extent: %1"_s.arg( cse.what() ),
          u"Server"_s,
          Qgis::MessageLevel::Warning
        );
      }

      // Get WMS extents in output CRSes
      QMap<QString, QgsRectangle> wmsCrsExtents;
      try
      {
        wmsCrsExtents = QgsWmsLayerInfos::transformExtentToCrsList(
          wmsExtent, project->crs(), outputCrsList, project->transformContext()
        );
      }
      catch ( QgsCsException &cse )
      {
        QgsMessageLog::logMessage( u"Error transforming extent: %1"_s.arg( cse.what() ), u"Server"_s, Qgis::MessageLevel::Warning );
      }

      layerParentElem.setAttribute(
        u"queryable"_s,
        hasQueryableLayers( projectLayerTreeRoot->findLayerIds(), wmsLayerInfos ) ? u"1"_s : u"0"_s
      );

      appendCrsElementsToLayer( doc, layerParentElem, wmsCrsExtents.keys(), QStringList() );
      appendLayerWgs84BoundingRect( doc, layerParentElem, wmsWgs84BoundingRect );
      appendLayerCrsExtents( doc, layerParentElem, wmsCrsExtents );

      QList<QgsDateTimeRange> parentDateRanges;
      appendLayersFromTreeGroup( doc, layerParentElem, serverIface, project, request, projectLayerTreeRoot, wmsLayerInfos, projectSettings, parentDateRanges );
    }
    else
    {
      QList<QgsDateTimeRange> parentDateRanges;
      handleLayersFromTreeGroup( doc, layerParentElem, serverIface, project, request, projectLayerTreeRoot, wmsLayerInfos, projectSettings, parentDateRanges );
    }

    return layerParentElem;
  }

  namespace
  {
    //! helper method to write all server properties except
    // - name: because it's differently managed between group and layer
    // - legendUrl because it's part of styles
    void writeServerProperties( QDomDocument &doc, QDomElement &layerElem, const QgsProject *project, const QgsMapLayerServerProperties *serverProperties, const QString &name, const QString &version )
    {
      const QString title = serverProperties->title();
      QDomElement titleElem = doc.createElement( u"Title"_s );
      QDomText titleText = doc.createTextNode( !title.isEmpty() ? title : name );
      titleElem.appendChild( titleText );
      layerElem.appendChild( titleElem );

      const QString abstract = serverProperties->abstract();
      if ( !abstract.isEmpty() )
      {
        QDomElement abstractElem = doc.createElement( u"Abstract"_s );
        QDomText abstractText = doc.createTextNode( abstract );
        abstractElem.appendChild( abstractText );
        layerElem.appendChild( abstractElem );
      }

      //keyword list
      const bool siaFormat = QgsServerProjectUtils::wmsInfoFormatSia2045( *project );
      const QStringList keywords = !serverProperties->keywordList().isEmpty() ? serverProperties->keywordList().split( ',' ) : QStringList();
      if ( !keywords.isEmpty() )
      {
        QDomElement keywordListElem = doc.createElement( u"KeywordList"_s );
        for ( const QString &keyword : std::as_const( keywords ) )
        {
          QDomElement keywordElem = doc.createElement( u"Keyword"_s );
          QDomText keywordText = doc.createTextNode( keyword.trimmed() );
          keywordElem.appendChild( keywordText );
          if ( siaFormat )
          {
            keywordElem.setAttribute( u"vocabulary"_s, u"SIA_Geo405"_s );
          }
          keywordListElem.appendChild( keywordElem );
        }
        layerElem.appendChild( keywordListElem );
      }

      // layer data URL
      const QString dataUrl = serverProperties->dataUrl();
      if ( !dataUrl.isEmpty() )
      {
        QDomElement dataUrlElem = doc.createElement( u"DataURL"_s );
        QDomElement dataUrlFormatElem = doc.createElement( u"Format"_s );
        const QString dataUrlFormat = serverProperties->dataUrlFormat();
        QDomText dataUrlFormatText = doc.createTextNode( dataUrlFormat );
        dataUrlFormatElem.appendChild( dataUrlFormatText );
        dataUrlElem.appendChild( dataUrlFormatElem );
        QDomElement dataORElem = doc.createElement( u"OnlineResource"_s );
        dataORElem.setAttribute( u"xmlns:xlink"_s, u"http://www.w3.org/1999/xlink"_s );
        dataORElem.setAttribute( u"xlink:type"_s, u"simple"_s );
        dataORElem.setAttribute( u"xlink:href"_s, dataUrl );
        dataUrlElem.appendChild( dataORElem );
        layerElem.appendChild( dataUrlElem );
      }

      // layer attribution
      const QString attribution = serverProperties->attribution();
      if ( !attribution.isEmpty() )
      {
        QDomElement attribElem = doc.createElement( u"Attribution"_s );
        QDomElement attribTitleElem = doc.createElement( u"Title"_s );
        QDomText attribText = doc.createTextNode( attribution );
        attribTitleElem.appendChild( attribText );
        attribElem.appendChild( attribTitleElem );
        const QString attributionUrl = serverProperties->attributionUrl();
        if ( !attributionUrl.isEmpty() )
        {
          QDomElement attribORElem = doc.createElement( u"OnlineResource"_s );
          attribORElem.setAttribute( u"xmlns:xlink"_s, u"http://www.w3.org/1999/xlink"_s );
          attribORElem.setAttribute( u"xlink:type"_s, u"simple"_s );
          attribORElem.setAttribute( u"xlink:href"_s, attributionUrl );
          attribElem.appendChild( attribORElem );
        }
        layerElem.appendChild( attribElem );
      }

      // layer metadata URL
      const QList<QgsServerMetadataUrlProperties::MetadataUrl> metadataUrls = serverProperties->metadataUrls();
      for ( const QgsMapLayerServerProperties::MetadataUrl &metadataUrl : std::as_const( metadataUrls ) )
      {
        QDomElement metaUrlElem = doc.createElement( u"MetadataURL"_s );
        const QString metadataUrlType = metadataUrl.type;
        if ( version == "1.1.1"_L1 )
        {
          metaUrlElem.setAttribute( u"type"_s, metadataUrlType );
        }
        else if ( metadataUrlType == "FGDC"_L1 )
        {
          metaUrlElem.setAttribute( u"type"_s, u"FGDC:1998"_s );
        }
        else if ( metadataUrlType == "TC211"_L1 )
        {
          metaUrlElem.setAttribute( u"type"_s, u"ISO19115:2003"_s );
        }
        else
        {
          metaUrlElem.setAttribute( u"type"_s, metadataUrlType );
        }
        const QString metadataUrlFormat = metadataUrl.format;
        if ( !metadataUrlFormat.isEmpty() )
        {
          QDomElement metaUrlFormatElem = doc.createElement( u"Format"_s );
          QDomText metaUrlFormatText = doc.createTextNode( metadataUrlFormat );
          metaUrlFormatElem.appendChild( metaUrlFormatText );
          metaUrlElem.appendChild( metaUrlFormatElem );
        }
        QDomElement metaUrlORElem = doc.createElement( u"OnlineResource"_s );
        metaUrlORElem.setAttribute( u"xmlns:xlink"_s, u"http://www.w3.org/1999/xlink"_s );
        metaUrlORElem.setAttribute( u"xlink:type"_s, u"simple"_s );
        metaUrlORElem.setAttribute( u"xlink:href"_s, metadataUrl.url );
        metaUrlElem.appendChild( metaUrlORElem );
        layerElem.appendChild( metaUrlElem );
      }
    }

    void writeLegendUrl( QDomDocument &doc, QDomElement &styleElem, const QString &legendUrl, const QString &legendUrlFormat, const QString &name, const QString &styleName, const QgsProject *project, const QgsWmsRequest &request, const QgsServerSettings *settings )
    {
      // QString LegendURL for explicit layerbased GetLegendGraphic request
      QDomElement getLayerLegendGraphicElem = doc.createElement( u"LegendURL"_s );

      QString customHrefString = legendUrl;

      QStringList getLayerLegendGraphicFormats;
      if ( !customHrefString.isEmpty() )
      {
        getLayerLegendGraphicFormats << legendUrlFormat;
      }
      else
      {
        getLayerLegendGraphicFormats << u"image/png"_s; // << "jpeg" << "image/jpeg"
      }

      for ( const QString &getLayerLegendGraphicFormat : std::as_const( getLayerLegendGraphicFormats ) )
      {
        QDomElement getLayerLegendGraphicFormatElem = doc.createElement( u"Format"_s );
        QDomText getLayerLegendGraphicFormatText = doc.createTextNode( getLayerLegendGraphicFormat );
        getLayerLegendGraphicFormatElem.appendChild( getLayerLegendGraphicFormatText );
        getLayerLegendGraphicElem.appendChild( getLayerLegendGraphicFormatElem );
      }

      // no parameters on custom hrefUrl, because should link directly to graphic
      if ( customHrefString.isEmpty() )
      {
        // href needs to be a prefix
        QUrl href = serviceUrl( request, project, *settings );
        const QString hrefString = href.toString() + ( href.hasQuery() ? "&" : "?" );

        QUrl mapUrl( hrefString );
        QUrlQuery mapUrlQuery( mapUrl.query() );
        mapUrlQuery.addQueryItem( u"SERVICE"_s, u"WMS"_s );
        mapUrlQuery.addQueryItem( u"VERSION"_s, request.wmsParameters().version() );
        mapUrlQuery.addQueryItem( u"REQUEST"_s, u"GetLegendGraphic"_s );
        mapUrlQuery.addQueryItem( u"LAYER"_s, name );
        mapUrlQuery.addQueryItem( u"FORMAT"_s, u"image/png"_s );
        mapUrlQuery.addQueryItem( u"STYLE"_s, styleName );
        if ( request.wmsParameters().version() == "1.3.0"_L1 )
        {
          mapUrlQuery.addQueryItem( u"SLD_VERSION"_s, u"1.1.0"_s );
        }
        mapUrl.setQuery( mapUrlQuery );
        customHrefString = mapUrl.toString();
      }

      QDomElement getLayerLegendGraphicORElem = doc.createElement( u"OnlineResource"_s );
      getLayerLegendGraphicORElem.setAttribute( u"xmlns:xlink"_s, u"http://www.w3.org/1999/xlink"_s );
      getLayerLegendGraphicORElem.setAttribute( u"xlink:type"_s, u"simple"_s );
      getLayerLegendGraphicORElem.setAttribute( u"xlink:href"_s, customHrefString );
      getLayerLegendGraphicElem.appendChild( getLayerLegendGraphicORElem );
      styleElem.appendChild( getLayerLegendGraphicElem );
    }

    QDomElement createStyleElement( QDomDocument &doc, const QString &styleName )
    {
      QDomElement styleElem = doc.createElement( u"Style"_s );
      QDomElement styleNameElem = doc.createElement( u"Name"_s );
      QDomText styleNameText = doc.createTextNode( styleName );
      styleNameElem.appendChild( styleNameText );
      QDomElement styleTitleElem = doc.createElement( u"Title"_s );
      QDomText styleTitleText = doc.createTextNode( styleName );
      styleTitleElem.appendChild( styleTitleText );
      styleElem.appendChild( styleNameElem );
      styleElem.appendChild( styleTitleElem );

      return styleElem;
    }

    //! Return TRUE if date only have been written, FALSE if there is datetime
    bool writeTimeDimensionNode( QDomDocument &doc, QDomElement &layerElem, const QList<QgsDateTimeRange> &dateRanges )
    {
      // Apparently, for vectors allTemporalRanges is always empty :/
      // there is no way to know the type of range or the individual instants

      // we write a TIME dimension even if dateRanges is empty. Not sure this is appropriate but
      // it was like that from the beginning so better keep it that way to avoid regression on client side

      const bool hasDateTime = std::any_of( dateRanges.constBegin(), dateRanges.constEnd(), []( const QgsDateTimeRange &r ) { return r.begin().time() != QTime( 0, 0 )
                                                                                                                                     || ( !r.isInstant() && r.end().time() != QTime( 0, 0 ) ); } );

      const QString dateFormat = hasDateTime ? u"yyyy-MM-ddTHH:mm:ss"_s : u"yyyy-MM-dd"_s;

      QStringList strValues;
      for ( const QgsDateTimeRange &range : dateRanges )
      {
        // Standard ISO8601 doesn't support range with no defined begin or end
        if ( range.begin().isValid() && range.end().isValid() )
        {
          strValues << ( range.isInstant() ? range.begin().toString( dateFormat ) : u"%1/%2"_s.arg( range.begin().toString( dateFormat ) ).arg( range.end().toString( dateFormat ) ) );
        }
      }

      QDomElement dimElem = doc.createElement( u"Dimension"_s );
      dimElem.setAttribute( u"name"_s, u"TIME"_s );
      dimElem.setAttribute( u"units"_s, u"ISO8601"_s );
      QDomText dimValuesText = doc.createTextNode( strValues.join( QChar( ',' ) ) );
      dimElem.appendChild( dimValuesText );

      layerElem.appendChild( dimElem );

      return !hasDateTime;
    }

    void appendLayersFromTreeGroup( QDomDocument &doc, QDomElement &parentLayer, QgsServerInterface *serverIface, const QgsProject *project, const QgsWmsRequest &request, const QgsLayerTreeGroup *layerTreeGroup, const QMap<QString, QgsWmsLayerInfos> &wmsLayerInfos, bool projectSettings, QList<QgsDateTimeRange> &parentDateRanges )
    {
      const QString version = request.wmsParameters().version();

      const QStringList restrictedLayers = QgsServerProjectUtils::wmsRestrictedLayers( *project );
      const bool skipNameForGroup = QgsServerProjectUtils::wmsSkipNameForGroup( *project );

      QList<QgsLayerTreeNode *> layerTreeGroupChildren = layerTreeGroup->children();
      for ( int i = 0; i < layerTreeGroupChildren.size(); ++i )
      {
        QgsLayerTreeNode *treeNode = layerTreeGroupChildren.at( i );
        QDomElement layerElem = doc.createElement( u"Layer"_s );

        if ( projectSettings )
        {
          layerElem.setAttribute( u"visible"_s, treeNode->isVisible() );
          layerElem.setAttribute( u"visibilityChecked"_s, treeNode->itemVisibilityChecked() );
          layerElem.setAttribute( u"expanded"_s, treeNode->isExpanded() );
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
            layerElem.setAttribute( u"mutuallyExclusive"_s, treeGroupChild->isMutuallyExclusive() );
          }

          const QString shortName = treeGroupChild->serverProperties()->shortName();

          if ( !skipNameForGroup )
          {
            QDomElement nameElem = doc.createElement( u"Name"_s );
            QDomText nameText;
            if ( !shortName.isEmpty() )
              nameText = doc.createTextNode( shortName );
            else
              nameText = doc.createTextNode( name );
            nameElem.appendChild( nameText );
            layerElem.appendChild( nameElem );
          }

          writeServerProperties( doc, layerElem, project, treeGroupChild->serverProperties(), name, version );

          // There is no style assicated with layer tree group so just use a default one
          const QString styleName = u"default"_s;
          QDomElement styleElem = createStyleElement( doc, styleName );
          writeLegendUrl( doc, styleElem, treeGroupChild->serverProperties()->legendUrl(), treeGroupChild->serverProperties()->legendUrlFormat(), name, styleName, project, request, serverIface->serverSettings() );

          layerElem.appendChild( styleElem );

          // Layer tree name
          if ( projectSettings )
          {
            QDomElement treeNameElem = doc.createElement( u"TreeName"_s );
            QDomText treeNameText = doc.createTextNode( name );
            treeNameElem.appendChild( treeNameText );
            layerElem.appendChild( treeNameElem );
          }

          QList<QgsDateTimeRange> childrenDateRanges;
          handleLayersFromTreeGroup( doc, layerElem, serverIface, project, request, treeGroupChild, wmsLayerInfos, projectSettings, childrenDateRanges );

          if ( treeGroupChild->hasWmsTimeDimension() )
          {
            writeTimeDimensionNode( doc, layerElem, childrenDateRanges );
            parentDateRanges.append( childrenDateRanges );
          }

          // Check if child layer elements have been added
          if ( layerElem.elementsByTagName( u"Layer"_s ).length() == 0 )
          {
            continue;
          }
        }
        else
        {
          QgsLayerTreeLayer *treeLayer = static_cast<QgsLayerTreeLayer *>( treeNode );
          QgsMapLayer *l = treeLayer->layer();
          if ( !wmsLayerInfos.contains( treeLayer->layerId() ) ) //unpublished layer
          {
            continue;
          }

          const QgsWmsLayerInfos &layerInfos = wmsLayerInfos[treeLayer->layerId()];

          layerElem.setAttribute(
            u"queryable"_s,
            layerInfos.queryable ? u"1"_s : u"0"_s
          );

          QDomElement nameElem = doc.createElement( u"Name"_s );
          QDomText nameText = doc.createTextNode( layerInfos.name );
          nameElem.appendChild( nameText );
          layerElem.appendChild( nameElem );

          writeServerProperties( doc, layerElem, project, l->serverProperties(), l->name(), version );

          // Append not null Bounding rectangles
          if ( !layerInfos.wgs84BoundingRect.isNull() )
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
            // Convert double to string and remove trailing zero and last point if present
            auto formatScale = []( double value ) {
              const thread_local QRegularExpression trailingZeroRegEx = QRegularExpression( u"0+$"_s );
              const thread_local QRegularExpression trailingPointRegEx = QRegularExpression( u"[.]+$"_s );
              return QString::number( value, 'f' ).remove( trailingZeroRegEx ).remove( trailingPointRegEx );
            };

            if ( version == "1.1.1"_L1 )
            {
              double OGC_PX_M = 0.00028; // OGC reference pixel size in meter, also used by qgis
              double SCALE_TO_SCALEHINT = OGC_PX_M * M_SQRT2;

              QDomElement scaleHintElem = doc.createElement( u"ScaleHint"_s );
              scaleHintElem.setAttribute( u"min"_s, formatScale( layerInfos.maxScale * SCALE_TO_SCALEHINT ) );
              scaleHintElem.setAttribute( u"max"_s, formatScale( layerInfos.minScale * SCALE_TO_SCALEHINT ) );
              layerElem.appendChild( scaleHintElem );
            }
            else
            {
              QDomElement minScaleElem = doc.createElement( u"MinScaleDenominator"_s );
              QDomText minScaleText = doc.createTextNode( formatScale( layerInfos.maxScale ) );
              minScaleElem.appendChild( minScaleText );
              layerElem.appendChild( minScaleElem );

              QDomElement maxScaleElem = doc.createElement( u"MaxScaleDenominator"_s );
              QDomText maxScaleText = doc.createTextNode( formatScale( layerInfos.minScale ) );
              maxScaleElem.appendChild( maxScaleText );
              layerElem.appendChild( maxScaleElem );
            }
          }

          bool timeDimensionAdded { false };

          // Add dimensions
          if ( l->type() == Qgis::LayerType::Vector )
          {
            QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( l );
            QgsMapLayerServerProperties *serverProperties = static_cast<QgsMapLayerServerProperties *>( vl->serverProperties() );
            const QList<QgsMapLayerServerProperties::WmsDimensionInfo> wmsDims = serverProperties->wmsDimensions();
            for ( const QgsMapLayerServerProperties::WmsDimensionInfo &dim : wmsDims )
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

              QDomElement dimElem = doc.createElement( u"Dimension"_s );
              dimElem.setAttribute( u"name"_s, dim.name );

              if ( dim.name.toUpper() == "TIME"_L1 )
              {
                timeDimensionAdded = true;
              }

              if ( !dim.units.isEmpty() )
              {
                dimElem.setAttribute( u"units"_s, dim.units );
              }
              if ( !dim.unitSymbol.isEmpty() )
              {
                dimElem.setAttribute( u"unitSymbol"_s, dim.unitSymbol );
              }
              if ( !values.isEmpty() && dim.defaultDisplayType == QgsMapLayerServerProperties::WmsDimensionInfo::MinValue )
              {
                dimElem.setAttribute( u"default"_s, values.first().toString() );
              }
              else if ( !values.isEmpty() && dim.defaultDisplayType == QgsMapLayerServerProperties::WmsDimensionInfo::MaxValue )
              {
                dimElem.setAttribute( u"default"_s, values.last().toString() );
              }
              else if ( dim.defaultDisplayType == QgsMapLayerServerProperties::WmsDimensionInfo::ReferenceValue )
              {
                dimElem.setAttribute( u"default"_s, dim.referenceValue.toString() );
              }
              dimElem.setAttribute( u"multipleValues"_s, u"1"_s );
              dimElem.setAttribute( u"nearestValue"_s, u"0"_s );
              if ( projectSettings )
              {
                dimElem.setAttribute( u"fieldName"_s, dim.fieldName );
                dimElem.setAttribute( u"endFieldName"_s, dim.endFieldName );
              }
              // values list
              QStringList strValues;
              for ( const QVariant &v : values )
              {
                strValues << v.toString();
              }
              QDomText dimValuesText = doc.createTextNode( strValues.join( ", "_L1 ) );
              dimElem.appendChild( dimValuesText );
              layerElem.appendChild( dimElem );
            }
          }

          // Add WMS time dimension if not already added
          if ( !timeDimensionAdded
               && l->temporalProperties()
               && l->temporalProperties()->isActive() )
          {
            // TODO: set "default" (reference value)

            // Add all values
            const QList<QgsDateTimeRange> allRanges { l->temporalProperties()->allTemporalRanges( l ) };
            const bool isDateList = writeTimeDimensionNode( doc, layerElem, allRanges );

            parentDateRanges.append( allRanges );

            QDomElement timeExtentElem = doc.createElement( u"Extent"_s );
            timeExtentElem.setAttribute( u"name"_s, u"TIME"_s );

            const QgsDateTimeRange timeExtent { l->temporalProperties()->calculateTemporalExtent( l ) };
            QString extent;
            if ( isDateList )
            {
              extent = u"%1/%2"_s.arg( timeExtent.begin().date().toString( Qt::DateFormat::ISODate ), timeExtent.end().date().toString( Qt::DateFormat::ISODate ) );
            }
            else
            {
              extent = u"%1/%2"_s.arg( timeExtent.begin().toString( Qt::DateFormat::ISODate ), timeExtent.end().toString( Qt::DateFormat::ISODate ) );
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

    void appendLayerStyles( QDomDocument &doc, QDomElement &layerElem, const QgsWmsLayerInfos &layerInfos, const QgsProject *project, const QgsWmsRequest &request, const QgsServerSettings *settings )
    {
      for ( const QString &styleName : std::as_const( layerInfos.styles ) )
      {
        QDomElement styleElem = createStyleElement( doc, styleName );

        writeLegendUrl( doc, styleElem, layerInfos.legendUrl, layerInfos.legendUrlFormat, layerInfos.name, styleName, project, request, settings );

        layerElem.appendChild( styleElem );
      }
    }

    void appendCrsElementsToLayer( QDomDocument &doc, QDomElement &layerElement, const QStringList &crsList, const QStringList &constrainedCrsList )
    {
      if ( layerElement.isNull() )
      {
        return;
      }

      const QString version = doc.documentElement().attribute( u"version"_s );

      //insert the CRS elements after the title element to be in accordance with the WMS 1.3 specification
      QDomElement titleElement = layerElement.firstChildElement( u"Title"_s );
      QDomElement abstractElement = layerElement.firstChildElement( u"Abstract"_s );
      QDomElement keywordListElement = layerElement.firstChildElement( u"KeywordList"_s );
      QDomElement CRSPrecedingElement = !keywordListElement.isNull() ? keywordListElement : !abstractElement.isNull() ? abstractElement
                                                                                                                      : titleElement;

      if ( CRSPrecedingElement.isNull() )
      {
        // keyword list element is never empty
        const QDomElement keyElement = layerElement.firstChildElement( u"KeywordList"_s );
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
      if ( version == "1.3.0"_L1 )
      {
        appendCrsElementToLayer( doc, layerElement, CRSPrecedingElement, QString( "CRS:84" ) );
      }
    }

    void appendCrsElementToLayer( QDomDocument &doc, QDomElement &layerElement, const QDomElement &precedingElement, const QString &crsText )
    {
      if ( crsText.isEmpty() )
        return;
      const QString version = doc.documentElement().attribute( u"version"_s );
      QDomElement crsElement = doc.createElement( version == "1.1.1"_L1 ? "SRS" : "CRS" );
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
      const QString version = doc.documentElement().attribute( u"version"_s );
      if ( version == "1.1.1"_L1 ) // WMS Version 1.1.1
      {
        ExGeoBBoxElement = doc.createElement( u"LatLonBoundingBox"_s );
        ExGeoBBoxElement.setAttribute( u"minx"_s, qgsDoubleToString( QgsServerProjectUtils::floorWithPrecision( wgs84BoundingRect.xMinimum(), wgs84precision ), wgs84precision ) );
        ExGeoBBoxElement.setAttribute( u"miny"_s, qgsDoubleToString( QgsServerProjectUtils::floorWithPrecision( wgs84BoundingRect.yMinimum(), wgs84precision ), wgs84precision ) );
        ExGeoBBoxElement.setAttribute( u"maxx"_s, qgsDoubleToString( QgsServerProjectUtils::ceilWithPrecision( wgs84BoundingRect.xMaximum(), wgs84precision ), wgs84precision ) );
        ExGeoBBoxElement.setAttribute( u"maxy"_s, qgsDoubleToString( QgsServerProjectUtils::ceilWithPrecision( wgs84BoundingRect.yMaximum(), wgs84precision ), wgs84precision ) );
      }
      else // WMS Version 1.3.0
      {
        ExGeoBBoxElement = doc.createElement( u"EX_GeographicBoundingBox"_s );
        QDomElement wBoundLongitudeElement = doc.createElement( u"westBoundLongitude"_s );
        QDomText wBoundLongitudeText = doc.createTextNode( qgsDoubleToString( QgsServerProjectUtils::floorWithPrecision( wgs84BoundingRect.xMinimum(), wgs84precision ), wgs84precision ) );
        wBoundLongitudeElement.appendChild( wBoundLongitudeText );
        ExGeoBBoxElement.appendChild( wBoundLongitudeElement );
        QDomElement eBoundLongitudeElement = doc.createElement( u"eastBoundLongitude"_s );
        QDomText eBoundLongitudeText = doc.createTextNode( qgsDoubleToString( QgsServerProjectUtils::ceilWithPrecision( wgs84BoundingRect.xMaximum(), wgs84precision ), wgs84precision ) );
        eBoundLongitudeElement.appendChild( eBoundLongitudeText );
        ExGeoBBoxElement.appendChild( eBoundLongitudeElement );
        QDomElement sBoundLatitudeElement = doc.createElement( u"southBoundLatitude"_s );
        QDomText sBoundLatitudeText = doc.createTextNode( qgsDoubleToString( QgsServerProjectUtils::floorWithPrecision( wgs84BoundingRect.yMinimum(), wgs84precision ), wgs84precision ) );
        sBoundLatitudeElement.appendChild( sBoundLatitudeText );
        ExGeoBBoxElement.appendChild( sBoundLatitudeElement );
        QDomElement nBoundLatitudeElement = doc.createElement( u"northBoundLatitude"_s );
        QDomText nBoundLatitudeText = doc.createTextNode( qgsDoubleToString( QgsServerProjectUtils::ceilWithPrecision( wgs84BoundingRect.yMaximum(), wgs84precision ), wgs84precision ) );
        nBoundLatitudeElement.appendChild( nBoundLatitudeText );
        ExGeoBBoxElement.appendChild( nBoundLatitudeElement );
      }

      const QDomElement lastCRSElem = layerElem.lastChildElement( version == "1.1.1"_L1 ? "SRS" : "CRS" );
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
      const QString version = doc.documentElement().attribute( u"version"_s );

      const auto &keys = crsExtents.keys();
      for ( const QString &crsText : std::as_const( keys ) )
      {
        QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( crsText );
        QgsRectangle crsExtent( crsExtents[crsText] );

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
        QDomElement bBoxElement = doc.createElement( u"BoundingBox"_s );
        if ( crs.isValid() )
        {
          bBoxElement.setAttribute( version == "1.1.1"_L1 ? "SRS" : "CRS", crs.authid() );
        }

        if ( version != "1.1.1"_L1 && crs.hasAxisInverted() )
        {
          crsExtent.invert();
        }

        bBoxElement.setAttribute( u"minx"_s, qgsDoubleToString( QgsServerProjectUtils::floorWithPrecision( crsExtent.xMinimum(), precision ), precision ) );
        bBoxElement.setAttribute( u"miny"_s, qgsDoubleToString( QgsServerProjectUtils::floorWithPrecision( crsExtent.yMinimum(), precision ), precision ) );
        bBoxElement.setAttribute( u"maxx"_s, qgsDoubleToString( QgsServerProjectUtils::ceilWithPrecision( crsExtent.xMaximum(), precision ), precision ) );
        bBoxElement.setAttribute( u"maxy"_s, qgsDoubleToString( QgsServerProjectUtils::ceilWithPrecision( crsExtent.yMaximum(), precision ), precision ) );

        QDomElement lastBBoxElem = layerElem.lastChildElement( u"BoundingBox"_s );
        if ( !lastBBoxElem.isNull() )
        {
          layerElem.insertAfter( bBoxElement, lastBBoxElem );
        }
        else
        {
          lastBBoxElem = layerElem.lastChildElement( version == "1.1.1"_L1 ? "LatLonBoundingBox" : "EX_GeographicBoundingBox" );
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

    void appendDrawingOrder( QDomDocument &doc, QDomElement &parentElem, QgsServerInterface *serverIface, const QgsProject *project )
    {
#ifdef HAVE_SERVER_PYTHON_PLUGINS
      QgsAccessControl *accessControl = serverIface->accessControls();
#else
      ( void ) serverIface;
#endif
      bool useLayerIds = QgsServerProjectUtils::wmsUseLayerIds( *project );
      QStringList restrictedLayers = QgsServerProjectUtils::wmsRestrictedLayers( *project );

      QStringList layerList;

      const QgsLayerTree *projectLayerTreeRoot = project->layerTreeRoot();
      QList<QgsMapLayer *> projectLayerOrder = projectLayerTreeRoot->layerOrder();
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
        else if ( !l->serverProperties()->shortName().isEmpty() )
        {
          wmsName = l->serverProperties()->shortName();
        }

        layerList << wmsName;
      }

      if ( !layerList.isEmpty() )
      {
        QStringList reversedList;
        reversedList.reserve( layerList.size() );
        for ( int i = layerList.size() - 1; i >= 0; --i )
          reversedList << layerList[i];

        QDomElement layerDrawingOrderElem = doc.createElement( u"LayerDrawingOrder"_s );
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
      QDomElement treeNameElem = doc.createElement( u"TreeName"_s );
      QDomText treeNameText = doc.createTextNode( currentLayer->name() );
      treeNameElem.appendChild( treeNameText );
      layerElem.appendChild( treeNameElem );

      switch ( currentLayer->type() )
      {
        case Qgis::LayerType::Vector:
        {
          QgsVectorLayer *vLayer = static_cast<QgsVectorLayer *>( currentLayer );

          int displayFieldIdx = -1;
          QString displayField = u"maptip"_s;
          QgsExpression exp( vLayer->displayExpression() );
          if ( exp.isField() )
          {
            displayField = static_cast<const QgsExpressionNodeColumnRef *>( exp.rootNode() )->name();
            displayFieldIdx = vLayer->fields().lookupField( displayField );
          }

          //attributes
          QDomElement attributesElem = doc.createElement( u"Attributes"_s );
          const QgsFields layerFields = vLayer->fields();
          for ( int idx = 0; idx < layerFields.count(); ++idx )
          {
            QgsField field = layerFields.at( idx );
            if ( field.configurationFlags().testFlag( Qgis::FieldConfigurationFlag::HideFromWms ) )
            {
              continue;
            }
            // field alias in case of displayField
            if ( idx == displayFieldIdx )
            {
              displayField = vLayer->attributeDisplayName( idx );
            }
            QDomElement attributeElem = doc.createElement( u"Attribute"_s );
            attributeElem.setAttribute( u"name"_s, field.name() );
            attributeElem.setAttribute( u"type"_s, QVariant::typeToName( field.type() ) );
            attributeElem.setAttribute( u"typeName"_s, field.typeName() );
            QString alias = field.alias();
            if ( !alias.isEmpty() )
            {
              attributeElem.setAttribute( u"alias"_s, alias );
            }

            //edit type to text
            attributeElem.setAttribute( u"editType"_s, vLayer->editorWidgetSetup( idx ).type() );
            attributeElem.setAttribute( u"comment"_s, field.comment() );
            attributeElem.setAttribute( u"length"_s, field.length() );
            attributeElem.setAttribute( u"precision"_s, field.precision() );
            attributesElem.appendChild( attributeElem );
          }

          //displayfield
          layerElem.setAttribute( u"displayField"_s, displayField );

          //primary key
          QgsAttributeList pkAttributes = vLayer->primaryKeyAttributes();
          if ( pkAttributes.size() > 0 )
          {
            QDomElement pkElem = doc.createElement( u"PrimaryKey"_s );
            QgsAttributeList::const_iterator pkIt = pkAttributes.constBegin();
            for ( ; pkIt != pkAttributes.constEnd(); ++pkIt )
            {
              QDomElement pkAttributeElem = doc.createElement( u"PrimaryKeyAttribute"_s );
              QDomText pkAttName = doc.createTextNode( layerFields.at( *pkIt ).name() );
              pkAttributeElem.appendChild( pkAttName );
              pkElem.appendChild( pkAttributeElem );
            }
            layerElem.appendChild( pkElem );
          }

          //geometry type
          layerElem.setAttribute( u"geometryType"_s, QgsWkbTypes::displayString( vLayer->wkbType() ) );

          //opacity
          layerElem.setAttribute( u"opacity"_s, QString::number( vLayer->opacity() ) );

          layerElem.appendChild( attributesElem );
          break;
        }

        case Qgis::LayerType::Raster:
        {
          const QgsDataProvider *provider = currentLayer->dataProvider();
          if ( provider && provider->name() == "wms" )
          {
            //advertise as web map background layer
            QVariant wmsBackgroundLayer = currentLayer->customProperty( u"WMSBackgroundLayer"_s, false );
            QDomElement wmsBackgroundLayerElem = doc.createElement( "WMSBackgroundLayer" );
            QDomText wmsBackgroundLayerText = doc.createTextNode( wmsBackgroundLayer.toBool() ? u"1"_s : u"0"_s );
            wmsBackgroundLayerElem.appendChild( wmsBackgroundLayerText );
            layerElem.appendChild( wmsBackgroundLayerElem );

            //publish datasource
            QVariant wmsPublishDataSourceUrl = currentLayer->customProperty( u"WMSPublishDataSourceUrl"_s, false );
            if ( wmsPublishDataSourceUrl.toBool() )
            {
              bool tiled = qobject_cast<const QgsRasterDataProvider *>( provider )
                             ? !qobject_cast<const QgsRasterDataProvider *>( provider )->nativeResolutions().isEmpty()
                             : false;

              QDomElement dataSourceElem = doc.createElement( tiled ? u"WMTSDataSource"_s : u"WMSDataSource"_s );
              QDomText dataSourceUri = doc.createTextNode( provider->dataSourceUri() );
              dataSourceElem.appendChild( dataSourceUri );
              layerElem.appendChild( dataSourceElem );
            }
          }

          QVariant wmsPrintLayer = currentLayer->customProperty( u"WMSPrintLayer"_s );
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
            layerElem.setAttribute( u"opacity"_s, QString::number( rasterRenderer->opacity() ) );
          }
          break;
        }

        case Qgis::LayerType::Mesh:
        case Qgis::LayerType::VectorTile:
        case Qgis::LayerType::Plugin:
        case Qgis::LayerType::Annotation:
        case Qgis::LayerType::PointCloud:
        case Qgis::LayerType::Group:
        case Qgis::LayerType::TiledScene:
          break;
      }
    }

    void addKeywordListElement( const QgsProject *project, QDomDocument &doc, QDomElement &parent )
    {
      bool sia2045 = QgsServerProjectUtils::wmsInfoFormatSia2045( *project );

      QDomElement keywordsElem = doc.createElement( u"KeywordList"_s );
      //add default keyword
      QDomElement keywordElem = doc.createElement( u"Keyword"_s );
      keywordElem.setAttribute( u"vocabulary"_s, u"ISO"_s );
      QDomText keywordText = doc.createTextNode( u"infoMapAccessService"_s );
      keywordElem.appendChild( keywordText );
      keywordsElem.appendChild( keywordElem );
      parent.appendChild( keywordsElem );
      QStringList keywords = QgsServerProjectUtils::owsServiceKeywords( *project );
      for ( const QString &keyword : std::as_const( keywords ) )
      {
        if ( !keyword.isEmpty() )
        {
          keywordElem = doc.createElement( u"Keyword"_s );
          keywordText = doc.createTextNode( keyword );
          keywordElem.appendChild( keywordText );
          if ( sia2045 )
          {
            keywordElem.setAttribute( u"vocabulary"_s, u"SIA_Geo405"_s );
          }
          keywordsElem.appendChild( keywordElem );
        }
      }
      parent.appendChild( keywordsElem );
    }
  } // namespace

  bool hasQueryableLayers( const QStringList &layerIds, const QMap<QString, QgsWmsLayerInfos> &wmsLayerInfos )
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

  QgsRectangle combineWgs84BoundingRect( const QStringList &layerIds, const QMap<QString, QgsWmsLayerInfos> &wmsLayerInfos )
  {
    QgsRectangle combined;
    bool empty = true;

    for ( const QString &id : std::as_const( layerIds ) )
    {
      if ( !wmsLayerInfos.contains( id ) )
      {
        continue;
      }

      QgsRectangle rect = wmsLayerInfos[id].wgs84BoundingRect;
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

  QMap<QString, QgsRectangle> combineCrsExtents( const QStringList &layerIds, const QMap<QString, QgsWmsLayerInfos> &wmsLayerInfos )
  {
    QMap<QString, QgsRectangle> combined;

    for ( const QString &id : std::as_const( layerIds ) )
    {
      if ( !wmsLayerInfos.contains( id ) )
      {
        continue;
      }

      const QgsWmsLayerInfos &layerInfos = wmsLayerInfos[id];
      const auto keys = layerInfos.crsExtents.keys();
      for ( const QString &crs : std::as_const( keys ) )
      {
        const QgsRectangle rect = layerInfos.crsExtents[crs];
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
          combined[crs] = rect;
        }
        else
        {
          combined[crs].combineExtentWith( rect );
        }
      }
    }

    return combined;
  }

} // namespace QgsWms
