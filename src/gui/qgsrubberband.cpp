/***************************************************************************
    qgsrubberband.cpp - Rubberband widget for drawing multilines and polygons
     --------------------------------------
    Date                 : 07-Jan-2006
    Copyright            : (C) 2006 by Tom Elwertowski
    Email                : telwertowski at users dot sourceforge dot net
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrubberband.h"
#include "qgsfeature.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmaprenderer.h"
#include "qgsvectorlayer.h"
#include <QPainter>

/*!
  \class QgsRubberBand
  \brief The QgsRubberBand class provides a transparent overlay widget
  for tracking the mouse while drawing polylines or polygons.
*/
QgsRubberBand::QgsRubberBand( QgsMapCanvas* mapCanvas, QGis::GeometryType geometryType )
    : QgsMapCanvasItem( mapCanvas )
    , mGeometryType( geometryType )
    , mTranslationOffsetX( 0.0 )
    , mTranslationOffsetY( 0.0 )
{
  reset( geometryType );
  setColor( QColor( Qt::lightGray ) );
}

QgsRubberBand::QgsRubberBand( QgsMapCanvas* mapCanvas, bool isPolygon )
    : QgsMapCanvasItem( mapCanvas )
{
  QgsRubberBand( mapCanvas, isPolygon ? QGis::Polygon : QGis::Line );
}

QgsRubberBand::QgsRubberBand(): QgsMapCanvasItem( 0 )
{
}

QgsRubberBand::~QgsRubberBand()
{
}

/*!
  Set the outline and fill color.
  */
void QgsRubberBand::setColor( const QColor & color )
{
  mPen.setColor( color );
  QColor fillColor( color.red(), color.green(), color.blue(), 63 );
  mBrush.setColor( fillColor );
  mBrush.setStyle( Qt::SolidPattern );
}

/*!
  Set the outline width.
  */
void QgsRubberBand::setWidth( int width )
{
  mWidth = width;
}

/*!
  Remove all points from the shape being created.
  */
void QgsRubberBand::reset( QGis::GeometryType geometryType )
{
  mPoints.clear();
  mGeometryType = geometryType;
  updateRect();
  update();
}

void QgsRubberBand::reset( bool isPolygon )
{
  mPoints.clear();
  mGeometryType = isPolygon ? QGis::Polygon : QGis::Line;
  updateRect();
  update();
}

/*!
  Add a point to the shape being created.
  */
void QgsRubberBand::addPoint( const QgsPoint & p, bool do_update /* = true */, int geometryIndex )
{
  if ( geometryIndex < 0 )
  {
    geometryIndex = mPoints.size() - 1;
  }

  if ( geometryIndex < 0 || geometryIndex > mPoints.size() )
  {
    return;
  }

  if ( geometryIndex == mPoints.size() )
  {
    mPoints.push_back( QList<QgsPoint>() << p );
  }

  if ( mPoints[geometryIndex].size() == 2 &&
       mPoints[geometryIndex][0] == mPoints[geometryIndex][1] )
  {
    mPoints[geometryIndex].last() = p;
  }
  else
  {
    mPoints[geometryIndex] << p;
  }


  if ( do_update )
  {
    updateRect();
    update();
  }
}

void QgsRubberBand::removeLastPoint( int geometryIndex )
{
  if ( mPoints.size() < geometryIndex + 1 )
  {
    return;
  }

  if ( mPoints[geometryIndex].size() > 0 )
  {
    mPoints[geometryIndex].pop_back();
  }

  updateRect();
  update();
}

/*!
  Update the line between the last added point and the mouse position.
  */
void QgsRubberBand::movePoint( const QgsPoint & p, int geometryIndex )
{
  if ( mPoints.size() < geometryIndex + 1 )
  {
    return;
  }

  if ( mPoints.at( geometryIndex ).size() < 1 )
  {
    return;
  }

  mPoints[geometryIndex].last() = p;

  updateRect();
  update();
}

void QgsRubberBand::movePoint( int index, const QgsPoint& p, int geometryIndex )
{
  if ( mPoints.size() < geometryIndex + 1 )
  {
    return;
  }

  if ( mPoints.at( geometryIndex ).size() < index )
  {
    return;
  }

  mPoints[geometryIndex][index] = p;

  updateRect();
  update();
}

void QgsRubberBand::setToGeometry( QgsGeometry* geom, QgsVectorLayer* layer )
{
  reset( mGeometryType );
  addGeometry( geom, layer );
}

void QgsRubberBand::addGeometry( QgsGeometry* geom, QgsVectorLayer* layer )
{
  if ( !geom )
  {
    return;
  }

  //maprender object of canvas
  QgsMapRenderer* mr = mMapCanvas->mapRenderer();
  if ( !mr )
  {
    return;
  }

  int idx = mPoints.size();

  switch ( geom->wkbType() )
  {

    case QGis::WKBPoint:
    case QGis::WKBPoint25D:
    {
      double d = mMapCanvas->extent().width() * 0.005;
      QgsPoint pt;
      if ( layer )
      {
        pt = mr->layerToMapCoordinates( layer, geom->asPoint() );
      }
      else
      {
        pt = geom->asPoint();
      }
      addPoint( QgsPoint( pt.x() - d, pt.y() - d ), false, idx );
      addPoint( QgsPoint( pt.x() + d, pt.y() - d ), false, idx );
      addPoint( QgsPoint( pt.x() + d, pt.y() + d ), false, idx );
      addPoint( QgsPoint( pt.x() - d, pt.y() + d ), false, idx );
    }
    break;

    case QGis::WKBMultiPoint:
    case QGis::WKBMultiPoint25D:
    {
      double d = mMapCanvas->extent().width() * 0.005;
      QgsMultiPoint mpt = geom->asMultiPoint();
      for ( int i = 0; i < mpt.size(); ++i, ++idx )
      {
        QgsPoint pt = mpt[i];
        if ( layer )
        {
          addPoint( mr->layerToMapCoordinates( layer, QgsPoint( pt.x() - d, pt.y() - d ) ), false, idx );
          addPoint( mr->layerToMapCoordinates( layer, QgsPoint( pt.x() + d, pt.y() - d ) ), false, idx );
          addPoint( mr->layerToMapCoordinates( layer, QgsPoint( pt.x() + d, pt.y() + d ) ), false, idx );
          addPoint( mr->layerToMapCoordinates( layer, QgsPoint( pt.x() - d, pt.y() + d ) ), false, idx );
        }
        else
        {
          addPoint( QgsPoint( pt.x() - d, pt.y() - d ), false, idx );
          addPoint( QgsPoint( pt.x() + d, pt.y() - d ), false, idx );
          addPoint( QgsPoint( pt.x() + d, pt.y() + d ), false, idx );
          addPoint( QgsPoint( pt.x() - d, pt.y() + d ), false, idx );
        }
      }
    }
    break;

    case QGis::WKBLineString:
    case QGis::WKBLineString25D:
    {
      QgsPolyline line = geom->asPolyline();
      for ( int i = 0; i < line.count(); i++ )
      {
        if ( layer )
        {
          addPoint( mr->layerToMapCoordinates( layer, line[i] ), false, idx );
        }
        else
        {
          addPoint( line[i], false, idx );
        }
      }
    }
    break;

    case QGis::WKBMultiLineString:
    case QGis::WKBMultiLineString25D:
    {
      mPoints.clear();

      QgsMultiPolyline mline = geom->asMultiPolyline();
      for ( int i = 0; i < mline.size(); ++i, ++idx )
      {
        QgsPolyline line = mline[i];

        if ( line.size() == 0 )
        {
          --idx;
        }

        for ( int j = 0; j < line.size(); ++j )
        {
          if ( layer )
          {
            addPoint( mr->layerToMapCoordinates( layer, line[j] ), false, idx );
          }
          else
          {
            addPoint( line[j], false, idx );
          }
        }
      }
    }
    break;

    case QGis::WKBPolygon:
    case QGis::WKBPolygon25D:
    {
      QgsPolygon poly = geom->asPolygon();
      QgsPolyline line = poly[0];
      for ( int i = 0; i < line.count(); i++ )
      {
        if ( layer )
        {
          addPoint( mr->layerToMapCoordinates( layer, line[i] ), false, idx );
        }
        else
        {
          addPoint( line[i], false, idx );
        }
      }
    }
    break;

    case QGis::WKBMultiPolygon:
    case QGis::WKBMultiPolygon25D:
    {
      mPoints.clear();

      QgsMultiPolygon multipoly = geom->asMultiPolygon();
      for ( int i = 0; i < multipoly.size(); ++i, ++idx )
      {
        QgsPolygon poly = multipoly[i];
        QgsPolyline line = poly[0];
        for ( int j = 0; j < line.count(); ++j )
        {
          if ( layer )
          {
            addPoint( mr->layerToMapCoordinates( layer, line[j] ), false, idx );
          }
          else
          {
            addPoint( line[j], false, idx );
          }
        }
      }
    }
    break;

    case QGis::WKBUnknown:
    default:
      return;
  }

  updateRect();
  update();
}

void QgsRubberBand::setToCanvasRectangle( const QRect& rect )
{
  if ( !mMapCanvas )
  {
    return;
  }

  const QgsMapToPixel* transform = mMapCanvas->getCoordinateTransform();
  QgsPoint ll = transform->toMapCoordinates( rect.left(), rect.bottom() );
  QgsPoint ur = transform->toMapCoordinates( rect.right(), rect.top() );

  reset( QGis::Polygon );
  addPoint( ll, false );
  addPoint( QgsPoint( ur.x(), ll.y() ), false );
  addPoint( ur, false );
  addPoint( QgsPoint( ll.x(), ur.y() ), true );
}

/*!
  Draw the shape in response to an update event.
  */
void QgsRubberBand::paint( QPainter* p )
{
  QList<QgsPoint> currentList;
  if ( mPoints.size() > 0 )
  {
    p->setBrush( mBrush );

    for ( int i = 0; i < mPoints.size(); ++i )
    {
      QVector<QPointF> pts;
      QList<QgsPoint>::const_iterator it = mPoints.at( i ).constBegin();
      for ( ; it != mPoints.at( i ).constEnd(); ++it )
      {
        pts.append( toCanvasCoordinates( QgsPoint( it->x() + mTranslationOffsetX, it->y() + mTranslationOffsetY ) ) - pos() );
      }

      switch ( mGeometryType )
      {
        case QGis::Polygon:
        {
          mPen.setWidth( mWidth );
          p->setPen( mPen );
          p->drawPolygon( pts );
        }
        break;

        case QGis::Point:
        {
          mPen.setWidth( 1 );
          p->setPen( mPen );
          QVector<QPointF>::const_iterator ptIt = pts.constBegin();
          for( ; ptIt != pts.constEnd(); ++ptIt )
          {
            p->drawEllipse( (*ptIt).x() - mWidth/2, (*ptIt).y() - mWidth/2, mWidth, mWidth );
          }
        }
        break;

        case QGis::Line:
        default:
        {
          mPen.setWidth( mWidth );
          p->setPen( mPen );
          p->drawPolyline( pts );
        }
        break;
      }
    }
  }
}

void QgsRubberBand::updateRect()
{
  if ( mPoints.size() > 0 )
  {
    //initial point
    QList<QgsPoint>::const_iterator it = mPoints.at( 0 ).constBegin();
    if ( it == mPoints.at( 0 ).constEnd() )
    {
      return;
    }
    QgsRectangle r( it->x() + mTranslationOffsetX, it->y() + mTranslationOffsetY,
                    it->x() + mTranslationOffsetX, it->y() + mTranslationOffsetY );

    for ( int i = 0; i < mPoints.size(); ++i )
    {
      QList<QgsPoint>::const_iterator it = mPoints.at( i ).constBegin();
      for ( ; it != mPoints.at( i ).constEnd(); ++it )
      {
        r.combineExtentWith( it->x() + mTranslationOffsetX, it->y() + mTranslationOffsetY );
      }
    }
    setRect( r );
  }
  else
  {
    setRect( QgsRectangle() );
  }
  setVisible( mPoints.size() > 0 );
}

void QgsRubberBand::setTranslationOffset( double dx, double dy )
{
  mTranslationOffsetX = dx;
  mTranslationOffsetY = dy;
  updateRect();
}

int QgsRubberBand::size() const
{
  return mPoints.size();
}

int QgsRubberBand::numberOfVertices() const
{
  int count = 0;
  QList<QList<QgsPoint> >::const_iterator it = mPoints.constBegin();
  for ( ; it != mPoints.constEnd(); ++it )
  {
    QList<QgsPoint>::const_iterator iter = it->constBegin();
    for ( ; iter != it->constEnd(); ++iter )
    {
      ++count;
    }
  }
  return count;
}

const QgsPoint *QgsRubberBand::getPoint( int i, int j ) const
{
  if ( i < mPoints.size() && j < mPoints[i].size() )
    return &mPoints[i][j];
  else
    return 0;
}

QgsGeometry *QgsRubberBand::asGeometry()
{
  QgsGeometry *geom = NULL;

  switch( mGeometryType )
  {
    case QGis::Polygon:
    {
      QgsPolygon polygon;
      QList< QList<QgsPoint> >::const_iterator it = mPoints.constBegin();
      for ( ; it != mPoints.constEnd(); ++it )
      {
        polygon.append( getPolyline( *it ) );
      }
      geom = QgsGeometry::fromPolygon( polygon );
      break;
    }

    case QGis::Point:
    {
      QgsMultiPoint multiPoint;

      QList< QList<QgsPoint> >::const_iterator it = mPoints.constBegin();
      for ( ; it != mPoints.constEnd(); ++it )
      {
        multiPoint += getPolyline( *it );
      }
      geom = QgsGeometry::fromMultiPoint( multiPoint );
      break;
    }

    case QGis::Line:
    default:
    {
      if ( mPoints.size() > 0 )
      {
        if ( mPoints.size() > 1 )
        {
          QgsMultiPolyline multiPolyline;
          QList< QList<QgsPoint> >::const_iterator it = mPoints.constBegin();
          for ( ; it != mPoints.constEnd(); ++it )
          {
            multiPolyline.append( getPolyline( *it ) );
          }
          geom = QgsGeometry::fromMultiPolyline( multiPolyline );
        }
        else
        {
          geom = QgsGeometry::fromPolyline( getPolyline( mPoints[0] ) );
        }
      }
      break;
    }
  }
  return geom;
}

QgsPolyline QgsRubberBand::getPolyline( const QList<QgsPoint> & points )
{
  QgsPolyline polyline;
  QList<QgsPoint>::const_iterator iter = points.constBegin();
  for ( ; iter != points.constEnd(); ++iter )
  {
    polyline.append( *iter );
  }
  return polyline;
}
