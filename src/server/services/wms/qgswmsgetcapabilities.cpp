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
#include "qgscomposition.h"
#include "qgscomposermap.h"
#include "qgscomposerlabel.h"
#include "qgscomposerhtml.h"
#include "qgscomposerframe.h"


namespace QgsWms
{
  void writeGetCapabilities( QgsServerInterface *serverIface, const QgsProject *project,
                             const QString &version, const QgsServerRequest &request,
                             QgsServerResponse &response, bool projectSettings )
  {
    QString configFilePath = serverIface->configFilePath();
    QgsCapabilitiesCache *capabilitiesCache = serverIface->capabilitiesCache();

    QStringList cacheKeyList;
    cacheKeyList << ( projectSettings ? QStringLiteral( "projectSettings" ) : version );
    cacheKeyList << request.url().host();
    bool cache = true;

#ifdef HAVE_SERVER_PYTHON_PLUGINS
    QgsAccessControl *accessControl = serverIface->accessControls();
    if ( accessControl )
      cache = accessControl->fillCacheKey( cacheKeyList );
#endif

    QDomDocument doc;
    QString cacheKey = cacheKeyList.join( QStringLiteral( "-" ) );
    const QDomDocument *capabilitiesDocument = capabilitiesCache->searchCapabilitiesDocument( configFilePath, cacheKey );
    if ( !capabilitiesDocument ) //capabilities xml not in cache. Create a new one
    {
      QgsMessageLog::logMessage( QStringLiteral( "Capabilities document not found in cache" ) );

      doc = getCapabilities( serverIface, project, version, request, projectSettings );

      if ( cache )
      {
        capabilitiesCache->insertCapabilitiesDocument( configFilePath, cacheKey, &doc );
        capabilitiesDocument = capabilitiesCache->searchCapabilitiesDocument( configFilePath, cacheKey );
      }
      else
      {
        doc = doc.cloneNode().toDocument();
        capabilitiesDocument = &doc;
      }
    }
    else
    {
      QgsMessageLog::logMessage( QStringLiteral( "Found capabilities document in cache" ) );
    }

    response.setHeader( QStringLiteral( "Content-Type" ), QStringLiteral( "text/xml; charset=utf-8" ) );
    response.write( capabilitiesDocument->toByteArray() );
  }

  QDomDocument getCapabilities( QgsServerInterface *serverIface, const QgsProject *project,
                                const QString &version, const QgsServerRequest &request,
                                bool projectSettings )
  {
    QDomDocument doc;
    QDomElement wmsCapabilitiesElement;

    QgsWmsConfigParser *configParser = getConfigParser( serverIface );

    QgsServerRequest::Parameters parameters = request.parameters();

    // Get service URL
    QUrl href = serviceUrl( request, project );

    //href needs to be a prefix
    QString hrefString = href.toString( QUrl::FullyDecoded );
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

    if ( version == QLatin1String( "1.1.1" ) )
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
      schemaLocation += QLatin1String( " http://www.qgis.org/wms" );
      if ( QgsServerProjectUtils::wmsInspireActivated( *project ) )
      {
        wmsCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:inspire_common" ), QStringLiteral( "http://inspire.ec.europa.eu/schemas/common/1.0" ) );
        wmsCapabilitiesElement.setAttribute( QStringLiteral( "xmlns:inspire_vs" ), QStringLiteral( "http://inspire.ec.europa.eu/schemas/inspire_vs/1.0" ) );
        schemaLocation += QLatin1String( " http://inspire.ec.europa.eu/schemas/inspire_vs/1.0" );
        schemaLocation += QLatin1String( " http://inspire.ec.europa.eu/schemas/inspire_vs/1.0/inspire_vs.xsd" );
      }

      schemaLocation += " " + hrefString + "SERVICE=WMS&REQUEST=GetSchemaExtension";
      wmsCapabilitiesElement.setAttribute( QStringLiteral( "xsi:schemaLocation" ), schemaLocation );
    }
    wmsCapabilitiesElement.setAttribute( QStringLiteral( "version" ), version );
    doc.appendChild( wmsCapabilitiesElement );

    //INSERT Service
    wmsCapabilitiesElement.appendChild( getServiceElement( doc, project, version ) );

    //wms:Capability element
    QDomElement capabilityElement = getCapabilityElement( doc, project, version, request, projectSettings );
    wmsCapabilitiesElement.appendChild( capabilityElement );

    if ( projectSettings )
    {
      //Insert <ComposerTemplate> elements derived from wms:_ExtendedCapabilities
      capabilityElement.appendChild( getComposerTemplatesElement( doc, project ) );

      //WFS layers
      capabilityElement.appendChild( getWFSLayersElement( doc, project ) );
    }

    //add the xml content for the individual layers/styles
    QString wmsServiceUrl = QgsServerProjectUtils::wmsServiceUrl( *project );
    configParser->layersAndStylesCapabilities( capabilityElement, doc, version,
        wmsServiceUrl, projectSettings );

    return doc;
  }

  QDomElement getServiceElement( QDomDocument &doc, const QgsProject *project, const QString &version )
  {
    bool sia2045 = QgsServerProjectUtils::wmsInfoFormatSIA2045( *project );

    //Service element
    QDomElement serviceElem = doc.createElement( QStringLiteral( "Service" ) );

    //Service name
    QDomElement nameElem = doc.createElement( QStringLiteral( "Name" ) );
    QDomText nameText = doc.createTextNode( QStringLiteral( "WMS" ) );
    nameElem.appendChild( nameText );
    serviceElem.appendChild( nameElem );

    QString title = QgsServerProjectUtils::owsServiceTitle( *project );
    if ( !title.isEmpty() )
    {
      QDomElement titleElem = doc.createElement( QStringLiteral( "Title" ) );
      QDomText titleText = doc.createTextNode( title );
      titleElem.appendChild( titleText );
      serviceElem.appendChild( titleElem );
    }

    QString abstract = QgsServerProjectUtils::owsServiceAbstract( *project );
    if ( !abstract.isEmpty() )
    {
      QDomElement abstractElem = doc.createElement( QStringLiteral( "Abstract" ) );
      QDomText abstractText = doc.createCDATASection( abstract );
      abstractElem.appendChild( abstractText );
      serviceElem.appendChild( abstractElem );
    }

    QDomElement keywordsElem = doc.createElement( QStringLiteral( "KeywordList" ) );
    //add default keyword
    QDomElement keywordElem = doc.createElement( QStringLiteral( "Keyword" ) );
    keywordElem.setAttribute( QStringLiteral( "vocabulary" ), QStringLiteral( "ISO" ) );
    QDomText keywordText = doc.createTextNode( QStringLiteral( "infoMapAccessService" ) );
    keywordElem.appendChild( keywordText );
    keywordsElem.appendChild( keywordElem );
    serviceElem.appendChild( keywordsElem );
    QStringList keywords = QgsServerProjectUtils::owsServiceKeywords( *project );
    if ( !keywords.isEmpty() )
    {
      for ( int i = 0; i < keywords.size(); ++i )
      {
        QString keyword = keywords.at( i );
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
      serviceElem.appendChild( keywordsElem );
    }

    QString onlineResource = QgsServerProjectUtils::owsServiceOnlineResource( *project );
    if ( !onlineResource.isEmpty() )
    {
      QDomElement onlineResourceElem = doc.createElement( QStringLiteral( "OnlineResource" ) );
      onlineResourceElem.setAttribute( QStringLiteral( "xmlns:xlink" ), QStringLiteral( "http://www.w3.org/1999/xlink" ) );
      onlineResourceElem.setAttribute( QStringLiteral( "xlink:type" ), QStringLiteral( "simple" ) );
      onlineResourceElem.setAttribute( QStringLiteral( "xlink:href" ), onlineResource );
      serviceElem.appendChild( onlineResourceElem );
    }

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
           !contactOrganization.isEmpty() ||
           !contactPosition.isEmpty() )
      {
        QDomElement contactPersonPrimaryElem = doc.createElement( QStringLiteral( "ContactPersonPrimary" ) );

        if ( !contactPerson.isEmpty() )
        {
          QDomElement contactPersonElem = doc.createElement( QStringLiteral( "ContactPerson" ) );
          QDomText contactPersonText = doc.createTextNode( contactPerson );
          contactPersonElem.appendChild( contactPersonText );
          contactPersonPrimaryElem.appendChild( contactPersonElem );
        }

        if ( !contactOrganization.isEmpty() )
        {
          QDomElement contactOrganizationElem = doc.createElement( QStringLiteral( "ContactOrganization" ) );
          QDomText contactOrganizationText = doc.createTextNode( contactOrganization );
          contactOrganizationElem.appendChild( contactOrganizationText );
          contactPersonPrimaryElem.appendChild( contactOrganizationElem );
        }

        if ( !contactPosition.isEmpty() )
        {
          QDomElement contactPositionElem = doc.createElement( QStringLiteral( "ContactPosition" ) );
          QDomText contactPositionText = doc.createTextNode( contactPosition );
          contactPositionElem.appendChild( contactPositionText );
          contactPersonPrimaryElem.appendChild( contactPositionElem );
        }

        contactInfoElem.appendChild( contactPersonPrimaryElem );
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

    if ( version == QLatin1String( "1.3.0" ) )
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
                                    const QString &version, const QgsServerRequest &request,
                                    bool projectSettings )
  {
    QgsServerRequest::Parameters parameters = request.parameters();

    // Get service URL
    QUrl href = serviceUrl( request, project );

    //href needs to be a prefix
    QString hrefString = href.toString( QUrl::FullyDecoded );
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

    // SOAP platform
    //only give this information if it is not a WMS request to be in sync with the WMS capabilities schema
    // XXX Not even sure that cam be ever true
    if ( parameters.value( QStringLiteral( "SERVICE" ) ).compare( QLatin1String( "WMS" ), Qt::CaseInsensitive ) != 0 )
    {
      QDomElement soapElem = doc.createElement( QStringLiteral( "SOAP" )/*wms:SOAP*/ );
      httpElem.appendChild( soapElem );
      QDomElement soapResourceElem = doc.createElement( QStringLiteral( "OnlineResource" )/*wms:OnlineResource*/ );
      soapResourceElem.setAttribute( QStringLiteral( "xmlns:xlink" ), QStringLiteral( "http://www.w3.org/1999/xlink" ) );
      soapResourceElem.setAttribute( QStringLiteral( "xlink:type" ), QStringLiteral( "simple" ) );
      soapResourceElem.setAttribute( QStringLiteral( "xlink:href" ), hrefString );
      soapElem.appendChild( soapResourceElem );
    }

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
    elem.appendChild( dcpTypeElem.cloneNode().toElement() ); //this is the same as for 'GetCapabilities'
    requestElem.appendChild( elem );

    //wms:GetLegendGraphic
    elem = doc.createElement( ( version == QLatin1String( "1.1.1" ) ? "GetLegendGraphic" : "sld:GetLegendGraphic" )/*wms:GetLegendGraphic*/ );
    appendFormat( elem, QStringLiteral( "image/jpeg" ) );
    appendFormat( elem, QStringLiteral( "image/png" ) );
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

    if ( projectSettings ) //remove composer templates from GetCapabilities in the long term
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

      if ( QgsServerProjectUtils::wmsInspireActivated( *project ) )
      {
        capabilityElem.appendChild( getInspireCapabilitiesElement( doc, project ) );
      }
    }

    return capabilityElem;
  }

  QDomElement getInspireCapabilitiesElement( QDomDocument &doc, const QgsProject *project )
  {
    QDomElement inspireCapabilitiesElem;

    if ( !QgsServerProjectUtils::wmsInspireActivated( *project ) )
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
    QList<QgsComposition *> projectComposers = project->layoutManager()->compositions();
    if ( projectComposers.size() == 0 )
      return QDomElement();

    QStringList restrictedComposers = QgsServerProjectUtils::wmsRestrictedComposers( *project );

    QDomElement composerTemplatesElem = doc.createElement( QStringLiteral( "ComposerTemplates" ) );
    QList<QgsComposition *>::const_iterator cIt = projectComposers.constBegin();
    for ( ; cIt != projectComposers.constEnd(); ++cIt )
    {
      QgsComposition *composer = *cIt;
      if ( restrictedComposers.contains( composer->name() ) )
        continue;

      QDomElement composerTemplateElem = doc.createElement( QStringLiteral( "ComposerTemplate" ) );
      composerTemplateElem.setAttribute( QStringLiteral( "name" ), composer->name() );

      //get paper width and hight in mm from composition
      composerTemplateElem.setAttribute( QStringLiteral( "width" ), composer->paperWidth() );
      composerTemplateElem.setAttribute( QStringLiteral( "height" ), composer->paperHeight() );

      //add available composer maps and their size in mm
      QList<const QgsComposerMap *> composerMapList = composer->composerMapItems();
      QList<const QgsComposerMap *>::const_iterator cmIt = composerMapList.constBegin();
      for ( ; cmIt != composerMapList.constEnd(); ++cmIt )
      {
        const QgsComposerMap *composerMap = *cmIt;

        QDomElement composerMapElem = doc.createElement( QStringLiteral( "ComposerMap" ) );
        composerMapElem.setAttribute( QStringLiteral( "name" ), QStringLiteral( "map%s" ).arg( composerMap->id() ) );
        composerMapElem.setAttribute( QStringLiteral( "width" ), composerMap->rect().width() );
        composerMapElem.setAttribute( QStringLiteral( "height" ), composerMap->rect().height() );
        composerTemplateElem.appendChild( composerMapElem );
      }

      //add available composer labels
      QList<QgsComposerLabel *> composerLabelList;
      composer->composerItems( composerLabelList );
      QList<QgsComposerLabel *>::const_iterator clIt = composerLabelList.constBegin();
      for ( ; clIt != composerLabelList.constEnd(); ++clIt )
      {
        QgsComposerLabel *composerLabel = *clIt;
        QString id = composerLabel->id();
        if ( id.isEmpty() )
          continue;

        QDomElement composerLabelElem = doc.createElement( QStringLiteral( "ComposerLabel" ) );
        composerLabelElem.setAttribute( QStringLiteral( "name" ), id );
        composerTemplateElem.appendChild( composerLabelElem );
      }

      //add available composer HTML
      QList<QgsComposerHtml *> composerHtmlList;
      composer->composerItems( composerHtmlList );
      QList<QgsComposerHtml *>::const_iterator chIt = composerHtmlList.constBegin();
      for ( ; chIt != composerHtmlList.constEnd(); ++chIt )
      {
        QgsComposerHtml *composerHtml = *chIt;
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
      if ( layer->type() != QgsMapLayer::LayerType::VectorLayer )
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

} // namespace QgsWms




