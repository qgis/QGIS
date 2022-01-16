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

#include "layer.h"
#include "pal.h"
#include "costcalculator.h"
#include "feature.h"
#include "geomfunction.h"
#include "labelposition.h"
#include "qgsgeos.h"
#include "qgsmessagelog.h"
#include <cmath>
#include <cfloat>

using namespace pal;

LabelPosition::LabelPosition( int id, double x1, double y1, double w, double h, double alpha, double cost, FeaturePart *feature, bool isReversed, Quadrant quadrant )
  : id( id )
  , feature( feature )
  , probFeat( 0 )
  , nbOverlap( 0 )
  , alpha( alpha )
  , w( w )
  , h( h )
  , partId( -1 )
  , reversed( isReversed )
  , upsideDown( false )
  , quadrant( quadrant )
  , mCost( cost )
  , mHasObstacleConflict( false )
  , mUpsideDownCharCount( 0 )
{
  type = GEOS_POLYGON;
  nbPoints = 4;
  x.resize( nbPoints );
  y.resize( nbPoints );

  // alpha take his value bw 0 and 2*pi rad
  while ( this->alpha > 2 * M_PI )
    this->alpha -= 2 * M_PI;

  while ( this->alpha < 0 )
    this->alpha += 2 * M_PI;

  const double beta = this->alpha + M_PI_2;

  double dx1, dx2, dy1, dy2;

  dx1 = std::cos( this->alpha ) * w;
  dy1 = std::sin( this->alpha ) * w;

  dx2 = std::cos( beta ) * h;
  dy2 = std::sin( beta ) * h;

  x[0] = x1;
  y[0] = y1;

  x[1] = x1 + dx1;
  y[1] = y1 + dy1;

  x[2] = x1 + dx1 + dx2;
  y[2] = y1 + dy1 + dy2;

  x[3] = x1 + dx2;
  y[3] = y1 + dy2;

  // upside down ? (curved labels are always correct)
  if ( !feature->layer()->isCurved() &&
       this->alpha > M_PI_2 && this->alpha <= 3 * M_PI_2 )
  {
    if ( feature->onlyShowUprightLabels() )
    {
      // Turn label upsidedown by inverting boundary points
      double tx, ty;

      tx = x[0];
      ty = y[0];

      x[0] = x[2];
      y[0] = y[2];

      x[2] = tx;
      y[2] = ty;

      tx = x[1];
      ty = y[1];

      x[1] = x[3];
      y[1] = y[3];

      x[3] = tx;
      y[3] = ty;

      if ( this->alpha < M_PI )
        this->alpha += M_PI;
      else
        this->alpha -= M_PI;

      // labels with text shown upside down are not classified as upsideDown,
      // only those whose boundary points have been inverted
      upsideDown = true;
    }
  }

  for ( int i = 0; i < nbPoints; ++i )
  {
    xmin = std::min( xmin, x[i] );
    xmax = std::max( xmax, x[i] );
    ymin = std::min( ymin, y[i] );
    ymax = std::max( ymax, y[i] );
  }
}

LabelPosition::LabelPosition( const LabelPosition &other )
  : PointSet( other )
{
  id = other.id;
  mCost = other.mCost;
  feature = other.feature;
  probFeat = other.probFeat;
  nbOverlap = other.nbOverlap;

  alpha = other.alpha;
  w = other.w;
  h = other.h;

  if ( other.mNextPart )
    mNextPart = std::make_unique< LabelPosition >( *other.mNextPart );

  partId = other.partId;
  upsideDown = other.upsideDown;
  reversed = other.reversed;
  quadrant = other.quadrant;
  mHasObstacleConflict = other.mHasObstacleConflict;
  mUpsideDownCharCount = other.mUpsideDownCharCount;
}

bool LabelPosition::isIn( double *bbox )
{
  int i;

  for ( i = 0; i < 4; i++ )
  {
    if ( x[i] >= bbox[0] && x[i] <= bbox[2] &&
         y[i] >= bbox[1] && y[i] <= bbox[3] )
      return true;
  }

  if ( mNextPart )
    return mNextPart->isIn( bbox );
  else
    return false;
}

bool LabelPosition::isIntersect( double *bbox )
{
  int i;

  for ( i = 0; i < 4; i++ )
  {
    if ( x[i] >= bbox[0] && x[i] <= bbox[2] &&
         y[i] >= bbox[1] && y[i] <= bbox[3] )
      return true;
  }

  if ( mNextPart )
    return mNextPart->isIntersect( bbox );
  else
    return false;
}

bool LabelPosition::intersects( const GEOSPreparedGeometry *geometry )
{
  if ( !mGeos )
    createGeosGeom();

  try
  {
    if ( GEOSPreparedIntersects_r( QgsGeos::getGEOSHandler(), geometry, mGeos ) == 1 )
    {
      return true;
    }
    else if ( mNextPart )
    {
      return mNextPart->intersects( geometry );
    }
  }
  catch ( GEOSException &e )
  {
    qWarning( "GEOS exception: %s", e.what() );
    QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );
    return false;
  }

  return false;
}

bool LabelPosition::within( const GEOSPreparedGeometry *geometry )
{
  if ( !mGeos )
    createGeosGeom();

  try
  {
    if ( GEOSPreparedContains_r( QgsGeos::getGEOSHandler(), geometry, mGeos ) != 1 )
    {
      return false;
    }
    else if ( mNextPart )
    {
      return mNextPart->within( geometry );
    }
  }
  catch ( GEOSException &e )
  {
    qWarning( "GEOS exception: %s", e.what() );
    QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );
    return false;
  }

  return true;
}

bool LabelPosition::isInside( double *bbox )
{
  for ( int i = 0; i < 4; i++ )
  {
    if ( !( x[i] >= bbox[0] && x[i] <= bbox[2] &&
            y[i] >= bbox[1] && y[i] <= bbox[3] ) )
      return false;
  }

  if ( mNextPart )
    return mNextPart->isInside( bbox );
  else
    return true;
}

bool LabelPosition::isInConflict( const LabelPosition *lp ) const
{
  if ( this->probFeat == lp->probFeat ) // bugfix #1
    return false; // always overlaping itself !

  if ( !nextPart() && !lp->nextPart() )
  {
    if ( qgsDoubleNear( alpha, 0 ) && qgsDoubleNear( lp->alpha, 0 ) )
    {
      // simple case -- both candidates are oriented to axis, so shortcut with easy calculation
      return boundingBoxIntersects( lp );
    }
  }

  return isInConflictMultiPart( lp );
}

bool LabelPosition::isInConflictMultiPart( const LabelPosition *lp ) const
{
  if ( !mMultipartGeos )
    createMultiPartGeosGeom();

  if ( !lp->mMultipartGeos )
    lp->createMultiPartGeosGeom();

  GEOSContextHandle_t geosctxt = QgsGeos::getGEOSHandler();
  try
  {
    const bool result = ( GEOSPreparedIntersects_r( geosctxt, preparedMultiPartGeom(), lp->mMultipartGeos ) == 1 );
    return result;
  }
  catch ( GEOSException &e )
  {
    qWarning( "GEOS exception: %s", e.what() );
    QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );
    return false;
  }

  return false;
}

int LabelPosition::partCount() const
{
  if ( mNextPart )
    return mNextPart->partCount() + 1;
  else
    return 1;
}

void LabelPosition::offsetPosition( double xOffset, double yOffset )
{
  for ( int i = 0; i < 4; i++ )
  {
    x[i] += xOffset;
    y[i] += yOffset;
  }

  if ( mNextPart )
    mNextPart->offsetPosition( xOffset, yOffset );

  invalidateGeos();
}

int LabelPosition::getId() const
{
  return id;
}

double LabelPosition::getX( int i ) const
{
  return ( i >= 0 && i < 4 ? x[i] : -1 );
}

double LabelPosition::getY( int i ) const
{
  return ( i >= 0 && i < 4 ? y[i] : -1 );
}

double LabelPosition::getAlpha() const
{
  return alpha;
}

void LabelPosition::validateCost()
{
  if ( mCost >= 1 )
  {
    mCost -= int ( mCost ); // label cost up to 1
  }
}

FeaturePart *LabelPosition::getFeaturePart() const
{
  return feature;
}

void LabelPosition::getBoundingBox( double amin[2], double amax[2] ) const
{
  if ( mNextPart )
  {
    mNextPart->getBoundingBox( amin, amax );
  }
  else
  {
    amin[0] = std::numeric_limits<double>::max();
    amax[0] = std::numeric_limits<double>::lowest();
    amin[1] = std::numeric_limits<double>::max();
    amax[1] = std::numeric_limits<double>::lowest();
  }
  for ( int c = 0; c < 4; c++ )
  {
    if ( x[c] < amin[0] )
      amin[0] = x[c];
    if ( x[c] > amax[0] )
      amax[0] = x[c];
    if ( y[c] < amin[1] )
      amin[1] = y[c];
    if ( y[c] > amax[1] )
      amax[1] = y[c];
  }
}

void LabelPosition::setConflictsWithObstacle( bool conflicts )
{
  mHasObstacleConflict = conflicts;
  if ( mNextPart )
    mNextPart->setConflictsWithObstacle( conflicts );
}

void LabelPosition::setHasHardObstacleConflict( bool conflicts )
{
  mHasHardConflict = conflicts;
  if ( mNextPart )
    mNextPart->setHasHardObstacleConflict( conflicts );
}

void LabelPosition::removeFromIndex( PalRtree<LabelPosition> &index )
{
  double amin[2];
  double amax[2];
  getBoundingBox( amin, amax );
  index.remove( this, QgsRectangle( amin[0], amin[1], amax[0], amax[1] ) );
}

void LabelPosition::insertIntoIndex( PalRtree<LabelPosition> &index )
{
  double amin[2];
  double amax[2];
  getBoundingBox( amin, amax );
  index.insert( this, QgsRectangle( amin[0], amin[1], amax[0], amax[1] ) );
}


void LabelPosition::createMultiPartGeosGeom() const
{
  GEOSContextHandle_t geosctxt = QgsGeos::getGEOSHandler();

  std::vector< const GEOSGeometry * > geometries;
  const LabelPosition *tmp1 = this;
  while ( tmp1 )
  {
    const GEOSGeometry *partGeos = tmp1->geos();
    if ( !GEOSisEmpty_r( geosctxt, partGeos ) )
      geometries.emplace_back( partGeos );
    tmp1 = tmp1->nextPart();
  }

  const std::size_t partCount = geometries.size();
  GEOSGeometry **geomarr = new GEOSGeometry*[ partCount ];
  for ( std::size_t i = 0; i < partCount; ++i )
  {
    geomarr[i ] = GEOSGeom_clone_r( geosctxt, geometries[i] );
  }

  mMultipartGeos = GEOSGeom_createCollection_r( geosctxt, GEOS_MULTIPOLYGON, geomarr, partCount );
  delete [] geomarr;
}

const GEOSPreparedGeometry *LabelPosition::preparedMultiPartGeom() const
{
  if ( !mMultipartGeos )
    createMultiPartGeosGeom();

  if ( !mMultipartPreparedGeos )
  {
    mMultipartPreparedGeos = GEOSPrepare_r( QgsGeos::getGEOSHandler(), mMultipartGeos );
  }
  return mMultipartPreparedGeos;
}

double LabelPosition::getDistanceToPoint( double xp, double yp ) const
{
  //first check if inside, if so then distance is -1
  bool contains = false;
  if ( alpha == 0 )
  {
    // easy case -- horizontal label
    contains = x[0] <= xp && x[1] >= xp && y[0] <= yp && y[2] >= yp;
  }
  else
  {
    contains = containsPoint( xp, yp );
  }

  double distance = -1;
  if ( !contains )
  {
    if ( alpha == 0 )
    {
      const double dx = std::max( std::max( x[0] - xp, 0.0 ), xp - x[1] );
      const double dy = std::max( std::max( y[0] - yp, 0.0 ), yp - y[2] );
      distance = std::sqrt( dx * dx + dy * dy );
    }
    else
    {
      distance = std::sqrt( minDistanceToPoint( xp, yp ) );
    }
  }

  if ( mNextPart && distance > 0 )
    return std::min( distance, mNextPart->getDistanceToPoint( xp, yp ) );

  return distance;
}

bool LabelPosition::crossesLine( PointSet *line ) const
{
  if ( !mGeos )
    createGeosGeom();

  if ( !line->mGeos )
    line->createGeosGeom();

  GEOSContextHandle_t geosctxt = QgsGeos::getGEOSHandler();
  try
  {
    if ( GEOSPreparedIntersects_r( geosctxt, line->preparedGeom(), mGeos ) == 1 )
    {
      return true;
    }
    else if ( mNextPart )
    {
      return mNextPart->crossesLine( line );
    }
  }
  catch ( GEOSException &e )
  {
    qWarning( "GEOS exception: %s", e.what() );
    QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );
    return false;
  }

  return false;
}

bool LabelPosition::crossesBoundary( PointSet *polygon ) const
{
  if ( !mGeos )
    createGeosGeom();

  if ( !polygon->mGeos )
    polygon->createGeosGeom();

  GEOSContextHandle_t geosctxt = QgsGeos::getGEOSHandler();
  try
  {
    if ( GEOSPreparedIntersects_r( geosctxt, polygon->preparedGeom(), mGeos ) == 1
         && GEOSPreparedContains_r( geosctxt, polygon->preparedGeom(), mGeos ) != 1 )
    {
      return true;
    }
    else if ( mNextPart )
    {
      return mNextPart->crossesBoundary( polygon );
    }
  }
  catch ( GEOSException &e )
  {
    qWarning( "GEOS exception: %s", e.what() );
    QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );
    return false;
  }

  return false;
}

int LabelPosition::polygonIntersectionCost( PointSet *polygon ) const
{
  //effectively take the average polygon intersection cost for all label parts
  const double totalCost = polygonIntersectionCostForParts( polygon );
  const int n = partCount();
  return std::ceil( totalCost / n );
}

bool LabelPosition::intersectsWithPolygon( PointSet *polygon ) const
{
  if ( !mGeos )
    createGeosGeom();

  if ( !polygon->mGeos )
    polygon->createGeosGeom();

  GEOSContextHandle_t geosctxt = QgsGeos::getGEOSHandler();
  try
  {
    if ( GEOSPreparedIntersects_r( geosctxt, polygon->preparedGeom(), mGeos ) == 1 )
    {
      return true;
    }
  }
  catch ( GEOSException &e )
  {
    qWarning( "GEOS exception: %s", e.what() );
    QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );
  }

  if ( mNextPart )
  {
    return mNextPart->intersectsWithPolygon( polygon );
  }
  else
  {
    return false;
  }
}

double LabelPosition::polygonIntersectionCostForParts( PointSet *polygon ) const
{
  if ( !mGeos )
    createGeosGeom();

  if ( !polygon->mGeos )
    polygon->createGeosGeom();

  GEOSContextHandle_t geosctxt = QgsGeos::getGEOSHandler();
  double cost = 0;
  try
  {
    if ( GEOSPreparedIntersects_r( geosctxt, polygon->preparedGeom(), mGeos ) == 1 )
    {
      //at least a partial intersection
      cost += 1;

      double px, py;

      // check each corner
      for ( int i = 0; i < 4; ++i )
      {
        px = x[i];
        py = y[i];

        for ( int a = 0; a < 2; ++a ) // and each middle of segment
        {
          if ( polygon->containsPoint( px, py ) )
            cost++;
          px = ( x[i] + x[( i + 1 ) % 4] ) / 2.0;
          py = ( y[i] + y[( i + 1 ) % 4] ) / 2.0;
        }
      }

      px = ( x[0] + x[2] ) / 2.0;
      py = ( y[0] + y[2] ) / 2.0;

      //check the label center. if covered by polygon, cost of 4
      if ( polygon->containsPoint( px, py ) )
        cost += 4;
    }
  }
  catch ( GEOSException &e )
  {
    qWarning( "GEOS exception: %s", e.what() );
    QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );
  }

  //maintain scaling from 0 -> 12
  cost = 12.0 * cost / 13.0;

  if ( mNextPart )
  {
    cost += mNextPart->polygonIntersectionCostForParts( polygon );
  }

  return cost;
}

double LabelPosition::angleDifferential()
{
  double angleDiff = 0.0;
  double angleLast = 0.0;
  LabelPosition *tmp = this;
  while ( tmp )
  {
    if ( tmp != this ) // not first?
    {
      double diff = std::fabs( tmp->getAlpha() - angleLast );
      if ( diff > 2 * M_PI )
        diff -= 2 * M_PI;
      diff = std::min( diff, 2 * M_PI - diff ); // difference 350 deg is actually just 10 deg...
      angleDiff += diff;
    }

    angleLast = tmp->getAlpha();
    tmp = tmp->nextPart();
  }
  return angleDiff;
}
