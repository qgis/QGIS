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
#include "qgspointcloudblock.h"
#include "qgsstyle.h"
#include "qgscolorramp.h"
#include "qgssymbollayerutils.h"

QgsPointCloudAttributeByRampRenderer::QgsPointCloudAttributeByRampRenderer()
{
  mColorRamp.reset( QgsStyle::defaultStyle()->colorRamp( QStringLiteral( "Viridis" ) ) );
}

QString QgsPointCloudAttributeByRampRenderer::type() const
{
  return QStringLiteral( "ramp" );
}

QgsPointCloudRenderer *QgsPointCloudAttributeByRampRenderer::clone() const
{
  std::unique_ptr< QgsPointCloudAttributeByRampRenderer > res = qgis::make_unique< QgsPointCloudAttributeByRampRenderer >();
  res->mAttribute = mAttribute;
  res->mColorRamp.reset( colorRamp() ? colorRamp()->clone() : QgsStyle::defaultStyle()->colorRamp( QStringLiteral( "Viridis" ) ) );
  res->mMin = mMin;
  res->mMax = mMax;

  copyCommonProperties( res.get() );

  return res.release();
}

void QgsPointCloudAttributeByRampRenderer::renderBlock( const QgsPointCloudBlock *block, QgsPointCloudRenderContext &context )
{
  const QgsMapToPixel mapToPixel = context.renderContext().mapToPixel();

  const QgsRectangle visibleExtent = context.renderContext().extent();

  QPen pen;
  pen.setWidth( mPainterPenWidth );
  pen.setCapStyle( Qt::FlatCap );
  //pen.setJoinStyle( Qt::MiterJoin );

  const char *ptr = block->data();
  int count = block->pointCount();
  const QgsPointCloudAttributeCollection request = block->attributes();

  const std::size_t recordSize = request.pointRecordSize();
  int attributeOffset = 0;
  const QgsPointCloudAttribute *attribute = request.find( mAttribute, attributeOffset );
  if ( !attribute )
    return;
  const QgsPointCloudAttribute::DataType attributeType = attribute->type();

  const QgsDoubleRange zRange = context.renderContext().zRange();
  const bool considerZ = !zRange.isInfinite();

  const bool applyZOffset = attribute->name() == QLatin1String( "Z" );

  int rendered = 0;
  double x = 0;
  double y = 0;
  double z = 0;
  const QgsCoordinateTransform ct = context.renderContext().coordinateTransform();
  const bool reproject = ct.isValid();
  for ( int i = 0; i < count; ++i )
  {
    if ( considerZ )
    {
      // z value filtering is cheapest, if we're doing it...
      z = pointZ( context, ptr, i );
      if ( !zRange.contains( z ) )
        continue;
    }

    pointXY( context, ptr, i, x, y );
    if ( visibleExtent.contains( QgsPointXY( x, y ) ) )
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

      if ( applyZOffset )
        attributeValue = context.offset().z() + context.scale().z() * attributeValue;

      mapToPixel.transformInPlace( x, y );

      const QColor color = mColorRamp->color( ( attributeValue - mMin ) / ( mMax - mMin ) );
#if 0
      pen.setColor( QColor( red, green, blue ) );
      context.renderContext().painter()->setPen( pen );
      context.renderContext().painter()->drawPoint( QPointF( x, y ) );
#else

      context.renderContext().painter()->fillRect( QRectF( x - mPainterPenWidth * 0.5,
          y - mPainterPenWidth * 0.5,
          mPainterPenWidth, mPainterPenWidth ), color );
#endif

      rendered++;
    }
  }
  context.incrementPointsRendered( rendered );
}


QgsPointCloudRenderer *QgsPointCloudAttributeByRampRenderer::create( QDomElement &element, const QgsReadWriteContext &context )
{
  std::unique_ptr< QgsPointCloudAttributeByRampRenderer > r = qgis::make_unique< QgsPointCloudAttributeByRampRenderer >();

  r->setAttribute( element.attribute( QStringLiteral( "attribute" ), QStringLiteral( "Intensity" ) ) );
  QDomElement sourceColorRampElem = element.firstChildElement( QStringLiteral( "colorramp" ) );
  if ( !sourceColorRampElem.isNull() && sourceColorRampElem.attribute( QStringLiteral( "name" ) ) == QLatin1String( "[source]" ) )
  {
    r->setColorRamp( QgsSymbolLayerUtils::loadColorRamp( sourceColorRampElem ) );
  }
  r->setMin( element.attribute( QStringLiteral( "min" ), QStringLiteral( "0" ) ).toDouble() );
  r->setMax( element.attribute( QStringLiteral( "max" ), QStringLiteral( "100" ) ).toDouble() );

  r->restoreCommonProperties( element, context );

  return r.release();
}

QDomElement QgsPointCloudAttributeByRampRenderer::save( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement rendererElem = doc.createElement( QStringLiteral( "renderer" ) );

  rendererElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "ramp" ) );
  rendererElem.setAttribute( QStringLiteral( "min" ), mMin );
  rendererElem.setAttribute( QStringLiteral( "max" ), mMax );

  rendererElem.setAttribute( QStringLiteral( "attribute" ), mAttribute );
  QDomElement colorRampElem = QgsSymbolLayerUtils::saveColorRamp( QStringLiteral( "[source]" ), mColorRamp.get(), doc );
  rendererElem.appendChild( colorRampElem );

  saveCommonProperties( rendererElem, context );

  return rendererElem;
}

void QgsPointCloudAttributeByRampRenderer::startRender( QgsPointCloudRenderContext &context )
{
  QgsPointCloudRenderer::startRender( context );

  mPainterPenWidth = context.renderContext().convertToPainterUnits( pointSize(), pointSizeUnit(), pointSizeMapUnitScale() );
}

void QgsPointCloudAttributeByRampRenderer::stopRender( QgsPointCloudRenderContext &context )
{
  QgsPointCloudRenderer::stopRender( context );
}

QSet<QString> QgsPointCloudAttributeByRampRenderer::usedAttributes( const QgsPointCloudRenderContext & ) const
{
  QSet<QString> res;
  res << mAttribute;
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

QgsColorRamp *QgsPointCloudAttributeByRampRenderer::colorRamp() const
{
  return mColorRamp.get();
}

void QgsPointCloudAttributeByRampRenderer::setColorRamp( QgsColorRamp *ramp )
{
  mColorRamp.reset( ramp );
}

double QgsPointCloudAttributeByRampRenderer::min() const
{
  return mMin;
}

void QgsPointCloudAttributeByRampRenderer::setMin( double value )
{
  mMin = value;
}

double QgsPointCloudAttributeByRampRenderer::max() const
{
  return mMax;
}

void QgsPointCloudAttributeByRampRenderer::setMax( double value )
{
  mMax = value;
}

