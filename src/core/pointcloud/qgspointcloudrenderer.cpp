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

#include <QElapsedTimer>

#include "qgspointcloudrenderer.h"
#include "qgspointcloudlayer.h"
#include "qgsrendercontext.h"
#include "qgspointcloudindex.h"
#include "qgsstyle.h"
#include "qgscolorramp.h"
#include "qgspointcloudrequest.h"
#include "qgspointcloudattribute.h"
#include "qgslogger.h"

///@cond PRIVATE
QgsPointCloudRendererConfig::QgsPointCloudRendererConfig() = default;

QgsPointCloudRendererConfig::QgsPointCloudRendererConfig( const QgsPointCloudRendererConfig &other )
{
  mZMin = other.zMin();
  mZMax = other.zMax();
  mPenWidth = other.penWidth();
  mColorRamp.reset( other.colorRamp()->clone() );
}

QgsPointCloudRendererConfig &QgsPointCloudRendererConfig::QgsPointCloudRendererConfig::operator =( const QgsPointCloudRendererConfig &other )
{
  mZMin = other.zMin();
  mZMax = other.zMax();
  mPenWidth = other.penWidth();
  mColorRamp.reset( other.colorRamp()->clone() );
  return *this;
}

double QgsPointCloudRendererConfig::zMin() const
{
  return mZMin;
}

void QgsPointCloudRendererConfig::setZMin( double value )
{
  mZMin = value;
}

double QgsPointCloudRendererConfig::zMax() const
{
  return mZMax;
}

void QgsPointCloudRendererConfig::setZMax( double value )
{
  mZMax = value;
}

int QgsPointCloudRendererConfig::penWidth() const
{
  return mPenWidth;
}

void QgsPointCloudRendererConfig::setPenWidth( int value )
{
  mPenWidth = value;
}

QgsColorRamp *QgsPointCloudRendererConfig::colorRamp() const
{
  return mColorRamp.get();
}

void QgsPointCloudRendererConfig::setColorRamp( QgsColorRamp *value )
{
  mColorRamp.reset( value );
}

float QgsPointCloudRendererConfig::maximumScreenError() const
{
  return mMaximumScreenError;
}

QString QgsPointCloudRendererConfig::attribute() const
{
  return mAttribute;
}

void QgsPointCloudRendererConfig::setAttribute( const QString &attribute )
{
  mAttribute = attribute;
}

///@endcond

QgsPointCloudLayerRenderer::QgsPointCloudLayerRenderer( QgsPointCloudLayer *layer, QgsRenderContext &context )
  : QgsMapLayerRenderer( layer->id(), &context )
  , mLayer( layer )
{
  // TODO: use config from layer
  mConfig.setPenWidth( context.convertToPainterUnits( 1, QgsUnitTypes::RenderUnit::RenderMillimeters ) );
  // good range for 26850_12580.laz
  mConfig.setZMin( layer->customProperty( QStringLiteral( "pcMin" ), 400 ).toInt() );
  mConfig.setZMax( layer->customProperty( QStringLiteral( "pcMax" ), 600 ).toInt() );
  mConfig.setColorRamp( QgsStyle::defaultStyle()->colorRamp( layer->customProperty( QStringLiteral( "pcRamp" ), QStringLiteral( "Viridis" ) ).toString() ) );
  mConfig.setAttribute( layer->customProperty( QStringLiteral( "pcAttribute" ), QStringLiteral( "Z" ) ).toString() );

  // TODO: we must not keep pointer to mLayer (it's dangerous) - we must copy anything we need for rendering
  // or use some locking to prevent read/write from multiple threads
  if ( !mLayer || !mLayer->dataProvider() || !mLayer->dataProvider()->index() )
    return;

  mAttributes.push_back( QgsPointCloudAttribute( QStringLiteral( "X" ), QgsPointCloudAttribute::Int32 ) );
  mAttributes.push_back( QgsPointCloudAttribute( QStringLiteral( "Y" ), QgsPointCloudAttribute::Int32 ) );

  int offset;
  const QgsPointCloudAttribute *renderAttribute = mLayer->attributes().find( mConfig.attribute(), offset );
  if ( !renderAttribute )
    return;

  mAttributes.push_back( *renderAttribute );
}

bool QgsPointCloudLayerRenderer::render()
{
  // TODO cache!?
  QgsPointCloudIndex *pc = mLayer->dataProvider()->index();
  if ( !pc )
    return false;

  QgsRenderContext &context = *renderContext();

  // Set up the render configuration options
  QPainter *painter = context.painter();

  painter->save();
  context.setPainterFlagsUsingContext( painter );

  QgsPointCloudDataBounds db;

  QElapsedTimer t;
  t.start();

  const IndexedPointCloudNode root = pc->root();
  float maximumError = mConfig.maximumScreenError(); // in pixels
  float rootError = pc->nodeError( root ); // in map coords
  double mapUnitsPerPixel = context.mapToPixel().mapUnitsPerPixel();
  if ( ( rootError < 0.0 ) || ( mapUnitsPerPixel < 0.0 ) || ( maximumError < 0.0 ) )
  {
    qDebug() << "invalid screen error";
    return false;
  }
  float rootErrorPixels = rootError / mapUnitsPerPixel; // in pixels
  const QList<IndexedPointCloudNode> nodes = traverseTree( pc, context, pc->root(), maximumError, rootErrorPixels );

  // drawing
  for ( const IndexedPointCloudNode &n : nodes )
  {
    if ( context.renderingStopped() )
    {
      qDebug() << "canceled";
      break;
    }
    QgsPointCloudRequest request;
    request.setAttributes( mAttributes );
    std::unique_ptr<QgsPointCloudBlock> block( pc->nodeData( n, request ) );
    drawData( painter, block.get(), mConfig );
  }

  qDebug() << "totals:" << nodesDrawn << "nodes | " << pointsDrawn << " points | " << t.elapsed() << "ms";

  painter->restore();

  return true;
}

QList<IndexedPointCloudNode> QgsPointCloudLayerRenderer::traverseTree( const QgsPointCloudIndex *pc,
    const QgsRenderContext &context,
    IndexedPointCloudNode n,
    float maxErrorPixels,
    float nodeErrorPixels )
{
  QList<IndexedPointCloudNode> nodes;

  if ( context.renderingStopped() )
  {
    qDebug() << "canceled";
    return nodes;
  }

  if ( !context.mapExtent().intersects( pc->nodeMapExtent( n ) ) )
    return nodes;

  nodes.append( n );

  float childrenErrorPixels = nodeErrorPixels / 2.0f;
  if ( childrenErrorPixels < maxErrorPixels )
    return nodes;

  const QList<IndexedPointCloudNode> children = pc->nodeChildren( n );
  for ( const IndexedPointCloudNode &nn : children )
  {
    nodes += traverseTree( pc, context, nn, maxErrorPixels, childrenErrorPixels );
  }

  return nodes;
}

QgsPointCloudLayerRenderer::~QgsPointCloudLayerRenderer() = default;

void QgsPointCloudLayerRenderer::drawData( QPainter *painter, const QgsPointCloudBlock *data, const QgsPointCloudRendererConfig &config )
{
  Q_ASSERT( mLayer->dataProvider() );
  Q_ASSERT( mLayer->dataProvider()->index() );

  if ( !data )
    return;

  const QgsMapToPixel mapToPixel = renderContext()->mapToPixel();
  const QgsVector3D scale = mLayer->dataProvider()->index()->scale();
  const QgsVector3D offset = mLayer->dataProvider()->index()->offset();

  QgsRectangle mapExtent = renderContext()->mapExtent();

  QPen pen;
  pen.setWidth( config.penWidth() );
  pen.setCapStyle( Qt::FlatCap );
  //pen.setJoinStyle( Qt::MiterJoin );

  const char *ptr = data->data();
  int count = data->pointCount();
  const QgsPointCloudAttributeCollection request = data->attributes();
  const std::size_t recordSize = request.pointRecordSize();

  for ( int i = 0; i < count; ++i )
  {
    // TODO clean up!
    qint32 ix = *( qint32 * )( ptr + i * recordSize + 0 );
    qint32 iy = *( qint32 * )( ptr + i * recordSize + 4 );

    double x = offset.x() + scale.x() * ix;
    double y = offset.y() + scale.y() * iy;
    if ( mapExtent.contains( QgsPointXY( x, y ) ) )
    {
      double atr = 0;
      switch ( mAttributes.at( 2 ).type() )
      {
        case QgsPointCloudAttribute::Char:
          continue;

        case QgsPointCloudAttribute::Int32:
          atr = *( qint32 * )( ptr + i * recordSize + 8 );
          break;

        case QgsPointCloudAttribute::Short:
          atr = *( short * )( ptr + i * recordSize + 8 );
          break;

        case QgsPointCloudAttribute::Float:
          atr = *( float * )( ptr + i * recordSize + 8 );
          break;

        case QgsPointCloudAttribute::Double:
          atr = *( double * )( ptr + i * recordSize + 8 );
          break;
      }

      if ( mAttributes.at( 2 ).name() == QLatin1String( "Z" ) )
        atr = offset.z() + scale.z() * atr;

      mapToPixel.transformInPlace( x, y );

      pen.setColor( config.colorRamp()->color( ( atr - config.zMin() ) / ( config.zMax() - config.zMin() ) ) );
      painter->setPen( pen );
      painter->drawPoint( QPointF( x, y ) );
    }
  }

  // stats
  ++nodesDrawn;
  pointsDrawn += count;
}
