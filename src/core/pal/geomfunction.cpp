/*
 *   libpal - Automated Placement of Labels Library
 *
 *   Copyright (C) 2008 Maxence Laurent, MIS-TIC, HEIG-VD
 *                      University of Applied Sciences, Western Switzerland
 *                      http://www.hes-so.ch
 *
 *   Contact:
 *      maxence.laurent <at> heig-vd <dot> ch
 *    or
 *      eric.taillard <at> heig-vd <dot> ch
 *
 * This file is part of libpal.
 *
 * libpal is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libpal is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libpal.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "geomfunction.h"
#include "feature.h"
#include "util.h"
#include "qgis.h"
#include "pal.h"
#include "qgsmessagelog.h"
#include <vector>

using namespace pal;

void heapsort( std::vector< int > &sid, int *id, const std::vector< double > &x, std::size_t N )
{
  std::size_t n = N;
  std::size_t i = n / 2;
  std::size_t parent;
  std::size_t child;
  int tx;
  for ( ;; )
  {
    if ( i > 0 )
    {
      i--;
      tx = sid[i];
    }
    else
    {
      n--;
      if ( n == 0 ) return;
      tx = sid[n];
      sid[n] = sid[0];
    }
    parent = i;
    child = i * 2 + 1;
    while ( child < n )
    {
      if ( child + 1 < n  &&  x[id[sid[child + 1]]] > x[id[sid[child]]] )
      {
        child++;
      }
      if ( x[id[sid[child]]] > x[id[tx]] )
      {
        sid[parent] = sid[child];
        parent = child;
        child = parent * 2 + 1;
      }
      else
      {
        break;
      }
    }
    sid[parent] = tx;
  }
}


void heapsort2( int *x, double *heap, std::size_t N )
{
  std::size_t n = N;
  std::size_t i = n / 2;
  std::size_t parent;
  std::size_t child;
  double t;
  int tx;
  for ( ;; )
  {
    if ( i > 0 )
    {
      i--;
      t = heap[i];
      tx = x[i];
    }
    else
    {
      n--;
      if ( n == 0 ) return;
      t = heap[n];
      tx = x[n];
      heap[n] = heap[0];
      x[n] = x[0];
    }
    parent = i;
    child = i * 2 + 1;
    while ( child < n )
    {
      if ( child + 1 < n  &&  heap[child + 1] > heap[child] )
      {
        child++;
      }
      if ( heap[child] > t )
      {
        heap[parent] = heap[child];
        x[parent] = x[child];
        parent = child;
        child = parent * 2 + 1;
      }
      else
      {
        break;
      }
    }
    heap[parent] = t;
    x[parent] = tx;
  }
}

bool GeomFunction::isSegIntersects( double x1, double y1, double x2, double y2,  // 1st segment
                                    double x3, double y3, double x4, double y4 )  // 2nd segment
{
  return ( cross_product( x1, y1, x2, y2, x3, y3 ) * cross_product( x1, y1, x2, y2, x4, y4 ) < 0
           && cross_product( x3, y3, x4, y4, x1, y1 ) * cross_product( x3, y3, x4, y4, x2, y2 ) < 0 );
}

bool GeomFunction::computeLineIntersection( double x1, double y1, double x2, double y2,  // 1st line (segment)
    double x3, double y3, double x4, double y4,  // 2nd line segment
    double *x, double *y )
{

  double a1, a2, b1, b2, c1, c2;
  double denom;

  a1 = y2 - y1;
  b1 = x1 - x2;
  c1 = x2 * y1 - x1 * y2;

  a2 = y4 - y3;
  b2 = x3 - x4;
  c2 = x4 * y3 - x3 * y4;

  denom = a1 * b2 - a2 * b1;
  if ( qgsDoubleNear( denom, 0.0 ) )
  {
    return false;
  }
  else
  {
    *x = ( b1 * c2 - b2 * c1 ) / denom;
    *y = ( a2 * c1 - a1 * c2 ) / denom;
  }

  return true;
}

std::vector< int > GeomFunction::convexHullId( std::vector< int > &id, const std::vector< double > &x, const std::vector< double > &y )
{
  std::vector< int > convexHull( x.size() );
  for ( std::size_t i = 0; i < x.size(); i++ )
  {
    convexHull[i] = static_cast< int >( i );
  }

  if ( x.size() <= 3 )
    return convexHull;

  std::vector< int > stack( x.size() );
  std::vector< double > tan( x.size() );

  // find the lowest y value
  heapsort( convexHull, id.data(), y, y.size() );

  // find the lowest x value from the lowest y
  std::size_t ref = 1;
  while ( ref < x.size() && qgsDoubleNear( y[id[convexHull[ref]]], y[id[convexHull[0]]] ) )
    ref++;

  heapsort( convexHull, id.data(), x, ref );

  // the first point is now for sure in the hull as well as the ref one
  for ( std::size_t i = ref; i < x.size(); i++ )
  {
    if ( qgsDoubleNear( y[id[convexHull[i]]], y[id[convexHull[0]]] ) )
      tan[i] = FLT_MAX;
    else
      tan[i] = ( x[id[convexHull[0]]] - x[id[convexHull[i]]] ) / ( y[id[convexHull[i]]] - y[id[convexHull[0]]] );
  }

  if ( ref < x.size() )
    heapsort2( convexHull.data() + ref, tan.data() + ref, x.size() - ref );

  // the second point is in too
  stack[0] = convexHull[0];
  if ( ref == 1 )
  {
    stack[1] = convexHull[1];
    ref++;
  }
  else
    stack[1] = convexHull[ref - 1];

  std::size_t top = 1;
  std::size_t second = 0;

  for ( std::size_t i = ref; i < x.size(); i++ )
  {
    double result = cross_product( x[id[stack[second]]], y[id[stack[second]]],
                                   x[id[stack[top]]], y[id[stack[top]]], x[id[convexHull[i]]], y[id[convexHull[i]]] );
    // Coolineaire !! garder le plus éloigné
    if ( qgsDoubleNear( result, 0.0 ) )
    {
      if ( dist_euc2d_sq( x[id[stack[second]]], y[id[stack[second]]], x[id[convexHull[i]]], y[id[convexHull[i]]] )
           >  dist_euc2d_sq( x[id[stack[second]]], y[id[stack[second]]], x[id[stack[top]]], y[id[stack[top]]] ) )
      {
        stack[top] = convexHull[i];
      }
    }
    else if ( result > 0 ) //convexe
    {
      second++;
      top++;
      stack[top] = convexHull[i];
    }
    else
    {
      while ( result < 0 && top > 1 )
      {
        second--;
        top--;
        result = cross_product( x[id[stack[second]]],
                                y[id[stack[second]]], x[id[stack[top]]],
                                y[id[stack[top]]], x[id[convexHull[i]]], y[id[convexHull[i]]] );
      }
      second++;
      top++;
      stack[top] = convexHull[i];
    }
  }

  for ( std::size_t i = 0; i <= top; i++ )
  {
    convexHull[i] = stack[i];
  }

  convexHull.resize( top + 1 );
  return convexHull;
}

bool GeomFunction::reorderPolygon( std::vector<double> &x, std::vector<double> &y )
{
  std::vector< int > pts( x.size() );
  for ( std::size_t i = 0; i < x.size(); i++ )
    pts[i] = static_cast< int >( i );

  std::vector< int > convexHull = convexHullId( pts, x, y );

  int inc = 0;
  if ( pts[convexHull[0]] < pts[convexHull[1]] && pts[convexHull[1]] < pts[convexHull[2]] )
    inc = 1;
  else if ( pts[convexHull[0]] > pts[convexHull[1]] && pts[convexHull[1]] > pts[convexHull[2]] )
    inc = -1;
  else if ( pts[convexHull[0]] > pts[convexHull[1]] && pts[convexHull[1]] < pts[convexHull[2]] && pts[convexHull[2]] < pts[convexHull[0]] )
    inc = 1;
  else if ( pts[convexHull[0]] > pts[convexHull[1]] && pts[convexHull[1]] < pts[convexHull[2]] && pts[convexHull[2]] > pts[convexHull[0]] )
    inc = -1;
  else if ( pts[convexHull[0]] < pts[convexHull[1]] && pts[convexHull[1]] > pts[convexHull[2]] && pts[convexHull[2]] > pts[convexHull[0]] )
    inc = -1;
  else if ( pts[convexHull[0]] < pts[convexHull[1]] && pts[convexHull[1]] > pts[convexHull[2]] && pts[convexHull[2]] < pts[convexHull[0]] )
    inc = 1;
  else
  {
    // wrong cHull
    return false;
  }

  if ( inc == -1 ) // re-order points
  {
    for ( std::size_t i = 0, j = x.size() - 1; i <= j; i++, j-- )
    {
      std::swap( x[i], x[j] );
      std::swap( y[i], y[j] );
    }
  }
  return true;
}

bool GeomFunction::containsCandidate( const GEOSPreparedGeometry *geom, double x, double y, double width, double height, double alpha )
{
  if ( !geom )
    return false;

  try
  {
    GEOSContextHandle_t geosctxt = QgsGeos::getGEOSHandler();
    GEOSCoordSequence *coord = GEOSCoordSeq_create_r( geosctxt, 5, 2 );

    GEOSCoordSeq_setXY_r( geosctxt, coord, 0, x, y );
    if ( !qgsDoubleNear( alpha, 0.0 ) )
    {
      const double beta = alpha + M_PI_2;
      const double dx1 = std::cos( alpha ) * width;
      const double dy1 = std::sin( alpha ) * width;
      const double dx2 = std::cos( beta ) * height;
      const double dy2 = std::sin( beta ) * height;
      GEOSCoordSeq_setXY_r( geosctxt, coord, 1, x  + dx1, y + dy1 );
      GEOSCoordSeq_setXY_r( geosctxt, coord, 2, x + dx1 + dx2, y + dy1 + dy2 );
      GEOSCoordSeq_setXY_r( geosctxt, coord, 3, x + dx2, y + dy2 );
    }
    else
    {
      GEOSCoordSeq_setXY_r( geosctxt, coord, 1, x + width, y );
      GEOSCoordSeq_setXY_r( geosctxt, coord, 2, x + width, y + height );
      GEOSCoordSeq_setXY_r( geosctxt, coord, 3, x, y + height );
    }
    //close ring
    GEOSCoordSeq_setXY_r( geosctxt, coord, 4, x, y );

    geos::unique_ptr bboxGeos( GEOSGeom_createLinearRing_r( geosctxt, coord ) );
    const bool result = ( GEOSPreparedContainsProperly_r( geosctxt, geom, bboxGeos.get() ) == 1 );
    return result;
  }
  catch ( GEOSException &e )
  {
    qWarning( "GEOS exception: %s", e.what() );
    Q_NOWARN_UNREACHABLE_PUSH
    QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );
    return false;
    Q_NOWARN_UNREACHABLE_POP
  }
  return false;
}

void GeomFunction::findLineCircleIntersection( double cx, double cy, double radius,
    double x1, double y1, double x2, double y2,
    double &xRes, double &yRes )
{
  double multiplier = 1;
  if ( radius < 10 )
  {
    // these calculations get unstable for small coordinates differences, e.g. as a result of map labeling in a geographic
    // CRS
    multiplier = 10000;
    x1 *= multiplier;
    y1 *= multiplier;
    x2 *= multiplier;
    y2 *= multiplier;
    cx *= multiplier;
    cy *= multiplier;
    radius *= multiplier;
  }

  const double dx = x2 - x1;
  const double dy = y2 - y1;

  const double A = dx * dx + dy * dy;
  const double B = 2 * ( dx * ( x1 - cx ) + dy * ( y1 - cy ) );
  const double C = ( x1 - cx ) * ( x1 - cx ) + ( y1 - cy ) * ( y1 - cy ) - radius * radius;

  const double det = B * B - 4 * A * C;
  if ( A <= 0.000000000001 || det < 0 )
    // Should never happen, No real solutions.
    return;

  if ( qgsDoubleNear( det, 0.0 ) )
  {
    // Could potentially happen.... One solution.
    const double t = -B / ( 2 * A );
    xRes = x1 + t * dx;
    yRes = y1 + t * dy;
  }
  else
  {
    // Two solutions.
    // Always use the 1st one
    // We only really have one solution here, as we know the line segment will start in the circle and end outside
    const double t = ( -B + std::sqrt( det ) ) / ( 2 * A );
    xRes = x1 + t * dx;
    yRes = y1 + t * dy;
  }

  if ( multiplier != 1 )
  {
    xRes /= multiplier;
    yRes /= multiplier;
  }
}
