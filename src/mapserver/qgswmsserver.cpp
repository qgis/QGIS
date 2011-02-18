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
#include "qgsepsgcache.h"
#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaprenderer.h"
#include "qgsmaptopixel.h"
#include "qgspallabeling.h"
#include "qgsproject.h"
#include "qgsrasterlayer.h"
#include "qgsscalecalculator.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsmapserverlogger.h"
#include "qgsmapserviceexception.h"
#include "qgssldparser.h"
#include "qgssymbol.h"
#include "qgssymbolv2.h"
#include "qgsrenderer.h"
#include "qgslegendmodel.h"
#include "qgscomposerlegenditem.h"
#include "qgslogger.h"
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

QgsWMSServer::QgsWMSServer( std::map<QString, QString> parameters, QgsMapRenderer* renderer )
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

QDomDocument QgsWMSServer::getCapabilities()
{
  QgsMSDebugMsg( "Entering." );
  QDomDocument doc;
  //wms:WMS_Capabilities element
  QDomElement wmsCapabilitiesElement = doc.createElement( "WMS_Capabilities"/*wms:WMS_Capabilities*/ );
  wmsCapabilitiesElement.setAttribute( "xmlns", "http://www.opengis.net/wms" );
  wmsCapabilitiesElement.setAttribute( "xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance" );
  wmsCapabilitiesElement.setAttribute( "xsi:schemaLocation", "http://www.opengis.net/wms http://schemas.opengis.net/wms/1.3.0/capabilities_1_3_0.xsd" );
  wmsCapabilitiesElement.setAttribute( "version", "1.3.0" );
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
  //wms:GetCapabilities
  QDomElement getCapabilitiesElement = doc.createElement( "GetCapabilities"/*wms:GetCapabilities*/ );
  requestElement.appendChild( getCapabilitiesElement );
  QDomElement capabilitiesFormatElement = doc.createElement( "Format" );/*wms:Format*/
  getCapabilitiesElement.appendChild( capabilitiesFormatElement );
  QDomText capabilitiesFormatText = doc.createTextNode( "text/xml" );
  capabilitiesFormatElement.appendChild( capabilitiesFormatText );

  QDomElement dcpTypeElement = doc.createElement( "DCPType"/*wms:DCPType*/ );
  getCapabilitiesElement.appendChild( dcpTypeElement );
  QDomElement httpElement = doc.createElement( "HTTP"/*wms:HTTP*/ );
  dcpTypeElement.appendChild( httpElement );

  //Prepare url
  //Some client requests already have http://<SERVER_NAME> in the REQUEST_URI variable
  QString hrefString;
  QString requestUrl = getenv( "REQUEST_URI" );
  requestUrl.truncate( requestUrl.indexOf( "?" ) + 1 );
  if ( requestUrl.contains( "http" ) )
  {
    hrefString = requestUrl;
  }
  else
  {
    hrefString = "http://" + QString( getenv( "SERVER_NAME" ) ) + requestUrl;
  }


  // SOAP platform
  //only give this information if it is not a WMS request to be in sync with the WMS capabilities schema
  std::map<QString, QString>::const_iterator service_it = mParameterMap.find( "SERVICE" );
  if ( service_it != mParameterMap.end() && service_it->second.compare( "WMS", Qt::CaseInsensitive ) != 0 )
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
  requestUrl.truncate( requestUrl.indexOf( "?" ) + 1 );
  olResourceElement.setAttribute( "xlink:href", hrefString );
  getElement.appendChild( olResourceElement );

  // POST already used by SOAP
//  QDomElement postElement = doc.createElement("post"/*wms:SOAP*/);
//  httpElement.appendChild(postElement);
//  QDomElement postResourceElement = doc.createElement("OnlineResource"/*wms:OnlineResource*/);
//  postResourceElement.setAttribute("xmlns:xlink","http://www.w3.org/1999/xlink");
//  postResourceElement.setAttribute("xlink:type","simple");
//  postResourceElement.setAttribute("xlink:href", "http://" + QString(getenv("SERVER_NAME")) + QString(getenv("REQUEST_URI")));
//  postElement.appendChild(postResourceElement);
//  dcpTypeElement.appendChild(postElement);

  //wms:GetMap
  QDomElement getMapElement = doc.createElement( "GetMap"/*wms:GetMap*/ );
  requestElement.appendChild( getMapElement );
  QDomElement jpgFormatElement = doc.createElement( "Format"/*wms:Format*/ );
  QDomText jpgFormatText = doc.createTextNode( "image/jpeg" );
  jpgFormatElement.appendChild( jpgFormatText );
  getMapElement.appendChild( jpgFormatElement );
  QDomElement pngFormatElement = doc.createElement( "Format"/*wms:Format*/ );
  QDomText pngFormatText = doc.createTextNode( "image/png" );
  pngFormatElement.appendChild( pngFormatText );
  getMapElement.appendChild( pngFormatElement );
  QDomElement getMapDhcTypeElement = dcpTypeElement.cloneNode().toElement();//this is the same as for 'GetCapabilities'
  getMapElement.appendChild( getMapDhcTypeElement );

  //wms:GetFeatureInfo
  QDomElement getFeatureInfoElem = doc.createElement( "GetFeatureInfo" );
  //text/plain
  QDomElement textFormatElem = doc.createElement( "Format" );
  QDomText textFormatText = doc.createTextNode( "text/plain" );
  textFormatElem.appendChild( textFormatText );
  getFeatureInfoElem.appendChild( textFormatElem );
  //text/html
  QDomElement htmlFormatElem = doc.createElement( "Format" );
  QDomText htmlFormatText = doc.createTextNode( "text/html" );
  htmlFormatElem.appendChild( htmlFormatText );
  getFeatureInfoElem.appendChild( htmlFormatElem );
  //text/xml
  QDomElement xmlFormatElem = doc.createElement( "Format" );
  QDomText xmlFormatText = doc.createTextNode( "text/xml" );
  xmlFormatElem.appendChild( xmlFormatText );
  getFeatureInfoElem.appendChild( xmlFormatElem );

  //dcpType
  QDomElement getFeatureInfoDhcTypeElement = dcpTypeElement.cloneNode().toElement();//this is the same as for 'GetCapabilities'
  getFeatureInfoElem.appendChild( getFeatureInfoDhcTypeElement );
  requestElement.appendChild( getFeatureInfoElem );

  //Exception element is mandatory
  QDomElement exceptionElement = doc.createElement( "Exception" );
  QDomElement exFormatElement = doc.createElement( "Format" );
  QDomText formatText = doc.createTextNode( "text/xml" );
  exFormatElement.appendChild( formatText );
  exceptionElement.appendChild( exFormatElement );
  capabilityElement.appendChild( exceptionElement );

  //Insert <ComposerTemplate> elements derived from wms:_ExtendedCapabilities
  if ( mConfigParser )
  {
    mConfigParser->printCapabilities( capabilityElement, doc );
  }

  //add the xml content for the individual layers/styles
  QgsMSDebugMsg( "calling layersAndStylesCapabilities" );
  if ( mConfigParser )
  {
    mConfigParser->layersAndStylesCapabilities( capabilityElement, doc );
  }
  QgsMSDebugMsg( "layersAndStylesCapabilities returned" );

  //for debugging: save the document to disk
  /*QFile capabilitiesFile( QDir::tempPath() + "/capabilities.txt" );
  if ( capabilitiesFile.open( QIODevice::WriteOnly | QIODevice::Text ) )
  {
    QTextStream capabilitiesStream( &capabilitiesFile );
    doc.save( capabilitiesStream, 4 );
  }*/
  return doc;
}

QImage* QgsWMSServer::getLegendGraphics()
{
  if ( !mConfigParser || !mMapRenderer )
  {
    return false;
  }

  QStringList layersList, stylesList;

  if ( readLayersAndStyles( layersList, stylesList ) != 0 )
  {
    QgsMSDebugMsg( "error reading layers and styles" );
    return 0;
  }

  if ( layersList.size() < 1 )
  {
    return 0;
  }

  QgsCoordinateReferenceSystem dummyCRS;
  QStringList layerIds = layerSet( layersList, stylesList, dummyCRS );
  QgsLegendModel legendModel;
  legendModel.setLayerSet( layerIds );

  //create first image (to find out dpi)
  QImage* theImage = createImage( 10, 10 );
  if ( !theImage )
  {
    return 0;
  }
  double mmToPixelFactor = theImage->dotsPerMeterX() / 1000;

  //get icon size, spaces between legend items and font from config parser
  double boxSpace, layerSpace, symbolSpace, iconLabelSpace, symbolWidth, symbolHeight;
  boxSpace = mConfigParser->legendBoxSpace() * mmToPixelFactor;
  layerSpace = mConfigParser->legendLayerSpace() * mmToPixelFactor;
  symbolSpace = mConfigParser->legendSymbolSpace() * mmToPixelFactor;
  iconLabelSpace = mConfigParser->legendIconLabelSpace() * mmToPixelFactor;
  symbolWidth = mConfigParser->legendSymbolWidth() * mmToPixelFactor;
  symbolHeight = mConfigParser->legendSymbolHeight() * mmToPixelFactor;
  double maxTextWidth = 0;
  double maxSymbolWidth = 0;
  double currentY = 0;
  double fontOversamplingFactor = 10.0;
  QFont layerFont = mConfigParser->legendLayerFont();
  layerFont.setPixelSize( layerFont.pointSizeF() * 0.3528 * mmToPixelFactor * fontOversamplingFactor );
  QFont itemFont = mConfigParser->legendItemFont();
  itemFont.setPixelSize( itemFont.pointSizeF() * 0.3528 * mmToPixelFactor * fontOversamplingFactor );

  //first find out image dimensions without painting
  QStandardItem* rootItem = legendModel.invisibleRootItem();
  if ( !rootItem )
  {
    return 0;
  }

  int numLayerItems = rootItem->rowCount();
  if ( numLayerItems < 1 )
  {
    return 0;
  }

  currentY = boxSpace;
  for ( int i = 0; i < numLayerItems; ++i )
  {
    QgsComposerLayerItem* layerItem = dynamic_cast<QgsComposerLayerItem*>( rootItem->child( i ) );
    if ( layerItem )
    {

      drawLegendLayerItem( layerItem, 0, maxTextWidth, maxSymbolWidth, currentY, layerFont, itemFont, boxSpace, layerSpace, symbolSpace,
                           iconLabelSpace, symbolWidth, symbolHeight, fontOversamplingFactor, theImage->dotsPerMeterX() * 0.0254 );
    }
  }
  currentY += boxSpace;

  //create second image with the right dimensions
  QImage* paintImage = createImage( maxTextWidth + maxSymbolWidth, currentY );

  //go through the items a second time for painting
  QPainter p( paintImage );
  p.setRenderHint( QPainter::Antialiasing, true );
  currentY = boxSpace;

  for ( int i = 0; i < numLayerItems; ++i )
  {
    QgsComposerLayerItem* layerItem = dynamic_cast<QgsComposerLayerItem*>( rootItem->child( i ) );
    if ( layerItem )
    {
      drawLegendLayerItem( layerItem, &p, maxTextWidth, maxSymbolWidth, currentY, layerFont, itemFont, boxSpace, layerSpace, symbolSpace,
                           iconLabelSpace, symbolWidth, symbolHeight, fontOversamplingFactor, theImage->dotsPerMeterX() * 0.0254 );
    }
  }

  QgsMapLayerRegistry::instance()->mapLayers().clear();
  delete theImage;
  return paintImage;
}

QDomDocument QgsWMSServer::getStyle()
{
  QDomDocument doc;
  std::map<QString, QString>::const_iterator style_it = mParameterMap.find( "STYLE" );
  if ( style_it == mParameterMap.end() )
  {
    throw QgsMapServiceException( "StyleNotSpecified", "Style is manadatory for GetStyle operation" );
  }
  std::map<QString, QString>::const_iterator layer_it = mParameterMap.find( "LAYER" );
  if ( layer_it == mParameterMap.end() )
  {
    throw QgsMapServiceException( "LayerNotSpecified", "Layer is manadatory for GetStyle operation" );
  }

  QString styleName = style_it->second;
  QString layerName = layer_it->second;

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

  //GetPrint request needs a template parameter
  std::map<QString, QString>::const_iterator templateIt = mParameterMap.find( "TEMPLATE" );
  if ( templateIt == mParameterMap.end() )
  {
    throw QgsMapServiceException( "ParameterMissing", "The TEMPLATE parameter is required for the GetPrint request" );
  }

  QgsComposition* c = mConfigParser->createPrintComposition( templateIt->second, mMapRenderer, QMap<QString, QString>( mParameterMap ) );
  if ( !c )
  {
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
    QRectF sourceArea( 0, 0, c->paperWidth(), c->paperHeight() );
    QRectF targetArea( 0, 0, width, height );
    if ( c->printAsRaster() ) //embed one raster into the svg
    {
      QImage* img = printCompositionToImage( c );
      if ( img )
      {
        p.drawImage( targetArea, *img, QRectF( 0, 0, img->width(), img->height() ) );
      }
      delete img;
    }
    else
    {
      c->render( &p, targetArea, sourceArea );
    }
    p.end();
  }
  else if ( formatString.compare( "png", Qt::CaseInsensitive ) == 0 || formatString.compare( "jpg", Qt::CaseInsensitive ) == 0 )
  {
    QImage* image = printCompositionToImage( c );
    if ( image )
    {
      ba = new QByteArray();
      QBuffer buffer( ba );
      buffer.open( QIODevice::WriteOnly );
      image->save( &buffer, formatString.toLocal8Bit().data(), -1 );
    }
    delete image;
  }
  else if ( formatString.compare( "pdf", Qt::CaseInsensitive ) == 0 )
  {
    QTemporaryFile tempFile;
    if ( !tempFile.open() )
    {
      delete c;
      return 0;
    }

    QPrinter printer;
    printer.setResolution( c->printResolution() );
    printer.setFullPage( true );
    printer.setOutputFormat( QPrinter::PdfFormat );
    printer.setOutputFileName( tempFile.fileName() );
    printer.setPaperSize( QSizeF( c->paperWidth(), c->paperHeight() ), QPrinter::Millimeter );
    QRectF paperRectMM = printer.pageRect( QPrinter::Millimeter );
    QRectF paperRectPixel = printer.pageRect( QPrinter::DevicePixel );
    QPainter p( &printer );
    if ( c->printAsRaster() ) //embed one raster into the pdf
    {
      QImage* img = printCompositionToImage( c );
      if ( img )
      {
        p.drawImage( paperRectPixel, *img, QRectF( 0, 0, img->width(), img->height() ) );
      }
      delete img;
    }
    else //vector pdf
    {
      c->render( &p, paperRectPixel, paperRectMM );
    }
    p.end();

    ba = new QByteArray();
    *ba = tempFile.readAll();
  }
  else //unknown format
  {
    throw QgsMapServiceException( "InvalidFormat", "Output format '" + formatString + "' is not supported in the GetPrint request" );
  }

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
  QStringList layersList, stylesList, layerIdList;
  QImage* theImage = initializeRendering( layersList, stylesList, layerIdList );

  QPainter thePainter( theImage );
  thePainter.setRenderHint( QPainter::Antialiasing ); //make it look nicer
  mMapRenderer->render( &thePainter );

  QgsMapLayerRegistry::instance()->mapLayers().clear();
  return theImage;
}

int QgsWMSServer::getFeatureInfo( QDomDocument& result )
{
  if ( !mMapRenderer || !mConfigParser )
  {
    return 1;
  }

  result.clear();
  QStringList layersList, stylesList;
  bool conversionSuccess;

  for ( std::map<QString, QString>::iterator it = mParameterMap.begin(); it != mParameterMap.end(); ++it )
  {
    QgsMapServerLogger::instance()->printMessage( it->first + "//" + it->second );
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

  //find out the current scale denominater and set it to the SLD parser
  QgsScaleCalculator scaleCalc(( outputImage->logicalDpiX() + outputImage->logicalDpiY() ) / 2 , mMapRenderer->destinationSrs().mapUnits() );
  QgsRectangle mapExtent = mMapRenderer->extent();
  mConfigParser->setScaleDenominator( scaleCalc.calculate( mapExtent, outputImage->width() ) );
  delete outputImage; //no longer needed for feature info

  //read FEATURE_COUNT
  int featureCount = 1;
  std::map<QString, QString>::const_iterator feature_count_it = mParameterMap.find( "FEATURE_COUNT" );
  if ( feature_count_it != mParameterMap.end() )
  {
    featureCount = feature_count_it->second.toInt( &conversionSuccess );
    if ( !conversionSuccess )
    {
      featureCount = 1;
    }
  }

  //read QUERY_LAYERS
  std::map<QString, QString>::const_iterator query_layers_it = mParameterMap.find( "QUERY_LAYERS" );
  if ( query_layers_it == mParameterMap.end() )
  {
    return 3;
  }
  QStringList queryLayerList = query_layers_it->second.split( ",", QString::SkipEmptyParts );
  if ( queryLayerList.size() < 1 )
  {
    return 4;
  }

  //read I,J resp. X,Y
  QString iString, jString;
  int i = -1;
  int j = -1;

  std::map<QString, QString>::const_iterator i_it = mParameterMap.find( "I" );
  if ( i_it == mParameterMap.end() )
  {
    std::map<QString, QString>::const_iterator x_it = mParameterMap.find( "X" );
    if ( x_it == mParameterMap.end() )
    {
      return 5;
    }
    iString = x_it->second;
  }
  else
  {
    iString = i_it->second;
  }
  i = iString.toInt( &conversionSuccess );
  if ( !conversionSuccess )
  {
    return 6;
  }

  std::map<QString, QString>::const_iterator j_it = mParameterMap.find( "J" );
  if ( j_it == mParameterMap.end() )
  {
    std::map<QString, QString>::const_iterator y_it = mParameterMap.find( "Y" );
    if ( y_it == mParameterMap.end() )
    {
      return 7;
    }
    jString = y_it->second;
  }
  else
  {
    jString = j_it->second;
  }
  j = jString.toInt( &conversionSuccess );
  if ( !conversionSuccess )
  {
    return 8;
  }

  QDomElement getFeatureInfoElement = result.createElement( "GetFeatureInfoResponse" );
  result.appendChild( getFeatureInfoElement );

  QStringList nonIdentifiableLayers = mConfigParser->identifyDisabledLayers();
  QMap< QString, QMap< int, QString > > aliasInfo = mConfigParser->layerAliasInfo();
  QMap< QString, QSet<QString> > hiddenAttributes = mConfigParser->hiddenAttributes();

  QList<QgsMapLayer*> layerList;
  QgsMapLayer* currentLayer = 0;
  QgsPoint infoPoint; //info point in coordinates of the layer
  QStringList::const_iterator layerIt;
  for ( layerIt = queryLayerList.constBegin(); layerIt != queryLayerList.constEnd(); ++layerIt )
  {
    //create maplayer from sld parser
    layerList = mConfigParser->mapLayerFromStyle( *layerIt, "" );
    currentLayer = layerList.at( 0 );
    if ( !currentLayer || nonIdentifiableLayers.contains( currentLayer->id() ) )
    {
      continue;
    }
    if ( infoPointToLayerCoordinates( i, j, infoPoint, mMapRenderer, currentLayer ) != 0 )
    {
      continue;
    }
    QgsMSDebugMsg( "Info point in layer crs: " + QString::number( infoPoint.x() ) + "//" + QString::number( infoPoint.y() ) );

    QDomElement layerElement = result.createElement( "Layer" );
    layerElement.setAttribute( "name", currentLayer->name() );
    getFeatureInfoElement.appendChild( layerElement );

    //switch depending on vector or raster
    QgsVectorLayer* vectorLayer = dynamic_cast<QgsVectorLayer*>( currentLayer );
    if ( vectorLayer )
    {
      //is there alias info for this vector layer?
      QMap< int, QString > layerAliasInfo;
      QMap< QString, QMap< int, QString > >::const_iterator aliasIt = aliasInfo.find( currentLayer->id() );
      if ( aliasIt != aliasInfo.constEnd() )
      {
        layerAliasInfo = aliasIt.value();
      }

      //hidden attributes for this layer
      QSet<QString> layerHiddenAttributes;
      QMap< QString, QSet<QString> >::const_iterator hiddenIt = hiddenAttributes.find( currentLayer->id() );
      if ( hiddenIt != hiddenAttributes.constEnd() )
      {
        layerHiddenAttributes = hiddenIt.value();
      }

      if ( featureInfoFromVectorLayer( vectorLayer, infoPoint, featureCount, result, layerElement, mMapRenderer, layerAliasInfo, layerHiddenAttributes ) != 0 )
      {
        return 9;
      }
    }
    else //raster layer
    {
      QgsRasterLayer* rasterLayer = dynamic_cast<QgsRasterLayer*>( currentLayer );
      if ( rasterLayer )
      {
        if ( featureInfoFromRasterLayer( rasterLayer, infoPoint, result, layerElement ) != 0 )
        {
          return 10;
        }
      }
      else
      {
        return 11;
      }
    }
  }
  return 0;
}

QImage* QgsWMSServer::initializeRendering( QStringList& layersList, QStringList& stylesList, QStringList& layerIdList )
{
  if ( !mConfigParser )
  {
    QgsMSDebugMsg( "Error: mSLDParser is 0" );
    return 0;
  }

  if ( !mMapRenderer )
  {
    QgsMSDebugMsg( "Error: mMapRenderer is 0" );
    return 0;
  }

  if ( readLayersAndStyles( layersList, stylesList ) != 0 )
  {
    QgsMSDebugMsg( "error reading layers and styles" );
    return 0;
  }

  if ( initializeSLDParser( layersList, stylesList ) != 0 )
  {
    return 0;
  }

  //pass external GML to the SLD parser.
  std::map<QString, QString>::const_iterator gmlIt = mParameterMap.find( "GML" );
  if ( gmlIt != mParameterMap.end() )
  {
    QDomDocument* gmlDoc = new QDomDocument();
    if ( gmlDoc->setContent( gmlIt->second, true ) )
    {
      QString layerName = gmlDoc->documentElement().attribute( "layerName" );
      QgsMSDebugMsg( "Adding entry with key: " + layerName + " to external GML data" );
      mConfigParser->addExternalGMLData( layerName, gmlDoc );
    }
    else
    {
      QgsMSDebugMsg( "Error, could not add external GML to QgsSLDParser" );
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
  mMapRenderer->setLabelingEngine( new QgsPalLabeling() );

  //find out the current scale denominater and set it to the SLD parser
  QgsScaleCalculator scaleCalc(( theImage->logicalDpiX() + theImage->logicalDpiY() ) / 2 , mMapRenderer->destinationSrs().mapUnits() );
  QgsRectangle mapExtent = mMapRenderer->extent();
  mConfigParser->setScaleDenominator( scaleCalc.calculate( mapExtent, theImage->width() ) );

  //create objects for qgis rendering
  QStringList theLayers = layerSet( layersList, stylesList, mMapRenderer->destinationSrs() );
  mMapRenderer->setLayerSet( theLayers );
  return theImage;
}

QImage* QgsWMSServer::createImage( int width, int height ) const
{
  bool conversionSuccess;

  if ( width < 0 )
  {
    std::map<QString, QString>::const_iterator wit = mParameterMap.find( "WIDTH" );
    if ( wit == mParameterMap.end() )
    {
      width = 0; //width parameter is mandatory
    }
    else
    {
      width = wit->second.toInt( &conversionSuccess );
      if ( !conversionSuccess )
      {
        width = 0;
      }
    }
  }

  if ( height < 0 )
  {
    std::map<QString, QString>::const_iterator hit = mParameterMap.find( "HEIGHT" );
    if ( hit == mParameterMap.end() )
    {
      height = 0; //height parameter is mandatory
    }
    else
    {
      height = hit->second.toInt( &conversionSuccess );
      if ( !conversionSuccess )
      {
        height = 0;
      }
    }
  }

  if ( width < 0 || height < 0 )
  {
    return 0;
  }

  QImage* theImage = 0;

  //is format jpeg?
  bool jpeg = false;
  std::map<QString, QString>::const_iterator formatIt = mParameterMap.find( "FORMAT" );
  if ( formatIt != mParameterMap.end() )
  {
    if ( formatIt->second.compare( "jpg", Qt::CaseInsensitive ) == 0
         || formatIt->second.compare( "jpeg", Qt::CaseInsensitive ) == 0
         || formatIt->second.compare( "image/jpeg", Qt::CaseInsensitive ) == 0 )
    {
      jpeg = true;
    }
  }

  //transparent parameter
  bool transparent = false;
  std::map<QString, QString>::const_iterator transparentIt = mParameterMap.find( "TRANSPARENT" );
  if ( transparentIt != mParameterMap.end() && transparentIt->second.compare( "true", Qt::CaseInsensitive ) == 0 )
  {
    transparent = true;
  }

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
  std::map<QString, QString>::const_iterator dit = mParameterMap.find( "DPI" );
  if ( dit != mParameterMap.end() )
  {
    int dpi = dit->second.toInt( &conversionSuccess );
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
  std::map<QString, QString>::const_iterator bbIt = mParameterMap.find( "BBOX" );
  if ( bbIt == mParameterMap.end() )
  {
    //GetPrint request is allowed to have missing BBOX parameter
    minx = 0; miny = 0; maxx = 0; maxy = 0;
  }
  else
  {
    bool bboxOk = true;
    QString bbString = bbIt->second;
    minx = bbString.section( ",", 0, 0 ).toDouble( &conversionSuccess );
    if ( !conversionSuccess ) {bboxOk = false;}
    miny = bbString.section( ",", 1, 1 ).toDouble( &conversionSuccess );
    if ( !conversionSuccess ) {bboxOk = false;}
    maxx = bbString.section( ",", 2, 2 ).toDouble( &conversionSuccess );
    if ( !conversionSuccess ) {bboxOk = false;}
    maxy = bbString.section( ",", 3, 3 ).toDouble( &conversionSuccess );
    if ( !conversionSuccess ) {bboxOk = false;}

    if ( !bboxOk )
    {
      //throw a service exception
      throw QgsMapServiceException( "InvalidParameterValue", "Invalid BBOX parameter" );
    }
  }

  QGis::UnitType mapUnits = QGis::Degrees;

  std::map<QString, QString>::const_iterator crsIt = mParameterMap.find( "CRS" );
  if ( crsIt == mParameterMap.end() )
  {
    crsIt = mParameterMap.find( "SRS" ); //some clients (e.g. QGIS) call it SRS
  }

  QgsCoordinateReferenceSystem outputCRS;

  //wms spec says that CRS parameter is mandatory.
  //we don't rejeict the request if it is not there but disable reprojection on the fly
  if ( crsIt == mParameterMap.end() )
  {
    //disable on the fly projection
    QgsProject::instance()->writeEntry( "SpatialRefSys", "/ProjectionsEnabled", 0 );
  }
  else
  {
    //enable on the fly projection
    QgsMSDebugMsg( "enable on the fly projection" );
    QgsProject::instance()->writeEntry( "SpatialRefSys", "/ProjectionsEnabled", 1 );

    QString crsString = crsIt->second;
    if ( !( crsString.left( 4 ) == "EPSG" ) )
    {
      return 3; // only EPSG ids supported
    }
    long epsgId = crsString.section( ":", 1, 1 ).toLong( &conversionSuccess );
    if ( !conversionSuccess )
    {
      return 4;
    }

    //destination SRS
    outputCRS = QgsEPSGCache::instance()->searchCRS( epsgId );
    if ( !outputCRS.isValid() )
    {
      QgsMSDebugMsg( "Error, could not create output CRS from EPSG" );
      throw QgsMapServiceException( "InvalidCRS", "Could not create output CRS" );
      return 5;
    }
    mMapRenderer->setDestinationSrs( outputCRS );
    mMapRenderer->setProjectionsEnabled( true );
    mapUnits = outputCRS.mapUnits();
  }
  mMapRenderer->setMapUnits( mapUnits );

  //Change x- and y- of BBOX for WMS 1.3.0 and geographic coordinate systems
  std::map<QString, QString>::const_iterator versionIt = mParameterMap.find( "VERSION" );
  if ( versionIt != mParameterMap.end() )
  {
    QString version = versionIt->second;
    if ( version == "1.3.0" && outputCRS.geographicFlag() )
    {
      //switch coordinates of extent
      double tmp;
      tmp = minx;
      minx = miny; miny = tmp;
      tmp = maxx;
      maxx = maxy; maxy = tmp;
    }
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
  //get layer and style lists from the parameters
  layersList.clear();
  stylesList.clear();
  std::map<QString, QString>::const_iterator layersIt = mParameterMap.find( "LAYERS" );
  if ( layersIt != mParameterMap.end() )
  {
    layersList = layersIt->second.split( "," );
  }

  std::map<QString, QString>::const_iterator stylesIt = mParameterMap.find( "STYLES" );
  if ( stylesIt != mParameterMap.end() )
  {
    stylesList = stylesIt->second.split( "," );
  }
  return 0;
}

int QgsWMSServer::initializeSLDParser( QStringList& layersList, QStringList& stylesList )
{
  std::map<QString, QString>::const_iterator sldIt = mParameterMap.find( "SLD" );
  if ( sldIt != mParameterMap.end() )
  {
    //ignore LAYERS and STYLES and take those information from the SLD
    QString xml = sldIt->second;
    QDomDocument* theDocument = new QDomDocument( "user.sld" );
    QString errorMsg;
    int errorLine, errorColumn;

    if ( !theDocument->setContent( xml, true, &errorMsg, &errorLine, &errorColumn ) )
    {
      //std::cout << xml.toAscii().data() << std::endl;
      QgsMapServerLogger::instance()->printMessage( "Error, could not create DomDocument from SLD" );
      QgsMapServerLogger::instance()->printMessage( "The error message is: " + errorMsg );
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
      QgsMSDebugMsg( "Error, no layers and styles found in SLD" );
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

int QgsWMSServer::infoPointToLayerCoordinates( int i, int j, QgsPoint& layerCoords, QgsMapRenderer* mapRender,
    QgsMapLayer* layer ) const
{
  if ( !mapRender || !layer || !mapRender->coordinateTransform() )
  {
    return 1;
  }

  //first transform i,j to map output coordinates
  QgsPoint mapPoint = mapRender->coordinateTransform()->toMapCoordinates( i, j );
  //and then to layer coordinates
  layerCoords = mapRender->mapToLayerCoordinates( layer, mapPoint );
  return 0;
}

int QgsWMSServer::featureInfoFromVectorLayer( QgsVectorLayer* layer,
    const QgsPoint& infoPoint,
    int nFeatures,
    QDomDocument& infoDocument,
    QDomElement& layerElement,
    QgsMapRenderer* mapRender,
    QMap<int, QString>& aliasMap,
    QSet<QString>& hiddenAttributes ) const
{
  if ( !layer || !mapRender )
  {
    return 1;
  }

  //we need a selection rect (0.01 of map width)
  QgsRectangle mapRect = mapRender->extent();
  QgsRectangle layerRect = mapRender->mapToLayerCoordinates( layer, mapRect );
  double searchRadius = ( layerRect.xMaximum() - layerRect.xMinimum() ) / 200;
  QgsRectangle searchRect( infoPoint.x() - searchRadius, infoPoint.y() - searchRadius,
                           infoPoint.x() + searchRadius, infoPoint.y() + searchRadius );

  //do a select with searchRect and go through all the features
  QgsVectorDataProvider* provider = layer->dataProvider();
  if ( !provider )
  {
    return 2;
  }

  QgsFeature feature;
  QgsAttributeMap featureAttributes;
  int featureCounter = 0;
  const QgsFieldMap& fields = provider->fields();

  provider->select( provider->attributeIndexes(), searchRect, true, true );
  while ( provider->nextFeature( feature ) )
  {
    if ( featureCounter > nFeatures )
    {
      break;
    }

    QDomElement featureElement = infoDocument.createElement( "Feature" );
    featureElement.setAttribute( "id", QString::number( feature.id() ) );
    layerElement.appendChild( featureElement );

    //read all attribute values from the feature
    featureAttributes = feature.attributeMap();
    for ( QgsAttributeMap::const_iterator it = featureAttributes.begin(); it != featureAttributes.end(); ++it )
    {

      QString attributeName = fields[it.key()].name();
      //skip attribute if it has edit type 'hidden'
      if ( hiddenAttributes.contains( attributeName ) )
      {
        continue;
      }

      //check if the attribute name should be replaced with an alias
      QMap<int, QString>::const_iterator aliasIt = aliasMap.find( it.key() );
      if ( aliasIt != aliasMap.constEnd() )
      {
        attributeName = aliasIt.value();
      }

      QDomElement attributeElement = infoDocument.createElement( "Attribute" );
      attributeElement.setAttribute( "name", attributeName );
      attributeElement.setAttribute( "value", it->toString() );
      featureElement.appendChild( attributeElement );
    }

    //also append the wkt geometry as an attribute
    QgsGeometry* geom = feature.geometry();
    if ( geom )
    {
      QDomElement geometryElement = infoDocument.createElement( "Attribute" );
      geometryElement.setAttribute( "name", "geometry" );
      geometryElement.setAttribute( "value", geom->exportToWkt() );
      geometryElement.setAttribute( "type", "derived" );
      featureElement.appendChild( geometryElement );
    }
  }


  return 0;
}

int QgsWMSServer::featureInfoFromRasterLayer( QgsRasterLayer* layer,
    const QgsPoint& infoPoint,
    QDomDocument& infoDocument,
    QDomElement& layerElement ) const
{
  if ( !layer )
  {
    return 1;
  }

  QMap<QString, QString> attributes;
  layer->identify( infoPoint, attributes );

  for ( QMap<QString, QString>::const_iterator it = attributes.constBegin(); it != attributes.constEnd(); ++it )
  {
    QDomElement attributeElement = infoDocument.createElement( "Attribute" );
    attributeElement.setAttribute( "name", it.key() );
    attributeElement.setAttribute( "value", it.value() );
    layerElement.appendChild( attributeElement );
  }
  return 0;
}

QStringList QgsWMSServer::layerSet( const QStringList& layersList,
                                    const QStringList& stylesList,
                                    const QgsCoordinateReferenceSystem& destCRS ) const
{
  QStringList layerKeys;
  QStringList::const_iterator llstIt;
  QStringList::const_iterator slstIt;
  QgsMapLayer* theMapLayer = 0;

  for ( llstIt = layersList.begin(), slstIt = stylesList.begin(); llstIt != layersList.end(); ++llstIt )
  {
    QString styleName;
    if ( slstIt != stylesList.end() )
    {
      styleName = *slstIt;
    }
    QgsMSDebugMsg( "Trying to get layer " + *llstIt + "//" + styleName );

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
        if ( theMapLayer->type() == QgsMapLayer::RasterLayer )
        {
          //set the sourceSRS to the same as the destSRS
          theMapLayer->setCrs( destCRS );
        }
        layerKeys.push_front( theMapLayer->id() );
        QgsMapLayerRegistry::instance()->addMapLayer( theMapLayer, false );
      }
      else
      {
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
                                        const QFont& itemFont, double boxSpace, double layerSpace, double symbolSpace,
                                        double iconLabelSpace, double symbolWidth, double symbolHeight, double fontOversamplingFactor,
                                        double dpi ) const
{
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

  currentY += layerSpace;

  int opacity = 0;
  QgsMapLayer* layerInstance = QgsMapLayerRegistry::instance()->mapLayer( item->layerID() );
  if ( layerInstance )
  {
    opacity = layerInstance->getTransparency(); //maplayer says transparency but means opacity
  }

  //then draw all the children
  if ( p )
  {
    p->setFont( itemFont );
  }
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
      case QgsComposerLegendItem::SymbologyItem:
        drawLegendSymbol( currentComposerItem, p, boxSpace, currentY, currentSymbolWidth, currentSymbolHeight, opacity, dpi, symbolDownShift );
        break;
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
    }

    //finally draw text
    currentTextWidth = itemFontMetrics.width( currentComposerItem->text() ) / fontOversamplingFactor;
    double symbolItemHeight = qMax( itemFontMetrics.ascent() / fontOversamplingFactor, currentSymbolHeight );

    if ( p )
    {
      p->save();
      p->scale( 1.0 / fontOversamplingFactor, 1.0 / fontOversamplingFactor );
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
    currentY += symbolSpace;
  }
}

void QgsWMSServer::drawLegendSymbol( QgsComposerLegendItem* item, QPainter* p, double boxSpace, double currentY, double& symbolWidth, double& symbolHeight, double layerOpacity,
                                     double dpi, double yDownShift ) const
{
  QgsComposerSymbolItem* symbolItem = dynamic_cast< QgsComposerSymbolItem* >( item );
  if ( !symbolItem )
  {
    return;
  }

  QgsSymbol* symbol = symbolItem->symbol();
  if ( !symbol )
  {
    return;
  }

  QGis::GeometryType symbolType = symbol->type();
  switch ( symbolType )
  {
    case QGis::Point:
      drawPointSymbol( p, symbol, boxSpace, currentY, symbolWidth, symbolHeight, layerOpacity, dpi );
      break;
    case QGis::Line:
      drawLineSymbol( p, symbol, boxSpace, currentY, symbolWidth, symbolHeight, layerOpacity, yDownShift );
      break;
    case QGis::Polygon:
      drawPolygonSymbol( p, symbol, boxSpace, currentY, symbolWidth, symbolHeight, layerOpacity, yDownShift );
      break;
    case QGis::UnknownGeometry:
    case QGis::NoGeometry:
      // shouldn't occur
      break;
  }
}

void QgsWMSServer::drawPointSymbol( QPainter* p, QgsSymbol* s, double boxSpace, double currentY, double& symbolWidth, double& symbolHeight, double layerOpacity, double dpi ) const
{
  if ( !s )
  {
    return;
  }

  QImage pointImage;

  //width scale is 1.0
  pointImage = s->getPointSymbolAsImage( dpi / 25.4, false, Qt::yellow, 1.0, 0.0, 1.0, layerOpacity / 255.0 );

  if ( p )
  {
    QPointF imageTopLeft( boxSpace, currentY );
    p->drawImage( imageTopLeft, pointImage );
  }

  symbolWidth = pointImage.width();
  symbolHeight = pointImage.height();
}

void QgsWMSServer::drawPolygonSymbol( QPainter* p, QgsSymbol* s, double boxSpace, double currentY, double symbolWidth, double symbolHeight, double layerOpacity, double yDownShift ) const
{
  if ( !s || !p )
  {
    return;
  }

  p->save();

  //brush
  QBrush symbolBrush = s->brush();
  QColor brushColor = symbolBrush.color();
  brushColor.setAlpha( layerOpacity );
  symbolBrush.setColor( brushColor );
  p->setBrush( symbolBrush );

  //pen
  QPen symbolPen = s->pen();
  QColor penColor = symbolPen.color();
  penColor.setAlpha( layerOpacity );
  symbolPen.setColor( penColor );
  p->setPen( symbolPen );

  p->drawRect( QRectF( boxSpace, currentY + yDownShift, symbolWidth, symbolHeight ) );

  p->restore();
}

void QgsWMSServer::drawLineSymbol( QPainter* p, QgsSymbol* s, double boxSpace, double currentY, double symbolWidth, double symbolHeight, double layerOpacity, double yDownShift ) const
{
  if ( !s || !p )
  {
    return;
  }

  p->save();
  QPen symbolPen = s->pen();
  QColor penColor = symbolPen.color();
  penColor.setAlpha( layerOpacity );
  symbolPen.setColor( penColor );
  symbolPen.setCapStyle( Qt::FlatCap );
  p->setPen( symbolPen );
  double lineY = currentY + symbolHeight / 2.0 + symbolPen.widthF() / 2.0 + yDownShift;
  p->drawLine( QPointF( boxSpace, lineY ), QPointF( boxSpace + symbolWidth, lineY ) );
  p->restore();
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

  double rasterScaleFactor = dpi / 2.0 / 25.4;

  if ( p )
  {
    p->save();
    p->translate( boxSpace, currentY + yDownShift );
    p->scale( 1.0 / rasterScaleFactor, 1.0 / rasterScaleFactor );
    symbol->drawPreviewIcon( p, QSize( symbolWidth * rasterScaleFactor, symbolHeight * rasterScaleFactor ) );
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

  QPen savedPen = p->pen();
  p->setPen( QPen( Qt::NoPen ) );

  QgsRasterLayer::DrawingStyle drawingStyle = layer->drawingStyle();
  if ( drawingStyle == QgsRasterLayer::SingleBandGray
       || drawingStyle == QgsRasterLayer::PalettedSingleBandGray
       || drawingStyle == QgsRasterLayer::MultiBandSingleGandGray )
  {
    int grayValue = 0;
    for ( int i = 0; i < symbolWidth; ++i )
    {
      grayValue = 255.0 * ( i / symbolWidth );
      p->setBrush( QColor( grayValue, grayValue, grayValue ) );
      p->drawRect( QRectF( boxSpace + i, currentY + yDownShift, 1, symbolHeight ) );
    }
  }
  else
  {
    //red/green/blue
    p->setBrush( Qt::red );
    p->drawRect( QRectF( boxSpace, currentY + yDownShift, symbolWidth / 3.0, symbolHeight ) );
    p->setBrush( Qt::green );
    p->drawRect( QRectF( boxSpace + symbolWidth / 3.0, currentY + yDownShift, symbolWidth / 3.0, symbolHeight ) );
    p->setBrush( Qt::blue );
    p->drawRect( QRectF( boxSpace + symbolWidth - symbolWidth / 3.0, currentY + yDownShift, symbolWidth / 3.0, symbolHeight ) );
  }
  p->setPen( savedPen );
}
