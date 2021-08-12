/***************************************************************************
                        qgsmultibandcolorrenderer.cpp
                        -----------------------------
   begin                : December 2011
   copyright            : (C) 2011 by Marco Hugentobler
   email                : marco at sourcepole dot ch
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmultibandcolorrenderer.h"
#include "qgscontrastenhancement.h"
#include "qgsrastertransparency.h"
#include "qgsrasterviewport.h"
#include "qgslayertreemodellegendnode.h"
#include "qgssymbol.h"

#include <QDomDocument>
#include <QDomElement>
#include <QImage>
#include <QSet>

QgsMultiBandColorRenderer::QgsMultiBandColorRenderer( QgsRasterInterface *input, int redBand, int greenBand, int blueBand,
    QgsContrastEnhancement *redEnhancement,
    QgsContrastEnhancement *greenEnhancement,
    QgsContrastEnhancement *blueEnhancement )
  : QgsRasterRenderer( input, QStringLiteral( "multibandcolor" ) )
  , mRedBand( redBand )
  , mGreenBand( greenBand )
  , mBlueBand( blueBand )
  , mRedContrastEnhancement( redEnhancement )
  , mGreenContrastEnhancement( greenEnhancement )
  , mBlueContrastEnhancement( blueEnhancement )
{
}

QgsMultiBandColorRenderer::~QgsMultiBandColorRenderer()
{
  delete mRedContrastEnhancement;
  delete mGreenContrastEnhancement;
  delete mBlueContrastEnhancement;
}

QgsMultiBandColorRenderer *QgsMultiBandColorRenderer::clone() const
{
  QgsMultiBandColorRenderer *renderer = new QgsMultiBandColorRenderer( nullptr, mRedBand, mGreenBand, mBlueBand );
  renderer->copyCommonProperties( this );

  if ( mRedContrastEnhancement )
  {
    renderer->setRedContrastEnhancement( new QgsContrastEnhancement( *mRedContrastEnhancement ) );
  }
  if ( mGreenContrastEnhancement )
  {
    renderer->setGreenContrastEnhancement( new QgsContrastEnhancement( *mGreenContrastEnhancement ) );
  }
  if ( mBlueContrastEnhancement )
  {
    renderer->setBlueContrastEnhancement( new QgsContrastEnhancement( *mBlueContrastEnhancement ) );
  }

  return renderer;
}

void QgsMultiBandColorRenderer::setRedContrastEnhancement( QgsContrastEnhancement *ce )
{
  delete mRedContrastEnhancement;
  mRedContrastEnhancement = ce;
}

void QgsMultiBandColorRenderer::setGreenContrastEnhancement( QgsContrastEnhancement *ce )
{
  delete mGreenContrastEnhancement;
  mGreenContrastEnhancement = ce;
}

void QgsMultiBandColorRenderer::setBlueContrastEnhancement( QgsContrastEnhancement *ce )
{
  delete mBlueContrastEnhancement;
  mBlueContrastEnhancement = ce;
}

QgsRasterRenderer *QgsMultiBandColorRenderer::create( const QDomElement &elem, QgsRasterInterface *input )
{
  if ( elem.isNull() )
  {
    return nullptr;
  }

  //red band, green band, blue band
  const int redBand = elem.attribute( QStringLiteral( "redBand" ), QStringLiteral( "-1" ) ).toInt();
  const int greenBand = elem.attribute( QStringLiteral( "greenBand" ), QStringLiteral( "-1" ) ).toInt();
  const int blueBand = elem.attribute( QStringLiteral( "blueBand" ), QStringLiteral( "-1" ) ).toInt();

  //contrast enhancements
  QgsContrastEnhancement *redContrastEnhancement = nullptr;
  const QDomElement redContrastElem = elem.firstChildElement( QStringLiteral( "redContrastEnhancement" ) );
  if ( !redContrastElem.isNull() )
  {
    redContrastEnhancement = new QgsContrastEnhancement( ( Qgis::DataType )(
          input->dataType( redBand ) ) );
    redContrastEnhancement->readXml( redContrastElem );
  }

  QgsContrastEnhancement *greenContrastEnhancement = nullptr;
  const QDomElement greenContrastElem = elem.firstChildElement( QStringLiteral( "greenContrastEnhancement" ) );
  if ( !greenContrastElem.isNull() )
  {
    greenContrastEnhancement = new QgsContrastEnhancement( ( Qgis::DataType )(
          input->dataType( greenBand ) ) );
    greenContrastEnhancement->readXml( greenContrastElem );
  }

  QgsContrastEnhancement *blueContrastEnhancement = nullptr;
  const QDomElement blueContrastElem = elem.firstChildElement( QStringLiteral( "blueContrastEnhancement" ) );
  if ( !blueContrastElem.isNull() )
  {
    blueContrastEnhancement = new QgsContrastEnhancement( ( Qgis::DataType )(
          input->dataType( blueBand ) ) );
    blueContrastEnhancement->readXml( blueContrastElem );
  }

  QgsRasterRenderer *r = new QgsMultiBandColorRenderer( input, redBand, greenBand, blueBand, redContrastEnhancement,
      greenContrastEnhancement, blueContrastEnhancement );
  r->readXml( elem );
  return r;
}

QgsRasterBlock *QgsMultiBandColorRenderer::block( int bandNo, QgsRectangle  const &extent, int width, int height, QgsRasterBlockFeedback *feedback )
{
  Q_UNUSED( bandNo )
  std::unique_ptr< QgsRasterBlock > outputBlock( new QgsRasterBlock() );
  if ( !mInput )
  {
    return outputBlock.release();
  }

  //In some (common) cases, we can simplify the drawing loop considerably and save render time
  bool fastDraw = ( !usesTransparency()
                    && mRedBand > 0 && mGreenBand > 0 && mBlueBand > 0
                    && mAlphaBand < 1 );

  QList<int> bands;
  if ( mRedBand > 0 )
  {
    bands << mRedBand;
  }
  if ( mGreenBand > 0 )
  {
    bands << mGreenBand;
  }
  if ( mBlueBand > 0 )
  {
    bands << mBlueBand;
  }
  if ( bands.empty() )
  {
    // no need to draw anything if no band is set
    // TODO:: we should probably return default color block
    return outputBlock.release();
  }

  if ( mAlphaBand > 0 )
  {
    bands << mAlphaBand;
  }

  QMap<int, QgsRasterBlock *> bandBlocks;
  QgsRasterBlock *defaultPointer = nullptr;
  QList<int>::const_iterator bandIt = bands.constBegin();
  for ( ; bandIt != bands.constEnd(); ++bandIt )
  {
    bandBlocks.insert( *bandIt, defaultPointer );
  }

  QgsRasterBlock *redBlock = nullptr;
  QgsRasterBlock *greenBlock = nullptr;
  QgsRasterBlock *blueBlock = nullptr;
  QgsRasterBlock *alphaBlock = nullptr;

  bandIt = bands.constBegin();
  for ( ; bandIt != bands.constEnd(); ++bandIt )
  {
    bandBlocks[*bandIt] = mInput->block( *bandIt, extent, width, height, feedback );
    if ( !bandBlocks[*bandIt] )
    {
      // We should free the allocated mem from block().
      QgsDebugMsg( QStringLiteral( "No input band" ) );
      --bandIt;
      for ( ; bandIt != bands.constBegin(); --bandIt )
      {
        delete bandBlocks[*bandIt];
      }
      return outputBlock.release();
    }
  }

  if ( mRedBand > 0 )
  {
    redBlock = bandBlocks[mRedBand];
  }
  if ( mGreenBand > 0 )
  {
    greenBlock = bandBlocks[mGreenBand];
  }
  if ( mBlueBand > 0 )
  {
    blueBlock = bandBlocks[mBlueBand];
  }
  if ( mAlphaBand > 0 )
  {
    alphaBlock = bandBlocks[mAlphaBand];
  }

  if ( !outputBlock->reset( Qgis::DataType::ARGB32_Premultiplied, width, height ) )
  {
    for ( int i = 0; i < bandBlocks.size(); i++ )
    {
      delete bandBlocks.value( i );
    }
    return outputBlock.release();
  }

  QRgb *outputBlockColorData = outputBlock->colorData();

  // faster data access to data for the common case that input data are coming from RGB image with 8-bit bands
  const bool hasByteRgb = ( redBlock && greenBlock && blueBlock && redBlock->dataType() == Qgis::DataType::Byte && greenBlock->dataType() == Qgis::DataType::Byte && blueBlock->dataType() == Qgis::DataType::Byte );
  const quint8 *redData = nullptr, *greenData = nullptr, *blueData = nullptr;
  if ( hasByteRgb )
  {
    redData = redBlock->byteData();
    greenData = greenBlock->byteData();
    blueData = blueBlock->byteData();
  }

  const QRgb myDefaultColor = renderColorForNodataPixel();

  if ( fastDraw )
  {
    // By default RGB raster layers have contrast enhancement assigned and normally that requires us to take the slow
    // route that applies the enhancement. However if the algorithm type is "no enhancement" and all input bands are byte-sized,
    // no transform would be applied to the input values and we can take the fast route.
    bool hasEnhancement;
    if ( hasByteRgb )
    {
      hasEnhancement =
        ( mRedContrastEnhancement && mRedContrastEnhancement->contrastEnhancementAlgorithm() != QgsContrastEnhancement::NoEnhancement ) ||
        ( mGreenContrastEnhancement && mGreenContrastEnhancement->contrastEnhancementAlgorithm() != QgsContrastEnhancement::NoEnhancement ) ||
        ( mBlueContrastEnhancement && mBlueContrastEnhancement->contrastEnhancementAlgorithm() != QgsContrastEnhancement::NoEnhancement );
    }
    else
    {
      hasEnhancement = mRedContrastEnhancement || mGreenContrastEnhancement || mBlueContrastEnhancement;
    }
    if ( hasEnhancement )
      fastDraw = false;
  }

  const qgssize count = ( qgssize )width * height;
  for ( qgssize i = 0; i < count; i++ )
  {
    if ( fastDraw ) //fast rendering if no transparency, stretching, color inversion, etc.
    {
      if ( hasByteRgb )
      {
        if ( redBlock->isNoData( i ) ||
             greenBlock->isNoData( i ) ||
             blueBlock->isNoData( i ) )
        {
          outputBlock->setColor( i, myDefaultColor );
        }
        else
        {
          outputBlockColorData[i] = qRgb( redData[i], greenData[i], blueData[i] );
        }
      }
      else
      {
        bool redIsNoData = false;
        bool greenIsNoData = false;
        bool blueIsNoData = false;
        int redVal = 0;
        int greenVal = 0;
        int blueVal = 0;

        redVal = redBlock->valueAndNoData( i, redIsNoData );
        // as soon as any channel has a no data value, don't do any more work -- the result will
        // always be the nodata color!
        if ( !redIsNoData )
          greenVal = greenBlock->valueAndNoData( i, greenIsNoData );
        if ( !redIsNoData && !greenIsNoData )
          blueVal = blueBlock->valueAndNoData( i, blueIsNoData );

        if ( redIsNoData ||
             greenIsNoData ||
             blueIsNoData )
        {
          outputBlock->setColor( i, myDefaultColor );
        }
        else
        {
          outputBlockColorData[i] = qRgb( redVal, greenVal, blueVal );
        }
      }
      continue;
    }

    bool isNoData = false;
    double redVal = 0;
    double greenVal = 0;
    double blueVal = 0;
    if ( mRedBand > 0 )
    {
      redVal = redBlock->valueAndNoData( i, isNoData );
    }
    if ( !isNoData && mGreenBand > 0 )
    {
      greenVal = greenBlock->valueAndNoData( i, isNoData );
    }
    if ( !isNoData && mBlueBand > 0 )
    {
      blueVal = blueBlock->valueAndNoData( i, isNoData );
    }
    if ( isNoData )
    {
      outputBlock->setColor( i, myDefaultColor );
      continue;
    }

    //apply default color if red, green or blue not in displayable range
    if ( ( mRedContrastEnhancement && !mRedContrastEnhancement->isValueInDisplayableRange( redVal ) )
         || ( mGreenContrastEnhancement && !mGreenContrastEnhancement->isValueInDisplayableRange( redVal ) )
         || ( mBlueContrastEnhancement && !mBlueContrastEnhancement->isValueInDisplayableRange( redVal ) ) )
    {
      outputBlock->setColor( i, myDefaultColor );
      continue;
    }

    //stretch color values
    if ( mRedContrastEnhancement )
    {
      redVal = mRedContrastEnhancement->enhanceContrast( redVal );
    }
    if ( mGreenContrastEnhancement )
    {
      greenVal = mGreenContrastEnhancement->enhanceContrast( greenVal );
    }
    if ( mBlueContrastEnhancement )
    {
      blueVal = mBlueContrastEnhancement->enhanceContrast( blueVal );
    }

    //opacity
    double currentOpacity = mOpacity;
    if ( mRasterTransparency )
    {
      currentOpacity = mRasterTransparency->alphaValue( redVal, greenVal, blueVal, mOpacity * 255 ) / 255.0;
    }
    if ( mAlphaBand > 0 )
    {
      currentOpacity *= alphaBlock->value( i ) / 255.0;
    }

    if ( qgsDoubleNear( currentOpacity, 1.0 ) )
    {
      outputBlock->setColor( i, qRgba( redVal, greenVal, blueVal, 255 ) );
    }
    else
    {
      outputBlock->setColor( i, qRgba( currentOpacity * redVal, currentOpacity * greenVal, currentOpacity * blueVal, currentOpacity * 255 ) );
    }
  }

  //delete input blocks
  QMap<int, QgsRasterBlock *>::const_iterator bandDelIt = bandBlocks.constBegin();
  for ( ; bandDelIt != bandBlocks.constEnd(); ++bandDelIt )
  {
    delete bandDelIt.value();
  }

  return outputBlock.release();
}

void QgsMultiBandColorRenderer::writeXml( QDomDocument &doc, QDomElement &parentElem ) const
{
  if ( parentElem.isNull() )
  {
    return;
  }

  QDomElement rasterRendererElem = doc.createElement( QStringLiteral( "rasterrenderer" ) );
  _writeXml( doc, rasterRendererElem );
  rasterRendererElem.setAttribute( QStringLiteral( "redBand" ), mRedBand );
  rasterRendererElem.setAttribute( QStringLiteral( "greenBand" ), mGreenBand );
  rasterRendererElem.setAttribute( QStringLiteral( "blueBand" ), mBlueBand );

  //contrast enhancement
  if ( mRedContrastEnhancement )
  {
    QDomElement redContrastElem = doc.createElement( QStringLiteral( "redContrastEnhancement" ) );
    mRedContrastEnhancement->writeXml( doc, redContrastElem );
    rasterRendererElem.appendChild( redContrastElem );
  }
  if ( mGreenContrastEnhancement )
  {
    QDomElement greenContrastElem = doc.createElement( QStringLiteral( "greenContrastEnhancement" ) );
    mGreenContrastEnhancement->writeXml( doc, greenContrastElem );
    rasterRendererElem.appendChild( greenContrastElem );
  }
  if ( mBlueContrastEnhancement )
  {
    QDomElement blueContrastElem = doc.createElement( QStringLiteral( "blueContrastEnhancement" ) );
    mBlueContrastEnhancement->writeXml( doc, blueContrastElem );
    rasterRendererElem.appendChild( blueContrastElem );
  }
  parentElem.appendChild( rasterRendererElem );
}

QList<int> QgsMultiBandColorRenderer::usesBands() const
{
  QList<int> bandList;
  if ( mRedBand != -1 )
  {
    bandList << mRedBand;
  }
  if ( mGreenBand != -1 )
  {
    bandList << mGreenBand;
  }
  if ( mBlueBand != -1 )
  {
    bandList << mBlueBand;
  }
  return bandList;
}

QList<QgsLayerTreeModelLegendNode *> QgsMultiBandColorRenderer::createLegendNodes( QgsLayerTreeLayer *nodeLayer )
{
  QList<QgsLayerTreeModelLegendNode *> res;
  if ( mRedBand != -1 )
  {
    res << new QgsRasterSymbolLegendNode( nodeLayer, QColor( 255, 0, 0 ), displayBandName( mRedBand ) );
  }
  if ( mGreenBand != -1 )
  {
    res << new QgsRasterSymbolLegendNode( nodeLayer, QColor( 0, 255, 0 ), displayBandName( mGreenBand ) );
  }
  if ( mBlueBand != -1 )
  {
    res << new QgsRasterSymbolLegendNode( nodeLayer, QColor( 0, 0, 255 ), displayBandName( mBlueBand ) );
  }

  return res;
}

void QgsMultiBandColorRenderer::toSld( QDomDocument &doc, QDomElement &element, const QVariantMap &props ) const
{
  // create base structure
  QgsRasterRenderer::toSld( doc, element, props );


#if 0
  // TODO: the following jumped code is necessary to avoid to export channelSelection in
  // case it's set as default value. The drawback is that it's necessary to calc band
  // statistics that can be really slow depending on dataProvider and rastr location.
  // this is the reason this part of code is commented and the channelSelection is
  // always exported.
  //
  // before to export check if the band combination and contrast setting are the
  // default ones to avoid to export this tags
  bool isDefaultCombination = true;
  QList<int> defaultBandCombination( { 1, 2, 3 } );

  isDefaultCombination = isDefaultCombination && ( usesBands() == defaultBandCombination );
  isDefaultCombination = isDefaultCombination && (
                           mRedContrastEnhancement->contrastEnhancementAlgorithm() == QgsContrastEnhancement::StretchToMinimumMaximum &&
                           mGreenContrastEnhancement->contrastEnhancementAlgorithm() == QgsContrastEnhancement::StretchToMinimumMaximum &&
                           mBlueContrastEnhancement->contrastEnhancementAlgorithm() == QgsContrastEnhancement::StretchToMinimumMaximum
                         );
  // compute raster statistics (slow) only if true the previous conditions
  if ( isDefaultCombination )
  {
    QgsRasterBandStats statRed = bandStatistics( 1, QgsRasterBandStats::Min | QgsRasterBandStats::Max );
    isDefaultCombination = isDefaultCombination && (
                             ( mRedContrastEnhancement->minimumValue() == statRed.minimumValue &&
                               mRedContrastEnhancement->maximumValue() == statRed.maximumValue )
                           );
  }
  if ( isDefaultCombination )
  {
    QgsRasterBandStats statGreen = bandStatistics( 2, QgsRasterBandStats::Min | QgsRasterBandStats::Max );
    isDefaultCombination = isDefaultCombination && (
                             ( mGreenContrastEnhancement->minimumValue() == statGreen.minimumValue &&
                               mGreenContrastEnhancement->maximumValue() == statGreen.maximumValue )
                           );
  }
  if ( isDefaultCombination )
  {
    QgsRasterBandStats statBlue = bandStatistics( 3, QgsRasterBandStats::Min | QgsRasterBandStats::Max );
    isDefaultCombination = isDefaultCombination && (
                             ( mBlueContrastEnhancement->minimumValue() == statBlue.minimumValue &&
                               mBlueContrastEnhancement->maximumValue() == statBlue.maximumValue )
                           );
  }
  if ( isDefaultCombination )
    return;
#endif

  // look for RasterSymbolizer tag
  QDomNodeList elements = element.elementsByTagName( QStringLiteral( "sld:RasterSymbolizer" ) );
  if ( elements.size() == 0 )
    return;

  // there SHOULD be only one
  QDomElement rasterSymbolizerElem = elements.at( 0 ).toElement();

  // add Channel Selection tags
  // Need to insert channelSelection in the correct sequence as in SLD standard e.g.
  // after opacity or geometry or as first element after sld:RasterSymbolizer
  QDomElement channelSelectionElem = doc.createElement( QStringLiteral( "sld:ChannelSelection" ) );
  elements = rasterSymbolizerElem.elementsByTagName( QStringLiteral( "sld:Opacity" ) );
  if ( elements.size() != 0 )
  {
    rasterSymbolizerElem.insertAfter( channelSelectionElem, elements.at( 0 ) );
  }
  else
  {
    elements = rasterSymbolizerElem.elementsByTagName( QStringLiteral( "sld:Geometry" ) );
    if ( elements.size() != 0 )
    {
      rasterSymbolizerElem.insertAfter( channelSelectionElem, elements.at( 0 ) );
    }
    else
    {
      rasterSymbolizerElem.insertBefore( channelSelectionElem, rasterSymbolizerElem.firstChild() );
    }
  }

  // for each mapped band
  static QStringList tags { QStringLiteral( "sld:RedChannel" ), QStringLiteral( "sld:GreenChannel" ), QStringLiteral( "sld:BlueChannel" ) };

  QList<QgsContrastEnhancement *> contrastEnhancements;
  contrastEnhancements.append( mRedContrastEnhancement );
  contrastEnhancements.append( mGreenContrastEnhancement );
  contrastEnhancements.append( mBlueContrastEnhancement );

  const QList<int> bands = usesBands();
  QList<int>::const_iterator bandIt = bands.constBegin();
  for ( int tagCounter = 0 ; bandIt != bands.constEnd(); ++bandIt, ++tagCounter )
  {
    if ( *bandIt < 0 )
      continue;

    QDomElement channelElem = doc.createElement( tags[ tagCounter ] );
    channelSelectionElem.appendChild( channelElem );

    // set band
    QDomElement sourceChannelNameElem = doc.createElement( QStringLiteral( "sld:SourceChannelName" ) );
    sourceChannelNameElem.appendChild( doc.createTextNode( QString::number( *bandIt ) ) );
    channelElem.appendChild( sourceChannelNameElem );

    // set ContrastEnhancement for each band
    // NO ContrastEnhancement parameter for the entire bands is managed e.g.
    // because min/max values can vary depending on band.
    if ( contrastEnhancements[ tagCounter ] )
    {
      QDomElement contrastEnhancementElem = doc.createElement( QStringLiteral( "sld:ContrastEnhancement" ) );
      contrastEnhancements[ tagCounter ]->toSld( doc, contrastEnhancementElem );
      channelElem.appendChild( contrastEnhancementElem );
    }
  }
}
