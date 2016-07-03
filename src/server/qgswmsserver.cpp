/***************************************************************************
                              qgswmsserver.cpp
                              -------------------
  begin                : May 14, 2006
  copyright            : (C) 2006 by Marco Hugentobler & Ionut Iosifescu Enescu
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgswmsserver.h"
#include "qgscapabilitiescache.h"
#include "qgscrscache.h"
#include "qgsdxfexport.h"
#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgslayertree.h"
#include "qgslayertreemodel.h"
#include "qgslayertreemodellegendnode.h"
#include "qgslegendrenderer.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerlegend.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaprenderer.h"
#include "qgsmaptopixel.h"
#include "qgsproject.h"
#include "qgsrasteridentifyresult.h"
#include "qgsrasterlayer.h"
#include "qgsscalecalculator.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsmapserviceexception.h"
#include "qgssldconfigparser.h"
#include "qgssymbolv2.h"
#include "qgsrendererv2.h"
#include "qgspaintenginehack.h"
#include "qgsogcutils.h"
#include "qgsfeature.h"
#include "qgseditorwidgetregistry.h"
#include "qgsserverstreamingdevice.h"
#include "qgsaccesscontrol.h"
#include "qgsfeaturerequest.h"

#include <QImage>
#include <QPainter>
#include <QStringList>
#include <QTemporaryFile>
#include <QTextStream>
#include <QDir>

//for printing
#include "qgscomposition.h"
#include <QBuffer>
#include <QPrinter>
#include <QSvgGenerator>
#include <QUrl>
#include <QPaintEngine>

QgsWMSServer::QgsWMSServer(
  const QString& configFilePath
  , QMap<QString, QString> &parameters
  , QgsWMSConfigParser* cp
  , QgsRequestHandler* rh
  , QgsMapRenderer* renderer
  , QgsCapabilitiesCache* capCache
#ifdef HAVE_SERVER_PYTHON_PLUGINS
  , const QgsAccessControl* accessControl
#endif
)
    : QgsOWSServer(
      configFilePath
      , parameters
      , rh
#ifdef HAVE_SERVER_PYTHON_PLUGINS
      , accessControl
#endif
    )
    , mMapRenderer( renderer )
    , mCapabilitiesCache( capCache )
    , mConfigParser( cp )
    , mOwnsConfigParser( false )
    , mDrawLegendLayerLabel( true )
    , mDrawLegendItemLabel( true )
{
#ifdef HAVE_SERVER_PYTHON_PLUGINS
  mMapRenderer->setFeatureFilterProvider( mAccessControl );
#endif
}

QgsWMSServer::QgsWMSServer()
    : QgsOWSServer(
      QString()
      , QMap<QString, QString>()
      , nullptr
#ifdef HAVE_SERVER_PYTHON_PLUGINS
      , nullptr
#endif
    )
    , mMapRenderer( nullptr )
    , mCapabilitiesCache()
    , mConfigParser( nullptr )
    , mOwnsConfigParser( false )
    , mDrawLegendLayerLabel( true )
    , mDrawLegendItemLabel( true )
{
}

QgsWMSServer::~QgsWMSServer()
{
}

void QgsWMSServer::cleanupAfterRequest()
{
  if ( mOwnsConfigParser )
  {
    delete mConfigParser;
    mConfigParser = nullptr;
    mOwnsConfigParser = false;
  }
}

void QgsWMSServer::executeRequest()
{
  if ( !mMapRenderer || !mConfigParser || !mRequestHandler || !mCapabilitiesCache )
  {
    return; //TODO: error handling
  }

  //request type
  QString request = mParameters.value( "REQUEST" );
  if ( request.isEmpty() )
  {
    QgsMessageLog::logMessage( "unable to find 'REQUEST' parameter, exiting..." );
    mRequestHandler->setServiceException( QgsMapServiceException( "OperationNotSupported", "Please check the value of the REQUEST parameter" ) );
    cleanupAfterRequest();
    return;
  }

  //version
  QString version = mParameters.value( "VERSION", "1.3.0" );
  bool getProjectSettings = ( request.compare( "GetProjectSettings", Qt::CaseInsensitive ) == 0 );
  if ( getProjectSettings )
  {
    version = "1.3.0"; //getProjectSettings extends WMS 1.3.0 capabilities
  }

  //GetCapabilities
  if ( request.compare( "GetCapabilities", Qt::CaseInsensitive ) == 0 || getProjectSettings )
  {
    QStringList cacheKeyList;
    cacheKeyList << ( getProjectSettings ? "projectSettings" : version );
    bool cache = true;
#ifdef HAVE_SERVER_PYTHON_PLUGINS
    cache = mAccessControl->fillCacheKey( cacheKeyList );
#endif
    QString cacheKey = cacheKeyList.join( "-" );
    const QDomDocument* capabilitiesDocument = mCapabilitiesCache->searchCapabilitiesDocument( mConfigFilePath, cacheKey );
    if ( !capabilitiesDocument ) //capabilities xml not in cache. Create a new one
    {
      QgsMessageLog::logMessage( "Capabilities document not found in cache" );
      QDomDocument doc;
      try
      {
        doc = getCapabilities( version, getProjectSettings );
      }
      catch ( QgsMapServiceException& ex )
      {
        mRequestHandler->setServiceException( ex );
        cleanupAfterRequest();
        return;
      }
      if ( cache )
      {
        mCapabilitiesCache->insertCapabilitiesDocument( mConfigFilePath, cacheKey, &doc );
        capabilitiesDocument = mCapabilitiesCache->searchCapabilitiesDocument( mConfigFilePath, cacheKey );
      }
      else
      {
        doc = doc.cloneNode().toDocument();
        capabilitiesDocument = &doc;
      }
    }
    else
    {
      QgsMessageLog::logMessage( "Found capabilities document in cache" );
    }

    if ( capabilitiesDocument )
    {
      mRequestHandler->setGetCapabilitiesResponse( *capabilitiesDocument );
    }
  }
  //GetMap
  else if ( request.compare( "GetMap", Qt::CaseInsensitive ) == 0 )
  {
    //export as dxf
    QString format = mParameters.value( "FORMAT" );
    if ( format.compare( "application/dxf", Qt::CaseInsensitive ) == 0 )
    {
      try
      {
        getMapAsDxf();
        cleanupAfterRequest();
        return;
      }
      catch ( QgsMapServiceException& ex )
      {
        QgsMessageLog::logMessage( "Caught exception during GetMap request" );
        mRequestHandler->setServiceException( ex );
        cleanupAfterRequest();
        return;
      }
    }

    QImage* result = nullptr;
    try
    {
      result = getMap();
    }
    catch ( QgsMapServiceException& ex )
    {
      QgsMessageLog::logMessage( "Caught exception during GetMap request" );
      mRequestHandler->setServiceException( ex );
      cleanupAfterRequest();
      return;
    }

    if ( result )
    {
      QgsMessageLog::logMessage( "Setting GetMap response" );
      mRequestHandler->setGetMapResponse( "WMS", result, getImageQuality() );
      QgsMessageLog::logMessage( "Response sent" );
    }
    else
    {
      //do some error handling
      QgsMessageLog::logMessage( "result image is 0" );
    }
    delete result;
  }
  //GetFeatureInfo
  else if ( request.compare( "GetFeatureInfo", Qt::CaseInsensitive ) == 0 )
  {
    QDomDocument featureInfoDoc;
    try
    {
      if ( getFeatureInfo( featureInfoDoc, version ) != 0 )
      {
        cleanupAfterRequest();
        return;
      }
    }
    catch ( QgsMapServiceException& ex )
    {
      mRequestHandler->setServiceException( ex );
      cleanupAfterRequest();
      return;
    }

    QString infoFormat = mParameters.value( "INFO_FORMAT",  "text/plain" );
    mRequestHandler->setGetFeatureInfoResponse( featureInfoDoc, infoFormat );
  }
  //GetContext
  else if ( request.compare( "GetContext", Qt::CaseInsensitive ) == 0 )
  {
    try
    {
      QDomDocument doc = getContext();
      mRequestHandler->setXmlResponse( doc );
    }
    catch ( QgsMapServiceException& ex )
    {
      mRequestHandler->setServiceException( ex );
    }
  }
  //GetSchemaExtension
  else if ( request.compare( "GetSchemaExtension", Qt::CaseInsensitive ) == 0 )
  {
    try
    {
      QDomDocument doc = getSchemaExtension();
      mRequestHandler->setXmlResponse( doc );
    }
    catch ( QgsMapServiceException& ex )
    {
      mRequestHandler->setServiceException( ex );
    }
  }
  //GetStyle for compatibility with earlier QGIS versions
  else if ( request.compare( "GetStyle", Qt::CaseInsensitive ) == 0 )
  {
    try
    {
      QDomDocument doc = getStyle();
      mRequestHandler->setXmlResponse( doc );
    }
    catch ( QgsMapServiceException& ex )
    {
      mRequestHandler->setServiceException( ex );
    }
  }
  //GetStyles
  else if ( request.compare( "GetStyles", Qt::CaseInsensitive ) == 0 )
  {
    // GetStyles is defined for WMS1.1.1/SLD1.0
    // and in qgis-server WMS1.3.0 extension
    try
    {
      QDomDocument doc = getStyles();
      mRequestHandler->setXmlResponse( doc );
    }
    catch ( QgsMapServiceException& ex )
    {
      mRequestHandler->setServiceException( ex );
    }
  }
  //GetStyles
  else if ( request.compare( "DescribeLayer", Qt::CaseInsensitive ) == 0 )
  {
    // DescribeLayer is defined for WMS1.1.1/SLD1.0
    // and in qgis-server WMS1.3.0 extension
    try
    {
      QDomDocument doc = describeLayer();
      mRequestHandler->setXmlResponse( doc );
    }
    catch ( QgsMapServiceException& ex )
    {
      mRequestHandler->setServiceException( ex );
    }
  }
  //GetLegendGraphic
  else if ( request.compare( "GetLegendGraphic", Qt::CaseInsensitive ) == 0 ||
            request.compare( "GetLegendGraphics", Qt::CaseInsensitive ) == 0 )
    // GetLegendGraphics for compatibility with earlier QGIS versions
  {
    QImage* result = nullptr;
    try
    {
      result = getLegendGraphics();
    }
    catch ( QgsMapServiceException& ex )
    {
      QgsMessageLog::logMessage( "Caught exception during GetLegendGraphic request" );
      mRequestHandler->setServiceException( ex );
    }

    if ( result )
    {
      QgsMessageLog::logMessage( "Setting GetLegendGraphic response" );
      //setting is the same for GetMap and GetLegendGraphic
      mRequestHandler->setGetMapResponse( "WMS", result, getImageQuality() );
      QgsMessageLog::logMessage( "Response sent" );
    }
    else
    {
      //do some error handling
      QgsMessageLog::logMessage( "result image is 0" );
    }
    delete result;
  }
  //GetPrint
  else if ( request.compare( "GetPrint", Qt::CaseInsensitive ) == 0 )
  {
    QByteArray* printOutput = nullptr;
    try
    {
      printOutput = getPrint( mRequestHandler->format() );
    }
    catch ( QgsMapServiceException& ex )
    {
      mRequestHandler->setServiceException( ex );
    }

    if ( printOutput )
    {
      mRequestHandler->setGetPrintResponse( printOutput );
    }
    delete printOutput;
  }
  else//unknown request
  {
    QgsMapServiceException e( "OperationNotSupported", "Operation " + request + " not supported" );
    mRequestHandler->setServiceException( e );
  }
  cleanupAfterRequest();
}

void QgsWMSServer::appendFormats( QDomDocument &doc, QDomElement &elem, const QStringList &formats )
{
  Q_FOREACH ( const QString& format, formats )
  {
    QDomElement formatElem = doc.createElement( "Format"/*wms:Format*/ );
    formatElem.appendChild( doc.createTextNode( format ) );
    elem.appendChild( formatElem );
  }
}

QDomDocument QgsWMSServer::getCapabilities( QString version, bool fullProjectInformation )
{
  QgsMessageLog::logMessage( "Entering." );
  QDomDocument doc;
  QDomElement wmsCapabilitiesElement;

  //Prepare url
  QString hrefString;
  if ( mConfigParser )
  {
    hrefString = mConfigParser->serviceUrl();
  }
  if ( hrefString.isEmpty() )
  {
    hrefString = serviceUrl();
  }

  if ( version == "1.1.1" )
  {
    doc = QDomDocument( "WMT_MS_Capabilities SYSTEM 'http://schemas.opengis.net/wms/1.1.1/WMS_MS_Capabilities.dtd'" );  //WMS 1.1.1 needs DOCTYPE  "SYSTEM http://schemas.opengis.net/wms/1.1.1/WMS_MS_Capabilities.dtd"
    addXMLDeclaration( doc );
    wmsCapabilitiesElement = doc.createElement( "WMT_MS_Capabilities"/*wms:WMS_Capabilities*/ );
  }
  else // 1.3.0 as default
  {
    addXMLDeclaration( doc );
    wmsCapabilitiesElement = doc.createElement( "WMS_Capabilities"/*wms:WMS_Capabilities*/ );
    wmsCapabilitiesElement.setAttribute( "xmlns", "http://www.opengis.net/wms" );
    wmsCapabilitiesElement.setAttribute( "xmlns:sld", "http://www.opengis.net/sld" );
    wmsCapabilitiesElement.setAttribute( "xmlns:qgs", "http://www.qgis.org/wms" );
    wmsCapabilitiesElement.setAttribute( "xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance" );
    QString schemaLocation = "http://www.opengis.net/wms";
    schemaLocation += " http://schemas.opengis.net/wms/1.3.0/capabilities_1_3_0.xsd";
    schemaLocation += " http://www.opengis.net/sld";
    schemaLocation += " http://schemas.opengis.net/sld/1.1.0/sld_capabilities.xsd";
    schemaLocation += " http://www.qgis.org/wms";
    if ( mConfigParser && mConfigParser->WMSInspireActivated() )
    {
      wmsCapabilitiesElement.setAttribute( "xmlns:inspire_common", "http://inspire.ec.europa.eu/schemas/common/1.0" );
      wmsCapabilitiesElement.setAttribute( "xmlns:inspire_vs", "http://inspire.ec.europa.eu/schemas/inspire_vs/1.0" );
      schemaLocation += " http://inspire.ec.europa.eu/schemas/inspire_vs/1.0";
      schemaLocation += " http://inspire.ec.europa.eu/schemas/inspire_vs/1.0/inspire_vs.xsd";
    }
    schemaLocation += " " + hrefString + "SERVICE=WMS&REQUEST=GetSchemaExtension";
    wmsCapabilitiesElement.setAttribute( "xsi:schemaLocation", schemaLocation );
  }
  wmsCapabilitiesElement.setAttribute( "version", version );
  doc.appendChild( wmsCapabilitiesElement );

  //todo: add service capabilities
  if ( mConfigParser )
  {
    mConfigParser->serviceCapabilities( wmsCapabilitiesElement, doc );
  }

  //wms:Capability element
  QDomElement capabilityElement = doc.createElement( "Capability"/*wms:Capability*/ );
  wmsCapabilitiesElement.appendChild( capabilityElement );
  //wms:Request element
  QDomElement requestElement = doc.createElement( "Request"/*wms:Request*/ );
  capabilityElement.appendChild( requestElement );

  QDomElement dcpTypeElement = doc.createElement( "DCPType"/*wms:DCPType*/ );
  QDomElement httpElement = doc.createElement( "HTTP"/*wms:HTTP*/ );
  dcpTypeElement.appendChild( httpElement );

  QDomElement elem;

  //wms:GetCapabilities
  elem = doc.createElement( "GetCapabilities"/*wms:GetCapabilities*/ );
  appendFormats( doc, elem, QStringList() << ( version == "1.1.1" ? "application/vnd.ogc.wms_xml" : "text/xml" ) );
  elem.appendChild( dcpTypeElement );
  requestElement.appendChild( elem );

  // SOAP platform
  //only give this information if it is not a WMS request to be in sync with the WMS capabilities schema
  QMap<QString, QString>::const_iterator service_it = mParameters.constFind( "SERVICE" );
  if ( service_it != mParameters.constEnd() && service_it.value().compare( "WMS", Qt::CaseInsensitive ) != 0 )
  {
    QDomElement soapElement = doc.createElement( "SOAP"/*wms:SOAP*/ );
    httpElement.appendChild( soapElement );
    QDomElement soapResourceElement = doc.createElement( "OnlineResource"/*wms:OnlineResource*/ );
    soapResourceElement.setAttribute( "xmlns:xlink", "http://www.w3.org/1999/xlink" );
    soapResourceElement.setAttribute( "xlink:type", "simple" );
    soapResourceElement.setAttribute( "xlink:href", hrefString );
    soapElement.appendChild( soapResourceElement );
  }

  //only Get supported for the moment
  QDomElement getElement = doc.createElement( "Get"/*wms:Get*/ );
  httpElement.appendChild( getElement );
  QDomElement olResourceElement = doc.createElement( "OnlineResource"/*wms:OnlineResource*/ );
  olResourceElement.setAttribute( "xmlns:xlink", "http://www.w3.org/1999/xlink" );
  olResourceElement.setAttribute( "xlink:type", "simple" );
  olResourceElement.setAttribute( "xlink:href", hrefString );
  getElement.appendChild( olResourceElement );

#if 0
  // POST already used by SOAP
  QDomElement postElement = doc.createElement( "post"/*wms:SOAP*/ );
  httpElement.appendChild( postElement );
  QDomElement postResourceElement = doc.createElement( "OnlineResource"/*wms:OnlineResource*/ );
  postResourceElement.setAttribute( "xmlns:xlink", "http://www.w3.org/1999/xlink" );
  postResourceElement.setAttribute( "xlink:type", "simple" );
  postResourceElement.setAttribute( "xlink:href", "http://" + QString( getenv( "SERVER_NAME" ) ) + QString( getenv( "REQUEST_URI" ) ) );
  postElement.appendChild( postResourceElement );
  dcpTypeElement.appendChild( postElement );
#endif

  //wms:GetMap
  elem = doc.createElement( "GetMap"/*wms:GetMap*/ );
  appendFormats( doc, elem, QStringList() << "image/jpeg" << "image/png" << "image/png; mode=16bit" << "image/png; mode=8bit" << "image/png; mode=1bit" << "application/dxf" );
  elem.appendChild( dcpTypeElement.cloneNode().toElement() ); //this is the same as for 'GetCapabilities'
  requestElement.appendChild( elem );

  //wms:GetFeatureInfo
  elem = doc.createElement( "GetFeatureInfo" );
  appendFormats( doc, elem, QStringList() << "text/plain" << "text/html" << "text/xml" << "application/vnd.ogc.gml" << "application/vnd.ogc.gml/3.1.1" );
  elem.appendChild( dcpTypeElement.cloneNode().toElement() ); //this is the same as for 'GetCapabilities'
  requestElement.appendChild( elem );

  //wms:GetLegendGraphic
  elem = doc.createElement(( version == "1.1.1" ? "GetLegendGraphic" : "sld:GetLegendGraphic" )/*wms:GetLegendGraphic*/ );
  appendFormats( doc, elem, QStringList() << "image/jpeg" << "image/png" );
  elem.appendChild( dcpTypeElement.cloneNode().toElement() ); // this is the same as for 'GetCapabilities'
  requestElement.appendChild( elem );

  //wms:DescribeLayer
  elem = doc.createElement(( version == "1.1.1" ? "DescribeLayer" : "sld:DescribeLayer" )/*wms:GetLegendGraphic*/ );
  appendFormats( doc, elem, QStringList() << "text/xml" );
  elem.appendChild( dcpTypeElement.cloneNode().toElement() ); // this is the same as for 'GetCapabilities'
  requestElement.appendChild( elem );

  //wms:GetStyles
  elem = doc.createElement(( version == "1.1.1" ? "GetStyles" : "qgs:GetStyles" )/*wms:GetStyles*/ );
  appendFormats( doc, elem, QStringList() << "text/xml" );
  elem.appendChild( dcpTypeElement.cloneNode().toElement() ); //this is the same as for 'GetCapabilities'
  requestElement.appendChild( elem );

  if ( fullProjectInformation ) //remove composer templates from GetCapabilities in the long term
  {
    //wms:GetPrint
    elem = doc.createElement( "GetPrint" /*wms:GetPrint*/ );
    appendFormats( doc, elem, QStringList() << "svg" << "png" << "pdf" );
    elem.appendChild( dcpTypeElement.cloneNode().toElement() ); //this is the same as for 'GetCapabilities'
    requestElement.appendChild( elem );
  }

  //Exception element is mandatory
  elem = doc.createElement( "Exception" );
  appendFormats( doc, elem, QStringList() << ( version == "1.1.1" ? "application/vnd.ogc.se_xml" : "text/xml" ) );
  capabilityElement.appendChild( elem );

  //UserDefinedSymbolization element
  if ( version == "1.3.0" )
  {
    elem = doc.createElement( "sld:UserDefinedSymbolization" );
    elem.setAttribute( "SupportSLD", "1" );
    elem.setAttribute( "UserLayer", "0" );
    elem.setAttribute( "UserStyle", "1" );
    elem.setAttribute( "RemoteWFS", "0" );
    elem.setAttribute( "InlineFeature", "0" );
    elem.setAttribute( "RemoteWCS", "0" );
    capabilityElement.appendChild( elem );

    if ( mConfigParser && mConfigParser->WMSInspireActivated() )
    {
      mConfigParser->inspireCapabilities( capabilityElement, doc );
    }
  }

  if ( mConfigParser && fullProjectInformation ) //remove composer templates from GetCapabilities in the long term
  {
    //Insert <ComposerTemplate> elements derived from wms:_ExtendedCapabilities
    mConfigParser->printCapabilities( capabilityElement, doc );
  }

  if ( mConfigParser && fullProjectInformation )
  {
    //WFS layers
    QStringList wfsLayers = mConfigParser->wfsLayerNames();
    if ( !wfsLayers.isEmpty() )
    {
      QDomElement wfsLayersElem = doc.createElement( "WFSLayers" );
      QStringList::const_iterator wfsIt = wfsLayers.constBegin();
      for ( ; wfsIt != wfsLayers.constEnd(); ++wfsIt )
      {
        QDomElement wfsLayerElem = doc.createElement( "WFSLayer" );
        wfsLayerElem.setAttribute( "name", *wfsIt );
        wfsLayersElem.appendChild( wfsLayerElem );
      }
      capabilityElement.appendChild( wfsLayersElem );
    }
  }

  //add the xml content for the individual layers/styles
  QgsMessageLog::logMessage( "calling layersAndStylesCapabilities" );
  if ( mConfigParser )
  {
    mConfigParser->layersAndStylesCapabilities( capabilityElement, doc, version, fullProjectInformation );
  }
  QgsMessageLog::logMessage( "layersAndStylesCapabilities returned" );

#if 0
  //for debugging: save the document to disk
  QFile capabilitiesFile( QDir::tempPath() + "/capabilities.txt" );
  if ( capabilitiesFile.open( QIODevice::WriteOnly | QIODevice::Text ) )
  {
    QTextStream capabilitiesStream( &capabilitiesFile );
    doc.save( capabilitiesStream, 4 );
  }
#endif

  return doc;
}

QDomDocument QgsWMSServer::getContext()
{
  QDomDocument doc;
  addXMLDeclaration( doc );
  QDomElement owsContextElem = doc.createElement( "OWSContext" );
  owsContextElem.setAttribute( "xmlns", "http://www.opengis.net/ows-context" );
  owsContextElem.setAttribute( "xmlns:ows-context", "http://www.opengis.net/ows-context" );
  owsContextElem.setAttribute( "xmlns:context", "http://www.opengis.net/context" );
  owsContextElem.setAttribute( "xmlns:ows", "http://www.opengis.net/ows" );
  owsContextElem.setAttribute( "xmlns:sld", "http://www.opengis.net/sld" );
  owsContextElem.setAttribute( "xmlns:ogc", "http://www.opengis.net/ogc" );
  owsContextElem.setAttribute( "xmlns:gml", "http://www.opengis.net/gml" );
  owsContextElem.setAttribute( "xmlns:kml", "http://www.opengis.net/kml/2.2" );
  owsContextElem.setAttribute( "xmlns:xlink", "http://www.w3.org/1999/xlink" );
  owsContextElem.setAttribute( "xmlns:ns9", "http://www.w3.org/2005/Atom" );
  owsContextElem.setAttribute( "xmlns:xal", "urn:oasis:names:tc:ciq:xsdschema:xAL:2.0" );
  owsContextElem.setAttribute( "xmlns:ins", "http://www.inspire.org" );
  owsContextElem.setAttribute( "version", "0.3.1" );
  doc.appendChild( owsContextElem );

  if ( mConfigParser )
  {
    //Prepare url
    QString hrefString = mConfigParser->serviceUrl();
    if ( hrefString.isEmpty() )
    {
      hrefString = serviceUrl();
    }
    mConfigParser->owsGeneralAndResourceList( owsContextElem, doc, hrefString );
  }

  return doc;
}


static QgsLayerTreeModelLegendNode* _findLegendNodeForRule( QgsLayerTreeModel* legendModel, const QString& rule )
{
  Q_FOREACH ( QgsLayerTreeLayer* nodeLayer, legendModel->rootGroup()->findLayers() )
  {
    Q_FOREACH ( QgsLayerTreeModelLegendNode* legendNode, legendModel->layerLegendNodes( nodeLayer ) )
    {
      if ( legendNode->data( Qt::DisplayRole ).toString() == rule )
        return legendNode;
    }
  }
  return nullptr;
}


static QgsRectangle _parseBBOX( const QString &bboxStr, bool &ok )
{
  ok = false;

  QStringList lst = bboxStr.split( "," );
  if ( lst.count() != 4 )
    return QgsRectangle();

  double d[4];
  for ( int i = 0; i < 4; i++ )
  {
    bool ok;
    lst[i].replace( " ", "+" );
    d[i] = lst[i].toDouble( &ok );
    if ( !ok )
      return QgsRectangle();
  }

  ok = true;
  return QgsRectangle( d[0], d[1], d[2], d[3] );
}


QImage* QgsWMSServer::getLegendGraphics()
{
  if ( !mConfigParser || !mMapRenderer )
  {
    return nullptr;
  }
  if ( !mParameters.contains( "LAYER" ) && !mParameters.contains( "LAYERS" ) )
  {
    throw QgsMapServiceException( "LayerNotSpecified", "LAYER is mandatory for GetLegendGraphic operation" );
  }
  if ( !mParameters.contains( "FORMAT" ) )
  {
    throw QgsMapServiceException( "FormatNotSpecified", "FORMAT is mandatory for GetLegendGraphic operation" );
  }

  bool contentBasedLegend = false;
  QgsRectangle contentBasedLegendExtent;

  if ( mParameters.contains( "BBOX" ) )
  {
    contentBasedLegend = true;

    bool bboxOk;
    contentBasedLegendExtent = _parseBBOX( mParameters["BBOX"], bboxOk );
    if ( !bboxOk || contentBasedLegendExtent.isEmpty() )
      throw QgsMapServiceException( "InvalidParameterValue", "Invalid BBOX parameter" );

    if ( mParameters.contains( "RULE" ) )
      throw QgsMapServiceException( "InvalidParameterValue", "BBOX parameter cannot be combined with RULE" );
  }

  QStringList layersList, stylesList;

  if ( readLayersAndStyles( layersList, stylesList ) != 0 )
  {
    QgsMessageLog::logMessage( "error reading layers and styles" );
    return nullptr;
  }

  if ( layersList.size() < 1 )
  {
    return nullptr;
  }

  //scale
  double scaleDenominator = -1;
  QMap<QString, QString>::const_iterator scaleIt = mParameters.constFind( "SCALE" );
  if ( scaleIt != mParameters.constEnd() )
  {
    bool conversionSuccess;
    double scaleValue = scaleIt.value().toDouble( &conversionSuccess );
    if ( conversionSuccess )
    {
      scaleDenominator = scaleValue;
    }
  }

  QgsCoordinateReferenceSystem dummyCRS;
  QStringList layerIds = layerSet( layersList, stylesList, dummyCRS, scaleDenominator );
  if ( layerIds.size() < 1 )
  {
    return nullptr;
  }

  //get icon size, spaces between legend items and font from config parser
  double boxSpace, layerSpace, layerTitleSpace, symbolSpace, iconLabelSpace, symbolWidth, symbolHeight;
  QFont layerFont, itemFont;
  QColor layerFontColor, itemFontColor;
  legendParameters( boxSpace, layerSpace, layerTitleSpace, symbolSpace,
                    iconLabelSpace, symbolWidth, symbolHeight, layerFont, itemFont, layerFontColor, itemFontColor );

  QString rule;
  int ruleSymbolWidth = 0, ruleSymbolHeight = 0;
  QMap<QString, QString>::const_iterator ruleIt = mParameters.constFind( "RULE" );
  if ( ruleIt != mParameters.constEnd() )
  {
    rule = ruleIt.value();

    QMap<QString, QString>::const_iterator widthIt = mParameters.constFind( "WIDTH" );
    if ( widthIt != mParameters.constEnd() )
    {
      bool conversionSuccess;
      double width = widthIt.value().toDouble( &conversionSuccess );
      if ( conversionSuccess )
      {
        ruleSymbolWidth = width;
      }
    }

    QMap<QString, QString>::const_iterator heightIt = mParameters.constFind( "HEIGHT" );
    if ( heightIt != mParameters.constEnd() )
    {
      bool conversionSuccess;
      double width = heightIt.value().toDouble( &conversionSuccess );
      if ( conversionSuccess )
      {
        ruleSymbolHeight = width;
      }
    }
  }

  // Checks showFeatureCount parameter
  bool showFeatureCount = false;
  if ( mParameters.contains( "SHOWFEATURECOUNT" ) )
    showFeatureCount = QVariant( mParameters[ "SHOWFEATURECOUNT" ] ).toBool();

  // Create the layer tree root
  QgsLayerTreeGroup rootGroup;
  // Store layers' name to reset them
  QMap<QString, QString> layerNameMap;
  // Create tree layer node for each layer
  Q_FOREACH ( const QString& layerId, layerIds )
  {
    // get layer
    QgsMapLayer *ml = QgsMapLayerRegistry::instance()->mapLayer( layerId );
    // create tree layer node
    QgsLayerTreeLayer *layer = rootGroup.addLayer( ml );
    // store the layer's name
    layerNameMap.insert( layerId, ml->name() );
    // set layer name with layer's title to have it in legend
    if ( !ml->title().isEmpty() )
      layer->setLayerName( ml->title() );
    // set show feature count
    if ( showFeatureCount )
      layer->setCustomProperty( "showFeatureCount", showFeatureCount );
  }
  QgsLayerTreeModel legendModel( &rootGroup );

  QList<QgsLayerTreeNode*> rootChildren = rootGroup.children();

  if ( scaleDenominator > 0 )
    legendModel.setLegendFilterByScale( scaleDenominator );

  if ( contentBasedLegend )
  {
    HitTest hitTest;
    getMap( &hitTest );

    Q_FOREACH ( QgsLayerTreeNode* node, rootGroup.children() )
    {
      Q_ASSERT( QgsLayerTree::isLayer( node ) );
      QgsLayerTreeLayer* nodeLayer = QgsLayerTree::toLayer( node );

      QgsVectorLayer* vl = qobject_cast<QgsVectorLayer*>( nodeLayer->layer() );
      if ( !vl || !vl->rendererV2() )
        continue;

      const SymbolV2Set& usedSymbols = hitTest[vl];
      QList<int> order;
      int i = 0;
      Q_FOREACH ( const QgsLegendSymbolItemV2& legendItem, vl->rendererV2()->legendSymbolItemsV2() )
      {
        if ( usedSymbols.contains( legendItem.legacyRuleKey() ) )
          order.append( i );
        ++i;
      }

      // either remove the whole layer or just filter out some items
      if ( order.isEmpty() )
        rootGroup.removeChildNode( nodeLayer );
      else
      {
        QgsMapLayerLegendUtils::setLegendNodeOrder( nodeLayer, order );
        legendModel.refreshLayerLegend( nodeLayer );
      }
    }
  }

  // find out DPI
  QImage* tmpImage = createImage( 1, 1 );
  if ( !tmpImage )
    return nullptr;
  qreal dpmm = tmpImage->dotsPerMeterX() / 1000.0;
  delete tmpImage;

  // setup legend configuration
  QgsLegendSettings legendSettings;
  legendSettings.setTitle( QString() );
  legendSettings.setBoxSpace( boxSpace );
  legendSettings.rstyle( QgsComposerLegendStyle::Subgroup ).setMargin( QgsComposerLegendStyle::Top, layerSpace );
  // TODO: not available: layer title space
  legendSettings.rstyle( QgsComposerLegendStyle::Symbol ).setMargin( QgsComposerLegendStyle::Top, symbolSpace );
  legendSettings.rstyle( QgsComposerLegendStyle::SymbolLabel ).setMargin( QgsComposerLegendStyle::Left, iconLabelSpace );
  legendSettings.setSymbolSize( QSizeF( symbolWidth, symbolHeight ) );
  legendSettings.rstyle( QgsComposerLegendStyle::Subgroup ).setFont( layerFont );
  legendSettings.rstyle( QgsComposerLegendStyle::SymbolLabel ).setFont( itemFont );
  // TODO: not available: layer font color
  legendSettings.setFontColor( itemFontColor );

  if ( contentBasedLegend )
  {
    legendSettings.setMapScale( mMapRenderer->scale() );
    double scaleFactor = mMapRenderer->outputUnits() == QgsMapRenderer::Millimeters ? mMapRenderer->outputDpi() / 25.4 : 1.0;
    legendSettings.setMmPerMapUnit( 1 / ( mMapRenderer->mapUnitsPerPixel() * scaleFactor ) );
  }

  if ( !rule.isEmpty() )
  {
    //create second image with the right dimensions
    QImage* paintImage = createImage( ruleSymbolWidth, ruleSymbolHeight );

    //go through the items a second time for painting
    QPainter p( paintImage );
    p.setRenderHint( QPainter::Antialiasing, true );
    p.scale( dpmm, dpmm );

    QgsLayerTreeModelLegendNode* legendNode = _findLegendNodeForRule( &legendModel, rule );
    if ( legendNode )
    {
      QgsLayerTreeModelLegendNode::ItemContext ctx;
      ctx.painter = &p;
      ctx.labelXOffset = 0;
      ctx.point = QPointF();
      double itemHeight = ruleSymbolHeight / dpmm;
      legendNode->drawSymbol( legendSettings, &ctx, itemHeight );
    }

    QgsMapLayerRegistry::instance()->removeAllMapLayers();
    return paintImage;
  }

  Q_FOREACH ( QgsLayerTreeNode* node, rootChildren )
  {
    if ( QgsLayerTree::isLayer( node ) )
    {
      QgsLayerTreeLayer* nodeLayer = QgsLayerTree::toLayer( node );

#ifdef HAVE_SERVER_PYTHON_PLUGINS
      if ( !mAccessControl->layerReadPermission( nodeLayer->layer() ) )
      {
        throw QgsMapServiceException( "Security", "You are not allowed to access to the layer: " + nodeLayer->layer()->name() );
      }
#endif

      // layer titles - hidden or not
      QgsLegendRenderer::setNodeLegendStyle( nodeLayer, mDrawLegendLayerLabel ? QgsComposerLegendStyle::Subgroup : QgsComposerLegendStyle::Hidden );

      // rule item titles
      if ( !mDrawLegendItemLabel )
      {
        Q_FOREACH ( QgsLayerTreeModelLegendNode* legendNode, legendModel.layerLegendNodes( nodeLayer ) )
        {
          legendNode->setUserLabel( " " ); // empty string = no override, so let's use one space
        }
      }
    }
  }

  QgsLegendRenderer legendRenderer( &legendModel, legendSettings );
  QSizeF minSize = legendRenderer.minimumSize();
  QSize s( minSize.width() * dpmm, minSize.height() * dpmm );

  QImage* paintImage = createImage( s.width(), s.height() );

  QPainter p( paintImage );
  p.setRenderHint( QPainter::Antialiasing, true );
  p.scale( dpmm, dpmm );

  legendRenderer.drawLegend( &p );

  p.end();

  // reset layers' name
  Q_FOREACH ( const QString& layerId, layerIds )
  {
    QgsMapLayer *ml = QgsMapLayerRegistry::instance()->mapLayer( layerId );
    ml->setName( layerNameMap[ layerId ] );
  }
  //  clear map layer registry
  QgsMapLayerRegistry::instance()->removeAllMapLayers();
  return paintImage;
}


void QgsWMSServer::runHitTest( QPainter* painter, HitTest& hitTest )
{
  QPaintDevice* thePaintDevice = painter->device();

  // setup QgsRenderContext in the same way as QgsMapRenderer does
  QgsRenderContext context;
  context.setPainter( painter ); // we are not going to draw anything, but we still need a working painter
  context.setRenderingStopped( false );
  context.setRasterScaleFactor(( thePaintDevice->logicalDpiX() + thePaintDevice->logicalDpiY() ) / 2.0 / mMapRenderer->outputDpi() );
  context.setScaleFactor( mMapRenderer->outputUnits() == QgsMapRenderer::Millimeters ? mMapRenderer->outputDpi() / 25.4 : 1.0 );
  context.setRendererScale( mMapRenderer->scale() );
  context.setMapToPixel( *mMapRenderer->coordinateTransform() );
  context.setExtent( mMapRenderer->extent() );

  Q_FOREACH ( const QString& layerID, mMapRenderer->layerSet() )
  {
    QgsVectorLayer* vl = qobject_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( layerID ) );
    if ( !vl || !vl->rendererV2() )
      continue;

    if ( vl->hasScaleBasedVisibility() && ( mMapRenderer->scale() < vl->minimumScale() || mMapRenderer->scale() > vl->maximumScale() ) )
    {
      hitTest[vl] = SymbolV2Set(); // no symbols -> will not be shown
      continue;
    }

    if ( mMapRenderer->hasCrsTransformEnabled() )
    {
      QgsRectangle r1 = mMapRenderer->extent(), r2;
      mMapRenderer->splitLayersExtent( vl, r1, r2 );
      if ( !r1.isFinite() || !r2.isFinite() ) //there was a problem transforming the extent. Skip the layer
        continue;
      context.setCoordinateTransform( mMapRenderer->transformation( vl ) );
      context.setExtent( r1 );
    }

    SymbolV2Set& usedSymbols = hitTest[vl];
    runHitTestLayer( vl, usedSymbols, context );
  }
}

void QgsWMSServer::runHitTestLayer( QgsVectorLayer* vl, SymbolV2Set& usedSymbols, QgsRenderContext& context )
{
  QgsFeatureRendererV2* r = vl->rendererV2();
  bool moreSymbolsPerFeature = r->capabilities() & QgsFeatureRendererV2::MoreSymbolsPerFeature;
  r->startRender( context, vl->pendingFields() );
  QgsFeature f;
  QgsFeatureRequest request( context.extent() );
  request.setFlags( QgsFeatureRequest::ExactIntersect );
  QgsFeatureIterator fi = vl->getFeatures( request );
  while ( fi.nextFeature( f ) )
  {
    context.expressionContext().setFeature( f );
    if ( moreSymbolsPerFeature )
    {
      Q_FOREACH ( QgsSymbolV2* s, r->originalSymbolsForFeature( f, context ) )
        usedSymbols.insert( s );
    }
    else
      usedSymbols.insert( r->originalSymbolForFeature( f, context ) );
  }
  r->stopRender( context );
}


void QgsWMSServer::legendParameters( double& boxSpace, double& layerSpace, double& layerTitleSpace,
                                     double& symbolSpace, double& iconLabelSpace, double& symbolWidth, double& symbolHeight,
                                     QFont& layerFont, QFont& itemFont, QColor& layerFontColor, QColor& itemFontColor )
{
  //spaces between legend elements
  QMap<QString, QString>::const_iterator boxSpaceIt = mParameters.constFind( "BOXSPACE" );
  boxSpace = ( boxSpaceIt == mParameters.constEnd() ) ? mConfigParser->legendBoxSpace() : boxSpaceIt.value().toDouble();
  QMap<QString, QString>::const_iterator layerSpaceIt = mParameters.constFind( "LAYERSPACE" );
  layerSpace = ( layerSpaceIt == mParameters.constEnd() ) ? mConfigParser->legendLayerSpace() : layerSpaceIt.value().toDouble();
  QMap<QString, QString>::const_iterator layerTitleSpaceIt = mParameters.constFind( "LAYERTITLESPACE" );
  layerTitleSpace = ( layerTitleSpaceIt == mParameters.constEnd() ) ? mConfigParser->legendLayerTitleSpace() : layerTitleSpaceIt.value().toDouble();
  QMap<QString, QString>::const_iterator symbolSpaceIt = mParameters.constFind( "SYMBOLSPACE" );
  symbolSpace = ( symbolSpaceIt == mParameters.constEnd() ) ? mConfigParser->legendSymbolSpace() : symbolSpaceIt.value().toDouble();
  QMap<QString, QString>::const_iterator iconLabelSpaceIt = mParameters.constFind( "ICONLABELSPACE" );
  iconLabelSpace = ( iconLabelSpaceIt == mParameters.constEnd() ) ? mConfigParser->legendIconLabelSpace() : iconLabelSpaceIt.value().toDouble();
  QMap<QString, QString>::const_iterator symbolWidthIt = mParameters.constFind( "SYMBOLWIDTH" );
  symbolWidth = ( symbolWidthIt == mParameters.constEnd() ) ? mConfigParser->legendSymbolWidth() : symbolWidthIt.value().toDouble();
  QMap<QString, QString>::const_iterator symbolHeightIt = mParameters.constFind( "SYMBOLHEIGHT" );
  symbolHeight = ( symbolHeightIt == mParameters.constEnd() ) ? mConfigParser->legendSymbolHeight() : symbolHeightIt.value().toDouble();

  //font properties
  layerFont = mConfigParser->legendLayerFont();
  QMap<QString, QString>::const_iterator layerFontFamilyIt = mParameters.constFind( "LAYERFONTFAMILY" );
  if ( layerFontFamilyIt != mParameters.constEnd() )
  {
    layerFont.setFamily( layerFontFamilyIt.value() );
  }
  QMap<QString, QString>::const_iterator layerFontBoldIt = mParameters.constFind( "LAYERFONTBOLD" );
  if ( layerFontBoldIt != mParameters.constEnd() )
  {
    layerFont.setBold( layerFontBoldIt.value().compare( "TRUE", Qt::CaseInsensitive ) == 0 );
  }
  QMap<QString, QString>::const_iterator layerFontItalicIt = mParameters.constFind( "LAYERFONTITALIC" );
  if ( layerFontItalicIt != mParameters.constEnd() )
  {
    layerFont.setItalic( layerFontItalicIt.value().compare( "TRUE", Qt::CaseInsensitive ) == 0 );
  }
  QMap<QString, QString>::const_iterator layerFontSizeIt = mParameters.constFind( "LAYERFONTSIZE" );
  layerFont.setPointSizeF( layerFontSizeIt != mParameters.constEnd() ? layerFontSizeIt.value().toDouble() : layerFont.pointSizeF() );
  QMap<QString, QString>::const_iterator layerFontColorIt = mParameters.constFind( "LAYERFONTCOLOR" );
  if ( layerFontColorIt != mParameters.constEnd() )
  {
    layerFontColor.setNamedColor( layerFontColorIt.value() );
  }
  else
  {
    layerFontColor = QColor( 0, 0, 0 );
  }
  QMap<QString, QString>::const_iterator layerTitleIt = mParameters.constFind( "LAYERTITLE" );
  if ( layerTitleIt != mParameters.constEnd() )
  {
    mDrawLegendLayerLabel = ( layerTitleIt.value().compare( "TRUE", Qt::CaseInsensitive ) == 0 );
  }
  else
  {
    mDrawLegendLayerLabel = true;
  }


  itemFont = mConfigParser->legendItemFont();
  QMap<QString, QString>::const_iterator itemFontFamilyIt = mParameters.constFind( "ITEMFONTFAMILY" );
  if ( itemFontFamilyIt != mParameters.constEnd() )
  {
    itemFont.setFamily( itemFontFamilyIt.value() );
  }
  QMap<QString, QString>::const_iterator itemFontBoldIt = mParameters.constFind( "ITEMFONTBOLD" );
  if ( itemFontBoldIt != mParameters.constEnd() )
  {
    itemFont.setBold( itemFontBoldIt.value().compare( "TRUE", Qt::CaseInsensitive ) == 0 );
  }
  QMap<QString, QString>::const_iterator itemFontItalicIt = mParameters.constFind( "ITEMFONTITALIC" );
  if ( itemFontItalicIt != mParameters.constEnd() )
  {
    itemFont.setItalic( itemFontItalicIt.value().compare( "TRUE", Qt::CaseInsensitive ) == 0 );
  }
  QMap<QString, QString>::const_iterator itemFontSizeIt = mParameters.constFind( "ITEMFONTSIZE" );
  itemFont.setPointSizeF( itemFontSizeIt != mParameters.constEnd() ? itemFontSizeIt.value().toDouble() : itemFont.pointSizeF() );
  QMap<QString, QString>::const_iterator itemFontColorIt = mParameters.constFind( "ITEMFONTCOLOR" );
  if ( itemFontColorIt != mParameters.constEnd() )
  {
    itemFontColor.setNamedColor( itemFontColorIt.value() );
  }
  else
  {
    itemFontColor = QColor( 0, 0, 0 );
  }
  QMap<QString, QString>::const_iterator itemLabelIt = mParameters.constFind( "RULELABEL" );
  if ( itemLabelIt != mParameters.constEnd() )
  {
    mDrawLegendItemLabel = ( itemLabelIt.value().compare( "TRUE", Qt::CaseInsensitive ) == 0 );
  }
  else
  {
    mDrawLegendItemLabel = true;
  }
}

QDomDocument QgsWMSServer::getSchemaExtension()
{
  QDomDocument xsdDoc;

  QFileInfo xsdFileInfo( "schemaExtension.xsd" );
  if ( !xsdFileInfo.exists() )
  {
    QgsMessageLog::logMessage( "Error, xsd file 'schemaExtension.xsd' does not exist", "Server", QgsMessageLog::CRITICAL );
    return xsdDoc;
  }

  QString xsdFilePath = xsdFileInfo.absoluteFilePath();
  QFile xsdFile( xsdFilePath );
  if ( !xsdFile.exists() )
  {
    QgsMessageLog::logMessage( "Error, xsd file 'schemaExtension.xsd' does not exist", "Server", QgsMessageLog::CRITICAL );
    return xsdDoc;
  }

  if ( !xsdFile.open( QIODevice::ReadOnly ) )
  {
    QgsMessageLog::logMessage( "Error, cannot open xsd file 'schemaExtension.xsd' does not exist", "Server", QgsMessageLog::CRITICAL );
    return xsdDoc;
  }

  QString errorMsg;
  int line, column;
  if ( !xsdDoc.setContent( &xsdFile, true, &errorMsg, &line, &column ) )
  {
    QgsMessageLog::logMessage( "Error parsing file 'schemaExtension.xsd" +
                               QString( "': parse error %1 at row %2, column %3" ).arg( errorMsg ).arg( line ).arg( column ), "Server", QgsMessageLog::CRITICAL );
    return xsdDoc;
  }
  return xsdDoc;
}

QDomDocument QgsWMSServer::getStyle()
{
  QDomDocument doc;
  if ( !mParameters.contains( "STYLE" ) )
  {
    throw QgsMapServiceException( "StyleNotSpecified", "Style is mandatory for GetStyle operation" );
  }

  if ( !mParameters.contains( "LAYER" ) )
  {
    throw QgsMapServiceException( "LayerNotSpecified", "Layer is mandatory for GetStyle operation" );
  }

  QString styleName = mParameters[ "STYLE" ];
  QString layerName = mParameters[ "LAYER" ];

  return mConfigParser->getStyle( styleName, layerName );
}

// GetStyles is defined for WMS1.1.1/SLD1.0 and in WMS 1.3.0 SLD Extension
QDomDocument QgsWMSServer::getStyles()
{
  QDomDocument doc;
  if ( !mParameters.contains( "LAYERS" ) )
  {
    throw QgsMapServiceException( "LayerNotSpecified", "Layers is mandatory for GetStyles operation" );
  }

  QStringList layersList = mParameters[ "LAYERS" ].split( ",", QString::SkipEmptyParts );
  if ( layersList.size() < 1 )
  {
    throw QgsMapServiceException( "LayerNotSpecified", "Layers is mandatory for GetStyles operation" );
  }

  return mConfigParser->getStyles( layersList );
}

// DescribeLayer is defined for WMS1.1.1/SLD1.0 and in WMS 1.3.0 SLD Extension
QDomDocument QgsWMSServer::describeLayer()
{
  if ( !mParameters.contains( "SLD_VERSION" ) )
  {
    throw QgsMapServiceException( "MissingParameterValue", "SLD_VERSION is mandatory for DescribeLayer operation" );
  }
  if ( mParameters[ "SLD_VERSION" ] != "1.1.0" )
  {
    throw QgsMapServiceException( "InvalidParameterValue", QString( "SLD_VERSION = %1 is not supported" ).arg( mParameters[ "SLD_VERSION" ] ) );
  }

  if ( !mParameters.contains( "LAYERS" ) )
  {
    throw QgsMapServiceException( "MissingParameterValue", "LAYERS is mandatory for DescribeLayer operation" );
  }

  QStringList layersList = mParameters[ "LAYERS" ].split( ",", QString::SkipEmptyParts );
  if ( layersList.size() < 1 )
  {
    throw QgsMapServiceException( "InvalidParameterValue", "Layers is empty" );
  }

  //Prepare url
  QString hrefString = mConfigParser->serviceUrl();
  if ( hrefString.isEmpty() )
  {
    hrefString = serviceUrl();
  }

  return mConfigParser->describeLayer( layersList, hrefString );
}

QByteArray* QgsWMSServer::getPrint( const QString& formatString )
{
  QStringList layersList, stylesList, layerIdList;
  QImage* theImage = initializeRendering( layersList, stylesList, layerIdList );
  if ( !theImage )
  {
    return nullptr;
  }
  delete theImage;

#ifdef HAVE_SERVER_PYTHON_PLUGINS
  Q_FOREACH ( QgsMapLayer *layer, QgsMapLayerRegistry::instance()->mapLayers() )
  {
    if ( !mAccessControl->layerReadPermission( layer ) )
    {
      throw QgsMapServiceException( "Security", "You are not allowed to access to the layer: " + layer->name() );
    }
  }
#endif

  //scoped pointer to restore all original layer filters (subsetStrings) when pointer goes out of scope
  //there's LOTS of potential exit paths here, so we avoid having to restore the filters manually
  QScopedPointer< QgsOWSServerFilterRestorer > filterRestorer( new QgsOWSServerFilterRestorer() );

  applyRequestedLayerFilters( layersList, filterRestorer->originalFilters() );

#ifdef HAVE_SERVER_PYTHON_PLUGINS
  applyAccessControlLayersFilters( layersList, filterRestorer->originalFilters() );
#endif

  QStringList selectedLayerIdList = applyFeatureSelections( layersList );

  //GetPrint request needs a template parameter
  if ( !mParameters.contains( "TEMPLATE" ) )
  {
    clearFeatureSelections( selectedLayerIdList );
    throw QgsMapServiceException( "ParameterMissing", "The TEMPLATE parameter is required for the GetPrint request" );
  }

  QList< QPair< QgsVectorLayer*, QgsFeatureRendererV2*> > bkVectorRenderers;
  QList< QPair< QgsRasterLayer*, QgsRasterRenderer* > > bkRasterRenderers;
  QList< QPair< QgsVectorLayer*, double > > labelTransparencies;
  QList< QPair< QgsVectorLayer*, double > > labelBufferTransparencies;

  applyOpacities( layersList, bkVectorRenderers, bkRasterRenderers, labelTransparencies, labelBufferTransparencies );

  QStringList highlightLayers;
  QgsComposition* c = mConfigParser->createPrintComposition( mParameters[ "TEMPLATE" ], mMapRenderer, QMap<QString, QString>( mParameters ), highlightLayers );
  if ( !c )
  {
    restoreOpacities( bkVectorRenderers, bkRasterRenderers, labelTransparencies, labelBufferTransparencies );
    clearFeatureSelections( selectedLayerIdList );
    QgsWMSConfigParser::removeHighlightLayers( highlightLayers );
    return nullptr;
  }

  QByteArray* ba = nullptr;
  c->setPlotStyle( QgsComposition::Print );

  //SVG export without a running X-Server is a problem. See e.g. http://developer.qt.nokia.com/forums/viewthread/2038
  if ( formatString.compare( "svg", Qt::CaseInsensitive ) == 0 )
  {
    c->setPlotStyle( QgsComposition::Print );

    QSvgGenerator generator;
    ba = new QByteArray();
    QBuffer svgBuffer( ba );
    generator.setOutputDevice( &svgBuffer );
    int width = ( int )( c->paperWidth() * c->printResolution() / 25.4 ); //width in pixel
    int height = ( int )( c->paperHeight() * c->printResolution() / 25.4 ); //height in pixel
    generator.setSize( QSize( width, height ) );
    generator.setResolution( c->printResolution() ); //because the rendering is done in mm, convert the dpi

    QPainter p( &generator );
    if ( c->printAsRaster() ) //embed one raster into the svg
    {
      QImage img = c->printPageAsRaster( 0 );
      p.drawImage( QRect( 0, 0, width, height ), img, QRectF( 0, 0, img.width(), img.height() ) );
    }
    else
    {
      c->renderPage( &p, 0 );
    }
    p.end();
  }
  else if ( formatString.compare( "png", Qt::CaseInsensitive ) == 0 || formatString.compare( "jpg", Qt::CaseInsensitive ) == 0 )
  {
    QImage image = c->printPageAsRaster( 0 ); //can only return the first page if pixmap is requested

    ba = new QByteArray();
    QBuffer buffer( ba );
    buffer.open( QIODevice::WriteOnly );
    image.save( &buffer, formatString.toLocal8Bit().data(), -1 );
  }
  else if ( formatString.compare( "pdf", Qt::CaseInsensitive ) == 0 )
  {
    QTemporaryFile tempFile;
    if ( !tempFile.open() )
    {
      delete c;
      restoreOpacities( bkVectorRenderers, bkRasterRenderers, labelTransparencies, labelBufferTransparencies );
      clearFeatureSelections( selectedLayerIdList );
      return nullptr;
    }

    c->exportAsPDF( tempFile.fileName() );
    ba = new QByteArray();
    *ba = tempFile.readAll();
  }
  else //unknown format
  {
    restoreOpacities( bkVectorRenderers, bkRasterRenderers, labelTransparencies, labelBufferTransparencies );
    clearFeatureSelections( selectedLayerIdList );
    throw QgsMapServiceException( "InvalidFormat", "Output format '" + formatString + "' is not supported in the GetPrint request" );
  }

  restoreOpacities( bkVectorRenderers, bkRasterRenderers, labelTransparencies, labelBufferTransparencies );
  clearFeatureSelections( selectedLayerIdList );
  QgsWMSConfigParser::removeHighlightLayers( highlightLayers );

  delete c;
  return ba;
}

#if 0
QImage* QgsWMSServer::printCompositionToImage( QgsComposition* c ) const
{
  int width = ( int )( c->paperWidth() * c->printResolution() / 25.4 ); //width in pixel
  int height = ( int )( c->paperHeight() * c->printResolution() / 25.4 ); //height in pixel
  QImage* image = new QImage( QSize( width, height ), QImage::Format_ARGB32 );
  image->setDotsPerMeterX( c->printResolution() / 25.4 * 1000 );
  image->setDotsPerMeterY( c->printResolution() / 25.4 * 1000 );
  image->fill( 0 );
  QPainter p( image );
  QRectF sourceArea( 0, 0, c->paperWidth(), c->paperHeight() );
  QRectF targetArea( 0, 0, width, height );
  c->render( &p, targetArea, sourceArea );
  p.end();
  return image;
}
#endif

QImage* QgsWMSServer::getMap( HitTest* hitTest )
{
  if ( !checkMaximumWidthHeight() )
  {
    throw QgsMapServiceException( "Size error", "The requested map size is too large" );
  }
  QStringList layersList, stylesList, layerIdList;
  QImage* theImage = initializeRendering( layersList, stylesList, layerIdList );

  QPainter thePainter( theImage );
  thePainter.setRenderHint( QPainter::Antialiasing ); //make it look nicer

  QStringList layerSet = mMapRenderer->layerSet();
  QStringList highlightLayers = QgsWMSConfigParser::addHighlightLayers( mParameters, layerSet );
  mMapRenderer->setLayerSet( layerSet );

#ifdef HAVE_SERVER_PYTHON_PLUGINS
  Q_FOREACH ( QgsMapLayer *layer, QgsMapLayerRegistry::instance()->mapLayers() )
  {
    if ( !mAccessControl->layerReadPermission( layer ) )
    {
      throw QgsMapServiceException( "Security", "You are not allowed to access to the layer: " + layer->name() );
    }
  }
#endif

  //scoped pointer to restore all original layer filters (subsetStrings) when pointer goes out of scope
  //there's LOTS of potential exit paths here, so we avoid having to restore the filters manually
  QScopedPointer< QgsOWSServerFilterRestorer > filterRestorer( new QgsOWSServerFilterRestorer() );

  applyRequestedLayerFilters( layersList, filterRestorer->originalFilters() );

#ifdef HAVE_SERVER_PYTHON_PLUGINS
  applyAccessControlLayersFilters( layersList, filterRestorer->originalFilters() );
#endif

  QStringList selectedLayerIdList = applyFeatureSelections( layersList );

  QList< QPair< QgsVectorLayer*, QgsFeatureRendererV2*> > bkVectorRenderers;
  QList< QPair< QgsRasterLayer*, QgsRasterRenderer* > > bkRasterRenderers;
  QList< QPair< QgsVectorLayer*, double > > labelTransparencies;
  QList< QPair< QgsVectorLayer*, double > > labelBufferTransparencies;

  applyOpacities( layersList, bkVectorRenderers, bkRasterRenderers, labelTransparencies, labelBufferTransparencies );

  if ( hitTest )
    runHitTest( &thePainter, *hitTest );
  else
  {
    mMapRenderer->render( &thePainter );
  }

  if ( mConfigParser )
  {
    //draw configuration format specific overlay items
    mConfigParser->drawOverlays( &thePainter, theImage->dotsPerMeterX() / 1000.0 * 25.4, theImage->width(), theImage->height() );
  }

  restoreOpacities( bkVectorRenderers, bkRasterRenderers, labelTransparencies, labelBufferTransparencies );
  clearFeatureSelections( selectedLayerIdList );
  QgsWMSConfigParser::removeHighlightLayers( highlightLayers );

  // QgsMessageLog::logMessage( "clearing filters" );
  if ( !hitTest )
    QgsMapLayerRegistry::instance()->removeAllMapLayers();

  //#ifdef QGISDEBUG
  //  theImage->save( QDir::tempPath() + QDir::separator() + "lastrender.png" );
  //#endif

  return theImage;
}

void QgsWMSServer::getMapAsDxf()
{
  QgsServerStreamingDevice d( "application/dxf" , mRequestHandler );
  if ( !d.open( QIODevice::WriteOnly ) )
  {
    throw QgsMapServiceException( "Internal server error", "Error opening output device for writing" );
  }

  QgsDxfExport dxf;

  //BBOX
  bool bboxOk;
  QgsRectangle extent = _parseBBOX( mParameters.value( "BBOX", "0,0,0,0" ), bboxOk );
  if ( !bboxOk )
  {
    extent = QgsRectangle();
  }
  dxf.setExtent( extent );

  //get format options (for MODE, USE_TITLE_AS_LAYERNAME, SCALE, LAYERATTRIBUTES)
  QMap<QString, QString > formatOptionsMap;
  readFormatOptions( formatOptionsMap );

  QList< QPair<QgsVectorLayer *, int > > layers;
  readDxfLayerSettings( layers, formatOptionsMap );
  dxf.addLayers( layers );

  dxf.setLayerTitleAsName( formatOptionsMap.contains( "USE_TITLE_AS_LAYERNAME" ) );

  //MODE
  QMap<QString, QString>::const_iterator modeIt = formatOptionsMap.find( "MODE" );

  QgsDxfExport::SymbologyExport se;
  if ( modeIt == formatOptionsMap.constEnd() )
  {
    se = QgsDxfExport::NoSymbology;
  }
  else
  {
    if ( modeIt->compare( "SymbolLayerSymbology", Qt::CaseInsensitive ) == 0 )
    {
      se = QgsDxfExport::SymbolLayerSymbology;
    }
    else if ( modeIt->compare( "FeatureSymbology", Qt::CaseInsensitive ) == 0 )
    {
      se = QgsDxfExport::FeatureSymbology;
    }
    else
    {
      se = QgsDxfExport::NoSymbology;
    }
  }
  dxf.setSymbologyExport( se );

  //SCALE
  QMap<QString, QString>::const_iterator scaleIt = formatOptionsMap.find( "SCALE" );
  if ( scaleIt != formatOptionsMap.constEnd() )
  {
    dxf.setSymbologyScaleDenominator( scaleIt->toDouble() );
  }

  QString codec = "ISO-8859-1";
  QMap<QString, QString>::const_iterator codecIt = formatOptionsMap.find( "CODEC" );
  if ( codecIt != formatOptionsMap.constEnd() )
  {
    codec = formatOptionsMap.value( "CODEC" );
  }

  dxf.writeToFile( &d, codec );
  d.close();
}

int QgsWMSServer::getFeatureInfo( QDomDocument& result, const QString& version )
{
  if ( !mMapRenderer || !mConfigParser )
  {
    return 1;
  }

  result.clear();
  QStringList layersList, stylesList;
  bool conversionSuccess;

  for ( QMap<QString, QString>::const_iterator it = mParameters.constBegin(); it != mParameters.constEnd(); ++it )
  {
    QgsMessageLog::logMessage( QString( "%1 // %2" ).arg( it.key(), it.value() ) );
  }

  if ( readLayersAndStyles( layersList, stylesList ) != 0 )
  {
    return 0;
  }
  if ( initializeSLDParser( layersList, stylesList ) != 0 )
  {
    return 0;
  }

  QImage* outputImage = createImage();
  if ( !outputImage )
  {
    return 1;
  }

  if ( configureMapRender( outputImage ) != 0 )
  {
    delete outputImage;
    return 2;
  }

  QgsMessageLog::logMessage( "mMapRenderer->extent(): " +  mMapRenderer->extent().toString() );
  QgsMessageLog::logMessage( QString( "mMapRenderer width = %1 height = %2" ).arg( mMapRenderer->outputSize().width() ).arg( mMapRenderer->outputSize().height() ) );
  QgsMessageLog::logMessage( QString( "mMapRenderer->mapUnitsPerPixel() = %1" ).arg( mMapRenderer->mapUnitsPerPixel() ) );

  //find out the current scale denominator and set it to the SLD parser
  QgsScaleCalculator scaleCalc(( outputImage->logicalDpiX() + outputImage->logicalDpiY() ) / 2, mMapRenderer->destinationCrs().mapUnits() );
  QgsRectangle mapExtent = mMapRenderer->extent();
  double scaleDenominator = scaleCalc.calculate( mapExtent, outputImage->width() );
  mConfigParser->setScaleDenominator( scaleDenominator );
  delete outputImage; //no longer needed for feature info

  //read FEATURE_COUNT
  int featureCount = 1;
  if ( mParameters.contains( "FEATURE_COUNT" ) )
  {
    featureCount = mParameters[ "FEATURE_COUNT" ].toInt( &conversionSuccess );
    if ( !conversionSuccess )
    {
      featureCount = 1;
    }
  }

  //read QUERY_LAYERS
  if ( !mParameters.contains( "QUERY_LAYERS" ) )
  {
    return 3;
  }

  QStringList queryLayerList = mParameters[ "QUERY_LAYERS" ].split( ",", QString::SkipEmptyParts );
  if ( queryLayerList.size() < 1 )
  {
    return 4;
  }

  //read I,J resp. X,Y
  QString iString = mParameters.value( "I", mParameters.value( "X" ) );
  int i = iString.toInt( &conversionSuccess );
  if ( !conversionSuccess )
  {
    i = -1;
  }

  QString jString = mParameters.value( "J", mParameters.value( "Y" ) );
  int j = jString.toInt( &conversionSuccess );
  if ( !conversionSuccess )
  {
    j = -1;
  }

  //Normally, I/J or X/Y are mandatory parameters.
  //However, in order to make attribute only queries via the FILTER parameter, it is allowed to skip them if the FILTER parameter is there

  QgsRectangle* featuresRect = nullptr;
  QScopedPointer<QgsPoint> infoPoint;

  if ( i == -1 || j == -1 )
  {
    if ( mParameters.contains( "FILTER" ) )
    {
      featuresRect = new QgsRectangle();
    }
    else
    {
      throw QgsMapServiceException( "ParameterMissing", "I/J parameters are required for GetFeatureInfo" );
    }
  }
  else
  {
    infoPoint.reset( new QgsPoint() );
    if ( !infoPointToMapCoordinates( i, j, infoPoint.data(), mMapRenderer ) )
    {
      return 5;
    }
  }

  //get the layer registered in QgsMapLayerRegistry and apply possible filters
  ( void )layerSet( layersList, stylesList, mMapRenderer->destinationCrs() );

  //scoped pointer to restore all original layer filters (subsetStrings) when pointer goes out of scope
  //there's LOTS of potential exit paths here, so we avoid having to restore the filters manually
  QScopedPointer< QgsOWSServerFilterRestorer > filterRestorer( new QgsOWSServerFilterRestorer() );
  applyRequestedLayerFilters( layersList, filterRestorer->originalFilters() );

#ifdef HAVE_SERVER_PYTHON_PLUGINS
  applyAccessControlLayersFilters( layersList, filterRestorer->originalFilters() );
#endif

  QDomElement getFeatureInfoElement;
  QString infoFormat = mParameters.value( "INFO_FORMAT" );
  if ( infoFormat.startsWith( "application/vnd.ogc.gml" ) )
  {
    getFeatureInfoElement = result.createElement( "wfs:FeatureCollection" );
    getFeatureInfoElement.setAttribute( "xmlns:wfs", "http://www.opengis.net/wfs" );
    getFeatureInfoElement.setAttribute( "xmlns:ogc", "http://www.opengis.net/ogc" );
    getFeatureInfoElement.setAttribute( "xmlns:gml", "http://www.opengis.net/gml" );
    getFeatureInfoElement.setAttribute( "xmlns:ows", "http://www.opengis.net/ows" );
    getFeatureInfoElement.setAttribute( "xmlns:xlink", "http://www.w3.org/1999/xlink" );
    getFeatureInfoElement.setAttribute( "xmlns:qgs", "http://qgis.org/gml" );
    getFeatureInfoElement.setAttribute( "xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance" );
    getFeatureInfoElement.setAttribute( "xsi:schemaLocation", "http://www.opengis.net/wfs http://schemas.opengis.net/wfs/1.0.0/wfs.xsd http://qgis.org/gml" );
  }
  else
  {
    QString featureInfoElemName = mConfigParser->featureInfoDocumentElement( "GetFeatureInfoResponse" );
    QString featureInfoElemNS = mConfigParser->featureInfoDocumentElementNS();
    if ( featureInfoElemNS.isEmpty() )
    {
      getFeatureInfoElement = result.createElement( featureInfoElemName );
    }
    else
    {
      getFeatureInfoElement = result.createElementNS( featureInfoElemNS, featureInfoElemName );
    }
    //feature info schema
    QString featureInfoSchema = mConfigParser->featureInfoSchema();
    if ( !featureInfoSchema.isEmpty() )
    {
      getFeatureInfoElement.setAttribute( "xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance" );
      getFeatureInfoElement.setAttribute( "xsi:schemaLocation", featureInfoSchema );
    }
  }
  result.appendChild( getFeatureInfoElement );

  QStringList nonIdentifiableLayers = mConfigParser->identifyDisabledLayers();

  //Render context is needed to determine feature visibility for vector layers
  QgsRenderContext renderContext;
  if ( mMapRenderer )
  {
    renderContext.setExtent( mMapRenderer->extent() );
    renderContext.setRasterScaleFactor( 1.0 );
    renderContext.setMapToPixel( *( mMapRenderer->coordinateTransform() ) );
    renderContext.setRendererScale( mMapRenderer->scale() );
    renderContext.setScaleFactor( mMapRenderer->outputDpi() / 25.4 );
    renderContext.setPainter( nullptr );
  }

  bool sia2045 = mConfigParser->featureInfoFormatSIA2045();

  //layers can have assigned a different name for GetCapabilities
  QHash<QString, QString> layerAliasMap = mConfigParser->featureInfoLayerAliasMap();

  QList<QgsMapLayer*> layerList;
  QgsMapLayer* currentLayer = nullptr;
  QStringList::const_iterator layerIt;
  for ( layerIt = queryLayerList.constBegin(); layerIt != queryLayerList.constEnd(); ++layerIt )
  {
    //create maplayers from sld parser (several layers are possible in case of feature info on a group)
    layerList = mConfigParser->mapLayerFromStyle( *layerIt, "" );
    QList<QgsMapLayer*>::iterator layerListIt = layerList.begin();
    for ( ; layerListIt != layerList.end(); ++layerListIt )
    {
      currentLayer = *layerListIt;
      if ( !currentLayer || nonIdentifiableLayers.contains( currentLayer->id() ) )
      {
        continue;
      }
      QgsMapLayer * registeredMapLayer = QgsMapLayerRegistry::instance()->mapLayer( currentLayer->id() );
      if ( registeredMapLayer )
      {
        currentLayer = registeredMapLayer;
      }

#ifdef HAVE_SERVER_PYTHON_PLUGINS
      if ( !mAccessControl->layerReadPermission( currentLayer ) )
      {
        throw QgsMapServiceException( "Security", "You are not allowed to access to the layer: " + currentLayer->name() );
      }
#endif

      //skip layer if not visible at current map scale
      bool useScaleConstraint = ( scaleDenominator > 0 && currentLayer->hasScaleBasedVisibility() );
      if ( useScaleConstraint && ( currentLayer->minimumScale() > scaleDenominator || currentLayer->maximumScale() < scaleDenominator ) )
      {
        continue;
      }

      //switch depending on vector or raster
      QgsVectorLayer* vectorLayer = dynamic_cast<QgsVectorLayer*>( currentLayer );

      QDomElement layerElement;
      if ( infoFormat.startsWith( "application/vnd.ogc.gml" ) )
      {
        layerElement = getFeatureInfoElement;
      }
      else
      {
        layerElement = result.createElement( "Layer" );
        QString layerName =  currentLayer->name();
        if ( mConfigParser && mConfigParser->useLayerIDs() )
          layerName = currentLayer->id();
        else if ( !currentLayer->shortName().isEmpty() )
          layerName = currentLayer->shortName();

        //check if the layer is given a different name for GetFeatureInfo output
        QHash<QString, QString>::const_iterator layerAliasIt = layerAliasMap.find( layerName );
        if ( layerAliasIt != layerAliasMap.constEnd() )
        {
          layerName = layerAliasIt.value();
        }
        layerElement.setAttribute( "name", layerName );
        getFeatureInfoElement.appendChild( layerElement );
        if ( sia2045 ) //the name might not be unique after alias replacement
        {
          layerElement.setAttribute( "id", currentLayer->id() );
        }
      }

      if ( vectorLayer )
      {
        if ( featureInfoFromVectorLayer( vectorLayer, infoPoint.data(), featureCount, result, layerElement, mMapRenderer, renderContext,
                                         version, infoFormat, featuresRect ) != 0 )
        {
          continue;
        }
      }
      else //raster layer
      {
        if ( infoFormat.startsWith( "application/vnd.ogc.gml" ) )
        {
          layerElement = result.createElement( "gml:featureMember"/*wfs:FeatureMember*/ );
          getFeatureInfoElement.appendChild( layerElement );
        }

        QgsRasterLayer* rasterLayer = dynamic_cast<QgsRasterLayer*>( currentLayer );
        if ( rasterLayer )
        {
          if ( !infoPoint.data() )
          {
            continue;
          }
          QgsPoint layerInfoPoint = mMapRenderer->mapToLayerCoordinates( currentLayer, *( infoPoint.data() ) );
          if ( featureInfoFromRasterLayer( rasterLayer, &layerInfoPoint, result, layerElement, version, infoFormat ) != 0 )
          {
            continue;
          }
        }
        else
        {
          continue;
        }
      }
    }
  }

  if ( featuresRect )
  {
    if ( infoFormat.startsWith( "application/vnd.ogc.gml" ) )
    {
      QDomElement bBoxElem = result.createElement( "gml:boundedBy" );
      QDomElement boxElem;
      int gmlVersion = infoFormat.startsWith( "application/vnd.ogc.gml/3" ) ? 3 : 2;
      if ( gmlVersion < 3 )
      {
        boxElem = QgsOgcUtils::rectangleToGMLBox( featuresRect, result, 8 );
      }
      else
      {
        boxElem = QgsOgcUtils::rectangleToGMLEnvelope( featuresRect, result, 8 );
      }

      QgsCoordinateReferenceSystem crs = mMapRenderer->destinationCrs();
      if ( crs.isValid() )
      {
        boxElem.setAttribute( "srsName", crs.authid() );
      }
      bBoxElem.appendChild( boxElem );
      getFeatureInfoElement.insertBefore( bBoxElem, QDomNode() ); //insert as first child
    }
    else
    {
      QDomElement bBoxElem = result.createElement( "BoundingBox" );
      bBoxElem.setAttribute( "CRS", mMapRenderer->destinationCrs().authid() );
      bBoxElem.setAttribute( "minx", qgsDoubleToString( featuresRect->xMinimum(), 8 ) );
      bBoxElem.setAttribute( "maxx", qgsDoubleToString( featuresRect->xMaximum(), 8 ) );
      bBoxElem.setAttribute( "miny", qgsDoubleToString( featuresRect->yMinimum(), 8 ) );
      bBoxElem.setAttribute( "maxy", qgsDoubleToString( featuresRect->yMaximum(), 8 ) );
      getFeatureInfoElement.insertBefore( bBoxElem, QDomNode() ); //insert as first child
    }
  }

  if ( sia2045 && infoFormat.compare( "text/xml", Qt::CaseInsensitive ) == 0 )
  {
    convertFeatureInfoToSIA2045( result );
  }

  //force restoration of original filters
  filterRestorer.reset();

  QgsMapLayerRegistry::instance()->removeAllMapLayers();
  delete featuresRect;
  return 0;
}

QImage* QgsWMSServer::initializeRendering( QStringList& layersList, QStringList& stylesList, QStringList& layerIdList )
{
  if ( !mConfigParser )
  {
    QgsMessageLog::logMessage( "Error: mSLDParser is 0" );
    return nullptr;
  }

  if ( !mMapRenderer )
  {
    QgsMessageLog::logMessage( "Error: mMapRenderer is 0" );
    return nullptr;
  }

  if ( readLayersAndStyles( layersList, stylesList ) != 0 )
  {
    QgsMessageLog::logMessage( "error reading layers and styles" );
    return nullptr;
  }

  if ( initializeSLDParser( layersList, stylesList ) != 0 )
  {
    return nullptr;
  }
  //pass external GML to the SLD parser.
  QString gml = mParameters.value( "GML" );
  if ( !gml.isEmpty() )
  {
    QDomDocument* gmlDoc = new QDomDocument();
    if ( gmlDoc->setContent( gml, true ) )
    {
      QString layerName = gmlDoc->documentElement().attribute( "layerName" );
      QgsMessageLog::logMessage( "Adding entry with key: " + layerName + " to external GML data" );
      mConfigParser->addExternalGMLData( layerName, gmlDoc );
    }
    else
    {
      QgsMessageLog::logMessage( "Error, could not add external GML to QgsSLDParser" );
      delete gmlDoc;
    }
  }

  QImage* theImage = createImage();
  if ( !theImage )
  {
    return nullptr;
  }

  if ( configureMapRender( theImage ) != 0 )
  {
    delete theImage;
    return nullptr;
  }

  //find out the current scale denominater and set it to the SLD parser
  QgsScaleCalculator scaleCalc(( theImage->logicalDpiX() + theImage->logicalDpiY() ) / 2, mMapRenderer->destinationCrs().mapUnits() );
  QgsRectangle mapExtent = mMapRenderer->extent();
  mConfigParser->setScaleDenominator( scaleCalc.calculate( mapExtent, theImage->width() ) );

  layerIdList = layerSet( layersList, stylesList, mMapRenderer->destinationCrs() );
#ifdef QGISDEBUG
  QgsMessageLog::logMessage( QString( "Number of layers to be rendered. %1" ).arg( layerIdList.count() ) );
#endif
  mMapRenderer->setLayerSet( layerIdList );

  // load label settings
  mConfigParser->loadLabelSettings( mMapRenderer->labelingEngine() );

  return theImage;
}

QImage* QgsWMSServer::createImage( int width, int height ) const
{
  bool conversionSuccess;

  if ( width < 0 )
  {
    width = mParameters.value( "WIDTH", "0" ).toInt( &conversionSuccess );
    if ( !conversionSuccess )
      width = 0;
  }

  if ( height < 0 )
  {
    height = mParameters.value( "HEIGHT", "0" ).toInt( &conversionSuccess );
    if ( !conversionSuccess )
    {
      height = 0;
    }
  }

  if ( width < 0 || height < 0 )
  {
    return nullptr;
  }

  QImage* theImage = nullptr;

  //is format jpeg?
  QString format = mParameters.value( "FORMAT" );
  bool jpeg = format.compare( "jpg", Qt::CaseInsensitive ) == 0
              || format.compare( "jpeg", Qt::CaseInsensitive ) == 0
              || format.compare( "image/jpeg", Qt::CaseInsensitive ) == 0;

  //transparent parameter
  bool transparent = mParameters.value( "TRANSPARENT" ).compare( "true", Qt::CaseInsensitive ) == 0;

  //use alpha channel only if necessary because it slows down performance
  if ( transparent && !jpeg )
  {
    theImage = new QImage( width, height, QImage::Format_ARGB32_Premultiplied );
    theImage->fill( 0 );
  }
  else
  {
    theImage = new QImage( width, height, QImage::Format_RGB32 );
    theImage->fill( qRgb( 255, 255, 255 ) );
  }

  if ( !theImage )
  {
    return nullptr;
  }

  //apply DPI parameter if present. This is an extension of Qgis Mapserver compared to WMS 1.3.
  //Because of backwards compatibility, this parameter is optional
  double OGC_PX_M = 0.00028; // OGC reference pixel size in meter, also used by qgis
  int dpm = 1 / OGC_PX_M;
  if ( mParameters.contains( "DPI" ) )
  {
    int dpi = mParameters[ "DPI" ].toInt( &conversionSuccess );
    if ( conversionSuccess )
    {
      dpm = dpi / 0.0254;
    }
  }
  theImage->setDotsPerMeterX( dpm );
  theImage->setDotsPerMeterY( dpm );
  return theImage;
}

int QgsWMSServer::configureMapRender( const QPaintDevice* paintDevice ) const
{
  if ( !mMapRenderer || !paintDevice )
  {
    return 1; //paint device is needed for height, width, dpi
  }

  mMapRenderer->clearLayerCoordinateTransforms();
  mMapRenderer->setOutputSize( QSize( paintDevice->width(), paintDevice->height() ), paintDevice->logicalDpiX() );

  //map extent
  bool bboxOk;
  QgsRectangle mapExtent = _parseBBOX( mParameters.value( "BBOX", "0,0,0,0" ), bboxOk );
  if ( !bboxOk )
  {
    //throw a service exception
    throw QgsMapServiceException( "InvalidParameterValue", "Invalid BBOX parameter" );
  }

  QGis::UnitType mapUnits = QGis::Degrees;

  QString crs = mParameters.value( "CRS", mParameters.value( "SRS" ) );

  QgsCoordinateReferenceSystem outputCRS;

  //wms spec says that CRS parameter is mandatory.
  //we don't reject the request if it is not there but disable reprojection on the fly
  if ( crs.isEmpty() )
  {
    //disable on the fly projection
    QgsProject::instance()->writeEntry( "SpatialRefSys", "/ProjectionsEnabled", 0 );
  }
  else
  {
    //enable on the fly projection
    QgsMessageLog::logMessage( "enable on the fly projection" );
    QgsProject::instance()->writeEntry( "SpatialRefSys", "/ProjectionsEnabled", 1 );

    //destination SRS
    outputCRS = QgsCRSCache::instance()->crsByOgcWmsCrs( crs );
    if ( !outputCRS.isValid() )
    {
      QgsMessageLog::logMessage( "Error, could not create output CRS from EPSG" );
      throw QgsMapServiceException( "InvalidCRS", "Could not create output CRS" );
    }

    //read layer coordinate transforms from project file (e.g. ct with special datum shift)
    if ( mConfigParser )
    {
      QList< QPair< QString, QgsLayerCoordinateTransform > > lt = mConfigParser->layerCoordinateTransforms();
      QList< QPair< QString, QgsLayerCoordinateTransform > >::const_iterator ltIt = lt.constBegin();
      for ( ; ltIt != lt.constEnd(); ++ltIt )
      {
        QgsLayerCoordinateTransform t = ltIt->second;
        mMapRenderer->addLayerCoordinateTransform( ltIt->first, t.srcAuthId, t.destAuthId, t.srcDatumTransform, t.destDatumTransform );
      }
    }

    //then set destinationCrs
    mMapRenderer->setDestinationCrs( outputCRS );
    mMapRenderer->setProjectionsEnabled( true );
    mapUnits = outputCRS.mapUnits();
  }
  mMapRenderer->setMapUnits( mapUnits );

  // Change x- and y- of BBOX for WMS 1.3.0 if axis inverted
  QString version = mParameters.value( "VERSION", "1.3.0" );
  if ( version != "1.1.1" && outputCRS.axisInverted() )
  {
    mapExtent.invert();
  }

  mMapRenderer->setExtent( mapExtent );

  if ( mConfigParser )
  {
    mMapRenderer->setOutputUnits( mConfigParser->outputUnits() );
  }
  else
  {
    mMapRenderer->setOutputUnits( QgsMapRenderer::Pixels ); //SLD units are in pixels normally
  }

  return 0;
}

int QgsWMSServer::readLayersAndStyles( QStringList& layersList, QStringList& stylesList ) const
{
  //get layer and style lists from the parameters trying LAYERS and LAYER as well as STYLE and STYLES for GetLegendGraphic compatibility
  layersList = mParameters.value( "LAYER" ).split( ",", QString::SkipEmptyParts );
  layersList = layersList + mParameters.value( "LAYERS" ).split( ",", QString::SkipEmptyParts );
  stylesList = mParameters.value( "STYLE" ).split( ",", QString::SkipEmptyParts );
  stylesList = stylesList + mParameters.value( "STYLES" ).split( ",", QString::SkipEmptyParts );

  return 0;
}

int QgsWMSServer::initializeSLDParser( QStringList& layersList, QStringList& stylesList )
{
  QString xml = mParameters.value( "SLD" );
  if ( !xml.isEmpty() )
  {
    //ignore LAYERS and STYLES and take those information from the SLD
    QDomDocument* theDocument = new QDomDocument( "user.sld" );
    QString errorMsg;
    int errorLine, errorColumn;

    if ( !theDocument->setContent( xml, true, &errorMsg, &errorLine, &errorColumn ) )
    {
      //std::cout << xml.toAscii().data() << std::endl;
      QgsMessageLog::logMessage( "Error, could not create DomDocument from SLD" );
      QgsMessageLog::logMessage( QString( "The error message is: %1" ).arg( errorMsg ) );
      delete theDocument;
      return 1;
    }

    QgsSLDConfigParser* userSLDParser = new QgsSLDConfigParser( theDocument, mParameters );
    userSLDParser->setFallbackParser( mConfigParser );
    mConfigParser = userSLDParser;
    mOwnsConfigParser = true;
    //now replace the content of layersList and stylesList (if present)
    layersList.clear();
    stylesList.clear();
    QStringList layersSTDList;
    QStringList stylesSTDList;
    if ( mConfigParser->layersAndStyles( layersSTDList, stylesSTDList ) != 0 )
    {
      QgsMessageLog::logMessage( "Error, no layers and styles found in SLD" );
      return 2;
    }
    QStringList::const_iterator layersIt;
    QStringList::const_iterator stylesIt;
    for ( layersIt = layersSTDList.constBegin(), stylesIt = stylesSTDList.constBegin(); layersIt != layersSTDList.constEnd(); ++layersIt, ++stylesIt )
    {
      layersList << *layersIt;
      stylesList << *stylesIt;
    }
  }
  return 0;
}

bool QgsWMSServer::infoPointToMapCoordinates( int i, int j, QgsPoint* infoPoint, QgsMapRenderer* mapRenderer )
{
  if ( !mapRenderer || !infoPoint )
  {
    return false;
  }

  double xRes = mapRenderer->extent().width() / mapRenderer->width();
  double yRes = mapRenderer->extent().height() / mapRenderer->height();
  infoPoint->setX( mapRenderer->extent().xMinimum() + i * xRes + xRes / 2.0 );
  infoPoint->setY( mapRenderer->extent().yMaximum() - j * yRes - yRes / 2.0 );
  return true;
}

int QgsWMSServer::featureInfoFromVectorLayer( QgsVectorLayer* layer,
    const QgsPoint* infoPoint,
    int nFeatures,
    QDomDocument& infoDocument,
    QDomElement& layerElement,
    QgsMapRenderer* mapRender,
    QgsRenderContext& renderContext,
    const QString& version,
    const QString& infoFormat,
    QgsRectangle* featureBBox ) const
{
  if ( !layer || !mapRender )
  {
    return 1;
  }

  //we need a selection rect (0.01 of map width)
  QgsRectangle mapRect = mapRender->extent();
  QgsRectangle layerRect = mapRender->mapToLayerCoordinates( layer, mapRect );


  QgsRectangle searchRect;

  //info point could be 0 in case there is only an attribute filter
  if ( infoPoint )
  {
    searchRect = featureInfoSearchRect( layer, mapRender, renderContext, *infoPoint );
  }
  else if ( mParameters.contains( "BBOX" ) )
  {
    searchRect = layerRect;
  }

  //do a select with searchRect and go through all the features

  QgsFeature feature;
  QgsAttributes featureAttributes;
  int featureCounter = 0;
  layer->updateFields();
  const QgsFields& fields = layer->pendingFields();
  bool addWktGeometry = mConfigParser && mConfigParser->featureInfoWithWktGeometry();
  const QSet<QString>& excludedAttributes = layer->excludeAttributesWMS();

  QgsFeatureRequest fReq;
  bool hasGeometry = addWktGeometry || featureBBox;
  fReq.setFlags((( hasGeometry ) ? QgsFeatureRequest::NoFlags : QgsFeatureRequest::NoGeometry ) | QgsFeatureRequest::ExactIntersect );

  if ( ! searchRect.isEmpty() )
  {
    fReq.setFilterRect( searchRect );
  }
  else
  {
    fReq.setFlags( fReq.flags() & ~ QgsFeatureRequest::ExactIntersect );
  }

#ifdef HAVE_SERVER_PYTHON_PLUGINS
  mAccessControl->filterFeatures( layer, fReq );

  QStringList attributes;
  QgsField field;
  Q_FOREACH ( field, layer->pendingFields().toList() )
  {
    attributes.append( field.name() );
  }
  attributes = mAccessControl->layerAttributes( layer, attributes );
  fReq.setSubsetOfAttributes( attributes, layer->pendingFields() );
#endif

  QgsFeatureIterator fit = layer->getFeatures( fReq );

  bool featureBBoxInitialized = false;
  while ( fit.nextFeature( feature ) )
  {
    ++featureCounter;
    if ( featureCounter > nFeatures )
    {
      break;
    }

    QgsFeatureRendererV2* r2 = layer->rendererV2();
    if ( !r2 )
    {
      continue;
    }

    renderContext.expressionContext().setFeature( feature );

    //check if feature is rendered at all
    r2->startRender( renderContext, layer->pendingFields() );
    bool renderV2 = r2->willRenderFeature( feature, renderContext );
    r2->stopRender( renderContext );
    if ( !renderV2 )
    {
      continue;
    }

    QgsRectangle box;
    if ( hasGeometry )
    {
      box = mapRender->layerExtentToOutputExtent( layer, feature.constGeometry()->boundingBox() );
      if ( featureBBox ) //extend feature info bounding box if requested
      {
        if ( !featureBBoxInitialized && featureBBox->isEmpty() )
        {
          *featureBBox = box;
          featureBBoxInitialized = true;
        }
        else
        {
          featureBBox->combineExtentWith( box );
        }
      }
    }

    QgsCoordinateReferenceSystem outputCrs = layer->crs();
    if ( layer->crs() != mapRender->destinationCrs() && mapRender->hasCrsTransformEnabled() )
    {
      outputCrs = mapRender->destinationCrs();
    }

    if ( infoFormat == "application/vnd.ogc.gml" )
    {
      bool withGeom = layer->wkbType() != QGis::WKBNoGeometry && addWktGeometry;
      int version = infoFormat.startsWith( "application/vnd.ogc.gml/3" ) ? 3 : 2;
      QString typeName =  layer->name();
      if ( mConfigParser && mConfigParser->useLayerIDs() )
        typeName = layer->id();
      else if ( !layer->shortName().isEmpty() )
        typeName = layer->shortName();
      QDomElement elem = createFeatureGML(
                           &feature, layer, infoDocument, outputCrs, typeName, withGeom, version
#ifdef HAVE_SERVER_PYTHON_PLUGINS
                           , &attributes
#endif
                         );
      QDomElement featureMemberElem = infoDocument.createElement( "gml:featureMember"/*wfs:FeatureMember*/ );
      featureMemberElem.appendChild( elem );
      layerElement.appendChild( featureMemberElem );
      continue;
    }
    else
    {
      QDomElement featureElement = infoDocument.createElement( "Feature" );
      featureElement.setAttribute( "id", FID_TO_STRING( feature.id() ) );
      layerElement.appendChild( featureElement );

      //read all attribute values from the feature
      featureAttributes = feature.attributes();
      for ( int i = 0; i < featureAttributes.count(); ++i )
      {
        //skip attribute if it is explicitly excluded from WMS publication
        if ( excludedAttributes.contains( fields[i].name() ) )
        {
          continue;
        }
#ifdef HAVE_SERVER_PYTHON_PLUGINS
        //skip attribute if it is excluded by access control
        if ( !attributes.contains( fields[i].name() ) )
        {
          continue;
        }
#endif

        //replace attribute name if there is an attribute alias?
        QString attributeName = layer->attributeDisplayName( i );

        QDomElement attributeElement = infoDocument.createElement( "Attribute" );
        attributeElement.setAttribute( "name", attributeName );
        attributeElement.setAttribute( "value",
                                       replaceValueMapAndRelation(
                                         layer, i,
                                         featureAttributes[i].isNull() ?  QString::null : QgsExpression::replaceExpressionText( featureAttributes[i].toString(), &renderContext.expressionContext() )
                                       )
                                     );
        featureElement.appendChild( attributeElement );
      }

      //add maptip attribute based on html/expression (in case there is no maptip attribute)
      if ( layer->fieldNameIndex( layer->displayField() ) < 0 )
      {
        QString displayField = layer->displayField();
        if ( !displayField.isEmpty() )
        {
          QDomElement maptipElem = infoDocument.createElement( "Attribute" );
          maptipElem.setAttribute( "name", "maptip" );
          maptipElem.setAttribute( "value",  QgsExpression::replaceExpressionText( displayField, &renderContext.expressionContext() ) );
          featureElement.appendChild( maptipElem );
        }
      }

      //append feature bounding box to feature info xml
      if ( hasGeometry && mapRender && mConfigParser )
      {
        QDomElement bBoxElem = infoDocument.createElement( "BoundingBox" );
        bBoxElem.setAttribute( version == "1.1.1" ? "SRS" : "CRS", outputCrs.authid() );
        bBoxElem.setAttribute( "minx", qgsDoubleToString( box.xMinimum(), getWMSPrecision( 8 ) ) );
        bBoxElem.setAttribute( "maxx", qgsDoubleToString( box.xMaximum(), getWMSPrecision( 8 ) ) );
        bBoxElem.setAttribute( "miny", qgsDoubleToString( box.yMinimum(), getWMSPrecision( 8 ) ) );
        bBoxElem.setAttribute( "maxy", qgsDoubleToString( box.yMaximum(), getWMSPrecision( 8 ) ) );
        featureElement.appendChild( bBoxElem );
      }

      //also append the wkt geometry as an attribute
      if ( addWktGeometry && hasGeometry )
      {
        QgsGeometry *geom = feature.geometry();
        if ( geom )
        {
          if ( layer->crs() != outputCrs )
          {
            const QgsCoordinateTransform *transform = mapRender->transformation( layer );
            if ( transform )
              geom->transform( *transform );
          }
          QDomElement geometryElement = infoDocument.createElement( "Attribute" );
          geometryElement.setAttribute( "name", "geometry" );
          geometryElement.setAttribute( "value", geom->exportToWkt( getWMSPrecision( 8 ) ) );
          geometryElement.setAttribute( "type", "derived" );
          featureElement.appendChild( geometryElement );
        }
      }
    }
  }

  return 0;
}

int QgsWMSServer::featureInfoFromRasterLayer( QgsRasterLayer* layer,
    const QgsPoint* infoPoint,
    QDomDocument& infoDocument,
    QDomElement& layerElement,
    const QString& version,
    const QString& infoFormat ) const
{
  Q_UNUSED( version );

  if ( !infoPoint || !layer || !layer->dataProvider() )
  {
    return 1;
  }

  QgsMessageLog::logMessage( QString( "infoPoint: %1 %2" ).arg( infoPoint->x() ).arg( infoPoint->y() ) );

  if ( !( layer->dataProvider()->capabilities() & QgsRasterDataProvider::IdentifyValue ) )
  {
    return 1;
  }
  QMap<int, QVariant> attributes;
  // use context extent, width height (comes with request) to use WCS cache
  // We can only use context if raster is not reprojected, otherwise it is difficult
  // to guess correct source resolution
  if ( mMapRenderer->hasCrsTransformEnabled() && layer->dataProvider()->crs() != mMapRenderer->destinationCrs() )
  {
    attributes = layer->dataProvider()->identify( *infoPoint, QgsRaster::IdentifyFormatValue ).results();
  }
  else
  {
    attributes = layer->dataProvider()->identify( *infoPoint, QgsRaster::IdentifyFormatValue, mMapRenderer->extent(), mMapRenderer->outputSize().width(), mMapRenderer->outputSize().height() ).results();
  }

  if ( infoFormat == "application/vnd.ogc.gml" )
  {
    QgsFeature feature;
    QgsFields fields;
    feature.initAttributes( attributes.count() );
    int index = 0;
    for ( QMap<int, QVariant>::const_iterator it = attributes.constBegin(); it != attributes.constEnd(); ++it )
    {
      fields.append( QgsField( layer->bandName( it.key() ), QVariant::Double ) );
      feature.setAttribute( index++, QString::number( it.value().toDouble() ) );
    }
    feature.setFields( fields );

    QgsCoordinateReferenceSystem layerCrs = layer->crs();
    int version = infoFormat.startsWith( "application/vnd.ogc.gml/3" ) ? 3 : 2;
    QString typeName =  layer->name();
    if ( mConfigParser && mConfigParser->useLayerIDs() )
      typeName = layer->id();
    else if ( !layer->shortName().isEmpty() )
      typeName = layer->shortName();
    QDomElement elem = createFeatureGML(
                         &feature, nullptr, infoDocument, layerCrs, typeName, false, version, nullptr );
    layerElement.appendChild( elem );
  }
  else
  {
    for ( QMap<int, QVariant>::const_iterator it = attributes.constBegin(); it != attributes.constEnd(); ++it )
    {
      QDomElement attributeElement = infoDocument.createElement( "Attribute" );
      attributeElement.setAttribute( "name", layer->bandName( it.key() ) );
      attributeElement.setAttribute( "value", QString::number( it.value().toDouble() ) );
      layerElement.appendChild( attributeElement );
    }
  }
  return 0;
}

QStringList QgsWMSServer::layerSet( const QStringList &layersList,
                                    const QStringList &stylesList,
                                    const QgsCoordinateReferenceSystem &destCRS, double scaleDenominator ) const
{
  Q_UNUSED( destCRS );
  QStringList layerKeys;
  QStringList::const_iterator llstIt;
  QStringList::const_iterator slstIt;
  QgsMapLayer* theMapLayer = nullptr;
  QgsMessageLog::logMessage( QString( "Calculating layerset using %1 layers, %2 styles and CRS %3" ).arg( layersList.count() ).arg( stylesList.count() ).arg( destCRS.description() ) );
  for ( llstIt = layersList.begin(), slstIt = stylesList.begin(); llstIt != layersList.end(); ++llstIt )
  {
    QString styleName;
    if ( slstIt != stylesList.end() )
    {
      styleName = *slstIt;
    }
    QgsMessageLog::logMessage( "Trying to get layer " + *llstIt + "//" + styleName );

    //does the layer name appear several times in the layer list?
    //if yes, layer caching must be disabled because several named layers could have
    //several user styles
    bool allowCaching = true;
    if ( layersList.count( *llstIt ) > 1 )
    {
      allowCaching = false;
    }

    QList<QgsMapLayer*> layerList = mConfigParser->mapLayerFromStyle( *llstIt, styleName, allowCaching );
    int listIndex;

    for ( listIndex = layerList.size() - 1; listIndex >= 0; listIndex-- )
    {
      theMapLayer = layerList.at( listIndex );
      if ( theMapLayer )
      {
        QString lName =  theMapLayer->name();
        if ( mConfigParser && mConfigParser->useLayerIDs() )
          lName = theMapLayer->id();
        else if ( !theMapLayer->shortName().isEmpty() )
          lName = theMapLayer->shortName();
        QgsMessageLog::logMessage( QString( "Checking layer: %1" ).arg( lName ) );
        //test if layer is visible in requested scale
        bool useScaleConstraint = ( scaleDenominator > 0 && theMapLayer->hasScaleBasedVisibility() );
        if ( !useScaleConstraint ||
             ( theMapLayer->minimumScale() <= scaleDenominator && theMapLayer->maximumScale() >= scaleDenominator ) )
        {
          layerKeys.push_front( theMapLayer->id() );
          QgsMapLayerRegistry::instance()->addMapLayers(
            QList<QgsMapLayer *>() << theMapLayer, false, false );
        }
      }
      else
      {
        QgsMessageLog::logMessage( "Layer or style not defined, aborting" );
        throw QgsMapServiceException( "LayerNotDefined", "Layer '" + *llstIt + "' and/or style '" + styleName + "' not defined" );
      }
    }

    if ( slstIt != stylesList.end() )
    {
      ++slstIt;
    }
  }
  return layerKeys;
}


void QgsWMSServer::applyRequestedLayerFilters( const QStringList& layerList , QHash<QgsMapLayer*, QString>& originalFilters ) const
{
  if ( layerList.isEmpty() )
  {
    return;
  }

  QString filterParameter = mParameters.value( "FILTER" );
  if ( !filterParameter.isEmpty() )
  {
    QStringList layerSplit = filterParameter.split( ";" );
    QStringList::const_iterator layerIt = layerSplit.constBegin();
    for ( ; layerIt != layerSplit.constEnd(); ++layerIt )
    {
      QStringList eqSplit = layerIt->split( ":" );
      if ( eqSplit.size() < 2 )
      {
        continue;
      }

      //filter string could be unsafe (danger of sql injection)
      if ( !testFilterStringSafety( eqSplit.at( 1 ) ) )
      {
        throw QgsMapServiceException( "Filter string rejected", "The filter string " + eqSplit.at( 1 ) +
                                      " has been rejected because of security reasons. Note: Text strings have to be enclosed in single or double quotes. " +
                                      "A space between each word / special character is mandatory. Allowed Keywords and special characters are " +
                                      "AND,OR,IN,<,>=,>,>=,!=,',',(,),DMETAPHONE,SOUNDEX. Not allowed are semicolons in the filter expression." );
      }

      //we need to find the maplayer objects matching the layer name
      QList<QgsMapLayer*> layersToFilter;

      Q_FOREACH ( QgsMapLayer *layer, QgsMapLayerRegistry::instance()->mapLayers() )
      {
        if ( layer )
        {
          QString lName =  layer->name();
          if ( mConfigParser && mConfigParser->useLayerIDs() )
            lName = layer->id();
          else if ( !layer->shortName().isEmpty() )
            lName = layer->shortName();
          if ( lName == eqSplit.at( 0 ) )
            layersToFilter.push_back( layer );
        }
      }

      Q_FOREACH ( QgsMapLayer *filter, layersToFilter )
      {
        QgsVectorLayer* filteredLayer = dynamic_cast<QgsVectorLayer*>( filter );
        if ( filteredLayer )
        {
          originalFilters.insert( filteredLayer, filteredLayer->subsetString() );
          QString newSubsetString = eqSplit.at( 1 );
          if ( !filteredLayer->subsetString().isEmpty() )
          {
            newSubsetString.prepend( " AND " );
            newSubsetString.prepend( filteredLayer->subsetString() );
          }
          filteredLayer->setSubsetString( newSubsetString );
        }
      }
    }

    //No BBOX parameter in request. We use the union of the filtered layer
    //to provide the functionality of zooming to selected records via (enhanced) WMS.
    if ( mMapRenderer && mMapRenderer->extent().isEmpty() )
    {
      QgsRectangle filterExtent;
      QHash<QgsMapLayer*, QString>::const_iterator filterIt = originalFilters.constBegin();
      for ( ; filterIt != originalFilters.constEnd(); ++filterIt )
      {
        QgsMapLayer* mapLayer = filterIt.key();
        if ( !mapLayer )
        {
          continue;
        }

        QgsRectangle layerExtent = mMapRenderer->layerToMapCoordinates( mapLayer, mapLayer->extent() );
        if ( filterExtent.isEmpty() )
        {
          filterExtent = layerExtent;
        }
        else
        {
          filterExtent.combineExtentWith( layerExtent );
        }
      }
      mMapRenderer->setExtent( filterExtent );
    }
  }
}

#ifdef HAVE_SERVER_PYTHON_PLUGINS
void QgsWMSServer::applyAccessControlLayersFilters( const QStringList& layerList, QHash<QgsMapLayer*, QString>& originalLayerFilters ) const
{
  Q_FOREACH ( const QString& layerName, layerList )
  {
    QList<QgsMapLayer*> mapLayers = QgsMapLayerRegistry::instance()->mapLayersByName( layerName );
    Q_FOREACH ( QgsMapLayer* mapLayer, mapLayers )
    {
      applyAccessControlLayerFilters( mapLayer, originalLayerFilters );
    }
  }
}
#endif

bool QgsWMSServer::testFilterStringSafety( const QString& filter ) const
{
  //; too dangerous for sql injections
  if ( filter.contains( ";" ) )
  {
    return false;
  }

  QStringList tokens = filter.split( " ", QString::SkipEmptyParts );
  groupStringList( tokens, "'" );
  groupStringList( tokens, "\"" );

  QStringList::const_iterator tokenIt = tokens.constBegin();
  for ( ; tokenIt != tokens.constEnd(); ++tokenIt )
  {
    //whitelist of allowed characters and keywords
    if ( tokenIt->compare( "," ) == 0
         || tokenIt->compare( "(" ) == 0
         || tokenIt->compare( ")" ) == 0
         || tokenIt->compare( "=" ) == 0
         || tokenIt->compare( "!=" ) == 0
         || tokenIt->compare( "<" ) == 0
         || tokenIt->compare( "<=" ) == 0
         || tokenIt->compare( ">" ) == 0
         || tokenIt->compare( ">=" ) == 0
         || tokenIt->compare( "%" ) == 0
         || tokenIt->compare( "AND", Qt::CaseInsensitive ) == 0
         || tokenIt->compare( "OR", Qt::CaseInsensitive ) == 0
         || tokenIt->compare( "IN", Qt::CaseInsensitive ) == 0
         || tokenIt->compare( "LIKE", Qt::CaseInsensitive ) == 0
         || tokenIt->compare( "ILIKE", Qt::CaseInsensitive ) == 0
         || tokenIt->compare( "DMETAPHONE", Qt::CaseInsensitive ) == 0
         || tokenIt->compare( "SOUNDEX", Qt::CaseInsensitive ) == 0 )
    {
      continue;
    }

    //numbers are ok
    bool isNumeric;
    tokenIt->toDouble( &isNumeric );
    if ( isNumeric )
    {
      continue;
    }

    //numeric strings need to be quoted once either with single or with double quotes

    //empty strings are ok
    if ( *tokenIt == "''" )
    {
      continue;
    }

    //single quote
    if ( tokenIt->size() > 2
         && ( *tokenIt )[0] == QChar( '\'' )
         && ( *tokenIt )[tokenIt->size() - 1] == QChar( '\'' )
         && ( *tokenIt )[1] != QChar( '\'' )
         && ( *tokenIt )[tokenIt->size() - 2] != QChar( '\'' ) )
    {
      continue;
    }

    //double quote
    if ( tokenIt->size() > 2
         && ( *tokenIt )[0] == QChar( '"' )
         && ( *tokenIt )[tokenIt->size() - 1] == QChar( '"' )
         && ( *tokenIt )[1] != QChar( '"' )
         && ( *tokenIt )[tokenIt->size() - 2] != QChar( '"' ) )
    {
      continue;
    }

    return false;
  }

  return true;
}

void QgsWMSServer::groupStringList( QStringList& list, const QString& groupString )
{
  //group contents within single quotes together
  bool groupActive = false;
  int startGroup = -1;
  QString concatString;

  for ( int i = 0; i < list.size(); ++i )
  {
    QString& str = list[i];
    if ( str.startsWith( groupString ) )
    {
      startGroup = i;
      groupActive = true;
      concatString.clear();
    }

    if ( groupActive )
    {
      if ( i != startGroup )
      {
        concatString.append( " " );
      }
      concatString.append( str );
    }

    if ( str.endsWith( groupString ) )
    {
      int endGroup = i;
      groupActive = false;

      if ( startGroup != -1 )
      {
        list[startGroup] = concatString;
        for ( int j = startGroup + 1; j <= endGroup; ++j )
        {
          list.removeAt( startGroup + 1 );
          --i;
        }
      }

      concatString.clear();
      startGroup = -1;
    }
  }
}

QStringList QgsWMSServer::applyFeatureSelections( const QStringList& layerList ) const
{
  QStringList layersWithSelections;
  if ( layerList.isEmpty() )
  {
    return layersWithSelections;
  }

  QString selectionString = mParameters.value( "SELECTION" );
  if ( selectionString.isEmpty() )
  {
    return layersWithSelections;
  }

  Q_FOREACH ( const QString& selectionLayer, selectionString.split( ";" ) )
  {
    //separate layer name from id list
    QStringList layerIdSplit = selectionLayer.split( ":" );
    if ( layerIdSplit.size() < 2 )
    {
      continue;
    }

    //find layerId for layer name
    QString layerName = layerIdSplit.at( 0 );
    QgsVectorLayer* vLayer = nullptr;

    Q_FOREACH ( QgsMapLayer *layer, QgsMapLayerRegistry::instance()->mapLayers() )
    {
      if ( layer )
      {
        QString lName =  layer->name();
        if ( mConfigParser && mConfigParser->useLayerIDs() )
          lName = layer->id();
        else if ( !layer->shortName().isEmpty() )
          lName = layer->shortName();
        if ( lName == layerName )
        {
          vLayer = qobject_cast<QgsVectorLayer*>( layer );
          layersWithSelections.push_back( vLayer->id() );
          break;
        }
      }
    }

    if ( !vLayer )
    {
      continue;
    }

    QStringList idList = layerIdSplit.at( 1 ).split( "," );
    QgsFeatureIds selectedIds;

    Q_FOREACH ( const QString& id, idList )
    {
      selectedIds.insert( STRING_TO_FID( id ) );
    }

    vLayer->selectByIds( selectedIds );
  }


  return layersWithSelections;
}

void QgsWMSServer::clearFeatureSelections( const QStringList& layerIds ) const
{
  const QMap<QString, QgsMapLayer*>& layerMap = QgsMapLayerRegistry::instance()->mapLayers();

  Q_FOREACH ( const QString& id, layerIds )
  {
    QgsVectorLayer *layer = qobject_cast< QgsVectorLayer * >( layerMap.value( id, nullptr ) );
    if ( !layer )
      continue;

    layer->selectByIds( QgsFeatureIds() );
  }

  return;
}

void QgsWMSServer::applyOpacities( const QStringList& layerList, QList< QPair< QgsVectorLayer*, QgsFeatureRendererV2*> >& vectorRenderers,
                                   QList< QPair< QgsRasterLayer*, QgsRasterRenderer* > >& rasterRenderers,
                                   QList< QPair< QgsVectorLayer*, double > >& labelTransparencies,
                                   QList< QPair< QgsVectorLayer*, double > >& labelBufferTransparencies )
{
  //get opacity list
  QMap<QString, QString>::const_iterator opIt = mParameters.constFind( "OPACITIES" );
  if ( opIt == mParameters.constEnd() )
  {
    return;
  }
  QStringList opacityList = opIt.value().split( "," );

  //collect leaf layers and their opacity
  QVector< QPair< QgsMapLayer*, int > > layerOpacityList;
  QStringList::const_iterator oIt = opacityList.constBegin();
  QStringList::const_iterator lIt = layerList.constBegin();
  for ( ; oIt != opacityList.constEnd() && lIt != layerList.constEnd(); ++oIt, ++lIt )
  {
    //get layer list for
    int opacity = oIt->toInt();
    if ( opacity < 0 || opacity > 255 )
    {
      continue;
    }
    QList<QgsMapLayer*> llist = mConfigParser->mapLayerFromStyle( *lIt, "" );
    QList<QgsMapLayer*>::const_iterator lListIt = llist.constBegin();
    for ( ; lListIt != llist.constEnd(); ++lListIt )
    {
      layerOpacityList.push_back( qMakePair( *lListIt, opacity ) );
    }
  }

  QVector< QPair< QgsMapLayer*, int > >::const_iterator lOpIt = layerOpacityList.constBegin();
  for ( ; lOpIt != layerOpacityList.constEnd(); ++lOpIt )
  {
    //vector or raster?
    QgsMapLayer* ml = lOpIt->first;
    int opacity = lOpIt->second;
    double opacityRatio = opacity / 255.0; //opacity value between 0 and 1

    if ( !ml || opacity == 255 )
    {
      continue;
    }

    if ( ml->type() == QgsMapLayer::VectorLayer )
    {
      QgsVectorLayer* vl = qobject_cast<QgsVectorLayer*>( ml );

      QgsFeatureRendererV2* rendererV2 = vl->rendererV2();
      //backup old renderer
      vectorRenderers.push_back( qMakePair( vl, rendererV2->clone() ) );
      //modify symbols of current renderer
      QgsRenderContext context;
      context.expressionContext() << QgsExpressionContextUtils::globalScope()
      << QgsExpressionContextUtils::projectScope()
      << QgsExpressionContextUtils::layerScope( vl );

      QgsSymbolV2List symbolList = rendererV2->symbols( context );
      QgsSymbolV2List::iterator symbolIt = symbolList.begin();
      for ( ; symbolIt != symbolList.end(); ++symbolIt )
      {
        ( *symbolIt )->setAlpha(( *symbolIt )->alpha() * opacityRatio );
      }

      //labeling
      if ( vl->customProperty( "labeling/enabled" ).toString() == "true" )
      {
        double labelTransparency = vl->customProperty( "labeling/textTransp" ).toDouble();
        labelTransparencies.push_back( qMakePair( vl, labelTransparency ) );
        vl->setCustomProperty( "labeling/textTransp", labelTransparency + ( 100 - labelTransparency ) * ( 1.0 - opacityRatio ) );
        double bufferTransparency = vl->customProperty( "labeling/bufferTransp" ).toDouble();
        labelBufferTransparencies.push_back( qMakePair( vl, bufferTransparency ) );
        vl->setCustomProperty( "labeling/bufferTransp", bufferTransparency + ( 100 - bufferTransparency )* ( 1.0 - opacityRatio ) );
      }
    }
    else if ( ml->type() == QgsMapLayer::RasterLayer )
    {
      QgsRasterLayer* rl = qobject_cast<QgsRasterLayer*>( ml );
      if ( rl )
      {
        QgsRasterRenderer* rasterRenderer = rl->renderer();
        if ( rasterRenderer )
        {
          rasterRenderers.push_back( qMakePair( rl, rasterRenderer->clone() ) );
          rasterRenderer->setOpacity( rasterRenderer->opacity() * opacityRatio );
        }
      }
    }
  }
}

void QgsWMSServer::restoreOpacities( QList< QPair< QgsVectorLayer*, QgsFeatureRendererV2*> >& vectorRenderers,
                                     QList < QPair< QgsRasterLayer*, QgsRasterRenderer* > >& rasterRenderers,
                                     QList< QPair< QgsVectorLayer*, double > >& labelOpacities,
                                     QList< QPair< QgsVectorLayer*, double > >& labelBufferOpacities )
{
  if ( vectorRenderers.isEmpty() && rasterRenderers.isEmpty() )
  {
    return;
  }

  QList< QPair< QgsVectorLayer*, QgsFeatureRendererV2*> >::iterator vIt = vectorRenderers.begin();
  for ( ; vIt != vectorRenderers.end(); ++vIt )
  {
    ( *vIt ).first->setRendererV2(( *vIt ).second );
  }

  QList< QPair< QgsRasterLayer*, QgsRasterRenderer* > >::iterator rIt = rasterRenderers.begin();
  for ( ; rIt != rasterRenderers.end(); ++rIt )
  {
    ( *rIt ).first->setRenderer(( *rIt ).second );
  }

  QList< QPair< QgsVectorLayer*, double > >::iterator loIt = labelOpacities.begin();
  for ( ; loIt != labelOpacities.end(); ++loIt )
  {
    ( *loIt ).first->setCustomProperty( "labeling/textTransp", ( *loIt ).second );
  }

  QList< QPair< QgsVectorLayer*, double > >::iterator lboIt = labelBufferOpacities.begin();
  for ( ; lboIt != labelBufferOpacities.end(); ++lboIt )
  {
    ( *lboIt ).first->setCustomProperty( "labeling/bufferTransp", ( *lboIt ).second );
  }
}

bool QgsWMSServer::checkMaximumWidthHeight() const
{
  //test if maxWidth / maxHeight set and WIDTH / HEIGHT parameter is in the range
  if ( mConfigParser->maxWidth() != -1 )
  {
    QMap<QString, QString>::const_iterator widthIt = mParameters.find( "WIDTH" );
    if ( widthIt != mParameters.constEnd() )
    {
      if ( widthIt->toInt() > mConfigParser->maxWidth() )
      {
        return false;
      }
    }
  }
  if ( mConfigParser->maxHeight() != -1 )
  {
    QMap<QString, QString>::const_iterator heightIt = mParameters.find( "HEIGHT" );
    if ( heightIt != mParameters.constEnd() )
    {
      if ( heightIt->toInt() > mConfigParser->maxHeight() )
      {
        return false;
      }
    }
  }
  return true;
}

QString QgsWMSServer::serviceUrl() const
{
  QString requestUri = getenv( "REQUEST_URI" );
  if ( requestUri.isEmpty() )
  {
    // in some cases (e.g. when running through python's CGIHTTPServer) the REQUEST_URI is not defined
    requestUri = QString( getenv( "SCRIPT_NAME" ) ) + "?" + QString( getenv( "QUERY_STRING" ) );
  }

  QUrl mapUrl( requestUri );
  mapUrl.setHost( getenv( "SERVER_NAME" ) );

  //Add non-default ports to url
  QString portString = getenv( "SERVER_PORT" );
  if ( !portString.isEmpty() )
  {
    bool portOk;
    int portNumber = portString.toInt( &portOk );
    if ( portOk )
    {
      if ( portNumber != 80 )
      {
        mapUrl.setPort( portNumber );
      }
    }
  }

  if ( QString( getenv( "HTTPS" ) ).compare( "on", Qt::CaseInsensitive ) == 0 )
  {
    mapUrl.setScheme( "https" );
  }
  else
  {
    mapUrl.setScheme( "http" );
  }

  QList<QPair<QString, QString> > queryItems = mapUrl.queryItems();
  QList<QPair<QString, QString> >::const_iterator queryIt = queryItems.constBegin();
  for ( ; queryIt != queryItems.constEnd(); ++queryIt )
  {
    if ( queryIt->first.compare( "REQUEST", Qt::CaseInsensitive ) == 0 )
    {
      mapUrl.removeQueryItem( queryIt->first );
    }
    else if ( queryIt->first.compare( "VERSION", Qt::CaseInsensitive ) == 0 )
    {
      mapUrl.removeQueryItem( queryIt->first );
    }
    else if ( queryIt->first.compare( "SERVICE", Qt::CaseInsensitive ) == 0 )
    {
      mapUrl.removeQueryItem( queryIt->first );
    }
    else if ( queryIt->first.compare( "LAYERS", Qt::CaseInsensitive ) == 0 )
    {
      mapUrl.removeQueryItem( queryIt->first );
    }
    else if ( queryIt->first.compare( "SLD_VERSION", Qt::CaseInsensitive ) == 0 )
    {
      mapUrl.removeQueryItem( queryIt->first );
    }
    else if ( queryIt->first.compare( "_DC", Qt::CaseInsensitive ) == 0 )
    {
      mapUrl.removeQueryItem( queryIt->first );
    }
  }
  return mapUrl.toString();
}

void QgsWMSServer::addXMLDeclaration( QDomDocument& doc ) const
{
  QDomProcessingInstruction xmlDeclaration = doc.createProcessingInstruction( "xml", "version=\"1.0\" encoding=\"utf-8\"" );
  doc.appendChild( xmlDeclaration );
}

void QgsWMSServer::convertFeatureInfoToSIA2045( QDomDocument& doc )
{
  QDomDocument SIAInfoDoc;
  QDomElement infoDocElement = doc.documentElement();
  QDomElement SIAInfoDocElement = SIAInfoDoc.importNode( infoDocElement, false ).toElement();
  SIAInfoDoc.appendChild( SIAInfoDocElement );

  QString currentAttributeName;
  QString currentAttributeValue;
  QDomElement currentAttributeElem;
  QString currentLayerName;
  QDomElement currentLayerElem;
  QDomNodeList layerNodeList = infoDocElement.elementsByTagName( "Layer" );
  for ( int i = 0; i < layerNodeList.size(); ++i )
  {
    currentLayerElem = layerNodeList.at( i ).toElement();
    currentLayerName = currentLayerElem.attribute( "name" );

    QDomElement currentFeatureElem;

    QDomNodeList featureList = currentLayerElem.elementsByTagName( "Feature" );
    if ( featureList.size() < 1 )
    {
      //raster?
      QDomNodeList attributeList = currentLayerElem.elementsByTagName( "Attribute" );
      QDomElement rasterLayerElem;
      if ( !attributeList.isEmpty() )
      {
        rasterLayerElem = SIAInfoDoc.createElement( currentLayerName );
      }
      for ( int j = 0; j < attributeList.size(); ++j )
      {
        currentAttributeElem = attributeList.at( j ).toElement();
        currentAttributeName = currentAttributeElem.attribute( "name" );
        currentAttributeValue = currentAttributeElem.attribute( "value" );
        QDomElement outAttributeElem = SIAInfoDoc.createElement( currentAttributeName );
        QDomText outAttributeText = SIAInfoDoc.createTextNode( currentAttributeValue );
        outAttributeElem.appendChild( outAttributeText );
        rasterLayerElem.appendChild( outAttributeElem );
      }
      if ( !attributeList.isEmpty() )
      {
        SIAInfoDocElement.appendChild( rasterLayerElem );
      }
    }
    else //vector
    {
      //property attributes
      QSet<QString> layerPropertyAttributes;
      QString currentLayerId = currentLayerElem.attribute( "id" );
      if ( !currentLayerId.isEmpty() )
      {
        QgsMapLayer* currentLayer = QgsMapLayerRegistry::instance()->mapLayer( currentLayerId );
        if ( currentLayer )
        {
          QString WMSPropertyAttributesString = currentLayer->customProperty( "WMSPropertyAttributes" ).toString();
          if ( !WMSPropertyAttributesString.isEmpty() )
          {
            QStringList propertyList = WMSPropertyAttributesString.split( "//" );
            QStringList::const_iterator propertyIt = propertyList.constBegin();
            for ( ; propertyIt != propertyList.constEnd(); ++propertyIt )
            {
              layerPropertyAttributes.insert( *propertyIt );
            }
          }
        }
      }

      QDomElement propertyRefChild; //child to insert the next property after (or
      for ( int j = 0; j < featureList.size(); ++j )
      {
        QDomElement SIAFeatureElem = SIAInfoDoc.createElement( currentLayerName );
        currentFeatureElem = featureList.at( j ).toElement();
        QDomNodeList attributeList = currentFeatureElem.elementsByTagName( "Attribute" );

        for ( int k = 0; k < attributeList.size(); ++k )
        {
          currentAttributeElem = attributeList.at( k ).toElement();
          currentAttributeName = currentAttributeElem.attribute( "name" );
          currentAttributeValue = currentAttributeElem.attribute( "value" );
          if ( layerPropertyAttributes.contains( currentAttributeName ) )
          {
            QDomElement propertyElem = SIAInfoDoc.createElement( "property" );
            QDomElement identifierElem = SIAInfoDoc.createElement( "identifier" );
            QDomText identifierText = SIAInfoDoc.createTextNode( currentAttributeName );
            identifierElem.appendChild( identifierText );
            QDomElement valueElem = SIAInfoDoc.createElement( "value" );
            QDomText valueText = SIAInfoDoc.createTextNode( currentAttributeValue );
            valueElem.appendChild( valueText );
            propertyElem.appendChild( identifierElem );
            propertyElem.appendChild( valueElem );
            if ( propertyRefChild.isNull() )
            {
              SIAFeatureElem.insertBefore( propertyElem, QDomNode() );
              propertyRefChild = propertyElem;
            }
            else
            {
              SIAFeatureElem.insertAfter( propertyElem, propertyRefChild );
            }
          }
          else
          {
            QDomElement SIAAttributeElem = SIAInfoDoc.createElement( currentAttributeName );
            QDomText SIAAttributeText = SIAInfoDoc.createTextNode( currentAttributeValue );
            SIAAttributeElem.appendChild( SIAAttributeText );
            SIAFeatureElem.appendChild( SIAAttributeElem );
          }
        }
        SIAInfoDocElement.appendChild( SIAFeatureElem );
      }
    }
  }
  doc = SIAInfoDoc;
}

QDomElement QgsWMSServer::createFeatureGML(
  QgsFeature* feat,
  QgsVectorLayer* layer,
  QDomDocument& doc,
  QgsCoordinateReferenceSystem& crs,
  const QString& typeName,
  bool withGeom,
  int version,
  QStringList* attributes ) const
{
  //qgs:%TYPENAME%
  QDomElement typeNameElement = doc.createElement( "qgs:" + typeName /*qgs:%TYPENAME%*/ );
  typeNameElement.setAttribute( "fid", typeName + "." + QString::number( feat->id() ) );

  const QgsCoordinateTransform* transform = nullptr;
  if ( layer && layer->crs() != crs )
  {
    transform = mMapRenderer->transformation( layer );
  }

  QgsGeometry* geom = feat->geometry();

  QgsExpressionContext expressionContext;
  expressionContext << QgsExpressionContextUtils::globalScope()
  << QgsExpressionContextUtils::projectScope();
  if ( layer )
    expressionContext << QgsExpressionContextUtils::layerScope( layer );
  expressionContext.setFeature( *feat );

  // always add bounding box info if feature contains geometry
  if ( geom && geom->type() != QGis::UnknownGeometry &&  geom->type() != QGis::NoGeometry )
  {
    QgsRectangle box = feat->constGeometry()->boundingBox();
    if ( transform )
    {
      try
      {
        QgsRectangle transformedBox = transform->transformBoundingBox( box );
        box = transformedBox;
      }
      catch ( QgsCsException &e )
      {
        QgsMessageLog::logMessage( QString( "Transform error caught: %1" ).arg( e.what() ) );
      }
    }

    QDomElement bbElem = doc.createElement( "gml:boundedBy" );
    QDomElement boxElem;
    if ( version < 3 )
    {
      boxElem = QgsOgcUtils::rectangleToGMLBox( &box, doc, 8 );
    }
    else
    {
      boxElem = QgsOgcUtils::rectangleToGMLEnvelope( &box, doc, 8 );
    }

    if ( crs.isValid() )
    {
      boxElem.setAttribute( "srsName", crs.authid() );
    }
    bbElem.appendChild( boxElem );
    typeNameElement.appendChild( bbElem );
  }

  if ( withGeom && geom )
  {
    //add geometry column (as gml)

    if ( transform )
    {
      geom->transform( *transform );
    }

    QDomElement geomElem = doc.createElement( "qgs:geometry" );
    QDomElement gmlElem;
    if ( version < 3 )
    {
      gmlElem = QgsOgcUtils::geometryToGML( geom, doc, 8 );
    }
    else
    {
      gmlElem = QgsOgcUtils::geometryToGML( geom, doc, "GML3", 8 );
    }

    if ( !gmlElem.isNull() )
    {
      if ( crs.isValid() )
      {
        gmlElem.setAttribute( "srsName", crs.authid() );
      }
      geomElem.appendChild( gmlElem );
      typeNameElement.appendChild( geomElem );
    }
  }

  //read all allowed attribute values from the feature
  QgsAttributes featureAttributes = feat->attributes();
  const QgsFields* fields = feat->fields();
  for ( int i = 0; i < fields->count(); ++i )
  {
    QString attributeName = fields->at( i ).name();
    //skip attribute if it is explicitly excluded from WMS publication
    if ( layer && layer->excludeAttributesWMS().contains( attributeName ) )
    {
      continue;
    }
    //skip attribute if it is excluded by access control
    if ( attributes && !attributes->contains( attributeName ) )
    {
      continue;
    }

    QDomElement fieldElem = doc.createElement( "qgs:" + attributeName.replace( QString( " " ), QString( "_" ) ) );
    QString fieldTextString = featureAttributes.at( i ).toString();
    if ( layer )
    {
      fieldTextString = replaceValueMapAndRelation( layer, i, QgsExpression::replaceExpressionText( fieldTextString, &expressionContext ) );
    }
    QDomText fieldText = doc.createTextNode( fieldTextString );
    fieldElem.appendChild( fieldText );
    typeNameElement.appendChild( fieldElem );
  }

  //add maptip attribute based on html/expression (in case there is no maptip attribute)
  if ( layer && layer->fieldNameIndex( layer->displayField() ) < 0 )
  {
    QString displayField = layer->displayField();
    if ( !displayField.isEmpty() )
    {
      QString fieldTextString = QgsExpression::replaceExpressionText( displayField, &expressionContext );
      QDomElement fieldElem = doc.createElement( "qgs:maptip" );
      QDomText maptipText = doc.createTextNode( fieldTextString );
      fieldElem.appendChild( maptipText );
      typeNameElement.appendChild( fieldElem );
    }
  }

  return typeNameElement;
}

QString QgsWMSServer::replaceValueMapAndRelation( QgsVectorLayer* vl, int idx, const QString& attributeVal )
{
  if ( QgsEditorWidgetFactory *factory = QgsEditorWidgetRegistry::instance()->factory( vl->editFormConfig()->widgetType( idx ) ) )
  {
    QgsEditorWidgetConfig cfg( vl->editFormConfig()->widgetConfig( idx ) );
    QString value( factory->representValue( vl, idx, cfg, QVariant(), attributeVal ) );
    if ( cfg.value( "AllowMulti" ).toBool() && value.startsWith( "{" ) && value.endsWith( "}" ) )
    {
      value = value.mid( 1, value.size() - 2 );
    }
    return value;
  }
  else
    return QString( "(%1)" ).arg( attributeVal );
}

int QgsWMSServer::getImageQuality() const
{

  // First taken from QGIS project
  int imageQuality = mConfigParser->imageQuality();

  // Then checks if a parameter is given, if so use it instead
  if ( mParameters.contains( "IMAGE_QUALITY" ) )
  {
    bool conversionSuccess;
    int imageQualityParameter;
    imageQualityParameter = mParameters[ "IMAGE_QUALITY" ].toInt( &conversionSuccess );
    if ( conversionSuccess )
    {
      imageQuality = imageQualityParameter;
    }
  }
  return imageQuality;
}

int QgsWMSServer::getWMSPrecision( int defaultValue = 8 ) const
{
  // First taken from QGIS project
  int WMSPrecision = mConfigParser->WMSPrecision();

  // Then checks if a parameter is given, if so use it instead
  if ( mParameters.contains( "WMS_PRECISION" ) )
  {
    bool conversionSuccess;
    int WMSPrecisionParameter;
    WMSPrecisionParameter = mParameters[ "WMS_PRECISION" ].toInt( &conversionSuccess );
    if ( conversionSuccess )
    {
      WMSPrecision = WMSPrecisionParameter;
    }
  }
  if ( WMSPrecision == -1 )
  {
    WMSPrecision = defaultValue;
  }
  return WMSPrecision;
}

QgsRectangle QgsWMSServer::featureInfoSearchRect( QgsVectorLayer* ml, QgsMapRenderer* mr, const QgsRenderContext& rct, const QgsPoint& infoPoint ) const
{
  if ( !ml || !mr )
  {
    return QgsRectangle();
  }

  double mapUnitTolerance = 0.0;
  if ( ml->geometryType() == QGis::Polygon )
  {
    QMap<QString, QString>::const_iterator tolIt = mParameters.find( "FI_POLYGON_TOLERANCE" );
    if ( tolIt != mParameters.constEnd() )
    {
      mapUnitTolerance = tolIt.value().toInt() * rct.mapToPixel().mapUnitsPerPixel();
    }
    else
    {
      mapUnitTolerance = mr->extent().width() / 400.0;
    }
  }
  else if ( ml->geometryType() == QGis::Line )
  {
    QMap<QString, QString>::const_iterator tolIt = mParameters.find( "FI_LINE_TOLERANCE" );
    if ( tolIt != mParameters.constEnd() )
    {
      mapUnitTolerance = tolIt.value().toInt() * rct.mapToPixel().mapUnitsPerPixel();
    }
    else
    {
      mapUnitTolerance = mr->extent().width() / 200.0;
    }
  }
  else //points
  {
    QMap<QString, QString>::const_iterator tolIt = mParameters.find( "FI_POINT_TOLERANCE" );
    if ( tolIt != mParameters.constEnd() )
    {
      mapUnitTolerance = tolIt.value().toInt() * rct.mapToPixel().mapUnitsPerPixel();
    }
    else
    {
      mapUnitTolerance = mr->extent().width() / 100.0;
    }
  }

  QgsRectangle mapRectangle( infoPoint.x() - mapUnitTolerance, infoPoint.y() - mapUnitTolerance,
                             infoPoint.x() + mapUnitTolerance, infoPoint.y() + mapUnitTolerance );
  return( mr->mapToLayerCoordinates( ml, mapRectangle ) );
}

void QgsWMSServer::readFormatOptions( QMap<QString, QString>& formatOptions ) const
{
  formatOptions.clear();
  QString fo = mParameters.value( "FORMAT_OPTIONS" );
  QStringList formatOptionsList = fo.split( ";" );
  QStringList::const_iterator optionsIt = formatOptionsList.constBegin();
  for ( ; optionsIt != formatOptionsList.constEnd(); ++optionsIt )
  {
    int equalIdx = optionsIt->indexOf( ":" );
    if ( equalIdx > 0 && equalIdx < ( optionsIt->length() - 1 ) )
    {
      formatOptions.insert( optionsIt->left( equalIdx ).toUpper(), optionsIt->right( optionsIt->length() - equalIdx - 1 ).toUpper() );
    }
  }
}

void QgsWMSServer::readDxfLayerSettings( QList< QPair<QgsVectorLayer *, int > >& layers, const QMap<QString, QString>& formatOptionsMap ) const
{
  layers.clear();

  QSet<QString> wfsLayers = QSet<QString>::fromList( mConfigParser->wfsLayerNames() );

  QStringList layerAttributes;
  QMap<QString, QString>::const_iterator layerAttributesIt = formatOptionsMap.find( "LAYERATTRIBUTES" );
  if ( layerAttributesIt != formatOptionsMap.constEnd() )
  {
    layerAttributes = formatOptionsMap.value( "LAYERATTRIBUTES" ).split( "," );
  }

  //LAYERS and STYLES
  QStringList layerList, styleList;
  if ( readLayersAndStyles( layerList, styleList ) != 0 )
  {
    return;
  }

  for ( int i = 0; i < layerList.size(); ++i )
  {
    QString layerName = layerList.at( i );
    QString styleName;
    if ( styleList.size() > i )
    {
      styleName = styleList.at( i );
    }

    QList<QgsMapLayer*> layerList = mConfigParser->mapLayerFromStyle( layerName, styleName );
    QList<QgsMapLayer*>::const_iterator layerIt = layerList.constBegin();
    for ( ; layerIt != layerList.constEnd(); ++layerIt )
    {
      if ( !( *layerIt ) )
      {
        continue;
      }

      //vector layer?
      if (( *layerIt )->type() != QgsMapLayer::VectorLayer )
      {
        continue;
      }

      QgsVectorLayer* vlayer = static_cast<QgsVectorLayer*>( *layerIt );

      int layerAttribute = -1;
      if ( layerAttributes.size() > i )
      {
        layerAttribute = vlayer->pendingFields().indexFromName( layerAttributes.at( i ) );
      }

      //only wfs layers are allowed to be published
      if ( !wfsLayers.contains( vlayer->name() ) )
      {
        continue;
      }

      layers.append( qMakePair( vlayer, layerAttribute ) );
    }
  }
}
