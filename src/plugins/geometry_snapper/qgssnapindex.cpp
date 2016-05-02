/***************************************************************************
 *  qgssnapindex.cpp                                                       *
 *  -------------------                                                    *
 *  copyright            : (C) 2014 by Sandro Mani / Sourcepole AG         *
 *  email                : smani@sourcepole.ch                             *
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssnapindex.h"
#include "qgsgeometryutils.h"
#include <qmath.h>
#include <limits>

QgsSnapIndex::PointSnapItem::PointSnapItem( const QgsSnapIndex::CoordIdx* _idx )
    : SnapItem( QgsSnapIndex::SnapPoint )
    , idx( _idx )
{}

QgsPointV2 QgsSnapIndex::PointSnapItem::getSnapPoint( const QgsPointV2 &/*p*/ ) const
{
  return idx->point();
}

///////////////////////////////////////////////////////////////////////////////

QgsSnapIndex::SegmentSnapItem::SegmentSnapItem( const QgsSnapIndex::CoordIdx* _idxFrom, const QgsSnapIndex::CoordIdx* _idxTo )
    : SnapItem( QgsSnapIndex::SnapSegment )
    , idxFrom( _idxFrom )
    , idxTo( _idxTo )
{}

QgsPointV2 QgsSnapIndex::SegmentSnapItem::getSnapPoint( const QgsPointV2 &p ) const
{
  return QgsGeometryUtils::projPointOnSegment( p, idxFrom->point(), idxTo->point() );
}

bool QgsSnapIndex::SegmentSnapItem::getIntersection( const QgsPointV2 &p1, const QgsPointV2 &p2, QgsPointV2& inter ) const
{
  const QgsPointV2& q1 = idxFrom->point(), & q2 = idxTo->point();
  QgsVector v( p2.x() - p1.x(), p2.y() - p1.y() );
  QgsVector w( q2.x() - q1.x(), q2.y() - q1.y() );
  double vl = v.length();
  double wl = w.length();

  if ( qFuzzyIsNull( vl ) || qFuzzyIsNull( wl ) )
  {
    return false;
  }
  v = v / vl;
  w = w / wl;

  double d = v.y() * w.x() - v.x() * w.y();

  if ( d == 0 )
    return false;

  double dx = q1.x() - p1.x();
  double dy = q1.y() - p1.y();
  double k = ( dy * w.x() - dx * w.y() ) / d;

  inter = QgsPointV2( p1.x() + v.x() * k, p1.y() + v.y() * k );

  double lambdav = QgsVector( inter.x() - p1.x(), inter.y() - p1.y() ) *  v;
  if ( lambdav < 0. + 1E-8 || lambdav > vl - 1E-8 )
    return false;

  double lambdaw = QgsVector( inter.x() - q1.x(), inter.y() - q1.y() ) * w;
  if ( lambdaw < 0. + 1E-8 || lambdaw >= wl - 1E-8 )
    return false;

  return true;
}

bool QgsSnapIndex::SegmentSnapItem::getProjection( const QgsPointV2 &p, QgsPointV2 &pProj )
{
  const QgsPointV2& s1 = idxFrom->point();
  const QgsPointV2& s2 = idxTo->point();
  double nx = s2.y() - s1.y();
  double ny = -( s2.x() - s1.x() );
  double t = ( p.x() * ny - p.y() * nx - s1.x() * ny + s1.y() * nx ) / (( s2.x() - s1.x() ) * ny - ( s2.y() - s1.y() ) * nx );
  if ( t < 0. || t > 1. )
  {
    return false;
  }
  pProj = QgsPointV2( s1.x() + ( s2.x() - s1.x() ) * t, s1.y() + ( s2.y() - s1.y() ) * t );
  return true;
}

///////////////////////////////////////////////////////////////////////////////

class Raytracer
{
    // Raytrace on an integer, unit-width 2D grid with floating point coordinates
    // See http://playtechs.blogspot.ch/2007/03/raytracing-on-grid.html
  public:
    Raytracer( float x0, float y0, float x1, float y1 )
        : m_dx( qAbs( x1 - x0 ) )
        , m_dy( qAbs( y1 - y0 ) )
        , m_x( qFloor( x0 ) )
        , m_y( qFloor( y0 ) )
        , m_n( 1 )
    {
      if ( m_dx == 0. )
      {
        m_xInc = 0.;
        m_error = std::numeric_limits<float>::infinity();
      }
      else if ( x1 > x0 )
      {
        m_xInc = 1;
        m_n += int( qFloor( x1 ) ) - m_x;
        m_error = ( qFloor( x0 ) + 1 - x0 ) * m_dy;
      }
      else
      {
        m_xInc = -1;
        m_n += m_x - int( qFloor( x1 ) );
        m_error = ( x0 - qFloor( x0 ) ) * m_dy;
      }
      if ( m_dy == 0. )
      {
        m_yInc = 0.;
        m_error = -std::numeric_limits<float>::infinity();
      }
      else if ( y1 > y0 )
      {
        m_yInc = 1;
        m_n += int( qFloor( y1 ) ) - m_y;
        m_error -= ( qFloor( y0 ) + 1 - y0 ) * m_dx;
      }
      else
      {
        m_yInc = -1;
        m_n += m_y - int( qFloor( y1 ) );
        m_error -= ( y0 - qFloor( y0 ) ) * m_dx;
      }
    }
    int curCol() const { return m_x; }
    int curRow() const { return m_y; }
    void next()
    {
      if ( m_error > 0 )
      {
        m_y += m_yInc;
        m_error -= m_dx;
      }
      else if ( m_error < 0 )
      {
        m_x += m_xInc;
        m_error += m_dy;
      }
      else
      {
        m_x += m_xInc;
        m_y += m_yInc;
        m_error += m_dx;
        m_error -= m_dy;
        --m_n;
      }
      --m_n;
    }

    bool isValid() const { return m_n > 0; }

  private:
    float m_dx, m_dy;
    int m_x, m_y;
    int m_xInc, m_yInc;
    float m_error;
    int m_n;
};

///////////////////////////////////////////////////////////////////////////////

QgsSnapIndex::GridRow::~GridRow()
{
  Q_FOREACH ( const QgsSnapIndex::Cell& cell, mCells )
  {
    qDeleteAll( cell );
  }
}

QgsSnapIndex::Cell& QgsSnapIndex::GridRow::getCreateCell( int col )
{
  if ( col < mColStartIdx )
  {
    for ( int i = col; i < mColStartIdx; ++i )
    {
      mCells.prepend( Cell() );
    }
    mColStartIdx = col;
    return mCells.front();
  }
  else if ( col >= mColStartIdx + mCells.size() )
  {
    for ( int i = mColStartIdx + mCells.size(); i <= col; ++i )
    {
      mCells.append( Cell() );
    }
    return mCells.back();
  }
  else
  {
    return mCells[col - mColStartIdx];
  }
}

const QgsSnapIndex::Cell* QgsSnapIndex::GridRow::getCell( int col ) const
{
  if ( col < mColStartIdx || col >= mColStartIdx + mCells.size() )
  {
    return nullptr;
  }
  else
  {
    return &mCells[col - mColStartIdx];
  }
}

QList<QgsSnapIndex::SnapItem*> QgsSnapIndex::GridRow::getSnapItems( int colStart, int colEnd ) const
{
  colStart = qMax( colStart, mColStartIdx );
  colEnd = qMin( colEnd, mColStartIdx + mCells.size() - 1 );

  QList<SnapItem*> items;

  for ( int col = colStart; col <= colEnd; ++col )
  {
    items.append( mCells[col - mColStartIdx] );
  }
  return items;
}

///////////////////////////////////////////////////////////////////////////////

QgsSnapIndex::QgsSnapIndex( const QgsPointV2& origin, double cellSize )
    : mOrigin( origin )
    , mCellSize( cellSize )
    , mRowsStartIdx( 0 )
{
}

QgsSnapIndex::~QgsSnapIndex()
{
  qDeleteAll( mCoordIdxs );
}


const QgsSnapIndex::Cell *QgsSnapIndex::getCell( int col, int row ) const
{
  if ( row < mRowsStartIdx || row >= mRowsStartIdx + mGridRows.size() )
  {
    return nullptr;
  }
  else
  {
    return mGridRows[row - mRowsStartIdx].getCell( col );
  }
}

QgsSnapIndex::Cell& QgsSnapIndex::getCreateCell( int col, int row )
{
  if ( row < mRowsStartIdx )
  {
    for ( int i = row; i < mRowsStartIdx; ++i )
    {
      mGridRows.prepend( GridRow() );
    }
    mRowsStartIdx = row;
    return mGridRows.front().getCreateCell( col );
  }
  else if ( row >= mRowsStartIdx + mGridRows.size() )
  {
    for ( int i = mRowsStartIdx + mGridRows.size(); i <= row; ++i )
    {
      mGridRows.append( GridRow() );
    }
    return mGridRows.back().getCreateCell( col );
  }
  else
  {
    return mGridRows[row - mRowsStartIdx].getCreateCell( col );
  }
}

void QgsSnapIndex::addPoint( const CoordIdx* idx )
{
  QgsPointV2 p = idx->point();
  int col = qFloor(( p.x() - mOrigin.x() ) / mCellSize );
  int row = qFloor(( p.y() - mOrigin.y() ) / mCellSize );
  getCreateCell( col, row ).append( new PointSnapItem( idx ) );
}

void QgsSnapIndex::addSegment( const CoordIdx* idxFrom, const CoordIdx* idxTo )
{
  QgsPointV2 pFrom = idxFrom->point();
  QgsPointV2 pTo = idxTo->point();
  // Raytrace along the grid, get touched cells
  float x0 = ( pFrom.x() - mOrigin.x() ) / mCellSize;
  float y0 = ( pFrom.y() - mOrigin.y() ) / mCellSize;
  float x1 = ( pTo.x() - mOrigin.x() ) / mCellSize;
  float y1 = ( pTo.y() - mOrigin.y() ) / mCellSize;

  Raytracer rt( x0, y0, x1, y1 );
  for ( ; rt.isValid(); rt.next() )
  {
    getCreateCell( rt.curCol(), rt.curRow() ).append( new SegmentSnapItem( idxFrom, idxTo ) );
  }
}

void QgsSnapIndex::addGeometry( const QgsAbstractGeometryV2* geom )
{
  for ( int iPart = 0, nParts = geom->partCount(); iPart < nParts; ++iPart )
  {
    for ( int iRing = 0, nRings = geom->ringCount( iPart ); iRing < nRings; ++iRing )
    {
      for ( int iVert = 0, nVerts = geom->vertexCount( iPart, iRing ) - 1; iVert < nVerts; ++iVert )
      {
        CoordIdx* idx = new CoordIdx( geom, QgsVertexId( iPart, iRing, iVert ) );
        CoordIdx* idx1 = new CoordIdx( geom, QgsVertexId( iPart, iRing, iVert + 1 ) );
        mCoordIdxs.append( idx );
        mCoordIdxs.append( idx1 );
        addPoint( idx );
        addSegment( idx, idx1 );
      }
    }
  }
}


QgsPointV2 QgsSnapIndex::getClosestSnapToPoint( const QgsPointV2& p, const QgsPointV2& q )
{
  // Look for intersections on segment from the target point to the point opposite to the point reference point
  // p2 =  p1 + 2 * (q - p1)
  QgsPointV2 p2( 2 * q.x() - p.x(), 2 * q.y() - p.y() );

  // Raytrace along the grid, get touched cells
  float x0 = ( p.x() - mOrigin.x() ) / mCellSize;
  float y0 = ( p.y() - mOrigin.y() ) / mCellSize;
  float x1 = ( p2.x() - mOrigin.x() ) / mCellSize;
  float y1 = ( p2.y() - mOrigin.y() ) / mCellSize;

  Raytracer rt( x0, y0, x1, y1 );
  double dMin = std::numeric_limits<double>::max();
  QgsPointV2 pMin = p;
  for ( ; rt.isValid(); rt.next() )
  {
    const Cell* cell = getCell( rt.curCol(), rt.curRow() );
    if ( !cell )
    {
      continue;
    }
    Q_FOREACH ( const SnapItem* item, *cell )
    {
      if ( item->type == SnapSegment )
      {
        QgsPointV2 inter;
        if ( static_cast<const SegmentSnapItem*>( item )->getIntersection( p, p2, inter ) )
        {
          double dist = QgsGeometryUtils::sqrDistance2D( q, inter );
          if ( dist < dMin )
          {
            dMin = dist;
            pMin = inter;
          }
        }
      }
    }
  }

  return pMin;
}

QgsSnapIndex::SnapItem* QgsSnapIndex::getSnapItem( const QgsPointV2& pos, double tol, QgsSnapIndex::PointSnapItem** pSnapPoint, QgsSnapIndex::SegmentSnapItem** pSnapSegment ) const
{
  int colStart = qFloor(( pos.x() - tol - mOrigin.x() ) / mCellSize );
  int rowStart = qFloor(( pos.y() - tol - mOrigin.y() ) / mCellSize );
  int colEnd = qFloor(( pos.x() + tol - mOrigin.x() ) / mCellSize );
  int rowEnd = qFloor(( pos.y() + tol - mOrigin.y() ) / mCellSize );

  rowStart = qMax( rowStart, mRowsStartIdx );
  rowEnd = qMin( rowEnd, mRowsStartIdx + mGridRows.size() - 1 );

  QList<SnapItem*> items;
  for ( int row = rowStart; row <= rowEnd; ++row )
  {
    items.append( mGridRows[row - mRowsStartIdx].getSnapItems( colStart, colEnd ) );
  }

  double minDistSegment = std::numeric_limits<double>::max();
  double minDistPoint = std::numeric_limits<double>::max();
  QgsSnapIndex::SegmentSnapItem* snapSegment = nullptr;
  QgsSnapIndex::PointSnapItem* snapPoint = nullptr;

  Q_FOREACH ( QgsSnapIndex::SnapItem* item, items )
  {
    if ( item->type == SnapPoint )
    {
      double dist = QgsGeometryUtils::sqrDistance2D( item->getSnapPoint( pos ), pos );
      if ( dist < minDistPoint )
      {
        minDistPoint = dist;
        snapPoint = static_cast<PointSnapItem*>( item );
      }
    }
    else if ( item->type == SnapSegment )
    {
      QgsPointV2 pProj;
      if ( !static_cast<SegmentSnapItem*>( item )->getProjection( pos, pProj ) )
      {
        continue;
      }
      double dist = QgsGeometryUtils::sqrDistance2D( pProj, pos );
      if ( dist < minDistSegment )
      {
        minDistSegment = dist;
        snapSegment = static_cast<SegmentSnapItem*>( item );
      }
    }
  }
  snapPoint = minDistPoint < tol * tol ? snapPoint : nullptr;
  snapSegment = minDistSegment < tol * tol ? snapSegment : nullptr;
  if ( pSnapPoint ) *pSnapPoint = snapPoint;
  if ( pSnapSegment ) *pSnapSegment = snapSegment;
  return minDistPoint < minDistSegment ? static_cast<QgsSnapIndex::SnapItem*>( snapPoint ) : static_cast<QgsSnapIndex::SnapItem*>( snapSegment );
}
