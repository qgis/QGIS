/***************************************************************************
                              qgssldparser.h
                 Creates a set of maplayers from an sld
                              -------------------
  begin                : May 07, 2006
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

#include "qgssldparser.h"
#include "qgsapplication.h"
#include "qgscoordinatetransform.h"
#include "qgsftptransaction.h"
#include "qgshttptransaction.h"
#include "qgsrendererv2.h"
#include "qgssinglesymbolrendererv2.h"
#include "qgssymbolv2.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsmapserviceexception.h"
#include "qgslogger.h"
#include "qgsmslayercache.h"
#include "qgsmsutils.h"
#include "qgsrasterlayer.h"
#include "qgscolorrampshader.h"
#include "qgscoordinatereferencesystem.h"
#include "qgslabelattributes.h"

#include <QDomDocument>
#include <QDomElement>
#include <QBrush>
#include <QDir>
#include <QFileInfo>
#include <QPen>
#include <QTextStream>
#include <QVector>
#include <float.h>
#include <time.h>
#include "qgslabel.h"
#include <stdlib.h>
#include <fcgi_stdio.h>


//layer builders
#include "qgshostedrdsbuilder.h"
#include "qgshostedvdsbuilder.h"
#include "qgsremotedatasourcebuilder.h"
#include "qgsremoteowsbuilder.h"
#include "qgssentdatasourcebuilder.h"

#ifdef DIAGRAMSERVER
#include "qgsdiagramoverlay.h"
#include "qgsbardiagramfactory.h"
#include "qgspiediagramfactory.h"
#include "qgsdiagramrenderer.h"
#include "qgssvgdiagramfactory.h"
#endif //DIAGRAMSERVER

//for contours
#include "gdal_alg.h"
#include "ogr_srs_api.h"
#include "ogr_api.h"

//for raster interpolation
#include "qgsinterpolationlayerbuilder.h"
#include "qgsgridfilewriter.h"
#include "qgsidwinterpolator.h"
#include "qgstininterpolator.h"

#if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM >= 1800
#define TO8(x) (x).toUtf8().constData()
#else
#define TO8(x) (x).toLocal8Bit().constData()
#endif

QgsSLDParser::QgsSLDParser( QDomDocument* doc ): QgsConfigParser(), mXMLDoc( doc )
{
  mSLDNamespace = "http://www.opengis.net/sld";
  if ( doc->firstChild().namespaceURI() != "http://www.opengis.net/sld" && doc->firstChild().nodeName().startsWith( "sld:" ) )
  {
    mSLDNamespace = "";
  }

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

QgsSLDParser::QgsSLDParser(): mXMLDoc( 0 )
{
  mSLDNamespace = "http://www.opengis.net/sld";
}

QgsSLDParser::~QgsSLDParser()
{
  delete mXMLDoc;
}

int QgsSLDParser::numberOfLayers() const
{
  if ( !mXMLDoc )
  {
    return 0;
  }

  QDomElement sldElem = mXMLDoc->documentElement().toElement();
  if ( sldElem.isNull() )
  {
    return 0;
  }
  QDomNodeList userLayerList = sldElem.elementsByTagName( "UserLayer" );
  QDomNodeList namedLayerList = sldElem.elementsByTagName( "NamedLayer" );
  return ( userLayerList.size() + namedLayerList.size() );
}

void QgsSLDParser::layersAndStylesCapabilities( QDomElement& parentElement, QDomDocument& doc, const QString& version, bool fullProjectSettings ) const
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
        QStringList crsNumbers = createCRSListForLayer( theMapLayer );
        appendCRSElementsToLayer( layerElement, doc, crsNumbers );
        appendLayerBoundingBoxes( layerElement, doc, theMapLayer->extent(), theMapLayer->crs() );

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

QList<QgsMapLayer*> QgsSLDParser::mapLayerFromStyle( const QString& layerName, const QString& styleName, bool useCache ) const
{
  QList<QgsMapLayer*> fallbackLayerList;
  QList<QgsMapLayer*> resultList;

  //first check if this layer is a named layer with a user style
  QList<QDomElement> namedLayerElemList = findNamedLayerElements( layerName );
  for ( int i = 0; i < namedLayerElemList.size(); ++i )
  {
    QDomElement userStyleElement = findUserStyleElement( namedLayerElemList[i], styleName );
    if ( !userStyleElement.isNull() )
    {
      fallbackLayerList = mFallbackParser->mapLayerFromStyle( layerName, "", false );
      if ( fallbackLayerList.size() > 0 )
      {
        QgsVectorLayer* v = dynamic_cast<QgsVectorLayer*>( fallbackLayerList.at( 0 ) );
        if ( v )
        {
          QgsFeatureRendererV2* r = rendererFromUserStyle( userStyleElement, v );
          v->setRendererV2( r );
          labelSettingsFromUserStyle( userStyleElement, v );
#ifdef DIAGRAMSERVER
          overlaysFromUserStyle( userStyleElement, v );
#endif //DIAGRAMSERVER
          setOpacityForLayer( namedLayerElemList[i], v );

          resultList.push_back( v );
          return resultList;
        }
        else
        {
          QgsRasterLayer* r = dynamic_cast<QgsRasterLayer*>( fallbackLayerList.at( 0 ) ); //a raster layer?
          if ( r )
          {
            rasterSymbologyFromUserStyle( userStyleElement, r );

            setOpacityForLayer( namedLayerElemList[i], r );

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
  }

  QDomElement userLayerElement = findUserLayerElement( layerName );

  if ( userLayerElement.isNull() )
  {
    //maybe named layer and named style is defined in the fallback SLD?
    if ( mFallbackParser )
    {
      resultList = mFallbackParser->mapLayerFromStyle( layerName, styleName, useCache );
    }

    QList<QgsMapLayer*>::iterator it = resultList.begin();
    for ( ; it != resultList.end(); ++it )
    {
      setOpacityForLayer( userLayerElement, *it );
    }

    return resultList;
  }

  QDomElement userStyleElement = findUserStyleElement( userLayerElement, styleName );

  QgsMapLayer* theMapLayer = mapLayerFromUserLayer( userLayerElement, layerName, useCache );
  if ( !theMapLayer )
  {
    return resultList;
  }

  QgsFeatureRendererV2* theRenderer = 0;

  QgsVectorLayer* theVectorLayer = dynamic_cast<QgsVectorLayer*>( theMapLayer );
  if ( !theVectorLayer )
  {
    //a raster layer
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
      setOpacityForLayer( userLayerElement, theMapLayer );
      resultList.push_back( theMapLayer );

      return resultList;
    }
  }

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
#ifdef DIAGRAMSERVER
    //apply any vector overlays
    QgsDebugMsg( "Trying to get overlays from user style" );
    overlaysFromUserStyle( userStyleElement, theVectorLayer );
#endif //DIAGRAMSERVER
  }

  if ( !theRenderer )
  {
    QgsDebugMsg( "Error, could not create a renderer" );
    delete theVectorLayer;
    return resultList;
  }
  theVectorLayer->setRendererV2( theRenderer );
  QgsDebugMsg( "Returning the vectorlayer" );
  setOpacityForLayer( userLayerElement, theVectorLayer );
  resultList.push_back( theVectorLayer );
  return resultList;
}

QgsFeatureRendererV2* QgsSLDParser::rendererFromUserStyle( const QDomElement& userStyleElement, QgsVectorLayer* vec ) const
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

bool QgsSLDParser::rasterSymbologyFromUserStyle( const QDomElement& userStyleElement, QgsRasterLayer* r ) const
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

// ---------------labelSettingsFromUserStyle-----------------------

bool QgsSLDParser::labelSettingsFromUserStyle( const QDomElement& userStyleElement, QgsVectorLayer* vec ) const
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

// ------------------findUserLayerElement-------------------

QDomElement QgsSLDParser::findUserLayerElement( const QString& layerName ) const
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

QList<QDomElement> QgsSLDParser::findNamedLayerElements( const QString& layerName ) const
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

QDomElement QgsSLDParser::findUserStyleElement( const QDomElement& userLayerElement, const QString& styleName ) const
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

int QgsSLDParser::layersAndStyles( QStringList& layers, QStringList& styles ) const
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

QgsMapLayer* QgsSLDParser::mapLayerFromUserLayer( const QDomElement& userLayerElem, const QString& layerName, bool allowCaching ) const
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
  if ( !hostedRDSNode.isNull() )
  {
    builderRootElement = hostedRDSNode.toElement();
    layerBuilder = new QgsHostedRDSBuilder();
  }

  //remote OWS (WMS, WFS, WCS)?
  QDomNode remoteOWSNode = userLayerElem.namedItem( "RemoteOWS" );
  if ( !remoteOWSNode.isNull() )
  {
    builderRootElement = remoteOWSNode.toElement();
    layerBuilder = new QgsRemoteOWSBuilder( mParameterMap );
  }

  //remote vector/raster datasource
  QDomNode remoteRDSNode = userLayerElem.namedItem( "RemoteRDS" );
  if ( !remoteRDSNode.isNull() )
  {
    builderRootElement = remoteRDSNode.toElement();
    layerBuilder = new QgsRemoteDataSourceBuilder();
    QgsDebugMsg( "Detected remote raster datasource" );
  }
  QDomNode remoteVDSNode = userLayerElem.namedItem( "RemoteVDS" );

  if ( !remoteVDSNode.isNull() )
  {
    builderRootElement = remoteVDSNode.toElement();
    layerBuilder = new QgsRemoteDataSourceBuilder();
    QgsDebugMsg( "Detected remote vector datasource" );
  }

  //sent vector/raster datasource
  QDomNode sentVDSNode = userLayerElem.namedItem( "SentVDS" );
  if ( !sentVDSNode.isNull() )
  {
    builderRootElement = sentVDSNode.toElement();
    layerBuilder = new QgsSentDataSourceBuilder();
  }

  QDomNode sentRDSNode = userLayerElem.namedItem( "SentRDS" );
  if ( !sentRDSNode.isNull() )
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

QgsVectorLayer* QgsSLDParser::vectorLayerFromGML( const QDomElement gmlRootElement ) const
{
  QgsDebugMsg( "Entering." );

  //QString tempFilePath = QgsMSUtils::createTempFilePath();
  //QFile tempFile(tempFilePath);
  QTemporaryFile* tmpFile = new QTemporaryFile();
  if ( tmpFile->open() )
  {
    mFilesToRemove.push_back( tmpFile );
    QTextStream tempFileStream( tmpFile );
    gmlRootElement.save( tempFileStream, 4 );
    tmpFile->close();
  }
  else
  {
    delete tmpFile;
    return 0;
  }

  QgsVectorLayer* theVectorLayer = new QgsVectorLayer( tmpFile->fileName(), layerNameFromUri( tmpFile->fileName() ), "WFS" );
  if ( !theVectorLayer || !theVectorLayer->isValid() )
  {
    QgsDebugMsg( "invalid maplayer" );
    return 0;
  }
  QgsDebugMsg( "returning maplayer" );

  mLayersToRemove.push_back( theVectorLayer ); //make sure the layer gets deleted after each request

  return theVectorLayer;
}

QgsVectorLayer* QgsSLDParser::contourLayerFromRaster( const QDomElement& userStyleElem, QgsRasterLayer* rasterLayer ) const
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

  int b3D = FALSE, bNoDataSet = FALSE, bIgnoreNoData = FALSE;

  hSrcDS = GDALOpen( TO8( rasterLayer->source() ), GA_ReadOnly );
  if ( hSrcDS == NULL )
    exit( 2 );

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
    fprintf( FCGI_stderr, "Unable to find format driver named 'ESRI Shapefile'.\n" );
    exit( 10 );
  }

  hDS = OGR_Dr_CreateDataSource( hDriver, TO8( tmpFileName ), NULL );
  if ( hDS == NULL )
    exit( 1 );

  hLayer = OGR_DS_CreateLayer( hDS, "contour", hSRS,
                               b3D ? wkbLineString25D : wkbLineString,
                               NULL );
  if ( hLayer == NULL )
    exit( 1 );

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

  delete adfFixedLevels;

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

QDomDocument QgsSLDParser::getStyle( const QString& styleName, const QString& layerName ) const
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

QString QgsSLDParser::layerNameFromUri( const QString& uri ) const
{
  //file based?
  QFileInfo f( uri );
  if ( f.exists() )
  {
    return f.baseName();
  }

  //http based?
  if ( uri.startsWith( "http", Qt::CaseInsensitive ) )
  {
    return uri;
  }

  //database?
  if ( uri.contains( "dbname" ) )
  {
    //take tablename
    QStringList spaceSplit = uri.split( " " );
    QStringList::const_iterator slIt;
    for ( slIt = spaceSplit.constBegin(); slIt != spaceSplit.constEnd(); ++slIt )
    {
      if ( slIt->startsWith( "table" ) )
      {
        return slIt->section( "=", 1, 1 );
      }
    }
  }

  return "";
}

void QgsSLDParser::setOpacityForLayer( const QDomElement& layerElem, QgsMapLayer* layer ) const
{
  QDomNode opacityNode = layerElem.namedItem( "Opacity" );
  if ( !layer || opacityNode.isNull() )
  {
    return ;
  }
  QDomElement opacityElem = opacityNode.toElement();
  bool conversionSuccess;
  int opacityValue = opacityElem.text().toInt( &conversionSuccess );
  if ( !conversionSuccess )
  {
    return;
  }

  //make sure value is between 0 and 255
  if ( opacityValue > 255 )
  {
    opacityValue = 255;
  }
  else if ( opacityValue < 0 )
  {
    opacityValue = 0;
  }

  QgsDebugMsg( "Setting opacity value: " + QString::number( opacityValue ) );
  layer->setTransparency( opacityValue );
}

void QgsSLDParser::clearRasterSymbology( QgsRasterLayer* rl ) const
{
  if ( rl )
  {
    if ( rl->rasterType() == QgsRasterLayer::GrayOrUndefined )
    {
      rl->setDrawingStyle( QgsRasterLayer::SingleBandPseudoColor );
    }
  }
}

void QgsSLDParser::setCrsForLayer( const QDomElement& layerElem, QgsMapLayer* ml ) const
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
      ml->setCrs( srs );
    }
  }
}

QgsComposition* QgsSLDParser::initComposition( const QString& composerTemplate, QgsMapRenderer* mapRenderer, QList< QgsComposerMap*>& mapList, QList< QgsComposerLabel* >& labelList ) const
{
  if ( mFallbackParser )
  {
    return mFallbackParser->initComposition( composerTemplate, mapRenderer, mapList, labelList );
  }
  return 0;
}

void QgsSLDParser::printCapabilities( QDomElement& parentElement, QDomDocument& doc ) const
{
  if ( mFallbackParser )
  {
    mFallbackParser->printCapabilities( parentElement, doc );
  }
}

bool QgsSLDParser::featureInfoWithWktGeometry() const
{
  if ( mFallbackParser )
  {
    return mFallbackParser->featureInfoWithWktGeometry();
  }
  return false;
}

QHash<QString, QString> QgsSLDParser::featureInfoLayerAliasMap() const
{
  if ( mFallbackParser )
  {
    return mFallbackParser->featureInfoLayerAliasMap();
  }
  return QHash<QString, QString>();
}

bool QgsSLDParser::featureInfoFormatSIA2045() const
{
  if ( mFallbackParser )
  {
    return mFallbackParser->featureInfoFormatSIA2045();
  }
  return false;
}

void QgsSLDParser::drawOverlays( QPainter* p, int dpi, int width, int height ) const
{
  if ( mFallbackParser )
  {
    mFallbackParser->drawOverlays( p, dpi, width, height );
  }
}

#ifdef DIAGRAMSERVER
int QgsSLDParser::overlaysFromUserStyle( const QDomElement& userStyleElement, QgsVectorLayer* vec ) const
{
  if ( userStyleElement.isNull() || !vec )
  {
    return 1;
  }

  QgsVectorOverlay* currentOverlay = 0;

  //look for <DiagramSymbolizer>
  QDomNodeList diagramSymbolizerList = userStyleElement.elementsByTagName/*NS*/( /*mSLDNamespace,*/ "DiagramSymbolizer" );
  for ( int i = 0; i < diagramSymbolizerList.size(); ++i )
  {
    currentOverlay = vectorOverlayFromDiagramSymbolizer( diagramSymbolizerList.at( i ).toElement(), vec );
    if ( currentOverlay )
    {
      currentOverlay->setDisplayFlag( true );
      vec->addOverlay( currentOverlay );
    }
  }
  return 0;
}

QgsVectorOverlay* QgsSLDParser::vectorOverlayFromDiagramSymbolizer( const QDomElement& symbolizerElem, QgsVectorLayer* vec ) const
{
  if ( symbolizerElem.isNull() )
  {
    return 0;
  }

  QDomNodeList diagramNodeList = symbolizerElem.elementsByTagName( "Diagram" );
  if ( diagramNodeList.size() < 1 )
  {
    return 0;
  }

  QDomElement diagramElem = diagramNodeList.at( 0 ).toElement();
  if ( diagramElem.isNull() )
  {
    return 0;
  }

  QgsVectorDataProvider* provider = vec->dataProvider();
  if ( !provider )
  {
    return 0;
  }

  QgsDiagramOverlay* theOverlay = 0;
  QgsDiagramFactory* theFactory = 0;
  QList<QgsDiagramItem> theItems;
  QList<int> scalingAttributes;
  QgsAttributeList factoryCategoryAttributes;
  QgsDiagramRenderer::ItemInterpretation interpretation = QgsDiagramRenderer::DISCRETE; //discrete or linear

  //scale
  double scaleFactor = scaleFactorFromScaleTag( diagramElem.namedItem( "Scale" ).toElement() );

  //size
  QDomNodeList sizeNodeList = diagramElem.elementsByTagName( "Size" );
  if ( sizeNodeList.size() > 0 )
  {
    QDomElement sizeElem = sizeNodeList.at( 0 ).toElement();
    //fixed value or categorize/interpolate?
    QDomNode valueNode = sizeElem.namedItem( "Value" );
    QDomNode propertyNameNode = sizeElem.namedItem( "PropertyName" );

    if ( !valueNode.isNull() ) //fixed value
    {
      double value = valueNode.toElement().text().toDouble();
      QgsDiagramItem item;
      item.value = 0;
      item.size = ( int )value;
      theItems.push_back( item );
      interpretation = QgsDiagramRenderer::CONSTANT;
    }
    else if ( !propertyNameNode.isNull() )//take size from attribute(s)
    {
      QList<QDomElement> propertyNameElements;
      int currentScalingIndex;
      QDomElement currentElement = propertyNameNode.toElement();
      while ( !currentElement.isNull() )
      {
        currentScalingIndex = provider->fieldNameIndex( currentElement.text() );
        if ( currentScalingIndex != -1 )
        {
          scalingAttributes.push_back( currentScalingIndex );
        }
        currentElement = currentElement.nextSiblingElement( "PropertyName" );
      }
      //add all the attributes as scaling attributes
      interpretation = QgsDiagramRenderer::ATTRIBUTE;

    }
    else //categorize or interpolate
    {
      QDomNodeList categorizeNodeList = sizeElem.elementsByTagName( "Categorize" );
      QDomNodeList interpolateNodeList = sizeElem.elementsByTagName( "Interpolate" );
      if ( categorizeNodeList.size() > 0 )
      {
        QDomElement categorizeElem = categorizeNodeList.at( 0 ).toElement();
        QString scalingAttributeName;
        if ( diagramItemsFromCategorize( categorizeElem, theItems, scalingAttributeName ) != 0 )
        {
          return 0;
        }
        scalingAttributes.push_back( provider->fieldNameIndex( scalingAttributeName ) );
        interpretation = QgsDiagramRenderer::DISCRETE;
      }
      else if ( interpolateNodeList.size() > 0 )
      {
        QDomElement interpolateElem = interpolateNodeList.at( 0 ).toElement();
        QString scalingAttributeName;
        if ( diagramItemsFromInterpolate( interpolateElem, theItems, scalingAttributeName ) != 0 )
        {
          return 0;
        }
        scalingAttributes.push_back( provider->fieldNameIndex( scalingAttributeName ) );
        if ( interpolateElem.attribute( "mode" ) == "linear" )
        {
          interpretation = QgsDiagramRenderer::LINEAR;
        }
      }
      else
      {
        return 0;
      }
    }
  }

  QgsDiagramRenderer* theRenderer = new QgsDiagramRenderer( scalingAttributes );
  theRenderer->setDiagramItems( theItems );
  theRenderer->setItemInterpretation( interpretation );
  theRenderer->setScaleFactor( scaleFactor );


  //opacity

  //rotation

  //displacement

  QDomNodeList wknNodeList = diagramElem.elementsByTagName( "WellKnownName" );
  QDomNodeList svgSymbolNodeList = diagramElem.elementsByTagName( "SvgSymbol" );

  if ( wknNodeList.size() > 0 ) //well known name diagram
  {
    //well known diagram name
    QString wellKnownName = wknNodeList.at( 0 ).toElement().text();
    if ( wellKnownName == "Pie" )
    {
      theFactory = new QgsPieDiagramFactory();
    }
    else if ( wellKnownName == "Bar" )
    {
      theFactory = new QgsBarDiagramFactory();
    }
    else
    {
      //unknown diagram type

    }


    //subtype

    //categories, take field and fill color for now and add it to QgsWKNDiagramFactory
    QList<QgsDiagramCategory> diagramCategories;

    if ( symbologyFromCategoryTags( diagramElem, provider, diagramCategories ) != 0 )
    {
      delete theRenderer;
      delete theFactory;
      return 0;
    }

    QList<QgsDiagramCategory>::const_iterator c_it = diagramCategories.constBegin();
    for ( ; c_it != diagramCategories.constEnd(); ++c_it )
    {
      (( QgsWKNDiagramFactory* )theFactory )->addCategory( *c_it );
      //diagram renderer also needs to know which attributes are needed
      factoryCategoryAttributes.push_back( c_it->propertyIndex() );
    }

    //3D?

    (( QgsWKNDiagramFactory* )theFactory )->setScalingAttributes( scalingAttributes );
    (( QgsWKNDiagramFactory* )theFactory )->setDiagramType( wellKnownName );

  }
  else if ( svgSymbolNodeList.size() > 0 ) //svg symbol?
  {
    theFactory = new QgsSVGDiagramFactory();
    QDomDocument svgDocument;
    svgDocument.appendChild( svgDocument.importNode( svgSymbolNodeList.at( 0 ).toElement(), true ) );

    if ( !(( QgsSVGDiagramFactory* )theFactory )->setSVGData( svgDocument.toByteArray() ) )
    {
      delete theRenderer; delete theFactory;
      return 0;
    }
  }
  else
  {
    delete theRenderer;
    return 0;
  }

  theOverlay = new QgsDiagramOverlay( vec );

  if ( theOverlay && theRenderer && theFactory )
  {
    theRenderer->setFactory( theFactory );
    //overlay needs to fetch scaling attributes and category attributes
    theOverlay->setAttributes( factoryCategoryAttributes + scalingAttributes );
    theOverlay->setDiagramRenderer( theRenderer );
    return theOverlay;
  }

  return 0;
}

int QgsSLDParser::diagramItemsFromCategorize( const QDomElement& categorizeElement, QList<QgsDiagramItem>& items, QString& attribute ) const
{
  items.clear();

  //get name of scaling attribute
  QDomNode lookupNode = categorizeElement.namedItem( "LookupValue" );
  if ( lookupNode.isNull() )
  {
    return 1;
  }
  QDomNode propertyNameNode = lookupNode.namedItem( "PropertyName" );
  if ( propertyNameNode.isNull() )
  {
    return 2;
  }
  attribute = propertyNameNode.toElement().text();

  QDomNodeList valueNodeList = categorizeElement.elementsByTagName( "Value" );
  QDomNodeList thresholdNodeList = categorizeElement.elementsByTagName( "Threshold" );

  if ( ! valueNodeList.size() == ( thresholdNodeList.size() + 1 ) )
  {
    QgsDebugMsg( "error, sizes of value and threshold lists do not match" );
    return 3;
  }

  if ( valueNodeList.size() < 1 || thresholdNodeList.size() < 1 )
  {
    return 4;
  }

  QVariant currentThreshold;
  QgsDiagramItem currentItem;

  //In SE specification, there is a single value tag that stands for values below the first threshold
  //In QGIS, this is modelled with an extra item
  currentItem.size = valueNodeList.at( 0 ).toElement().text().toInt();
  currentItem.value = thresholdNodeList.at( 0 ).toElement().text();
  items.push_back( currentItem );

  int vIndex = 1;
  int tIndex = 0;

  for ( ; vIndex < valueNodeList.size() && tIndex < thresholdNodeList.size(); ++vIndex, ++tIndex )
  {
    currentItem.size = ( int )( valueNodeList.at( vIndex ).toElement().text().toDouble() );
    currentItem.value = thresholdNodeList.at( tIndex ).toElement().text();
    items.push_back( currentItem );
  }

  return 0;
}

int QgsSLDParser::diagramItemsFromInterpolate( const QDomElement& interpolateElement, QList<QgsDiagramItem>& items, QString& attribute ) const
{
  items.clear();

  //get name of scaling attribute
  QDomNode lookupNode = interpolateElement.namedItem( "LookupValue" );
  if ( lookupNode.isNull() )
  {
    return 1;
  }
  QDomNode propertyNameNode = lookupNode.namedItem( "PropertyName" );
  if ( propertyNameNode.isNull() )
  {
    return 2;
  }
  attribute = propertyNameNode.toElement().text();

  //<InterpolationPoint>
  QgsDiagramItem currentItem;
  QDomNode dataNode, valueNode;

  QDomNodeList interpolationPointList = interpolateElement.elementsByTagName( "InterpolationPoint" );
  for ( int i = 0; i < interpolationPointList.size(); ++i )
  {
    //<Data>
    //<Value> (size)
    dataNode = interpolationPointList.at( i ).namedItem( "Data" );
    valueNode = interpolationPointList.at( i ).namedItem( "Value" );
    if ( dataNode.isNull() || valueNode.isNull() )
    {
      continue;
    }
    currentItem.size = ( int )( valueNode.toElement().text().toDouble() );
    currentItem.value = dataNode.toElement().text().toDouble();
    items.push_back( currentItem );
  }
  return 0;
}

int QgsSLDParser::symbologyFromCategoryTags( const QDomElement& diagramElement, const QgsVectorDataProvider* p, QList<QgsDiagramCategory>& categories ) const
{
  if ( !p )
  {
    return 1;
  }

  QDomNodeList categoryList = diagramElement.elementsByTagName( "Category" );

  categories.clear();

  for ( int i = 0; i < categoryList.size(); ++i )
  {
    QgsDiagramCategory currentCategory;
    QPen currentPen( Qt::NoPen );
    QBrush currentBrush( Qt::SolidPattern );

    //look for PropertyName and fill
    QDomNodeList propertyNameList = categoryList.at( i ).toElement().elementsByTagName( "PropertyName" );
    if ( propertyNameList.size() < 1 )
    {
      continue;
    }

    //read svg parameters and create brushes and pens from it
    QDomNodeList SvgParamList = categoryList.at( i ).toElement().elementsByTagName( "SvgParameter" );
    for ( int j = 0; j < SvgParamList.size(); ++j )
    {
      QDomElement svgParamElem = SvgParamList.at( j ).toElement();
      if ( svgParamElem.attribute( "name" ) == "fill" )
      {
        currentBrush.setColor( QColor( svgParamElem.text() ) );
        currentBrush.setStyle( Qt::SolidPattern );
      }
      else if ( svgParamElem.attribute( "name" ) == "stroke" )
      {
        currentPen.setColor( QColor( svgParamElem.text() ) );
        currentPen.setStyle( Qt::SolidLine );
      }
      else if ( svgParamElem.attribute( "name" ) == "stroke-width" )
      {
        currentPen.setWidth( svgParamElem.text().toInt() );
        currentPen.setStyle( Qt::SolidLine );
      }
    }

    //Gap?
    QDomNodeList gapNodeList = categoryList.at( i ).toElement().elementsByTagName( "Gap" );
    if ( gapNodeList.size() > 0 )
    {
      currentCategory.setGap( gapNodeList.at( 0 ).toElement().text().toInt() );
    }

    currentCategory.setPen( currentPen );
    currentCategory.setBrush( currentBrush );
    currentCategory.setPropertyIndex( p->fieldNameIndex( propertyNameList.at( 0 ).toElement().text() ) );
    categories.push_back( currentCategory );
  }

  return 0;
}

double QgsSLDParser::scaleFactorFromScaleTag( const QDomElement& scaleElem ) const
{
  if ( scaleElem.isNull() )
  {
    return 1.0;
  }

  double minScaleDenom, maxScaleDenom, minScaleSizeMult, maxScaleSizeMult;

  QDomElement minScaleDenomElem = scaleElem.namedItem( "MinScaleDenominator" ).toElement();
  QDomElement maxScaleDenomElem = scaleElem.namedItem( "MaxScaleDenominator" ).toElement();
  QDomElement minScaleSizeMultElem = scaleElem.namedItem( "MinScaleSizeMultiplication" ).toElement();
  QDomElement maxScaleSizeMultElem = scaleElem.namedItem( "MaxScaleSizeMultiplication" ).toElement();

  if ( minScaleDenomElem.isNull() || maxScaleDenomElem.isNull() || minScaleSizeMultElem.isNull() || maxScaleSizeMultElem.isNull() )
  {
    return 1.0;
  }

  minScaleDenom = minScaleDenomElem.text().toDouble();
  maxScaleDenom = maxScaleDenomElem.text().toDouble();
  minScaleSizeMult = minScaleSizeMultElem.text().toDouble();
  maxScaleSizeMult = maxScaleSizeMultElem.text().toDouble();

  if ( mScaleDenominator <= minScaleDenom )
  {
    return minScaleSizeMult;
  }
  else if ( mScaleDenominator >= maxScaleDenom )
  {
    return maxScaleSizeMult;
  }
  else
  {
    double factor = ( mScaleDenominator - minScaleDenom ) / ( maxScaleDenom - minScaleDenom ) * ( maxScaleSizeMult - minScaleSizeMult ) + minScaleSizeMult;
    return factor;
  }
}

#endif //DIAGRAMSERVER
