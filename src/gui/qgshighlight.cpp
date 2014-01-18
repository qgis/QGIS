/***************************************************************************
    qgshighlight.cpp - widget to highlight features on the map
     --------------------------------------
    Date                 : 02-03-2011
    Copyright            : (C) 2011 by Juergen E. Fischer, norBIT GmbH
    Email                : jef at norbit dot de
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <typeinfo>
#include "qgsmarkersymbollayerv2.h"
#include "qgslinesymbollayerv2.h"

#include "qgscoordinatetransform.h"
#include "qgsgeometry.h"
#include "qgshighlight.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsmaprenderer.h"
#include "qgsrendercontext.h"
#include "qgssymbollayerv2.h"
#include "qgssymbolv2.h"
#include "qgsvectorlayer.h"

/*!
  \class QgsHighlight
  \brief The QgsHighlight class provides a transparent overlay widget
  for highlightng features on the map.
*/
QgsHighlight::QgsHighlight( QgsMapCanvas* mapCanvas, QgsGeometry *geom, QgsMapLayer *layer )
    : QgsMapCanvasItem( mapCanvas )
    , mLayer( layer )
    , mRenderer( 0 )
{
  mGeometry = geom ? new QgsGeometry( *geom ) : 0;
  init();
}

QgsHighlight::QgsHighlight( QgsMapCanvas* mapCanvas, QgsGeometry *geom, QgsVectorLayer *layer )
    : QgsMapCanvasItem( mapCanvas )
    , mLayer( static_cast<QgsMapLayer *>( layer ) )
    , mRenderer( 0 )
{
  mGeometry = geom ? new QgsGeometry( *geom ) : 0;
  init();
}

QgsHighlight::QgsHighlight( QgsMapCanvas* mapCanvas, const QgsFeature& feature, QgsVectorLayer *layer )
    : QgsMapCanvasItem( mapCanvas )
    , mGeometry( 0 )
    , mLayer( static_cast<QgsMapLayer *>( layer ) )
    , mFeature( feature )
    , mRenderer( 0 )
{
  init();
}

void QgsHighlight::init()
{
  if ( mMapCanvas->mapRenderer()->hasCrsTransformEnabled() )
  {
    const QgsCoordinateTransform* ct = mMapCanvas->mapRenderer()->transformation( mLayer );
    if ( ct )
    {
      if ( mGeometry )
      {
        mGeometry->transform( *ct );
      }
      else if ( mFeature.geometry() )
      {
        mFeature.geometry()->transform( *ct );
      }
    }
  }
  updateRect();
  update();
  setColor( QColor( Qt::lightGray ) );
}

QgsHighlight::~QgsHighlight()
{
  delete mGeometry;
  delete mRenderer;
}

/*!
  Set the outline and fill color.
  */
void QgsHighlight::setColor( const QColor & color )
{
  mPen.setColor( color );
  QColor fillColor( color.red(), color.green(), color.blue(), 63 );
  mBrush.setColor( fillColor );
  mBrush.setStyle( Qt::SolidPattern );

  delete mRenderer;
  mRenderer = 0;
  QgsVectorLayer *layer = vectorLayer();
  if ( layer && layer->rendererV2() )
  {
    mRenderer = layer->rendererV2()->clone();
  }
  if ( mRenderer )
  {
    foreach ( QgsSymbolV2* symbol, mRenderer->symbols() )
    {
      if ( !symbol ) continue;
      setSymbolColor( symbol, color );
    }
  }
}

void QgsHighlight::setSymbolColor( QgsSymbolV2* symbol, const QColor & color )
{
  if ( !symbol ) return;

  QColor fillColor( color.red(), color.green(), color.blue(), 63 );

  for ( int i = symbol->symbolLayerCount() - 1; i >= 0;  i-- )
  {
    QgsSymbolLayerV2* symbolLayer = symbol->symbolLayer( i );
    if ( !symbolLayer ) continue;

    if ( symbolLayer->subSymbol() )
    {
      setSymbolColor( symbolLayer->subSymbol(), color );
    }
    else
    {
      // We must insert additional highlight symbol layer above each original layer
      // otherwise lower layers would become visible through transparent fill color.
      QgsSymbolLayerV2* highlightLayer = symbolLayer->clone();

      highlightLayer->setColor( color ); // line symbology layers
      highlightLayer->setOutlineColor( color ); // marker and fill symbology layers
      highlightLayer->setFillColor( fillColor ); // marker and fill symbology layers
      symbol->insertSymbolLayer( i + 1, highlightLayer );
    }
  }
}

/*!
  Set the outline width.
  */
void QgsHighlight::setWidth( int width )
{
  mPen.setWidth( width );
}

void QgsHighlight::paintPoint( QPainter *p, QgsPoint point )
{
  QPolygonF r( 5 );

  double d = mMapCanvas->extent().width() * 0.005;
  r[0] = toCanvasCoordinates( point + QgsVector( -d, -d ) ) - pos();
  r[1] = toCanvasCoordinates( point + QgsVector( d, -d ) ) - pos();
  r[2] = toCanvasCoordinates( point + QgsVector( d, d ) ) - pos();
  r[3] = toCanvasCoordinates( point + QgsVector( -d, d ) ) - pos();
  r[4] = r[0];

  p->drawPolygon( r );
}

void QgsHighlight::paintLine( QPainter *p, QgsPolyline line )
{
  QPolygonF polygon( line.size() );

  for ( int i = 0; i < line.size(); i++ )
  {
    polygon[i] = toCanvasCoordinates( line[i] ) - pos();
  }

  p->drawPolyline( polygon );
}

void QgsHighlight::paintPolygon( QPainter *p, QgsPolygon polygon )
{
  // OddEven fill rule by default
  QPainterPath path;

  p->setPen( mPen );
  p->setBrush( mBrush );

  for ( int i = 0; i < polygon.size(); i++ )
  {
    if ( polygon[i].empty() ) continue;

    QPolygonF ring;
    ring.reserve( polygon[i].size() + 1 );

    for ( int j = 0; j < polygon[i].size(); j++ )
    {
      //adding point only if it is more than a pixel appart from the previous one
      const QPointF cur = toCanvasCoordinates( polygon[i][j] ) - pos();
      if ( 0 == j || std::abs( ring.back().x() - cur.x() ) > 1 || std::abs( ring.back().y() - cur.y() ) > 1 )
      {
        ring.push_back( cur );
      }
    }

    ring.push_back( ring[ 0 ] );

    path.addPolygon( ring );
  }

  p->drawPath( path );
}

/*!
  Draw the shape in response to an update event.
  */
void QgsHighlight::paint( QPainter* p )
{
  if ( mGeometry )
  {
    p->setPen( mPen );
    p->setBrush( mBrush );

    switch ( mGeometry->wkbType() )
    {
      case QGis::WKBPoint:
      case QGis::WKBPoint25D:
      {
        paintPoint( p, mGeometry->asPoint() );
      }
      break;

      case QGis::WKBMultiPoint:
      case QGis::WKBMultiPoint25D:
      {
        QgsMultiPoint m = mGeometry->asMultiPoint();
        for ( int i = 0; i < m.size(); i++ )
        {
          paintPoint( p, m[i] );
        }
      }
      break;

      case QGis::WKBLineString:
      case QGis::WKBLineString25D:
      {
        paintLine( p, mGeometry->asPolyline() );
      }
      break;

      case QGis::WKBMultiLineString:
      case QGis::WKBMultiLineString25D:
      {
        QgsMultiPolyline m = mGeometry->asMultiPolyline();

        for ( int i = 0; i < m.size(); i++ )
        {
          paintLine( p, m[i] );
        }
      }
      break;

      case QGis::WKBPolygon:
      case QGis::WKBPolygon25D:
      {
        paintPolygon( p, mGeometry->asPolygon() );
      }
      break;

      case QGis::WKBMultiPolygon:
      case QGis::WKBMultiPolygon25D:
      {
        QgsMultiPolygon m = mGeometry->asMultiPolygon();
        for ( int i = 0; i < m.size(); i++ )
        {
          paintPolygon( p, m[i] );
        }
      }
      break;

      case QGis::WKBUnknown:
      default:
        return;
    }
  }
  else if ( mFeature.geometry() && mRenderer )
  {
    QgsVectorLayer *layer = vectorLayer();
    if ( layer )
    {
      QgsRenderContext context = *( mMapCanvas->mapRenderer()->rendererContext() );

      // The context is local rectangle of QgsHighlight we previously set.
      // Because QgsMapCanvasItem::setRect() adds 1 pixel on border we cannot simply
      // use boundingRect().height() for QgsMapToPixel height.
      QgsRectangle extent = mMapCanvas->extent();
      if ( extent != rect() ) // catches also canvas resize as it is causing extent change
      {
        updateRect();
        return; // it will be repainted after updateRect()
      }
      double height = toCanvasCoordinates( QgsPoint( extent.xMinimum(), extent.yMinimum() ) ).y() - toCanvasCoordinates( QgsPoint( extent.xMinimum(), extent.yMaximum() ) ).y();

      QgsMapToPixel mapToPixel = QgsMapToPixel( mMapCanvas->mapUnitsPerPixel(),
                                 height, extent.yMinimum(), extent.xMinimum() );
      context.setMapToPixel( mapToPixel );
      context.setExtent( extent );
      context.setCoordinateTransform( 0 ); // we reprojected geometry in init()
      context.setPainter( p );
      mRenderer->startRender( context, layer );
      mRenderer->renderFeature( mFeature, context );
      mRenderer->stopRender( context );
    }
  }
}

void QgsHighlight::updateRect()
{
  if ( mGeometry )
  {
    QgsRectangle r = mGeometry->boundingBox();

    if ( r.isEmpty() )
    {
      double d = mMapCanvas->extent().width() * 0.005;
      r.setXMinimum( r.xMinimum() - d );
      r.setYMinimum( r.yMinimum() - d );
      r.setXMaximum( r.xMaximum() + d );
      r.setYMaximum( r.yMaximum() + d );
    }

    setRect( r );
    setVisible( mGeometry );
  }
  else if ( mFeature.geometry() )
  {
    // We are currently using full map canvas extent for two reasons:
    // 1) currently there is no method in QgsFeatureRendererV2 to get rendered feature
    //    bounding box
    // 2) using different extent would result in shifted fill patterns
    setRect( mMapCanvas->extent() );
    setVisible( true );
  }
  else
  {
    setRect( QgsRectangle() );
  }
}

QgsVectorLayer * QgsHighlight::vectorLayer()
{
  return dynamic_cast<QgsVectorLayer *>( mLayer );
}
