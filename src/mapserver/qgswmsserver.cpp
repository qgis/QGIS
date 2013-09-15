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
#include "qgsconfigparser.h"
#include "qgscrscache.h"
#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgsmaplayer.h"
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
#include "qgsmapserviceexception.h"
#include "qgssldparser.h"
#include "qgssymbolv2.h"
#include "qgsrendererv2.h"
#include "qgslegendmodel.h"
#include "qgscomposerlegenditem.h"
#include "qgspaintenginehack.h"
#include "qgsogcutils.h"
#include "qgsfeature.h"

#include <QImage>
#include <QPainter>
#include <QStringList>
#include <QTextStream>
#include <QDir>

//for printing
#include "qgscomposition.h"
#include <QBuffer>
#include <QPrinter>
#include <QSvgGenerator>
#include <QUrl>
#include <QPaintEngine>

QgsWMSServer::QgsWMSServer( QMap<QString, QString> parameters, QgsMapRenderer* renderer )
    : mParameterMap( parameters )
    , mConfigParser( 0 )
    , mMapRenderer( renderer )
{
}

QgsWMSServer::~QgsWMSServer()
{
}

QgsWMSServer::QgsWMSServer()
{
}

void QgsWMSServer::appendFormats( QDomDocument &doc, QDomElement &elem, const QStringList &formats )
{
  foreach ( QString format, formats )
  {
    QDomElement formatElem = doc.createElement( "Format"/*wms:Format*/ );
    formatElem.appendChild( doc.createTextNode( format ) );
    elem.appendChild( formatElem );
  }
}

QDomDocument QgsWMSServer::getCapabilities( QString version, bool fullProjectInformation )
{
  QgsDebugMsg( "Entering." );
  QDomDocument doc;
  QDomElement wmsCapabilitiesElement;

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
    wmsCapabilitiesElement.setAttribute( "xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance" );
    wmsCapabilitiesElement.setAttribute( "xsi:schemaLocation", "http://www.opengis.net/wms http://schemas.opengis.net/wms/1.3.0/capabilities_1_3_0.xsd" );
  }
  wmsCapabilitiesElement.setAttribute( "version", version );
  doc.appendChild( wmsCapabilitiesElement );

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

  //Prepare url
  QString hrefString = mConfigParser->serviceUrl();
  if ( hrefString.isEmpty() )
  {
    hrefString = serviceUrl();
  }


  // SOAP platform
  //only give this information if it is not a WMS request to be in sync with the WMS capabilities schema
  QMap<QString, QString>::const_iterator service_it = mParameterMap.find( "SERVICE" );
  if ( service_it != mParameterMap.end() && service_it.value().compare( "WMS", Qt::CaseInsensitive ) != 0 )
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
  appendFormats( doc, elem, QStringList() << "image/jpeg" << "image/png" << "image/png; mode=16bit" << "image/png; mode=8bit" << "image/png; mode=1bit" );
  elem.appendChild( dcpTypeElement.cloneNode().toElement() ); //this is the same as for 'GetCapabilities'
  requestElement.appendChild( elem );

  //wms:GetFeatureInfo
  elem = doc.createElement( "GetFeatureInfo" );
  appendFormats( doc, elem, QStringList() << "text/plain" << "text/html" << "text/xml" << "application/vnd.ogc.gml" << "application/vnd.ogc.gml/3.1.1");
  elem.appendChild( dcpTypeElement.cloneNode().toElement() ); //this is the same as for 'GetCapabilities'
  requestElement.appendChild( elem );

  //wms:GetLegendGraphic
  elem = doc.createElement( "GetLegendGraphic"/*wms:GetLegendGraphic*/ );
  appendFormats( doc, elem, QStringList() << "image/jpeg" << "image/png" );
  elem.appendChild( dcpTypeElement.cloneNode().toElement() ); // this is the same as for 'GetCapabilities'
  requestElement.appendChild( elem );

  //wms:GetStyles
  elem = doc.createElement( "GetStyles"/*wms:GetStyles*/ );
  appendFormats( doc, elem, QStringList() << "text/xml" );
  elem.appendChild( dcpTypeElement.cloneNode().toElement() ); //this is the same as for 'GetCapabilities'
  requestElement.appendChild( elem );

  //wms:GetPrint
  elem = doc.createElement( "GetPrint"/*wms:GetPrint*/ );
  appendFormats( doc, elem, QStringList() << "svg" << "png" << "pdf" );
  elem.appendChild( dcpTypeElement.cloneNode().toElement() ); //this is the same as for 'GetCapabilities'
  requestElement.appendChild( elem );

  //Exception element is mandatory
  elem = doc.createElement( "Exception" );
  appendFormats( doc, elem, QStringList() << ( version == "1.1.1" ? "application/vnd.ogc.se_xml" : "text/xml" ) );
  capabilityElement.appendChild( elem );

  if ( mConfigParser /*&& fullProjectInformation*/ ) //remove composer templates from GetCapabilities in the long term
  {
    //Insert <ComposerTemplate> elements derived from wms:_ExtendedCapabilities
    mConfigParser->printCapabilities( capabilityElement, doc );
  }

  if ( mConfigParser && fullProjectInformation )
  {
    //WFS layers
    QStringList wfsLayers = mConfigParser->wfsLayerNames();
    if ( wfsLayers.size() > 0 )
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
  QgsDebugMsg( "calling layersAndStylesCapabilities" );
  if ( mConfigParser )
  {
    mConfigParser->layersAndStylesCapabilities( capabilityElement, doc, version, fullProjectInformation );
  }
  QgsDebugMsg( "layersAndStylesCapabilities returned" );

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

QImage* QgsWMSServer::getLegendGraphics()
{
  if ( !mConfigParser || !mMapRenderer )
  {
    return 0;
  }
  if ( !mParameterMap.contains( "LAYER" ) && !mParameterMap.contains( "LAYERS" ) )
  {
    throw QgsMapServiceException( "LayerNotSpecified", "LAYER is mandatory for GetLegendGraphic operation" );
  }
  if ( !mParameterMap.contains( "FORMAT" ) )
  {
    throw QgsMapServiceException( "FormatNotSpecified", "FORMAT is mandatory for GetLegendGraphic operation" );
  }

  QStringList layersList, stylesList;

  if ( readLayersAndStyles( layersList, stylesList ) != 0 )
  {
    QgsDebugMsg( "error reading layers and styles" );
    return 0;
  }

  if ( layersList.size() < 1 )
  {
    return 0;
  }

  //scale
  double scaleDenominator = -1;
  QMap<QString, QString>::const_iterator scaleIt = mParameterMap.find( "SCALE" );
  if ( scaleIt != mParameterMap.constEnd() )
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
    return 0;
  }

  QgsLegendModel legendModel;
  legendModel.setLayerSet( layerIds, scaleDenominator );

  //create first image (to find out dpi)
  QImage* theImage = createImage( 10, 10 );
  if ( !theImage )
  {
    return 0;
  }
  double mmToPixelFactor = theImage->dotsPerMeterX() / 1000.0;
  double maxTextWidth = 0;
  double maxSymbolWidth = 0;
  double fontOversamplingFactor = 10.0;

  //get icon size, spaces between legend items and font from config parser
  double boxSpace, layerSpace, layerTitleSpace, symbolSpace, iconLabelSpace, symbolWidth, symbolHeight;
  QFont layerFont, itemFont;
  QColor layerFontColor, itemFontColor;
  legendParameters( mmToPixelFactor, fontOversamplingFactor, boxSpace, layerSpace, layerTitleSpace, symbolSpace,
                    iconLabelSpace, symbolWidth, symbolHeight, layerFont, itemFont, layerFontColor, itemFontColor );

  //first find out image dimensions without painting
  QStandardItem* rootItem = legendModel.invisibleRootItem();
  if ( !rootItem )
  {
    return 0;
  }

  double currentY = drawLegendGraphics( 0, fontOversamplingFactor, rootItem, boxSpace, layerSpace, layerTitleSpace, symbolSpace,
                                        iconLabelSpace, symbolWidth, symbolHeight, layerFont, itemFont, layerFontColor, itemFontColor, maxTextWidth,
                                        maxSymbolWidth, theImage->dotsPerMeterX() * 0.0254 );

  //create second image with the right dimensions
  QImage* paintImage = createImage( maxTextWidth + maxSymbolWidth, ceil( currentY ) );

  //go through the items a second time for painting
  QPainter p( paintImage );
  p.setRenderHint( QPainter::Antialiasing, true );

  drawLegendGraphics( &p, fontOversamplingFactor, rootItem, boxSpace, layerSpace, layerTitleSpace, symbolSpace,
                      iconLabelSpace, symbolWidth, symbolHeight, layerFont, itemFont, layerFontColor, itemFontColor, maxTextWidth,
                      maxSymbolWidth, theImage->dotsPerMeterX() * 0.0254 );

  QgsMapLayerRegistry::instance()->removeAllMapLayers();
  delete theImage;
  return paintImage;
}

double QgsWMSServer::drawLegendGraphics( QPainter* p, double fontOversamplingFactor, QStandardItem* rootItem, double boxSpace,
    double layerSpace, double layerTitleSpace, double symbolSpace, double iconLabelSpace,
    double symbolWidth, double symbolHeight, const QFont& layerFont, const QFont& itemFont,
    const QColor& layerFontColor, const QColor& itemFontColor, double& maxTextWidth, double& maxSymbolWidth, double dpi )
{
  if ( !rootItem )
  {
    return 0;
  }

  int numLayerItems = rootItem->rowCount();
  if ( numLayerItems < 1 )
  {
    return 0;
  }

  double currentY = boxSpace;
  for ( int i = 0; i < numLayerItems; ++i )
  {
    QgsComposerLayerItem* layerItem = dynamic_cast<QgsComposerLayerItem*>( rootItem->child( i ) );
    if ( layerItem )
    {
      if ( i > 0 )
      {
        currentY += layerSpace;
      }
      drawLegendLayerItem( layerItem, p, maxTextWidth, maxSymbolWidth, currentY, layerFont, layerFontColor, itemFont, itemFontColor,
                           boxSpace, layerSpace, layerTitleSpace, symbolSpace, iconLabelSpace, symbolWidth, symbolHeight, fontOversamplingFactor,
                           dpi );
    }
  }
  currentY += boxSpace;
  return currentY;
}

void QgsWMSServer::legendParameters( double mmToPixelFactor, double fontOversamplingFactor, double& boxSpace, double& layerSpace, double& layerTitleSpace,
                                     double& symbolSpace, double& iconLabelSpace, double& symbolWidth, double& symbolHeight,
                                     QFont& layerFont, QFont& itemFont, QColor& layerFontColor, QColor& itemFontColor )
{
  //spaces between legend elements
  QMap<QString, QString>::const_iterator boxSpaceIt = mParameterMap.find( "BOXSPACE" );
  boxSpace = ( boxSpaceIt == mParameterMap.constEnd() ) ? mConfigParser->legendBoxSpace() * mmToPixelFactor :
             boxSpaceIt.value().toDouble() * mmToPixelFactor;
  QMap<QString, QString>::const_iterator layerSpaceIt = mParameterMap.find( "LAYERSPACE" );
  layerSpace = ( layerSpaceIt == mParameterMap.constEnd() ) ? mConfigParser->legendLayerSpace() * mmToPixelFactor :
               layerSpaceIt.value().toDouble() * mmToPixelFactor;
  QMap<QString, QString>::const_iterator layerTitleSpaceIt = mParameterMap.find( "LAYERTITLESPACE" );
  layerTitleSpace = ( layerTitleSpaceIt == mParameterMap.constEnd() ) ? mConfigParser->legendLayerTitleSpace() * mmToPixelFactor :
                    layerTitleSpaceIt.value().toDouble() * mmToPixelFactor;
  QMap<QString, QString>::const_iterator symbolSpaceIt = mParameterMap.find( "SYMBOLSPACE" );
  symbolSpace = ( symbolSpaceIt == mParameterMap.constEnd() ) ? mConfigParser->legendSymbolSpace() * mmToPixelFactor :
                symbolSpaceIt.value().toDouble() * mmToPixelFactor;
  QMap<QString, QString>::const_iterator iconLabelSpaceIt = mParameterMap.find( "ICONLABELSPACE" );
  iconLabelSpace = ( iconLabelSpaceIt == mParameterMap.constEnd() ) ? mConfigParser->legendIconLabelSpace() * mmToPixelFactor :
                   iconLabelSpaceIt.value().toDouble() * mmToPixelFactor;
  QMap<QString, QString>::const_iterator symbolWidthIt = mParameterMap.find( "SYMBOLWIDTH" );
  symbolWidth = ( symbolWidthIt == mParameterMap.constEnd() ) ? mConfigParser->legendSymbolWidth() * mmToPixelFactor :
                symbolWidthIt.value().toDouble() * mmToPixelFactor;
  QMap<QString, QString>::const_iterator symbolHeightIt = mParameterMap.find( "SYMBOLHEIGHT" );
  symbolHeight = ( symbolHeightIt == mParameterMap.constEnd() ) ? mConfigParser->legendSymbolHeight() * mmToPixelFactor :
                 symbolHeightIt.value().toDouble() * mmToPixelFactor;

  //font properties
  layerFont = mConfigParser->legendLayerFont();
  QMap<QString, QString>::const_iterator layerFontFamilyIt = mParameterMap.find( "LAYERFONTFAMILY" );
  if ( layerFontFamilyIt != mParameterMap.constEnd() )
  {
    layerFont.setFamily( layerFontFamilyIt.value() );
  }
  QMap<QString, QString>::const_iterator layerFontBoldIt = mParameterMap.find( "LAYERFONTBOLD" );
  if ( layerFontBoldIt != mParameterMap.constEnd() )
  {
    layerFont.setBold( layerFontBoldIt.value().compare( "TRUE" , Qt::CaseInsensitive ) == 0 );
  }
  QMap<QString, QString>::const_iterator layerFontItalicIt = mParameterMap.find( "LAYERFONTITALIC" );
  if ( layerFontItalicIt != mParameterMap.constEnd() )
  {
    layerFont.setItalic( layerFontItalicIt.value().compare( "TRUE", Qt::CaseInsensitive ) == 0 );
  }
  QMap<QString, QString>::const_iterator layerFontSizeIt = mParameterMap.find( "LAYERFONTSIZE" );
  if ( layerFontSizeIt != mParameterMap.constEnd() )
  {
    layerFont.setPixelSize( layerFontSizeIt.value().toDouble() * 0.3528 * mmToPixelFactor * fontOversamplingFactor );
  }
  else
  {
    layerFont.setPixelSize( layerFont.pointSizeF() * 0.3528 * mmToPixelFactor * fontOversamplingFactor );
  }
  QMap<QString, QString>::const_iterator layerFontColorIt = mParameterMap.find( "LAYERFONTCOLOR" );
  if ( layerFontColorIt != mParameterMap.constEnd() )
  {
    layerFontColor.setNamedColor( layerFontColorIt.value() );
  }
  else
  {
    layerFontColor = QColor( 0, 0, 0 );
  }


  itemFont = mConfigParser->legendItemFont();
  QMap<QString, QString>::const_iterator itemFontFamilyIt = mParameterMap.find( "ITEMFONTFAMILY" );
  if ( itemFontFamilyIt != mParameterMap.constEnd() )
  {
    itemFont.setFamily( itemFontFamilyIt.value() );
  }
  QMap<QString, QString>::const_iterator itemFontBoldIt = mParameterMap.find( "ITEMFONTBOLD" );
  if ( itemFontBoldIt != mParameterMap.constEnd() )
  {
    itemFont.setBold( itemFontBoldIt.value().compare( "TRUE" , Qt::CaseInsensitive ) == 0 );
  }
  QMap<QString, QString>::const_iterator itemFontItalicIt = mParameterMap.find( "ITEMFONTITALIC" );
  if ( itemFontItalicIt != mParameterMap.constEnd() )
  {
    itemFont.setItalic( itemFontItalicIt.value().compare( "TRUE", Qt::CaseInsensitive ) == 0 );
  }
  QMap<QString, QString>::const_iterator itemFontSizeIt = mParameterMap.find( "ITEMFONTSIZE" );
  if ( itemFontSizeIt != mParameterMap.constEnd() )
  {
    itemFont.setPixelSize( itemFontSizeIt.value().toDouble() * 0.3528 * mmToPixelFactor * fontOversamplingFactor );
  }
  else
  {
    itemFont.setPixelSize( itemFont.pointSizeF() * 0.3528 * mmToPixelFactor * fontOversamplingFactor );
  }
  QMap<QString, QString>::const_iterator itemFontColorIt = mParameterMap.find( "ITEMFONTCOLOR" );
  if ( itemFontColorIt != mParameterMap.constEnd() )
  {
    itemFontColor.setNamedColor( itemFontColorIt.value() );
  }
  else
  {
    itemFontColor = QColor( 0, 0, 0 );
  }
}

QDomDocument QgsWMSServer::getStyle()
{
  QDomDocument doc;
  if ( !mParameterMap.contains( "STYLE" ) )
  {
    throw QgsMapServiceException( "StyleNotSpecified", "Style is mandatory for GetStyle operation" );
  }

  if ( !mParameterMap.contains( "LAYER" ) )
  {
    throw QgsMapServiceException( "LayerNotSpecified", "Layer is mandatory for GetStyle operation" );
  }

  QString styleName = mParameterMap[ "STYLE" ];
  QString layerName = mParameterMap[ "LAYER" ];

  return mConfigParser->getStyle( styleName, layerName );
}

QByteArray* QgsWMSServer::getPrint( const QString& formatString )
{
  QStringList layersList, stylesList, layerIdList;
  QString dummyFormat;
  QImage* theImage = initializeRendering( layersList, stylesList, layerIdList );
  if ( !theImage )
  {
    return 0;
  }
  delete theImage;

  QMap<QString, QString> originalLayerFilters = applyRequestedLayerFilters( layersList );
  QStringList selectedLayerIdList = applyFeatureSelections( layersList );

  //GetPrint request needs a template parameter
  if ( !mParameterMap.contains( "TEMPLATE" ) )
  {
    throw QgsMapServiceException( "ParameterMissing", "The TEMPLATE parameter is required for the GetPrint request" );
  }

  QList< QPair< QgsVectorLayer*, QgsFeatureRendererV2*> > bkVectorRenderers;
  QList< QPair< QgsRasterLayer*, QgsRasterRenderer* > > bkRasterRenderers;
  QList< QPair< QgsVectorLayer*, double > > labelTransparencies;
  QList< QPair< QgsVectorLayer*, double > > labelBufferTransparencies;

  applyOpacities( layersList, bkVectorRenderers, bkRasterRenderers, labelTransparencies, labelBufferTransparencies );


  QgsComposition* c = mConfigParser->createPrintComposition( mParameterMap[ "TEMPLATE" ], mMapRenderer, QMap<QString, QString>( mParameterMap ) );
  if ( !c )
  {
    restoreLayerFilters( originalLayerFilters );
    clearFeatureSelections( selectedLayerIdList );
    return 0;
  }

  QByteArray* ba = 0;
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
      restoreLayerFilters( originalLayerFilters );
      clearFeatureSelections( selectedLayerIdList );
      return 0;
    }

    c->exportAsPDF( tempFile.fileName() );
    ba = new QByteArray();
    *ba = tempFile.readAll();
  }
  else //unknown format
  {
    throw QgsMapServiceException( "InvalidFormat", "Output format '" + formatString + "' is not supported in the GetPrint request" );
  }

  restoreOpacities( bkVectorRenderers, bkRasterRenderers, labelTransparencies, labelBufferTransparencies );
  restoreLayerFilters( originalLayerFilters );
  clearFeatureSelections( selectedLayerIdList );

  delete c;
  return ba;
}

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

QImage* QgsWMSServer::getMap()
{
  if ( !checkMaximumWidthHeight() )
  {
    throw QgsMapServiceException( "Size error", "The requested map size is too large" );
  }
  QStringList layersList, stylesList, layerIdList;
  QImage* theImage = initializeRendering( layersList, stylesList, layerIdList );

  QPainter thePainter( theImage );
  thePainter.setRenderHint( QPainter::Antialiasing ); //make it look nicer

  QMap<QString, QString> originalLayerFilters = applyRequestedLayerFilters( layersList );
  QStringList selectedLayerIdList = applyFeatureSelections( layersList );

  QList< QPair< QgsVectorLayer*, QgsFeatureRendererV2*> > bkVectorRenderers;
  QList< QPair< QgsRasterLayer*, QgsRasterRenderer* > > bkRasterRenderers;
  QList< QPair< QgsVectorLayer*, double > > labelTransparencies;
  QList< QPair< QgsVectorLayer*, double > > labelBufferTransparencies;

  applyOpacities( layersList, bkVectorRenderers, bkRasterRenderers, labelTransparencies, labelBufferTransparencies );

  mMapRenderer->render( &thePainter );
  if ( mConfigParser )
  {
    //draw configuration format specific overlay items
    mConfigParser->drawOverlays( &thePainter, theImage->dotsPerMeterX() / 1000.0 * 25.4, theImage->width(), theImage->height() );
  }

  restoreOpacities( bkVectorRenderers, bkRasterRenderers, labelTransparencies, labelBufferTransparencies );
  restoreLayerFilters( originalLayerFilters );
  clearFeatureSelections( selectedLayerIdList );

  QgsDebugMsg( "clearing filters" );
  QgsMapLayerRegistry::instance()->removeAllMapLayers();

#ifdef QGISDEBUG
  theImage->save( QDir::tempPath() + QDir::separator() + "lastrender.png" );
#endif
  return theImage;
}

int QgsWMSServer::getFeatureInfo( QDomDocument& result, QString version )
{
  if ( !mMapRenderer || !mConfigParser )
  {
    return 1;
  }

  result.clear();
  QStringList layersList, stylesList;
  bool conversionSuccess;

  for ( QMap<QString, QString>::iterator it = mParameterMap.begin(); it != mParameterMap.end(); ++it )
  {
    QgsDebugMsg( QString( "%1  // %2" ).arg( it.key() ).arg( it.value() ) );
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

  QgsDebugMsg( "mMapRenderer->extent(): " +  mMapRenderer->extent().toString() );
  QgsDebugMsg( QString( "mMapRenderer width = %1 height = %2" ).arg( mMapRenderer->outputSize().width() ).arg( mMapRenderer->outputSize().height() ) );
  QgsDebugMsg( QString( "mMapRenderer->mapUnitsPerPixel() = %1" ).arg( mMapRenderer->mapUnitsPerPixel() ) );

  //find out the current scale denominater and set it to the SLD parser
  QgsScaleCalculator scaleCalc(( outputImage->logicalDpiX() + outputImage->logicalDpiY() ) / 2 , mMapRenderer->destinationCrs().mapUnits() );
  QgsRectangle mapExtent = mMapRenderer->extent();
  double scaleDenominator = scaleCalc.calculate( mapExtent, outputImage->width() );
  mConfigParser->setScaleDenominator( scaleDenominator );
  delete outputImage; //no longer needed for feature info

  //read FEATURE_COUNT
  int featureCount = 1;
  if ( mParameterMap.contains( "FEATURE_COUNT" ) )
  {
    featureCount = mParameterMap[ "FEATURE_COUNT" ].toInt( &conversionSuccess );
    if ( !conversionSuccess )
    {
      featureCount = 1;
    }
  }

  //read QUERY_LAYERS
  if ( !mParameterMap.contains( "QUERY_LAYERS" ) )
  {
    return 3;
  }

  QStringList queryLayerList = mParameterMap[ "QUERY_LAYERS" ].split( ",", QString::SkipEmptyParts );
  if ( queryLayerList.size() < 1 )
  {
    return 4;
  }

  //read I,J resp. X,Y
  QString iString, jString;
  int i = -1;
  int j = -1;

  iString = mParameterMap.value( "I", mParameterMap.value( "X" ) );
  i = iString.toInt( &conversionSuccess );
  if ( !conversionSuccess )
  {
    i = -1;
  }

  jString = mParameterMap.value( "J", mParameterMap.value( "Y" ) );
  j = jString.toInt( &conversionSuccess );
  if ( !conversionSuccess )
  {
    j = -1;
  }

  //Normally, I/J or X/Y are mandatory parameters.
  //However, in order to make attribute only queries via the FILTER parameter, it is allowed to skip them if the FILTER parameter is there

  QgsRectangle* featuresRect = 0;
  QgsPoint* infoPoint = 0;
  if ( i == -1 || j == -1 )
  {
    if ( mParameterMap.contains( "FILTER" ) )
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
    infoPoint = new QgsPoint();
  }

  //get the layer registered in QgsMapLayerRegistry and apply possible filters
  QStringList layerIds = layerSet( layersList, stylesList, mMapRenderer->destinationCrs() );
  QMap<QString, QString> originalLayerFilters = applyRequestedLayerFilters( layersList );

  QDomElement getFeatureInfoElement;
  QString infoFormat = mParameterMap.value("INFO_FORMAT");
  if (infoFormat.startsWith("application/vnd.ogc.gml")) {
    getFeatureInfoElement = result.createElement("wfs:FeatureCollection");
    getFeatureInfoElement.setAttribute("xmlns:wfs", "http://www.opengis.net/wfs");
    getFeatureInfoElement.setAttribute("xmlns:ogc", "http://www.opengis.net/ogc");
    getFeatureInfoElement.setAttribute("xmlns:gml", "http://www.opengis.net/gml");
    getFeatureInfoElement.setAttribute("xmlns:ows", "http://www.opengis.net/ows");
    getFeatureInfoElement.setAttribute("xmlns:xlink", "http://www.w3.org/1999/xlink");
    getFeatureInfoElement.setAttribute("xmlns:qgs", "http://www.qgis.org/gml");
    getFeatureInfoElement.setAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    getFeatureInfoElement.setAttribute("xsi:schemaLocation", "http://www.opengis.net/wfs http://schemas.opengis.net/wfs/1.0.0/wfs.xsd http://www.qgis.org/gml");
  }
  else {
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
    renderContext.setPainter( 0 );
  }

  bool sia2045 = mConfigParser->featureInfoFormatSIA2045();

  //layers can have assigned a different name for GetCapabilities
  QHash<QString, QString> layerAliasMap = mConfigParser->featureInfoLayerAliasMap();

  QList<QgsMapLayer*> layerList;
  QgsMapLayer* currentLayer = 0;
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

      //skip layer if not visible at current map scale
      if ( currentLayer->hasScaleBasedVisibility() && ( currentLayer->minimumScale() > scaleDenominator || currentLayer->maximumScale() < scaleDenominator ) )
      {
        continue;
      }

      if ( infoPoint && infoPointToLayerCoordinates( i, j, infoPoint, mMapRenderer, currentLayer ) != 0 )
      {
        continue;
      }

      //switch depending on vector or raster
      QgsVectorLayer* vectorLayer = dynamic_cast<QgsVectorLayer*>( currentLayer );

      QDomElement layerElement;
      if (infoFormat.startsWith("application/vnd.ogc.gml")) {
        layerElement = getFeatureInfoElement;
      }
      else {
        layerElement = result.createElement( "Layer" );
        QString layerName = currentLayer->name();

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
        if ( vectorLayer->vectorJoins().size() > 0 )
        {
          QList<QgsMapLayer *> joinLayers;
          //JoinBuffer is based on qgsmaplayerregistry!!!!!
          //insert existing join info
          const QList< QgsVectorJoinInfo >& joins = vectorLayer->vectorJoins();
          for ( int i = 0; i < joins.size(); ++i )
          {
            QgsMapLayer* joinLayer = mConfigParser->mapLayerFromLayerId( joins[i].joinLayerId );
            if ( joinLayer )
            {
              joinLayers << joinLayer;
            }
            QgsMapLayerRegistry::instance()->addMapLayers( joinLayers, false, true );
          }
          vectorLayer->updateFields();
        }
        if ( featureInfoFromVectorLayer( vectorLayer, infoPoint, featureCount, result, layerElement, mMapRenderer, renderContext,
                                         version, infoFormat, featuresRect ) != 0 )
        {
          continue;
        }
      }
      else //raster layer
      {
        if (infoFormat.startsWith("application/vnd.ogc.gml")) {
          layerElement = result.createElement( "gml:featureMember"/*wfs:FeatureMember*/ );
          getFeatureInfoElement.appendChild( layerElement );
        }

        QgsRasterLayer* rasterLayer = dynamic_cast<QgsRasterLayer*>( currentLayer );
        if ( rasterLayer )
        {
          if ( featureInfoFromRasterLayer( rasterLayer, infoPoint, result, layerElement, version, infoFormat ) != 0 )
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
    QDomElement bBoxElem = result.createElement( "BoundingBox" );
    bBoxElem.setAttribute( "CRS", mMapRenderer->destinationCrs().authid() );
    bBoxElem.setAttribute( "minx", QString::number( featuresRect->xMinimum() ) );
    bBoxElem.setAttribute( "maxx", QString::number( featuresRect->xMaximum() ) );
    bBoxElem.setAttribute( "miny", QString::number( featuresRect->yMinimum() ) );
    bBoxElem.setAttribute( "maxy", QString::number( featuresRect->yMaximum() ) );
    getFeatureInfoElement.insertBefore( bBoxElem, QDomNode() ); //insert as first child
  }

  if ( sia2045 && infoFormat.compare( "text/xml", Qt::CaseInsensitive ) == 0 )
  {
    convertFeatureInfoToSIA2045( result );
  }

  restoreLayerFilters( originalLayerFilters );
  QgsMapLayerRegistry::instance()->removeAllMapLayers();
  delete featuresRect;
  delete infoPoint;
  return 0;
}

QImage* QgsWMSServer::initializeRendering( QStringList& layersList, QStringList& stylesList, QStringList& layerIdList )
{
  if ( !mConfigParser )
  {
    QgsDebugMsg( "Error: mSLDParser is 0" );
    return 0;
  }

  if ( !mMapRenderer )
  {
    QgsDebugMsg( "Error: mMapRenderer is 0" );
    return 0;
  }

  if ( readLayersAndStyles( layersList, stylesList ) != 0 )
  {
    QgsDebugMsg( "error reading layers and styles" );
    return 0;
  }

  if ( initializeSLDParser( layersList, stylesList ) != 0 )
  {
    return 0;
  }
  //pass external GML to the SLD parser.
  QString gml = mParameterMap.value( "GML" );
  if ( !gml.isEmpty() )
  {
    QDomDocument* gmlDoc = new QDomDocument();
    if ( gmlDoc->setContent( gml, true ) )
    {
      QString layerName = gmlDoc->documentElement().attribute( "layerName" );
      QgsDebugMsg( "Adding entry with key: " + layerName + " to external GML data" );
      mConfigParser->addExternalGMLData( layerName, gmlDoc );
    }
    else
    {
      QgsDebugMsg( "Error, could not add external GML to QgsSLDParser" );
      delete gmlDoc;
    }
  }

  QImage* theImage = createImage();
  if ( !theImage )
  {
    return 0;
  }

  if ( configureMapRender( theImage ) != 0 )
  {
    delete theImage;
    return 0;
  }

  //find out the current scale denominater and set it to the SLD parser
  QgsScaleCalculator scaleCalc(( theImage->logicalDpiX() + theImage->logicalDpiY() ) / 2 , mMapRenderer->destinationCrs().mapUnits() );
  QgsRectangle mapExtent = mMapRenderer->extent();
  mConfigParser->setScaleDenominator( scaleCalc.calculate( mapExtent, theImage->width() ) );

  layerIdList = layerSet( layersList, stylesList, mMapRenderer->destinationCrs() );
#ifdef QGISDEBUG
  QgsDebugMsg( QString( "Number of layers to be rendered. %1" ).arg( layerIdList.count() ) );
#endif
  mMapRenderer->setLayerSet( layerIdList );

  return theImage;
}

QImage* QgsWMSServer::createImage( int width, int height ) const
{
  bool conversionSuccess;

  if ( width < 0 )
  {
    width = mParameterMap.value( "WIDTH", "0" ).toInt( &conversionSuccess );
    if ( !conversionSuccess )
      width = 0;
  }

  if ( height < 0 )
  {
    height = mParameterMap.value( "HEIGHT", "0" ).toInt( &conversionSuccess );
    if ( !conversionSuccess )
    {
      height = 0;
    }
  }

  if ( width < 0 || height < 0 )
  {
    return 0;
  }

  QImage* theImage = 0;

  //is format jpeg?
  QString format = mParameterMap.value( "FORMAT" );
  bool jpeg = format.compare( "jpg", Qt::CaseInsensitive ) == 0
              || format.compare( "jpeg", Qt::CaseInsensitive ) == 0
              || format.compare( "image/jpeg", Qt::CaseInsensitive ) == 0;

  //transparent parameter
  bool transparent = mParameterMap.value( "TRANSPARENT" ).compare( "true", Qt::CaseInsensitive ) == 0;

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
    return 0;
  }

  //apply DPI parameter if present. This is an extension of QGIS mapserver compared to WMS 1.3.
  //Because of backwards compatibility, this parameter is optional
  if ( mParameterMap.contains( "DPI" ) )
  {
    int dpi = mParameterMap[ "DPI" ].toInt( &conversionSuccess );
    if ( conversionSuccess )
    {
      int dpm = dpi / 0.0254;
      theImage->setDotsPerMeterX( dpm );
      theImage->setDotsPerMeterY( dpm );
    }
  }
  return theImage;
}

int QgsWMSServer::configureMapRender( const QPaintDevice* paintDevice ) const
{
  if ( !mMapRenderer || !paintDevice )
  {
    return 1; //paint device is needed for height, width, dpi
  }

  mMapRenderer->setOutputSize( QSize( paintDevice->width(), paintDevice->height() ), paintDevice->logicalDpiX() );

  //map extent
  bool conversionSuccess;
  double minx, miny, maxx, maxy;
  QString bbString = mParameterMap.value( "BBOX", "0,0,0,0" );

  bool bboxOk = true;
  minx = bbString.section( ",", 0, 0 ).toDouble( &conversionSuccess );
  if ( !conversionSuccess )
    bboxOk = false;
  miny = bbString.section( ",", 1, 1 ).toDouble( &conversionSuccess );
  if ( !conversionSuccess )
    bboxOk = false;
  maxx = bbString.section( ",", 2, 2 ).toDouble( &conversionSuccess );
  if ( !conversionSuccess )
    bboxOk = false;
  maxy = bbString.section( ",", 3, 3 ).toDouble( &conversionSuccess );
  if ( !conversionSuccess )
    bboxOk = false;

  if ( !bboxOk )
  {
    //throw a service exception
    throw QgsMapServiceException( "InvalidParameterValue", "Invalid BBOX parameter" );
  }

  QGis::UnitType mapUnits = QGis::Degrees;

  QString crs = mParameterMap.value( "CRS", mParameterMap.value( "SRS" ) );

  QgsCoordinateReferenceSystem outputCRS;

  //wms spec says that CRS parameter is mandatory.
  //we don't rejeict the request if it is not there but disable reprojection on the fly
  if ( crs.isEmpty() )
  {
    //disable on the fly projection
    QgsProject::instance()->writeEntry( "SpatialRefSys", "/ProjectionsEnabled", 0 );
  }
  else
  {
    //enable on the fly projection
    QgsDebugMsg( "enable on the fly projection" );
    QgsProject::instance()->writeEntry( "SpatialRefSys", "/ProjectionsEnabled", 1 );

    //destination SRS
    outputCRS = QgsCRSCache::instance()->crsByAuthId( crs );
    if ( !outputCRS.isValid() )
    {
      QgsDebugMsg( "Error, could not create output CRS from EPSG" );
      throw QgsMapServiceException( "InvalidCRS", "Could not create output CRS" );
      return 5;
    }
    mMapRenderer->setDestinationCrs( outputCRS );
    mMapRenderer->setProjectionsEnabled( true );
    mapUnits = outputCRS.mapUnits();
  }
  mMapRenderer->setMapUnits( mapUnits );

  // Change x- and y- of BBOX for WMS 1.3.0 if axis inverted
  QString version = mParameterMap.value( "VERSION", "1.3.0" );
  if ( version != "1.1.1" && outputCRS.axisInverted() )
  {
    //switch coordinates of extent
    double tmp;
    tmp = minx;
    minx = miny; miny = tmp;
    tmp = maxx;
    maxx = maxy; maxy = tmp;
  }

  QgsRectangle mapExtent( minx, miny, maxx, maxy );
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
  layersList = mParameterMap.value( "LAYER" ).split( ",", QString::SkipEmptyParts );
  layersList = layersList + mParameterMap.value( "LAYERS" ).split( ",", QString::SkipEmptyParts );
  stylesList = mParameterMap.value( "STYLE" ).split( ",", QString::SkipEmptyParts );
  stylesList = stylesList + mParameterMap.value( "STYLES" ).split( ",", QString::SkipEmptyParts );

  return 0;
}

int QgsWMSServer::initializeSLDParser( QStringList& layersList, QStringList& stylesList )
{
  QString xml = mParameterMap.value( "SLD" );
  if ( !xml.isEmpty() )
  {
    //ignore LAYERS and STYLES and take those information from the SLD
    QDomDocument* theDocument = new QDomDocument( "user.sld" );
    QString errorMsg;
    int errorLine, errorColumn;

    if ( !theDocument->setContent( xml, true, &errorMsg, &errorLine, &errorColumn ) )
    {
      //std::cout << xml.toAscii().data() << std::endl;
      QgsDebugMsg( "Error, could not create DomDocument from SLD" );
      QgsDebugMsg( QString( "The error message is: %1" ).arg( errorMsg ) );
      delete theDocument;
      return 0;
    }
    QgsSLDParser* userSLDParser = new QgsSLDParser( theDocument );
    userSLDParser->setParameterMap( mParameterMap );
    userSLDParser->setFallbackParser( mConfigParser );
    mConfigParser = userSLDParser;
    //now replace the content of layersList and stylesList (if present)
    layersList.clear();
    stylesList.clear();
    QStringList layersSTDList;
    QStringList stylesSTDList;
    if ( mConfigParser->layersAndStyles( layersSTDList, stylesSTDList ) != 0 )
    {
      QgsDebugMsg( "Error, no layers and styles found in SLD" );
      return 0;
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

int QgsWMSServer::infoPointToLayerCoordinates( int i, int j, QgsPoint* layerCoords, QgsMapRenderer* mapRender,
    QgsMapLayer* layer ) const
{
  if ( !layerCoords || !mapRender || !layer || !mapRender->coordinateTransform() )
  {
    return 1;
  }

  //first transform i,j to map output coordinates
  // toMapCoordinates() is currently (Oct 18 2012) using average resolution
  // to calc point but GetFeatureInfo request may be sent with different
  // resolutions in each axis
  //QgsPoint mapPoint = mapRender->coordinateTransform()->toMapCoordinates( i, j );
  double xRes = mapRender->extent().width() / mapRender->width();
  double yRes = mapRender->extent().height() / mapRender->height();
  QgsPoint mapPoint( mapRender->extent().xMinimum() + i * xRes,
                     mapRender->extent().yMaximum() - j * yRes );

  QgsDebugMsg( QString( "mapPoint (corner): %1 %2" ).arg( mapPoint.x() ).arg( mapPoint.y() ) );
  // use pixel center instead of corner
  // Unfortunately going through pixel (integer) we cannot reconstruct precisely
  // the coordinate clicked on client and thus result may differ from
  // the same raster loaded and queried localy on client
  mapPoint.setX( mapPoint.x() + xRes / 2 );
  mapPoint.setY( mapPoint.y() - yRes / 2 );

  QgsDebugMsg( QString( "mapPoint (pixel center): %1 %2" ).arg( mapPoint.x() ).arg( mapPoint.y() ) );

  //and then to layer coordinates
  *layerCoords = mapRender->mapToLayerCoordinates( layer, mapPoint );
  QgsDebugMsg( QString( "mapPoint: %1 %2" ).arg( mapPoint.x() ).arg( mapPoint.y() ) );
  return 0;
}

int QgsWMSServer::featureInfoFromVectorLayer( QgsVectorLayer* layer,
    const QgsPoint* infoPoint,
    int nFeatures,
    QDomDocument& infoDocument,
    QDomElement& layerElement,
    QgsMapRenderer* mapRender,
    QgsRenderContext& renderContext,
    QString version,
    QString infoFormat,
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
    double searchRadius = 0;
    if ( layer->geometryType() == QGis::Polygon )
    {
      searchRadius = layerRect.width() / 400;
    }
    else if ( layer->geometryType() == QGis::Line )
    {
      searchRadius = layerRect.width() / 200;
    }
    else
    {
      searchRadius = layerRect.width() / 100;
    }

    searchRect.set( infoPoint->x() - searchRadius, infoPoint->y() - searchRadius,
                    infoPoint->x() + searchRadius, infoPoint->y() + searchRadius );
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
  fReq.setFlags((( addWktGeometry || featureBBox ) ? QgsFeatureRequest::NoFlags : QgsFeatureRequest::NoGeometry ) | QgsFeatureRequest::ExactIntersect );
  if ( !searchRect.isEmpty() )
  {
    fReq.setFilterRect( searchRect );
  }
  QgsFeatureIterator fit = layer->getFeatures( fReq );

  while ( fit.nextFeature( feature ) )
  {
    ++featureCounter;
    if ( featureCounter > nFeatures )
    {
      break;
    }

    // Creates the gml:featureMember only if we have at least one element
    if ( infoFormat.startsWith("application/vnd.ogc.gml") && featureCounter == 1 )
    {
      QDomElement realLayerElement = infoDocument.createElement( "gml:featureMember"/*wfs:FeatureMember*/ );
      layerElement.appendChild( realLayerElement );
      layerElement = realLayerElement;
    }

    QgsFeatureRendererV2* r2 = layer->rendererV2();
    if ( !r2 )
    {
      continue;
    }

    //check if feature is rendered at all
    r2->startRender( renderContext, layer );
    bool renderV2 = r2->willRenderFeature( feature );
    r2->stopRender( renderContext );
    if ( !renderV2 )
    {
      continue;
    }

    if (infoFormat == "application/vnd.ogc.gml") {
      QgsCoordinateReferenceSystem layerCrs = layer->crs();
      bool withGeom = layer->wkbType() != QGis::WKBNoGeometry;
      int version = infoFormat.startsWith("application/vnd.ogc.gml/3") ? 3 : 2;
      QDomElement elem = createFeatureGML(&feature, infoDocument, layerCrs, layer->name(), withGeom, version);
      layerElement.appendChild(elem);
      continue;
    }
    else {
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
  
        //replace attribute name if there is an attribute alias?
        QString attributeName = layer->attributeDisplayName( i );
  
        QDomElement attributeElement = infoDocument.createElement( "Attribute" );
        attributeElement.setAttribute( "name", attributeName );
        attributeElement.setAttribute( "value", featureAttributes[i].toString() );
        featureElement.appendChild( attributeElement );
      }
  
      //also append the wkt geometry as an attribute
      QgsGeometry* geom = feature.geometry();
      if ( addWktGeometry && geom )
      {
        QDomElement geometryElement = infoDocument.createElement( "Attribute" );
        geometryElement.setAttribute( "name", "geometry" );
        geometryElement.setAttribute( "value", geom->exportToWkt() );
        geometryElement.setAttribute( "type", "derived" );
        featureElement.appendChild( geometryElement );
      }
      if ( featureBBox && geom && mapRender ) //extend feature info bounding box if requested
      {
        QgsRectangle box = mapRender->layerExtentToOutputExtent( layer, geom->boundingBox() );
        if ( featureBBox->isEmpty() )
        {
          *featureBBox = box;
        }
        else
        {
          featureBBox->combineExtentWith( &box );
        }
  
        //append feature bounding box to feature info xml
        QDomElement bBoxElem = infoDocument.createElement( "BoundingBox" );
        bBoxElem.setAttribute( version == "1.1.1" ? "SRS" : "CRS", mapRender->destinationCrs().authid() );
        bBoxElem.setAttribute( "minx", QString::number( box.xMinimum() ) );
        bBoxElem.setAttribute( "maxx", QString::number( box.xMaximum() ) );
        bBoxElem.setAttribute( "miny", QString::number( box.yMinimum() ) );
        bBoxElem.setAttribute( "maxy", QString::number( box.yMaximum() ) );
        featureElement.appendChild( bBoxElem );
      }
    }
  }

  return 0;
}

int QgsWMSServer::featureInfoFromRasterLayer( QgsRasterLayer* layer,
    const QgsPoint* infoPoint,
    QDomDocument& infoDocument,
    QDomElement& layerElement,
    QString version,
    QString infoFormat ) const
{
  Q_UNUSED( version );

  if ( !infoPoint || !layer || !layer->dataProvider() )
  {
    return 1;
  }

  QgsDebugMsg( QString( "infoPoint: %1 %2" ).arg( infoPoint->x() ).arg( infoPoint->y() ) );

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

  if (infoFormat == "application/vnd.ogc.gml") {
    QgsFeature feature;
    for (QMap<int, QVariant>::const_iterator it = attributes.constBegin(); it != attributes.constEnd(); ++it)
    {
      feature.setAttribute(layer->bandName(it.key()), QString::number(it.value().toDouble()));
    }
    
    QgsCoordinateReferenceSystem layerCrs = layer->crs();
    int version = infoFormat.startsWith("application/vnd.ogc.gml/3") ? 3 : 2;
    QDomElement elem = createFeatureGML(&feature, infoDocument, layerCrs, layer->name(), false, version);
    layerElement.appendChild(elem);
  }
  else {
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
  QgsMapLayer* theMapLayer = 0;
  QgsDebugMsg( QString( "Calculating layerset using %1 layers, %2 styles and CRS %3" ).arg( layersList.count() ).arg( stylesList.count() ).arg( destCRS.description() ) );
  for ( llstIt = layersList.begin(), slstIt = stylesList.begin(); llstIt != layersList.end(); ++llstIt )
  {

    QString styleName;
    if ( slstIt != stylesList.end() )
    {
      styleName = *slstIt;
    }
    QgsDebugMsg( "Trying to get layer " + *llstIt + "//" + styleName );

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
      QgsDebugMsg( QString( "Checking layer: %1" ).arg( theMapLayer->name() ) );
      if ( theMapLayer )
      {
        //test if layer is visible in requested scale
        bool useScaleConstraint = ( scaleDenominator > 0 && theMapLayer->hasScaleBasedVisibility() );
        if ( !useScaleConstraint ||
             ( theMapLayer->minimumScale() <= scaleDenominator && theMapLayer->maximumScale() >= scaleDenominator ) )
        {
          layerKeys.push_front( theMapLayer->id() );
          //joinVectorLayers
          QgsVectorLayer* vectorLayer = dynamic_cast<QgsVectorLayer*>( theMapLayer );
          if ( vectorLayer && vectorLayer->vectorJoins().size() > 0 )
          {
            QList<QgsMapLayer *> joinLayers;
            //insert existing join info
            const QList< QgsVectorJoinInfo >& joins = vectorLayer->vectorJoins();
            for ( int i = 0; i < joins.size(); ++i )
            {
              QgsMapLayer* joinLayer = mConfigParser->mapLayerFromLayerId( joins[i].joinLayerId );
              if ( joinLayer )
              {
                joinLayers << joinLayer;
              }
              QgsMapLayerRegistry::instance()->addMapLayers( joinLayers, false, true );
            }
            vectorLayer->updateFields();
          }
          QgsMapLayerRegistry::instance()->addMapLayers(
            QList<QgsMapLayer *>() << theMapLayer, false, false );
        }
      }
      else
      {
        QgsDebugMsg( "Layer or style not defined, aborting" );
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

void QgsWMSServer::drawLegendLayerItem( QgsComposerLayerItem* item, QPainter* p, double& maxTextWidth, double& maxSymbolWidth, double& currentY, const QFont& layerFont,
                                        const QColor& layerFontColor, const QFont& itemFont, const QColor&  itemFontColor, double boxSpace, double layerSpace,
                                        double layerTitleSpace, double symbolSpace, double iconLabelSpace, double symbolWidth, double symbolHeight, double fontOversamplingFactor,
                                        double dpi ) const
{
  Q_UNUSED( layerSpace );
  if ( !item )
  {
    return;
  }

  QFontMetricsF layerFontMetrics( layerFont );
  currentY += layerFontMetrics.ascent() / fontOversamplingFactor;

  //draw layer title first
  if ( p )
  {
    p->save();
    p->scale( 1.0 / fontOversamplingFactor, 1.0 / fontOversamplingFactor );
    p->setPen( layerFontColor );
    p->setFont( layerFont );
    p->drawText( boxSpace * fontOversamplingFactor, currentY * fontOversamplingFactor, item->text() );
    p->restore();
  }
  else
  {
    double layerItemWidth = layerFontMetrics.width( item->text() ) / fontOversamplingFactor + boxSpace;
    if ( layerItemWidth > maxTextWidth )
    {
      maxTextWidth = layerItemWidth;
    }
  }

  currentY += layerTitleSpace;

  //then draw all the children
  QFontMetricsF itemFontMetrics( itemFont );

  int nChildItems = item->rowCount();
  QgsComposerLegendItem* currentComposerItem = 0;

  for ( int i = 0; i < nChildItems; ++i )
  {
    currentComposerItem = dynamic_cast<QgsComposerLegendItem*>( item->child( i ) );
    if ( !currentComposerItem )
    {
      continue;
    }

    double currentSymbolHeight = symbolHeight;
    double currentSymbolWidth = symbolWidth; //symbol width (without box space and icon/label space
    double currentTextWidth = 0;

    //if the font is larger than the standard symbol size, try to draw the symbol centered (shifting towards the bottom)
    double symbolDownShift = ( itemFontMetrics.ascent() / fontOversamplingFactor - symbolHeight ) / 2.0;
    if ( symbolDownShift < 0 )
    {
      symbolDownShift = 0;
    }

    QgsComposerLegendItem::ItemType type = currentComposerItem->itemType();
    switch ( type )
    {
      case QgsComposerLegendItem::SymbologyV2Item:
        drawLegendSymbolV2( currentComposerItem, p, boxSpace, currentY, currentSymbolWidth, currentSymbolHeight, dpi, symbolDownShift );
        break;
      case QgsComposerLegendItem::RasterSymbolItem:
        drawRasterSymbol( currentComposerItem, p, boxSpace, currentY, currentSymbolWidth, currentSymbolHeight, symbolDownShift );
        break;
      case QgsComposerLegendItem::GroupItem:
        //QgsDebugMsg( "GroupItem not handled" );
        break;
      case QgsComposerLegendItem::LayerItem:
        //QgsDebugMsg( "GroupItem not handled" );
        break;
      case QgsComposerLegendItem::StyleItem:
        //QgsDebugMsg( "StyleItem not handled" );
        break;
    }

    //finally draw text
    currentTextWidth = itemFontMetrics.width( currentComposerItem->text() ) / fontOversamplingFactor;
    double symbolItemHeight = qMax( itemFontMetrics.ascent() / fontOversamplingFactor, currentSymbolHeight );

    if ( p )
    {
      p->save();
      p->scale( 1.0 / fontOversamplingFactor, 1.0 / fontOversamplingFactor );
      p->setPen( itemFontColor );
      p->setFont( itemFont );
      p->drawText( maxSymbolWidth * fontOversamplingFactor,
                   ( currentY + symbolItemHeight / 2.0 ) * fontOversamplingFactor + itemFontMetrics.ascent() / 2.0, currentComposerItem->text() );
      p->restore();
    }
    else
    {
      if ( currentTextWidth > maxTextWidth )
      {
        maxTextWidth = currentTextWidth;
      }
      double symbolWidth = boxSpace + currentSymbolWidth + iconLabelSpace;
      if ( symbolWidth > maxSymbolWidth )
      {
        maxSymbolWidth = symbolWidth;
      }
    }

    currentY += symbolItemHeight;
    if ( i < ( nChildItems - 1 ) )
    {
      currentY += symbolSpace;
    }
  }
}


void QgsWMSServer::drawLegendSymbolV2( QgsComposerLegendItem* item, QPainter* p, double boxSpace, double currentY, double& symbolWidth,
                                       double& symbolHeight, double dpi, double yDownShift ) const
{
  QgsComposerSymbolV2Item* symbolItem = dynamic_cast< QgsComposerSymbolV2Item* >( item );
  if ( !symbolItem )
  {
    return;
  }
  QgsSymbolV2* symbol = symbolItem->symbolV2();
  if ( !symbol )
  {
    return;
  }

  //markers might have a different size
  QgsMarkerSymbolV2* markerSymbol = dynamic_cast< QgsMarkerSymbolV2* >( symbol );
  if ( markerSymbol )
  {
    symbolWidth = markerSymbol->size() * dpi / 25.4;
    symbolHeight = markerSymbol->size() * dpi / 25.4;
  }

  if ( p )
  {
    p->save();
    p->translate( boxSpace, currentY + yDownShift );
    p->scale( 1.0, 1.0 );
    symbol->drawPreviewIcon( p, QSize( symbolWidth, symbolHeight ) );
    p->restore();
  }
}

void QgsWMSServer::drawRasterSymbol( QgsComposerLegendItem* item, QPainter* p, double boxSpace, double currentY, double symbolWidth, double symbolHeight, double yDownShift ) const
{
  if ( !item || ! p )
  {
    return;
  }

  QgsComposerRasterSymbolItem* rasterItem = dynamic_cast< QgsComposerRasterSymbolItem* >( item );
  if ( !rasterItem )
  {
    return;
  }

  QgsRasterLayer* layer = qobject_cast< QgsRasterLayer* >( QgsMapLayerRegistry::instance()->mapLayer( rasterItem->layerID() ) );
  if ( !layer )
  {
    return;
  }

  p->setBrush( QBrush( rasterItem->color() ) );
  p->drawRect( QRectF( boxSpace, currentY + yDownShift, symbolWidth, symbolHeight ) );
}

QMap<QString, QString> QgsWMSServer::applyRequestedLayerFilters( const QStringList& layerList ) const
{
  QMap<QString, QString> filterMap;

  if ( layerList.isEmpty() )
  {
    return filterMap;
  }

  QString filterParameter = mParameterMap.value( "FILTER" );
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

      foreach ( QgsMapLayer *layer, QgsMapLayerRegistry::instance()->mapLayers() )
      {
        if ( layer && layer->name() == eqSplit.at( 0 ) )
        {
          layersToFilter.push_back( layer );
        }
      }

      foreach ( QgsMapLayer *filter, layersToFilter )
      {
        QgsVectorLayer* filteredLayer = dynamic_cast<QgsVectorLayer*>( filter );
        if ( filteredLayer )
        {
          filterMap.insert( filteredLayer->id(), filteredLayer->subsetString() );
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
      QMap<QString, QString>::const_iterator filterIt = filterMap.constBegin();
      for ( ; filterIt != filterMap.constEnd(); ++filterIt )
      {
        QgsMapLayer* mapLayer = QgsMapLayerRegistry::instance()->mapLayer( filterIt.key() );
        if ( !mapLayer )
        {
          continue;
        }

        QgsRectangle layerExtent = mapLayer->extent();
        if ( filterExtent.isEmpty() )
        {
          filterExtent = layerExtent;
        }
        else
        {
          filterExtent.combineExtentWith( &layerExtent );
        }
      }
      mMapRenderer->setExtent( filterExtent );
    }
  }
  return filterMap;
}

void QgsWMSServer::restoreLayerFilters( const QMap < QString, QString >& filterMap ) const
{
  QMap < QString, QString >::const_iterator filterIt = filterMap.constBegin();
  for ( ; filterIt != filterMap.constEnd(); ++filterIt )
  {
    QgsVectorLayer* filteredLayer = dynamic_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( filterIt.key() ) );
    if ( filteredLayer )
    {
      QgsVectorDataProvider* dp = filteredLayer->dataProvider();
      if ( dp )
      {
        dp->setSubsetString( filterIt.value() );
      }
    }
  }
}

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
         || tokenIt->compare( "AND", Qt::CaseInsensitive ) == 0
         || tokenIt->compare( "OR", Qt::CaseInsensitive ) == 0
         || tokenIt->compare( "IN", Qt::CaseInsensitive ) == 0
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
  //group contens within single quotes together
  bool groupActive = false;
  int startGroup = -1;
  int endGroup = -1;
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
      endGroup = i;
      groupActive = false;

      if ( startGroup != -1 )
      {
        list[startGroup] = concatString;
        for ( int j = startGroup + 1; j <= endGroup; ++j )
        {
          list.removeAt( j );
          --i;
        }
      }

      concatString.clear();
      startGroup = -1;
      endGroup = -1;
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

  QString selectionString = mParameterMap.value( "SELECTION" );
  if ( selectionString.isEmpty() )
  {
    return layersWithSelections;
  }

  foreach ( QString selectionLayer, selectionString.split( ";" ) )
  {
    //separate layer name from id list
    QStringList layerIdSplit = selectionLayer.split( ":" );
    if ( layerIdSplit.size() < 2 )
    {
      continue;
    }

    //find layerId for layer name
    QString layerName = layerIdSplit.at( 0 );
    QgsVectorLayer* vLayer = 0;

    foreach ( QgsMapLayer *layer, QgsMapLayerRegistry::instance()->mapLayers() )
    {
      if ( layer && layer->name() == layerName )
      {
        vLayer = qobject_cast<QgsVectorLayer*>( layer );
        layersWithSelections.push_back( vLayer->id() );
        break;
      }
    }

    if ( !vLayer )
    {
      continue;
    }

    QStringList idList = layerIdSplit.at( 1 ).split( "," );
    QgsFeatureIds selectedIds;

    foreach ( QString id, idList )
    {
      selectedIds.insert( STRING_TO_FID( id ) );
    }

    vLayer->setSelectedFeatures( selectedIds );
  }


  return layersWithSelections;
}

void QgsWMSServer::clearFeatureSelections( const QStringList& layerIds ) const
{
  const QMap<QString, QgsMapLayer*>& layerMap = QgsMapLayerRegistry::instance()->mapLayers();

  foreach ( QString id, layerIds )
  {
    QgsVectorLayer *layer = qobject_cast< QgsVectorLayer * >( layerMap.value( id, 0 ) );
    if ( !layer )
      continue;

    layer->setSelectedFeatures( QgsFeatureIds() );
  }

  return;
}

void QgsWMSServer::applyOpacities( const QStringList& layerList, QList< QPair< QgsVectorLayer*, QgsFeatureRendererV2*> >& vectorRenderers,
                                   QList< QPair< QgsRasterLayer*, QgsRasterRenderer* > >& rasterRenderers,
                                   QList< QPair< QgsVectorLayer*, double > >& labelTransparencies,
                                   QList< QPair< QgsVectorLayer*, double > >& labelBufferTransparencies )
{
  //get opacity list
  QMap<QString, QString>::const_iterator opIt = mParameterMap.find( "OPACITIES" );
  if ( opIt == mParameterMap.constEnd() )
  {
    return;
  }
  QStringList opacityList = opIt.value().split( "," );

  //collect leaf layers and their opacity
  QList< QPair< QgsMapLayer*, int > > layerOpacityList;
  QStringList::const_iterator oIt = opacityList.constBegin();
  QStringList::const_iterator lIt = layerList.constBegin();
  for ( ; oIt != opacityList.constEnd(); ++oIt, ++lIt )
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

  QList< QPair< QgsMapLayer*, int > >::const_iterator lOpIt = layerOpacityList.constBegin();
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
      QgsSymbolV2List symbolList = rendererV2->symbols();
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
          rasterRenderers.push_back( qMakePair( rl, dynamic_cast<QgsRasterRenderer*>( rasterRenderer->clone() ) ) );
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
    QMap<QString, QString>::const_iterator widthIt = mParameterMap.find( "WIDTH" );
    if ( widthIt != mParameterMap.constEnd() )
    {
      if ( widthIt->toInt() > mConfigParser->maxWidth() )
      {
        return false;
      }
    }
  }
  if ( mConfigParser->maxHeight() != -1 )
  {
    QMap<QString, QString>::const_iterator heightIt = mParameterMap.find( "HEIGHT" );
    if ( heightIt != mParameterMap.constEnd() )
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
  QUrl mapUrl( getenv( "REQUEST_URI" ) );
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
      if ( attributeList.size() > 0 )
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
      if ( attributeList.size() > 0 )
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
    QDomDocument& doc,
    QgsCoordinateReferenceSystem& crs,
    QString typeName,
    bool withGeom,
    int version) const
{
  //qgs:%TYPENAME%
  QDomElement typeNameElement = doc.createElement( "qgs:" + typeName /*qgs:%TYPENAME%*/ );
  typeNameElement.setAttribute( "fid", typeName + "." + QString::number( feat->id() ) );

  if ( withGeom )
  {
    //add geometry column (as gml)
    QgsGeometry* geom = feat->geometry();

    QDomElement geomElem = doc.createElement( "qgs:geometry" );
    QDomElement gmlElem;
    if (version < 3) {
      gmlElem = QgsOgcUtils::geometryToGML( geom, doc );
    }
    else {
      gmlElem = QgsOgcUtils::geometryToGML( geom, doc, "GML3" );
    }
    
    if ( !gmlElem.isNull() )
    {
      QgsRectangle box = geom->boundingBox();
      QDomElement bbElem = doc.createElement( "gml:boundedBy" );
      QDomElement boxElem;
      if (version < 3) {
        boxElem = QgsOgcUtils::rectangleToGMLBox( &box, doc );
      }
      else {
        boxElem = QgsOgcUtils::rectangleToGMLEnvelope( &box, doc );
      }

      if ( crs.isValid() )
      {
        boxElem.setAttribute( "srsName", crs.authid() );
        gmlElem.setAttribute( "srsName", crs.authid() );
      }

      bbElem.appendChild( boxElem );
      typeNameElement.appendChild( bbElem );

      geomElem.appendChild( gmlElem );
      typeNameElement.appendChild( geomElem );
    }
  }

  //read all attribute values from the feature
  QgsAttributes featureAttributes = feat->attributes();
  const QgsFields* fields = feat->fields();
  for ( int i = 0; i < fields->count(); ++i )
  {
    QString attributeName = fields->at( i ).name();
    QDomElement fieldElem = doc.createElement( "qgs:" + attributeName.replace( QString( " " ), QString( "_" ) ) );
    QDomText fieldText = doc.createTextNode( featureAttributes[i].toString() );
    fieldElem.appendChild( fieldText );
    typeNameElement.appendChild( fieldElem );
  }

  return typeNameElement;
}
