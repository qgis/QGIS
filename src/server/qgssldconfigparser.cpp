/***************************************************************************
                              qgssldconfigparser.cpp
                              ----------------------
  begin                : March 28, 2014
  copyright            : (C) 2014 by Marco Hugentobler
  email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssldconfigparser.h"
#include "qgsapplication.h"
#include "qgsconfigparserutils.h"
#include "qgslabel.h"
#include "qgslabelattributes.h"
#include "qgslogger.h"
#include "qgsmapserviceexception.h"
#include "qgsrasterlayer.h"
#include "qgsrendererv2.h"
#include "qgssinglesymbolrendererv2.h"
#include "qgssymbolv2.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include <sqlite3.h>

//layer builders
#include "qgshostedrdsbuilder.h"
#include "qgshostedvdsbuilder.h"
#include "qgsremotedatasourcebuilder.h"
#include "qgsremoteowsbuilder.h"
#include "qgssentdatasourcebuilder.h"

//for contours
#include "gdal_alg.h"
#include "ogr_srs_api.h"
#include "ogr_api.h"

//for raster interpolation
#include "qgsinterpolationlayerbuilder.h"
#include "qgsgridfilewriter.h"
#include "qgsidwinterpolator.h"
#include "qgstininterpolator.h"

#include <QTemporaryFile>

#if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM >= 1800
#define TO8(x) (x).toUtf8().constData()
#else
#define TO8(x) (x).toLocal8Bit().constData()
#endif

QgsSLDConfigParser::QgsSLDConfigParser( QDomDocument* doc, const QMap<QString, QString>& parameters )
    : QgsWMSConfigParser()
    , mXMLDoc( doc )
    , mParameterMap( parameters )
    , mSLDNamespace( "http://www.opengis.net/sld" )
    , mOutputUnits( QgsMapRenderer::Pixels )
    , mFallbackParser( 0 )
{

  //set output units
  if ( mXMLDoc )
  {
    //first search attribute "units" in <StyledLayerDescriptor> element
    QDomElement sldElement = mXMLDoc->documentElement();
    if ( !sldElement.isNull() )
    {
      QString unitString = sldElement.attribute( "units" );
      if ( !unitString.isEmpty() )
      {
        if ( unitString == "mm" )
        {
          mOutputUnits = QgsMapRenderer::Millimeters;
        }
        else if ( unitString == "pixel" )
        {
          mOutputUnits = QgsMapRenderer::Pixels;
        }
      }
    }
  }
}

QgsSLDConfigParser::~QgsSLDConfigParser()
{
  delete mXMLDoc;
}

void QgsSLDConfigParser::layersAndStylesCapabilities( QDomElement& parentElement, QDomDocument& doc, const QString& version, bool fullProjectSettings ) const
{
  Q_UNUSED( version );
  Q_UNUSED( fullProjectSettings );

  //iterate over all <UserLayer> nodes
  if ( mXMLDoc )
  {
    QDomNode sldNode = mXMLDoc->documentElement();
    if ( !sldNode.isNull() )
    {
      //create wgs84 to reproject the layer bounding boxes
      //QgsCoordinateReferenceSystem wgs84;
      //wgs84.createFromEpsg(4326);

      QDomNodeList layerNodeList = sldNode.toElement().elementsByTagName( "UserLayer" );
      for ( int i = 0; i < layerNodeList.size(); ++i )
      {
        QDomElement layerElement = doc.createElement( "Layer" );
        layerElement.setAttribute( "queryable", "1" ); //support GetFeatureInfo for all layers
        parentElement.appendChild( layerElement );

        //add name
        QDomNodeList nameList = layerNodeList.item( i ).toElement().elementsByTagName( "Name" );
        if ( nameList.size() > 0 )
        {
          //layer name
          QDomElement layerNameElement = doc.createElement( "Name" );
          QDomText layerNameText = doc.createTextNode( nameList.item( 0 ).toElement().text() );
          layerNameElement.appendChild( layerNameText );
          layerElement.appendChild( layerNameElement );
        }

        //add title
        QDomNodeList titleList = layerNodeList.item( i ).toElement().elementsByTagName( "Title" );
        if ( titleList.size() > 0 )
        {
          QDomElement layerTitleElement = doc.createElement( "Title" );
          QDomText layerTitleText = doc.createTextNode( titleList.item( 0 ).toElement().text() );
          layerTitleElement.appendChild( layerTitleText );
          layerElement.appendChild( layerTitleElement );
        }
        //add abstract
        QDomNodeList abstractList = layerNodeList.item( i ).toElement().elementsByTagName( "Abstract" );
        if ( abstractList.size() > 0 )
        {
          QDomElement layerAbstractElement = doc.createElement( "Abstract" );
          QDomText layerAbstractText = doc.createTextNode( abstractList.item( 0 ).toElement().text() );
          layerAbstractElement.appendChild( layerAbstractText );
          layerElement.appendChild( layerAbstractElement );
        }


        //get QgsMapLayer object to add Ex_GeographicalBoundingBox, Bounding Box
        QList<QgsMapLayer*> layerList = mapLayerFromStyle( nameList.item( 0 ).toElement().text(), "" );
        if ( layerList.size() < 1 )//error while generating the layer
        {
          QgsDebugMsg( "Error, no maplayer in layer list" );
          continue;
        }

        //get only the first layer since we don't want to have the other ones in the capabilities document
        QgsMapLayer* theMapLayer = layerList.at( 0 );
        if ( !theMapLayer )//error while generating the layer
        {
          QgsDebugMsg( "Error, QgsMapLayer object is 0" );
          continue;
        }

        //append geographic bbox and the CRS elements
        QStringList crsNumbers = QgsConfigParserUtils::createCRSListForLayer( theMapLayer );
        QStringList crsRestriction; //no crs restrictions in SLD parser
        QgsConfigParserUtils::appendCRSElementsToLayer( layerElement, doc, crsNumbers, crsRestriction );
        QgsConfigParserUtils::appendLayerBoundingBoxes( layerElement, doc, theMapLayer->extent(), theMapLayer->crs() );

        //iterate over all <UserStyle> nodes within a user layer
        QDomNodeList userStyleList = layerNodeList.item( i ).toElement().elementsByTagName( "UserStyle" );
        for ( int j = 0; j < userStyleList.size(); ++j )
        {
          QDomElement styleElement = doc.createElement( "Style" );
          layerElement.appendChild( styleElement );
          //Name
          QDomNodeList nameList = userStyleList.item( j ).toElement().elementsByTagName( "Name" );
          if ( nameList.size() > 0 )
          {
            QDomElement styleNameElement = doc.createElement( "Name" );
            QDomText styleNameText = doc.createTextNode( nameList.item( 0 ).toElement().text() );
            styleNameElement.appendChild( styleNameText );
            styleElement.appendChild( styleNameElement );

            QDomElement styleTitleElement = doc.createElement( "Title" );
            QDomText styleTitleText = doc.createTextNode( nameList.item( 0 ).toElement().text() );
            styleTitleElement.appendChild( styleTitleText );
            styleElement.appendChild( styleTitleElement );
          }
          //Title
          QDomNodeList titleList = userStyleList.item( j ).toElement().elementsByTagName( "Title" );
          if ( titleList.size() > 0 )
          {
            QDomElement styleTitleElement = doc.createElement( "Title" );
            QDomText styleTitleText = doc.createTextNode( titleList.item( 0 ).toElement().text() );
            styleTitleElement.appendChild( styleTitleText );
            styleElement.appendChild( styleTitleElement );
          }
          //Abstract
          QDomNodeList abstractList = userStyleList.item( j ).toElement().elementsByTagName( "Abstract" );
          if ( abstractList.size() > 0 )
          {
            QDomElement styleAbstractElement = doc.createElement( "Abstract" );
            QDomText styleAbstractText = doc.createTextNode( abstractList.item( 0 ).toElement().text() );
            styleAbstractElement.appendChild( styleAbstractText );
            styleElement.appendChild( styleAbstractElement );
          }
        }
      }
    }
  }
}

QList<QgsMapLayer*> QgsSLDConfigParser::mapLayerFromStyle( const QString& lName, const QString& styleName, bool useCache ) const
{
  QList<QgsMapLayer*> fallbackLayerList;
  QList<QgsMapLayer*> resultList;

  //first check if this layer is a named layer with a user style
  QList<QDomElement> namedLayerElemList = findNamedLayerElements( lName );
  for ( int i = 0; i < namedLayerElemList.size(); ++i )
  {
    QDomElement userStyleElement = findUserStyleElement( namedLayerElemList[i], styleName );
    if ( !userStyleElement.isNull() )
    {
      fallbackLayerList = mFallbackParser->mapLayerFromStyle( lName, "", false );
      if ( fallbackLayerList.size() > 0 )
      {
        QgsVectorLayer* v = dynamic_cast<QgsVectorLayer*>( fallbackLayerList.at( 0 ) );
        if ( v )
        {
          QgsFeatureRendererV2* r = rendererFromUserStyle( userStyleElement, v );
          v->setRendererV2( r );
          labelSettingsFromUserStyle( userStyleElement, v );

          resultList.push_back( v );
          return resultList;
        }
        else
        {
          QgsRasterLayer* r = dynamic_cast<QgsRasterLayer*>( fallbackLayerList.at( 0 ) ); //a raster layer?
          if ( r )
          {
            rasterSymbologyFromUserStyle( userStyleElement, r );

            //Using a contour symbolizer, there may be a raster and a vector layer
            QgsVectorLayer* v = contourLayerFromRaster( userStyleElement, r );
            if ( v )
            {
              resultList.push_back( v ); //contour layer must be added before raster
              mLayersToRemove.push_back( v ); //don't cache contour layers at the moment
            }
            resultList.push_back( r );
            return resultList;
          }
        }
      }
    }

    // try with a predefined named style
    QDomElement namedStyleElement = findNamedStyleElement( namedLayerElemList[i], styleName );
    if ( !namedStyleElement.isNull() )
    {
      fallbackLayerList = mFallbackParser->mapLayerFromStyle( lName, styleName, false );
      if ( fallbackLayerList.size() > 0 )
      {
        resultList << fallbackLayerList;
        return resultList;
      }
    }
  }

  QDomElement userLayerElement = findUserLayerElement( lName );

  if ( userLayerElement.isNull() )
  {
    //maybe named layer and named style is defined in the fallback SLD?
    if ( mFallbackParser )
    {
      resultList = mFallbackParser->mapLayerFromStyle( lName, styleName, useCache );
    }

    return resultList;
  }

  QDomElement userStyleElement = findUserStyleElement( userLayerElement, styleName );

  QgsMapLayer* theMapLayer = mapLayerFromUserLayer( userLayerElement, lName, useCache );
  if ( !theMapLayer )
  {
    return resultList;
  }

  QgsFeatureRendererV2* theRenderer = 0;

  QgsRasterLayer* theRasterLayer = dynamic_cast<QgsRasterLayer*>( theMapLayer );
  if ( theRasterLayer )
  {
    QgsDebugMsg( "Layer is a rasterLayer" );
    if ( !userStyleElement.isNull() )
    {
      QgsDebugMsg( "Trying to add raster symbology" );
      rasterSymbologyFromUserStyle( userStyleElement, theRasterLayer );
      //todo: possibility to have vector layer or raster layer
      QgsDebugMsg( "Trying to find contour symbolizer" );
      QgsVectorLayer* v = contourLayerFromRaster( userStyleElement, theRasterLayer );
      if ( v )
      {
        QgsDebugMsg( "Returning vector layer" );
        resultList.push_back( v );
        mLayersToRemove.push_back( v );
      }
    }
    resultList.push_back( theMapLayer );

    return resultList;
  }

  QgsVectorLayer* theVectorLayer = dynamic_cast<QgsVectorLayer*>( theMapLayer );
  if ( userStyleElement.isNull() )//apply a default style
  {
    QgsSymbolV2* symbol = QgsSymbolV2::defaultSymbol( theVectorLayer->geometryType() );
    theRenderer = new QgsSingleSymbolRendererV2( symbol );
  }
  else
  {
    QgsDebugMsg( "Trying to get a renderer from the user style" );
    theRenderer = rendererFromUserStyle( userStyleElement, theVectorLayer );
    //apply labels if <TextSymbolizer> tag is present
    labelSettingsFromUserStyle( userStyleElement, theVectorLayer );
  }

  if ( !theRenderer )
  {
    QgsDebugMsg( "Error, could not create a renderer" );
    delete theVectorLayer;
    return resultList;
  }
  theVectorLayer->setRendererV2( theRenderer );
  QgsDebugMsg( "Returning the vectorlayer" );
  resultList.push_back( theVectorLayer );
  return resultList;
}

int QgsSLDConfigParser::layersAndStyles( QStringList& layers, QStringList& styles ) const
{
  QgsDebugMsg( "Entering." );
  layers.clear();
  styles.clear();

  if ( mXMLDoc )
  {
    QDomElement sldElem = mXMLDoc->documentElement();
    if ( !sldElem.isNull() )
    {
      //go through all the children and search for <NamedLayers> and <UserLayers>
      QDomNodeList layerNodes = sldElem.childNodes();
      for ( int i = 0; i < layerNodes.size(); ++i )
      {
        QDomElement currentLayerElement = layerNodes.item( i ).toElement();
        if ( currentLayerElement.localName() == "NamedLayer" )
        {
          QgsDebugMsg( "Found a NamedLayer" );
          //layer name
          QDomNodeList nameList = currentLayerElement.elementsByTagName/*NS*/( /*mSLDNamespace,*/ "Name" );
          if ( nameList.length() < 1 )
          {
            continue; //a layer name is mandatory
          }
          QString layerName = nameList.item( 0 ).toElement().text();

          //find the Named Styles and the corresponding names
          QDomNodeList namedStyleList = currentLayerElement.elementsByTagName/*NS*/( /*mSLDNamespace,*/ "NamedStyle" );
          for ( int j = 0; j < namedStyleList.size(); ++j )
          {
            QDomNodeList styleNameList = namedStyleList.item( j ).toElement().elementsByTagName/*NS*/( /*mSLDNamespace,*/ "Name" );
            if ( styleNameList.size() < 1 )
            {
              continue; //a layer name is mandatory
            }
            QString styleName = styleNameList.item( 0 ).toElement().text();
            QgsDebugMsg( "styleName is: " + styleName );
            layers.push_back( layerName );
            styles.push_back( styleName );
          }

          //named layers can also have User Styles
          QDomNodeList userStyleList = currentLayerElement.elementsByTagName/*NS*/( /*mSLDNamespace,*/ "UserStyle" );
          for ( int j = 0; j < userStyleList.size(); ++j )
          {
            QDomNodeList styleNameList = userStyleList.item( j ).toElement().elementsByTagName/*NS*/( /*mSLDNamespace,*/ "Name" );
            if ( styleNameList.size() < 1 )
            {
              continue; //a layer name is mandatory
            }
            QString styleName = styleNameList.item( 0 ).toElement().text();
            QgsDebugMsg( "styleName is: " + styleName );
            layers.push_back( layerName );
            styles.push_back( styleName );
          }
        }
        else if ( currentLayerElement.localName() == "UserLayer" )
        {
          QgsDebugMsg( "Found a UserLayer" );
          //layer name
          QDomNodeList nameList = currentLayerElement.elementsByTagName/*NS*/( /*mSLDNamespace,*/ "Name" );
          if ( nameList.length() < 1 )
          {
            QgsDebugMsg( "Namelist size is <1" );
            continue; //a layer name is mandatory
          }
          QString layerName = nameList.item( 0 ).toElement().text();
          QgsDebugMsg( "layerName is: " + layerName );
          //find the User Styles and the corresponding names
          QDomNodeList userStyleList = currentLayerElement.elementsByTagName/*NS*/( /*mSLDNamespace,*/ "UserStyle" );
          for ( int j = 0; j < userStyleList.size(); ++j )
          {
            QDomNodeList styleNameList = userStyleList.item( j ).toElement().elementsByTagName/*NS*/( /*mSLDNamespace,*/ "Name" );
            if ( styleNameList.size() < 1 )
            {
              QgsDebugMsg( "Namelist size is <1" );
              continue;
            }

            QString styleName = styleNameList.item( 0 ).toElement().text();
            QgsDebugMsg( "styleName is: " + styleName );
            layers.push_back( layerName );
            styles.push_back( styleName );
          }
        }
      }
    }
  }
  else
  {
    return 1;
  }
  return 0;
}

QDomDocument QgsSLDConfigParser::getStyle( const QString& styleName, const QString& layerName ) const
{
  QDomElement userLayerElement = findUserLayerElement( layerName );

  if ( userLayerElement.isNull() )
  {
    throw QgsMapServiceException( "LayerNotDefined", "Operation request is for a Layer not offered by the server." );
  }

  QDomElement userStyleElement = findUserStyleElement( userLayerElement, styleName );

  if ( userStyleElement.isNull() )
  {
    throw QgsMapServiceException( "StyleNotDefined", "Operation request references a Style not offered by the server." );
  }

  QDomDocument styleDoc;
  styleDoc.appendChild( styleDoc.importNode( userStyleElement, true ) );
  return styleDoc;
}

QDomDocument QgsSLDConfigParser::getStyles( QStringList& layerList ) const
{
  QDomDocument styleDoc;
  for ( int i = 0; i < layerList.size(); i++ )
  {
    QString layerName;
    QString typeName;
    layerName = layerList.at( i );
    QDomElement userLayerElement = findUserLayerElement( layerName );
    if ( userLayerElement.isNull() )
    {
      throw QgsMapServiceException( "LayerNotDefined", "Operation request is for a Layer not offered by the server." );
    }
    QDomNodeList userStyleList = userLayerElement.elementsByTagName( "UserStyle" );
    for ( int j = 0; j < userStyleList.size(); j++ )
    {
      QDomElement userStyleElement = userStyleList.item( i ).toElement();
      styleDoc.appendChild( styleDoc.importNode( userStyleElement, true ) );
    }
  }
  return styleDoc;
}

QDomDocument QgsSLDConfigParser::describeLayer( QStringList& layerList, const QString& hrefString ) const
{
  if ( mFallbackParser )
  {
    return mFallbackParser->describeLayer( layerList, hrefString );
  }
  return QDomDocument();
}

QgsMapRenderer::OutputUnits QgsSLDConfigParser::outputUnits() const
{
  return mOutputUnits;
}

QStringList QgsSLDConfigParser::identifyDisabledLayers() const
{
  if ( mFallbackParser )
  {
    return mFallbackParser->identifyDisabledLayers();
  }
  return QStringList();
}

bool QgsSLDConfigParser::featureInfoWithWktGeometry() const
{
  if ( mFallbackParser )
  {
    return mFallbackParser->featureInfoWithWktGeometry();
  }
  return false;
}


QHash<QString, QString> QgsSLDConfigParser::featureInfoLayerAliasMap() const
{
  if ( mFallbackParser )
  {
    return mFallbackParser->featureInfoLayerAliasMap();
  }
  return QHash<QString, QString>();
}

QString QgsSLDConfigParser::featureInfoDocumentElement( const QString& defaultValue ) const
{
  return defaultValue;
}

QString QgsSLDConfigParser::featureInfoDocumentElementNS() const
{
  return QString();
}

QString QgsSLDConfigParser::featureInfoSchema() const
{
  return QString();
}

bool QgsSLDConfigParser::featureInfoFormatSIA2045() const
{
  if ( mFallbackParser )
  {
    return mFallbackParser->featureInfoFormatSIA2045();
  }
  return false;
}

void QgsSLDConfigParser::drawOverlays( QPainter* p, int dpi, int width, int height ) const
{
  if ( mFallbackParser )
  {
    mFallbackParser->drawOverlays( p, dpi, width, height );
  }
}

void QgsSLDConfigParser::loadLabelSettings( QgsLabelingEngineInterface * lbl ) const
{
  if ( mFallbackParser )
  {
    mFallbackParser->loadLabelSettings( lbl );
  }
}

QString QgsSLDConfigParser::serviceUrl() const
{
  if ( mFallbackParser )
  {
    return mFallbackParser->serviceUrl();
  }
  return QString();
}

QStringList QgsSLDConfigParser::wfsLayerNames() const
{
  if ( mFallbackParser )
  {
    return mFallbackParser->wfsLayerNames();
  }
  return QStringList();
}

void QgsSLDConfigParser::owsGeneralAndResourceList( QDomElement& parentElement, QDomDocument& doc, const QString& strHref ) const
{
  if ( mFallbackParser )
  {
    mFallbackParser->owsGeneralAndResourceList( parentElement, doc, strHref );
  }
}

double QgsSLDConfigParser::legendBoxSpace() const
{
  if ( mFallbackParser )
  {
    return mFallbackParser->legendBoxSpace();
  }
  return 0;
}

double QgsSLDConfigParser::legendLayerSpace() const
{
  if ( mFallbackParser )
  {
    return mFallbackParser->legendLayerSpace();
  }
  return 0;
}

double QgsSLDConfigParser::legendLayerTitleSpace() const
{
  if ( mFallbackParser )
  {
    return mFallbackParser->legendLayerTitleSpace();
  }
  return 0;
}

double QgsSLDConfigParser::legendSymbolSpace() const
{
  if ( mFallbackParser )
  {
    return mFallbackParser->legendSymbolSpace();
  }
  return 0;
}

double QgsSLDConfigParser::legendIconLabelSpace() const
{
  if ( mFallbackParser )
  {
    return mFallbackParser->legendIconLabelSpace();
  }
  return 0;
}

double QgsSLDConfigParser::legendSymbolWidth() const
{
  if ( mFallbackParser )
  {
    return mFallbackParser->legendSymbolWidth();
  }
  return 0;
}

double QgsSLDConfigParser::legendSymbolHeight() const
{
  if ( mFallbackParser )
  {
    return mFallbackParser->legendSymbolHeight();
  }
  return 0;
}

const QFont& QgsSLDConfigParser::legendLayerFont() const
{
  if ( mFallbackParser )
  {
    return mFallbackParser->legendLayerFont();
  }
  return mLegendLayerFont;
}

const QFont& QgsSLDConfigParser::legendItemFont() const
{
  if ( mFallbackParser )
  {
    return mFallbackParser->legendItemFont();
  }
  return mLegendItemFont;
}

double QgsSLDConfigParser::maxWidth() const
{
  if ( mFallbackParser )
  {
    return mFallbackParser->maxWidth();
  }
  return -1;
}

double QgsSLDConfigParser::maxHeight() const
{
  if ( mFallbackParser )
  {
    return mFallbackParser->maxHeight();
  }
  return -1;
}

double QgsSLDConfigParser::imageQuality() const
{
  if ( mFallbackParser )
  {
    return mFallbackParser->imageQuality();
  }
  return -1;
}

int QgsSLDConfigParser::WMSPrecision() const
{
  if ( mFallbackParser )
  {
    return mFallbackParser->WMSPrecision();
  }
  return -1;
}

QgsComposition* QgsSLDConfigParser::createPrintComposition( const QString& composerTemplate, QgsMapRenderer* mapRenderer, const QMap< QString, QString >& parameterMap ) const
{
  if ( mFallbackParser )
  {
    return mFallbackParser->createPrintComposition( composerTemplate, mapRenderer, parameterMap );
  }
  return 0;
}

QgsComposition* QgsSLDConfigParser::initComposition( const QString& composerTemplate, QgsMapRenderer* mapRenderer, QList< QgsComposerMap*>& mapList, QList< QgsComposerLegend* >& legendList, QList< QgsComposerLabel* >& labelList, QList<const QgsComposerHtml *>& htmlFrameList ) const
{
  if ( mFallbackParser )
  {
    return mFallbackParser->initComposition( composerTemplate, mapRenderer, mapList, legendList, labelList, htmlFrameList );
  }
  return 0;
}

void QgsSLDConfigParser::printCapabilities( QDomElement& parentElement, QDomDocument& doc ) const
{
  if ( mFallbackParser )
  {
    mFallbackParser->printCapabilities( parentElement, doc );
  }
}

void QgsSLDConfigParser::setScaleDenominator( double )
{
  //soon...
}

void QgsSLDConfigParser::addExternalGMLData( const QString &, QDomDocument * )
{
  //soon...
}

QList< QPair< QString, QgsLayerCoordinateTransform > > QgsSLDConfigParser::layerCoordinateTransforms() const
{
  return QList< QPair< QString, QgsLayerCoordinateTransform > >();
}

int QgsSLDConfigParser::nLayers() const
{
  if ( mXMLDoc )
  {
    QDomNode sldNode = mXMLDoc->documentElement();
    if ( !sldNode.isNull() )
    {
      QDomNodeList layerNodeList = sldNode.toElement().elementsByTagName( "UserLayer" );
      return layerNodeList.size();
    }
  }
  return 0;
}

void QgsSLDConfigParser::serviceCapabilities( QDomElement& parentElement, QDomDocument& doc ) const
{
  if ( mFallbackParser )
  {
    mFallbackParser->serviceCapabilities( parentElement, doc );
  }
  else
  {
    QgsConfigParserUtils::fallbackServiceCapabilities( parentElement, doc );
  }
}

QList<QDomElement> QgsSLDConfigParser::findNamedLayerElements( const QString& layerName ) const
{
  QList<QDomElement> resultList;
  if ( mXMLDoc )
  {
    QDomElement sldElement = mXMLDoc->documentElement();
    if ( !sldElement.isNull() )
    {
      QDomNodeList NamedLayerList = sldElement.elementsByTagName( "NamedLayer" );
      for ( int i = 0; i < NamedLayerList.size(); ++i )
      {
        QDomNodeList nameList = NamedLayerList.item( i ).toElement().elementsByTagName( "Name" );
        if ( nameList.size() > 0 )
        {
          if ( nameList.item( 0 ).toElement().text() == layerName )
          {
            resultList.push_back( NamedLayerList.item( i ).toElement() );
          }
        }
      }
    }
  }
  return resultList;
}

QDomElement QgsSLDConfigParser::findUserStyleElement( const QDomElement& userLayerElement, const QString& styleName ) const
{
  QDomElement defaultResult;
  if ( !userLayerElement.isNull() )
  {
    QDomNodeList userStyleList = userLayerElement.elementsByTagName( "UserStyle" );
    for ( int i = 0; i < userStyleList.size(); ++i )
    {
      QDomNodeList nameList = userStyleList.item( i ).toElement().elementsByTagName( "Name" );
      if ( nameList.size() > 0 )
      {
        if ( nameList.item( 0 ).toElement().text() == styleName )
        {
          return userStyleList.item( i ).toElement();
        }
      }
    }
  }
  return defaultResult;
}

QDomElement QgsSLDConfigParser::findNamedStyleElement( const QDomElement& layerElement, const QString& styleName ) const
{
  QDomElement defaultResult;
  if ( !layerElement.isNull() )
  {
    QDomNodeList styleList = layerElement.elementsByTagName( "NamedStyle" );
    for ( int i = 0; i < styleList.size(); ++i )
    {
      QDomNodeList nameList = styleList.item( i ).toElement().elementsByTagName( "Name" );
      if ( nameList.size() > 0 )
      {
        if ( nameList.item( 0 ).toElement().text() == styleName )
        {
          return styleList.item( i ).toElement();
        }
      }
    }
  }
  return defaultResult;
}

QgsFeatureRendererV2* QgsSLDConfigParser::rendererFromUserStyle( const QDomElement& userStyleElement, QgsVectorLayer* vec ) const
{
  if ( !vec || userStyleElement.isNull() )
  {
    return 0;
  }

  QgsDebugMsg( "Entering" );

  QString errorMessage;
  QgsFeatureRendererV2* renderer = QgsFeatureRendererV2::loadSld( userStyleElement.parentNode(), vec->geometryType(), errorMessage );
  if ( !renderer )
  {
    throw QgsMapServiceException( "SLD error", errorMessage );
  }
  return renderer;
}

bool QgsSLDConfigParser::rasterSymbologyFromUserStyle( const QDomElement& userStyleElement, QgsRasterLayer* r ) const
{
  return false;
#if 0 //needs to be fixed
  QgsDebugMsg( "Entering" );
  if ( !r )
  {
    return false;
  }

  //search raster symbolizer
  QDomNodeList rasterSymbolizerList = userStyleElement.elementsByTagName( "RasterSymbolizer" );
  if ( rasterSymbolizerList.size() < 1 )
  {
    return false;
  }

  QDomElement rasterSymbolizerElem = rasterSymbolizerList.at( 0 ).toElement();

  //search colormap and entries
  QDomNodeList colorMapList = rasterSymbolizerElem.elementsByTagName( "ColorMap" );
  if ( colorMapList.size() < 1 )
  {
    return false;
  }

  QDomElement colorMapElem = colorMapList.at( 0 ).toElement();

  //search for color map entries
  r->setColorShadingAlgorithm( QgsRasterLayer::ColorRampShader );
  QgsColorRampShader* myRasterShaderFunction = ( QgsColorRampShader* )r->rasterShader()->rasterShaderFunction();
  QList<QgsColorRampShader::ColorRampItem> colorRampItems;

  QDomNodeList colorMapEntryList = colorMapElem.elementsByTagName( "ColorMapEntry" );
  QDomElement currentColorMapEntryElem;
  bool conversion;
  int red, green, blue;

  for ( int i = 0; i < colorMapEntryList.size(); ++i )
  {
    currentColorMapEntryElem = colorMapEntryList.at( i ).toElement();
    QgsColorRampShader::ColorRampItem myNewColorRampItem;
    QString color = currentColorMapEntryElem.attribute( "color" );
    if ( color.length() != 7 ) //color string must be in the form #ffffff
    {
      continue;
    }
    red = color.mid( 1, 2 ).toInt( &conversion, 16 );
    if ( !conversion )
    {
      red = 0;
    }
    green = color.mid( 3, 2 ).toInt( &conversion, 16 );
    if ( !conversion )
    {
      green = 0;
    }
    blue = color.mid( 5, 2 ).toInt( &conversion, 16 );
    if ( !conversion )
    {
      blue = 0;
    }
    myNewColorRampItem.color = QColor( red, green, blue );
    QString value = currentColorMapEntryElem.attribute( "quantity" );
    myNewColorRampItem.value = value.toDouble();
    QgsDebugMsg( "Adding colormap entry" );
    colorRampItems.push_back( myNewColorRampItem );
  }

  myRasterShaderFunction->setColorRampItemList( colorRampItems );

  //linear interpolation or discrete classification
  QString interpolation = colorMapElem.attribute( "interpolation" );
  if ( interpolation == "linear" )
  {
    myRasterShaderFunction->setColorRampType( QgsColorRampShader::INTERPOLATED );
  }
  else if ( interpolation == "discrete" )
  {
    myRasterShaderFunction->setColorRampType( QgsColorRampShader::DISCRETE );
  }

  //QgsDebugMsg("Setting drawing style");
  r->setDrawingStyle( QgsRasterLayer::SingleBandPseudoColor );

  //set pseudo color mode
  return true;
#else
  Q_UNUSED( userStyleElement );
  Q_UNUSED( r );
#endif //0
}

bool QgsSLDConfigParser::labelSettingsFromUserStyle( const QDomElement& userStyleElement, QgsVectorLayer* vec ) const
{
  if ( userStyleElement.isNull() || !vec )
  {
    return false;
  }
  else
  {
    vec->enableLabels( false );
    QDomNodeList featureTypeList = userStyleElement.elementsByTagName( "FeatureTypeStyle" );
    if ( featureTypeList.size() > 0 )
    {
      //QGIS WMS server only supports one featureTypeStyle per layer
      QDomNodeList ruleNodeList = featureTypeList.item( 0 ).toElement().elementsByTagName( "Rule" );
      // if there are rule elements:
      if ( ruleNodeList.size() > 0 )
      {
        // rule element
        QDomElement ruleElement = ruleNodeList.item( ruleNodeList.size() - 1 ).toElement();
        //find <TextSymbolizer>.
        //Unfortunately, QGIS does not support having different labels for different classifications.
        //Therefore we take the last text symbolizer for all features
        QDomNodeList textSymbolizerList = ruleElement.elementsByTagName( "TextSymbolizer" );
        // if there are textSymbolizers
        if ( textSymbolizerList.size() > 0 )
        {
          int opacity = 255;
          int polyColorRed = 0;
          int polyColorGreen = 0;
          int polyColorBlue = 0;
          QString elemText;
          QString fontfamily = QString( "Helvetica" );
          QString fontstyle = QString( "Normal" );
          int fontsize = 14;
          QString fontweight = QString( "Normal" );
          QString fontunderline = QString( "Normal" );
          bool success = false;

          QDomElement textSymbolizerElement = textSymbolizerList.item( textSymbolizerList.size() - 1 ).toElement();
          // if there is a viable text textSymbolizerElement
          if ( !textSymbolizerElement.isNull() )
          {
            QgsLabelAttributes * myLabelAttributes = vec->label()->labelAttributes();
            //element <Label> contains the attribute name
            QDomNodeList labelNodeList = textSymbolizerElement.elementsByTagName( "Label" );
            // if a viable label element is provided
            if ( labelNodeList.size() > 0 )
            {
              QDomElement labelElement = labelNodeList.item( 0 ).toElement();
              //we need the text of an <ogc:PropertyName> element
              QDomNodeList propertyNameList = labelElement.elementsByTagName( "PropertyName" );
              if ( propertyNameList.size() > 0 )
              {
                vec->enableLabels( true );
                QDomElement propertyNameElement = propertyNameList.item( 0 ).toElement();
                QString labelAttribute = propertyNameElement.text();
                vec->label()->setLabelField( QgsLabel::Text, vec->dataProvider()->fieldNameIndex( labelAttribute ) );

                // Iterate through each of CssParameter from the sld:font, sld:fill, sld:halo
                QDomNodeList labelFontElementList = textSymbolizerElement.elementsByTagName( "Font" );
                QDomNodeList labelFillElementList = textSymbolizerElement.elementsByTagName( "Fill" );
                QDomNodeList labelBufferElementList = textSymbolizerElement.elementsByTagName( "Halo" );
                QDomNodeList labelPlacementElementList = textSymbolizerElement.elementsByTagName( "LabelPlacement" );
                // Iterate through sld:font
                if (( labelFontElementList.size() > 0 ) )
                {
                  if ( !labelFontElementList.item( 0 ).toElement().isNull() )
                  {
                    QDomNodeList cssNodes = labelFontElementList.item( 0 ).toElement().elementsByTagName( "CssParameter" );
                    QString cssName;
                    QDomElement currentElement;
                    QgsDebugMsg( "Number of Css Properties: " + QString::number( cssNodes.size() ) );
                    for ( int i = 0; i < cssNodes.size(); ++i )
                    {
                      currentElement = cssNodes.item( i ).toElement();
                      if ( currentElement.isNull() )
                      {
                        continue;
                      }
                      QString elemText = currentElement.text();

                      //switch depending on attribute 'name'
                      cssName = currentElement.attribute( "name", "not_found" );
                      QgsDebugMsg( "property " + QString::number( i ) + ": " + cssName  + " " + elemText );
                      if ( cssName != "not_found" )
                      {
                        if ( cssName == "font-family" )
                        {
                          QgsDebugMsg( cssName + " " + elemText );
                          fontfamily = elemText;
                        }
                        else if ( cssName == "font-style" )
                        {
                          QgsDebugMsg( cssName + " " + elemText );
                          fontstyle = elemText;
                        }
                        else if ( cssName == "font-size" )
                        {
                          QgsDebugMsg( cssName + " " + elemText );
                          success = false;
                          fontsize = elemText.toInt( &success );
                          if ( !success )
                          {
                            fontsize = 12;
                          }

                        }
                        else if ( cssName == "font-weight" )
                        {
                          QgsDebugMsg( cssName + " " + elemText );
                          fontweight = elemText;
                        }
                        else if ( cssName == "font-underline" )
                        {
                          QgsDebugMsg( cssName + " " + elemText );
                          fontunderline = elemText;
                        }
                      }

                    }
                  }
                }
                // Iterate through sld:fill
                if (( labelFillElementList.size() > 0 ) )
                {
                  if ( !labelFillElementList.item( 0 ).toElement().isNull() )
                  {
                    QDomNodeList cssNodes = labelFillElementList.item( 0 ).toElement().elementsByTagName( "CssParameter" );
                    QString cssName;
                    QDomElement currentElement;
                    QgsDebugMsg( "Number of Css Properties: " + QString::number( cssNodes.size() ) );
                    for ( int i = 0; i < cssNodes.size(); ++i )
                    {
                      currentElement = cssNodes.item( i ).toElement();
                      if ( currentElement.isNull() )
                      {
                        continue;
                      }
                      QString elemText = currentElement.text();

                      //switch depending on attribute 'name'
                      cssName = currentElement.attribute( "name", "not_found" );
                      QgsDebugMsg( "property " + QString::number( i ) + ": " + cssName  + " " + elemText );
                      if ( cssName != "not_found" )
                      {
                        if ( cssName == "fill" )
                        {
                          QgsDebugMsg( cssName + " " + elemText );
                          //accept input in the form of #ff0000
                          if ( elemText.length() == 7 )
                          {
                            bool success;
                            polyColorRed = elemText.mid( 1, 2 ).toInt( &success, 16 );
                            if ( !success )
                            {
                              polyColorRed = 0;
                            }
                            polyColorGreen = elemText.mid( 3, 2 ).toInt( &success, 16 );
                            if ( !success )
                            {
                              polyColorGreen = 0;
                            }
                            polyColorBlue = elemText.mid( 5, 2 ).toInt( &success, 16 );
                            if ( !success )
                            {
                              polyColorBlue = 0;
                            }
                          }
                        }
                        else if ( cssName == "fill-opacity" )
                        {
                          QgsDebugMsg( cssName + " " + elemText );
                          bool success;
                          double op = elemText.toDouble( &success );
                          if ( success )
                          {
                            if ( op > 1.0 )
                            {
                              opacity = 255;
                            }
                            else if ( op < 0.0 )
                            {
                              opacity = 0;
                            }
                            else
                            {
                              opacity = ( int )( 255 * op );
                            }
                          }
                        }
                      }

                    }
                  }
                }



                myLabelAttributes->setSize( fontsize, QgsLabelAttributes::PointUnits );
                myLabelAttributes->setFamily( fontfamily );
                myLabelAttributes->setColor( QColor( polyColorRed, polyColorGreen, polyColorBlue, opacity ) );
                if (( fontstyle == "italic" ) || ( fontstyle == "Italic" ) )
                {
                  myLabelAttributes->setItalic( true );
                }
                if (( fontweight == "bold" ) || ( fontweight == "Bold" ) )
                {
                  myLabelAttributes->setBold( true );
                }
                if (( fontunderline == "underline" ) || ( fontunderline == "Underline" ) )
                {
                  myLabelAttributes->setUnderline( true );
                }
                // set label buffer(sld:halo)

                if (( labelBufferElementList.size() > 0 ) )
                {
                  if ( !labelBufferElementList.item( 0 ).toElement().isNull() )
                  {
                    QDomNodeList cssNodes = labelBufferElementList.item( 0 ).toElement().elementsByTagName( "CssParameter" );
                    QString cssName;
                    QDomElement currentElement;
                    QgsDebugMsg( "Number of Css Properties: " + QString::number( cssNodes.size() ) );
                    for ( int i = 0; i < cssNodes.size(); ++i )
                    {
                      currentElement = cssNodes.item( i ).toElement();
                      if ( currentElement.isNull() )
                      {
                        continue;
                      }
                      QString elemText = currentElement.text();

                      //switch depending on attribute 'name'
                      cssName = currentElement.attribute( "name", "not_found" );
                      QgsDebugMsg( "property " + QString::number( i ) + ": " + cssName  + " " + elemText );
                      if ( cssName != "not_found" )
                      {
                        if ( cssName == "fill" )
                        {
                          QgsDebugMsg( cssName + " " + elemText );
                          //accept input in the form of #ff0000
                          if ( elemText.length() == 7 )
                          {
                            bool success;
                            polyColorRed = elemText.mid( 1, 2 ).toInt( &success, 16 );
                            if ( !success )
                            {
                              polyColorRed = 255;
                            }
                            polyColorGreen = elemText.mid( 3, 2 ).toInt( &success, 16 );
                            if ( !success )
                            {
                              polyColorGreen = 255;
                            }
                            polyColorBlue = elemText.mid( 5, 2 ).toInt( &success, 16 );
                            if ( !success )
                            {
                              polyColorBlue = 255;
                            }
                          }
                        }
                        else if ( cssName == "fill-opacity" )
                        {
                          QgsDebugMsg( cssName + " " + elemText );
                          bool success;
                          double op = elemText.toDouble( &success );
                          if ( success )
                          {
                            if ( op > 1.0 )
                            {
                              opacity = 255;
                            }
                            else if ( op < 0.0 )
                            {
                              opacity = 0;
                            }
                            else
                            {
                              opacity = ( int )( 255 * op );
                            }
                          }
                        }
                      }
                    }

                    //QgsMapServerLogger::instance()->printMessage("radius " + QString::number(radius));
                    myLabelAttributes->setBufferEnabled( true );
                    myLabelAttributes->setBufferColor( QColor( polyColorRed, polyColorGreen, polyColorBlue, opacity ) );

#if 0
                    double radius = 5.0;
                    QDomElement radiusElement = labelBufferElementList.item( 0 ).toElement().elementsByTagName( "Radius" ).item( 0 ).toElement();
                    if ( !radiusElement.isNull() )
                    {
                      bool success = false;
                      radius = radiusElement.text().toDouble( &success );
                      if ( !success )
                      {
                        radius = 5.0;
                      }
                    }
                    myLabelAttributes->setBufferSize( radius, QgsLabelAttributes::PointUnits );
#endif

                    // ******** BUG ************  see why setting buffersize dows not work (is a problem in QGIS vector layer rendering)

                  }
                }

                // label placement
                if (( labelPlacementElementList.size() > 0 ) )
                {
                  if ( !labelPlacementElementList.item( 0 ).toElement().isNull() )
                  {
                    double displacementX = 0.0;
                    double displacementY = 0.0;
                    double rotationAngle = 0.0;

                    QDomElement pointPlacementElement = labelPlacementElementList.item( 0 ).toElement().elementsByTagName( "PointPlacement" ).item( 0 ).toElement();
                    if ( !pointPlacementElement.isNull() )
                    {
                      bool success = false;
                      rotationAngle = pointPlacementElement.elementsByTagName( "Rotation" ).item( 0 ).toElement().text().toDouble( &success );
                      if ( !success )
                      {
                        rotationAngle = 0.0;
                      }
                      success = false;
                      displacementX = pointPlacementElement.elementsByTagName( "DisplacementX" ).item( 0 ).toElement().text().toDouble( &success );
                      if ( !success )
                      {
                        displacementX = 0.0;
                      }
                      displacementY = pointPlacementElement.elementsByTagName( "DisplacementY" ).item( 0 ).toElement().text().toDouble( &success );
                      if ( !success )
                      {
                        displacementY = 0.0;
                      }
                    }
                    QgsDebugMsg( "rotationAngle " + QString::number( rotationAngle ) );

                    myLabelAttributes->setOffset( displacementX, displacementY, QgsLabelAttributes::PointUnits );
                    myLabelAttributes->setAngle( rotationAngle );
                  }
                } // end labelPlacement
                vec->enableLabels( true );
                return true;
              }
              else
              {
                return false;
              }
            }
            else
            {
              //from the specs: 'if a Label element is not provided ... then no text will be rendered'
              return false;
            }  // end if a viable label element is provided

          }    // end if there is a viable text textSymbolizerElement
          else
          {
            return false;
          }

        } // end if there are textSymbolizers
        else
        {
          return false;
        }
      } // end if there are rule elements
      else
      {
        return false;
      }
    }
    else
    {
      return false;
    }
  }  // end else from the beginning
}

QgsVectorLayer* QgsSLDConfigParser::contourLayerFromRaster( const QDomElement& userStyleElem, QgsRasterLayer* rasterLayer ) const
{
  QgsDebugMsg( "Entering." );

  if ( !rasterLayer )
  {
    return 0;
  }

  //get <ContourSymbolizer> element
  QDomNodeList contourNodeList = userStyleElem.elementsByTagName( "ContourSymbolizer" );
  if ( contourNodeList.size() < 1 )
  {
    return 0;
  }

  QDomElement contourSymbolizerElem = contourNodeList.item( 0 ).toElement();
  if ( contourSymbolizerElem.isNull() )
  {
    return 0;
  }

  double equidistance, minValue, maxValue, offset;
  QString propertyName;

  equidistance = contourSymbolizerElem.attribute( "equidistance" ).toDouble();
  minValue = contourSymbolizerElem.attribute( "minValue" ).toDouble();
  maxValue = contourSymbolizerElem.attribute( "maxValue" ).toDouble();
  offset = contourSymbolizerElem.attribute( "offset" ).toDouble();
  propertyName = contourSymbolizerElem.attribute( "propertyName" );

  if ( equidistance <= 0.0 )
  {
    return 0;
  }

  QTemporaryFile* tmpFile1 = new QTemporaryFile();
  tmpFile1->open();
  mFilesToRemove.push_back( tmpFile1 );
  QString tmpBaseName = tmpFile1->fileName();
  QString tmpFileName = tmpBaseName + ".shp";

  //hack: use gdal_contour first to write into a temporary file
  /* todo: use GDALContourGenerate( hBand, dfInterval, dfOffset,
                                nFixedLevelCount, adfFixedLevels,
                                bNoDataSet, dfNoData,
                                hLayer, 0, nElevField,
                                GDALTermProgress, NULL );*/


  //do the stuff that is also done in the main method of gdal_contour...
  /* -------------------------------------------------------------------- */
  /*      Open source raster file.                                        */
  /* -------------------------------------------------------------------- */
  GDALRasterBandH hBand;
  GDALDatasetH hSrcDS;

  int numberOfLevels = 0;
  double currentLevel = 0.0;

  if ( maxValue > minValue )
  {
    //find first level
    currentLevel = ( int )(( minValue - offset ) / equidistance + 0.5 ) * equidistance + offset;
    while ( currentLevel <= maxValue )
    {
      ++numberOfLevels;
      currentLevel += equidistance;
    }
  }

  double *adfFixedLevels = new double[numberOfLevels];
  int    nFixedLevelCount = numberOfLevels;
  currentLevel = ( int )(( minValue - offset ) / equidistance + 0.5 ) * equidistance + offset;
  for ( int i = 0; i < numberOfLevels; ++i )
  {
    adfFixedLevels[i] = currentLevel;
    currentLevel += equidistance;
  }
  int nBandIn = 1;
  double dfInterval = equidistance, dfNoData = 0.0, dfOffset = offset;

  int /* b3D = FALSE, */ bNoDataSet = FALSE, bIgnoreNoData = FALSE;

  hSrcDS = GDALOpen( TO8( rasterLayer->source() ), GA_ReadOnly );
  if ( hSrcDS == NULL )
  {
    throw QgsMapServiceException( "LayerNotDefined", "Operation request is for a file not available on the server." );
  }

  hBand = GDALGetRasterBand( hSrcDS, nBandIn );
  if ( hBand == NULL )
  {
    CPLError( CE_Failure, CPLE_AppDefined,
              "Band %d does not exist on dataset.",
              nBandIn );
  }

  if ( !bNoDataSet && !bIgnoreNoData )
    dfNoData = GDALGetRasterNoDataValue( hBand, &bNoDataSet );

  /* -------------------------------------------------------------------- */
  /*      Try to get a coordinate system from the raster.                 */
  /* -------------------------------------------------------------------- */
  OGRSpatialReferenceH hSRS = NULL;

  const char *pszWKT = GDALGetProjectionRef( hBand );

  if ( pszWKT != NULL && strlen( pszWKT ) != 0 )
    hSRS = OSRNewSpatialReference( pszWKT );

  /* -------------------------------------------------------------------- */
  /*      Create the outputfile.                                          */
  /* -------------------------------------------------------------------- */
  OGRDataSourceH hDS;
  OGRSFDriverH hDriver = OGRGetDriverByName( "ESRI Shapefile" );
  OGRFieldDefnH hFld;
  OGRLayerH hLayer;
  int nElevField = -1;

  if ( hDriver == NULL )
  {
    //fprintf( FCGI_stderr, "Unable to find format driver named 'ESRI Shapefile'.\n" );
    throw QgsMapServiceException( "LayerNotDefined", "Operation request is for a file not available on the server." );
  }

  hDS = OGR_Dr_CreateDataSource( hDriver, TO8( tmpFileName ), NULL );
  if ( hDS == NULL )
  {
    throw QgsMapServiceException( "LayerNotDefined", "Operation request cannot create data source." );
  }

  hLayer = OGR_DS_CreateLayer( hDS, "contour", hSRS,
                               /* b3D ? wkbLineString25D : */ wkbLineString,
                               NULL );
  if ( hLayer == NULL )
  {
    throw QgsMapServiceException( "LayerNotDefined", "Operation request could not create contour file." );
  }

  hFld = OGR_Fld_Create( "ID", OFTInteger );
  OGR_Fld_SetWidth( hFld, 8 );
  OGR_L_CreateField( hLayer, hFld, FALSE );
  OGR_Fld_Destroy( hFld );

  if ( !propertyName.isEmpty() )
  {
    hFld = OGR_Fld_Create( TO8( propertyName ), OFTReal );
    OGR_Fld_SetWidth( hFld, 12 );
    OGR_Fld_SetPrecision( hFld, 3 );
    OGR_L_CreateField( hLayer, hFld, FALSE );
    OGR_Fld_Destroy( hFld );
    nElevField = 1;
  }

  /* -------------------------------------------------------------------- */
  /*      Invoke.                                                         */
  /* -------------------------------------------------------------------- */
  GDALContourGenerate( hBand, dfInterval, dfOffset,
                       nFixedLevelCount, adfFixedLevels,
                       bNoDataSet, dfNoData,
                       hLayer, 0, nElevField,
                       GDALTermProgress, NULL );

  delete [] adfFixedLevels;

  OGR_DS_Destroy( hDS );
  GDALClose( hSrcDS );

  //todo: store those three files elsewhere...
  //mark shp, dbf and shx to delete after the request
  mFilePathsToRemove.push_back( tmpBaseName + ".shp" );
  mFilePathsToRemove.push_back( tmpBaseName + ".dbf" );
  mFilePathsToRemove.push_back( tmpBaseName + ".shx" );

  QgsVectorLayer* contourLayer = new QgsVectorLayer( tmpFileName, "layer", "ogr" );

  //create renderer
  QgsFeatureRendererV2* theRenderer = rendererFromUserStyle( userStyleElem, contourLayer );
  contourLayer->setRendererV2( theRenderer );

  //add labelling if requested
  labelSettingsFromUserStyle( userStyleElem, contourLayer );

  QgsDebugMsg( "Returning the contour layer" );
  return contourLayer;
}

QDomElement QgsSLDConfigParser::findUserLayerElement( const QString& layerName ) const
{
  QDomElement defaultResult;
  if ( mXMLDoc )
  {
    QDomElement sldElement = mXMLDoc->documentElement();
    if ( !sldElement.isNull() )
    {
      QDomNodeList UserLayerList = sldElement.elementsByTagName( "UserLayer" );
      for ( int i = 0; i < UserLayerList.size(); ++i )
      {
        QDomNodeList nameList = UserLayerList.item( i ).toElement().elementsByTagName( "Name" );
        if ( nameList.size() > 0 )
        {
          if ( nameList.item( 0 ).toElement().text() == layerName )
          {
            return UserLayerList.item( i ).toElement();
          }
        }
      }
    }
  }
  return defaultResult;
}

QgsMapLayer* QgsSLDConfigParser::mapLayerFromUserLayer( const QDomElement& userLayerElem, const QString& layerName, bool allowCaching ) const
{
  QgsDebugMsg( "Entering." );
  QgsMSLayerBuilder* layerBuilder = 0;
  QDomElement builderRootElement;

  //hosted vector data?
  QDomNode hostedVDSNode = userLayerElem.namedItem( "HostedVDS" );
  if ( !hostedVDSNode.isNull() )
  {
    builderRootElement = hostedVDSNode.toElement();
    layerBuilder = new QgsHostedVDSBuilder();
  }

  //hosted raster data?
  QDomNode hostedRDSNode = userLayerElem.namedItem( "HostedRDS" );
  if ( !layerBuilder && !hostedRDSNode.isNull() )
  {
    builderRootElement = hostedRDSNode.toElement();
    layerBuilder = new QgsHostedRDSBuilder();
  }

  //remote OWS (WMS, WFS, WCS)?
  QDomNode remoteOWSNode = userLayerElem.namedItem( "RemoteOWS" );
  if ( !layerBuilder && !remoteOWSNode.isNull() )
  {
    builderRootElement = remoteOWSNode.toElement();
    layerBuilder = new QgsRemoteOWSBuilder( mParameterMap );
  }

  //remote vector/raster datasource
  QDomNode remoteRDSNode = userLayerElem.namedItem( "RemoteRDS" );
  if ( !layerBuilder && !remoteRDSNode.isNull() )
  {
    builderRootElement = remoteRDSNode.toElement();
    layerBuilder = new QgsRemoteDataSourceBuilder();
    QgsDebugMsg( "Detected remote raster datasource" );
  }

  QDomNode remoteVDSNode = userLayerElem.namedItem( "RemoteVDS" );
  if ( !layerBuilder && !remoteVDSNode.isNull() )
  {
    builderRootElement = remoteVDSNode.toElement();
    layerBuilder = new QgsRemoteDataSourceBuilder();
    QgsDebugMsg( "Detected remote vector datasource" );
  }

  //sent vector/raster datasource
  QDomNode sentVDSNode = userLayerElem.namedItem( "SentVDS" );
  if ( !layerBuilder && !sentVDSNode.isNull() )
  {
    builderRootElement = sentVDSNode.toElement();
    layerBuilder = new QgsSentDataSourceBuilder();
  }

  QDomNode sentRDSNode = userLayerElem.namedItem( "SentRDS" );
  if ( !layerBuilder && !sentRDSNode.isNull() )
  {
    builderRootElement = sentRDSNode.toElement();
    layerBuilder = new QgsSentDataSourceBuilder();
  }

  if ( !layerBuilder )
  {
    return 0;
  }

  QgsMapLayer* theMapLayer = layerBuilder->createMapLayer( builderRootElement, layerName, mFilesToRemove, mLayersToRemove, allowCaching );
  if ( theMapLayer )
  {
    setCrsForLayer( builderRootElement, theMapLayer ); //consider attributes "epsg" and "proj"
  }

  //maybe the datasource is defined in the fallback SLD?
  if ( !theMapLayer && mFallbackParser )
  {
    QList<QgsMapLayer*> fallbackList = mFallbackParser->mapLayerFromStyle( layerName, "", allowCaching );
    if ( fallbackList.size() > 0 )
    {
      QgsMapLayer* fallbackLayer = fallbackList.at( 0 ); //todo: prevent crash if layer list is empty
      if ( fallbackLayer )
      {
        theMapLayer = dynamic_cast<QgsVectorLayer*>( fallbackLayer );
      }
    }
  }

#if 0 //todo: fixme
  //GML from outside the SLD?
  if ( !theMapLayer )
  {
    QMap<QString, QDomDocument*>::const_iterator gmlIt = mExternalGMLDatasets.find( layerName );

    if ( gmlIt != mExternalGMLDatasets.end() )
    {
      QgsDebugMsg( "Trying to get maplayer from external GML" );
      theMapLayer = vectorLayerFromGML( gmlIt.value()->documentElement() );
    }
  }
#endif //0

  //raster layer from interpolation

  QDomNode rasterInterpolationNode = userLayerElem.namedItem( "RasterInterpolation" );
  if ( !rasterInterpolationNode.isNull() )
  {
    QgsVectorLayer* vectorCast = dynamic_cast<QgsVectorLayer*>( theMapLayer );
    if ( vectorCast )
    {
      builderRootElement = rasterInterpolationNode.toElement();
      layerBuilder = new QgsInterpolationLayerBuilder( vectorCast );
      theMapLayer = layerBuilder->createMapLayer( builderRootElement, layerName, mFilesToRemove, mLayersToRemove, allowCaching );
    }
  }

  return theMapLayer;
}

void QgsSLDConfigParser::setCrsForLayer( const QDomElement& layerElem, QgsMapLayer* ml ) const
{
  //create CRS if specified as attribute ("epsg" or "proj")
  QString epsg = layerElem.attribute( "epsg", "" );
  if ( !epsg.isEmpty() )
  {
    bool conversionOk;
    int epsgnr = epsg.toInt( &conversionOk );
    if ( conversionOk )
    {
      //set spatial ref sys
      QgsCoordinateReferenceSystem srs;
      srs.createFromOgcWmsCrs( QString( "EPSG:%1" ).arg( epsgnr ) );
      ml->setCrs( srs );
    }
  }
  else
  {
    QString projString = layerElem.attribute( "proj", "" );
    if ( !projString.isEmpty() )
    {
      QgsCoordinateReferenceSystem srs;
      srs.createFromProj4( projString );
      //TODO: createFromProj4 used to save to the user database any new CRS
      // this behavior was changed in order to separate creation and saving.
      // Not sure if it necessary to save it here, should be checked by someone
      // familiar with the code (should also give a more descriptive name to the generated CRS)
      if ( srs.srsid() == 0 )
      {
        QString myName = QString( " * %1 (%2)" )
                         .arg( QObject::tr( "Generated CRS", "A CRS automatically generated from layer info get this prefix for description" ) )
                         .arg( srs.toProj4() );
        srs.saveAsUserCRS( myName );
      }

      ml->setCrs( srs );
    }
  }
}






