/***************************************************************************
                         qgspointcloudrenderer.cpp
                         --------------------
    begin                : October 2020
    copyright            : (C) 2020 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointcloudrenderer.h"
#include "qgspointcloudrendererregistry.h"
#include "qgsapplication.h"
#include "qgssymbollayerutils.h"

QgsPointCloudRenderContext::QgsPointCloudRenderContext( QgsRenderContext &context, const QgsVector3D &scale, const QgsVector3D &offset )
  : mRenderContext( context )
  , mScale( scale )
  , mOffset( offset )
{

}

long QgsPointCloudRenderContext::pointsRendered() const
{
  return mPointsRendered;
}

void QgsPointCloudRenderContext::incrementPointsRendered( long count )
{
  mPointsRendered += count;
}

void QgsPointCloudRenderContext::setAttributes( const QgsPointCloudAttributeCollection &attributes )
{
  mAttributes = attributes;
  mPointRecordSize = mAttributes.pointRecordSize();

  // fetch offset for x/y/z attributes
  attributes.find( QStringLiteral( "X" ), mXOffset );
  attributes.find( QStringLiteral( "Y" ), mYOffset );
  attributes.find( QStringLiteral( "Z" ), mZOffset );
}

QgsPointCloudRenderer *QgsPointCloudRenderer::load( QDomElement &element, const QgsReadWriteContext &context )
{
  if ( element.isNull() )
    return nullptr;

  // load renderer
  const QString rendererType = element.attribute( QStringLiteral( "type" ) );

  QgsPointCloudRendererAbstractMetadata *m = QgsApplication::pointCloudRendererRegistry()->rendererMetadata( rendererType );
  if ( !m )
    return nullptr;

  std::unique_ptr< QgsPointCloudRenderer > r( m->createRenderer( element, context ) );
  return r.release();
}

QSet<QString> QgsPointCloudRenderer::usedAttributes( const QgsPointCloudRenderContext & ) const
{
  return QSet< QString >();
}

void QgsPointCloudRenderer::startRender( QgsPointCloudRenderContext & )
{
#ifdef QGISDEBUG
  if ( !mThread )
  {
    mThread = QThread::currentThread();
  }
  else
  {
    Q_ASSERT_X( mThread == QThread::currentThread(), "QgsPointCloudRenderer::startRender", "startRender called in a different thread - use a cloned renderer instead" );
  }
#endif
}

void QgsPointCloudRenderer::stopRender( QgsPointCloudRenderContext & )
{
#ifdef QGISDEBUG
  Q_ASSERT_X( mThread == QThread::currentThread(), "QgsPointCloudRenderer::stopRender", "stopRender called in a different thread - use a cloned renderer instead" );
#endif
}

double QgsPointCloudRenderer::maximumScreenError() const
{
  return mMaximumScreenError;
}

void QgsPointCloudRenderer::setMaximumScreenError( double error )
{
  mMaximumScreenError = error;
}

QgsUnitTypes::RenderUnit QgsPointCloudRenderer::maximumScreenErrorUnit() const
{
  return mMaximumScreenErrorUnit;
}

void QgsPointCloudRenderer::setMaximumScreenErrorUnit( QgsUnitTypes::RenderUnit unit )
{
  mMaximumScreenErrorUnit = unit;
}

void QgsPointCloudRenderer::copyCommonProperties( QgsPointCloudRenderer *destination ) const
{
  destination->setPointSize( mPointSize );
  destination->setPointSizeUnit( mPointSizeUnit );
  destination->setPointSizeMapUnitScale( mPointSizeMapUnitScale );
  destination->setMaximumScreenError( mMaximumScreenError );
  destination->setMaximumScreenErrorUnit( mMaximumScreenErrorUnit );
}

void QgsPointCloudRenderer::restoreCommonProperties( const QDomElement &element, const QgsReadWriteContext & )
{
  mPointSize = element.attribute( QStringLiteral( "pointSize" ), QStringLiteral( "1" ) ).toDouble();
  mPointSizeUnit = QgsUnitTypes::decodeRenderUnit( element.attribute( QStringLiteral( "pointSizeUnit" ), QStringLiteral( "MM" ) ) );
  mPointSizeMapUnitScale = QgsSymbolLayerUtils::decodeMapUnitScale( element.attribute( QStringLiteral( "pointSizeMapUnitScale" ), QString() ) );

  mMaximumScreenError = element.attribute( QStringLiteral( "maximumScreenError" ), QStringLiteral( "5" ) ).toDouble();
  mMaximumScreenErrorUnit = QgsUnitTypes::decodeRenderUnit( element.attribute( QStringLiteral( "maximumScreenErrorUnit" ), QStringLiteral( "MM" ) ) );
}

void QgsPointCloudRenderer::saveCommonProperties( QDomElement &element, const QgsReadWriteContext & ) const
{
  element.setAttribute( QStringLiteral( "pointSize" ), qgsDoubleToString( mPointSize ) );
  element.setAttribute( QStringLiteral( "pointSizeUnit" ), QgsUnitTypes::encodeUnit( mPointSizeUnit ) );
  element.setAttribute( QStringLiteral( "pointSizeMapUnitScale" ), QgsSymbolLayerUtils::encodeMapUnitScale( mPointSizeMapUnitScale ) );

  element.setAttribute( QStringLiteral( "maximumScreenError" ), qgsDoubleToString( mMaximumScreenError ) );
  element.setAttribute( QStringLiteral( "maximumScreenErrorUnit" ), QgsUnitTypes::encodeUnit( mMaximumScreenErrorUnit ) );
}


///@cond PRIVATE

#include "qgscolorramp.h"
#include "qgspointcloudblock.h"
#include "qgsstyle.h"

QgsDummyPointCloudRenderer::QgsDummyPointCloudRenderer()
{
  mColorRamp.reset( QgsStyle::defaultStyle()->colorRamp( QStringLiteral( "Viridis" ) ) );
}

QgsPointCloudRenderer *QgsDummyPointCloudRenderer::clone() const
{
  std::unique_ptr< QgsDummyPointCloudRenderer > res = qgis::make_unique< QgsDummyPointCloudRenderer >();

  res->mZMin = zMin();
  res->mZMax = zMax();
  res->mPenWidth = penWidth();
  res->mColorRamp.reset( colorRamp() ? colorRamp()->clone() : QgsStyle::defaultStyle()->colorRamp( QStringLiteral( "Viridis" ) ) );
  res->mAttribute = attribute();

  return res.release();
}

void QgsDummyPointCloudRenderer::renderBlock( const QgsPointCloudBlock *block, QgsPointCloudRenderContext &context )
{
  const QgsMapToPixel mapToPixel = context.renderContext().mapToPixel();
  const QgsVector3D scale = context.scale();
  const QgsVector3D offset = context.offset();

  QgsRectangle mapExtent = context.renderContext().mapExtent();

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

  const QgsPointCloudAttribute::DataType type = attribute->type();
  const bool applyZOffset = attribute->name() == QLatin1String( "Z" );

  int rendered = 0;
  double x = 0;
  double y = 0;
  for ( int i = 0; i < count; ++i )
  {
    pointXY( context, ptr, i, x, y );

    if ( mapExtent.contains( QgsPointXY( x, y ) ) )
    {
      double atr = 0;
      switch ( type )
      {
        case QgsPointCloudAttribute::Char:
          continue;

        case QgsPointCloudAttribute::Int32:
          atr = *( qint32 * )( ptr + i * recordSize + attributeOffset );
          break;

        case QgsPointCloudAttribute::Short:
          atr = *( short * )( ptr + i * recordSize + attributeOffset );
          break;

        case QgsPointCloudAttribute::Float:
          atr = *( float * )( ptr + i * recordSize + attributeOffset );
          break;

        case QgsPointCloudAttribute::Double:
          atr = *( double * )( ptr + i * recordSize + attributeOffset );
          break;
      }

      if ( applyZOffset )
        atr = offset.z() + scale.z() * atr;

      mapToPixel.transformInPlace( x, y );

      pen.setColor( colorRamp()->color( ( atr - zMin() ) / ( zMax() - zMin() ) ) );
      context.renderContext().painter()->setPen( pen );
      context.renderContext().painter()->drawPoint( QPointF( x, y ) );

      rendered++;
    }
  }
  context.incrementPointsRendered( rendered );
}

#include "qgssymbollayerutils.h"


QgsPointCloudRenderer *QgsDummyPointCloudRenderer::create( QDomElement &element, const QgsReadWriteContext & )
{
  std::unique_ptr< QgsDummyPointCloudRenderer > r = qgis::make_unique< QgsDummyPointCloudRenderer >();

  r->setAttribute( element.attribute( QStringLiteral( "attribute" ) ) );
  r->setZMin( element.attribute( QStringLiteral( "min" ), QStringLiteral( "0" ) ).toDouble() );
  r->setZMax( element.attribute( QStringLiteral( "max" ), QStringLiteral( "100" ) ).toDouble() );
  r->setPenWidth( element.attribute( QStringLiteral( "penwidth" ), QStringLiteral( "5" ) ).toInt() );

  QDomElement sourceColorRampElem = element.firstChildElement( QStringLiteral( "colorramp" ) );
  if ( !sourceColorRampElem.isNull() && sourceColorRampElem.attribute( QStringLiteral( "name" ) ) == QLatin1String( "[source]" ) )
  {
    r->setColorRamp( QgsSymbolLayerUtils::loadColorRamp( sourceColorRampElem ) );
  }

  return r.release();
}

QDomElement QgsDummyPointCloudRenderer::save( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( context )
  QDomElement rendererElem = doc.createElement( QStringLiteral( "renderer" ) );

  rendererElem.setAttribute( QStringLiteral( "type" ), QStringLiteral( "dummy" ) );
  rendererElem.setAttribute( QStringLiteral( "penwidth" ), mPenWidth );
  rendererElem.setAttribute( QStringLiteral( "min" ), mZMin );
  rendererElem.setAttribute( QStringLiteral( "max" ), mZMax );
  rendererElem.setAttribute( QStringLiteral( "attribute" ), mAttribute );

  QDomElement colorRampElem = QgsSymbolLayerUtils::saveColorRamp( QStringLiteral( "[source]" ), mColorRamp.get(), doc );
  rendererElem.appendChild( colorRampElem );

  return rendererElem;
}

void QgsDummyPointCloudRenderer::startRender( QgsPointCloudRenderContext &context )
{
  QgsPointCloudRenderer::startRender( context );

  mPainterPenWidth = context.renderContext().convertToPainterUnits( mPenWidth, QgsUnitTypes::RenderUnit::RenderMillimeters );
}

void QgsDummyPointCloudRenderer::stopRender( QgsPointCloudRenderContext &context )
{
  QgsPointCloudRenderer::stopRender( context );
}

QSet<QString> QgsDummyPointCloudRenderer::usedAttributes( const QgsPointCloudRenderContext & ) const
{
  return QSet<QString>() << mAttribute;
}

double QgsDummyPointCloudRenderer::zMin() const
{
  return mZMin;
}

void QgsDummyPointCloudRenderer::setZMin( double value )
{
  mZMin = value;
}

double QgsDummyPointCloudRenderer::zMax() const
{
  return mZMax;
}

void QgsDummyPointCloudRenderer::setZMax( double value )
{
  mZMax = value;
}

int QgsDummyPointCloudRenderer::penWidth() const
{
  return mPenWidth;
}

void QgsDummyPointCloudRenderer::setPenWidth( int value )
{
  mPenWidth = value;
}

QgsColorRamp *QgsDummyPointCloudRenderer::colorRamp() const
{
  return mColorRamp.get();
}

void QgsDummyPointCloudRenderer::setColorRamp( QgsColorRamp *value )
{
  mColorRamp.reset( value );
}

float QgsDummyPointCloudRenderer::maximumScreenError() const
{
  return mMaximumScreenError;
}

QString QgsDummyPointCloudRenderer::attribute() const
{
  return mAttribute;
}

void QgsDummyPointCloudRenderer::setAttribute( const QString &attribute )
{
  mAttribute = attribute;
}

///@endcond
