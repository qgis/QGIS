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

#include "pointset.h"
#include "util.h"
#include "pal.h"
#include "geomfunction.h"
#include "qgsgeos.h"
#include "qgsmessagelog.h"
#include "qgsgeometryutils.h"
#include <qglobal.h>

using namespace pal;

PointSet::PointSet()
{
  nbPoints = cHullSize = 0;
  cHull = nullptr;
  type = -1;
}

PointSet::PointSet( int nbPoints, double *x, double *y )
  : nbPoints( nbPoints )
  , type( GEOS_POLYGON )
{
  this->x.resize( nbPoints );
  this->y.resize( nbPoints );
  int i;

  for ( i = 0; i < nbPoints; i++ )
  {
    this->x[i] = x[i];
    this->y[i] = y[i];
  }

}

PointSet::PointSet( double aX, double aY )
  : type( GEOS_POINT )
  , xmin( aX )
  , xmax( aY )
  , ymin( aX )
  , ymax( aY )

{
  nbPoints = cHullSize = 1;
  x.resize( 1 );
  y.resize( 1 );
  x[0] = aX;
  y[0] = aY;
}

PointSet::PointSet( const PointSet &ps )
  : xmin( ps.xmin )
  , xmax( ps.xmax )
  , ymin( ps.ymin )
  , ymax( ps.ymax )
{
  int i;

  nbPoints = ps.nbPoints;
  x = ps.x;
  y = ps.y;

  if ( ps.cHull )
  {
    cHullSize = ps.cHullSize;
    cHull = new int[cHullSize];
    for ( i = 0; i < cHullSize; i++ )
    {
      cHull[i] = ps.cHull[i];
    }
  }
  else
  {
    cHull = nullptr;
    cHullSize = 0;
  }

  type = ps.type;

  holeOf = ps.holeOf;

  if ( ps.mGeos )
  {
    mGeos = GEOSGeom_clone_r( QgsGeos::getGEOSHandler(), ps.mGeos );
    mOwnsGeom = true;
  }
}

void PointSet::createGeosGeom() const
{
  GEOSContextHandle_t geosctxt = QgsGeos::getGEOSHandler();

  bool needClose = false;
  if ( type == GEOS_POLYGON && ( !qgsDoubleNear( x[0], x[ nbPoints - 1] ) || !qgsDoubleNear( y[0], y[ nbPoints - 1 ] ) ) )
  {
    needClose = true;
  }

  GEOSCoordSequence *coord = GEOSCoordSeq_create_r( geosctxt, nbPoints + ( needClose ? 1 : 0 ), 2 );
  for ( int i = 0; i < nbPoints; ++i )
  {
#if GEOS_VERSION_MAJOR>3 || GEOS_VERSION_MINOR>=8
    GEOSCoordSeq_setXY_r( geosctxt, coord, i, x[i], y[i] );
#else
    GEOSCoordSeq_setX_r( geosctxt, coord, i, x[i] );
    GEOSCoordSeq_setY_r( geosctxt, coord, i, y[i] );
#endif
  }

  //close ring if needed
  if ( needClose )
  {
#if GEOS_VERSION_MAJOR>3 || GEOS_VERSION_MINOR>=8
    GEOSCoordSeq_setXY_r( geosctxt, coord, nbPoints, x[0], y[0] );
#else
    GEOSCoordSeq_setX_r( geosctxt, coord, nbPoints, x[0] );
    GEOSCoordSeq_setY_r( geosctxt, coord, nbPoints, y[0] );
#endif
  }

  switch ( type )
  {
    case GEOS_POLYGON:
      mGeos = GEOSGeom_createPolygon_r( geosctxt, GEOSGeom_createLinearRing_r( geosctxt, coord ), nullptr, 0 );
      break;

    case GEOS_LINESTRING:
      mGeos = GEOSGeom_createLineString_r( geosctxt, coord );
      break;

    case GEOS_POINT:
      mGeos = GEOSGeom_createPoint_r( geosctxt, coord );
      break;
  }

  mOwnsGeom = true;
}

const GEOSPreparedGeometry *PointSet::preparedGeom() const
{
  if ( !mGeos )
    createGeosGeom();

  if ( !mPreparedGeom )
  {
    mPreparedGeom = GEOSPrepare_r( QgsGeos::getGEOSHandler(), mGeos );
  }
  return mPreparedGeom;
}

void PointSet::invalidateGeos()
{
  GEOSContextHandle_t geosctxt = QgsGeos::getGEOSHandler();
  if ( mOwnsGeom ) // delete old geometry if we own it
    GEOSGeom_destroy_r( geosctxt, mGeos );
  GEOSPreparedGeom_destroy_r( geosctxt, mPreparedGeom );
  mOwnsGeom = false;
  mGeos = nullptr;
  mPreparedGeom = nullptr;
}

PointSet::~PointSet()
{
  GEOSContextHandle_t geosctxt = QgsGeos::getGEOSHandler();

  if ( mGeos && mOwnsGeom )
  {
    GEOSGeom_destroy_r( geosctxt, mGeos );
    mGeos = nullptr;
  }
  GEOSPreparedGeom_destroy_r( geosctxt, mPreparedGeom );

  deleteCoords();

  delete[] cHull;
}

void PointSet::deleteCoords()
{
  x.clear();
  y.clear();
}

PointSet *PointSet::extractShape( int nbPtSh, int imin, int imax, int fps, int fpe, double fptx, double fpty )
{

  int i, j;

  PointSet *newShape = new PointSet();
  newShape->type = GEOS_POLYGON;
  newShape->nbPoints = nbPtSh;
  newShape->x.resize( newShape->nbPoints );
  newShape->y.resize( newShape->nbPoints );

  // new shape # 1 from imin to imax
  for ( j = 0, i = imin; i != ( imax + 1 ) % nbPoints; i = ( i + 1 ) % nbPoints, j++ )
  {
    newShape->x[j] = x[i];
    newShape->y[j] = y[i];
  }
  // is the cutting point a new one ?
  if ( fps != fpe )
  {
    // yes => so add it
    newShape->x[j] = fptx;
    newShape->y[j] = fpty;
  }

  return newShape;
}

std::unique_ptr<PointSet> PointSet::clone() const
{
  return std::unique_ptr< PointSet>( new PointSet( *this ) );
}

bool PointSet::containsPoint( double x, double y ) const
{
  GEOSContextHandle_t geosctxt = QgsGeos::getGEOSHandler();
  try
  {
#if GEOS_VERSION_MAJOR>3 || GEOS_VERSION_MINOR>=8
    geos::unique_ptr point( GEOSGeom_createPointFromXY_r( geosctxt, x, y ) );
#else
    GEOSCoordSequence *seq = GEOSCoordSeq_create_r( geosctxt, 1, 2 );
    GEOSCoordSeq_setX_r( geosctxt, seq, 0, x );
    GEOSCoordSeq_setY_r( geosctxt, seq, 0, y );
    geos::unique_ptr point( GEOSGeom_createPoint_r( geosctxt, seq ) );
#endif
    bool result = ( GEOSPreparedContainsProperly_r( geosctxt, preparedGeom(), point.get() ) == 1 );

    return result;
  }
  catch ( GEOSException &e )
  {
    QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );
    return false;
  }

}

bool PointSet::containsLabelCandidate( double x, double y, double width, double height, double alpha ) const
{
  return GeomFunction::containsCandidate( preparedGeom(), x, y, width, height, alpha );
}

void PointSet::splitPolygons( QLinkedList<PointSet *> &shapes_toProcess,
                              QLinkedList<PointSet *> &shapes_final,
                              double xrm, double yrm )
{
  int i, j;

  int nbp;
  std::vector< double > x;
  std::vector< double > y;

  int *pts = nullptr;

  int *cHull = nullptr;
  int cHullSize;

  double cp;
  double bestcp = 0;

  double bestArea = 0;
  double area;

  double base;
  double b, c;
  double s;

  int ihs;
  int ihn;

  int ips;
  int ipn;

  int holeS = -1;  // hole start and end points
  int holeE = -1;

  int retainedPt = -1;
  int pt = 0;

  double labelArea = xrm * yrm;

  PointSet *shape = nullptr;

  while ( !shapes_toProcess.isEmpty() )
  {
    shape = shapes_toProcess.takeFirst();

    x = shape->x;
    y = shape->y;
    nbp = shape->nbPoints;
    pts = new int[nbp];

    for ( i = 0; i < nbp; i++ )
    {
      pts[i] = i;
    }

    // conpute convex hull
    shape->cHullSize = GeomFunction::convexHullId( pts, x, y, nbp, shape->cHull );

    cHull = shape->cHull;
    cHullSize = shape->cHullSize;

    bestArea = 0;
    retainedPt = -1;

    // lookup for a hole
    for ( ihs = 0; ihs < cHullSize; ihs++ )
    {
      // ihs->ihn => cHull'seg
      ihn = ( ihs + 1 ) % cHullSize;

      ips = cHull[ihs];
      ipn = ( ips + 1 ) % nbp;
      if ( ipn != cHull[ihn] ) // next point on shape is not the next point on cHull => there is a hole here !
      {
        bestcp = 0;
        pt = -1;
        // lookup for the deepest point in the hole
        for ( i = ips; i != cHull[ihn]; i = ( i + 1 ) % nbp )
        {
          cp = std::fabs( GeomFunction::cross_product( x[cHull[ihs]], y[cHull[ihs]],
                          x[cHull[ihn]], y[cHull[ihn]],
                          x[i], y[i] ) );
          if ( cp - bestcp > EPSILON )
          {
            bestcp = cp;
            pt = i;
          }
        }

        if ( pt  != -1 )
        {
          // compute the ihs->ihn->pt triangle's area
          base = GeomFunction::dist_euc2d( x[cHull[ihs]], y[cHull[ihs]],
                                           x[cHull[ihn]], y[cHull[ihn]] );

          b = GeomFunction::dist_euc2d( x[cHull[ihs]], y[cHull[ihs]],
                                        x[pt], y[pt] );

          c = GeomFunction::dist_euc2d( x[cHull[ihn]], y[cHull[ihn]],
                                        x[pt], y[pt] );

          s = ( base + b + c ) / 2; // s = half perimeter
          area = s * ( s - base ) * ( s - b ) * ( s - c );
          if ( area < 0 )
            area = -area;

          // retain the biggest area
          if ( area - bestArea > EPSILON )
          {
            bestArea = area;
            retainedPt = pt;
            holeS = ihs;
            holeE = ihn;
          }
        }
      }
    }

    // we have a hole, its area, and the deppest point in hole
    // we're going to find the second point to cup the shape
    // holeS = hole starting point
    // holeE = hole ending point
    // retainedPt = deppest point in hole
    // bestArea = area of triangle HoleS->holeE->retainedPoint
    bestArea = std::sqrt( bestArea );
    double cx, cy, dx, dy, ex, ey, fx, fy, seg_length, ptx = 0, pty = 0, fptx = 0, fpty = 0;
    int ps = -1, pe = -1, fps = -1, fpe = -1;
    if ( retainedPt >= 0 && bestArea > labelArea ) // there is a hole so we'll cut the shape in two new shape (only if hole area is bigger than twice labelArea)
    {
      c = std::numeric_limits<double>::max();

      // iterate on all shape points except points which are in the hole
      bool isValid;
      int k, l;
      for ( i = ( cHull[holeE] + 1 ) % nbp; i != ( cHull[holeS] - 1 + nbp ) % nbp; i = j )
      {
        j = ( i + 1 ) % nbp; // i->j is shape segment not in hole

        // compute distance between retainedPoint and segment
        // whether perpendicular distance (if retaindPoint is fronting segment i->j)
        // or distance between retainedPt and i or j (choose the nearest)
        seg_length = GeomFunction::dist_euc2d( x[i], y[i], x[j], y[j] );
        cx = ( x[i] + x[j] ) / 2.0;
        cy = ( y[i] + y[j] ) / 2.0;
        dx = cy - y[i];
        dy = cx - x[i];

        ex = cx - dx;
        ey = cy + dy;
        fx = cx + dx;
        fy = cy - dy;

        if ( seg_length < EPSILON || std::fabs( ( b = GeomFunction::cross_product( ex, ey, fx, fy, x[retainedPt], y[retainedPt] ) / ( seg_length ) ) ) > ( seg_length / 2 ) )   // retainedPt is not fronting i->j
        {
          if ( ( ex = GeomFunction::dist_euc2d_sq( x[i], y[i], x[retainedPt], y[retainedPt] ) ) < ( ey = GeomFunction::dist_euc2d_sq( x[j], y[j], x[retainedPt], y[retainedPt] ) ) )
          {
            b = ex;
            ps = i;
            pe = i;
          }
          else
          {
            b = ey;
            ps = j;
            pe = j;
          }
        }
        else   // point fronting i->j => compute pependicular distance  => create a new point
        {
          b = GeomFunction::cross_product( x[i], y[i], x[j], y[j], x[retainedPt], y[retainedPt] ) / seg_length;
          b *= b;
          ps = i;
          pe = j;

          if ( !GeomFunction::computeLineIntersection( x[i], y[i], x[j], y[j], x[retainedPt], y[retainedPt], x[retainedPt] - dx, y[retainedPt] + dy, &ptx, &pty ) )
          {
            //error - it should intersect the line
          }
        }

        isValid = true;
        double pointX, pointY;
        if ( ps == pe )
        {
          pointX = x[pe];
          pointY = y[pe];
        }
        else
        {
          pointX = ptx;
          pointY = pty;
        }

        for ( k = cHull[holeS]; k != cHull[holeE]; k = ( k + 1 ) % nbp )
        {
          l = ( k + 1 ) % nbp;
          if ( GeomFunction::isSegIntersects( x[retainedPt], y[retainedPt], pointX, pointY, x[k], y[k], x[l], y[l] ) )
          {
            isValid = false;
            break;
          }
        }


        if ( isValid && b < c )
        {
          c = b;
          fps = ps;
          fpe = pe;
          fptx = ptx;
          fpty = pty;
        }
      }  // for point which are not in hole

      // we will cut the shapeu in two new shapes, one from [retainedPoint] to [newPoint] and one form [newPoint] to [retainedPoint]
      int imin = retainedPt;
      int imax = ( ( ( fps < retainedPt && fpe < retainedPt ) || ( fps > retainedPt && fpe > retainedPt ) ) ? std::min( fps, fpe ) : std::max( fps, fpe ) );

      int nbPtSh1, nbPtSh2; // how many points in new shapes ?
      if ( imax > imin )
        nbPtSh1 = imax - imin + 1 + ( fpe != fps );
      else
        nbPtSh1 = imax + nbp - imin + 1 + ( fpe != fps );

      if ( ( imax == fps ? fpe : fps ) < imin )
        nbPtSh2 = imin - ( imax == fps ? fpe : fps ) + 1 + ( fpe != fps );
      else
        nbPtSh2 = imin + nbp - ( imax == fps ? fpe : fps ) + 1 + ( fpe != fps );

      if ( retainedPt == -1 || fps == -1 || fpe == -1 )
      {
        if ( shape->parent )
          delete shape;
      }
      // check for useless splitting
      else if ( imax == imin || nbPtSh1 <= 2 || nbPtSh2 <= 2 || nbPtSh1 == nbp  || nbPtSh2 == nbp )
      {
        shapes_final.append( shape );
      }
      else
      {

        PointSet *newShape = shape->extractShape( nbPtSh1, imin, imax, fps, fpe, fptx, fpty );

        if ( shape->parent )
          newShape->parent = shape->parent;
        else
          newShape->parent = shape;

        shapes_toProcess.append( newShape );

        if ( imax == fps )
          imax = fpe;
        else
          imax = fps;

        newShape = shape->extractShape( nbPtSh2, imax, imin, fps, fpe, fptx, fpty );

        if ( shape->parent )
          newShape->parent = shape->parent;
        else
          newShape->parent = shape;

        shapes_toProcess.append( newShape );

        if ( shape->parent )
          delete shape;
      }
    }
    else
    {
      shapes_final.append( shape );
    }
    delete[] pts;
  }
}

void PointSet::extendLineByDistance( double startDistance, double endDistance, double smoothDistance )
{
  if ( nbPoints < 2 )
    return;

  double x0 = x[0];
  double y0 = y[0];
  if ( startDistance > 0 )
  {
    // trace forward by smoothDistance
    double x1 = x[1];
    double y1 = y[1];

    double distanceConsumed = 0;
    double lastX = x0;
    double lastY = y0;
    for ( int i = 1; i < nbPoints; ++i )
    {
      double thisX = x[i];
      double thisY = y[i];
      const double thisSegmentLength = std::sqrt( ( thisX - lastX ) * ( thisX - lastX ) + ( thisY - lastY ) * ( thisY - lastY ) );
      distanceConsumed += thisSegmentLength;
      if ( distanceConsumed >= smoothDistance )
      {
        double c = ( distanceConsumed - smoothDistance ) / thisSegmentLength;
        x1 = lastX + c * ( thisX - lastX );
        y1 = lastY + c * ( thisY - lastY );
        break;
      }
      lastX = thisX;
      lastY = thisY;
    }

    const double distance = std::sqrt( ( x1 - x0 ) * ( x1 - x0 ) + ( y1 - y0 ) * ( y1 - y0 ) );
    const double extensionFactor = ( startDistance + distance ) / distance;
    const QgsPointXY newStart = QgsGeometryUtils::interpolatePointOnLine( x1, y1, x0, y0, extensionFactor );
    x0 = newStart.x();
    y0 = newStart.y();
    // defer actually changing the stored start until we've calculated the new end point
  }

  if ( endDistance > 0 )
  {
    double xend0 = x[nbPoints - 1];
    double yend0 = y[nbPoints - 1];
    double xend1 = x[nbPoints - 2];
    double yend1 = y[nbPoints - 2];

    // trace backward by smoothDistance
    double distanceConsumed = 0;
    double lastX = x0;
    double lastY = y0;
    for ( int i = nbPoints - 2; i >= 0; --i )
    {
      double thisX = x[i];
      double thisY = y[i];
      const double thisSegmentLength = std::sqrt( ( thisX - lastX ) * ( thisX - lastX ) + ( thisY - lastY ) * ( thisY - lastY ) );
      distanceConsumed += thisSegmentLength;
      if ( distanceConsumed >= smoothDistance )
      {
        double c = ( distanceConsumed - smoothDistance ) / thisSegmentLength;
        xend1 = lastX + c * ( thisX - lastX );
        yend1 = lastY + c * ( thisY - lastY );
        break;
      }
      lastX = thisX;
      lastY = thisY;
    }

    const double distance = std::sqrt( ( xend1 - xend0 ) * ( xend1 - xend0 ) + ( yend1 - yend0 ) * ( yend1 - yend0 ) );
    const double extensionFactor = ( endDistance + distance ) / distance;
    const QgsPointXY newEnd = QgsGeometryUtils::interpolatePointOnLine( xend1, yend1, xend0, yend0, extensionFactor );
    x.emplace_back( newEnd.x() );
    y.emplace_back( newEnd.y() );
    nbPoints++;
  }

  if ( startDistance > 0 )
  {
    x.insert( x.begin(), x0 );
    y.insert( y.begin(), y0 );
    nbPoints++;
  }

  invalidateGeos();
}

CHullBox *PointSet::compute_chull_bbox()
{
  int i;
  int j;

  double bbox[4]; // xmin, ymin, xmax, ymax

  double alpha;
  int alpha_d;

  double alpha_seg;

  double dref;
  double d1, d2;

  double bb[16];   // {ax, ay, bx, by, cx, cy, dx, dy, ex, ey, fx, fy, gx, gy, hx, hy}}

  double cp;
  double best_cp;
  double distNearestPoint;

  double area;
  double width;
  double length;

  double best_area = std::numeric_limits<double>::max();
  double best_alpha = -1;
  double best_bb[16];
  double best_length = 0;
  double best_width = 0;


  bbox[0] = std::numeric_limits<double>::max();
  bbox[1] = std::numeric_limits<double>::max();
  bbox[2] = std::numeric_limits<double>::lowest();
  bbox[3] = std::numeric_limits<double>::lowest();

  for ( i = 0; i < cHullSize; i++ )
  {
    if ( x[cHull[i]] < bbox[0] )
      bbox[0] = x[cHull[i]];

    if ( x[cHull[i]] > bbox[2] )
      bbox[2] = x[cHull[i]];

    if ( y[cHull[i]] < bbox[1] )
      bbox[1] = y[cHull[i]];

    if ( y[cHull[i]] > bbox[3] )
      bbox[3] = y[cHull[i]];
  }


  dref = bbox[2] - bbox[0];

  for ( alpha_d = 0; alpha_d < 90; alpha_d++ )
  {
    alpha = alpha_d *  M_PI / 180.0;
    d1 = std::cos( alpha ) * dref;
    d2 = std::sin( alpha ) * dref;

    bb[0]  = bbox[0];
    bb[1]  = bbox[3]; // ax, ay

    bb[4]  = bbox[0];
    bb[5]  = bbox[1]; // cx, cy

    bb[8]  = bbox[2];
    bb[9]  = bbox[1]; // ex, ey

    bb[12] = bbox[2];
    bb[13] = bbox[3]; // gx, gy


    bb[2]  = bb[0] + d1;
    bb[3]  = bb[1] + d2; // bx, by
    bb[6]  = bb[4] - d2;
    bb[7]  = bb[5] + d1; // dx, dy
    bb[10] = bb[8] - d1;
    bb[11] = bb[9] - d2; // fx, fy
    bb[14] = bb[12] + d2;
    bb[15] = bb[13] - d1; // hx, hy

    // adjust all points
    for ( i = 0; i < 16; i += 4 )
    {

      alpha_seg = ( ( i / 4 > 0 ? ( i / 4 ) - 1 : 3 ) ) * M_PI_2 + alpha;

      best_cp = std::numeric_limits<double>::max();
      for ( j = 0; j < nbPoints; j++ )
      {
        cp = GeomFunction::cross_product( bb[i + 2], bb[i + 3], bb[i], bb[i + 1], x[cHull[j]], y[cHull[j]] );
        if ( cp < best_cp )
        {
          best_cp = cp;
        }
      }

      distNearestPoint = best_cp / dref;

      d1 = std::cos( alpha_seg ) * distNearestPoint;
      d2 = std::sin( alpha_seg ) * distNearestPoint;

      bb[i]   += d1; // x
      bb[i + 1] += d2; // y
      bb[i + 2] += d1; // x
      bb[i + 3] += d2; // y
    }

    // compute and compare AREA
    width = GeomFunction::cross_product( bb[6], bb[7], bb[4], bb[5], bb[12], bb[13] ) / dref;
    length = GeomFunction::cross_product( bb[2], bb[3], bb[0], bb[1], bb[8], bb[9] ) / dref;

    area = width * length;

    if ( area < 0 )
      area *= -1;


    if ( best_area - area > EPSILON )
    {
      best_area = area;
      best_length = length;
      best_width = width;
      best_alpha = alpha;
      memcpy( best_bb, bb, sizeof( double ) * 16 );
    }
  }

  // best bbox is defined

  CHullBox *finalBb = new CHullBox();

  for ( i = 0; i < 16; i = i + 4 )
  {
    GeomFunction::computeLineIntersection( best_bb[i], best_bb[i + 1], best_bb[i + 2], best_bb[i + 3],
                                           best_bb[( i + 4 ) % 16], best_bb[( i + 5 ) % 16], best_bb[( i + 6 ) % 16], best_bb[( i + 7 ) % 16],
                                           &finalBb->x[int ( i / 4 )], &finalBb->y[int ( i / 4 )] );
  }

  finalBb->alpha = best_alpha;
  finalBb->width = best_width;
  finalBb->length = best_length;

  return finalBb;
}

double PointSet::minDistanceToPoint( double px, double py, double *rx, double *ry ) const
{
  if ( !mGeos )
    createGeosGeom();

  if ( !mGeos )
    return 0;

  GEOSContextHandle_t geosctxt = QgsGeos::getGEOSHandler();
  try
  {
#if GEOS_VERSION_MAJOR>3 || GEOS_VERSION_MINOR>=8
    geos::unique_ptr geosPt( GEOSGeom_createPointFromXY_r( geosctxt, px, py ) );
#else
    GEOSCoordSequence *coord = GEOSCoordSeq_create_r( geosctxt, 1, 2 );
    GEOSCoordSeq_setX_r( geosctxt, coord, 0, px );
    GEOSCoordSeq_setY_r( geosctxt, coord, 0, py );
    geos::unique_ptr geosPt( GEOSGeom_createPoint_r( geosctxt, coord ) );
#endif
    int type = GEOSGeomTypeId_r( geosctxt, mGeos );
    const GEOSGeometry *extRing = nullptr;
    if ( type != GEOS_POLYGON )
    {
      extRing = mGeos;
    }
    else
    {
      //for polygons, we want distance to exterior ring (not an interior point)
      extRing = GEOSGetExteriorRing_r( geosctxt, mGeos );
    }
    geos::coord_sequence_unique_ptr nearestCoord( GEOSNearestPoints_r( geosctxt, extRing, geosPt.get() ) );
    double nx;
    double ny;
#if GEOS_VERSION_MAJOR>3 || GEOS_VERSION_MINOR>=8
    ( void )GEOSCoordSeq_getXY_r( geosctxt, nearestCoord.get(), 0, &nx, &ny );
#else
    ( void )GEOSCoordSeq_getX_r( geosctxt, nearestCoord.get(), 0, &nx );
    ( void )GEOSCoordSeq_getY_r( geosctxt, nearestCoord.get(), 0, &ny );
#endif

    if ( rx )
      *rx = nx;
    if ( ry )
      *ry = ny;

    return GeomFunction::dist_euc2d_sq( px, py, nx, ny );
  }
  catch ( GEOSException &e )
  {
    QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );
    return 0;
  }
}

void PointSet::getCentroid( double &px, double &py, bool forceInside ) const
{
  if ( !mGeos )
    createGeosGeom();

  if ( !mGeos )
    return;

  try
  {
    GEOSContextHandle_t geosctxt = QgsGeos::getGEOSHandler();
    geos::unique_ptr centroidGeom( GEOSGetCentroid_r( geosctxt, mGeos ) );
    if ( centroidGeom )
    {
      const GEOSCoordSequence *coordSeq = GEOSGeom_getCoordSeq_r( geosctxt, centroidGeom.get() );
#if GEOS_VERSION_MAJOR>3 || GEOS_VERSION_MINOR>=8
      GEOSCoordSeq_getXY_r( geosctxt, coordSeq, 0, &px, &py );
#else
      GEOSCoordSeq_getX_r( geosctxt, coordSeq, 0, &px );
      GEOSCoordSeq_getY_r( geosctxt, coordSeq, 0, &py );
#endif
    }

    // check if centroid inside in polygon
    if ( forceInside && !containsPoint( px, py ) )
    {
      geos::unique_ptr pointGeom( GEOSPointOnSurface_r( geosctxt, mGeos ) );

      if ( pointGeom )
      {
        const GEOSCoordSequence *coordSeq = GEOSGeom_getCoordSeq_r( geosctxt, pointGeom.get() );
#if GEOS_VERSION_MAJOR>3 || GEOS_VERSION_MINOR>=8
        GEOSCoordSeq_getXY_r( geosctxt, coordSeq, 0, &px, &py );
#else
        GEOSCoordSeq_getX_r( geosctxt, coordSeq, 0, &px );
        GEOSCoordSeq_getY_r( geosctxt, coordSeq, 0, &py );
#endif
      }
    }
  }
  catch ( GEOSException &e )
  {
    QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );
    return;
  }
}

bool PointSet::boundingBoxIntersects( const PointSet *other ) const
{
  double x1 = ( xmin > other->xmin ? xmin : other->xmin );
  double x2 = ( xmax < other->xmax ? xmax : other->xmax );
  if ( x1 > x2 )
    return false;
  double y1 = ( ymin > other->ymin ? ymin : other->ymin );
  double y2 = ( ymax < other->ymax ? ymax : other->ymax );
  return y1 <= y2;
}

void PointSet::getPointByDistance( double *d, double *ad, double dl, double *px, double *py )
{
  int i;
  double dx, dy, di;
  double distr;

  i = 0;
  if ( dl >= 0 )
  {
    while ( i < nbPoints && ad[i] <= dl ) i++;
    i--;
  }

  if ( i < nbPoints - 1 )
  {
    if ( dl < 0 )
    {
      dx = x[nbPoints - 1] - x[0];
      dy = y[nbPoints - 1] - y[0];
      di = std::sqrt( dx * dx + dy * dy );
    }
    else
    {
      dx = x[i + 1] - x[i];
      dy = y[i + 1] - y[i];
      di = d[i];
    }

    distr = dl - ad[i];
    *px = x[i] + dx * distr / di;
    *py = y[i] + dy * distr / di;
  }
  else    // just select last point...
  {
    *px = x[i];
    *py = y[i];
  }
}

const GEOSGeometry *PointSet::geos() const
{
  if ( !mGeos )
    createGeosGeom();

  return mGeos;
}

double PointSet::length() const
{
  if ( !mGeos )
    createGeosGeom();

  if ( !mGeos )
    return -1;

  GEOSContextHandle_t geosctxt = QgsGeos::getGEOSHandler();

  try
  {
    double len = 0;
    ( void )GEOSLength_r( geosctxt, mGeos, &len );
    return len;
  }
  catch ( GEOSException &e )
  {
    QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );
    return -1;
  }
}

bool PointSet::isClosed() const
{
  return qgsDoubleNear( x[0], x[nbPoints - 1] ) && qgsDoubleNear( y[0], y[nbPoints - 1] );
}
