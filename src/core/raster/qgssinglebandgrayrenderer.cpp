/***************************************************************************
                         qgssinglebandgrayrenderer.cpp
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

#include "qgssinglebandgrayrenderer.h"
#include "qgscontrastenhancement.h"
#include "qgsrastertransparency.h"
#include "qgscolorramplegendnode.h"
#include "qgscolorramplegendnodesettings.h"
#include "qgsreadwritecontext.h"
#include "qgscolorrampimpl.h"
#include "qgssymbol.h"

#include <QDomDocument>
#include <QDomElement>
#include <QImage>
#include <QColor>
#include <memory>

QgsSingleBandGrayRenderer::QgsSingleBandGrayRenderer( QgsRasterInterface *input, int grayBand )
  : QgsRasterRenderer( input, QStringLiteral( "singlebandgray" ) )
  , mGrayBand( grayBand )
  , mGradient( BlackToWhite )
  , mContrastEnhancement( nullptr )
  , mLegendSettings( std::make_unique< QgsColorRampLegendNodeSettings >() )
{
}

QgsSingleBandGrayRenderer *QgsSingleBandGrayRenderer::clone() const
{
  QgsSingleBandGrayRenderer *renderer = new QgsSingleBandGrayRenderer( nullptr, mGrayBand );
  renderer->copyCommonProperties( this );

  renderer->setGradient( mGradient );
  if ( mContrastEnhancement )
  {
    renderer->setContrastEnhancement( new QgsContrastEnhancement( *mContrastEnhancement ) );
  }
  renderer->setLegendSettings( mLegendSettings ? new QgsColorRampLegendNodeSettings( *mLegendSettings.get() ) : new QgsColorRampLegendNodeSettings() );
  return renderer;
}

QgsRasterRenderer *QgsSingleBandGrayRenderer::create( const QDomElement &elem, QgsRasterInterface *input )
{
  if ( elem.isNull() )
  {
    return nullptr;
  }

  const int grayBand = elem.attribute( QStringLiteral( "grayBand" ), QStringLiteral( "-1" ) ).toInt();
  QgsSingleBandGrayRenderer *r = new QgsSingleBandGrayRenderer( input, grayBand );
  r->readXml( elem );

  if ( elem.attribute( QStringLiteral( "gradient" ) ) == QLatin1String( "WhiteToBlack" ) )
  {
    r->setGradient( WhiteToBlack );  // BlackToWhite is default
  }

  const QDomElement contrastEnhancementElem = elem.firstChildElement( QStringLiteral( "contrastEnhancement" ) );
  if ( !contrastEnhancementElem.isNull() )
  {
    QgsContrastEnhancement *ce = new QgsContrastEnhancement( ( Qgis::DataType )(
          input->dataType( grayBand ) ) );
    ce->readXml( contrastEnhancementElem );
    r->setContrastEnhancement( ce );
  }

  std::unique_ptr< QgsColorRampLegendNodeSettings > legendSettings = std::make_unique< QgsColorRampLegendNodeSettings >();
  legendSettings->readXml( elem, QgsReadWriteContext() );
  r->setLegendSettings( legendSettings.release() );

  return r;
}

void QgsSingleBandGrayRenderer::setContrastEnhancement( QgsContrastEnhancement *ce )
{
  mContrastEnhancement.reset( ce );
}

QgsRasterBlock *QgsSingleBandGrayRenderer::block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback )
{
  Q_UNUSED( bandNo )
  QgsDebugMsgLevel( QStringLiteral( "width = %1 height = %2" ).arg( width ).arg( height ), 4 );

  std::unique_ptr< QgsRasterBlock > outputBlock( new QgsRasterBlock() );
  if ( !mInput )
  {
    return outputBlock.release();
  }

  const std::shared_ptr< QgsRasterBlock > inputBlock( mInput->block( mGrayBand, extent, width, height, feedback ) );
  if ( !inputBlock || inputBlock->isEmpty() )
  {
    QgsDebugMsg( QStringLiteral( "No raster data!" ) );
    return outputBlock.release();
  }

  std::shared_ptr< QgsRasterBlock > alphaBlock;

  if ( mAlphaBand > 0 && mGrayBand != mAlphaBand )
  {
    alphaBlock.reset( mInput->block( mAlphaBand, extent, width, height, feedback ) );
    if ( !alphaBlock || alphaBlock->isEmpty() )
    {
      // TODO: better to render without alpha
      return outputBlock.release();
    }
  }
  else if ( mAlphaBand > 0 )
  {
    alphaBlock = inputBlock;
  }

  if ( !outputBlock->reset( Qgis::DataType::ARGB32_Premultiplied, width, height ) )
  {
    return outputBlock.release();
  }

  const QRgb myDefaultColor = renderColorForNodataPixel();
  bool isNoData = false;
  for ( qgssize i = 0; i < ( qgssize )width * height; i++ )
  {
    double grayVal = inputBlock->valueAndNoData( i, isNoData );

    if ( isNoData )
    {
      outputBlock->setColor( i, myDefaultColor );
      continue;
    }

    double currentAlpha = mOpacity;
    if ( mRasterTransparency )
    {
      currentAlpha = mRasterTransparency->alphaValue( grayVal, mOpacity * 255 ) / 255.0;
    }
    if ( mAlphaBand > 0 )
    {
      currentAlpha *= alphaBlock->value( i ) / 255.0;
    }

    if ( mContrastEnhancement )
    {
      if ( !mContrastEnhancement->isValueInDisplayableRange( grayVal ) )
      {
        outputBlock->setColor( i, myDefaultColor );
        continue;
      }
      grayVal = mContrastEnhancement->enhanceContrast( grayVal );
    }

    if ( mGradient == WhiteToBlack )
    {
      grayVal = 255 - grayVal;
    }

    if ( qgsDoubleNear( currentAlpha, 1.0 ) )
    {
      outputBlock->setColor( i, qRgba( grayVal, grayVal, grayVal, 255 ) );
    }
    else
    {
      outputBlock->setColor( i, qRgba( currentAlpha * grayVal, currentAlpha * grayVal, currentAlpha * grayVal, currentAlpha * 255 ) );
    }
  }

  return outputBlock.release();
}

void QgsSingleBandGrayRenderer::writeXml( QDomDocument &doc, QDomElement &parentElem ) const
{
  if ( parentElem.isNull() )
  {
    return;
  }

  QDomElement rasterRendererElem = doc.createElement( QStringLiteral( "rasterrenderer" ) );
  _writeXml( doc, rasterRendererElem );

  rasterRendererElem.setAttribute( QStringLiteral( "grayBand" ), mGrayBand );

  QString gradient;
  if ( mGradient == BlackToWhite )
  {
    gradient = QStringLiteral( "BlackToWhite" );
  }
  else
  {
    gradient = QStringLiteral( "WhiteToBlack" );
  }
  rasterRendererElem.setAttribute( QStringLiteral( "gradient" ), gradient );

  if ( mContrastEnhancement )
  {
    QDomElement contrastElem = doc.createElement( QStringLiteral( "contrastEnhancement" ) );
    mContrastEnhancement->writeXml( doc, contrastElem );
    rasterRendererElem.appendChild( contrastElem );
  }

  if ( mLegendSettings )
    mLegendSettings->writeXml( doc, rasterRendererElem, QgsReadWriteContext() );

  parentElem.appendChild( rasterRendererElem );
}

QList<QPair<QString, QColor> > QgsSingleBandGrayRenderer::legendSymbologyItems() const
{
  QList<QPair<QString, QColor> >  symbolItems;
  if ( mContrastEnhancement && mContrastEnhancement->contrastEnhancementAlgorithm() != QgsContrastEnhancement::NoEnhancement )
  {
    const QColor minColor = ( mGradient == BlackToWhite ) ? Qt::black : Qt::white;
    const QColor maxColor = ( mGradient == BlackToWhite ) ? Qt::white : Qt::black;
    symbolItems.push_back( qMakePair( QString::number( mContrastEnhancement->minimumValue() ), minColor ) );
    symbolItems.push_back( qMakePair( QString::number( mContrastEnhancement->maximumValue() ), maxColor ) );
  }
  return symbolItems;
}

QList<QgsLayerTreeModelLegendNode *> QgsSingleBandGrayRenderer::createLegendNodes( QgsLayerTreeLayer *nodeLayer )
{
  QList<QgsLayerTreeModelLegendNode *> res;
  if ( mContrastEnhancement && mContrastEnhancement->contrastEnhancementAlgorithm() != QgsContrastEnhancement::NoEnhancement )
  {
    const QString name = displayBandName( mGrayBand );
    if ( !name.isEmpty() )
    {
      res << new QgsSimpleLegendNode( nodeLayer, name );
    }

    const QColor minColor = ( mGradient == BlackToWhite ) ? Qt::black : Qt::white;
    const QColor maxColor = ( mGradient == BlackToWhite ) ? Qt::white : Qt::black;
    res << new QgsColorRampLegendNode( nodeLayer, new QgsGradientColorRamp( minColor, maxColor ),
                                       mLegendSettings ? *mLegendSettings : QgsColorRampLegendNodeSettings(),
                                       mContrastEnhancement->minimumValue(),
                                       mContrastEnhancement->maximumValue() );
  }
  return res;
}

QList<int> QgsSingleBandGrayRenderer::usesBands() const
{
  QList<int> bandList;
  if ( mGrayBand != -1 )
  {
    bandList << mGrayBand;
  }
  return bandList;
}

void QgsSingleBandGrayRenderer::toSld( QDomDocument &doc, QDomElement &element, const QVariantMap &props ) const
{
  // create base structure
  QgsRasterRenderer::toSld( doc, element, props );

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

  // for gray band
  QDomElement channelElem = doc.createElement( QStringLiteral( "sld:GrayChannel" ) );
  channelSelectionElem.appendChild( channelElem );

  // set band
  QDomElement sourceChannelNameElem = doc.createElement( QStringLiteral( "sld:SourceChannelName" ) );
  sourceChannelNameElem.appendChild( doc.createTextNode( QString::number( grayBand() ) ) );
  channelElem.appendChild( sourceChannelNameElem );

  // set ContrastEnhancement
  if ( auto *lContrastEnhancement = contrastEnhancement() )
  {
    QDomElement contrastEnhancementElem = doc.createElement( QStringLiteral( "sld:ContrastEnhancement" ) );
    lContrastEnhancement->toSld( doc, contrastEnhancementElem );

    // do changes to minValue/maxValues depending on stretching algorithm. This is necessary because
    // geoserver does a first stretch on min/max, then applies color map rules.
    // In some combination it is necessary to use real min/max values and in
    // others the actual edited min/max values
    switch ( lContrastEnhancement->contrastEnhancementAlgorithm() )
    {
      case QgsContrastEnhancement::StretchAndClipToMinimumMaximum:
      case QgsContrastEnhancement::ClipToMinimumMaximum:
      {
        // with this renderer export have to be check against real min/max values of the raster
        const QgsRasterBandStats myRasterBandStats = mInput->bandStatistics( grayBand(), QgsRasterBandStats::Min | QgsRasterBandStats::Max );

        // if minimum range differ from the real minimum => set is in exported SLD vendor option
        if ( !qgsDoubleNear( lContrastEnhancement->minimumValue(), myRasterBandStats.minimumValue ) )
        {
          // look for VendorOption tag to look for that with minValue attribute
          const QDomNodeList vendorOptions = contrastEnhancementElem.elementsByTagName( QStringLiteral( "sld:VendorOption" ) );
          for ( int i = 0; i < vendorOptions.size(); ++i )
          {
            QDomElement vendorOption = vendorOptions.at( i ).toElement();
            if ( vendorOption.attribute( QStringLiteral( "name" ) ) != QLatin1String( "minValue" ) )
              continue;

            // remove old value and add the new one
            vendorOption.removeChild( vendorOption.firstChild() );
            vendorOption.appendChild( doc.createTextNode( QString::number( myRasterBandStats.minimumValue ) ) );
          }
        }
        break;
      }
      case QgsContrastEnhancement::UserDefinedEnhancement:
        break;
      case QgsContrastEnhancement::NoEnhancement:
        break;
      case QgsContrastEnhancement::StretchToMinimumMaximum:
        break;
    }

    channelElem.appendChild( contrastEnhancementElem );
  }

  // for each color set a ColorMapEntry tag nested into "sld:ColorMap" tag
  // e.g. <ColorMapEntry color="#EEBE2F" quantity="-300" label="label" opacity="0"/>
  QList< QPair< QString, QColor > > classes = legendSymbologyItems();

  // add ColorMap tag
  QDomElement colorMapElem = doc.createElement( QStringLiteral( "sld:ColorMap" ) );
  rasterSymbolizerElem.appendChild( colorMapElem );

  // TODO: add clip intervals basing on real min/max without trigger
  // min/max calculation again that can takes a lot for remote or big images
  //
  // contrast enhancement against a color map can be SLD simulated playing with ColorMapEntryies
  // each ContrastEnhancementAlgorithm need a specific management.
  // set type of ColorMap ramp [ramp, intervals, values]
  // basing on interpolation algorithm of the raster shader
  QList< QPair< QString, QColor > > colorMapping( classes );
  switch ( contrastEnhancement()->contrastEnhancementAlgorithm() )
  {
    case ( QgsContrastEnhancement::StretchAndClipToMinimumMaximum ):
    case ( QgsContrastEnhancement::ClipToMinimumMaximum ):
    {
      const QString lowValue = classes[0].first;
      QColor lowColor = classes[0].second;
      lowColor.setAlpha( 0 );
      const QString highValue = classes[1].first;
      QColor highColor = classes[1].second;
      highColor.setAlpha( 0 );

      colorMapping.prepend( QPair< QString, QColor >( lowValue, lowColor ) );
      colorMapping.append( QPair< QString, QColor >( highValue, highColor ) );
      break;
    }
    case ( QgsContrastEnhancement::StretchToMinimumMaximum ):
    {
      colorMapping[0].first = QStringLiteral( "0" );
      colorMapping[1].first = QStringLiteral( "255" );
      break;
    }
    case ( QgsContrastEnhancement::UserDefinedEnhancement ):
      break;
    case ( QgsContrastEnhancement::NoEnhancement ):
      break;
  }

  // create tags
  for ( auto it = colorMapping.constBegin(); it != colorMapping.constEnd() ; ++it )
  {
    // set low level color mapping
    QDomElement lowColorMapEntryElem = doc.createElement( QStringLiteral( "sld:ColorMapEntry" ) );
    colorMapElem.appendChild( lowColorMapEntryElem );
    lowColorMapEntryElem.setAttribute( QStringLiteral( "color" ), it->second.name() );
    lowColorMapEntryElem.setAttribute( QStringLiteral( "quantity" ), it->first );
    if ( it->second.alphaF() == 0.0 )
    {
      lowColorMapEntryElem.setAttribute( QStringLiteral( "opacity" ), QString::number( it->second.alpha() ) );
    }
  }
}

const QgsColorRampLegendNodeSettings *QgsSingleBandGrayRenderer::legendSettings() const
{
  return mLegendSettings.get();
}

void QgsSingleBandGrayRenderer::setLegendSettings( QgsColorRampLegendNodeSettings *settings )
{
  if ( settings == mLegendSettings.get() )
    return;
  mLegendSettings.reset( settings );
}
