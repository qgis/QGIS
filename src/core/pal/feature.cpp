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


#if defined(_VERBOSE_) || (_DEBUG_)
#include <iostream>
#endif

#include "qgsgeometry.h"
#include "pal.h"
#include "layer.h"
#include "feature.h"
#include "geomfunction.h"
#include "labelposition.h"
#include "pointset.h"
#include "util.h"
#include "qgis.h"
#include <QLinkedList>
#include <cmath>
#include <cfloat>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace pal
{
  Feature::Feature( Layer* l, const QString &geom_id, PalGeometry* userG, double lx, double ly )
      : layer( l )
      , userGeom( userG )
      , label_x( lx )
      , label_y( ly )
      , distlabel( 0 )
      , labelInfo( NULL )
      , uid( geom_id )
      , fixedPos( false )
      , fixedPosX( 0.0 )
      , fixedPosY( 0.0 )
      , quadOffset( false )
      , quadOffsetX( 0.0 )
      , quadOffsetY( 0.0 )
      , offsetPos( false )
      , offsetPosX( 0.0 )
      , offsetPosY( 0.0 )
      , fixedRotation( false )
      , fixedAngle( 0.0 )
      , repeatDist( 0.0 )
      , alwaysShow( false )
      , mFixedQuadrant( false )
      , mIsObstacle( true )
      , mObstacleFactor( 1.0 )
      , mPriority( -1.0 )
  {
    assert( finite( lx ) && finite( ly ) );
  }

  Feature::~Feature()
  {

  }

  double Feature::calculatePriority() const
  {
    return mPriority >= 0 ? mPriority : layer->priority();
  }

  ////////////

  FeaturePart::FeaturePart( Feature *feat, const GEOSGeometry* geom )
      : mFeature( feat )
  {
    // we'll remove const, but we won't modify that geometry
    mGeos = const_cast<GEOSGeometry*>( geom );
    mOwnsGeom = false; // geometry is owned by Feature class

    extractCoords( geom );

    holeOf = NULL;
    for ( int i = 0; i < mHoles.count(); i++ )
    {
      mHoles.at( i )->holeOf = this;
    }
  }


  FeaturePart::~FeaturePart()
  {
    // X and Y are deleted in PointSet

    qDeleteAll( mHoles );
    mHoles.clear();

  }

  void FeaturePart::extractCoords( const GEOSGeometry* geom )
  {
    const GEOSCoordSequence *coordSeq;
    GEOSContextHandle_t geosctxt = geosContext();

    type = GEOSGeomTypeId_r( geosctxt, geom );

    if ( type == GEOS_POLYGON )
    {
      if ( GEOSGetNumInteriorRings_r( geosctxt, geom ) > 0 )
      {
        int numHoles = GEOSGetNumInteriorRings_r( geosctxt, geom );

        for ( int i = 0; i < numHoles; ++i )
        {
          const GEOSGeometry* interior =  GEOSGetInteriorRingN_r( geosctxt, geom, i );
          FeaturePart* hole = new FeaturePart( mFeature, interior );
          hole->holeOf = NULL;
          // possibly not needed. it's not done for the exterior ring, so I'm not sure
          // why it's just done here...
          reorderPolygon( hole->nbPoints, hole->x, hole->y );

          mHoles << hole;
        }
      }

      // use exterior ring for the extraction of coordinates that follows
      geom = GEOSGetExteriorRing_r( geosctxt, geom );
    }
    else
    {
      qDeleteAll( mHoles );
      mHoles.clear();
    }

    // find out number of points
    nbPoints = GEOSGetNumCoordinates_r( geosctxt, geom );
    coordSeq = GEOSGeom_getCoordSeq_r( geosctxt, geom );

    // initialize bounding box
    xmin = ymin = DBL_MAX;
    xmax = ymax = -DBL_MAX;

    // initialize coordinate arrays
    x = new double[nbPoints];
    y = new double[nbPoints];

    for ( int i = 0; i < nbPoints; ++i )
    {
      GEOSCoordSeq_getX_r( geosctxt, coordSeq, i, &x[i] );
      GEOSCoordSeq_getY_r( geosctxt, coordSeq, i, &y[i] );

      xmax = x[i] > xmax ? x[i] : xmax;
      xmin = x[i] < xmin ? x[i] : xmin;

      ymax = y[i] > ymax ? y[i] : ymax;
      ymin = y[i] < ymin ? y[i] : ymin;
    }
  }

  Layer* FeaturePart::layer()
  {
    return mFeature->layer;
  }

  QString FeaturePart::getUID() const
  {
    return mFeature->uid;
  }

  LabelPosition::Quadrant FeaturePart::quadrantFromOffset() const
  {
    if ( mFeature->quadOffsetX < 0 )
    {
      if ( mFeature->quadOffsetY < 0 )
      {
        return LabelPosition::QuadrantAboveLeft;
      }
      else if ( mFeature->quadOffsetY > 0 )
      {
        return LabelPosition::QuadrantBelowLeft;
      }
      else
      {
        return LabelPosition::QuadrantLeft;
      }
    }
    else  if ( mFeature->quadOffsetX > 0 )
    {
      if ( mFeature->quadOffsetY < 0 )
      {
        return LabelPosition::QuadrantAboveRight;
      }
      else if ( mFeature->quadOffsetY > 0 )
      {
        return LabelPosition::QuadrantBelowRight;
      }
      else
      {
        return LabelPosition::QuadrantRight;
      }
    }
    else
    {
      if ( mFeature->quadOffsetY < 0 )
      {
        return LabelPosition::QuadrantAbove;
      }
      else if ( mFeature->quadOffsetY > 0 )
      {
        return LabelPosition::QuadrantBelow;
      }
      else
      {
        return LabelPosition::QuadrantOver;
      }
    }
  }

  int FeaturePart::setPositionOverPoint( double x, double y, LabelPosition ***lPos, double angle, PointSet *mapShape )
  {
    int nbp = 1;
    *lPos = new LabelPosition *[nbp];

    // get from feature
    double labelW = mFeature->label_x;
    double labelH = mFeature->label_y;

    double cost = 0.0001;
    int id = 0;

    double xdiff = -labelW / 2.0;
    double ydiff = -labelH / 2.0;

    if ( mFeature->quadOffset )
    {
      if ( mFeature->quadOffsetX != 0 )
      {
        xdiff += labelW / 2.0 * mFeature->quadOffsetX;
      }
      if ( mFeature->quadOffsetY != 0 )
      {
        ydiff += labelH / 2.0 * mFeature->quadOffsetY;
      }
    }

    if ( ! mFeature->fixedPosition() )
    {
      if ( angle != 0 )
      {
        double xd = xdiff * cos( angle ) - ydiff * sin( angle );
        double yd = xdiff * sin( angle ) + ydiff * cos( angle );
        xdiff = xd;
        ydiff = yd;
      }
    }

    if ( mFeature->layer->arrangement() == P_POINT )
    {
      //if in "around point" placement mode, then we use the label distance to determine
      //the label's offset
      if ( qgsDoubleNear( mFeature->quadOffsetX , 0.0 ) )
      {
        ydiff += mFeature->quadOffsetY * mFeature->distlabel;
      }
      else if ( qgsDoubleNear( mFeature->quadOffsetY, 0.0 ) )
      {
        xdiff += mFeature->quadOffsetX * mFeature->distlabel;
      }
      else
      {
        xdiff += mFeature->quadOffsetX * M_SQRT1_2 * mFeature->distlabel;
        ydiff += mFeature->quadOffsetY * M_SQRT1_2 * mFeature->distlabel;
      }
    }
    else if ( mFeature->offsetPos )
    {
      if ( mFeature->offsetPosX != 0 )
      {
        xdiff += mFeature->offsetPosX;
      }
      if ( mFeature->offsetPosY != 0 )
      {
        ydiff += mFeature->offsetPosY;
      }
    }

    double lx = x + xdiff;
    double ly = y + ydiff;

    if ( mapShape && type == GEOS_POLYGON && mFeature->layer->fitInPolygonOnly() )
    {
      if ( !mapShape->containsLabelCandidate( lx, ly, labelW, labelH, angle ) )
      {
        delete[] *lPos;
        *lPos = 0;
        return 0;
      }
    }

    ( *lPos )[0] = new LabelPosition( id, lx, ly, labelW, labelH, angle, cost, this, false, quadrantFromOffset() );
    return nbp;
  }

  int FeaturePart::setPositionForPoint( double x, double y, LabelPosition ***lPos, double angle, PointSet *mapShape )
  {

#ifdef _DEBUG_
    std::cout << "SetPosition (point) : " << layer->name << "/" << uid << std::endl;
#endif

    double labelWidth = mFeature->label_x;
    double labelHeight = mFeature->label_y;
    double distanceToLabel = mFeature->distlabel;

    int numberCandidates = mFeature->layer->pal->point_p;

    //std::cout << "Nbp : " << nbp << std::endl;
    int icost = 0;
    int inc = 2;

    double candidateAngleIncrement = 2 * M_PI / numberCandidates; /* angle bw 2 pos */

    /* various angles */
    double a90  = M_PI / 2;
    double a180 = M_PI;
    double a270 = a180 + a90;
    double a360 = 2 * M_PI;

    double gamma1, gamma2;

    if ( distanceToLabel > 0 )
    {
      gamma1 = atan2( labelHeight / 2, distanceToLabel + labelWidth / 2 );
      gamma2 = atan2( labelWidth / 2, distanceToLabel + labelHeight / 2 );
    }
    else
    {
      gamma1 = gamma2 = a90 / 3.0;
    }

    if ( gamma1 > a90 / 3.0 )
      gamma1 = a90 / 3.0;

    if ( gamma2 > a90 / 3.0 )
      gamma2 = a90 / 3.0;


    if ( gamma1 == 0 || gamma2 == 0 )
    {
      std::cout << "Oups... label size error..." << std::endl;
    }

    QList< LabelPosition* > candidates;

    int i;
    double angleToCandidate;
    for ( i = 0, angleToCandidate = M_PI / 4; i < numberCandidates; i++, angleToCandidate += candidateAngleIncrement )
    {
      double labelX = x;
      double labelY = y;

      if ( angleToCandidate > a360 )
        angleToCandidate -= a360;

      LabelPosition::Quadrant quadrant = LabelPosition::QuadrantOver;

      if ( angleToCandidate < gamma1 || angleToCandidate > a360 - gamma1 )  // on the right
      {
        labelX += distanceToLabel;
        double iota = ( angleToCandidate + gamma1 );
        if ( iota > a360 - gamma1 )
          iota -= a360;

        //ly += -yrm/2.0 + tan(alpha)*(distlabel + xrm/2);
        labelY += -labelHeight + labelHeight * iota / ( 2 * gamma1 );

        quadrant = LabelPosition::QuadrantRight;
      }
      else if ( angleToCandidate < a90 - gamma2 )  // top-right
      {
        labelX += distanceToLabel * cos( angleToCandidate );
        labelY += distanceToLabel * sin( angleToCandidate );
        quadrant = LabelPosition::QuadrantAboveRight;
      }
      else if ( angleToCandidate < a90 + gamma2 ) // top
      {
        //lx += -xrm/2.0 - tan(alpha+a90)*(distlabel + yrm/2);
        labelX += -labelWidth * ( angleToCandidate - a90 + gamma2 ) / ( 2 * gamma2 );
        labelY += distanceToLabel;
        quadrant = LabelPosition::QuadrantAbove;
      }
      else if ( angleToCandidate < a180 - gamma1 )  // top left
      {
        labelX += distanceToLabel * cos( angleToCandidate ) - labelWidth;
        labelY += distanceToLabel * sin( angleToCandidate );
        quadrant = LabelPosition::QuadrantAboveLeft;
      }
      else if ( angleToCandidate < a180 + gamma1 ) // left
      {
        labelX += -distanceToLabel - labelWidth;
        //ly += -yrm/2.0 - tan(alpha)*(distlabel + xrm/2);
        labelY += - ( angleToCandidate - a180 + gamma1 ) * labelHeight / ( 2 * gamma1 );
        quadrant = LabelPosition::QuadrantLeft;
      }
      else if ( angleToCandidate < a270 - gamma2 ) // down - left
      {
        labelX += distanceToLabel * cos( angleToCandidate ) - labelWidth;
        labelY += distanceToLabel * sin( angleToCandidate ) - labelHeight;
        quadrant = LabelPosition::QuadrantBelowLeft;
      }
      else if ( angleToCandidate < a270 + gamma2 ) // down
      {
        labelY += -distanceToLabel - labelHeight;
        //lx += -xrm/2.0 + tan(alpha+a90)*(distlabel + yrm/2);
        labelX += -labelWidth + ( angleToCandidate - a270 + gamma2 ) * labelWidth / ( 2 * gamma2 );
        quadrant = LabelPosition::QuadrantBelow;
      }
      else if ( angleToCandidate < a360 ) // down - right
      {
        labelX += distanceToLabel * cos( angleToCandidate );
        labelY += distanceToLabel * sin( angleToCandidate ) - labelHeight;
        quadrant = LabelPosition::QuadrantBelowRight;
      }

      double cost;

      if ( numberCandidates == 1 )
        cost = 0.0001;
      else
        cost = 0.0001 + 0.0020 * double( icost ) / double( numberCandidates - 1 );


      if ( mapShape && type == GEOS_POLYGON && mFeature->layer->fitInPolygonOnly() )
      {
        if ( !mapShape->containsLabelCandidate( labelX, labelY, labelWidth, labelHeight, angle ) )
        {
          continue;
        }
      }

      candidates << new LabelPosition( i, labelX, labelY, labelWidth, labelHeight, angle, cost, this, false, quadrant );

      icost += inc;

      if ( icost == numberCandidates )
      {
        icost = numberCandidates - 1;
        inc = -2;
      }
      else if ( icost > numberCandidates )
      {
        icost = numberCandidates - 2;
        inc = -2;
      }

    }

    if ( !candidates.isEmpty() )
    {
      *lPos = new LabelPosition *[candidates.count()];
      for ( int i = 0; i < candidates.count(); ++i )
      {
        ( *lPos )[i] = candidates.at( i );
      }
    }

    return candidates.count();
  }

// TODO work with squared distance by removing call to sqrt or dist_euc2d
  int FeaturePart::setPositionForLine( LabelPosition ***lPos, PointSet *mapShape )
  {
#ifdef _DEBUG_
    std::cout << "SetPosition (line) : " << layer->name << "/" << uid << std::endl;
#endif
    int i;
    double distlabel = mFeature->distlabel;

    double xrm = mFeature->label_x;
    double yrm = mFeature->label_y;

    double dist;
    double bx, by, ex, ey;
    int nbls;
    double alpha;
    double cost;

    LineArrangementFlags flags = mFeature->layer->arrangementFlags();
    if ( flags == 0 )
      flags = FLAG_ON_LINE; // default flag

    QLinkedList<LabelPosition*> positions;

    int nbPoints;
    double *x;
    double *y;

    PointSet * line = mapShape;
#ifdef _DEBUG_FULL_
    std::cout << "New line of " << line->nbPoints << " points with label " << xrm << "x" << yrm << std::endl;
#endif

    nbPoints = line->nbPoints;
    x = line->x;
    y = line->y;

    double ll = line->length(); // line length
    nbls = ( int )( ll / xrm ); // ratio bw line length and label width

#ifdef _DEBUG_FULL_
    std::cout << "line length :" << ll << std::endl;
    std::cout << "nblp :" << nbls << std::endl;
#endif

    dist = ( ll - xrm );

    double l;

    if ( nbls > 0 )
    {
      //dist /= nbls;
      l = 0;
      dist = qMin( yrm, xrm );
    }
    else   // line length < label with => centering label position
    {
      l = - ( xrm - ll ) / 2.0;
      dist = xrm;
      ll = xrm;
    }

    double birdfly;
    double beta;
    i = 0;
    //for (i=0;i<nbp;i++){
#ifdef _DEBUG_FULL_
    std::cout << l << " / " << ll - xrm << std::endl;
#endif
    while ( l < ll - xrm )
    {
      // => bx, by
      line->getPointByDistance( l, &bx, &by );
      // same but l = l+xrm
      line->getPointByDistance( l + xrm, &ex, &ey );

      // Label is bigger than line ...
      if ( l < 0 )
        birdfly = sqrt(( x[nbPoints-1] - x[0] ) * ( x[nbPoints-1] - x[0] )
                       + ( y[nbPoints-1] - y[0] ) * ( y[nbPoints-1] - y[0] ) );
      else
        birdfly = sqrt(( ex - bx ) * ( ex - bx ) + ( ey - by ) * ( ey - by ) );

      cost = birdfly / xrm;
      if ( cost > 0.98 )
        cost = 0.0001;
      else
        cost = ( 1 - cost ) / 100; // < 0.0001, 0.01 > (but 0.005 is already pretty much)

      // penalize positions which are further from the line's midpoint
      double costCenter = qAbs( ll / 2 - ( l + xrm / 2 ) ) / ll; // <0, 0.5>
      cost += costCenter / 1000;  // < 0, 0.0005 >

      if ( qgsDoubleNear( ey, by ) && qgsDoubleNear( ex, bx ) )
      {
        std::cout << "b: " << bx << ";" << by << std::endl;
        std::cout << "e: " << ex << ";" << ey << std::endl;
        alpha = 0.0;
      }
      else
        alpha = atan2( ey - by, ex - bx );

      beta = alpha + M_PI / 2;

#ifdef _DEBUG_FULL_
      std::cout << "  Create new label" << std::endl;
#endif
      if ( mFeature->layer->arrangement() == P_LINE )
      {
        // find out whether the line direction for this candidate is from right to left
        bool isRightToLeft = ( alpha > M_PI / 2 || alpha <= -M_PI / 2 );
        // meaning of above/below may be reversed if using line position dependent orientation
        // and the line has right-to-left direction
        bool reversed = (( flags & FLAG_MAP_ORIENTATION ) ? isRightToLeft : false );
        bool aboveLine = ( !reversed && ( flags & FLAG_ABOVE_LINE ) ) || ( reversed && ( flags & FLAG_BELOW_LINE ) );
        bool belowLine = ( !reversed && ( flags & FLAG_BELOW_LINE ) ) || ( reversed && ( flags & FLAG_ABOVE_LINE ) );

        if ( aboveLine )
          positions.append( new LabelPosition( i, bx + cos( beta ) *distlabel, by + sin( beta ) *distlabel, xrm, yrm, alpha, cost, this, isRightToLeft ) ); // Line
        if ( belowLine )
          positions.append( new LabelPosition( i, bx - cos( beta ) *( distlabel + yrm ), by - sin( beta ) *( distlabel + yrm ), xrm, yrm, alpha, cost, this, isRightToLeft ) );   // Line
        if ( flags & FLAG_ON_LINE )
          positions.append( new LabelPosition( i, bx - yrm*cos( beta ) / 2, by - yrm*sin( beta ) / 2, xrm, yrm, alpha, cost, this, isRightToLeft ) ); // Line
      }
      else if ( mFeature->layer->arrangement() == P_HORIZ )
      {
        positions.append( new LabelPosition( i, bx - xrm / 2, by - yrm / 2, xrm, yrm, 0, cost, this ) ); // Line
      }
      else
      {
        // an invalid arrangement?
      }

      l += dist;

      i++;

      if ( nbls == 0 )
        break;
    }

    int nbp = positions.size();
    *lPos = new LabelPosition *[nbp];
    i = 0;
    while ( positions.size() > 0 )
    {
      ( *lPos )[i] = positions.takeFirst();
      i++;
    }

    return nbp;
  }


  LabelPosition* FeaturePart::curvedPlacementAtOffset( PointSet* path_positions, double* path_distances, int orientation, int index, double distance )
  {
    // Check that the given distance is on the given index and find the correct index and distance if not
    while ( distance < 0 && index > 1 )
    {
      index--;
      distance += path_distances[index];
    }

    if ( index <= 1 && distance < 0 ) // We've gone off the start, fail out
    {
      return NULL;
    }

    // Same thing, checking if we go off the end
    while ( index < path_positions->nbPoints && distance > path_distances[index] )
    {
      distance -= path_distances[index];
      index += 1;
    }
    if ( index >= path_positions->nbPoints )
    {
      return NULL;
    }

    // Keep track of the initial index,distance incase we need to re-call get_placement_offset
    int initial_index = index;
    double initial_distance = distance;

    double string_height = mFeature->labelInfo->label_height;
    double old_x = path_positions->x[index-1];
    double old_y = path_positions->y[index-1];

    double new_x = path_positions->x[index];
    double new_y = path_positions->y[index];

    double dx = new_x - old_x;
    double dy = new_y - old_y;

    double segment_length = path_distances[index];
    if ( segment_length == 0 )
    {
      // Not allowed to place across on 0 length segments or discontinuities
      return NULL;
    }

    LabelPosition* slp = NULL;
    LabelPosition* slp_tmp = NULL;
    // current_placement = placement_result()
    double angle = atan2( -dy, dx );

    bool orientation_forced = ( orientation != 0 ); // Whether the orientation was set by the caller
    if ( !orientation_forced )
      orientation = ( angle > 0.55 * M_PI || angle < -0.45 * M_PI ? -1 : 1 );

    int upside_down_char_count = 0; // Count of characters that are placed upside down.

    for ( int i = 0; i < mFeature->labelInfo->char_num; i++ )
    {
      double last_character_angle = angle;

      // grab the next character according to the orientation
      LabelInfo::CharacterInfo& ci = ( orientation > 0 ? mFeature->labelInfo->char_info[i] : mFeature->labelInfo->char_info[mFeature->labelInfo->char_num-i-1] );

      // Coordinates this character will start at
      if ( segment_length == 0 )
      {
        // Not allowed to place across on 0 length segments or discontinuities
        delete slp;
        return NULL;
      }

      double start_x = old_x + dx * distance / segment_length;
      double start_y = old_y + dy * distance / segment_length;
      // Coordinates this character ends at, calculated below
      double end_x = 0;
      double end_y = 0;

      //std::cerr << "segment len " << segment_length << "   distance " << distance << std::endl;
      if ( segment_length - distance  >= ci.width )
      {
        // if the distance remaining in this segment is enough, we just go further along the segment
        distance += ci.width;
        end_x = old_x + dx * distance / segment_length;
        end_y = old_y + dy * distance / segment_length;
      }
      else
      {
        // If there isn't enough distance left on this segment
        // then we need to search until we find the line segment that ends further than ci.width away
        do
        {
          old_x = new_x;
          old_y = new_y;
          index++;
          if ( index >= path_positions->nbPoints ) // Bail out if we run off the end of the shape
          {
            delete slp;
            return NULL;
          }
          new_x = path_positions->x[index];
          new_y = path_positions->y[index];
          dx = new_x - old_x;
          dy = new_y - old_y;
          segment_length = path_distances[index];

          //std::cerr << "-> " << sqrt(pow(start_x - new_x,2) + pow(start_y - new_y,2)) << " vs " << ci.width << std::endl;

        }
        while ( sqrt( pow( start_x - new_x, 2 ) + pow( start_y - new_y, 2 ) ) < ci.width ); // Distance from start_ to new_

        // Calculate the position to place the end of the character on
        findLineCircleIntersection( start_x, start_y, ci.width, old_x, old_y, new_x, new_y, end_x, end_y );

        // Need to calculate distance on the new segment
        distance = sqrt( pow( old_x - end_x, 2 ) + pow( old_y - end_y, 2 ) );
      }

      // Calculate angle from the start of the character to the end based on start_/end_ position
      angle = atan2( start_y - end_y, end_x - start_x );
      //angle = atan2(end_y-start_y, end_x-start_x);

      // Test last_character_angle vs angle
      // since our rendering angle has changed then check against our
      // max allowable angle change.
      double angle_delta = last_character_angle - angle;
      // normalise between -180 and 180
      while ( angle_delta > M_PI ) angle_delta -= 2 * M_PI;
      while ( angle_delta < -M_PI ) angle_delta += 2 * M_PI;
      if (( mFeature->labelInfo->max_char_angle_inside > 0 && angle_delta > 0
            && angle_delta > mFeature->labelInfo->max_char_angle_inside*( M_PI / 180 ) )
          || ( mFeature->labelInfo->max_char_angle_outside < 0 && angle_delta < 0
               && angle_delta < mFeature->labelInfo->max_char_angle_outside*( M_PI / 180 ) ) )
      {
        delete slp;
        return NULL;
      }

      double render_angle = angle;

      double render_x = start_x;
      double render_y = start_y;

      // Center the text on the line
      //render_x -= ((string_height/2.0) - 1.0)*math.cos(render_angle+math.pi/2)
      //render_y += ((string_height/2.0) - 1.0)*math.sin(render_angle+math.pi/2)

      if ( orientation < 0 )
      {
        // rotate in place
        render_x += ci.width * cos( render_angle ); //- (string_height-2)*sin(render_angle);
        render_y -= ci.width * sin( render_angle ); //+ (string_height-2)*cos(render_angle);
        render_angle += M_PI;
      }

      //std::cerr << "adding part: " << render_x << "  " << render_y << std::endl;
      LabelPosition* tmp = new LabelPosition( 0, render_x /*- xBase*/, render_y /*- yBase*/, ci.width, string_height, -render_angle, 0.0001, this );
      tmp->setPartId( orientation > 0 ? i : mFeature->labelInfo->char_num - i - 1 );
      if ( slp == NULL )
        slp = tmp;
      else
        slp_tmp->setNextPart( tmp );
      slp_tmp = tmp;

      //current_placement.add_node(ci.character,render_x, -render_y, render_angle);
      //current_placement.add_node(ci.character,render_x - current_placement.starting_x, render_y - current_placement.starting_y, render_angle)

      // Normalise to 0 <= angle < 2PI
      while ( render_angle >= 2*M_PI ) render_angle -= 2 * M_PI;
      while ( render_angle < 0 ) render_angle += 2 * M_PI;

      if ( render_angle > M_PI / 2 && render_angle < 1.5*M_PI )
        upside_down_char_count++;
    }
    // END FOR

    // If we placed too many characters upside down
    if ( upside_down_char_count >= mFeature->labelInfo->char_num / 2.0 )
    {
      // if we auto-detected the orientation then retry with the opposite orientation
      if ( !orientation_forced )
      {
        orientation = -orientation;
        delete slp;
        slp = curvedPlacementAtOffset( path_positions, path_distances, orientation, initial_index, initial_distance );
      }
      else
      {
        // Otherwise we have failed to find a placement
        delete slp;
        return NULL;
      }
    }

    return slp;
  }

  static LabelPosition* _createCurvedCandidate( LabelPosition* lp, double angle, double dist )
  {
    LabelPosition* newLp = new LabelPosition( *lp );
    newLp->offsetPosition( dist*cos( angle + M_PI / 2 ), dist*sin( angle + M_PI / 2 ) );
    return newLp;
  }

  int FeaturePart::setPositionForLineCurved( LabelPosition ***lPos, PointSet* mapShape )
  {
    // label info must be present
    if ( mFeature->labelInfo == NULL || mFeature->labelInfo->char_num == 0 )
      return 0;

    // distance calculation
    double* path_distances = new double[mapShape->nbPoints];
    double total_distance = 0;
    double old_x = -1.0, old_y = -1.0;
    for ( int i = 0; i < mapShape->nbPoints; i++ )
    {
      if ( i == 0 )
        path_distances[i] = 0;
      else
        path_distances[i] = sqrt( pow( old_x - mapShape->x[i], 2 ) + pow( old_y - mapShape->y[i], 2 ) );
      old_x = mapShape->x[i];
      old_y = mapShape->y[i];

      total_distance += path_distances[i];
    }

    if ( total_distance == 0 )
    {
      delete[] path_distances;
      return 0;
    }

    QLinkedList<LabelPosition*> positions;
    double delta = qMax( mFeature->labelInfo->label_height, total_distance / 10.0 );

    unsigned long flags = mFeature->layer->arrangementFlags();
    if ( flags == 0 )
      flags = FLAG_ON_LINE; // default flag

    // generate curved labels
    for ( int i = 0; i*delta < total_distance; i++ )
    {
      LabelPosition* slp = curvedPlacementAtOffset( mapShape, path_distances, 0, 1, i * delta );

      if ( slp )
      {
        // evaluate cost
        double angle_diff = 0.0, angle_last = 0.0, diff;
        LabelPosition* tmp = slp;
        double sin_avg = 0, cos_avg = 0;
        while ( tmp )
        {
          if ( tmp != slp ) // not first?
          {
            diff = fabs( tmp->getAlpha() - angle_last );
            if ( diff > 2*M_PI ) diff -= 2 * M_PI;
            diff = qMin( diff, 2 * M_PI - diff ); // difference 350 deg is actually just 10 deg...
            angle_diff += diff;
          }

          sin_avg += sin( tmp->getAlpha() );
          cos_avg += cos( tmp->getAlpha() );
          angle_last = tmp->getAlpha();
          tmp = tmp->getNextPart();
        }

        double angle_diff_avg = mFeature->labelInfo->char_num > 1 ? ( angle_diff / ( mFeature->labelInfo->char_num - 1 ) ) : 0; // <0, pi> but pi/8 is much already
        double cost = angle_diff_avg / 100; // <0, 0.031 > but usually <0, 0.003 >
        if ( cost < 0.0001 ) cost = 0.0001;

        // penalize positions which are further from the line's midpoint
        double labelCenter = ( i * delta ) + mFeature->label_x / 2;
        double costCenter = qAbs( total_distance / 2 - labelCenter ) / total_distance; // <0, 0.5>
        cost += costCenter / 1000;  // < 0, 0.0005 >
        //std::cerr << "cost " << angle_diff << " vs " << costCenter << std::endl;
        slp->setCost( cost );


        // average angle is calculated with respect to periodicity of angles
        double angle_avg = atan2( sin_avg / mFeature->labelInfo->char_num, cos_avg / mFeature->labelInfo->char_num );
        // displacement
        if ( flags & FLAG_ABOVE_LINE )
          positions.append( _createCurvedCandidate( slp, angle_avg, mFeature->distlabel ) );
        if ( flags & FLAG_ON_LINE )
          positions.append( _createCurvedCandidate( slp, angle_avg, -mFeature->labelInfo->label_height / 2 ) );
        if ( flags & FLAG_BELOW_LINE )
          positions.append( _createCurvedCandidate( slp, angle_avg, -mFeature->labelInfo->label_height - mFeature->distlabel ) );

        // delete original candidate
        delete slp;
      }
    }


    int nbp = positions.size();
    ( *lPos ) = new LabelPosition*[nbp];
    for ( int i = 0; i < nbp; i++ )
    {
      ( *lPos )[i] = positions.takeFirst();
    }
    delete[] path_distances;

    return nbp;
  }




  /*
   *             seg 2
   *     pt3 ____________pt2
   *        ¦            ¦
   *        ¦            ¦
   * seg 3  ¦    BBOX    ¦ seg 1
   *        ¦            ¦
   *        ¦____________¦
   *     pt0    seg 0    pt1
   *
   */

  int FeaturePart::setPositionForPolygon( LabelPosition ***lPos, PointSet *mapShape )
  {

#ifdef _DEBUG_
    std::cout << "SetPosition (polygon) : " << layer->name << "/" << uid << std::endl;
#endif

    int i;
    int j;

    double labelWidth = mFeature->label_x;
    double labelHeight = mFeature->label_y;

    //print();

    QLinkedList<PointSet*> shapes_toProcess;
    QLinkedList<PointSet*> shapes_final;

    mapShape->parent = NULL;

    shapes_toProcess.append( mapShape );

    splitPolygons( shapes_toProcess, shapes_final, labelWidth, labelHeight, mFeature->uid );

    int nbp;

    if ( shapes_final.size() > 0 )
    {
      QLinkedList<LabelPosition*> positions;

      int id = 0; // ids for candidates
      double dlx, dly; // delta from label center and bottom-left corner
      double alpha = 0.0; // rotation for the label
      double px, py;
      double dx;
      double dy;
      int bbid;
      double beta;
      double diago = sqrt( labelWidth * labelWidth / 4.0 + labelHeight * labelHeight / 4 );
      double rx, ry;
      CHullBox **boxes = new CHullBox*[shapes_final.size()];
      j = 0;

      // Compute bounding box foreach finalShape
      while ( shapes_final.size() > 0 )
      {
        PointSet *shape = shapes_final.takeFirst();
        boxes[j] = shape->compute_chull_bbox();

        if ( shape->parent )
          delete shape;

        j++;
      }

      //dx = dy = min( yrm, xrm ) / 2;
      dx = labelWidth / 2.0;
      dy = labelHeight / 2.0;


      int numTry = 0;

      //fit in polygon only mode slows down calculation a lot, so if it's enabled
      //then use a smaller limit for number of iterations
      int maxTry = mFeature->layer->fitInPolygonOnly() ? 7 : 10;

      do
      {
        for ( bbid = 0; bbid < j; bbid++ )
        {
          CHullBox *box = boxes[bbid];

          if (( box->length * box->width ) > ( xmax - xmin ) *( ymax - ymin ) *5 )
          {
            std::cout << "Very Large BBOX (should never occur : bug-report please)" << std::endl;
            std::cout << "   Box size:  " << box->length << "/" << box->width << std::endl;
            std::cout << "   Alpha:     " << alpha << "   " << alpha * 180 / M_PI << std::endl;
            std::cout << "   Dx;Dy:     " << dx << "   " << dy  << std::endl;
            std::cout << "   LabelSizerm: " << labelWidth << "   " << labelHeight  << std::endl;
            std::cout << "   LabelSizeUn: " << mFeature->label_x << "   " << mFeature->label_y << std::endl;
            continue;
          }

          if ( mFeature->layer->arrangement() == P_HORIZ && mFeature->layer->fitInPolygonOnly() )
          {
            //check width/height of bbox is sufficient for label
            if ( box->length < labelWidth || box->width < labelHeight )
            {
              //no way label can fit in this box, skip it
              continue;
            }
          }

#ifdef _DEBUG_FULL_
          std::cout << "New BBox : " << bbid << std::endl;
          for ( i = 0; i < 4; i++ )
          {
            std::cout << box->x[i] << "\t" << box->y[i] << std::endl;
          }
#endif

          bool enoughPlace = false;
          if ( mFeature->layer->arrangement() == P_FREE )
          {
            enoughPlace = true;
            px = ( box->x[0] + box->x[2] ) / 2 - labelWidth;
            py = ( box->y[0] + box->y[2] ) / 2 - labelHeight;
            int i, j;

            // Virtual label: center on bbox center, label size = 2x original size
            // alpha = 0.
            // If all corner are in bbox then place candidates horizontaly
            for ( rx = px, i = 0; i < 2; rx = rx + 2 * labelWidth, i++ )
            {
              for ( ry = py, j = 0; j < 2; ry = ry + 2 * labelHeight, j++ )
              {
                if ( !mapShape->containsPoint( rx, ry ) )
                {
                  enoughPlace = false;
                  break;
                }
              }
              if ( !enoughPlace )
              {
                break;
              }
            }

          } // arrangement== FREE ?

          if ( mFeature->layer->arrangement() == P_HORIZ || enoughPlace )
          {
            alpha = 0.0; // HORIZ
          }
          else if ( box->length > 1.5*labelWidth && box->width > 1.5*labelWidth )
          {
            if ( box->alpha <= M_PI / 4 )
            {
              alpha = box->alpha;
            }
            else
            {
              alpha = box->alpha - M_PI / 2;
            }
          }
          else if ( box->length > box->width )
          {
            alpha = box->alpha - M_PI / 2;
          }
          else
          {
            alpha = box->alpha;
          }

          beta  = atan2( labelHeight, labelWidth ) + alpha;


          //alpha = box->alpha;

          // delta from label center and down-left corner
          dlx = cos( beta ) * diago;
          dly = sin( beta ) * diago;

          double px0, py0;

          px0 = box->width / 2.0;
          py0 = box->length / 2.0;

          px0 -= ceil( px0 / dx ) * dx;
          py0 -= ceil( py0 / dy ) * dy;

          for ( px = px0; px <= box->width; px += dx )
          {
            for ( py = py0; py <= box->length; py += dy )
            {

              rx = cos( box->alpha ) * px + cos( box->alpha - M_PI / 2 ) * py;
              ry = sin( box->alpha ) * px + sin( box->alpha - M_PI / 2 ) * py;

              rx += box->x[0];
              ry += box->y[0];

              bool candidateAcceptable = ( mFeature->layer->fitInPolygonOnly()
                                           ? mapShape->containsLabelCandidate( rx - dlx, ry - dly, labelWidth, labelHeight, alpha )
                                           : mapShape->containsPoint( rx, ry ) );
              if ( candidateAcceptable )
              {
                // cost is set to minimal value, evaluated later
                positions.append( new LabelPosition( id++, rx - dlx, ry - dly, labelWidth, labelHeight, alpha, 0.0001, this ) ); // Polygon
              }
            }
          }
        } // forall box

        nbp = positions.size();
        if ( nbp == 0 )
        {
          dx /= 2;
          dy /= 2;
          numTry++;
        }
      }
      while ( nbp == 0 && numTry < maxTry );

      nbp = positions.size();

      ( *lPos ) = new LabelPosition*[nbp];
      for ( i = 0; i < nbp; i++ )
      {
        ( *lPos )[i] = positions.takeFirst();
      }

      for ( bbid = 0; bbid < j; bbid++ )
      {
        delete boxes[bbid];
      }

      delete[] boxes;
    }
    else
    {
      nbp = 0;
    }

#ifdef _DEBUG_FULL_
    std::cout << "NbLabelPosition: " << nbp << std::endl;
#endif
    return nbp;
  }

#if 0
  void FeaturePart::print()
  {
    int i, j;
    std::cout << "Geometry id : " << f->uid << std::endl;
    std::cout << "Type: " << type << std::endl;
    if ( x && y )
    {
      for ( i = 0; i < nbPoints; i++ )
        std::cout << x[i] << ", " << y[i] << std::endl;
      std::cout << "Obstacle: " << nbHoles << std::endl;
      for ( i = 0; i < nbHoles; i++ )
      {
        std::cout << "  obs " << i << std::endl;
        for ( j = 0; j < holes[i]->nbPoints; j++ )
        {
          std::cout << holes[i]->x[j] << ";" << holes[i]->y[j] << std::endl;
        }
      }
    }

    std::cout << std::endl;
  }
#endif

  int FeaturePart::setPosition( LabelPosition ***lPos,
                                double bbox_min[2], double bbox_max[2],
                                PointSet *mapShape, RTree<LabelPosition*, double, 2, double> *candidates )
  {
    int nbp = 0;
    int i;
    double bbox[4];

    bbox[0] = bbox_min[0];
    bbox[1] = bbox_min[1];
    bbox[2] = bbox_max[0];
    bbox[3] = bbox_max[1];

    double angle = mFeature->fixedRotation ? mFeature->fixedAngle : 0.0;

    if ( mFeature->fixedPosition() )
    {
      nbp = 1;
      *lPos = new LabelPosition *[nbp];
      ( *lPos )[0] = new LabelPosition( 0, mFeature->fixedPosX, mFeature->fixedPosY, mFeature->label_x, mFeature->label_y, angle, 0.0, this );
    }
    else
    {
      switch ( type )
      {
        case GEOS_POINT:
          if ( mFeature->layer->arrangement() == P_POINT_OVER || mFeature->fixedQuadrant() )
            nbp = setPositionOverPoint( x[0], y[0], lPos, angle );
          else
            nbp = setPositionForPoint( x[0], y[0], lPos, angle );
          break;
        case GEOS_LINESTRING:
          if ( mFeature->layer->arrangement() == P_CURVED )
            nbp = setPositionForLineCurved( lPos, mapShape );
          else
            nbp = setPositionForLine( lPos, mapShape );
          break;

        case GEOS_POLYGON:
          switch ( mFeature->layer->arrangement() )
          {
            case P_POINT:
            case P_POINT_OVER:
              double cx, cy;
              mapShape->getCentroid( cx, cy, mFeature->layer->centroidInside() );
              if ( mFeature->layer->arrangement() == P_POINT_OVER )
                nbp = setPositionOverPoint( cx, cy, lPos, angle, mapShape );
              else
                nbp = setPositionForPoint( cx, cy, lPos, angle, mapShape );
              break;
            case P_LINE:
              nbp = setPositionForLine( lPos, mapShape );
              break;
            default:
              nbp = setPositionForPolygon( lPos, mapShape );
              break;
          }
      }
    }

    int rnbp = nbp;

    // purge candidates that are outside the bbox
    for ( i = 0; i < nbp; i++ )
    {
      bool outside = false;
      if ( mFeature->layer->pal->getShowPartial() )
        outside = !( *lPos )[i]->isIntersect( bbox );
      else
        outside = !( *lPos )[i]->isInside( bbox );
      if ( outside )
      {
        rnbp--;
        ( *lPos )[i]->setCost( DBL_MAX ); // infinite cost => do not use
      }
      else   // this one is OK
      {
        ( *lPos )[i]->insertIntoIndex( candidates );
      }
    }

    sort(( void** )( *lPos ), nbp, LabelPosition::costGrow );

    for ( i = rnbp; i < nbp; i++ )
    {
      delete( *lPos )[i];
    }

    return rnbp;
  }

  void FeaturePart::addSizePenalty( int nbp, LabelPosition** lPos, double bbx[4], double bby[4] )
  {
    if ( !mGeos )
      createGeosGeom();

    GEOSContextHandle_t ctxt = geosContext();
    int geomType = GEOSGeomTypeId_r( ctxt, mGeos );

    double sizeCost = 0;
    if ( geomType == GEOS_LINESTRING )
    {
      double length;
      if ( GEOSLength_r( ctxt, mGeos, &length ) != 1 )
        return; // failed to calculate length
      double bbox_length = qMax( bbx[2] - bbx[0], bby[2] - bby[0] );
      if ( length >= bbox_length / 4 )
        return; // the line is longer than quarter of height or width - don't penalize it

      sizeCost = 1 - ( length / ( bbox_length / 4 ) ); // < 0,1 >
    }
    else if ( geomType == GEOS_POLYGON )
    {
      double area;
      if ( GEOSArea_r( ctxt, mGeos, &area ) != 1 )
        return;
      double bbox_area = ( bbx[2] - bbx[0] ) * ( bby[2] - bby[0] );
      if ( area >= bbox_area / 16 )
        return; // covers more than 1/16 of our view - don't penalize it

      sizeCost = 1 - ( area / ( bbox_area / 16 ) ); // < 0, 1 >
    }
    else
      return; // no size penalty for points

    //std::cout << "size cost " << sizeCost << std::endl;

    // apply the penalty
    for ( int i = 0; i < nbp; i++ )
    {
      lPos[i]->setCost( lPos[i]->cost() + sizeCost / 100 );
    }
  }

  bool FeaturePart::isConnected( FeaturePart* p2 )
  {
    if ( !p2->mGeos )
      p2->createGeosGeom();

    return ( GEOSPreparedTouches_r( geosContext(), preparedGeom(), p2->mGeos ) == 1 );
  }

  bool FeaturePart::mergeWithFeaturePart( FeaturePart* other )
  {
    if ( !mGeos )
      createGeosGeom();
    if ( !other->mGeos )
      other->createGeosGeom();

    GEOSContextHandle_t ctxt = geosContext();
    GEOSGeometry* g1 = GEOSGeom_clone_r( ctxt, mGeos );
    GEOSGeometry* g2 = GEOSGeom_clone_r( ctxt, other->mGeos );
    GEOSGeometry* geoms[2] = { g1, g2 };
    GEOSGeometry* g = GEOSGeom_createCollection_r( ctxt, GEOS_MULTILINESTRING, geoms, 2 );
    GEOSGeometry* gTmp = GEOSLineMerge_r( ctxt, g );
    GEOSGeom_destroy_r( ctxt, g );

    if ( GEOSGeomTypeId_r( ctxt, gTmp ) != GEOS_LINESTRING )
    {
      // sometimes it's not possible to merge lines (e.g. they don't touch at endpoints)
      GEOSGeom_destroy_r( ctxt, gTmp );
      return false;
    }

    invalidateGeos();

    // set up new geometry
    mGeos = gTmp;
    mOwnsGeom = true;

    deleteCoords();
    qDeleteAll( mHoles );
    mHoles.clear();
    extractCoords( mGeos );
    return true;
  }

} // end namespace pal
