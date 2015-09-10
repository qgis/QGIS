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

#define _CRT_SECURE_NO_DEPRECATE

#include "layer.h"
#include "pal.h"
#include "costcalculator.h"
#include "feature.h"
#include "geomfunction.h"
#include "labelposition.h"
#include "qgsgeos.h"
#include "qgsmessagelog.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <cfloat>

#ifndef M_PI
#define M_PI 3.1415926535897931159979634685
#endif


namespace pal
{
  LabelPosition::LabelPosition( int id, double x1, double y1, double w, double h, double alpha, double cost, FeaturePart *feature, bool isReversed, Quadrant quadrant )
      : PointSet()
      , id( id )
      , feature( feature )
      , probFeat( 0 )
      , nbOverlap( 0 )
      , alpha( alpha )
      , w( w )
      , h( h )
      , nextPart( NULL )
      , partId( -1 )
      , reversed( isReversed )
      , upsideDown( false )
      , quadrant( quadrant )
      , mCost( cost )
      , mHasObstacleConflict( false )
  {
    type = GEOS_POLYGON;
    nbPoints = 4;
    x = new double[nbPoints];
    y = new double[nbPoints];

    // alpha take his value bw 0 and 2*pi rad
    while ( this->alpha > 2*M_PI )
      this->alpha -= 2 * M_PI;

    while ( this->alpha < 0 )
      this->alpha += 2 * M_PI;

    double beta = this->alpha + ( M_PI / 2 );

    double dx1, dx2, dy1, dy2;

    double tx, ty;

    dx1 = cos( this->alpha ) * w;
    dy1 = sin( this->alpha ) * w;

    dx2 = cos( beta ) * h;
    dy2 = sin( beta ) * h;

    x[0] = x1;
    y[0] = y1;

    x[1] = x1 + dx1;
    y[1] = y1 + dy1;

    x[2] = x1 + dx1 + dx2;
    y[2] = y1 + dy1 + dy2;

    x[3] = x1 + dx2;
    y[3] = y1 + dy2;

    // upside down ? (curved labels are always correct)
    if ( feature->layer()->arrangement() != P_CURVED &&
         this->alpha > M_PI / 2 && this->alpha <= 3*M_PI / 2 )
    {
      bool uprightLabel = false;

      switch ( feature->layer()->upsidedownLabels() )
      {
        case Layer::Upright:
          uprightLabel = true;
          break;
        case Layer::ShowDefined:
          // upright only dynamic labels
          if ( !feature->getFixedRotation() || ( !feature->getFixedPosition() && feature->getLabelAngle() == 0.0 ) )
          {
            uprightLabel = true;
          }
          break;
        case Layer::ShowAll:
          break;
        default:
          uprightLabel = true;
      }

      if ( uprightLabel )
      {
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
      xmin = qMin( xmin, x[i] );
      xmax = qMax( xmax, x[i] );
      ymin = qMin( ymin, y[i] );
      ymax = qMax( ymax, y[i] );
    }
  }

  LabelPosition::LabelPosition( const LabelPosition& other )
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

    if ( other.nextPart )
      nextPart = new LabelPosition( *other.nextPart );
    else
      nextPart = NULL;
    partId = other.partId;
    upsideDown = other.upsideDown;
    reversed = other.reversed;
    quadrant = other.quadrant;
    mHasObstacleConflict = other.mHasObstacleConflict;
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

    if ( nextPart )
      return nextPart->isIn( bbox );
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

    if ( nextPart )
      return nextPart->isIntersect( bbox );
    else
      return false;
  }

  bool LabelPosition::isInside( double *bbox )
  {
    for ( int i = 0; i < 4; i++ )
    {
      if ( !( x[i] >= bbox[0] && x[i] <= bbox[2] &&
              y[i] >= bbox[1] && y[i] <= bbox[3] ) )
        return false;
    }

    if ( nextPart )
      return nextPart->isInside( bbox );
    else
      return true;

  }

  void LabelPosition::print()
  {
    //  std::cout << feature->getLayer()->getName() << "/" << feature->getUID() << "/" << id;
    std::cout << " cost: " << mCost;
    std::cout << " alpha" << alpha << std::endl;
    std::cout << x[0] << ", " << y[0] << std::endl;
    std::cout << x[1] << ", " << y[1] << std::endl;
    std::cout << x[2] << ", " << y[2] << std::endl;
    std::cout << x[3] << ", " << y[3] << std::endl;
    std::cout << std::endl;
  }

  bool LabelPosition::isInConflict( LabelPosition *lp )
  {
    if ( this->probFeat == lp->probFeat ) // bugfix #1
      return false; // always overlaping itself !

    if ( nextPart == NULL && lp->nextPart == NULL )
      return isInConflictSinglePart( lp );
    else
      return isInConflictMultiPart( lp );
  }

  bool LabelPosition::isInConflictSinglePart( LabelPosition* lp )
  {
    if ( !mGeos )
      createGeosGeom();

    if ( !lp->mGeos )
      lp->createGeosGeom();

    GEOSContextHandle_t geosctxt = geosContext();
    try
    {
      bool result = ( GEOSPreparedIntersects_r( geosctxt, preparedGeom(), lp->mGeos ) == 1 );
      return result;
    }
    catch ( GEOSException &e )
    {
      QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );
      return false;
    }
  }

  bool LabelPosition::isInConflictMultiPart( LabelPosition* lp )
  {
    // check all parts against all parts of other one
    LabelPosition* tmp1 = this;
    while ( tmp1 )
    {
      // check tmp1 against parts of other label
      LabelPosition* tmp2 = lp;
      while ( tmp2 )
      {
        if ( tmp1->isInConflictSinglePart( tmp2 ) )
          return true;
        tmp2 = tmp2->nextPart;
      }

      tmp1 = tmp1->nextPart;
    }
    return false; // no conflict found
  }

  int LabelPosition::partCount() const
  {
    if ( nextPart )
      return nextPart->partCount() + 1;
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

    if ( nextPart )
      nextPart->offsetPosition( xOffset, yOffset );

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
      //   std::cout << " Warning: lp->cost == " << cost << " (from feat: " << feature->getUID() << "/" << getLayerName() << ")" << std::endl;
      mCost -= int ( mCost ); // label cost up to 1
    }
  }

  FeaturePart * LabelPosition::getFeaturePart()
  {
    return feature;
  }

  void LabelPosition::getBoundingBox( double amin[2], double amax[2] ) const
  {
    if ( nextPart )
    {
      //std::cout << "using next part" <<
      nextPart->getBoundingBox( amin, amax );
    }
    else
    {
      amin[0] = DBL_MAX;
      amax[0] = -DBL_MAX;
      amin[1] = DBL_MAX;
      amax[1] = -DBL_MAX;
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

  QString LabelPosition::getLayerName() const
  {
    return feature->layer()->name();
  }

  void LabelPosition::setConflictsWithObstacle( bool conflicts )
  {
    mHasObstacleConflict = conflicts;
    if ( nextPart )
      nextPart->setConflictsWithObstacle( conflicts );
  }

  bool LabelPosition::costShrink( void *l, void *r )
  {
    return (( LabelPosition* ) l )->mCost < (( LabelPosition* ) r )->mCost;
  }

  bool LabelPosition::costGrow( void *l, void *r )
  {
    return (( LabelPosition* ) l )->mCost > (( LabelPosition* ) r )->mCost;
  }

  bool LabelPosition::polygonObstacleCallback( FeaturePart *obstacle, void *ctx )
  {
    PolygonCostCalculator *pCost = ( PolygonCostCalculator* ) ctx;

    LabelPosition *lp = pCost->getLabel();
    if (( obstacle == lp->feature ) || ( obstacle->getHoleOf() && obstacle->getHoleOf() != lp->feature ) )
    {
      return true;
    }

    pCost->update( obstacle );

    return true;
  }


  void LabelPosition::removeFromIndex( RTree<LabelPosition*, double, 2, double> *index )
  {
    double amin[2];
    double amax[2];
    getBoundingBox( amin, amax );
    index->Remove( amin, amax, this );
  }


  void LabelPosition::insertIntoIndex( RTree<LabelPosition*, double, 2, double> *index )
  {
    double amin[2];
    double amax[2];
    getBoundingBox( amin, amax );
    index->Insert( amin, amax, this );
  }


  //////////

  bool LabelPosition::pruneCallback( LabelPosition *lp, void *ctx )
  {
    FeaturePart *feat = (( PruneCtx* ) ctx )->obstacle;

    if (( feat == lp->feature ) || ( feat->getHoleOf() && feat->getHoleOf() != lp->feature ) )
    {
      return true;
    }

    CostCalculator::addObstacleCostPenalty( lp, feat );

    return true;
  }


  bool LabelPosition::countOverlapCallback( LabelPosition *lp, void *ctx )
  {
    LabelPosition *lp2 = ( LabelPosition* ) ctx;

    //std::cerr << "checking " << lp2->getFeature()->getUID() << " x " << lp->getFeature()->getUID() << std::endl;
    if ( lp2->isInConflict( lp ) )
    {
      //std::cerr << "conflict!" << std::endl;
      lp2->nbOverlap++;
    }

    return true;
  }

  bool LabelPosition::countFullOverlapCallback( LabelPosition *lp, void *ctx )
  {
    LabelPosition *lp2 = (( CountContext* ) ctx )->lp;
    double *cost = (( CountContext* ) ctx )->cost;
    //int *feat = ((CountContext*)ctx)->feat;
    int *nbOv = (( CountContext* ) ctx )->nbOv;
    double *inactiveCost = (( CountContext* ) ctx )->inactiveCost;
    if ( lp2->isInConflict( lp ) )
    {
#ifdef _DEBUG_FULL_
      std::cout <<  "count overlap : " << lp->id << "<->" << lp2->id << std::endl;
#endif
      ( *nbOv ) ++;
      *cost += inactiveCost[lp->probFeat] + lp->cost();

    }

    return true;
  }


  bool LabelPosition::removeOverlapCallback( LabelPosition *lp, void *ctx )
  {
    LabelPosition *lp2 = ( LabelPosition * ) ctx;

    if ( lp2->isInConflict( lp ) )
    {
      //std::cout << "   hit !" << std::endl;
      lp->nbOverlap--;
      lp2->nbOverlap--;
    }

    return true;
  }

  double LabelPosition::getDistanceToPoint( double xp, double yp ) const
  {
    //first check if inside, if so then distance is -1
    double distance = ( containsPoint( xp, yp ) ? -1
                        : sqrt( minDistanceToPoint( xp, yp ) ) );

    if ( nextPart && distance > 0 )
      return qMin( distance, nextPart->getDistanceToPoint( xp, yp ) );

    return distance;
  }

  bool LabelPosition::crossesLine( PointSet* line ) const
  {
    if ( !mGeos )
      createGeosGeom();

    if ( !line->mGeos )
      line->createGeosGeom();

    GEOSContextHandle_t geosctxt = geosContext();
    try
    {
      if ( GEOSPreparedIntersects_r( geosctxt, line->preparedGeom(), mGeos ) == 1 )
      {
        return true;
      }
      else if ( nextPart )
      {
        return nextPart->crossesLine( line );
      }
    }
    catch ( GEOSException &e )
    {
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

    GEOSContextHandle_t geosctxt = geosContext();
    try
    {
      if ( GEOSPreparedOverlaps_r( geosctxt, polygon->preparedGeom(), mGeos ) == 1
           || GEOSPreparedTouches_r( geosctxt, polygon->preparedGeom(), mGeos ) == 1 )
      {
        return true;
      }
      else if ( nextPart )
      {
        return nextPart->crossesBoundary( polygon );
      }
    }
    catch ( GEOSException &e )
    {
      QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );
      return false;
    }

    return false;
  }

  int LabelPosition::polygonIntersectionCost( PointSet *polygon ) const
  {
    //effectively take the average polygon intersection cost for all label parts
    double totalCost = polygonIntersectionCostForParts( polygon );
    int n = partCount();
    return ceil( totalCost / n );
  }

  double LabelPosition::polygonIntersectionCostForParts( PointSet *polygon ) const
  {
    if ( !mGeos )
      createGeosGeom();

    if ( !polygon->mGeos )
      polygon->createGeosGeom();

    GEOSContextHandle_t geosctxt = geosContext();

    double cost = 0;
    //check the label center. if covered by polygon, initial cost of 4
    if ( polygon->containsPoint(( x[0] + x[2] ) / 2.0, ( y[0] + y[2] ) / 2.0 ) )
      cost += 4;

    try
    {
      //calculate proportion of label candidate which is covered by polygon
      GEOSGeometry* intersectionGeom = GEOSIntersection_r( geosctxt, mGeos, polygon->mGeos );
      if ( intersectionGeom )
      {
        double positionArea = 0;
        if ( GEOSArea_r( geosctxt, mGeos, &positionArea ) == 1 )
        {
          double intersectionArea = 0;
          if ( GEOSArea_r( geosctxt, intersectionGeom, &intersectionArea ) == 1 )
          {
            double portionCovered = intersectionArea / positionArea;
            cost += portionCovered * 8.0; //cost of 8 if totally covered
          }
        }
        GEOSGeom_destroy_r( geosctxt, intersectionGeom );
      }
    }
    catch ( GEOSException &e )
    {
      QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );
    }

    if ( nextPart )
    {
      cost += nextPart->polygonIntersectionCostForParts( polygon );
    }

    return cost;
  }

} // end namespace

