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

#include "qgshighlight.h"
#include "qgsgeometry.h"
#include "qgsmapcanvas.h"
#include "qgsmaprenderer.h"
#include "qgscoordinatetransform.h"
#include "qgsvectorlayer.h"
#include <QPainter>

/*!
  \class QgsHighlight
  \brief The QgsHighlight class provides a transparent overlay widget
  for highlightng features on the map.
*/
QgsHighlight::QgsHighlight( QgsMapCanvas* mapCanvas, QgsGeometry *geom, QgsVectorLayer *layer )
    : QgsMapCanvasItem( mapCanvas )
    , mLayer( layer )
{
  mGeometry = geom ? new QgsGeometry( *geom ) : 0;
  if ( mapCanvas->mapRenderer()->hasCrsTransformEnabled() )
  {
    QgsCoordinateTransform transform( mLayer->crs(), mapCanvas->mapRenderer()->destinationCrs() );
    mGeometry->transform( transform );
  }
  updateRect();
  update();
  setColor( QColor( Qt::lightGray ) );
}

QgsHighlight::~QgsHighlight()
{
  delete mGeometry;
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
  QPolygonF poly;

  // just ring outlines, no fill
  p->setPen( mPen );
  p->setBrush( Qt::NoBrush );

  for ( int i = 0; i < polygon.size(); i++ )
  {
    QPolygonF ring( polygon[i].size() + 1 );

    for ( int j = 0; j < polygon[i].size(); j++ )
    {
      ring[ j ] = toCanvasCoordinates( polygon[i][j] ) - pos();
    }

    ring[ polygon[i].size()] = ring[ 0 ];

    p->drawPolygon( ring );

    if ( i == 0 )
      poly = ring;
    else
      poly = poly.subtracted( ring );
  }

  // just fill, no outline
  p->setPen( Qt::NoPen );
  p->setBrush( mBrush );
  p->drawPolygon( poly );
}

/*!
  Draw the shape in response to an update event.
  */
void QgsHighlight::paint( QPainter* p )
{
  if ( !mGeometry )
  {
    return;
  }

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
  else
  {
    setRect( QgsRectangle() );
  }
}
