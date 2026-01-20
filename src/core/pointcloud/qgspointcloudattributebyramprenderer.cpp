/***************************************************************************
                         qgspointcloudattributebyramprenderer.h
                         --------------------
    begin                : October 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointcloudattributebyramprenderer.h"

#include "qgscolorramp.h"
#include "qgscolorramplegendnode.h"
#include "qgslayertreemodellegendnode.h"
#include "qgspointcloudblock.h"
#include "qgsstyle.h"
#include "qgssymbollayerutils.h"

QgsPointCloudAttributeByRampRenderer::QgsPointCloudAttributeByRampRenderer()
{
  mColorRampShader.setSourceColorRamp( QgsStyle::defaultStyle()->colorRamp( u"Viridis"_s ) );
  mColorRampShader.classifyColorRamp( 5, -1, QgsRectangle(), nullptr );
}

QString QgsPointCloudAttributeByRampRenderer::type() const
{
  return u"ramp"_s;
}

QgsPointCloudRenderer *QgsPointCloudAttributeByRampRenderer::clone() const
{
  auto res = std::make_unique< QgsPointCloudAttributeByRampRenderer >();
  res->mAttribute = mAttribute;
  res->mColorRampShader = mColorRampShader;
  res->mMin = mMin;
  res->mMax = mMax;

  copyCommonProperties( res.get() );

  return res.release();
}

void QgsPointCloudAttributeByRampRenderer::renderBlock( const QgsPointCloudBlock *block, QgsPointCloudRenderContext &context )
{
  QgsRectangle visibleExtent = context.renderContext().extent();
  if ( renderAsTriangles() )
  {
    // we need to include also points slightly outside of the visible extent,
    // otherwise the triangulation may be missing triangles near the edges and corners
    visibleExtent.grow( std::max( visibleExtent.width(), visibleExtent.height() ) * 0.05 );
  }

  const char *ptr = block->data();
  int count = block->pointCount();
  const QgsPointCloudAttributeCollection request = block->attributes();

  const std::size_t recordSize = request.pointRecordSize();
  int attributeOffset = 0;
  const QgsPointCloudAttribute *attribute = request.find( mAttribute, attributeOffset );
  if ( !attribute )
    return;
  const QgsPointCloudAttribute::DataType attributeType = attribute->type();

  const bool renderElevation = context.renderContext().elevationMap();
  const QgsDoubleRange zRange = context.renderContext().zRange();
  const bool considerZ = !zRange.isInfinite() || renderElevation;

  const bool applyZOffset = attribute->name() == "Z"_L1;
  const bool applyXOffset = attribute->name() == "X"_L1;
  const bool applyYOffset = attribute->name() == "Y"_L1;

  int rendered = 0;
  double x = 0;
  double y = 0;
  double z = 0;
  const QgsCoordinateTransform ct = context.renderContext().coordinateTransform();
  const bool reproject = ct.isValid();

  int red = 0;
  int green = 0;
  int blue = 0;
  int alpha = 0;
  for ( int i = 0; i < count; ++i )
  {
    if ( context.renderContext().renderingStopped() )
    {
      break;
    }

    if ( considerZ )
    {
      // z value filtering is cheapest, if we're doing it...
      z = pointZ( context, ptr, i );
      if ( !zRange.contains( z ) )
        continue;
    }

    pointXY( context, ptr, i, x, y );
    if ( visibleExtent.contains( x, y ) )
    {
      if ( reproject )
      {
        try
        {
          ct.transformInPlace( x, y, z );
        }
        catch ( QgsCsException & )
        {
          continue;
        }
      }

      double attributeValue = 0;
      context.getAttribute( ptr, i * recordSize + attributeOffset, attributeType, attributeValue );

      if ( applyXOffset )
        attributeValue = context.offset().x() + context.scale().x() * attributeValue;
      if ( applyYOffset )
        attributeValue = context.offset().y() + context.scale().y() * attributeValue;
      if ( applyZOffset )
        attributeValue = ( context.offset().z() + context.scale().z() * attributeValue ) * context.zValueScale() + context.zValueFixedOffset();

      mColorRampShader.shade( attributeValue, &red, &green, &blue, &alpha );

      if ( renderAsTriangles() )
      {
        addPointToTriangulation( x, y, z, QColor( red, green, blue, alpha ), context );

        // We don't want to render any points if we're rendering triangles and there is no preview painter
        if ( !context.renderContext().previewRenderPainter() )
          continue;
      }

      drawPoint( x, y, QColor( red, green, blue, alpha ), context );
      if ( renderElevation )
        drawPointToElevationMap( x, y, z, context );

      rendered++;
    }
  }
  context.incrementPointsRendered( rendered );
}


QgsPointCloudRenderer *QgsPointCloudAttributeByRampRenderer::create( QDomElement &element, const QgsReadWriteContext &context )
{
  auto r = std::make_unique< QgsPointCloudAttributeByRampRenderer >();

  r->setAttribute( element.attribute( u"attribute"_s, u"Intensity"_s ) );

  QDomElement elemShader = element.firstChildElement( u"colorrampshader"_s );
  r->mColorRampShader.readXml( elemShader, context );

  r->setMinimum( element.attribute( u"min"_s, u"0"_s ).toDouble() );
  r->setMaximum( element.attribute( u"max"_s, u"100"_s ).toDouble() );

  r->restoreCommonProperties( element, context );

  return r.release();
}

QDomElement QgsPointCloudAttributeByRampRenderer::save( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement rendererElem = doc.createElement( u"renderer"_s );

  rendererElem.setAttribute( u"type"_s, u"ramp"_s );
  rendererElem.setAttribute( u"min"_s, mMin );
  rendererElem.setAttribute( u"max"_s, mMax );

  rendererElem.setAttribute( u"attribute"_s, mAttribute );

  QDomElement elemShader = mColorRampShader.writeXml( doc, context );
  rendererElem.appendChild( elemShader );

  saveCommonProperties( rendererElem, context );

  return rendererElem;
}

QSet<QString> QgsPointCloudAttributeByRampRenderer::usedAttributes( const QgsPointCloudRenderContext & ) const
{
  QSet<QString> res;
  res << mAttribute;
  return res;
}

QList<QgsLayerTreeModelLegendNode *> QgsPointCloudAttributeByRampRenderer::createLegendNodes( QgsLayerTreeLayer *nodeLayer )
{
  QList<QgsLayerTreeModelLegendNode *> res;
  res << new QgsSimpleLegendNode( nodeLayer, mAttribute );

  switch ( mColorRampShader.colorRampType() )
  {
    case Qgis::ShaderInterpolationMethod::Linear:
      // for interpolated shaders we use a ramp legend node unless the settings flag
      // to use the continuous legend is not set, in that case we fall through
      if ( mColorRampShader.sourceColorRamp() && ( ! mColorRampShader.legendSettings() || mColorRampShader.legendSettings()->useContinuousLegend() ) )
      {
        res << new QgsColorRampLegendNode( nodeLayer, mColorRampShader.sourceColorRamp()->clone(),
                                           mColorRampShader.legendSettings() ? *mColorRampShader.legendSettings() : QgsColorRampLegendNodeSettings(),
                                           mColorRampShader.minimumValue(),
                                           mColorRampShader.maximumValue() );
        break;
      }
      [[fallthrough]];
    case Qgis::ShaderInterpolationMethod::Discrete:
    case Qgis::ShaderInterpolationMethod::Exact:
    {
      // for all others we use itemised lists
      QList< QPair< QString, QColor > > items;
      mColorRampShader.legendSymbologyItems( items );
      res.reserve( items.size() );
      for ( const QPair< QString, QColor > &item : std::as_const( items ) )
      {
        res << new QgsRasterSymbolLegendNode( nodeLayer, item.second, item.first );
      }
      break;
    }
  }
  return res;
}

QString QgsPointCloudAttributeByRampRenderer::attribute() const
{
  return mAttribute;
}

void QgsPointCloudAttributeByRampRenderer::setAttribute( const QString &attribute )
{
  mAttribute = attribute;
}

QgsColorRampShader QgsPointCloudAttributeByRampRenderer::colorRampShader() const
{
  return mColorRampShader;
}

void QgsPointCloudAttributeByRampRenderer::setColorRampShader( const QgsColorRampShader &shader )
{
  mColorRampShader = shader;
}

double QgsPointCloudAttributeByRampRenderer::minimum() const
{
  return mMin;
}

void QgsPointCloudAttributeByRampRenderer::setMinimum( double minimum )
{
  mMin = minimum;
}

double QgsPointCloudAttributeByRampRenderer::maximum() const
{
  return mMax;
}

void QgsPointCloudAttributeByRampRenderer::setMaximum( double value )
{
  mMax = value;
}

std::unique_ptr<QgsPreparedPointCloudRendererData> QgsPointCloudAttributeByRampRenderer::prepare()
{
  auto data = std::make_unique< QgsPointCloudAttributeByRampRendererPreparedData >();
  data->attributeName = mAttribute;
  data->colorRampShader = mColorRampShader;

  data->attributeIsX = mAttribute == "X"_L1;
  data->attributeIsY = mAttribute == "Y"_L1;
  data->attributeIsZ = mAttribute == "Z"_L1;
  return data;
}

QColor QgsPointCloudAttributeByRampRendererPreparedData::pointColor( const QgsPointCloudBlock *block, int i, double z )
{
  double attributeValue = 0;
  if ( attributeIsZ )
    attributeValue = z;
  else
    QgsPointCloudRenderContext::getAttribute( block->data(), i * block->pointRecordSize() + attributeOffset, attributeType, attributeValue );

  if ( attributeIsX )
    attributeValue = block->offset().x() + block->scale().x() * attributeValue;
  else if ( attributeIsY )
    attributeValue = block->offset().y() + block->scale().y() * attributeValue;

  int red = 0;
  int green = 0;
  int blue = 0;
  int alpha = 0;
  colorRampShader.shade( attributeValue, &red, &green, &blue, &alpha );
  return QColor( red, green, blue, alpha );
}

QSet<QString> QgsPointCloudAttributeByRampRendererPreparedData::usedAttributes() const
{
  return { attributeName };
}

bool QgsPointCloudAttributeByRampRendererPreparedData::prepareBlock( const QgsPointCloudBlock *block )
{
  const QgsPointCloudAttributeCollection attributes = block->attributes();
  const QgsPointCloudAttribute *attribute = attributes.find( attributeName, attributeOffset );
  if ( !attribute )
    return false;

  attributeType = attribute->type();
  return true;
}
