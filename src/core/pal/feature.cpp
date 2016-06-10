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

#include "qgsgeometry.h"
#include "pal.h"
#include "layer.h"
#include "feature.h"
#include "geomfunction.h"
#include "labelposition.h"
#include "pointset.h"
#include "util.h"
#include "qgis.h"
#include "qgsgeos.h"
#include "qgsmessagelog.h"
#include "costcalculator.h"
#include <QLinkedList>
#include <cmath>
#include <cfloat>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace pal;

FeaturePart::FeaturePart( QgsLabelFeature* feat, const GEOSGeometry* geom )
    : mLF( feat )
{
  // we'll remove const, but we won't modify that geometry
  mGeos = const_cast<GEOSGeometry*>( geom );
  mOwnsGeom = false; // geometry is owned by Feature class

  extractCoords( geom );

  holeOf = nullptr;
  for ( int i = 0; i < mHoles.count(); i++ )
  {
    mHoles.at( i )->holeOf = this;
  }

}

FeaturePart::FeaturePart( const FeaturePart& other )
    : PointSet( other )
    , mLF( other.mLF )
{
  Q_FOREACH ( const FeaturePart* hole, other.mHoles )
  {
    mHoles << new FeaturePart( *hole );
    mHoles.last()->holeOf = this;
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
        FeaturePart* hole = new FeaturePart( mLF, interior );
        hole->holeOf = nullptr;
        // possibly not needed. it's not done for the exterior ring, so I'm not sure
        // why it's just done here...
        GeomFunction::reorderPolygon( hole->nbPoints, hole->x, hole->y );

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
  deleteCoords();
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
  return mLF->layer();
}

QgsFeatureId FeaturePart::featureId() const
{
  return mLF->id();
}

bool FeaturePart::hasSameLabelFeatureAs( FeaturePart* part ) const
{
  if ( !part )
    return false;

  if ( mLF->layer()->name() != part->layer()->name() )
    return false;

  if ( mLF->id() == part->featureId() )
    return true;

  // any part of joined features are also treated as having the same label feature
  int connectedFeatureId = mLF->layer()->connectedFeatureId( mLF->id() );
  if ( connectedFeatureId >= 0 && connectedFeatureId == mLF->layer()->connectedFeatureId( part->featureId() ) )
    return true;

  return false;
}

LabelPosition::Quadrant FeaturePart::quadrantFromOffset() const
{
  QPointF quadOffset = mLF->quadOffset();
  qreal quadOffsetX = quadOffset.x(), quadOffsetY = quadOffset.y();

  if ( quadOffsetX < 0 )
  {
    if ( quadOffsetY < 0 )
    {
      return LabelPosition::QuadrantAboveLeft;
    }
    else if ( quadOffsetY > 0 )
    {
      return LabelPosition::QuadrantBelowLeft;
    }
    else
    {
      return LabelPosition::QuadrantLeft;
    }
  }
  else  if ( quadOffsetX > 0 )
  {
    if ( quadOffsetY < 0 )
    {
      return LabelPosition::QuadrantAboveRight;
    }
    else if ( quadOffsetY > 0 )
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
    if ( quadOffsetY < 0 )
    {
      return LabelPosition::QuadrantAbove;
    }
    else if ( quadOffsetY > 0 )
    {
      return LabelPosition::QuadrantBelow;
    }
    else
    {
      return LabelPosition::QuadrantOver;
    }
  }
}

int FeaturePart::createCandidatesOverPoint( double x, double y, QList< LabelPosition*>& lPos, double angle, PointSet *mapShape )
{
  int nbp = 1;

  // get from feature
  double labelW = getLabelWidth();
  double labelH = getLabelHeight();

  double cost = 0.0001;
  int id = 0;

  double xdiff = -labelW / 2.0;
  double ydiff = -labelH / 2.0;

  if ( !qgsDoubleNear( mLF->quadOffset().x(), 0.0 ) )
  {
    xdiff += labelW / 2.0 * mLF->quadOffset().x();
  }
  if ( !qgsDoubleNear( mLF->quadOffset().y(), 0.0 ) )
  {
    ydiff += labelH / 2.0 * mLF->quadOffset().y();
  }

  if ( ! mLF->hasFixedPosition() )
  {
    if ( !qgsDoubleNear( angle, 0.0 ) )
    {
      double xd = xdiff * cos( angle ) - ydiff * sin( angle );
      double yd = xdiff * sin( angle ) + ydiff * cos( angle );
      xdiff = xd;
      ydiff = yd;
    }
  }

  if ( mLF->layer()->arrangement() == QgsPalLayerSettings::AroundPoint )
  {
    //if in "around point" placement mode, then we use the label distance to determine
    //the label's offset
    if ( qgsDoubleNear( mLF->quadOffset().x(), 0.0 ) )
    {
      ydiff += mLF->quadOffset().y() * mLF->distLabel();
    }
    else if ( qgsDoubleNear( mLF->quadOffset().y(), 0.0 ) )
    {
      xdiff += mLF->quadOffset().x() * mLF->distLabel();
    }
    else
    {
      xdiff += mLF->quadOffset().x() * M_SQRT1_2 * mLF->distLabel();
      ydiff += mLF->quadOffset().y() * M_SQRT1_2 * mLF->distLabel();
    }
  }
  else
  {
    if ( !qgsDoubleNear( mLF->positionOffset().x(), 0.0 ) )
    {
      xdiff += mLF->positionOffset().x();
    }
    if ( !qgsDoubleNear( mLF->positionOffset().y(), 0.0 ) )
    {
      ydiff += mLF->positionOffset().y();
    }
  }

  double lx = x + xdiff;
  double ly = y + ydiff;

  if ( mapShape && type == GEOS_POLYGON && mLF->layer()->fitInPolygonOnly() )
  {
    if ( !mapShape->containsLabelCandidate( lx, ly, labelW, labelH, angle ) )
    {
      return 0;
    }
  }

  lPos << new LabelPosition( id, lx, ly, labelW, labelH, angle, cost, this, false, quadrantFromOffset() );
  return nbp;
}

int FeaturePart::createCandidatesAtOrderedPositionsOverPoint( double x, double y, QList<LabelPosition*>& lPos, double angle )
{
  QVector< QgsPalLayerSettings::PredefinedPointPosition > positions = mLF->predefinedPositionOrder();
  double labelWidth = getLabelWidth();
  double labelHeight = getLabelHeight();
  double distanceToLabel = getLabelDistance();
  const QgsLabelFeature::VisualMargin& visualMargin = mLF->visualMargin();

  double symbolWidthOffset = ( mLF->offsetType() == QgsPalLayerSettings::FromSymbolBounds ? mLF->symbolSize().width() / 2.0 : 0.0 );
  double symbolHeightOffset = ( mLF->offsetType() == QgsPalLayerSettings::FromSymbolBounds ? mLF->symbolSize().height() / 2.0 : 0.0 );

  double cost = 0.0001;
  int i = 0;
  Q_FOREACH ( QgsPalLayerSettings::PredefinedPointPosition position, positions )
  {
    double alpha = 0.0;
    double deltaX = 0;
    double deltaY = 0;
    LabelPosition::Quadrant quadrant = LabelPosition::QuadrantAboveLeft;
    switch ( position )
    {
      case QgsPalLayerSettings::TopLeft:
        quadrant = LabelPosition::QuadrantAboveLeft;
        alpha = 3 * M_PI_4;
        deltaX = -labelWidth + visualMargin.right - symbolWidthOffset;
        deltaY = -visualMargin.bottom + symbolHeightOffset;
        break;

      case QgsPalLayerSettings::TopSlightlyLeft:
        quadrant = LabelPosition::QuadrantAboveRight; //right quadrant, so labels are left-aligned
        alpha = M_PI_2;
        deltaX = -labelWidth / 4.0 - visualMargin.left;
        deltaY = -visualMargin.bottom + symbolHeightOffset;
        break;

      case QgsPalLayerSettings::TopMiddle:
        quadrant = LabelPosition::QuadrantAbove;
        alpha = M_PI_2;
        deltaX = -labelWidth / 2.0;
        deltaY = -visualMargin.bottom + symbolHeightOffset;
        break;

      case QgsPalLayerSettings::TopSlightlyRight:
        quadrant = LabelPosition::QuadrantAboveLeft; //left quadrant, so labels are right-aligned
        alpha = M_PI_2;
        deltaX = -labelWidth * 3.0 / 4.0 + visualMargin.right;
        deltaY = -visualMargin.bottom + symbolHeightOffset;
        break;

      case QgsPalLayerSettings::TopRight:
        quadrant = LabelPosition::QuadrantAboveRight;
        alpha = M_PI_4;
        deltaX = - visualMargin.left + symbolWidthOffset;
        deltaY = -visualMargin.bottom + symbolHeightOffset;
        break;

      case QgsPalLayerSettings::MiddleLeft:
        quadrant = LabelPosition::QuadrantLeft;
        alpha = M_PI;
        deltaX = -labelWidth + visualMargin.right - symbolWidthOffset;
        deltaY = -labelHeight / 2.0;// TODO - should this be adjusted by visual margin??
        break;

      case QgsPalLayerSettings::MiddleRight:
        quadrant = LabelPosition::QuadrantRight;
        alpha = 0.0;
        deltaX = -visualMargin.left + symbolWidthOffset;
        deltaY = -labelHeight / 2.0;// TODO - should this be adjusted by visual margin??
        break;

      case QgsPalLayerSettings::BottomLeft:
        quadrant = LabelPosition::QuadrantBelowLeft;
        alpha = 5 * M_PI_4;
        deltaX = -labelWidth + visualMargin.right - symbolWidthOffset;
        deltaY = -labelHeight + visualMargin.top - symbolHeightOffset;
        break;

      case QgsPalLayerSettings::BottomSlightlyLeft:
        quadrant = LabelPosition::QuadrantBelowRight; //right quadrant, so labels are left-aligned
        alpha = 3 * M_PI_2;
        deltaX = -labelWidth / 4.0 - visualMargin.left;
        deltaY = -labelHeight + visualMargin.top - symbolHeightOffset;
        break;

      case QgsPalLayerSettings::BottomMiddle:
        quadrant = LabelPosition::QuadrantBelow;
        alpha = 3 * M_PI_2;
        deltaX = -labelWidth / 2.0;
        deltaY = -labelHeight + visualMargin.top - symbolHeightOffset;
        break;

      case QgsPalLayerSettings::BottomSlightlyRight:
        quadrant = LabelPosition::QuadrantBelowLeft; //left quadrant, so labels are right-aligned
        alpha = 3 * M_PI_2;
        deltaX = -labelWidth * 3.0 / 4.0 + visualMargin.right;
        deltaY = -labelHeight + visualMargin.top - symbolHeightOffset;
        break;

      case QgsPalLayerSettings::BottomRight:
        quadrant = LabelPosition::QuadrantBelowRight;
        alpha = 7 * M_PI_4;
        deltaX = -visualMargin.left + symbolWidthOffset;
        deltaY = -labelHeight + visualMargin.top - symbolHeightOffset;
        break;
    }

    //have bearing, distance - calculate reference point
    double referenceX = cos( alpha ) * distanceToLabel + x;
    double referenceY = sin( alpha ) * distanceToLabel + y;

    double labelX = referenceX + deltaX;
    double labelY = referenceY + deltaY;

    lPos << new LabelPosition( i, labelX, labelY, labelWidth, labelHeight, angle, cost, this, false, quadrant );

    //TODO - tweak
    cost += 0.001;

    ++i;
  }

  return lPos.count();
}

int FeaturePart::createCandidatesAroundPoint( double x, double y, QList< LabelPosition* >& lPos, double angle, PointSet *mapShape )
{
  double labelWidth = getLabelWidth();
  double labelHeight = getLabelHeight();
  double distanceToLabel = getLabelDistance();

  int numberCandidates = mLF->layer()->pal->point_p;

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


    if ( mapShape && type == GEOS_POLYGON && mLF->layer()->fitInPolygonOnly() )
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
    for ( int i = 0; i < candidates.count(); ++i )
    {
      lPos << candidates.at( i );
    }
  }

  return candidates.count();
}

// TODO work with squared distance by removing call to sqrt or dist_euc2d
int FeaturePart::createCandidatesAlongLine( QList< LabelPosition* >& lPos, PointSet *mapShape )
{
  int i;
  double distlabel = getLabelDistance();

  double xrm = getLabelWidth();
  double yrm = getLabelHeight();

  double *d; // segments lengths distance bw pt[i] && pt[i+1]
  double *ad;  // absolute distance bw pt[0] and pt[i] along the line
  double ll; // line length
  double dist;
  double bx, by, ex, ey;
  int nbls;
  double alpha;
  double cost;

  LineArrangementFlags flags = mLF->layer()->arrangementFlags();
  if ( flags == 0 )
    flags = FLAG_ON_LINE; // default flag

  QLinkedList<LabelPosition*> positions;

  int nbPoints;
  double *x;
  double *y;

  PointSet * line = mapShape;
  nbPoints = line->nbPoints;
  x = line->x;
  y = line->y;

  d = new double[nbPoints-1];
  ad = new double[nbPoints];

  ll = 0.0; // line length
  for ( i = 0; i < line->nbPoints - 1; i++ )
  {
    if ( i == 0 )
      ad[i] = 0;
    else
      ad[i] = ad[i-1] + d[i-1];

    d[i] = GeomFunction::dist_euc2d( x[i], y[i], x[i+1], y[i+1] );
    ll += d[i];
  }

  ad[line->nbPoints-1] = ll;

  nbls = static_cast< int >( ll / xrm ); // ratio bw line length and label width
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
  while ( l < ll - xrm )
  {
    // => bx, by
    line->getPointByDistance( d, ad, l, &bx, &by );
    // same but l = l+xrm
    line->getPointByDistance( d, ad, l + xrm, &ex, &ey );

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
      alpha = 0.0;
    }
    else
      alpha = atan2( ey - by, ex - bx );

    beta = alpha + M_PI / 2;

    if ( mLF->layer()->arrangement() == QgsPalLayerSettings::Line )
    {
      // find out whether the line direction for this candidate is from right to left
      bool isRightToLeft = ( alpha > M_PI / 2 || alpha <= -M_PI / 2 );
      // meaning of above/below may be reversed if using line position dependent orientation
      // and the line has right-to-left direction
      bool reversed = (( flags & FLAG_MAP_ORIENTATION ) ? isRightToLeft : false );
      bool aboveLine = ( !reversed && ( flags & FLAG_ABOVE_LINE ) ) || ( reversed && ( flags & FLAG_BELOW_LINE ) );
      bool belowLine = ( !reversed && ( flags & FLAG_BELOW_LINE ) ) || ( reversed && ( flags & FLAG_ABOVE_LINE ) );

      if ( aboveLine )
      {
        if ( !mLF->layer()->fitInPolygonOnly() || mapShape->containsLabelCandidate( bx + cos( beta ) *distlabel, by + sin( beta ) *distlabel, xrm, yrm, alpha ) )
          positions.append( new LabelPosition( i, bx + cos( beta ) *distlabel, by + sin( beta ) *distlabel, xrm, yrm, alpha, cost, this, isRightToLeft ) ); // Line
      }
      if ( belowLine )
      {
        if ( !mLF->layer()->fitInPolygonOnly() || mapShape->containsLabelCandidate( bx - cos( beta ) *( distlabel + yrm ), by - sin( beta ) *( distlabel + yrm ), xrm, yrm, alpha ) )
          positions.append( new LabelPosition( i, bx - cos( beta ) *( distlabel + yrm ), by - sin( beta ) *( distlabel + yrm ), xrm, yrm, alpha, cost, this, isRightToLeft ) );   // Line
      }
      if ( flags & FLAG_ON_LINE )
      {
        if ( !mLF->layer()->fitInPolygonOnly() || mapShape->containsLabelCandidate( bx - yrm*cos( beta ) / 2, by - yrm*sin( beta ) / 2, xrm, yrm, alpha ) )
          positions.append( new LabelPosition( i, bx - yrm*cos( beta ) / 2, by - yrm*sin( beta ) / 2, xrm, yrm, alpha, cost, this, isRightToLeft ) ); // Line
      }
    }
    else if ( mLF->layer()->arrangement() == QgsPalLayerSettings::Horizontal )
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

  //delete line;

  delete[] d;
  delete[] ad;

  int nbp = positions.size();
  while ( !positions.isEmpty() )
  {
    lPos << positions.takeFirst();
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
    return nullptr;
  }

  // Same thing, checking if we go off the end
  while ( index < path_positions->nbPoints && distance > path_distances[index] )
  {
    distance -= path_distances[index];
    index += 1;
  }
  if ( index >= path_positions->nbPoints )
  {
    return nullptr;
  }

  LabelInfo* li = mLF->curvedLabelInfo();

  // Keep track of the initial index,distance incase we need to re-call get_placement_offset
  int initial_index = index;
  double initial_distance = distance;

  double string_height = li->label_height;
  double old_x = path_positions->x[index-1];
  double old_y = path_positions->y[index-1];

  double new_x = path_positions->x[index];
  double new_y = path_positions->y[index];

  double dx = new_x - old_x;
  double dy = new_y - old_y;

  double segment_length = path_distances[index];
  if ( qgsDoubleNear( segment_length, 0.0 ) )
  {
    // Not allowed to place across on 0 length segments or discontinuities
    return nullptr;
  }

  LabelPosition* slp = nullptr;
  LabelPosition* slp_tmp = nullptr;
  // current_placement = placement_result()
  double angle = atan2( -dy, dx );

  bool orientation_forced = ( orientation != 0 ); // Whether the orientation was set by the caller
  if ( !orientation_forced )
    orientation = ( angle > 0.55 * M_PI || angle < -0.45 * M_PI ? -1 : 1 );

  int upside_down_char_count = 0; // Count of characters that are placed upside down.

  for ( int i = 0; i < li->char_num; i++ )
  {
    double last_character_angle = angle;

    // grab the next character according to the orientation
    LabelInfo::CharacterInfo& ci = ( orientation > 0 ? li->char_info[i] : li->char_info[li->char_num-i-1] );

    // Coordinates this character will start at
    if ( qgsDoubleNear( segment_length, 0.0 ) )
    {
      // Not allowed to place across on 0 length segments or discontinuities
      delete slp;
      return nullptr;
    }

    double start_x = old_x + dx * distance / segment_length;
    double start_y = old_y + dy * distance / segment_length;
    // Coordinates this character ends at, calculated below
    double end_x = 0;
    double end_y = 0;

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
          return nullptr;
        }
        new_x = path_positions->x[index];
        new_y = path_positions->y[index];
        dx = new_x - old_x;
        dy = new_y - old_y;
        segment_length = path_distances[index];
      }
      while ( sqrt( pow( start_x - new_x, 2 ) + pow( start_y - new_y, 2 ) ) < ci.width ); // Distance from start_ to new_

      // Calculate the position to place the end of the character on
      GeomFunction::findLineCircleIntersection( start_x, start_y, ci.width, old_x, old_y, new_x, new_y, end_x, end_y );

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
    if (( li->max_char_angle_inside > 0 && angle_delta > 0
          && angle_delta > li->max_char_angle_inside*( M_PI / 180 ) )
        || ( li->max_char_angle_outside < 0 && angle_delta < 0
             && angle_delta < li->max_char_angle_outside*( M_PI / 180 ) ) )
    {
      delete slp;
      return nullptr;
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

    LabelPosition* tmp = new LabelPosition( 0, render_x /*- xBase*/, render_y /*- yBase*/, ci.width, string_height, -render_angle, 0.0001, this );
    tmp->setPartId( orientation > 0 ? i : li->char_num - i - 1 );
    if ( !slp )
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
  if ( upside_down_char_count >= li->char_num / 2.0 )
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
      return nullptr;
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

int FeaturePart::createCurvedCandidatesAlongLine( QList< LabelPosition* >& lPos, PointSet* mapShape )
{
  LabelInfo* li = mLF->curvedLabelInfo();

  // label info must be present
  if ( !li || li->char_num == 0 )
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

  if ( qgsDoubleNear( total_distance, 0.0 ) )
  {
    delete[] path_distances;
    return 0;
  }

  //calculate overall angle of line
  double lineAngle;
  double bx = mapShape->x[0];
  double by = mapShape->y[0];
  double ex = mapShape->x[ mapShape->nbPoints - 1 ];
  double ey = mapShape->y[ mapShape->nbPoints - 1 ];
  if ( qgsDoubleNear( ey, by ) && qgsDoubleNear( ex, bx ) )
  {
    lineAngle = 0.0;
  }
  else
    lineAngle = atan2( ey - by, ex - bx );

  // find out whether the line direction for this candidate is from right to left
  bool isRightToLeft = ( lineAngle > M_PI / 2 || lineAngle <= -M_PI / 2 );

  QLinkedList<LabelPosition*> positions;
  double delta = qMax( li->label_height, total_distance / 10.0 );

  unsigned long flags = mLF->layer()->arrangementFlags();
  if ( flags == 0 )
    flags = FLAG_ON_LINE; // default flag
  // placements may need to be reversed if using line position dependent orientation
  // and the line has right-to-left direction
  bool reversed = ( !( flags & FLAG_MAP_ORIENTATION ) ? isRightToLeft : false );

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

      double angle_diff_avg = li->char_num > 1 ? ( angle_diff / ( li->char_num - 1 ) ) : 0; // <0, pi> but pi/8 is much already
      double cost = angle_diff_avg / 100; // <0, 0.031 > but usually <0, 0.003 >
      if ( cost < 0.0001 ) cost = 0.0001;

      // penalize positions which are further from the line's midpoint
      double labelCenter = ( i * delta ) + getLabelWidth() / 2;
      double costCenter = qAbs( total_distance / 2 - labelCenter ) / total_distance; // <0, 0.5>
      cost += costCenter / 1000;  // < 0, 0.0005 >
      slp->setCost( cost );

      // average angle is calculated with respect to periodicity of angles
      double angle_avg = atan2( sin_avg / li->char_num, cos_avg / li->char_num );
      // displacement
      if (( !reversed && ( flags & FLAG_ABOVE_LINE ) ) || ( reversed && ( flags & FLAG_BELOW_LINE ) ) )
        positions.append( _createCurvedCandidate( slp, angle_avg, mLF->distLabel() ) );
      if ( flags & FLAG_ON_LINE )
        positions.append( _createCurvedCandidate( slp, angle_avg, -li->label_height / 2 ) );
      if (( !reversed && ( flags & FLAG_BELOW_LINE ) ) || ( reversed && ( flags & FLAG_ABOVE_LINE ) ) )
        positions.append( _createCurvedCandidate( slp, angle_avg, -li->label_height - mLF->distLabel() ) );

      // delete original candidate
      delete slp;
    }
  }


  int nbp = positions.size();
  for ( int i = 0; i < nbp; i++ )
  {
    lPos << positions.takeFirst();
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

int FeaturePart::createCandidatesForPolygon( QList< LabelPosition*>& lPos, PointSet *mapShape )
{
  int i;
  int j;

  double labelWidth = getLabelWidth();
  double labelHeight = getLabelHeight();

  QLinkedList<PointSet*> shapes_toProcess;
  QLinkedList<PointSet*> shapes_final;

  mapShape->parent = nullptr;

  shapes_toProcess.append( mapShape );

  splitPolygons( shapes_toProcess, shapes_final, labelWidth, labelHeight );

  int nbp;

  if ( !shapes_final.isEmpty() )
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
    while ( !shapes_final.isEmpty() )
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
    int maxTry = mLF->layer()->fitInPolygonOnly() ? 7 : 10;

    do
    {
      for ( bbid = 0; bbid < j; bbid++ )
      {
        CHullBox *box = boxes[bbid];

        if (( box->length * box->width ) > ( xmax - xmin ) *( ymax - ymin ) *5 )
        {
          // Very Large BBOX (should never occur)
          continue;
        }

        if ( mLF->layer()->arrangement() == QgsPalLayerSettings::Horizontal && mLF->layer()->fitInPolygonOnly() )
        {
          //check width/height of bbox is sufficient for label
          if ( box->length < labelWidth || box->width < labelHeight )
          {
            //no way label can fit in this box, skip it
            continue;
          }
        }

        bool enoughPlace = false;
        if ( mLF->layer()->arrangement() == QgsPalLayerSettings::Free )
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

        if ( mLF->layer()->arrangement() == QgsPalLayerSettings::Horizontal || enoughPlace )
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

            bool candidateAcceptable = ( mLF->layer()->fitInPolygonOnly()
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

    for ( i = 0; i < nbp; i++ )
    {
      lPos << positions.takeFirst();
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

  return nbp;
}

int FeaturePart::createCandidates( QList< LabelPosition*>& lPos,
                                   double bboxMin[2], double bboxMax[2],
                                   PointSet *mapShape, RTree<LabelPosition*, double, 2, double>* candidates )
{
  double bbox[4];

  bbox[0] = bboxMin[0];
  bbox[1] = bboxMin[1];
  bbox[2] = bboxMax[0];
  bbox[3] = bboxMax[1];

  double angle = mLF->hasFixedAngle() ? mLF->fixedAngle() : 0.0;

  if ( mLF->hasFixedPosition() )
  {
    lPos << new LabelPosition( 0, mLF->fixedPosition().x(), mLF->fixedPosition().y(), getLabelWidth(), getLabelHeight(), angle, 0.0, this );
  }
  else
  {
    switch ( type )
    {
      case GEOS_POINT:
        if ( mLF->layer()->arrangement() == QgsPalLayerSettings::OrderedPositionsAroundPoint )
          createCandidatesAtOrderedPositionsOverPoint( x[0], y[0], lPos, angle );
        else if ( mLF->layer()->arrangement() == QgsPalLayerSettings::OverPoint || mLF->hasFixedQuadrant() )
          createCandidatesOverPoint( x[0], y[0], lPos, angle );
        else
          createCandidatesAroundPoint( x[0], y[0], lPos, angle );
        break;
      case GEOS_LINESTRING:
        if ( mLF->layer()->arrangement() == QgsPalLayerSettings::Curved )
          createCurvedCandidatesAlongLine( lPos, mapShape );
        else
          createCandidatesAlongLine( lPos, mapShape );
        break;

      case GEOS_POLYGON:
        switch ( mLF->layer()->arrangement() )
        {
          case QgsPalLayerSettings::AroundPoint:
          case QgsPalLayerSettings::OverPoint:
            double cx, cy;
            mapShape->getCentroid( cx, cy, mLF->layer()->centroidInside() );
            if ( mLF->layer()->arrangement() == QgsPalLayerSettings::OverPoint )
              createCandidatesOverPoint( cx, cy, lPos, angle, mapShape );
            else
              createCandidatesAroundPoint( cx, cy, lPos, angle, mapShape );
            break;
          case QgsPalLayerSettings::Line:
            createCandidatesAlongLine( lPos, mapShape );
            break;
          default:
            createCandidatesForPolygon( lPos, mapShape );
            break;
        }
    }
  }

  // purge candidates that are outside the bbox

  QMutableListIterator< LabelPosition*> i( lPos );
  while ( i.hasNext() )
  {
    LabelPosition* pos = i.next();
    bool outside = false;
    if ( mLF->layer()->pal->getShowPartial() )
      outside = !pos->isIntersect( bbox );
    else
      outside = !pos->isInside( bbox );
    if ( outside )
    {
      i.remove();
      delete pos;
    }
    else   // this one is OK
    {
      pos->insertIntoIndex( candidates );
    }
  }

  qSort( lPos.begin(), lPos.end(), CostCalculator::candidateSortGrow );
  return lPos.count();
}

void FeaturePart::addSizePenalty( int nbp, QList< LabelPosition* >& lPos, double bbx[4], double bby[4] )
{
  if ( !mGeos )
    createGeosGeom();

  GEOSContextHandle_t ctxt = geosContext();
  int geomType = GEOSGeomTypeId_r( ctxt, mGeos );

  double sizeCost = 0;
  if ( geomType == GEOS_LINESTRING )
  {
    double length;
    try
    {
      if ( GEOSLength_r( ctxt, mGeos, &length ) != 1 )
        return; // failed to calculate length
    }
    catch ( GEOSException &e )
    {
      QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );
      return;
    }
    double bbox_length = qMax( bbx[2] - bbx[0], bby[2] - bby[0] );
    if ( length >= bbox_length / 4 )
      return; // the line is longer than quarter of height or width - don't penalize it

    sizeCost = 1 - ( length / ( bbox_length / 4 ) ); // < 0,1 >
  }
  else if ( geomType == GEOS_POLYGON )
  {
    double area;
    try
    {
      if ( GEOSArea_r( ctxt, mGeos, &area ) != 1 )
        return;
    }
    catch ( GEOSException &e )
    {
      QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );
      return;
    }
    double bbox_area = ( bbx[2] - bbx[0] ) * ( bby[2] - bby[0] );
    if ( area >= bbox_area / 16 )
      return; // covers more than 1/16 of our view - don't penalize it

    sizeCost = 1 - ( area / ( bbox_area / 16 ) ); // < 0, 1 >
  }
  else
    return; // no size penalty for points

  // apply the penalty
  for ( int i = 0; i < nbp; i++ )
  {
    lPos.at( i )->setCost( lPos.at( i )->cost() + sizeCost / 100 );
  }
}

bool FeaturePart::isConnected( FeaturePart* p2 )
{
  if ( !p2->mGeos )
    p2->createGeosGeom();

  try
  {
    return ( GEOSPreparedTouches_r( geosContext(), preparedGeom(), p2->mGeos ) == 1 );
  }
  catch ( GEOSException &e )
  {
    QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );
    return false;
  }
}

bool FeaturePart::mergeWithFeaturePart( FeaturePart* other )
{
  if ( !mGeos )
    createGeosGeom();
  if ( !other->mGeos )
    other->createGeosGeom();

  GEOSContextHandle_t ctxt = geosContext();
  try
  {
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
  catch ( GEOSException &e )
  {
    QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );
    return false;
  }
}

double FeaturePart::calculatePriority() const
{
  if ( mLF->alwaysShow() )
  {
    //if feature is set to always show, bump the priority up by orders of magnitude
    //so that other feature's labels are unlikely to be placed over the label for this feature
    //(negative numbers due to how pal::extract calculates inactive cost)
    return -0.2;
  }

  return mLF->priority() >= 0 ? mLF->priority() : mLF->layer()->priority();
}
