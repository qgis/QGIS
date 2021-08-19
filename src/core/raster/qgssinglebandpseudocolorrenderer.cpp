/***************************************************************************
                         qgssinglebandpseudocolorrenderer.cpp
                         ------------------------------------
    begin                : January 2012
    copyright            : (C) 2012 by Marco Hugentobler
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

#include "qgssinglebandpseudocolorrenderer.h"
#include "qgscolorramp.h"
#include "qgscolorrampshader.h"
#include "qgsrastershader.h"
#include "qgsrastertransparency.h"
#include "qgsrasterviewport.h"
#include "qgsstyleentityvisitor.h"
#include "qgscolorramplegendnode.h"

#include <QDomDocument>
#include <QDomElement>
#include <QImage>

QgsSingleBandPseudoColorRenderer::QgsSingleBandPseudoColorRenderer( QgsRasterInterface *input, int band, QgsRasterShader *shader )
  : QgsRasterRenderer( input, QStringLiteral( "singlebandpseudocolor" ) )
  , mShader( shader )
  , mBand( band )
  , mClassificationMin( std::numeric_limits<double>::quiet_NaN() )
  , mClassificationMax( std::numeric_limits<double>::quiet_NaN() )
{
}

void QgsSingleBandPseudoColorRenderer::setBand( int bandNo )
{
  if ( !mInput )
  {
    mBand = bandNo;
    return;
  }

  if ( bandNo <= mInput->bandCount() || bandNo > 0 )
  {
    mBand = bandNo;
  }
}

void QgsSingleBandPseudoColorRenderer::setClassificationMin( double min )
{
  mClassificationMin = min;
  if ( auto *lShader = shader() )
  {
    QgsColorRampShader *colorRampShader = dynamic_cast<QgsColorRampShader *>( lShader->rasterShaderFunction() );
    if ( colorRampShader )
    {
      colorRampShader->setMinimumValue( min );
    }
  }
}

void QgsSingleBandPseudoColorRenderer::setClassificationMax( double max )
{
  mClassificationMax = max;
  if ( auto *lShader = shader() )
  {
    QgsColorRampShader *colorRampShader = dynamic_cast<QgsColorRampShader *>( lShader->rasterShaderFunction() );
    if ( colorRampShader )
    {
      colorRampShader->setMaximumValue( max );
    }
  }
}

QgsSingleBandPseudoColorRenderer *QgsSingleBandPseudoColorRenderer::clone() const
{
  QgsRasterShader *shader = nullptr;

  if ( mShader )
  {
    shader = new QgsRasterShader( mShader->minimumValue(), mShader->maximumValue() );

    // Shader function
    const QgsColorRampShader *origColorRampShader = dynamic_cast<const QgsColorRampShader *>( mShader->rasterShaderFunction() );

    if ( origColorRampShader )
    {
      QgsColorRampShader *colorRampShader = new QgsColorRampShader( *origColorRampShader );
      shader->setRasterShaderFunction( colorRampShader );
    }
  }
  QgsSingleBandPseudoColorRenderer *renderer = new QgsSingleBandPseudoColorRenderer( nullptr, mBand, shader );
  renderer->copyCommonProperties( this );

  return renderer;
}

void QgsSingleBandPseudoColorRenderer::setShader( QgsRasterShader *shader )
{
  mShader.reset( shader );
}

void QgsSingleBandPseudoColorRenderer::createShader( QgsColorRamp *colorRamp, QgsColorRampShader::Type colorRampType, QgsColorRampShader::ClassificationMode classificationMode, int classes, bool clip, const QgsRectangle &extent )
{
  if ( band() == -1 || classificationMin() >= classificationMax() )
  {
    return;
  }

  QgsColorRampShader *colorRampShader = new QgsColorRampShader( classificationMin(), classificationMax(), colorRamp,  colorRampType, classificationMode );
  colorRampShader->classifyColorRamp( classes, band(), extent, input() );
  colorRampShader->setClip( clip );

  QgsRasterShader *rasterShader = new QgsRasterShader();
  rasterShader->setRasterShaderFunction( colorRampShader );
  setShader( rasterShader );
}

QgsRasterRenderer *QgsSingleBandPseudoColorRenderer::create( const QDomElement &elem, QgsRasterInterface *input )
{
  if ( elem.isNull() )
  {
    return nullptr;
  }

  const int band = elem.attribute( QStringLiteral( "band" ), QStringLiteral( "-1" ) ).toInt();
  QgsRasterShader *shader = nullptr;
  const QDomElement rasterShaderElem = elem.firstChildElement( QStringLiteral( "rastershader" ) );
  if ( !rasterShaderElem.isNull() )
  {
    shader = new QgsRasterShader();
    shader->readXml( rasterShaderElem );
  }

  QgsSingleBandPseudoColorRenderer *r = new QgsSingleBandPseudoColorRenderer( input, band, shader );
  r->readXml( elem );

  // TODO: add _readXML in superclass?
  r->setClassificationMin( elem.attribute( QStringLiteral( "classificationMin" ), QStringLiteral( "NaN" ) ).toDouble() );
  r->setClassificationMax( elem.attribute( QStringLiteral( "classificationMax" ), QStringLiteral( "NaN" ) ).toDouble() );

  // Backward compatibility with serialization of QGIS 2.X era
  const QString minMaxOrigin = elem.attribute( QStringLiteral( "classificationMinMaxOrigin" ) );
  if ( !minMaxOrigin.isEmpty() )
  {
    if ( minMaxOrigin.contains( QLatin1String( "MinMax" ) ) )
    {
      r->mMinMaxOrigin.setLimits( QgsRasterMinMaxOrigin::MinMax );
    }
    else if ( minMaxOrigin.contains( QLatin1String( "CumulativeCut" ) ) )
    {
      r->mMinMaxOrigin.setLimits( QgsRasterMinMaxOrigin::CumulativeCut );
    }
    else if ( minMaxOrigin.contains( QLatin1String( "StdDev" ) ) )
    {
      r->mMinMaxOrigin.setLimits( QgsRasterMinMaxOrigin::StdDev );
    }
    else
    {
      r->mMinMaxOrigin.setLimits( QgsRasterMinMaxOrigin::None );
    }

    if ( minMaxOrigin.contains( QLatin1String( "FullExtent" ) ) )
    {
      r->mMinMaxOrigin.setExtent( QgsRasterMinMaxOrigin::WholeRaster );
    }
    else if ( minMaxOrigin.contains( QLatin1String( "SubExtent" ) ) )
    {
      r->mMinMaxOrigin.setExtent( QgsRasterMinMaxOrigin::CurrentCanvas );
    }
    else
    {
      r->mMinMaxOrigin.setExtent( QgsRasterMinMaxOrigin::WholeRaster );
    }

    if ( minMaxOrigin.contains( QLatin1String( "Estimated" ) ) )
    {
      r->mMinMaxOrigin.setStatAccuracy( QgsRasterMinMaxOrigin::Estimated );
    }
    else // if ( minMaxOrigin.contains( QLatin1String( "Exact" ) ) )
    {
      r->mMinMaxOrigin.setStatAccuracy( QgsRasterMinMaxOrigin::Exact );
    }
  }

  return r;
}

QgsRasterBlock *QgsSingleBandPseudoColorRenderer::block( int bandNo, QgsRectangle  const &extent, int width, int height, QgsRasterBlockFeedback *feedback )
{
  Q_UNUSED( bandNo )

  std::unique_ptr< QgsRasterBlock > outputBlock( new QgsRasterBlock() );
  if ( !mInput || !mShader || !mShader->rasterShaderFunction() )
  {
    return outputBlock.release();
  }


  const std::shared_ptr< QgsRasterBlock > inputBlock( mInput->block( mBand, extent, width, height, feedback ) );
  if ( !inputBlock || inputBlock->isEmpty() )
  {
    QgsDebugMsg( QStringLiteral( "No raster data!" ) );
    return outputBlock.release();
  }

  //rendering is faster without considering user-defined transparency
  const bool hasTransparency = usesTransparency();

  std::shared_ptr< QgsRasterBlock > alphaBlock;
  if ( mAlphaBand > 0 && mAlphaBand != mBand )
  {
    alphaBlock.reset( mInput->block( mAlphaBand, extent, width, height, feedback ) );
    if ( !alphaBlock || alphaBlock->isEmpty() )
    {
      return outputBlock.release();
    }
  }
  else if ( mAlphaBand == mBand )
  {
    alphaBlock = inputBlock;
  }

  if ( !outputBlock->reset( Qgis::DataType::ARGB32_Premultiplied, width, height ) )
  {
    return outputBlock.release();
  }

  const QRgb myDefaultColor = renderColorForNodataPixel();
  QRgb *outputBlockData = outputBlock->colorData();
  const QgsRasterShaderFunction *fcn = mShader->rasterShaderFunction();

  const qgssize count = ( qgssize )width * height;
  bool isNoData = false;
  for ( qgssize i = 0; i < count; i++ )
  {
    const double val = inputBlock->valueAndNoData( i, isNoData );
    if ( isNoData )
    {
      outputBlockData[i] = myDefaultColor;
      continue;
    }

    int red, green, blue, alpha;
    if ( !fcn->shade( val, &red, &green, &blue, &alpha ) )
    {
      outputBlockData[i] = myDefaultColor;
      continue;
    }

    if ( alpha < 255 )
    {
      // Working with premultiplied colors, so multiply values by alpha
      red *= ( alpha / 255.0 );
      blue *= ( alpha / 255.0 );
      green *= ( alpha / 255.0 );
    }

    if ( !hasTransparency )
    {
      outputBlockData[i] = qRgba( red, green, blue, alpha );
    }
    else
    {
      //opacity
      double currentOpacity = mOpacity;
      if ( mRasterTransparency )
      {
        currentOpacity = mRasterTransparency->alphaValue( val, mOpacity * 255 ) / 255.0;
      }
      if ( mAlphaBand > 0 )
      {
        currentOpacity *= alphaBlock->value( i ) / 255.0;
      }

      outputBlockData[i] = qRgba( currentOpacity * red, currentOpacity * green, currentOpacity * blue, currentOpacity * alpha );
    }
  }

  return outputBlock.release();
}

void QgsSingleBandPseudoColorRenderer::writeXml( QDomDocument &doc, QDomElement &parentElem ) const
{
  if ( parentElem.isNull() )
  {
    return;
  }

  QDomElement rasterRendererElem = doc.createElement( QStringLiteral( "rasterrenderer" ) );
  _writeXml( doc, rasterRendererElem );
  rasterRendererElem.setAttribute( QStringLiteral( "band" ), mBand );
  if ( mShader )
  {
    mShader->writeXml( doc, rasterRendererElem ); //todo: include color ramp items directly in this renderer
  }
  rasterRendererElem.setAttribute( QStringLiteral( "classificationMin" ), QgsRasterBlock::printValue( mClassificationMin ) );
  rasterRendererElem.setAttribute( QStringLiteral( "classificationMax" ), QgsRasterBlock::printValue( mClassificationMax ) );

  parentElem.appendChild( rasterRendererElem );
}

QList< QPair< QString, QColor > > QgsSingleBandPseudoColorRenderer::legendSymbologyItems() const
{
  QList< QPair< QString, QColor > > symbolItems;
  if ( mShader )
  {
    QgsRasterShaderFunction *shaderFunction = mShader->rasterShaderFunction();
    if ( shaderFunction )
    {
      shaderFunction->legendSymbologyItems( symbolItems );
    }
  }
  return symbolItems;
}

QList<int> QgsSingleBandPseudoColorRenderer::usesBands() const
{
  QList<int> bandList;
  if ( mBand != -1 )
  {
    bandList << mBand;
  }
  return bandList;
}

void QgsSingleBandPseudoColorRenderer::toSld( QDomDocument &doc, QDomElement &element, const QVariantMap &props ) const
{
  // create base structure
  QgsRasterRenderer::toSld( doc, element, props );

  // look for RasterSymbolizer tag
  const QDomNodeList elements = element.elementsByTagName( QStringLiteral( "sld:RasterSymbolizer" ) );
  if ( elements.size() == 0 )
    return;

  // there SHOULD be only one
  QDomElement rasterSymbolizerElem = elements.at( 0 ).toElement();

  // add Channel Selection tags
  QDomElement channelSelectionElem = doc.createElement( QStringLiteral( "sld:ChannelSelection" ) );
  rasterSymbolizerElem.appendChild( channelSelectionElem );

  // for the mapped band
  QDomElement channelElem = doc.createElement( QStringLiteral( "sld:GrayChannel" ) );
  channelSelectionElem.appendChild( channelElem );

  // set band
  QDomElement sourceChannelNameElem = doc.createElement( QStringLiteral( "sld:SourceChannelName" ) );
  sourceChannelNameElem.appendChild( doc.createTextNode( QString::number( band() ) ) );
  channelElem.appendChild( sourceChannelNameElem );

  // add ColorMap tag
  QDomElement colorMapElem = doc.createElement( QStringLiteral( "sld:ColorMap" ) );

  // set type of ColorMap ramp [ramp, intervals, values]
  // basing on interpolation algorithm of the raster shader
  QString rampType = QStringLiteral( "ramp" );
  const QgsColorRampShader *rampShader = dynamic_cast<const QgsColorRampShader *>( mShader->rasterShaderFunction() );
  if ( !rampShader )
    return;

  switch ( rampShader->colorRampType() )
  {
    case ( QgsColorRampShader::Exact ):
      rampType = QStringLiteral( "values" );
      break;
    case ( QgsColorRampShader::Discrete ):
      rampType = QStringLiteral( "intervals" );
      break;
    case ( QgsColorRampShader::Interpolated ):
      rampType = QStringLiteral( "ramp" );
      break;
  }

  colorMapElem.setAttribute( QStringLiteral( "type" ), rampType );
  if ( rampShader->colorRampItemList().size() >= 255 )
    colorMapElem.setAttribute( QStringLiteral( "extended" ), QStringLiteral( "true" ) );
  rasterSymbolizerElem.appendChild( colorMapElem );

  // for each color set a ColorMapEntry tag nested into "sld:ColorMap" tag
  // e.g. <ColorMapEntry color="#EEBE2F" quantity="-300" label="label" opacity="0"/>
  const QList<QgsColorRampShader::ColorRampItem> classes = rampShader->colorRampItemList();
  QList<QgsColorRampShader::ColorRampItem>::const_iterator classDataIt = classes.constBegin();
  for ( ; classDataIt != classes.constEnd();  ++classDataIt )
  {
    QDomElement colorMapEntryElem = doc.createElement( QStringLiteral( "sld:ColorMapEntry" ) );
    colorMapElem.appendChild( colorMapEntryElem );

    // set colorMapEntryElem attributes
    colorMapEntryElem.setAttribute( QStringLiteral( "color" ), classDataIt->color.name() );
    colorMapEntryElem.setAttribute( QStringLiteral( "quantity" ), classDataIt->value );
    colorMapEntryElem.setAttribute( QStringLiteral( "label" ), classDataIt->label );
    if ( classDataIt->color.alphaF() != 1.0 )
    {
      colorMapEntryElem.setAttribute( QStringLiteral( "opacity" ), QString::number( classDataIt->color.alphaF() ) );
    }
  }
}

bool QgsSingleBandPseudoColorRenderer::accept( QgsStyleEntityVisitorInterface *visitor ) const
{
  if ( const QgsColorRampShader *shader = dynamic_cast< const QgsColorRampShader * >( mShader->rasterShaderFunction() ) )
  {
    QgsStyleColorRampEntity entity( shader->sourceColorRamp() );
    if ( !visitor->visit( QgsStyleEntityVisitorInterface::StyleLeaf( &entity ) ) )
      return false;
  }

  return true;
}

QList<QgsLayerTreeModelLegendNode *> QgsSingleBandPseudoColorRenderer::createLegendNodes( QgsLayerTreeLayer *nodeLayer )
{
  if ( !mShader )
    return QList<QgsLayerTreeModelLegendNode *>();

  const QgsColorRampShader *rampShader = dynamic_cast<const QgsColorRampShader *>( mShader->rasterShaderFunction() );
  if ( !rampShader )
    return QList<QgsLayerTreeModelLegendNode *>();

  QList<QgsLayerTreeModelLegendNode *> res;

  const QString name = displayBandName( mBand );
  if ( !name.isEmpty() )
  {
    res << new QgsSimpleLegendNode( nodeLayer, name );
  }

  switch ( rampShader->colorRampType() )
  {
    case QgsColorRampShader::Interpolated:
      // for interpolated shaders we use a ramp legend node unless the settings flag
      // to use the continuous legend is not set, in that case we fall through
      if ( ! rampShader->legendSettings() || rampShader->legendSettings()->useContinuousLegend() )
      {
        if ( !rampShader->colorRampItemList().isEmpty() )
        {
          res << new QgsColorRampLegendNode( nodeLayer, rampShader->createColorRamp(),
                                             rampShader->legendSettings() ? *rampShader->legendSettings() : QgsColorRampLegendNodeSettings(),
                                             rampShader->minimumValue(), rampShader->maximumValue() );
        }
        break;
      }
      Q_FALLTHROUGH();
    case QgsColorRampShader::Discrete:
    case QgsColorRampShader::Exact:
    {
      // for all others we use itemised lists
      const QList< QPair< QString, QColor > > items = legendSymbologyItems();
      res.reserve( items.size() );
      for ( const QPair< QString, QColor > &item : items )
      {
        res << new QgsRasterSymbolLegendNode( nodeLayer, item.second, item.first );
      }
      break;
    }
  }
  return res;
}
