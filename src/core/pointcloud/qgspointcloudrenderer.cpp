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

QgsPointCloudRenderer *QgsPointCloudRenderer::defaultRenderer()
{
  return new QgsDummyPointCloudRenderer();
}

QgsPointCloudRenderer *QgsPointCloudRenderer::load( QDomElement &element, const QgsReadWriteContext &context )
{
  if ( element.isNull() )
    return nullptr;

  // load renderer
  QString rendererType = element.attribute( QStringLiteral( "type" ) );

#if 0
  QgsRendererAbstractMetadata *m = QgsApplication::rendererRegistry()->rendererMetadata( rendererType );
  if ( !m )
    return nullptr;

  std::unique_ptr< QgsPointCloudRenderer > r( m->createRenderer( element, context ) );
  return r.release();
#endif
  return QgsDummyPointCloudRenderer::create( element, context );
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
  res->mColorRamp.reset( colorRamp()->clone() );
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

  int xOffset = 0;
  const QgsPointCloudAttribute *attribute = request.find( QStringLiteral( "X" ), xOffset );
  if ( !attribute )
    return;

  int yOffset = 0;
  attribute = request.find( QStringLiteral( "Y" ), yOffset );
  if ( !attribute )
    return;

  int attributeOffset = 0;
  attribute = request.find( mAttribute, attributeOffset );
  if ( !attribute )
    return;

  const QgsPointCloudAttribute::DataType type = attribute->type();
  const bool applyZOffset = attribute->name() == QLatin1String( "Z" );

  int rendered = 0;
  for ( int i = 0; i < count; ++i )
  {
    // TODO clean up!
    qint32 ix = *( qint32 * )( ptr + i * recordSize + xOffset );
    qint32 iy = *( qint32 * )( ptr + i * recordSize + yOffset );

    double x = offset.x() + scale.x() * ix;
    double y = offset.y() + scale.y() * iy;
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

