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

#include "qgssymbolv2.h"
#include "qgssymbollayerv2utils.h"

#include "qgslogger.h"
#include "qgsfeature.h"
#include "qgsvectorlayer.h"
#include "qgssymbollayerv2.h"
#include "qgsogcutils.h"
#include "qgsvectorcolorrampv2.h"
#include "qgsrendercontext.h"

#include <QDomDocument>
#include <QDomElement>

QgsHeatmapRenderer::QgsHeatmapRenderer( )
    : QgsFeatureRendererV2( "heatmapRenderer" )
    , mCalculatedMaxValue( 0 )
    , mRadius( 10 )
    , mRadiusPixels( 0 )
    , mRadiusSquared( 0 )
    , mRadiusUnit( QgsSymbolV2::MM )
    , mWeightAttrNum( -1 )
    , mGradientRamp( 0 )
    , mInvertRamp( false )
    , mExplicitMax( 0.0 )
    , mRenderQuality( 3 )
    , mFeaturesRendered( 0 )
{
  mGradientRamp = new QgsVectorGradientColorRampV2( QColor( 255, 255, 255 ), QColor( 0, 0, 0 ) );

}

QgsHeatmapRenderer::~QgsHeatmapRenderer()
{
  delete mGradientRamp;
}

void QgsHeatmapRenderer::initializeValues( QgsRenderContext& context )
{
  mValues.resize( context.painter()->device()->width() * context.painter()->device()->height() / ( mRenderQuality * mRenderQuality ) );
  mValues.fill( 0 );
  mCalculatedMaxValue = 0;
  mFeaturesRendered = 0;
  mRadiusPixels = qRound( mRadius * QgsSymbolLayerV2Utils::pixelSizeScaleFactor( context, mRadiusUnit, mRadiusMapUnitScale ) / mRenderQuality );
  mRadiusSquared = mRadiusPixels * mRadiusPixels;
}

void QgsHeatmapRenderer::startRender( QgsRenderContext& context, const QgsFields& fields )
{
  Q_UNUSED( fields );
  if ( !context.painter() )
  {
    return;
  }

  // find out classification attribute index from name
  mWeightAttrNum = fields.fieldNameIndex( mWeightExpressionString );
  if ( mWeightAttrNum == -1 )
  {
    mWeightExpression.reset( new QgsExpression( mWeightExpressionString ) );
    mWeightExpression->prepare( fields );
  }

  initializeValues( context );
}

QgsMultiPoint QgsHeatmapRenderer::convertToMultipoint( QgsGeometry* geom )
{
  QgsMultiPoint multiPoint;
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

bool QgsHeatmapRenderer::renderFeature( QgsFeature& feature, QgsRenderContext& context, int layer, bool selected, bool drawVertexMarker )
{
  Q_UNUSED( layer );
  Q_UNUSED( selected );
  Q_UNUSED( drawVertexMarker );

  if ( !context.painter() )
  {
    return false;
  }

  QgsGeometry* geom = feature.geometry();
  if ( geom->type() != QGis::Point )
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
      Q_ASSERT( mWeightExpression.data() );
      value = mWeightExpression->evaluate( &feature );
    }
    else
    {
      const QgsAttributes& attrs = feature.attributes();
      value = attrs.value( mWeightAttrNum );
    }
    bool ok = false;
    double evalWeight = value.toDouble( &ok );
    if ( ok )
    {
      weight = evalWeight;
    }
  }

  int width = context.painter()->device()->width() / mRenderQuality;
  int height = context.painter()->device()->height() / mRenderQuality;

  //convert point to multipoint
  QgsMultiPoint multiPoint = convertToMultipoint( geom );

  //loop through all points in multipoint
  for ( QgsMultiPoint::const_iterator pointIt = multiPoint.constBegin(); pointIt != multiPoint.constEnd(); ++pointIt )
  {
    QgsPoint pixel = context.mapToPixel().transform( *pointIt );
    int pointX = pixel.x() / mRenderQuality;
    int pointY = pixel.y() / mRenderQuality;
    for ( int x = qMax( pointX - mRadiusPixels, 0 ); x < qMin( pointX + mRadiusPixels, width ); ++x )
    {
      for ( int y = qMax( pointY - mRadiusPixels, 0 ); y < qMin( pointY + mRadiusPixels, height ); ++y )
      {
        int index = y * width + x;
        if ( index >= mValues.count( ) )
        {
          continue;
        }
        double distanceSquared = pow( pointX - x, 2.0 ) + pow( pointY - y, 2.0 );
        if ( distanceSquared > mRadiusSquared )
        {
          continue;
        }

        double score = weight * quarticKernel( sqrt( distanceSquared ), mRadiusPixels );
        double value = mValues[ index ] + score;
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
  Q_UNUSED( distance );
  Q_UNUSED( bandwidth );
  return 1.0;
}

double QgsHeatmapRenderer::quarticKernel( const double distance, const int bandwidth ) const
{
  return pow( 1. - pow( distance / ( double )bandwidth, 2 ), 2 );
}

double QgsHeatmapRenderer::triweightKernel( const double distance, const int bandwidth ) const
{
  return pow( 1. - pow( distance / ( double )bandwidth, 2 ), 3 );
}

double QgsHeatmapRenderer::epanechnikovKernel( const double distance, const int bandwidth ) const
{
  return ( 1. - pow( distance / ( double )bandwidth, 2 ) );
}

double QgsHeatmapRenderer::triangularKernel( const double distance, const int bandwidth ) const
{
  return ( 1. - ( distance / ( double )bandwidth ) );
}

void QgsHeatmapRenderer::stopRender( QgsRenderContext& context )
{
  renderImage( context );
  mWeightExpression.reset();
}

void QgsHeatmapRenderer::renderImage( QgsRenderContext& context )
{
  if ( !context.painter() || !mGradientRamp )
  {
    return;
  }

  QImage image( context.painter()->device()->width() / mRenderQuality,
                context.painter()->device()->height() / mRenderQuality,
                QImage::Format_ARGB32 );
  image.fill( Qt::transparent );

  double scaleMax = mExplicitMax > 0 ? mExplicitMax : mCalculatedMaxValue;

  int idx = 0;
  double pixVal = 0;
  QColor pixColor;
  for ( int heightIndex = 0; heightIndex < image.height(); ++heightIndex )
  {
    QRgb* scanLine = ( QRgb* )image.scanLine( heightIndex );
    for ( int widthIndex = 0; widthIndex < image.width(); ++widthIndex )
    {
      //scale result to fit in the range [0, 1]
      pixVal = mValues.at( idx ) > 0 ? qMin(( mValues.at( idx ) / scaleMax ), 1.0 ) : 0;

      //convert value to color from ramp
      pixColor = mGradientRamp->color( mInvertRamp ? 1 - pixVal : pixVal );

      scanLine[widthIndex] = pixColor.rgba();
      idx++;
    }
  }

  if ( mRenderQuality > 1 )
  {
    QImage resized = image.scaled( context.painter()->device()->width(),
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
  return "[HEATMAP]";
}

QgsFeatureRendererV2* QgsHeatmapRenderer::clone() const
{
  QgsHeatmapRenderer* newRenderer = new QgsHeatmapRenderer();
  if ( mGradientRamp )
  {
    newRenderer->setColorRamp( mGradientRamp->clone() );
  }
  newRenderer->setInvertRamp( mInvertRamp );
  newRenderer->setRadius( mRadius );
  newRenderer->setRadiusUnit( mRadiusUnit );
  newRenderer->setRadiusMapUnitScale( mRadiusMapUnitScale );
  newRenderer->setMaximumValue( mExplicitMax );
  newRenderer->setRenderQuality( mRenderQuality );
  newRenderer->setWeightExpression( mWeightExpressionString );

  return newRenderer;
}

void QgsHeatmapRenderer::modifyRequestExtent( QgsRectangle &extent, QgsRenderContext& context )
{
  //we need to expand out the request extent so that it includes points which are up to the heatmap radius outside of the
  //actual visible extent
  double extension = 0.0;
  if ( mRadiusUnit == QgsSymbolV2::Pixel )
  {
    extension = mRadius / QgsSymbolLayerV2Utils::pixelSizeScaleFactor( context, QgsSymbolV2::MapUnit, QgsMapUnitScale() );
  }
  else if ( mRadiusUnit == QgsSymbolV2::MM )
  {
    double pixelSize = mRadius * QgsSymbolLayerV2Utils::pixelSizeScaleFactor( context, QgsSymbolV2::MM, QgsMapUnitScale() );
    extension = pixelSize / QgsSymbolLayerV2Utils::pixelSizeScaleFactor( context, QgsSymbolV2::MapUnit, QgsMapUnitScale() );
  }
  else
  {
    extension = mRadius;
  }
  extent.setXMinimum( extent.xMinimum() - extension );
  extent.setXMaximum( extent.xMaximum() + extension );
  extent.setYMinimum( extent.yMinimum() - extension );
  extent.setYMaximum( extent.yMaximum() + extension );
}

QgsFeatureRendererV2* QgsHeatmapRenderer::create( QDomElement& element )
{
  QgsHeatmapRenderer* r = new QgsHeatmapRenderer();
  r->setRadius( element.attribute( "radius", "50.0" ).toFloat() );
  r->setRadiusUnit(( QgsSymbolV2::OutputUnit )element.attribute( "radius_unit", "0" ).toInt() );
  r->setRadiusMapUnitScale( QgsSymbolLayerV2Utils::decodeMapUnitScale( element.attribute( "radius_map_unit_scale", QString() ) ) );
  r->setMaximumValue( element.attribute( "max_value", "0.0" ).toFloat() );
  r->setRenderQuality( element.attribute( "quality", "0" ).toInt() );
  r->setWeightExpression( element.attribute( "weight_expression" ) );

  QDomElement sourceColorRampElem = element.firstChildElement( "colorramp" );
  if ( !sourceColorRampElem.isNull() && sourceColorRampElem.attribute( "name" ) == "[source]" )
  {
    r->setColorRamp( QgsSymbolLayerV2Utils::loadColorRamp( sourceColorRampElem ) );
  }
  r->setInvertRamp( element.attribute( "invert_ramp", "0" ).toInt() );
  return r;
}

QDomElement QgsHeatmapRenderer::save( QDomDocument& doc )
{
  QDomElement rendererElem = doc.createElement( RENDERER_TAG_NAME );
  rendererElem.setAttribute( "type", "heatmapRenderer" );
  rendererElem.setAttribute( "radius", QString::number( mRadius ) );
  rendererElem.setAttribute( "radius_unit", QString::number( mRadiusUnit ) );
  rendererElem.setAttribute( "radius_map_unit_scale", QgsSymbolLayerV2Utils::encodeMapUnitScale( mRadiusMapUnitScale ) );
  rendererElem.setAttribute( "max_value", QString::number( mExplicitMax ) );
  rendererElem.setAttribute( "quality", QString::number( mRenderQuality ) );
  rendererElem.setAttribute( "weight_expression", mWeightExpressionString );
  if ( mGradientRamp )
  {
    QDomElement colorRampElem = QgsSymbolLayerV2Utils::saveColorRamp( "[source]", mGradientRamp, doc );
    rendererElem.appendChild( colorRampElem );
  }
  rendererElem.setAttribute( "invert_ramp", QString::number( mInvertRamp ) );

  return rendererElem;
}

QgsSymbolV2* QgsHeatmapRenderer::symbolForFeature( QgsFeature& feature )
{
  Q_UNUSED( feature );
  return 0;
}

QgsSymbolV2List QgsHeatmapRenderer::symbols()
{
  return QgsSymbolV2List();
}

QList<QString> QgsHeatmapRenderer::usedAttributes()
{
  QSet<QString> attributes;

  // mAttrName can contain either attribute name or an expression.
  // Sometimes it is not possible to distinguish between those two,
  // e.g. "a - b" can be both a valid attribute name or expression.
  // Since we do not have access to fields here, try both options.
  attributes << mWeightExpressionString;

  QgsExpression testExpr( mWeightExpressionString );
  if ( !testExpr.hasParserError() )
    attributes.unite( testExpr.referencedColumns().toSet() );

  return attributes.toList();
}

QgsHeatmapRenderer* QgsHeatmapRenderer::convertFromRenderer( const QgsFeatureRendererV2 *renderer )
{
  if ( renderer->type() == "heatmapRenderer" )
  {
    return dynamic_cast<QgsHeatmapRenderer*>( renderer->clone() );
  }
  else
  {
    return new QgsHeatmapRenderer();
  }
}

void QgsHeatmapRenderer::setColorRamp( QgsVectorColorRampV2 *ramp )
{
  delete mGradientRamp;
  mGradientRamp = ramp;
}
