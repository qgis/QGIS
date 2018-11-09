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
  if ( bandNo > mInput->bandCount() || bandNo <= 0 )
  {
    return;
  }
  mBand = bandNo;
}

void QgsSingleBandPseudoColorRenderer::setClassificationMin( double min )
{
  mClassificationMin = min;
  if ( shader() )
  {
    QgsColorRampShader *colorRampShader = dynamic_cast<QgsColorRampShader *>( shader()->rasterShaderFunction() );
    if ( colorRampShader )
    {
      colorRampShader->setMinimumValue( min );
    }
  }
}

void QgsSingleBandPseudoColorRenderer::setClassificationMax( double max )
{
  mClassificationMax = max;
  if ( shader() )
  {
    QgsColorRampShader *colorRampShader = dynamic_cast<QgsColorRampShader *>( shader()->rasterShaderFunction() );
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
      QgsColorRampShader *colorRampShader = new QgsColorRampShader( mShader->minimumValue(), mShader->maximumValue() );

      if ( origColorRampShader->sourceColorRamp() )
      {
        colorRampShader->setSourceColorRamp( origColorRampShader->sourceColorRamp()->clone() );
      }
      colorRampShader->setColorRampType( origColorRampShader->colorRampType() );
      colorRampShader->setClassificationMode( origColorRampShader->classificationMode() );
      colorRampShader->setClip( origColorRampShader->clip() );
      colorRampShader->setColorRampItemList( origColorRampShader->colorRampItemList() );
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

  int band = elem.attribute( QStringLiteral( "band" ), QStringLiteral( "-1" ) ).toInt();
  QgsRasterShader *shader = nullptr;
  QDomElement rasterShaderElem = elem.firstChildElement( QStringLiteral( "rastershader" ) );
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
  QString minMaxOrigin = elem.attribute( QStringLiteral( "classificationMinMaxOrigin" ) );
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
  Q_UNUSED( bandNo );

  std::unique_ptr< QgsRasterBlock > outputBlock( new QgsRasterBlock() );
  if ( !mInput || !mShader || !mShader->rasterShaderFunction() )
  {
    return outputBlock.release();
  }


  std::shared_ptr< QgsRasterBlock > inputBlock( mInput->block( mBand, extent, width, height, feedback ) );
  if ( !inputBlock || inputBlock->isEmpty() )
  {
    QgsDebugMsg( QStringLiteral( "No raster data!" ) );
    return outputBlock.release();
  }

  //rendering is faster without considering user-defined transparency
  bool hasTransparency = usesTransparency();

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

  if ( !outputBlock->reset( Qgis::ARGB32_Premultiplied, width, height ) )
  {
    return outputBlock.release();
  }

  QRgb myDefaultColor = NODATA_COLOR;
  QRgb *outputBlockData = outputBlock->colorData();
  const QgsRasterShaderFunction *fcn = mShader->rasterShaderFunction();

  qgssize count = ( qgssize )width * height;
  for ( qgssize i = 0; i < count; i++ )
  {
    if ( inputBlock->isNoData( i ) )
    {
      outputBlockData[i] = myDefaultColor;
      continue;
    }
    double val = inputBlock->value( i );
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

void QgsSingleBandPseudoColorRenderer::legendSymbologyItems( QList< QPair< QString, QColor > > &symbolItems ) const
{
  if ( mShader )
  {
    QgsRasterShaderFunction *shaderFunction = mShader->rasterShaderFunction();
    if ( shaderFunction )
    {
      shaderFunction->legendSymbologyItems( symbolItems );
    }
  }
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
