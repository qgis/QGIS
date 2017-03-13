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

    QString cacheKey = cacheKeyList.join( QStringLiteral( "-" ) );
    const QDomDocument *capabilitiesDocument = capabilitiesCache->searchCapabilitiesDocument( configFilePath, cacheKey );
    if ( !capabilitiesDocument ) //capabilities xml not in cache. Create a new one
    {
      QgsMessageLog::logMessage( QStringLiteral( "Capabilities document not found in cache" ) );
      QDomDocument doc;

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
      if ( configParser && configParser->wmsInspireActivated() )
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

    configParser->serviceCapabilities( wmsCapabilitiesElement, doc );

    //wms:Capability element
    QDomElement capabilityElement = doc.createElement( QStringLiteral( "Capability" )/*wms:Capability*/ );
    wmsCapabilitiesElement.appendChild( capabilityElement );
    //wms:Request element
    QDomElement requestElement = doc.createElement( QStringLiteral( "Request" )/*wms:Request*/ );
    capabilityElement.appendChild( requestElement );

    QDomElement dcpTypeElement = doc.createElement( QStringLiteral( "DCPType" )/*wms:DCPType*/ );
    QDomElement httpElement = doc.createElement( QStringLiteral( "HTTP" )/*wms:HTTP*/ );
    dcpTypeElement.appendChild( httpElement );

    QDomElement elem;

    //wms:GetCapabilities
    elem = doc.createElement( QStringLiteral( "GetCapabilities" )/*wms:GetCapabilities*/ );
    appendFormat( elem, ( version == QLatin1String( "1.1.1" ) ? "application/vnd.ogc.wms_xml" : "text/xml" ) );
    elem.appendChild( dcpTypeElement );
    requestElement.appendChild( elem );

    // SOAP platform
    //only give this information if it is not a WMS request to be in sync with the WMS capabilities schema
    // XXX Not even sure that cam be ever true
    if ( parameters.value( QStringLiteral( "SERVICE" ) ).compare( QLatin1String( "WMS" ), Qt::CaseInsensitive ) != 0 )
    {
      QDomElement soapElement = doc.createElement( QStringLiteral( "SOAP" )/*wms:SOAP*/ );
      httpElement.appendChild( soapElement );
      QDomElement soapResourceElement = doc.createElement( QStringLiteral( "OnlineResource" )/*wms:OnlineResource*/ );
      soapResourceElement.setAttribute( QStringLiteral( "xmlns:xlink" ), QStringLiteral( "http://www.w3.org/1999/xlink" ) );
      soapResourceElement.setAttribute( QStringLiteral( "xlink:type" ), QStringLiteral( "simple" ) );
      soapResourceElement.setAttribute( QStringLiteral( "xlink:href" ), hrefString );
      soapElement.appendChild( soapResourceElement );
    }

    //only Get supported for the moment
    QDomElement getElement = doc.createElement( QStringLiteral( "Get" )/*wms:Get*/ );
    httpElement.appendChild( getElement );
    QDomElement olResourceElement = doc.createElement( QStringLiteral( "OnlineResource" )/*wms:OnlineResource*/ );
    olResourceElement.setAttribute( QStringLiteral( "xmlns:xlink" ), QStringLiteral( "http://www.w3.org/1999/xlink" ) );
    olResourceElement.setAttribute( QStringLiteral( "xlink:type" ), QStringLiteral( "simple" ) );
    olResourceElement.setAttribute( QStringLiteral( "xlink:href" ), hrefString );
    getElement.appendChild( olResourceElement );

    //wms:GetMap
    elem = doc.createElement( QStringLiteral( "GetMap" )/*wms:GetMap*/ );
    appendFormat( elem, QStringLiteral( "image/jpeg" ) );
    appendFormat( elem, QStringLiteral( "image/png" ) );
    appendFormat( elem, QStringLiteral( "image/png; mode=16bit" ) );
    appendFormat( elem, QStringLiteral( "image/png; mode=8bit" ) );
    appendFormat( elem, QStringLiteral( "image/png; mode=1bit" ) );
    appendFormat( elem, QStringLiteral( "application/dxf" ) );
    elem.appendChild( dcpTypeElement.cloneNode().toElement() ); //this is the same as for 'GetCapabilities'
    requestElement.appendChild( elem );

    //wms:GetFeatureInfo
    elem = doc.createElement( QStringLiteral( "GetFeatureInfo" ) );
    appendFormat( elem, QStringLiteral( "text/plain" ) );
    appendFormat( elem, QStringLiteral( "text/html" ) );
    appendFormat( elem, QStringLiteral( "text/xml" ) );
    appendFormat( elem, QStringLiteral( "application/vnd.ogc.gml" ) );
    appendFormat( elem, QStringLiteral( "application/vnd.ogc.gml/3.1.1" ) );
    elem.appendChild( dcpTypeElement.cloneNode().toElement() ); //this is the same as for 'GetCapabilities'
    requestElement.appendChild( elem );

    //wms:GetLegendGraphic
    elem = doc.createElement( ( version == QLatin1String( "1.1.1" ) ? "GetLegendGraphic" : "sld:GetLegendGraphic" )/*wms:GetLegendGraphic*/ );
    appendFormat( elem, QStringLiteral( "image/jpeg" ) );
    appendFormat( elem, QStringLiteral( "image/png" ) );
    elem.appendChild( dcpTypeElement.cloneNode().toElement() ); // this is the same as for 'GetCapabilities'
    requestElement.appendChild( elem );

    //wms:DescribeLayer
    elem = doc.createElement( ( version == QLatin1String( "1.1.1" ) ? "DescribeLayer" : "sld:DescribeLayer" )/*wms:GetLegendGraphic*/ );
    appendFormat( elem, QStringLiteral( "text/xml" ) );
    elem.appendChild( dcpTypeElement.cloneNode().toElement() ); // this is the same as for 'GetCapabilities'
    requestElement.appendChild( elem );

    //wms:GetStyles
    elem = doc.createElement( ( version == QLatin1String( "1.1.1" ) ? "GetStyles" : "qgs:GetStyles" )/*wms:GetStyles*/ );
    appendFormat( elem, QStringLiteral( "text/xml" ) );
    elem.appendChild( dcpTypeElement.cloneNode().toElement() ); //this is the same as for 'GetCapabilities'
    requestElement.appendChild( elem );

    if ( projectSettings ) //remove composer templates from GetCapabilities in the long term
    {
      //wms:GetPrint
      elem = doc.createElement( QStringLiteral( "GetPrint" ) /*wms:GetPrint*/ );
      appendFormat( elem, QStringLiteral( "svg" ) );
      appendFormat( elem, QStringLiteral( "png" ) );
      appendFormat( elem, QStringLiteral( "pdf" ) );
      elem.appendChild( dcpTypeElement.cloneNode().toElement() ); //this is the same as for 'GetCapabilities'
      requestElement.appendChild( elem );
    }

    //Exception element is mandatory
    elem = doc.createElement( QStringLiteral( "Exception" ) );
    appendFormat( elem, ( version == QLatin1String( "1.1.1" ) ? "application/vnd.ogc.se_xml" : "XML" ) );
    capabilityElement.appendChild( elem );

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
      capabilityElement.appendChild( elem );

      if ( configParser->wmsInspireActivated() )
      {
        configParser->inspireCapabilities( capabilityElement, doc );
      }
    }

    if ( projectSettings )
    {
      //Insert <ComposerTemplate> elements derived from wms:_ExtendedCapabilities
      configParser->printCapabilities( capabilityElement, doc );

      //WFS layers
      QStringList wfsLayers = configParser->wfsLayerNames();
      if ( !wfsLayers.isEmpty() )
      {
        QDomElement wfsLayersElem = doc.createElement( QStringLiteral( "WFSLayers" ) );
        for ( auto wfsIt = wfsLayers.constBegin() ; wfsIt != wfsLayers.constEnd(); ++wfsIt )
        {
          QDomElement wfsLayerElem = doc.createElement( QStringLiteral( "WFSLayer" ) );
          wfsLayerElem.setAttribute( QStringLiteral( "name" ), *wfsIt );
          wfsLayersElem.appendChild( wfsLayerElem );
        }
        capabilityElement.appendChild( wfsLayersElem );
      }
    }

    //add the xml content for the individual layers/styles
    QString wmsServiceUrl = QgsServerProjectUtils::wmsServiceUrl( *project );
    configParser->layersAndStylesCapabilities( capabilityElement, doc, version,
        wmsServiceUrl, projectSettings );

    return doc;
  }

} // namespace QgsWms




