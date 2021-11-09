/***************************************************************************
    qgsheatmaprenderer.cpp
    ----------------------
    begin                : November 2014
    copyright            : (C) 2014 Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsheatmaprenderer.h"

#include "qgssymbol.h"
#include "qgssymbollayerutils.h"

#include "qgslogger.h"
#include "qgsfeature.h"
#include "qgsvectorlayer.h"
#include "qgssymbollayer.h"
#include "qgsogcutils.h"
#include "qgscolorramp.h"
#include "qgscolorrampimpl.h"
#include "qgsrendercontext.h"
#include "qgspainteffect.h"
#include "qgspainteffectregistry.h"
#include "qgsstyleentityvisitor.h"

#include <QDomDocument>
#include <QDomElement>

QgsHeatmapRenderer::QgsHeatmapRenderer()
  : QgsFeatureRenderer( QStringLiteral( "heatmapRenderer" ) )
{
  mGradientRamp = new QgsGradientColorRamp( QColor( 255, 255, 255 ), QColor( 0, 0, 0 ) );
}

QgsHeatmapRenderer::~QgsHeatmapRenderer()
{
  delete mGradientRamp;
}

void QgsHeatmapRenderer::initializeValues( QgsRenderContext &context )
{
  mValues.resize( context.painter()->device()->width() * context.painter()->device()->height() / ( mRenderQuality * mRenderQuality ) );
  mValues.fill( 0 );
  mCalculatedMaxValue = 0;
  mFeaturesRendered = 0;
  mRadiusPixels = std::round( context.convertToPainterUnits( mRadius, mRadiusUnit, mRadiusMapUnitScale ) / mRenderQuality );
  mRadiusSquared = mRadiusPixels * mRadiusPixels;
}

void QgsHeatmapRenderer::startRender( QgsRenderContext &context, const QgsFields &fields )
{
  QgsFeatureRenderer::startRender( context, fields );

  if ( !context.painter() )
  {
    return;
  }

  // find out classification attribute index from name
  mWeightAttrNum = fields.lookupField( mWeightExpressionString );
  if ( mWeightAttrNum == -1 )
  {
    mWeightExpression.reset( new QgsExpression( mWeightExpressionString ) );
    mWeightExpression->prepare( &context.expressionContext() );
  }

  initializeValues( context );
}

QgsMultiPointXY QgsHeatmapRenderer::convertToMultipoint( const QgsGeometry *geom )
{
  QgsMultiPointXY multiPoint;
  if ( !geom->isMultipart() )
  {
    multiPoint << geom->asPoint();
  }
  else
  {
    multiPoint = geom->asMultiPoint();
  }

  return multiPoint;
}

bool QgsHeatmapRenderer::renderFeature( const QgsFeature &feature, QgsRenderContext &context, int layer, bool selected, bool drawVertexMarker )
{
  Q_UNUSED( layer )
  Q_UNUSED( selected )
  Q_UNUSED( drawVertexMarker )

  if ( !context.painter() )
  {
    return false;
  }

  if ( !feature.hasGeometry() || feature.geometry().type() != QgsWkbTypes::PointGeometry )
  {
    //can only render point type
    return false;
  }

  double weight = 1.0;
  if ( !mWeightExpressionString.isEmpty() )
  {
    QVariant value;
    if ( mWeightAttrNum == -1 )
    {
      Q_ASSERT( mWeightExpression.get() );
      value = mWeightExpression->evaluate( &context.expressionContext() );
    }
    else
    {
      const QgsAttributes attrs = feature.attributes();
      value = attrs.value( mWeightAttrNum );
    }
    bool ok = false;
    const double evalWeight = value.toDouble( &ok );
    if ( ok )
    {
      weight = evalWeight;
    }
  }

  const int width = context.painter()->device()->width() / mRenderQuality;
  const int height = context.painter()->device()->height() / mRenderQuality;

  //transform geometry if required
  QgsGeometry geom = feature.geometry();
  const QgsCoordinateTransform xform = context.coordinateTransform();
  if ( xform.isValid() )
  {
    geom.transform( xform );
  }

  //convert point to multipoint
  const QgsMultiPointXY multiPoint = convertToMultipoint( &geom );

  //loop through all points in multipoint
  for ( QgsMultiPointXY::const_iterator pointIt = multiPoint.constBegin(); pointIt != multiPoint.constEnd(); ++pointIt )
  {
    const QgsPointXY pixel = context.mapToPixel().transform( *pointIt );
    const int pointX = pixel.x() / mRenderQuality;
    const int pointY = pixel.y() / mRenderQuality;
    for ( int x = std::max( pointX - mRadiusPixels, 0 ); x < std::min( pointX + mRadiusPixels, width ); ++x )
    {
      if ( context.renderingStopped() )
        break;

      for ( int y = std::max( pointY - mRadiusPixels, 0 ); y < std::min( pointY + mRadiusPixels, height ); ++y )
      {
        const int index = y * width + x;
        if ( index >= mValues.count() )
        {
          continue;
        }
        const double distanceSquared = std::pow( pointX - x, 2.0 ) + std::pow( pointY - y, 2.0 );
        if ( distanceSquared > mRadiusSquared )
        {
          continue;
        }

        const double score = weight * quarticKernel( std::sqrt( distanceSquared ), mRadiusPixels );
        const double value = mValues.at( index ) + score;
        if ( value > mCalculatedMaxValue )
        {
          mCalculatedMaxValue = value;
        }
        mValues[ index ] = value;
      }
    }
  }

  mFeaturesRendered++;
#if 0
  //TODO - enable progressive rendering
  if ( mFeaturesRendered % 200  == 0 )
  {
    renderImage( context );
  }
#endif
  return true;
}


double QgsHeatmapRenderer::uniformKernel( const double distance, const int bandwidth ) const
{
  Q_UNUSED( distance )
  Q_UNUSED( bandwidth )
  return 1.0;
}

double QgsHeatmapRenderer::quarticKernel( const double distance, const int bandwidth ) const
{
  return std::pow( 1. - std::pow( distance / static_cast< double >( bandwidth ), 2 ), 2 );
}

double QgsHeatmapRenderer::triweightKernel( const double distance, const int bandwidth ) const
{
  return std::pow( 1. - std::pow( distance / static_cast< double >( bandwidth ), 2 ), 3 );
}

double QgsHeatmapRenderer::epanechnikovKernel( const double distance, const int bandwidth ) const
{
  return ( 1. - std::pow( distance / static_cast< double >( bandwidth ), 2 ) );
}

double QgsHeatmapRenderer::triangularKernel( const double distance, const int bandwidth ) const
{
  return ( 1. - ( distance / static_cast< double >( bandwidth ) ) );
}

void QgsHeatmapRenderer::stopRender( QgsRenderContext &context )
{
  QgsFeatureRenderer::stopRender( context );

  renderImage( context );
  mWeightExpression.reset();
}

void QgsHeatmapRenderer::renderImage( QgsRenderContext &context )
{
  if ( !context.painter() || !mGradientRamp || context.renderingStopped() )
  {
    return;
  }

  QImage image( context.painter()->device()->width() / mRenderQuality,
                context.painter()->device()->height() / mRenderQuality,
                QImage::Format_ARGB32 );
  image.fill( Qt::transparent );

  const double scaleMax = mExplicitMax > 0 ? mExplicitMax : mCalculatedMaxValue;

  int idx = 0;
  double pixVal = 0;
  QColor pixColor;
  for ( int heightIndex = 0; heightIndex < image.height(); ++heightIndex )
  {
    if ( context.renderingStopped() )
      break;

    QRgb *scanLine = reinterpret_cast< QRgb * >( image.scanLine( heightIndex ) );
    for ( int widthIndex = 0; widthIndex < image.width(); ++widthIndex )
    {
      //scale result to fit in the range [0, 1]
      pixVal = mValues.at( idx ) > 0 ? std::min( ( mValues.at( idx ) / scaleMax ), 1.0 ) : 0;

      //convert value to color from ramp
      pixColor = mGradientRamp->color( pixVal );

      scanLine[widthIndex] = pixColor.rgba();
      idx++;
    }
  }

  if ( mRenderQuality > 1 )
  {
    const QImage resized = image.scaled( context.painter()->device()->width(),
                                         context.painter()->device()->height() );
    context.painter()->drawImage( 0, 0, resized );
  }
  else
  {
    context.painter()->drawImage( 0, 0, image );
  }
}

QString QgsHeatmapRenderer::dump() const
{
  return QStringLiteral( "[HEATMAP]" );
}

QgsHeatmapRenderer *QgsHeatmapRenderer::clone() const
{
  QgsHeatmapRenderer *newRenderer = new QgsHeatmapRenderer();
  if ( mGradientRamp )
  {
    newRenderer->setColorRamp( mGradientRamp->clone() );
  }
  newRenderer->setRadius( mRadius );
  newRenderer->setRadiusUnit( mRadiusUnit );
  newRenderer->setRadiusMapUnitScale( mRadiusMapUnitScale );
  newRenderer->setMaximumValue( mExplicitMax );
  newRenderer->setRenderQuality( mRenderQuality );
  newRenderer->setWeightExpression( mWeightExpressionString );
  copyRendererData( newRenderer );

  return newRenderer;
}

void QgsHeatmapRenderer::modifyRequestExtent( QgsRectangle &extent, QgsRenderContext &context )
{
  //we need to expand out the request extent so that it includes points which are up to the heatmap radius outside of the
  //actual visible extent
  const double extension = context.convertToMapUnits( mRadius, mRadiusUnit, mRadiusMapUnitScale );
  extent.setXMinimum( extent.xMinimum() - extension );
  extent.setXMaximum( extent.xMaximum() + extension );
  extent.setYMinimum( extent.yMinimum() - extension );
  extent.setYMaximum( extent.yMaximum() + extension );
}

QgsFeatureRenderer *QgsHeatmapRenderer::create( QDomElement &element, const QgsReadWriteContext &context )
{
  Q_UNUSED( context )
  QgsHeatmapRenderer *r = new QgsHeatmapRenderer();
  r->setRadius( element.attribute( QStringLiteral( "radius" ), QStringLiteral( "50.0" ) ).toFloat() );
  r->setRadiusUnit( static_cast< QgsUnitTypes::RenderUnit >( element.attribute( QStringLiteral( "radius_unit" ), QStringLiteral( "0" ) ).toInt() ) );
  r->setRadiusMapUnitScale( QgsSymbolLayerUtils::decodeMapUnitScale( element.attribute( QStringLiteral( "radius_map_unit_scale" ), QString() ) ) );
  r->setMaximumValue( element.attribute( QStringLiteral( "max_value" ), QStringLiteral( "0.0" ) ).toFloat() );
  r->setRenderQuality( element.attribute( QStringLiteral( "quality" ), QStringLiteral( "0" ) ).toInt() );
  r->setWeightExpression( element.attribute( QStringLiteral( "weight_expression" ) ) );

  QDomElement sourceColorRampElem = element.firstChildElement( QStringLiteral( "colorramp" ) );
  if ( !sourceColorRampElem.isNull() && sourceColorRampElem.attribute( QStringLiteral( "name" ) ) == QLatin1String( "[source]" ) )
  {
    r->setColorRamp( QgsSymbolLayerUtils::loadColorRamp( sourceColorRampElem ) );
  }
  return r;
}

QDomElement QgsHeatmapRenderer::save( QDomDocument &doc, const QgsReadWriteContext &context )
{
  Q_UNUSED( context )
  QDomElement rendererElem = doc.createElement( RENDERER_TAG_NAME );
  rendererElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "heatmapRenderer" ) );
  rendererElem.setAttribute( QStringLiteral( "radius" ), QString::number( mRadius ) );
  rendererElem.setAttribute( QStringLiteral( "radius_unit" ), QString::number( mRadiusUnit ) );
  rendererElem.setAttribute( QStringLiteral( "radius_map_unit_scale" ), QgsSymbolLayerUtils::encodeMapUnitScale( mRadiusMapUnitScale ) );
  rendererElem.setAttribute( QStringLiteral( "max_value" ), QString::number( mExplicitMax ) );
  rendererElem.setAttribute( QStringLiteral( "quality" ), QString::number( mRenderQuality ) );
  rendererElem.setAttribute( QStringLiteral( "weight_expression" ), mWeightExpressionString );

  if ( mGradientRamp )
  {
    const QDomElement colorRampElem = QgsSymbolLayerUtils::saveColorRamp( QStringLiteral( "[source]" ), mGradientRamp, doc );
    rendererElem.appendChild( colorRampElem );
  }

  saveRendererData( doc, rendererElem, context );

  return rendererElem;
}

QgsSymbol *QgsHeatmapRenderer::symbolForFeature( const QgsFeature &feature, QgsRenderContext & ) const
{
  Q_UNUSED( feature )
  return nullptr;
}

QgsSymbolList QgsHeatmapRenderer::symbols( QgsRenderContext & ) const
{
  return QgsSymbolList();
}

QSet<QString> QgsHeatmapRenderer::usedAttributes( const QgsRenderContext & ) const
{
  QSet<QString> attributes;

  // mAttrName can contain either attribute name or an expression.
  // Sometimes it is not possible to distinguish between those two,
  // e.g. "a - b" can be both a valid attribute name or expression.
  // Since we do not have access to fields here, try both options.
  attributes << mWeightExpressionString;

  const QgsExpression testExpr( mWeightExpressionString );
  if ( !testExpr.hasParserError() )
    attributes.unite( testExpr.referencedColumns() );

  return attributes;
}

QgsHeatmapRenderer *QgsHeatmapRenderer::convertFromRenderer( const QgsFeatureRenderer *renderer )
{
  if ( renderer->type() == QLatin1String( "heatmapRenderer" ) )
  {
    return dynamic_cast<QgsHeatmapRenderer *>( renderer->clone() );
  }
  else
  {
    std::unique_ptr< QgsHeatmapRenderer > res = std::make_unique< QgsHeatmapRenderer >();
    renderer->copyRendererData( res.get() );
    return res.release();
  }
}

bool QgsHeatmapRenderer::accept( QgsStyleEntityVisitorInterface *visitor ) const
{
  if ( mGradientRamp )
  {
    QgsStyleColorRampEntity entity( mGradientRamp );
    if ( !visitor->visit( QgsStyleEntityVisitorInterface::StyleLeaf( &entity ) ) )
      return false;
  }
  return true;
}

void QgsHeatmapRenderer::setColorRamp( QgsColorRamp *ramp )
{
  delete mGradientRamp;
  mGradientRamp = ramp;
}
