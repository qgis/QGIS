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
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgsrectangle.h"
#include <QPainter>

QgsRubberBand::QgsRubberBand( QgsMapCanvas *mapCanvas, QgsWkbTypes::GeometryType geometryType )
  : QObject( nullptr )
  , QgsMapCanvasItem( mapCanvas )
  , mGeometryType( geometryType )
{
  reset( geometryType );
  QColor color( Qt::lightGray );
  color.setAlpha( 63 );
  setColor( color );
  setWidth( 1 );
  setLineStyle( Qt::SolidLine );
  setBrushStyle( Qt::SolidPattern );
  setSecondaryStrokeColor( QColor() );
}

QgsRubberBand::QgsRubberBand()
  : QObject( nullptr )
  , QgsMapCanvasItem( nullptr )
{
}

void QgsRubberBand::setColor( const QColor &color )
{
  setStrokeColor( color );
  setFillColor( color );
}

void QgsRubberBand::setFillColor( const QColor &color )
{
  if ( mBrush.color() == color )
    return;

  mBrush.setColor( color );
}

void QgsRubberBand::setStrokeColor( const QColor &color )
{
  mPen.setColor( color );
}

void QgsRubberBand::setSecondaryStrokeColor( const QColor &color )
{
  mSecondaryPen.setColor( color );
}

void QgsRubberBand::setWidth( int width )
{
  mPen.setWidth( width );
}

void QgsRubberBand::setIcon( IconType icon )
{
  mIconType = icon;
}

void QgsRubberBand::setSvgIcon( const QString &path, QPoint drawOffset )
{
  setIcon( ICON_SVG );
  mSvgRenderer = qgis::make_unique<QSvgRenderer>( path );
  mSvgOffset = drawOffset;
}

void QgsRubberBand::setIconSize( int iconSize )
{
  mIconSize = iconSize;
}

void QgsRubberBand::setLineStyle( Qt::PenStyle penStyle )
{
  mPen.setStyle( penStyle );
}

void QgsRubberBand::setBrushStyle( Qt::BrushStyle brushStyle )
{
  mBrush.setStyle( brushStyle );
}

void QgsRubberBand::reset( QgsWkbTypes::GeometryType geometryType )
{
  mPoints.clear();
  mGeometryType = geometryType;
  updateRect();
  update();
}

void QgsRubberBand::addPoint( const QgsPointXY &p, bool doUpdate /* = true */, int geometryIndex )
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
    mPoints.push_back( QList<QgsPointXY>() << p );
  }

  if ( mPoints.at( geometryIndex ).size() == 2 &&
       mPoints.at( geometryIndex ).at( 0 ) == mPoints.at( geometryIndex ).at( 1 ) )
  {
    mPoints[geometryIndex].last() = p;
  }
  else
  {
    mPoints[geometryIndex] << p;
  }


  if ( doUpdate )
  {
    setVisible( true );
    updateRect();
    update();
  }
}

void QgsRubberBand::closePoints( bool doUpdate, int geometryIndex )
{
  if ( geometryIndex < 0 || geometryIndex >= mPoints.size() )
  {
    return;
  }

  if ( mPoints.at( geometryIndex ).at( 0 ) != mPoints.at( geometryIndex ).at( mPoints.at( geometryIndex ).size() - 1 ) )
  {
    mPoints[geometryIndex] << mPoints.at( geometryIndex ).at( 0 );
  }

  if ( doUpdate )
  {
    setVisible( true );
    updateRect();
    update();
  }
}


void QgsRubberBand::removePoint( int index, bool doUpdate/* = true*/, int geometryIndex/* = 0*/ )
{

  if ( mPoints.size() < geometryIndex + 1 )
  {
    return;
  }


  if ( !mPoints[geometryIndex].isEmpty() )
  {
    // negative index removes from end, e.g., -1 removes last one
    if ( index < 0 )
    {
      index = mPoints.at( geometryIndex ).size() + index;
    }
    mPoints[geometryIndex].removeAt( index );
  }

  if ( doUpdate )
  {
    updateRect();
    update();
  }
}

void QgsRubberBand::removeLastPoint( int geometryIndex, bool doUpdate/* = true*/ )
{
  removePoint( -1, doUpdate, geometryIndex );
}

void QgsRubberBand::movePoint( const QgsPointXY &p, int geometryIndex )
{
  if ( mPoints.size() < geometryIndex + 1 )
  {
    return;
  }

  if ( mPoints.at( geometryIndex ).empty() )
  {
    return;
  }

  mPoints[geometryIndex].last() = p;

  updateRect();
  update();
}

void QgsRubberBand::movePoint( int index, const QgsPointXY &p, int geometryIndex )
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

void QgsRubberBand::setToGeometry( const QgsGeometry &geom, QgsVectorLayer *layer )
{
  if ( geom.isNull() )
  {
    reset( mGeometryType );
    return;
  }

  reset( geom.type() );
  addGeometry( geom, layer );
}

void QgsRubberBand::setToGeometry( const QgsGeometry &geom, const QgsCoordinateReferenceSystem &crs )
{
  if ( geom.isNull() )
  {
    reset( mGeometryType );
    return;
  }

  reset( geom.type() );
  addGeometry( geom, crs );
}

void QgsRubberBand::addGeometry( const QgsGeometry &geometry, QgsVectorLayer *layer )
{
  QgsGeometry geom = geometry;
  if ( layer )
  {
    QgsCoordinateTransform ct = mMapCanvas->mapSettings().layerTransform( layer );
    try
    {
      geom.transform( ct );
    }
    catch ( QgsCsException & )
    {
      return;
    }
  }

  addGeometry( geom );
}

void QgsRubberBand::addGeometry( const QgsGeometry &geometry, const QgsCoordinateReferenceSystem &crs )
{
  if ( geometry.isEmpty() )
  {
    return;
  }

  //maprender object of canvas
  const QgsMapSettings &ms = mMapCanvas->mapSettings();

  int idx = mPoints.size();

  QgsGeometry geom = geometry;
  if ( crs.isValid() )
  {
    QgsCoordinateTransform ct( crs, ms.destinationCrs(), QgsProject::instance() );
    geom.transform( ct );
  }

  QgsWkbTypes::Type geomType = geom.wkbType();
  if ( QgsWkbTypes::geometryType( geomType ) == QgsWkbTypes::PointGeometry && !QgsWkbTypes::isMultiType( geomType ) )
  {
    QgsPointXY pt = geom.asPoint();
    addPoint( pt, false, idx );
    removeLastPoint( idx, false );
  }
  else if ( QgsWkbTypes::geometryType( geomType ) == QgsWkbTypes::PointGeometry && QgsWkbTypes::isMultiType( geomType ) )
  {
    const QgsMultiPointXY mpt = geom.asMultiPoint();
    for ( const QgsPointXY &pt : mpt )
    {
      addPoint( pt, false, idx );
      removeLastPoint( idx, false );
      idx++;
    }
  }
  else if ( QgsWkbTypes::geometryType( geomType ) == QgsWkbTypes::LineGeometry && !QgsWkbTypes::isMultiType( geomType ) )
  {
    const QgsPolylineXY line = geom.asPolyline();
    for ( const QgsPointXY &pt : line )
    {
      addPoint( pt, false, idx );
    }
  }
  else if ( QgsWkbTypes::geometryType( geomType ) == QgsWkbTypes::LineGeometry && QgsWkbTypes::isMultiType( geomType ) )
  {
    const QgsMultiPolylineXY mline = geom.asMultiPolyline();
    for ( const QgsPolylineXY &line : mline )
    {
      if ( line.isEmpty() )
      {
        continue;
      }
      for ( const QgsPointXY &pt : line )
      {
        addPoint( pt, false, idx );
      }
      idx++;
    }
  }
  else if ( QgsWkbTypes::geometryType( geomType ) == QgsWkbTypes::PolygonGeometry && !QgsWkbTypes::isMultiType( geomType ) )
  {
    const QgsPolygonXY poly = geom.asPolygon();
    const QgsPolylineXY line = poly.at( 0 );
    for ( const QgsPointXY &pt : line )
    {
      addPoint( pt, false, idx );
    }
  }
  else if ( QgsWkbTypes::geometryType( geomType ) == QgsWkbTypes::PolygonGeometry && QgsWkbTypes::isMultiType( geomType ) )
  {
    const QgsMultiPolygonXY multipoly = geom.asMultiPolygon();
    for ( const QgsPolygonXY &poly : multipoly )
    {
      if ( poly.empty() )
        continue;

      const QgsPolylineXY line = poly.at( 0 );
      for ( const QgsPointXY &pt : line )
      {
        addPoint( pt, false, idx );
      }
      idx++;
    }
  }
  else
  {
    return;
  }

  setVisible( true );
  updateRect();
  update();
}

void QgsRubberBand::setToCanvasRectangle( QRect rect )
{
  if ( !mMapCanvas )
  {
    return;
  }

  const QgsMapToPixel *transform = mMapCanvas->getCoordinateTransform();
  QgsPointXY ll = transform->toMapCoordinates( rect.left(), rect.bottom() );
  QgsPointXY lr = transform->toMapCoordinates( rect.right(), rect.bottom() );
  QgsPointXY ul = transform->toMapCoordinates( rect.left(), rect.top() );
  QgsPointXY ur = transform->toMapCoordinates( rect.right(), rect.top() );

  reset( QgsWkbTypes::PolygonGeometry );
  addPoint( ll, false );
  addPoint( lr, false );
  addPoint( ur, false );
  addPoint( ul, true );
}

void QgsRubberBand::paint( QPainter *p )
{
  if ( mPoints.isEmpty() )
    return;

  QVector< QVector<QPointF> > shapes;
  shapes.reserve( mPoints.size() );
  for ( const QList<QgsPointXY> &line : qgis::as_const( mPoints ) )
  {
    QVector<QPointF> pts;
    pts.reserve( line.size() );
    for ( const QgsPointXY &pt : line )
    {
      const QPointF cur = toCanvasCoordinates( QgsPointXY( pt.x() + mTranslationOffsetX, pt.y() + mTranslationOffsetY ) ) - pos();
      if ( pts.empty() || std::abs( pts.back().x() - cur.x() ) > 1 ||  std::abs( pts.back().y() - cur.y() ) > 1 )
        pts.append( cur );
    }
    shapes << pts;
  }

  int iterations = mSecondaryPen.color().isValid() ? 2 : 1;
  for ( int i = 0; i < iterations; ++i )
  {
    if ( i == 0 && iterations > 1 )
    {
      // first iteration with multi-pen painting, so use secondary pen
      mSecondaryPen.setWidth( mPen.width() + 2 );
      p->setBrush( Qt::NoBrush );
      p->setPen( mSecondaryPen );
    }
    else
    {
      // "top" layer, use primary pen/brush
      p->setBrush( mBrush );
      p->setPen( mPen );
    }

    for ( const QVector<QPointF> &shape : qgis::as_const( shapes ) )
    {
      drawShape( p, shape );
    }
  }
}

void QgsRubberBand::drawShape( QPainter *p, const QVector<QPointF> &pts )
{
  switch ( mGeometryType )
  {
    case QgsWkbTypes::PolygonGeometry:
    {
      p->drawPolygon( pts );
    }
    break;

    case QgsWkbTypes::PointGeometry:
    {
      const auto constPts = pts;
      for ( QPointF pt : constPts )
      {
        double x = pt.x();
        double y = pt.y();

        qreal s = ( mIconSize - 1 ) / 2.0;

        switch ( mIconType )
        {
          case ICON_NONE:
            break;

          case ICON_CROSS:
            p->drawLine( QLineF( x - s, y, x + s, y ) );
            p->drawLine( QLineF( x, y - s, x, y + s ) );
            break;

          case ICON_X:
            p->drawLine( QLineF( x - s, y - s, x + s, y + s ) );
            p->drawLine( QLineF( x - s, y + s, x + s, y - s ) );
            break;

          case ICON_BOX:
            p->drawLine( QLineF( x - s, y - s, x + s, y - s ) );
            p->drawLine( QLineF( x + s, y - s, x + s, y + s ) );
            p->drawLine( QLineF( x + s, y + s, x - s, y + s ) );
            p->drawLine( QLineF( x - s, y + s, x - s, y - s ) );
            break;

          case ICON_FULL_BOX:
            p->drawRect( static_cast< int>( x - s ), static_cast< int >( y - s ), mIconSize, mIconSize );
            break;

          case ICON_CIRCLE:
            p->drawEllipse( static_cast< int >( x - s ), static_cast< int >( y - s ), mIconSize, mIconSize );
            break;

          case ICON_DIAMOND:
          case ICON_FULL_DIAMOND:
          {
            QPointF pts[] =
            {
              QPointF( x, y - s ),
              QPointF( x + s, y ),
              QPointF( x, y + s ),
              QPointF( x - s, y )
            };
            if ( mIconType == ICON_FULL_DIAMOND )
              p->drawPolygon( pts, 4 );
            else
              p->drawPolyline( pts, 4 );
            break;
          }

          case ICON_SVG:
            QRectF viewBox = mSvgRenderer->viewBoxF();
            QRectF r( mSvgOffset.x(), mSvgOffset.y(), viewBox.width(), viewBox.height() );
            p->save();
            p->translate( pt );
            mSvgRenderer->render( p, r );
            p->restore();
            break;
        }
      }
    }
    break;

    case QgsWkbTypes::LineGeometry:
    default:
    {
      p->drawPolyline( pts );
    }
    break;
  }
}

void QgsRubberBand::updateRect()
{
  if ( mPoints.empty() )
  {
    setRect( QgsRectangle() );
    setVisible( false );
    return;
  }

  const QgsMapToPixel &m2p = *( mMapCanvas->getCoordinateTransform() );

#if 0 // unused?
  double iconSize = ( mIconSize + 1 ) / 2.;
  if ( mSvgRenderer )
  {
    QRectF viewBox = mSvgRenderer->viewBoxF();
    iconSize = std::max( std::fabs( mSvgOffset.x() ) + .5 * viewBox.width(), std::fabs( mSvgOffset.y() ) + .5 * viewBox.height() );
  }
#endif

  qreal w = ( ( mIconSize - 1 ) / 2 + mPen.width() ); // in canvas units

  QgsRectangle r;  // in canvas units
  for ( const QList<QgsPointXY> &ring : qgis::as_const( mPoints ) )
  {
    for ( const QgsPointXY &point : ring )
    {
      QgsPointXY p( point.x() + mTranslationOffsetX, point.y() + mTranslationOffsetY );
      p = m2p.transform( p );
      QgsRectangle rect( p.x() - w, p.y() - w, p.x() + w, p.y() + w );

      if ( r.isEmpty() )
      {
        // Get rectangle of the first point
        r = rect;
      }
      else
      {
        r.combineExtentWith( rect );
      }
    }
  }

  // This is an hack to pass QgsMapCanvasItem::setRect what it
  // expects (encoding of position and size of the item)
  qreal res = m2p.mapUnitsPerPixel();
  QgsPointXY topLeft = m2p.toMapCoordinates( r.xMinimum(), r.yMinimum() );
  QgsRectangle rect( topLeft.x(), topLeft.y(), topLeft.x() + r.width()*res, topLeft.y() - r.height()*res );

  setRect( rect );
}

void QgsRubberBand::updatePosition()
{
  // re-compute rectangle
  // See https://github.com/qgis/QGIS/issues/20566
  // NOTE: could be optimized by saving map-extent
  //       of rubberband and simply re-projecting
  //       that to device-rectangle on "updatePosition"
  updateRect();
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

int QgsRubberBand::partSize( int geometryIndex ) const
{
  if ( geometryIndex < 0 || geometryIndex >= mPoints.size() ) return 0;
  return mPoints[geometryIndex].size();
}

int QgsRubberBand::numberOfVertices() const
{
  int count = 0;
  for ( const QList<QgsPointXY> &ring : qgis::as_const( mPoints ) )
  {
    count += ring.size();
  }
  return count;
}

const QgsPointXY *QgsRubberBand::getPoint( int i, int j ) const
{
  if ( i < mPoints.size() && j < mPoints[i].size() )
    return &mPoints[i][j];
  else
    return nullptr;
}

QgsGeometry QgsRubberBand::asGeometry() const
{
  QgsGeometry geom;

  switch ( mGeometryType )
  {
    case QgsWkbTypes::PolygonGeometry:
    {
      QgsPolygonXY polygon;
      for ( const QList<QgsPointXY> &ring : qgis::as_const( mPoints ) )
      {
        polygon.append( getPolyline( ring ) );
      }
      geom = QgsGeometry::fromPolygonXY( polygon );
      break;
    }

    case QgsWkbTypes::PointGeometry:
    {
      QgsMultiPointXY multiPoint;

      for ( const QList<QgsPointXY> &ring : qgis::as_const( mPoints ) )
      {
        multiPoint += getPolyline( ring );
      }
      geom = QgsGeometry::fromMultiPointXY( multiPoint );
      break;
    }

    case QgsWkbTypes::LineGeometry:
    default:
    {
      if ( !mPoints.isEmpty() )
      {
        if ( mPoints.size() > 1 )
        {
          QgsMultiPolylineXY multiPolyline;
          for ( const QList<QgsPointXY> &ring : qgis::as_const( mPoints ) )
          {
            multiPolyline.append( getPolyline( ring ) );
          }
          geom = QgsGeometry::fromMultiPolylineXY( multiPolyline );
        }
        else
        {
          geom = QgsGeometry::fromPolylineXY( getPolyline( mPoints.at( 0 ) ) );
        }
      }
      break;
    }
  }
  return geom;
}

QgsPolylineXY QgsRubberBand::getPolyline( const QList<QgsPointXY> &points )
{
  return points.toVector();
}
