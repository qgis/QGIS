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

#include "pal.h"
#include "layer.h"
#include "feature.h"
#include "geomfunction.h"
#include "labelposition.h"
#include "pointset.h"
#include "util.h"
#include "costcalculator.h"

#include "qgis.h"
#include "qgsgeometry.h"
#include "qgsgeos.h"
#include "qgstextlabelfeature.h"
#include "qgsmessagelog.h"
#include "qgsgeometryutils.h"
#include "qgslabeling.h"
#include "qgspolygon.h"
#include "qgstextrendererutils.h"

#include <QLinkedList>
#include <cmath>
#include <cfloat>

using namespace pal;

FeaturePart::FeaturePart( QgsLabelFeature *feat, const GEOSGeometry *geom )
  : mLF( feat )
{
  // we'll remove const, but we won't modify that geometry
  mGeos = const_cast<GEOSGeometry *>( geom );
  mOwnsGeom = false; // geometry is owned by Feature class

  extractCoords( geom );

  holeOf = nullptr;
  for ( int i = 0; i < mHoles.count(); i++ )
  {
    mHoles.at( i )->holeOf = this;
  }

}

FeaturePart::FeaturePart( const FeaturePart &other )
  : PointSet( other )
  , mLF( other.mLF )
{
  for ( const FeaturePart *hole : std::as_const( other.mHoles ) )
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

void FeaturePart::extractCoords( const GEOSGeometry *geom )
{
  const GEOSCoordSequence *coordSeq = nullptr;
  GEOSContextHandle_t geosctxt = QgsGeos::getGEOSHandler();

  type = GEOSGeomTypeId_r( geosctxt, geom );

  if ( type == GEOS_POLYGON )
  {
    if ( GEOSGetNumInteriorRings_r( geosctxt, geom ) > 0 )
    {
      int numHoles = GEOSGetNumInteriorRings_r( geosctxt, geom );

      for ( int i = 0; i < numHoles; ++i )
      {
        const GEOSGeometry *interior = GEOSGetInteriorRingN_r( geosctxt, geom, i );
        FeaturePart *hole = new FeaturePart( mLF, interior );
        hole->holeOf = nullptr;
        // possibly not needed. it's not done for the exterior ring, so I'm not sure
        // why it's just done here...
        GeomFunction::reorderPolygon( hole->x, hole->y );

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
  xmin = ymin = std::numeric_limits<double>::max();
  xmax = ymax = std::numeric_limits<double>::lowest();

  // initialize coordinate arrays
  deleteCoords();
  x.resize( nbPoints );
  y.resize( nbPoints );

  for ( int i = 0; i < nbPoints; ++i )
  {
#if GEOS_VERSION_MAJOR>3 || GEOS_VERSION_MINOR>=8
    GEOSCoordSeq_getXY_r( geosctxt, coordSeq, i, &x[i], &y[i] );
#else
    GEOSCoordSeq_getX_r( geosctxt, coordSeq, i, &x[i] );
    GEOSCoordSeq_getY_r( geosctxt, coordSeq, i, &y[i] );
#endif

    xmax = x[i] > xmax ? x[i] : xmax;
    xmin = x[i] < xmin ? x[i] : xmin;

    ymax = y[i] > ymax ? y[i] : ymax;
    ymin = y[i] < ymin ? y[i] : ymin;
  }
}

Layer *FeaturePart::layer()
{
  return mLF->layer();
}

QgsFeatureId FeaturePart::featureId() const
{
  return mLF->id();
}

std::size_t FeaturePart::maximumPointCandidates() const
{
  return mLF->layer()->maximumPointLabelCandidates();
}

std::size_t FeaturePart::maximumLineCandidates() const
{
  if ( mCachedMaxLineCandidates > 0 )
    return mCachedMaxLineCandidates;

  const double l = length();
  if ( l > 0 )
  {
    const std::size_t candidatesForLineLength = static_cast< std::size_t >( std::ceil( mLF->layer()->mPal->maximumLineCandidatesPerMapUnit() * l ) );
    const std::size_t maxForLayer = mLF->layer()->maximumLineLabelCandidates();
    if ( maxForLayer == 0 )
      mCachedMaxLineCandidates = candidatesForLineLength;
    else
      mCachedMaxLineCandidates = std::min( candidatesForLineLength, maxForLayer );
  }
  else
  {
    mCachedMaxLineCandidates = 1;
  }
  return mCachedMaxLineCandidates;
}

std::size_t FeaturePart::maximumPolygonCandidates() const
{
  if ( mCachedMaxPolygonCandidates > 0 )
    return mCachedMaxPolygonCandidates;

  const double a = area();
  if ( a > 0 )
  {
    const std::size_t candidatesForArea = static_cast< std::size_t >( std::ceil( mLF->layer()->mPal->maximumPolygonCandidatesPerMapUnitSquared() * a ) );
    const std::size_t maxForLayer = mLF->layer()->maximumPolygonLabelCandidates();
    if ( maxForLayer == 0 )
      mCachedMaxPolygonCandidates = candidatesForArea;
    else
      mCachedMaxPolygonCandidates = std::min( candidatesForArea, maxForLayer );
  }
  else
  {
    mCachedMaxPolygonCandidates = 1;
  }
  return mCachedMaxPolygonCandidates;
}

bool FeaturePart::hasSameLabelFeatureAs( FeaturePart *part ) const
{
  if ( !part )
    return false;

  if ( mLF->layer()->name() != part->layer()->name() )
    return false;

  if ( mLF->id() == part->featureId() )
    return true;

  // any part of joined features are also treated as having the same label feature
  int connectedFeatureId = mLF->layer()->connectedFeatureId( mLF->id() );
  return connectedFeatureId >= 0 && connectedFeatureId == mLF->layer()->connectedFeatureId( part->featureId() );
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

int FeaturePart::totalRepeats() const
{
  return mTotalRepeats;
}

void FeaturePart::setTotalRepeats( int totalRepeats )
{
  mTotalRepeats = totalRepeats;
}

std::size_t FeaturePart::createCandidateCenteredOverPoint( double x, double y, std::vector< std::unique_ptr< LabelPosition > > &lPos, double angle )
{
  // get from feature
  double labelW = getLabelWidth( angle );
  double labelH = getLabelHeight( angle );

  double cost = 0.00005;
  int id = lPos.size();

  double xdiff = -labelW / 2.0;
  double ydiff = -labelH / 2.0;

  feature()->setAnchorPosition( QgsPointXY( x, y ) );

  double lx = x + xdiff;
  double ly = y + ydiff;

  if ( mLF->permissibleZonePrepared() )
  {
    if ( !GeomFunction::containsCandidate( mLF->permissibleZonePrepared(), lx, ly, labelW, labelH, angle ) )
    {
      return 0;
    }
  }

  lPos.emplace_back( std::make_unique< LabelPosition >( id, lx, ly, labelW, labelH, angle, cost, this, false, LabelPosition::QuadrantOver ) );
  return 1;
}

std::size_t FeaturePart::createCandidatesOverPoint( double x, double y, std::vector< std::unique_ptr< LabelPosition > > &lPos, double angle )
{
  // get from feature
  double labelW = getLabelWidth( angle );
  double labelH = getLabelHeight( angle );

  double cost = 0.0001;
  int id = lPos.size();

  double xdiff = -labelW / 2.0;
  double ydiff = -labelH / 2.0;

  feature()->setAnchorPosition( QgsPointXY( x, y ) );

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
      double xd = xdiff * std::cos( angle ) - ydiff * std::sin( angle );
      double yd = xdiff * std::sin( angle ) + ydiff * std::cos( angle );
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

  if ( mLF->permissibleZonePrepared() )
  {
    if ( !GeomFunction::containsCandidate( mLF->permissibleZonePrepared(), lx, ly, labelW, labelH, angle ) )
    {
      return 0;
    }
  }

  lPos.emplace_back( std::make_unique< LabelPosition >( id, lx, ly, labelW, labelH, angle, cost, this, false, quadrantFromOffset() ) );
  return 1;
}

std::unique_ptr<LabelPosition> FeaturePart::createCandidatePointOnSurface( PointSet *mapShape )
{
  double px, py;
  try
  {
    GEOSContextHandle_t geosctxt = QgsGeos::getGEOSHandler();
    geos::unique_ptr pointGeom( GEOSPointOnSurface_r( geosctxt, mapShape->geos() ) );
    if ( pointGeom )
    {
      const GEOSCoordSequence *coordSeq = GEOSGeom_getCoordSeq_r( geosctxt, pointGeom.get() );
#if GEOS_VERSION_MAJOR>3 || GEOS_VERSION_MINOR>=8
      unsigned int nPoints = 0;
      GEOSCoordSeq_getSize_r( geosctxt, coordSeq, &nPoints );
      if ( nPoints == 0 )
        return nullptr;
      GEOSCoordSeq_getXY_r( geosctxt, coordSeq, 0, &px, &py );
#else
      GEOSCoordSeq_getX_r( geosctxt, coordSeq, 0, &px );
      GEOSCoordSeq_getY_r( geosctxt, coordSeq, 0, &py );
#endif
    }
  }
  catch ( GEOSException &e )
  {
    qWarning( "GEOS exception: %s", e.what() );
    QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );
    return nullptr;
  }

  return std::make_unique< LabelPosition >( 0, px, py, getLabelWidth(), getLabelHeight(), 0.0, 0.0, this, false, LabelPosition::QuadrantOver );
}

void createCandidateAtOrderedPositionOverPoint( double &labelX, double &labelY, LabelPosition::Quadrant &quadrant, double x, double y, double labelWidth, double labelHeight, QgsPalLayerSettings::PredefinedPointPosition position, double distanceToLabel, const QgsMargins &visualMargin, double symbolWidthOffset, double symbolHeightOffset, double angle )
{
  double alpha = 0.0;
  double deltaX = 0;
  double deltaY = 0;

  switch ( position )
  {
    case QgsPalLayerSettings::TopLeft:
      quadrant = LabelPosition::QuadrantAboveLeft;
      alpha = 3 * M_PI_4;
      deltaX = -labelWidth + visualMargin.right() - symbolWidthOffset;
      deltaY = -visualMargin.bottom() + symbolHeightOffset;
      break;

    case QgsPalLayerSettings::TopSlightlyLeft:
      quadrant = LabelPosition::QuadrantAboveRight; //right quadrant, so labels are left-aligned
      alpha = M_PI_2;
      deltaX = -labelWidth / 4.0 - visualMargin.left();
      deltaY = -visualMargin.bottom() + symbolHeightOffset;
      break;

    case QgsPalLayerSettings::TopMiddle:
      quadrant = LabelPosition::QuadrantAbove;
      alpha = M_PI_2;
      deltaX = -labelWidth / 2.0;
      deltaY = -visualMargin.bottom() + symbolHeightOffset;
      break;

    case QgsPalLayerSettings::TopSlightlyRight:
      quadrant = LabelPosition::QuadrantAboveLeft; //left quadrant, so labels are right-aligned
      alpha = M_PI_2;
      deltaX = -labelWidth * 3.0 / 4.0 + visualMargin.right();
      deltaY = -visualMargin.bottom() + symbolHeightOffset;
      break;

    case QgsPalLayerSettings::TopRight:
      quadrant = LabelPosition::QuadrantAboveRight;
      alpha = M_PI_4;
      deltaX = - visualMargin.left() + symbolWidthOffset;
      deltaY = -visualMargin.bottom() + symbolHeightOffset;
      break;

    case QgsPalLayerSettings::MiddleLeft:
      quadrant = LabelPosition::QuadrantLeft;
      alpha = M_PI;
      deltaX = -labelWidth + visualMargin.right() - symbolWidthOffset;
      deltaY = -labelHeight / 2.0;// TODO - should this be adjusted by visual margin??
      break;

    case QgsPalLayerSettings::MiddleRight:
      quadrant = LabelPosition::QuadrantRight;
      alpha = 0.0;
      deltaX = -visualMargin.left() + symbolWidthOffset;
      deltaY = -labelHeight / 2.0;// TODO - should this be adjusted by visual margin??
      break;

    case QgsPalLayerSettings::BottomLeft:
      quadrant = LabelPosition::QuadrantBelowLeft;
      alpha = 5 * M_PI_4;
      deltaX = -labelWidth + visualMargin.right() - symbolWidthOffset;
      deltaY = -labelHeight + visualMargin.top() - symbolHeightOffset;
      break;

    case QgsPalLayerSettings::BottomSlightlyLeft:
      quadrant = LabelPosition::QuadrantBelowRight; //right quadrant, so labels are left-aligned
      alpha = 3 * M_PI_2;
      deltaX = -labelWidth / 4.0 - visualMargin.left();
      deltaY = -labelHeight + visualMargin.top() - symbolHeightOffset;
      break;

    case QgsPalLayerSettings::BottomMiddle:
      quadrant = LabelPosition::QuadrantBelow;
      alpha = 3 * M_PI_2;
      deltaX = -labelWidth / 2.0;
      deltaY = -labelHeight + visualMargin.top() - symbolHeightOffset;
      break;

    case QgsPalLayerSettings::BottomSlightlyRight:
      quadrant = LabelPosition::QuadrantBelowLeft; //left quadrant, so labels are right-aligned
      alpha = 3 * M_PI_2;
      deltaX = -labelWidth * 3.0 / 4.0 + visualMargin.right();
      deltaY = -labelHeight + visualMargin.top() - symbolHeightOffset;
      break;

    case QgsPalLayerSettings::BottomRight:
      quadrant = LabelPosition::QuadrantBelowRight;
      alpha = 7 * M_PI_4;
      deltaX = -visualMargin.left() + symbolWidthOffset;
      deltaY = -labelHeight + visualMargin.top() - symbolHeightOffset;
      break;
  }

  // Take care of the label angle when creating candidates. See pr comments #44944 for details
  // https://github.com/qgis/QGIS/pull/44944#issuecomment-914670088
  QTransform transformRotation;
  transformRotation.rotate( angle * 180 / M_PI );
  transformRotation.map( deltaX, deltaY, &deltaX, &deltaY );

  //have bearing, distance - calculate reference point
  double referenceX = std::cos( alpha ) * distanceToLabel + x;
  double referenceY = std::sin( alpha ) * distanceToLabel + y;

  labelX = referenceX + deltaX;
  labelY = referenceY + deltaY;
}

std::size_t FeaturePart::createCandidatesAtOrderedPositionsOverPoint( double x, double y, std::vector< std::unique_ptr< LabelPosition > > &lPos, double angle )
{
  const QVector< QgsPalLayerSettings::PredefinedPointPosition > positions = mLF->predefinedPositionOrder();
  double labelWidth = getLabelWidth( angle );
  double labelHeight = getLabelHeight( angle );
  double distanceToLabel = getLabelDistance();
  const QgsMargins &visualMargin = mLF->visualMargin();

  double symbolWidthOffset = ( mLF->offsetType() == QgsPalLayerSettings::FromSymbolBounds ? mLF->symbolSize().width() / 2.0 : 0.0 );
  double symbolHeightOffset = ( mLF->offsetType() == QgsPalLayerSettings::FromSymbolBounds ? mLF->symbolSize().height() / 2.0 : 0.0 );

  double cost = 0.0001;
  std::size_t i = lPos.size();

  const std::size_t maxNumberCandidates = mLF->layer()->maximumPointLabelCandidates();
  std::size_t created = 0;
  for ( QgsPalLayerSettings::PredefinedPointPosition position : positions )
  {
    LabelPosition::Quadrant quadrant = LabelPosition::QuadrantAboveLeft;

    double labelX = 0;
    double labelY = 0;
    createCandidateAtOrderedPositionOverPoint( labelX, labelY, quadrant, x, y, labelWidth, labelHeight, position, distanceToLabel, visualMargin, symbolWidthOffset, symbolHeightOffset, angle );

    if ( ! mLF->permissibleZonePrepared() || GeomFunction::containsCandidate( mLF->permissibleZonePrepared(), labelX, labelY, labelWidth, labelHeight, angle ) )
    {
      lPos.emplace_back( std::make_unique< LabelPosition >( i, labelX, labelY, labelWidth, labelHeight, angle, cost, this, false, quadrant ) );
      created++;
      //TODO - tweak
      cost += 0.001;
      if ( maxNumberCandidates > 0 && created >= maxNumberCandidates )
        break;
    }
    ++i;
  }

  return created;
}

std::size_t FeaturePart::createCandidatesAroundPoint( double x, double y, std::vector< std::unique_ptr< LabelPosition > > &lPos, double angle )
{
  double labelWidth = getLabelWidth( angle );
  double labelHeight = getLabelHeight( angle );
  double distanceToLabel = getLabelDistance();

  std::size_t maxNumberCandidates = mLF->layer()->maximumPointLabelCandidates();
  if ( maxNumberCandidates == 0 )
    maxNumberCandidates = 16;

  int icost = 0;
  int inc = 2;
  int id = lPos.size();

  double candidateAngleIncrement = 2 * M_PI / maxNumberCandidates; /* angle bw 2 pos */

  /* various angles */
  double a90  = M_PI_2;
  double a180 = M_PI;
  double a270 = a180 + a90;
  double a360 = 2 * M_PI;

  double gamma1, gamma2;

  if ( distanceToLabel > 0 )
  {
    gamma1 = std::atan2( labelHeight / 2, distanceToLabel + labelWidth / 2 );
    gamma2 = std::atan2( labelWidth / 2, distanceToLabel + labelHeight / 2 );
  }
  else
  {
    gamma1 = gamma2 = a90 / 3.0;
  }

  if ( gamma1 > a90 / 3.0 )
    gamma1 = a90 / 3.0;

  if ( gamma2 > a90 / 3.0 )
    gamma2 = a90 / 3.0;

  std::size_t numberCandidatesGenerated = 0;

  std::size_t i;
  double angleToCandidate;
  for ( i = 0, angleToCandidate = M_PI_4; i < maxNumberCandidates; i++, angleToCandidate += candidateAngleIncrement )
  {
    double deltaX = 0.0;
    double deltaY = 0.0;

    if ( angleToCandidate > a360 )
      angleToCandidate -= a360;

    LabelPosition::Quadrant quadrant = LabelPosition::QuadrantOver;

    if ( angleToCandidate < gamma1 || angleToCandidate > a360 - gamma1 )  // on the right
    {
      deltaX = distanceToLabel;
      double iota = ( angleToCandidate + gamma1 );
      if ( iota > a360 - gamma1 )
        iota -= a360;

      //ly += -yrm/2.0 + tan(alpha)*(distlabel + xrm/2);
      deltaY = -labelHeight + labelHeight * iota / ( 2 * gamma1 );

      quadrant = LabelPosition::QuadrantRight;
    }
    else if ( angleToCandidate < a90 - gamma2 )  // top-right
    {
      deltaX = distanceToLabel * std::cos( angleToCandidate );
      deltaY = distanceToLabel * std::sin( angleToCandidate );
      quadrant = LabelPosition::QuadrantAboveRight;
    }
    else if ( angleToCandidate < a90 + gamma2 ) // top
    {
      //lx += -xrm/2.0 - tan(alpha+a90)*(distlabel + yrm/2);
      deltaX = -labelWidth * ( angleToCandidate - a90 + gamma2 ) / ( 2 * gamma2 );
      deltaY = distanceToLabel;
      quadrant = LabelPosition::QuadrantAbove;
    }
    else if ( angleToCandidate < a180 - gamma1 )  // top left
    {
      deltaX = distanceToLabel * std::cos( angleToCandidate ) - labelWidth;
      deltaY = distanceToLabel * std::sin( angleToCandidate );
      quadrant = LabelPosition::QuadrantAboveLeft;
    }
    else if ( angleToCandidate < a180 + gamma1 ) // left
    {
      deltaX = -distanceToLabel - labelWidth;
      //ly += -yrm/2.0 - tan(alpha)*(distlabel + xrm/2);
      deltaY = - ( angleToCandidate - a180 + gamma1 ) * labelHeight / ( 2 * gamma1 );
      quadrant = LabelPosition::QuadrantLeft;
    }
    else if ( angleToCandidate < a270 - gamma2 ) // down - left
    {
      deltaX = distanceToLabel * std::cos( angleToCandidate ) - labelWidth;
      deltaY = distanceToLabel * std::sin( angleToCandidate ) - labelHeight;
      quadrant = LabelPosition::QuadrantBelowLeft;
    }
    else if ( angleToCandidate < a270 + gamma2 ) // down
    {
      deltaY = -distanceToLabel - labelHeight;
      //lx += -xrm/2.0 + tan(alpha+a90)*(distlabel + yrm/2);
      deltaX = -labelWidth + ( angleToCandidate - a270 + gamma2 ) * labelWidth / ( 2 * gamma2 );
      quadrant = LabelPosition::QuadrantBelow;
    }
    else if ( angleToCandidate < a360 ) // down - right
    {
      deltaX = distanceToLabel * std::cos( angleToCandidate );
      deltaY = distanceToLabel * std::sin( angleToCandidate ) - labelHeight;
      quadrant = LabelPosition::QuadrantBelowRight;
    }

    // Take care of the label angle when creating candidates. See pr comments #44944 for details
    // https://github.com/qgis/QGIS/pull/44944#issuecomment-914670088
    QTransform transformRotation;
    transformRotation.rotate( angle * 180 / M_PI );
    transformRotation.map( deltaX, deltaY, &deltaX, &deltaY );

    double labelX = x + deltaX;
    double labelY = y + deltaY;

    double cost;

    if ( maxNumberCandidates == 1 )
      cost = 0.0001;
    else
      cost = 0.0001 + 0.0020 * double( icost ) / double( maxNumberCandidates - 1 );


    if ( mLF->permissibleZonePrepared() )
    {
      if ( !GeomFunction::containsCandidate( mLF->permissibleZonePrepared(), labelX, labelY, labelWidth, labelHeight, angle ) )
      {
        continue;
      }
    }

    lPos.emplace_back( std::make_unique< LabelPosition >( id + i, labelX, labelY, labelWidth, labelHeight, angle, cost, this, false, quadrant ) );
    numberCandidatesGenerated++;

    icost += inc;

    if ( icost == static_cast< int >( maxNumberCandidates ) )
    {
      icost = static_cast< int >( maxNumberCandidates ) - 1;
      inc = -2;
    }
    else if ( icost > static_cast< int >( maxNumberCandidates ) )
    {
      icost = static_cast< int >( maxNumberCandidates ) - 2;
      inc = -2;
    }

  }

  return numberCandidatesGenerated;
}

std::size_t FeaturePart::createCandidatesAlongLine( std::vector< std::unique_ptr< LabelPosition > > &lPos, PointSet *mapShape, bool allowOverrun, Pal *pal )
{
  if ( allowOverrun )
  {
    double shapeLength = mapShape->length();
    if ( totalRepeats() > 1 && shapeLength < getLabelWidth() )
      return 0;
    else if ( shapeLength < getLabelWidth() - 2 * std::min( getLabelWidth(), mLF->overrunDistance() ) )
    {
      // label doesn't fit on this line, don't waste time trying to make candidates
      return 0;
    }
  }

  //prefer to label along straightish segments:
  std::size_t candidates = 0;

  if ( mLF->lineAnchorType() == QgsLabelLineSettings::AnchorType::HintOnly )
    candidates = createCandidatesAlongLineNearStraightSegments( lPos, mapShape, pal );

  const std::size_t candidateTargetCount = maximumLineCandidates();
  if ( candidates < candidateTargetCount )
  {
    // but not enough candidates yet, so fallback to labeling near whole line's midpoint
    candidates = createCandidatesAlongLineNearMidpoint( lPos, mapShape, candidates > 0 ? 0.01 : 0.0, pal );
  }
  return candidates;
}

std::size_t FeaturePart::createHorizontalCandidatesAlongLine( std::vector<std::unique_ptr<LabelPosition> > &lPos, PointSet *mapShape, Pal *pal )
{
  const double labelWidth = getLabelWidth();
  const double labelHeight = getLabelHeight();

  PointSet *line = mapShape;
  int nbPoints = line->nbPoints;
  std::vector< double > &x = line->x;
  std::vector< double > &y = line->y;

  std::vector< double > segmentLengths( nbPoints - 1 ); // segments lengths distance bw pt[i] && pt[i+1]
  std::vector< double >distanceToSegment( nbPoints ); // absolute distance bw pt[0] and pt[i] along the line

  double totalLineLength = 0.0; // line length
  for ( int i = 0; i < line->nbPoints - 1; i++ )
  {
    if ( i == 0 )
      distanceToSegment[i] = 0;
    else
      distanceToSegment[i] = distanceToSegment[i - 1] + segmentLengths[i - 1];

    segmentLengths[i] = GeomFunction::dist_euc2d( x[i], y[i], x[i + 1], y[i + 1] );
    totalLineLength += segmentLengths[i];
  }
  distanceToSegment[line->nbPoints - 1] = totalLineLength;

  const std::size_t candidateTargetCount = maximumLineCandidates();
  double lineStepDistance = 0;

  const double lineAnchorPoint = totalLineLength * mLF->lineAnchorPercent();
  double currentDistanceAlongLine = lineStepDistance;
  switch ( mLF->lineAnchorType() )
  {
    case QgsLabelLineSettings::AnchorType::HintOnly:
      lineStepDistance = totalLineLength / ( candidateTargetCount + 1 ); // distance to move along line with each candidate
      break;

    case QgsLabelLineSettings::AnchorType::Strict:
      currentDistanceAlongLine = lineAnchorPoint;
      lineStepDistance = -1;
      break;
  }

  const QgsLabelLineSettings::AnchorTextPoint textPoint = mLF->lineAnchorTextPoint();

  double candidateCenterX, candidateCenterY;
  int i = 0;
  while ( currentDistanceAlongLine <= totalLineLength )
  {
    if ( pal->isCanceled() )
    {
      return lPos.size();
    }

    line->getPointByDistance( segmentLengths.data(), distanceToSegment.data(), currentDistanceAlongLine, &candidateCenterX, &candidateCenterY );

    // penalize positions which are further from the line's anchor point
    double cost = std::fabs( lineAnchorPoint - currentDistanceAlongLine ) / totalLineLength; // <0, 0.5>
    cost /= 1000;  // < 0, 0.0005 >

    double labelX = 0;
    switch ( textPoint )
    {
      case QgsLabelLineSettings::AnchorTextPoint::StartOfText:
        labelX = candidateCenterX;
        break;
      case QgsLabelLineSettings::AnchorTextPoint::CenterOfText:
        labelX = candidateCenterX - labelWidth / 2;
        break;
      case QgsLabelLineSettings::AnchorTextPoint::EndOfText:
        labelX = candidateCenterX - labelWidth;
        break;
      case QgsLabelLineSettings::AnchorTextPoint::FollowPlacement:
        // not possible here
        break;
    }
    lPos.emplace_back( std::make_unique< LabelPosition >( i, labelX, candidateCenterY - labelHeight / 2, labelWidth, labelHeight, 0, cost, this, false, LabelPosition::QuadrantOver ) );

    currentDistanceAlongLine += lineStepDistance;

    i++;

    if ( lineStepDistance < 0 )
      break;
  }

  return lPos.size();
}

std::size_t FeaturePart::createCandidatesAlongLineNearStraightSegments( std::vector< std::unique_ptr< LabelPosition > > &lPos, PointSet *mapShape, Pal *pal )
{
  double labelWidth = getLabelWidth();
  double labelHeight = getLabelHeight();
  double distanceLineToLabel = getLabelDistance();
  QgsLabeling::LinePlacementFlags flags = mLF->arrangementFlags();
  if ( flags == 0 )
    flags = QgsLabeling::LinePlacementFlag::OnLine; // default flag

  // first scan through the whole line and look for segments where the angle at a node is greater than 45 degrees - these form a "hard break" which labels shouldn't cross over
  QVector< int > extremeAngleNodes;
  PointSet *line = mapShape;
  int numberNodes = line->nbPoints;
  std::vector< double > &x = line->x;
  std::vector< double > &y = line->y;

  // closed line? if so, we need to handle the final node angle
  bool closedLine = qgsDoubleNear( x[0], x[ numberNodes - 1] ) && qgsDoubleNear( y[0], y[numberNodes - 1 ] );
  for ( int i = 1; i <= numberNodes - ( closedLine ? 1 : 2 ); ++i )
  {
    double x1 = x[i - 1];
    double x2 = x[i];
    double x3 = x[ i == numberNodes - 1 ? 1 : i + 1]; // wraparound for closed linestrings
    double y1 = y[i - 1];
    double y2 = y[i];
    double y3 = y[ i == numberNodes - 1 ? 1 : i + 1]; // wraparound for closed linestrings
    if ( qgsDoubleNear( y2, y3 ) && qgsDoubleNear( x2, x3 ) )
      continue;
    if ( qgsDoubleNear( y1, y2 ) && qgsDoubleNear( x1, x2 ) )
      continue;
    double vertexAngle = M_PI - ( std::atan2( y3 - y2, x3 - x2 ) - std::atan2( y2 - y1, x2 - x1 ) );
    vertexAngle = QgsGeometryUtils::normalizedAngle( vertexAngle );

    // extreme angles form more than 45 degree angle at a node - these are the ones we don't want labels to cross
    if ( vertexAngle < M_PI * 135.0 / 180.0 || vertexAngle > M_PI * 225.0 / 180.0 )
      extremeAngleNodes << i;
  }
  extremeAngleNodes << numberNodes - 1;

  if ( extremeAngleNodes.isEmpty() )
  {
    // no extreme angles - createCandidatesAlongLineNearMidpoint will be more appropriate
    return 0;
  }

  // calculate lengths of segments, and work out longest straight-ish segment
  std::vector< double > segmentLengths( numberNodes - 1 ); // segments lengths distance bw pt[i] && pt[i+1]
  std::vector< double > distanceToSegment( numberNodes ); // absolute distance bw pt[0] and pt[i] along the line
  double totalLineLength = 0.0;
  QVector< double > straightSegmentLengths;
  QVector< double > straightSegmentAngles;
  straightSegmentLengths.reserve( extremeAngleNodes.size() + 1 );
  straightSegmentAngles.reserve( extremeAngleNodes.size() + 1 );
  double currentStraightSegmentLength = 0;
  double longestSegmentLength = 0;
  int segmentIndex = 0;
  double segmentStartX = x[0];
  double segmentStartY = y[0];
  for ( int i = 0; i < numberNodes - 1; i++ )
  {
    if ( i == 0 )
      distanceToSegment[i] = 0;
    else
      distanceToSegment[i] = distanceToSegment[i - 1] + segmentLengths[i - 1];

    segmentLengths[i] = GeomFunction::dist_euc2d( x[i], y[i], x[i + 1], y[i + 1] );
    totalLineLength += segmentLengths[i];
    if ( extremeAngleNodes.contains( i ) )
    {
      // at an extreme angle node, so reset counters
      straightSegmentLengths << currentStraightSegmentLength;
      straightSegmentAngles << QgsGeometryUtils::normalizedAngle( std::atan2( y[i] - segmentStartY, x[i] - segmentStartX ) );
      longestSegmentLength = std::max( longestSegmentLength, currentStraightSegmentLength );
      segmentIndex++;
      currentStraightSegmentLength = 0;
      segmentStartX = x[i];
      segmentStartY = y[i];
    }
    currentStraightSegmentLength += segmentLengths[i];
  }
  distanceToSegment[line->nbPoints - 1] = totalLineLength;
  straightSegmentLengths << currentStraightSegmentLength;
  straightSegmentAngles << QgsGeometryUtils::normalizedAngle( std::atan2( y[numberNodes - 1] - segmentStartY, x[numberNodes - 1] - segmentStartX ) );
  longestSegmentLength = std::max( longestSegmentLength, currentStraightSegmentLength );
  const double lineAnchorPoint = totalLineLength * mLF->lineAnchorPercent();

  if ( totalLineLength < labelWidth )
  {
    return 0; //createCandidatesAlongLineNearMidpoint will be more appropriate
  }

  const QgsLabelLineSettings::AnchorTextPoint textPoint = mLF->lineAnchorTextPoint();

  const std::size_t candidateTargetCount = maximumLineCandidates();
  double lineStepDistance = ( totalLineLength - labelWidth ); // distance to move along line with each candidate
  lineStepDistance = std::min( std::min( labelHeight, labelWidth ), lineStepDistance / candidateTargetCount );

  double distanceToEndOfSegment = 0.0;
  int lastNodeInSegment = 0;
  // finally, loop through all these straight segments. For each we create candidates along the straight segment.
  for ( int i = 0; i < straightSegmentLengths.count(); ++i )
  {
    currentStraightSegmentLength = straightSegmentLengths.at( i );
    double currentSegmentAngle = straightSegmentAngles.at( i );
    lastNodeInSegment = extremeAngleNodes.at( i );
    double distanceToStartOfSegment = distanceToEndOfSegment;
    distanceToEndOfSegment = distanceToSegment[ lastNodeInSegment ];
    double distanceToCenterOfSegment = 0.5 * ( distanceToEndOfSegment + distanceToStartOfSegment );

    if ( currentStraightSegmentLength < labelWidth )
      // can't fit a label on here
      continue;

    double currentDistanceAlongLine = distanceToStartOfSegment;
    double candidateStartX, candidateStartY, candidateEndX, candidateEndY;
    double candidateLength = 0.0;
    double cost = 0.0;
    double angle = 0.0;
    double beta = 0.0;

    //calculate some cost penalties
    double segmentCost = 1.0 - ( distanceToEndOfSegment - distanceToStartOfSegment ) / longestSegmentLength; // 0 -> 1 (lower for longer segments)
    double segmentAngleCost = 1 - std::fabs( std::fmod( currentSegmentAngle, M_PI ) - M_PI_2 ) / M_PI_2; // 0 -> 1, lower for more horizontal segments

    while ( currentDistanceAlongLine + labelWidth < distanceToEndOfSegment )
    {
      if ( pal->isCanceled() )
      {
        return lPos.size();
      }

      // calculate positions along linestring corresponding to start and end of current label candidate
      line->getPointByDistance( segmentLengths.data(), distanceToSegment.data(), currentDistanceAlongLine, &candidateStartX, &candidateStartY );
      line->getPointByDistance( segmentLengths.data(), distanceToSegment.data(), currentDistanceAlongLine + labelWidth, &candidateEndX, &candidateEndY );

      candidateLength = std::sqrt( ( candidateEndX - candidateStartX ) * ( candidateEndX - candidateStartX ) + ( candidateEndY - candidateStartY ) * ( candidateEndY - candidateStartY ) );


      // LOTS OF DIFFERENT COSTS TO BALANCE HERE - feel free to tweak these, but please add a unit test
      // which covers the situation you are adjusting for (e.g., "given equal length lines, choose the more horizontal line")

      cost = candidateLength / labelWidth;
      if ( cost > 0.98 )
        cost = 0.0001;
      else
      {
        // jaggy line has a greater cost
        cost = ( 1 - cost ) / 100; // ranges from 0.0001 to 0.01 (however a cost 0.005 is already a lot!)
      }

      const double labelCenter = currentDistanceAlongLine + labelWidth / 2.0;
      double labelTextAnchor = 0;
      switch ( textPoint )
      {
        case QgsLabelLineSettings::AnchorTextPoint::StartOfText:
          labelTextAnchor = currentDistanceAlongLine;
          break;
        case QgsLabelLineSettings::AnchorTextPoint::CenterOfText:
          labelTextAnchor = currentDistanceAlongLine + labelWidth / 2.0;
          break;
        case QgsLabelLineSettings::AnchorTextPoint::EndOfText:
          labelTextAnchor = currentDistanceAlongLine + labelWidth;
          break;
        case QgsLabelLineSettings::AnchorTextPoint::FollowPlacement:
          // not possible here
          break;
      }

      const bool placementIsFlexible = mLF->lineAnchorPercent() > 0.1 && mLF->lineAnchorPercent() < 0.9;
      // penalize positions which are further from the straight segments's midpoint
      if ( placementIsFlexible )
      {
        // only apply this if labels are being placed toward the center of overall lines -- otherwise it messes with the distance from anchor cost
        double costCenter = 2 * std::fabs( labelCenter - distanceToCenterOfSegment ) / ( distanceToEndOfSegment - distanceToStartOfSegment ); // 0 -> 1
        cost += costCenter * 0.0005;  // < 0, 0.0005 >
      }

      if ( !closedLine )
      {
        // penalize positions which are further from line anchor point of whole linestring (by default the middle of the line)
        // this only applies to non closed linestrings, since the middle of a closed linestring is effectively arbitrary
        // and irrelevant to labeling
        double costLineCenter = 2 * std::fabs( labelTextAnchor - lineAnchorPoint ) / totalLineLength;  // 0 -> 1
        cost += costLineCenter * 0.0005;  // < 0, 0.0005 >
      }

      if ( placementIsFlexible )
      {
        cost += segmentCost * 0.0005; // prefer labels on longer straight segments
        cost += segmentAngleCost * 0.0001; // prefer more horizontal segments, but this is less important than length considerations
      }

      if ( qgsDoubleNear( candidateEndY, candidateStartY ) && qgsDoubleNear( candidateEndX, candidateStartX ) )
      {
        angle = 0.0;
      }
      else
        angle = std::atan2( candidateEndY - candidateStartY, candidateEndX - candidateStartX );

      labelWidth = getLabelWidth( angle );
      labelHeight = getLabelHeight( angle );
      beta = angle + M_PI_2;

      if ( mLF->layer()->arrangement() == QgsPalLayerSettings::Line )
      {
        // find out whether the line direction for this candidate is from right to left
        bool isRightToLeft = ( angle > M_PI_2 || angle <= -M_PI_2 );
        // meaning of above/below may be reversed if using map orientation and the line has right-to-left direction
        bool reversed = ( ( flags & QgsLabeling::LinePlacementFlag::MapOrientation ) ? isRightToLeft : false );
        bool aboveLine = ( !reversed && ( flags & QgsLabeling::LinePlacementFlag::AboveLine ) ) || ( reversed && ( flags & QgsLabeling::LinePlacementFlag::BelowLine ) );
        bool belowLine = ( !reversed && ( flags & QgsLabeling::LinePlacementFlag::BelowLine ) ) || ( reversed && ( flags & QgsLabeling::LinePlacementFlag::AboveLine ) );

        if ( belowLine )
        {
          if ( !mLF->permissibleZonePrepared() || GeomFunction::containsCandidate( mLF->permissibleZonePrepared(), candidateStartX - std::cos( beta ) * ( distanceLineToLabel + labelHeight ), candidateStartY - std::sin( beta ) * ( distanceLineToLabel + labelHeight ), labelWidth, labelHeight, angle ) )
          {
            const double candidateCost = cost + ( reversed ? 0 : 0.001 );
            lPos.emplace_back( std::make_unique< LabelPosition >( i, candidateStartX - std::cos( beta ) * ( distanceLineToLabel + labelHeight ), candidateStartY - std::sin( beta ) * ( distanceLineToLabel + labelHeight ), labelWidth, labelHeight, angle, candidateCost, this, isRightToLeft, LabelPosition::QuadrantOver ) ); // Line
          }
        }
        if ( aboveLine )
        {
          if ( !mLF->permissibleZonePrepared() || GeomFunction::containsCandidate( mLF->permissibleZonePrepared(), candidateStartX + std::cos( beta ) *distanceLineToLabel, candidateStartY + std::sin( beta ) *distanceLineToLabel, labelWidth, labelHeight, angle ) )
          {
            const double candidateCost = cost + ( !reversed ? 0 : 0.001 ); // no extra cost for above line placements
            lPos.emplace_back( std::make_unique< LabelPosition >( i, candidateStartX + std::cos( beta ) *distanceLineToLabel, candidateStartY + std::sin( beta ) *distanceLineToLabel, labelWidth, labelHeight, angle, candidateCost, this, isRightToLeft, LabelPosition::QuadrantOver ) ); // Line
          }
        }
        if ( flags & QgsLabeling::LinePlacementFlag::OnLine )
        {
          if ( !mLF->permissibleZonePrepared() || GeomFunction::containsCandidate( mLF->permissibleZonePrepared(), candidateStartX - labelHeight * std::cos( beta ) / 2, candidateStartY - labelHeight * std::sin( beta ) / 2, labelWidth, labelHeight, angle ) )
          {
            const double candidateCost = cost + 0.002;
            lPos.emplace_back( std::make_unique< LabelPosition >( i, candidateStartX - labelHeight * std::cos( beta ) / 2, candidateStartY - labelHeight * std::sin( beta ) / 2, labelWidth, labelHeight, angle, candidateCost, this, isRightToLeft, LabelPosition::QuadrantOver ) ); // Line
          }
        }
      }
      else if ( mLF->layer()->arrangement() == QgsPalLayerSettings::Horizontal )
      {
        lPos.emplace_back( std::make_unique< LabelPosition >( i, candidateStartX - labelWidth / 2, candidateStartY - labelHeight / 2, labelWidth, labelHeight, 0, cost, this, false, LabelPosition::QuadrantOver ) ); // Line
      }
      else
      {
        // an invalid arrangement?
      }

      currentDistanceAlongLine += lineStepDistance;
    }
  }

  return lPos.size();
}

std::size_t FeaturePart::createCandidatesAlongLineNearMidpoint( std::vector< std::unique_ptr< LabelPosition > > &lPos, PointSet *mapShape, double initialCost, Pal *pal )
{
  double distanceLineToLabel = getLabelDistance();

  double labelWidth = getLabelWidth();
  double labelHeight = getLabelHeight();

  double angle;
  double cost;

  QgsLabeling::LinePlacementFlags flags = mLF->arrangementFlags();
  if ( flags == 0 )
    flags = QgsLabeling::LinePlacementFlag::OnLine; // default flag

  PointSet *line = mapShape;
  int nbPoints = line->nbPoints;
  std::vector< double > &x = line->x;
  std::vector< double > &y = line->y;

  std::vector< double > segmentLengths( nbPoints - 1 ); // segments lengths distance bw pt[i] && pt[i+1]
  std::vector< double >distanceToSegment( nbPoints ); // absolute distance bw pt[0] and pt[i] along the line

  double totalLineLength = 0.0; // line length
  for ( int i = 0; i < line->nbPoints - 1; i++ )
  {
    if ( i == 0 )
      distanceToSegment[i] = 0;
    else
      distanceToSegment[i] = distanceToSegment[i - 1] + segmentLengths[i - 1];

    segmentLengths[i] = GeomFunction::dist_euc2d( x[i], y[i], x[i + 1], y[i + 1] );
    totalLineLength += segmentLengths[i];
  }
  distanceToSegment[line->nbPoints - 1] = totalLineLength;

  double lineStepDistance = ( totalLineLength - labelWidth ); // distance to move along line with each candidate
  double currentDistanceAlongLine = 0;

  const QgsLabelLineSettings::AnchorTextPoint textPoint = mLF->lineAnchorTextPoint();

  const std::size_t candidateTargetCount = maximumLineCandidates();

  if ( totalLineLength > labelWidth )
  {
    lineStepDistance = std::min( std::min( labelHeight, labelWidth ), lineStepDistance / candidateTargetCount );
  }
  else if ( !line->isClosed() ) // line length < label width => centering label position
  {
    currentDistanceAlongLine = - ( labelWidth - totalLineLength ) / 2.0;
    lineStepDistance = -1;
    totalLineLength = labelWidth;
  }
  else
  {
    // closed line, not long enough for label => no candidates!
    currentDistanceAlongLine = std::numeric_limits< double >::max();
  }

  const double lineAnchorPoint = totalLineLength * std::min( 0.99, mLF->lineAnchorPercent() ); // don't actually go **all** the way to end of line, just very close to!

  switch ( mLF->lineAnchorType() )
  {
    case QgsLabelLineSettings::AnchorType::HintOnly:
      break;

    case QgsLabelLineSettings::AnchorType::Strict:
      switch ( textPoint )
      {
        case QgsLabelLineSettings::AnchorTextPoint::StartOfText:
          currentDistanceAlongLine = std::min( lineAnchorPoint, totalLineLength * 0.99 - labelWidth );
          break;
        case QgsLabelLineSettings::AnchorTextPoint::CenterOfText:
          currentDistanceAlongLine = std::min( lineAnchorPoint - labelWidth / 2, totalLineLength * 0.99 - labelWidth );
          break;
        case QgsLabelLineSettings::AnchorTextPoint::EndOfText:
          currentDistanceAlongLine = std::min( lineAnchorPoint - labelWidth, totalLineLength * 0.99 - labelWidth );
          break;
        case QgsLabelLineSettings::AnchorTextPoint::FollowPlacement:
          // not possible here
          break;
      }
      lineStepDistance = -1;
      break;
  }

  double candidateLength;
  double beta;
  double candidateStartX, candidateStartY, candidateEndX, candidateEndY;
  int i = 0;
  while ( currentDistanceAlongLine <= totalLineLength - labelWidth || mLF->lineAnchorType() == QgsLabelLineSettings::AnchorType::Strict )
  {
    if ( pal->isCanceled() )
    {
      return lPos.size();
    }

    // calculate positions along linestring corresponding to start and end of current label candidate
    line->getPointByDistance( segmentLengths.data(), distanceToSegment.data(), currentDistanceAlongLine, &candidateStartX, &candidateStartY );
    line->getPointByDistance( segmentLengths.data(), distanceToSegment.data(), currentDistanceAlongLine + labelWidth, &candidateEndX, &candidateEndY );

    if ( currentDistanceAlongLine < 0 )
    {
      // label is bigger than line, use whole available line
      candidateLength = std::sqrt( ( x[nbPoints - 1] - x[0] ) * ( x[nbPoints - 1] - x[0] )
                                   + ( y[nbPoints - 1] - y[0] ) * ( y[nbPoints - 1] - y[0] ) );
    }
    else
    {
      candidateLength = std::sqrt( ( candidateEndX - candidateStartX ) * ( candidateEndX - candidateStartX ) + ( candidateEndY - candidateStartY ) * ( candidateEndY - candidateStartY ) );
    }

    cost = candidateLength / labelWidth;
    if ( cost > 0.98 )
      cost = 0.0001;
    else
    {
      // jaggy line has a greater cost
      cost = ( 1 - cost ) / 100; // ranges from 0.0001 to 0.01 (however a cost 0.005 is already a lot!)
    }

    // penalize positions which are further from the line's anchor point
    double textAnchorPoint = 0;
    switch ( textPoint )
    {
      case QgsLabelLineSettings::AnchorTextPoint::StartOfText:
        textAnchorPoint = currentDistanceAlongLine;
        break;
      case QgsLabelLineSettings::AnchorTextPoint::CenterOfText:
        textAnchorPoint = currentDistanceAlongLine + labelWidth / 2;
        break;
      case QgsLabelLineSettings::AnchorTextPoint::EndOfText:
        textAnchorPoint = currentDistanceAlongLine + labelWidth;
        break;
      case QgsLabelLineSettings::AnchorTextPoint::FollowPlacement:
        // not possible here
        break;
    }
    double costCenter = std::fabs( lineAnchorPoint - textAnchorPoint ) / totalLineLength; // <0, 0.5>
    cost += costCenter / 1000;  // < 0, 0.0005 >
    cost += initialCost;

    if ( qgsDoubleNear( candidateEndY, candidateStartY ) && qgsDoubleNear( candidateEndX, candidateStartX ) )
    {
      angle = 0.0;
    }
    else
      angle = std::atan2( candidateEndY - candidateStartY, candidateEndX - candidateStartX );

    labelWidth = getLabelWidth( angle );
    labelHeight = getLabelHeight( angle );
    beta = angle + M_PI_2;

    if ( mLF->layer()->arrangement() == QgsPalLayerSettings::Line )
    {
      // find out whether the line direction for this candidate is from right to left
      bool isRightToLeft = ( angle > M_PI_2 || angle <= -M_PI_2 );
      // meaning of above/below may be reversed if using map orientation and the line has right-to-left direction
      bool reversed = ( ( flags & QgsLabeling::LinePlacementFlag::MapOrientation ) ? isRightToLeft : false );
      bool aboveLine = ( !reversed && ( flags & QgsLabeling::LinePlacementFlag::AboveLine ) ) || ( reversed && ( flags & QgsLabeling::LinePlacementFlag::BelowLine ) );
      bool belowLine = ( !reversed && ( flags & QgsLabeling::LinePlacementFlag::BelowLine ) ) || ( reversed && ( flags & QgsLabeling::LinePlacementFlag::AboveLine ) );

      if ( aboveLine )
      {
        if ( !mLF->permissibleZonePrepared() || GeomFunction::containsCandidate( mLF->permissibleZonePrepared(), candidateStartX + std::cos( beta ) *distanceLineToLabel, candidateStartY + std::sin( beta ) *distanceLineToLabel, labelWidth, labelHeight, angle ) )
        {
          const double candidateCost = cost + ( !reversed ? 0 : 0.001 ); // no extra cost for above line placements
          lPos.emplace_back( std::make_unique< LabelPosition >( i, candidateStartX + std::cos( beta ) *distanceLineToLabel, candidateStartY + std::sin( beta ) *distanceLineToLabel, labelWidth, labelHeight, angle, candidateCost, this, isRightToLeft, LabelPosition::QuadrantOver ) ); // Line
        }
      }
      if ( belowLine )
      {
        if ( !mLF->permissibleZonePrepared() || GeomFunction::containsCandidate( mLF->permissibleZonePrepared(), candidateStartX - std::cos( beta ) * ( distanceLineToLabel + labelHeight ), candidateStartY - std::sin( beta ) * ( distanceLineToLabel + labelHeight ), labelWidth, labelHeight, angle ) )
        {
          const double candidateCost = cost + ( !reversed ? 0.001 : 0 );
          lPos.emplace_back( std::make_unique< LabelPosition >( i, candidateStartX - std::cos( beta ) * ( distanceLineToLabel + labelHeight ), candidateStartY - std::sin( beta ) * ( distanceLineToLabel + labelHeight ), labelWidth, labelHeight, angle, candidateCost, this, isRightToLeft, LabelPosition::QuadrantOver ) ); // Line
        }
      }
      if ( flags & QgsLabeling::LinePlacementFlag::OnLine )
      {
        if ( !mLF->permissibleZonePrepared() || GeomFunction::containsCandidate( mLF->permissibleZonePrepared(), candidateStartX - labelHeight * std::cos( beta ) / 2, candidateStartY - labelHeight * std::sin( beta ) / 2, labelWidth, labelHeight, angle ) )
        {
          const double candidateCost = cost + 0.002;
          lPos.emplace_back( std::make_unique< LabelPosition >( i, candidateStartX - labelHeight * std::cos( beta ) / 2, candidateStartY - labelHeight * std::sin( beta ) / 2, labelWidth, labelHeight, angle, candidateCost, this, isRightToLeft, LabelPosition::QuadrantOver ) ); // Line
        }
      }
    }
    else if ( mLF->layer()->arrangement() == QgsPalLayerSettings::Horizontal )
    {
      lPos.emplace_back( std::make_unique< LabelPosition >( i, candidateStartX - labelWidth / 2, candidateStartY - labelHeight / 2, labelWidth, labelHeight, 0, cost, this, false, LabelPosition::QuadrantOver ) ); // Line
    }
    else
    {
      // an invalid arrangement?
    }

    currentDistanceAlongLine += lineStepDistance;

    i++;

    if ( lineStepDistance < 0 )
      break;
  }

  return lPos.size();
}

std::unique_ptr< LabelPosition > FeaturePart::curvedPlacementAtOffset( PointSet *mapShape, const std::vector< double> &pathDistances, QgsTextRendererUtils::LabelLineDirection direction, const double offsetAlongLine, bool &labeledLineSegmentIsRightToLeft, bool applyAngleConstraints, bool uprightOnly )
{
  const QgsPrecalculatedTextMetrics *metrics = qgis::down_cast< QgsTextLabelFeature * >( mLF )->textMetrics();
  Q_ASSERT( metrics );

  const double maximumCharacterAngleInside = applyAngleConstraints ? std::fabs( qgis::down_cast< QgsTextLabelFeature *>( mLF )->maximumCharacterAngleInside() ) : -1;
  const double maximumCharacterAngleOutside = applyAngleConstraints ? std::fabs( qgis::down_cast< QgsTextLabelFeature *>( mLF )->maximumCharacterAngleOutside() ) : -1;

  std::unique_ptr< QgsTextRendererUtils::CurvePlacementProperties > placement(
    QgsTextRendererUtils::generateCurvedTextPlacement( *metrics, mapShape->x.data(), mapShape->y.data(), mapShape->nbPoints, pathDistances, offsetAlongLine, direction, maximumCharacterAngleInside, maximumCharacterAngleOutside, uprightOnly )
  );

  labeledLineSegmentIsRightToLeft = !uprightOnly ? placement->labeledLineSegmentIsRightToLeft : placement->flippedCharacterPlacementToGetUprightLabels;

  if ( placement->graphemePlacement.empty() )
    return nullptr;

  auto it = placement->graphemePlacement.constBegin();
  std::unique_ptr< LabelPosition > firstPosition = std::make_unique< LabelPosition >( 0, it->x, it->y, it->width, it->height, it->angle, 0.0001, this, false, LabelPosition::QuadrantOver );
  firstPosition->setUpsideDownCharCount( placement->upsideDownCharCount );
  firstPosition->setPartId( it->graphemeIndex );
  LabelPosition *previousPosition = firstPosition.get();
  it++;
  while ( it != placement->graphemePlacement.constEnd() )
  {
    std::unique_ptr< LabelPosition > position = std::make_unique< LabelPosition >( 0, it->x, it->y, it->width, it->height, it->angle, 0.0001, this, false, LabelPosition::QuadrantOver );
    position->setPartId( it->graphemeIndex );

    LabelPosition *nextPosition = position.get();
    previousPosition->setNextPart( std::move( position ) );
    previousPosition = nextPosition;
    it++;
  }

  return firstPosition;
}

std::size_t FeaturePart::createCurvedCandidatesAlongLine( std::vector< std::unique_ptr< LabelPosition > > &lPos, PointSet *mapShape, bool allowOverrun, Pal *pal )
{
  const QgsPrecalculatedTextMetrics *li = qgis::down_cast< QgsTextLabelFeature *>( mLF )->textMetrics();
  Q_ASSERT( li );

  // label info must be present
  if ( !li )
    return 0;

  const int characterCount = li->count();
  if ( characterCount == 0 )
    return 0;

  // TODO - we may need an explicit penalty for overhanging labels. Currently, they are penalized just because they
  // are further from the line center, so non-overhanding placements are picked where possible.

  double totalCharacterWidth = 0;
  for ( int i = 0; i < characterCount; ++i )
    totalCharacterWidth += li->characterWidth( i );

  std::unique_ptr< PointSet > expanded;
  double shapeLength = mapShape->length();

  if ( totalRepeats() > 1 )
    allowOverrun = false;

  // unless in strict mode, label overrun should NEVER exceed the label length (or labels would sit off in space).
  // in fact, let's require that a minimum of 5% of the label text has to sit on the feature,
  // as we don't want a label sitting right at the start or end corner of a line
  double overrun = 0;
  switch ( mLF->lineAnchorType() )
  {
    case QgsLabelLineSettings::AnchorType::HintOnly:
      overrun = std::min( mLF->overrunDistance(), totalCharacterWidth * 0.95 );
      break;
    case QgsLabelLineSettings::AnchorType::Strict:
      // in strict mode, we force sufficient overrun to ensure label will always "fit", even if it's placed
      // so that the label start sits right on the end of the line OR the label end sits right on the start of the line
      overrun = std::max( mLF->overrunDistance(), totalCharacterWidth * 1.05 );
      break;
  }

  if ( totalCharacterWidth > shapeLength )
  {
    if ( !allowOverrun || shapeLength < totalCharacterWidth - 2 * overrun )
    {
      // label doesn't fit on this line, don't waste time trying to make candidates
      return 0;
    }
  }

  // calculate the anchor point for the original line shape as a GEOS point.
  // this must be done BEFORE we account for overrun by extending the shape!
  const geos::unique_ptr originalPoint = mapShape->interpolatePoint( shapeLength * mLF->lineAnchorPercent() );

  if ( allowOverrun && overrun > 0 )
  {
    // expand out line on either side to fit label
    expanded = mapShape->clone();
    expanded->extendLineByDistance( overrun, overrun, mLF->overrunSmoothDistance() );
    mapShape = expanded.get();
    shapeLength += 2 * overrun;
  }

  QgsLabeling::LinePlacementFlags flags = mLF->arrangementFlags();
  if ( flags == 0 )
    flags = QgsLabeling::LinePlacementFlag::OnLine; // default flag
  const bool hasAboveBelowLinePlacement = flags & QgsLabeling::LinePlacementFlag::AboveLine || flags & QgsLabeling::LinePlacementFlag::BelowLine;
  const double offsetDistance = mLF->distLabel() + li->characterHeight() / 2;
  std::unique_ptr< PointSet > mapShapeOffsetPositive;
  std::unique_ptr< PointSet > mapShapeOffsetNegative;
  if ( hasAboveBelowLinePlacement && !qgsDoubleNear( offsetDistance, 0 ) )
  {
    // create offseted map shapes to be used for above and below line placements
    if ( ( flags & QgsLabeling::LinePlacementFlag::MapOrientation ) || ( flags & QgsLabeling::LinePlacementFlag::AboveLine ) )
      mapShapeOffsetPositive = mapShape->clone();
    if ( ( flags & QgsLabeling::LinePlacementFlag::MapOrientation ) || ( flags & QgsLabeling::LinePlacementFlag::BelowLine ) )
      mapShapeOffsetNegative = mapShape->clone();
    if ( offsetDistance >= 0.0 || !( flags & QgsLabeling::LinePlacementFlag::MapOrientation ) )
    {
      if ( mapShapeOffsetPositive )
        mapShapeOffsetPositive->offsetCurveByDistance( offsetDistance );
      if ( mapShapeOffsetNegative )
        mapShapeOffsetNegative->offsetCurveByDistance( offsetDistance * -1 );
    }
    else
    {
      // In case of a negative offset distance, above line placement switch to below line and vice versa
      if ( flags & QgsLabeling::LinePlacementFlag::AboveLine
           && !( flags & QgsLabeling::LinePlacementFlag::BelowLine ) )
      {
        flags &= ~QgsLabeling::LinePlacementFlag::AboveLine;
        flags |= QgsLabeling::LinePlacementFlag::BelowLine;
      }
      else if ( flags & QgsLabeling::LinePlacementFlag::BelowLine
                && !( flags & QgsLabeling::LinePlacementFlag::AboveLine ) )
      {
        flags &= ~QgsLabeling::LinePlacementFlag::BelowLine;
        flags |= QgsLabeling::LinePlacementFlag::AboveLine;
      }
      if ( mapShapeOffsetPositive )
        mapShapeOffsetPositive->offsetCurveByDistance( offsetDistance * -1 );
      if ( mapShapeOffsetNegative )
        mapShapeOffsetNegative->offsetCurveByDistance( offsetDistance );
    }
  }

  const QgsLabelLineSettings::AnchorTextPoint textPoint = mLF->lineAnchorTextPoint();

  std::vector< std::unique_ptr< LabelPosition >> positions;
  std::unique_ptr< LabelPosition > backupPlacement;
  for ( PathOffset offset : { PositiveOffset, NoOffset, NegativeOffset } )
  {
    PointSet *currentMapShape = nullptr;
    if ( offset == PositiveOffset && hasAboveBelowLinePlacement )
    {
      currentMapShape = mapShapeOffsetPositive.get();
    }
    if ( offset == NoOffset && flags & QgsLabeling::LinePlacementFlag::OnLine )
    {
      currentMapShape = mapShape;
    }
    if ( offset == NegativeOffset && hasAboveBelowLinePlacement )
    {
      currentMapShape = mapShapeOffsetNegative.get();
    }
    if ( !currentMapShape )
      continue;

    // distance calculation
    const auto [ pathDistances, totalDistance ] = currentMapShape->edgeDistances();
    if ( qgsDoubleNear( totalDistance, 0.0 ) )
      continue;

    double lineAnchorPoint = 0;
    if ( originalPoint && offset != NoOffset )
    {
      // the actual anchor point for the offset curves is the closest point on those offset curves
      // to the anchor point on the original line. This avoids anchor points which differ greatly
      // on the positive/negative offset lines due to line curvature.
      lineAnchorPoint = currentMapShape->lineLocatePoint( originalPoint.get() );
    }
    else
    {
      lineAnchorPoint = totalDistance * mLF->lineAnchorPercent();
      if ( offset == NegativeOffset )
        lineAnchorPoint = totalDistance - lineAnchorPoint;
    }

    if ( pal->isCanceled() )
      return 0;

    const std::size_t candidateTargetCount = maximumLineCandidates();
    double delta = std::max( li->characterHeight() / 6, totalDistance / candidateTargetCount );

    // generate curved labels
    double distanceAlongLineToStartCandidate = 0;
    bool singleCandidateOnly = false;
    switch ( mLF->lineAnchorType() )
    {
      case QgsLabelLineSettings::AnchorType::HintOnly:
        break;

      case QgsLabelLineSettings::AnchorType::Strict:
        switch ( textPoint )
        {
          case QgsLabelLineSettings::AnchorTextPoint::StartOfText:
            distanceAlongLineToStartCandidate = std::clamp( lineAnchorPoint, 0.0, totalDistance * 0.999 );
            break;
          case QgsLabelLineSettings::AnchorTextPoint::CenterOfText:
            distanceAlongLineToStartCandidate = std::clamp( lineAnchorPoint - getLabelWidth() / 2, 0.0, totalDistance * 0.999 - getLabelWidth() / 2 );
            break;
          case QgsLabelLineSettings::AnchorTextPoint::EndOfText:
            distanceAlongLineToStartCandidate = std::clamp( lineAnchorPoint - getLabelWidth(), 0.0, totalDistance * 0.999 - getLabelWidth() ) ;
            break;
          case QgsLabelLineSettings::AnchorTextPoint::FollowPlacement:
            // not possible here
            break;
        }
        singleCandidateOnly = true;
        break;
    }

    bool hasTestedFirstPlacement = false;
    for ( ; distanceAlongLineToStartCandidate <= totalDistance; distanceAlongLineToStartCandidate += delta )
    {
      if ( singleCandidateOnly && hasTestedFirstPlacement )
        break;

      if ( pal->isCanceled() )
        return 0;

      hasTestedFirstPlacement = true;
      // placements may need to be reversed if using map orientation and the line has right-to-left direction
      bool labeledLineSegmentIsRightToLeft = false;
      const QgsTextRendererUtils::LabelLineDirection direction = ( flags & QgsLabeling::LinePlacementFlag::MapOrientation ) ? QgsTextRendererUtils::RespectPainterOrientation : QgsTextRendererUtils::FollowLineDirection;
      std::unique_ptr< LabelPosition > labelPosition = curvedPlacementAtOffset( currentMapShape, pathDistances, direction, distanceAlongLineToStartCandidate, labeledLineSegmentIsRightToLeft, !singleCandidateOnly,
          onlyShowUprightLabels() && ( !singleCandidateOnly || !( flags & QgsLabeling::LinePlacementFlag::MapOrientation ) ) );

      if ( !labelPosition )
      {
        continue;
      }

      bool isBackupPlacementOnly = false;
      if ( flags & QgsLabeling::LinePlacementFlag::MapOrientation )
      {
        if ( ( offset != NoOffset ) && !labeledLineSegmentIsRightToLeft && !( flags & QgsLabeling::LinePlacementFlag::AboveLine ) )
        {
          if ( singleCandidateOnly && offset == PositiveOffset )
            isBackupPlacementOnly = true;
          else
            continue;
        }
        if ( ( offset != NoOffset ) && labeledLineSegmentIsRightToLeft && !( flags & QgsLabeling::LinePlacementFlag::BelowLine ) )
        {
          if ( singleCandidateOnly && offset == PositiveOffset )
            isBackupPlacementOnly = true;
          else
            continue;
        }
      }

      backupPlacement.reset();

      // evaluate cost
      const double angleDiff = labelPosition->angleDifferential();
      const double angleDiffAvg = characterCount > 1 ? ( angleDiff / ( characterCount - 1 ) ) : 0; // <0, pi> but pi/8 is much already

      // if anchor placement is towards start or end of line, we need to slightly tweak the costs to ensure that the
      // anchor weighting is sufficient to push labels towards start/end
      const bool anchorIsFlexiblePlacement = !singleCandidateOnly && mLF->lineAnchorPercent() > 0.1 && mLF->lineAnchorPercent() < 0.9;
      double cost = angleDiffAvg / 100; // <0, 0.031 > but usually <0, 0.003 >
      if ( cost < 0.0001 )
        cost = 0.0001;

      // penalize positions which are further from the line's anchor point
      double labelTextAnchor = 0;
      switch ( textPoint )
      {
        case QgsLabelLineSettings::AnchorTextPoint::StartOfText:
          labelTextAnchor = distanceAlongLineToStartCandidate;
          break;
        case QgsLabelLineSettings::AnchorTextPoint::CenterOfText:
          labelTextAnchor = distanceAlongLineToStartCandidate + getLabelWidth() / 2;
          break;
        case QgsLabelLineSettings::AnchorTextPoint::EndOfText:
          labelTextAnchor = distanceAlongLineToStartCandidate + getLabelWidth();
          break;
        case QgsLabelLineSettings::AnchorTextPoint::FollowPlacement:
          // not possible here
          break;
      }
      double costCenter = std::fabs( lineAnchorPoint - labelTextAnchor ) / totalDistance; // <0, 0.5>
      cost += costCenter / ( anchorIsFlexiblePlacement ? 100 : 10 );  // < 0, 0.005 >, or <0, 0.05> if preferring placement close to start/end of line

      const bool isBelow = ( offset != NoOffset ) && labeledLineSegmentIsRightToLeft;
      if ( isBelow )
      {
        // add additional cost for on line placement
        cost += 0.001;
      }
      else if ( offset == NoOffset )
      {
        // add additional cost for below line placement
        cost += 0.002;
      }

      labelPosition->setCost( cost );

      std::unique_ptr< LabelPosition > p = std::make_unique< LabelPosition >( *labelPosition );
      if ( p && mLF->permissibleZonePrepared() )
      {
        bool within = true;
        LabelPosition *currentPos = p.get();
        while ( within && currentPos )
        {
          within = GeomFunction::containsCandidate( mLF->permissibleZonePrepared(), currentPos->getX(), currentPos->getY(), currentPos->getWidth(), currentPos->getHeight(), currentPos->getAlpha() );
          currentPos = currentPos->nextPart();
        }
        if ( !within )
        {
          p.reset();
        }
      }

      if ( p )
      {
        if ( isBackupPlacementOnly )
          backupPlacement = std::move( p );
        else
          positions.emplace_back( std::move( p ) );
      }
    }
  }

  for ( std::unique_ptr< LabelPosition > &pos : positions )
  {
    lPos.emplace_back( std::move( pos ) );
  }

  if ( backupPlacement )
    lPos.emplace_back( std::move( backupPlacement ) );

  return positions.size();
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

std::size_t FeaturePart::createCandidatesForPolygon( std::vector< std::unique_ptr< LabelPosition > > &lPos, PointSet *mapShape, Pal *pal )
{
  double labelWidth = getLabelWidth();
  double labelHeight = getLabelHeight();

  const std::size_t maxPolygonCandidates = mLF->layer()->maximumPolygonLabelCandidates();
  const std::size_t targetPolygonCandidates = maxPolygonCandidates > 0 ? std::min( maxPolygonCandidates,  static_cast< std::size_t>( std::ceil( mLF->layer()->mPal->maximumPolygonCandidatesPerMapUnitSquared() * area() ) ) )
      : 0;

  const double totalArea = area();

  mapShape->parent = nullptr;

  if ( pal->isCanceled() )
    return 0;

  QLinkedList<PointSet *> shapes_final = splitPolygons( mapShape, labelWidth, labelHeight );
#if 0
  QgsDebugMsg( QStringLiteral( "PAL split polygons resulted in:" ) );
  for ( PointSet *ps : shapes_final )
  {
    QgsDebugMsg( ps->toWkt() );
  }
#endif

  std::size_t nbp = 0;

  if ( !shapes_final.isEmpty() )
  {
    int id = 0; // ids for candidates
    double dlx, dly; // delta from label center and bottom-left corner
    double alpha = 0.0; // rotation for the label
    double px, py;

    double beta;
    double diago = std::sqrt( labelWidth * labelWidth / 4.0 + labelHeight * labelHeight / 4 );
    double rx, ry;
    std::vector< OrientedConvexHullBoundingBox > boxes;
    boxes.reserve( shapes_final.size() );

    // Compute bounding box for each finalShape
    while ( !shapes_final.isEmpty() )
    {
      PointSet *shape = shapes_final.takeFirst();
      bool ok = false;
      OrientedConvexHullBoundingBox box = shape->computeConvexHullOrientedBoundingBox( ok );
      if ( ok )
        boxes.emplace_back( box );

      if ( shape->parent )
        delete shape;
    }

    if ( pal->isCanceled() )
      return 0;

    double densityX = 1.0 / std::sqrt( mLF->layer()->mPal->maximumPolygonCandidatesPerMapUnitSquared() );
    double densityY = densityX;
    int numTry = 0;

    //fit in polygon only mode slows down calculation a lot, so if it's enabled
    //then use a smaller limit for number of iterations
    int maxTry = mLF->permissibleZonePrepared() ? 7 : 10;

    std::size_t numberCandidatesGenerated = 0;

    do
    {
      for ( OrientedConvexHullBoundingBox &box : boxes )
      {
        // there is two possibilities here:
        // 1. no maximum candidates for polygon setting is in effect (i.e. maxPolygonCandidates == 0). In that case,
        // we base our dx/dy on the current maximumPolygonCandidatesPerMapUnitSquared value. That should give us the desired
        // density of candidates straight up. Easy!
        // 2. a maximum candidate setting IS in effect. In that case, we want to generate a good initial estimate for dx/dy
        // which gives us a good spatial coverage of the polygon while roughly matching the desired maximum number of candidates.
        // If dx/dy is too small, then too many candidates will be generated, which is both slow AND results in poor coverage of the
        // polygon (after culling candidates to the max number, only those clustered around the polygon's pole of inaccessibility
        // will remain).
        double dx = densityX;
        double dy = densityY;
        if ( numTry == 0 && maxPolygonCandidates > 0 )
        {
          // scale maxPolygonCandidates for just this convex hull
          const double boxArea = box.width * box.length;
          double maxThisBox = targetPolygonCandidates * boxArea / totalArea;
          dx = std::max( dx, std::sqrt( boxArea / maxThisBox ) * 0.8 );
          dy = dx;
        }

        if ( pal->isCanceled() )
          return numberCandidatesGenerated;

        if ( ( box.length * box.width ) > ( xmax - xmin ) * ( ymax - ymin ) * 5 )
        {
          // Very Large BBOX (should never occur)
          continue;
        }

        if ( mLF->layer()->arrangement() == QgsPalLayerSettings::Horizontal && mLF->permissibleZonePrepared() )
        {
          //check width/height of bbox is sufficient for label
          if ( mLF->permissibleZone().boundingBox().width() < labelWidth ||
               mLF->permissibleZone().boundingBox().height() < labelHeight )
          {
            //no way label can fit in this box, skip it
            continue;
          }
        }

        bool enoughPlace = false;
        if ( mLF->layer()->arrangement() == QgsPalLayerSettings::Free )
        {
          enoughPlace = true;
          px = ( box.x[0] + box.x[2] ) / 2 - labelWidth;
          py = ( box.y[0] + box.y[2] ) / 2 - labelHeight;
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
        else if ( box.length > 1.5 * labelWidth && box.width > 1.5 * labelWidth )
        {
          if ( box.alpha <= M_PI_4 )
          {
            alpha = box.alpha;
          }
          else
          {
            alpha = box.alpha - M_PI_2;
          }
        }
        else if ( box.length > box.width )
        {
          alpha = box.alpha - M_PI_2;
        }
        else
        {
          alpha = box.alpha;
        }

        beta  = std::atan2( labelHeight, labelWidth ) + alpha;


        //alpha = box->alpha;

        // delta from label center and down-left corner
        dlx = std::cos( beta ) * diago;
        dly = std::sin( beta ) * diago;

        double px0 = box.width / 2.0;
        double py0 = box.length / 2.0;

        px0 -= std::ceil( px0 / dx ) * dx;
        py0 -= std::ceil( py0 / dy ) * dy;

        for ( px = px0; px <= box.width; px += dx )
        {
          if ( pal->isCanceled() )
            break;

          for ( py = py0; py <= box.length; py += dy )
          {

            rx = std::cos( box.alpha ) * px + std::cos( box.alpha - M_PI_2 ) * py;
            ry = std::sin( box.alpha ) * px + std::sin( box.alpha - M_PI_2 ) * py;

            rx += box.x[0];
            ry += box.y[0];

            if ( mLF->permissibleZonePrepared() )
            {
              if ( GeomFunction::containsCandidate( mLF->permissibleZonePrepared(), rx - dlx, ry - dly, labelWidth, labelHeight, alpha ) )
              {
                // cost is set to minimal value, evaluated later
                lPos.emplace_back( std::make_unique< LabelPosition >( id++, rx - dlx, ry - dly, labelWidth, labelHeight, alpha, 0.0001, this, false, LabelPosition::QuadrantOver ) );
                numberCandidatesGenerated++;
              }
            }
            else
            {
              // TODO - this should be an intersection test, not just a contains test of the candidate centroid
              // because in some cases we would want to allow candidates which mostly overlap the polygon even though
              // their centroid doesn't overlap (e.g. a "U" shaped polygon)
              // but the bugs noted in CostCalculator currently prevent this
              if ( mapShape->containsPoint( rx, ry ) )
              {
                std::unique_ptr< LabelPosition > potentialCandidate = std::make_unique< LabelPosition >( id++, rx - dlx, ry - dly, labelWidth, labelHeight, alpha, 0.0001, this, false, LabelPosition::QuadrantOver );
                // cost is set to minimal value, evaluated later
                lPos.emplace_back( std::move( potentialCandidate ) );
                numberCandidatesGenerated++;
              }
            }
          }
        }
      } // forall box

      nbp = numberCandidatesGenerated;
      if ( maxPolygonCandidates > 0 && nbp < targetPolygonCandidates )
      {
        densityX /= 2;
        densityY /= 2;
        numTry++;
      }
      else
      {
        break;
      }
    }
    while ( numTry < maxTry );

    nbp = numberCandidatesGenerated;
  }
  else
  {
    nbp = 0;
  }

  return nbp;
}

std::size_t FeaturePart::createCandidatesOutsidePolygon( std::vector<std::unique_ptr<LabelPosition> > &lPos, Pal *pal )
{
  // calculate distance between horizontal lines
  const std::size_t maxPolygonCandidates = mLF->layer()->maximumPolygonLabelCandidates();
  std::size_t candidatesCreated = 0;

  double labelWidth = getLabelWidth();
  double labelHeight = getLabelHeight();
  double distanceToLabel = getLabelDistance();
  const QgsMargins &visualMargin = mLF->visualMargin();

  /*
   * From Rylov & Reimer (2016) "A practical algorithm for the external annotation of area features":
   *
   * The list of rules adapted to the
   * needs of externally labelling areal features is as follows:
   * R1. Labels should be placed horizontally.
   * R2. Label should be placed entirely outside at some
   * distance from the area feature.
   * R3. Name should not cross the boundary of its area
   * feature.
   * R4. The name should be placed in way that takes into
   * account the shape of the feature by achieving a
   * balance between the feature and its name, emphasizing their relationship.
   * R5. The lettering to the right and slightly above the
   * symbol is prioritized.
   *
   * In the following subsections we utilize four of the five rules
   * for two subtasks of label placement, namely, for candidate
   * positions generation (R1, R2, and R3) and for measuring their
   * ‘goodness’ (R4). The rule R5 is applicable only in the case when
   * the area of a polygonal feature is small and the feature can be
   * treated and labelled as a point-feature
   */

  /*
   * QGIS approach (cite Dawson (2020) if you want ;) )
   *
   * We differ from the horizontal sweep line approach described by Rylov & Reimer and instead
   * rely on just generating a set of points at regular intervals along the boundary of the polygon (exterior ring).
   *
   * In practice, this generates similar results as Rylov & Reimer, but has the additional benefits that:
   * 1. It avoids the need to calculate intersections between the sweep line and the polygon
   * 2. For horizontal or near horizontal segments, Rylov & Reimer propose generating evenly spaced points along
   * these segments-- i.e. the same approach as we do for the whole polygon
   * 3. It's easier to determine in advance exactly how many candidate positions we'll be generating, and accordingly
   * we can easily pick the distance between points along the exterior ring so that the number of positions generated
   * matches our target number (targetPolygonCandidates)
   */

  // TO consider -- for very small polygons (wrt label size), treat them just like a point feature?

  double cx, cy;
  getCentroid( cx, cy, false );

  GEOSContextHandle_t ctxt = QgsGeos::getGEOSHandler();

  // be a bit sneaky and only buffer out 50% here, and then do the remaining 50% when we make the label candidate itself.
  // this avoids candidates being created immediately over the buffered ring and always intersecting with it...
  geos::unique_ptr buffer( GEOSBuffer_r( ctxt, geos(), distanceToLabel * 0.5, 1 ) );
  std::unique_ptr< QgsAbstractGeometry> gg( QgsGeos::fromGeos( buffer.get() ) );

  geos::prepared_unique_ptr preparedBuffer( GEOSPrepare_r( ctxt, buffer.get() ) );

  const QgsPolygon *poly = qgsgeometry_cast< const QgsPolygon * >( gg.get() );
  if ( !poly )
    return candidatesCreated;

  const QgsLineString *ring = qgsgeometry_cast< const QgsLineString *>( poly->exteriorRing() );
  if ( !ring )
    return candidatesCreated;

  // we cheat here -- we don't use the polygon area when calculating the number of candidates, and rather use the perimeter (because that's more relevant,
  // i.e a loooooong skinny polygon with small area should still generate a large number of candidates)
  const double ringLength = ring->length();
  const double circleArea = std::pow( ringLength, 2 ) / ( 4 * M_PI );
  const std::size_t candidatesForArea = static_cast< std::size_t>( std::ceil( mLF->layer()->mPal->maximumPolygonCandidatesPerMapUnitSquared() * circleArea ) );
  const std::size_t targetPolygonCandidates = std::max( static_cast< std::size_t >( 16 ), maxPolygonCandidates > 0 ? std::min( maxPolygonCandidates,  candidatesForArea ) : candidatesForArea );

  // assume each position generates one candidate
  const double delta = ringLength / targetPolygonCandidates;
  geos::unique_ptr geosPoint;

  const double maxDistCentroidToLabelX = std::max( xmax - cx, cx - xmin ) + distanceToLabel;
  const double maxDistCentroidToLabelY = std::max( ymax - cy, cy - ymin ) + distanceToLabel;
  const double estimateOfMaxPossibleDistanceCentroidToLabel = std::sqrt( maxDistCentroidToLabelX * maxDistCentroidToLabelX + maxDistCentroidToLabelY * maxDistCentroidToLabelY );

  // Satisfy R1: Labels should be placed horizontally.
  const double labelAngle = 0;

  std::size_t i = lPos.size();
  auto addCandidate = [&]( double x, double y, QgsPalLayerSettings::PredefinedPointPosition position )
  {
    double labelX = 0;
    double labelY = 0;
    LabelPosition::Quadrant quadrant = LabelPosition::QuadrantAboveLeft;

    // Satisfy R2: Label should be placed entirely outside at some distance from the area feature.
    createCandidateAtOrderedPositionOverPoint( labelX, labelY, quadrant, x, y, labelWidth, labelHeight, position, distanceToLabel * 0.5, visualMargin, 0, 0, labelAngle );

    std::unique_ptr< LabelPosition > candidate = std::make_unique< LabelPosition >( i, labelX, labelY, labelWidth, labelHeight, labelAngle, 0, this, false, quadrant );
    if ( candidate->intersects( preparedBuffer.get() ) )
    {
      // satisfy R3. Name should not cross the boundary of its area feature.

      // actually, we use the buffered geometry here, because a label shouldn't be closer to the polygon then the minimum distance value
      return;
    }

    // cost candidates by their distance to the feature's centroid (following Rylov & Reimer)

    // Satisfy R4. The name should be placed in way that takes into
    // account the shape of the feature by achieving a
    // balance between the feature and its name, emphasizing their relationship.


    // here we deviate a little from R&R, and instead of just calculating the centroid distance
    // to centroid of label, we calculate the distance from the centroid to the nearest point on the label

    const double centroidDistance = candidate->getDistanceToPoint( cx, cy );
    const double centroidCost = centroidDistance / estimateOfMaxPossibleDistanceCentroidToLabel;
    candidate->setCost( centroidCost );

    lPos.emplace_back( std::move( candidate ) );
    candidatesCreated++;
    ++i;
  };

  ring->visitPointsByRegularDistance( delta, [&]( double x, double y, double, double,
                                      double startSegmentX, double startSegmentY, double, double,
                                      double endSegmentX, double endSegmentY, double, double )
  {
    // get normal angle for segment
    float angle = atan2( static_cast< float >( endSegmentY - startSegmentY ), static_cast< float >( endSegmentX - startSegmentX ) ) * 180 / M_PI;
    if ( angle < 0 )
      angle += 360;

    // adapted fom Rylov & Reimer figure 9
    if ( angle >= 0 && angle <= 5 )
    {
      addCandidate( x, y, QgsPalLayerSettings::TopMiddle );
      addCandidate( x, y, QgsPalLayerSettings::TopLeft );
    }
    else if ( angle <= 85 )
    {
      addCandidate( x, y, QgsPalLayerSettings::TopLeft );
    }
    else if ( angle <= 90 )
    {
      addCandidate( x, y, QgsPalLayerSettings::TopLeft );
      addCandidate( x, y, QgsPalLayerSettings::MiddleLeft );
    }

    else if ( angle <= 95 )
    {
      addCandidate( x, y, QgsPalLayerSettings::MiddleLeft );
      addCandidate( x, y, QgsPalLayerSettings::BottomLeft );
    }
    else if ( angle <= 175 )
    {
      addCandidate( x, y, QgsPalLayerSettings::BottomLeft );
    }
    else if ( angle <= 180 )
    {
      addCandidate( x, y, QgsPalLayerSettings::BottomLeft );
      addCandidate( x, y, QgsPalLayerSettings::BottomMiddle );
    }

    else if ( angle <= 185 )
    {
      addCandidate( x, y, QgsPalLayerSettings::BottomMiddle );
      addCandidate( x, y, QgsPalLayerSettings::BottomRight );
    }
    else if ( angle <= 265 )
    {
      addCandidate( x, y, QgsPalLayerSettings::BottomRight );
    }
    else if ( angle <= 270 )
    {
      addCandidate( x, y, QgsPalLayerSettings::BottomRight );
      addCandidate( x, y, QgsPalLayerSettings::MiddleRight );
    }
    else if ( angle <= 275 )
    {
      addCandidate( x, y, QgsPalLayerSettings::MiddleRight );
      addCandidate( x, y, QgsPalLayerSettings::TopRight );
    }
    else if ( angle <= 355 )
    {
      addCandidate( x, y, QgsPalLayerSettings::TopRight );
    }
    else
    {
      addCandidate( x, y, QgsPalLayerSettings::TopRight );
      addCandidate( x, y, QgsPalLayerSettings::TopMiddle );
    }

    return !pal->isCanceled();
  } );

  return candidatesCreated;
}

std::vector< std::unique_ptr< LabelPosition > > FeaturePart::createCandidates( Pal *pal )
{
  std::vector< std::unique_ptr< LabelPosition > > lPos;
  double angle = mLF->hasFixedAngle() ? mLF->fixedAngle() : 0.0;

  if ( mLF->hasFixedPosition() )
  {
    lPos.emplace_back( std::make_unique< LabelPosition> ( 0, mLF->fixedPosition().x(), mLF->fixedPosition().y(), getLabelWidth( angle ), getLabelHeight( angle ), angle, 0.0, this, false, LabelPosition::Quadrant::QuadrantOver ) );
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
        if ( mLF->layer()->arrangement() == QgsPalLayerSettings::Horizontal )
          createHorizontalCandidatesAlongLine( lPos, this, pal );
        else if ( mLF->layer()->isCurved() )
          createCurvedCandidatesAlongLine( lPos, this, true, pal );
        else
          createCandidatesAlongLine( lPos, this, true, pal );
        break;

      case GEOS_POLYGON:
      {
        const double labelWidth = getLabelWidth();
        const double labelHeight = getLabelHeight();

        const bool allowOutside = mLF->polygonPlacementFlags() & QgsLabeling::PolygonPlacementFlag::AllowPlacementOutsideOfPolygon;
        const bool allowInside =  mLF->polygonPlacementFlags() & QgsLabeling::PolygonPlacementFlag::AllowPlacementInsideOfPolygon;
        //check width/height of bbox is sufficient for label

        if ( ( allowOutside && !allowInside ) || ( mLF->layer()->arrangement() == QgsPalLayerSettings::OutsidePolygons ) )
        {
          // only allowed to place outside of polygon
          createCandidatesOutsidePolygon( lPos, pal );
        }
        else if ( allowOutside && ( std::fabs( xmax - xmin ) < labelWidth ||
                                    std::fabs( ymax - ymin ) < labelHeight ) )
        {
          //no way label can fit in this polygon -- shortcut and only place label outside
          createCandidatesOutsidePolygon( lPos, pal );
        }
        else
        {
          std::size_t created = 0;
          if ( allowInside )
          {
            switch ( mLF->layer()->arrangement() )
            {
              case QgsPalLayerSettings::AroundPoint:
              {
                double cx, cy;
                getCentroid( cx, cy, mLF->layer()->centroidInside() );
                if ( qgsDoubleNear( mLF->distLabel(), 0.0 ) )
                  created += createCandidateCenteredOverPoint( cx, cy, lPos, angle );
                created += createCandidatesAroundPoint( cx, cy, lPos, angle );
                break;
              }
              case QgsPalLayerSettings::OverPoint:
              {
                double cx, cy;
                getCentroid( cx, cy, mLF->layer()->centroidInside() );
                created += createCandidatesOverPoint( cx, cy, lPos, angle );
                break;
              }
              case QgsPalLayerSettings::Line:
                created += createCandidatesAlongLine( lPos, this, false, pal );
                break;
              case QgsPalLayerSettings::PerimeterCurved:
                created += createCurvedCandidatesAlongLine( lPos, this, false, pal );
                break;
              default:
                created += createCandidatesForPolygon( lPos, this, pal );
                break;
            }
          }

          if ( allowOutside )
          {
            // add fallback for labels outside the polygon
            createCandidatesOutsidePolygon( lPos, pal );

            if ( created > 0 )
            {
              // TODO (maybe) increase cost for outside placements (i.e. positions at indices >= created)?
              // From my initial testing this doesn't seem necessary
            }
          }
        }
      }
    }
  }

  return lPos;
}

void FeaturePart::addSizePenalty( std::vector< std::unique_ptr< LabelPosition > > &lPos, double bbx[4], double bby[4] )
{
  if ( !mGeos )
    createGeosGeom();

  GEOSContextHandle_t ctxt = QgsGeos::getGEOSHandler();
  int geomType = GEOSGeomTypeId_r( ctxt, mGeos );

  double sizeCost = 0;
  if ( geomType == GEOS_LINESTRING )
  {
    const double l = length();
    if ( l <= 0 )
      return; // failed to calculate length
    double bbox_length = std::max( bbx[2] - bbx[0], bby[2] - bby[0] );
    if ( l >= bbox_length / 4 )
      return; // the line is longer than quarter of height or width - don't penalize it

    sizeCost = 1 - ( l / ( bbox_length / 4 ) ); // < 0,1 >
  }
  else if ( geomType == GEOS_POLYGON )
  {
    const double a = area();
    if ( a <= 0 )
      return;
    double bbox_area = ( bbx[2] - bbx[0] ) * ( bby[2] - bby[0] );
    if ( a >= bbox_area / 16 )
      return; // covers more than 1/16 of our view - don't penalize it

    sizeCost = 1 - ( a / ( bbox_area / 16 ) ); // < 0, 1 >
  }
  else
    return; // no size penalty for points

// apply the penalty
  for ( std::unique_ptr< LabelPosition > &pos : lPos )
  {
    pos->setCost( pos->cost() + sizeCost / 100 );
  }
}

bool FeaturePart::isConnected( FeaturePart *p2 )
{
  if ( !nbPoints || !p2->nbPoints )
    return false;

  // here we only care if the lines start or end at the other line -- we don't want to test
  // touches as that is true for "T" type joins!
  const double x1first = x.front();
  const double x1last = x.back();
  const double x2first = p2->x.front();
  const double x2last = p2->x.back();
  const double y1first = y.front();
  const double y1last = y.back();
  const double y2first = p2->y.front();
  const double y2last = p2->y.back();

  const bool p2startTouches = ( qgsDoubleNear( x1first, x2first ) && qgsDoubleNear( y1first, y2first ) )
                              || ( qgsDoubleNear( x1last, x2first ) && qgsDoubleNear( y1last, y2first ) );

  const bool p2endTouches = ( qgsDoubleNear( x1first, x2last ) && qgsDoubleNear( y1first, y2last ) )
                            || ( qgsDoubleNear( x1last, x2last ) && qgsDoubleNear( y1last, y2last ) );
  // only one endpoint can touch, not both
  if ( ( !p2startTouches && !p2endTouches ) || ( p2startTouches && p2endTouches ) )
    return false;

  // now we know that we have one line endpoint touching only, but there's still a chance
  // that the other side of p2 may touch the original line NOT at the other endpoint
  // so we need to check that this point doesn't intersect
  const double p2otherX = p2startTouches ? x2last : x2first;
  const double p2otherY = p2startTouches ? y2last : y2first;

  GEOSContextHandle_t geosctxt = QgsGeos::getGEOSHandler();

  GEOSCoordSequence *coord = GEOSCoordSeq_create_r( geosctxt, 1, 2 );
#if GEOS_VERSION_MAJOR>3 || GEOS_VERSION_MINOR>=8
  GEOSCoordSeq_setXY_r( geosctxt, coord, 0, p2otherX, p2otherY );
#else
  GEOSCoordSeq_setX_r( geosctxt, coord, 0, p2otherX );
  GEOSCoordSeq_setY_r( geosctxt, coord, 0, p2otherY );
#endif

  geos::unique_ptr p2OtherEnd( GEOSGeom_createPoint_r( geosctxt, coord ) );
  try
  {
    return ( GEOSPreparedIntersects_r( geosctxt, preparedGeom(), p2OtherEnd.get() ) != 1 );
  }
  catch ( GEOSException &e )
  {
    qWarning( "GEOS exception: %s", e.what() );
    QgsMessageLog::logMessage( QObject::tr( "Exception: %1" ).arg( e.what() ), QObject::tr( "GEOS" ) );
    return false;
  }
}

bool FeaturePart::mergeWithFeaturePart( FeaturePart *other )
{
  if ( !mGeos )
    createGeosGeom();
  if ( !other->mGeos )
    other->createGeosGeom();

  GEOSContextHandle_t ctxt = QgsGeos::getGEOSHandler();
  try
  {
    GEOSGeometry *g1 = GEOSGeom_clone_r( ctxt, mGeos );
    GEOSGeometry *g2 = GEOSGeom_clone_r( ctxt, other->mGeos );
    GEOSGeometry *geoms[2] = { g1, g2 };
    geos::unique_ptr g( GEOSGeom_createCollection_r( ctxt, GEOS_MULTILINESTRING, geoms, 2 ) );
    geos::unique_ptr gTmp( GEOSLineMerge_r( ctxt, g.get() ) );

    if ( GEOSGeomTypeId_r( ctxt, gTmp.get() ) != GEOS_LINESTRING )
    {
      // sometimes it's not possible to merge lines (e.g. they don't touch at endpoints)
      return false;
    }
    invalidateGeos();

    // set up new geometry
    mGeos = gTmp.release();
    mOwnsGeom = true;

    deleteCoords();
    qDeleteAll( mHoles );
    mHoles.clear();
    extractCoords( mGeos );
    return true;
  }
  catch ( GEOSException &e )
  {
    qWarning( "GEOS exception: %s", e.what() );
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

bool FeaturePart::onlyShowUprightLabels() const
{
  bool uprightLabel = false;

  switch ( mLF->layer()->upsidedownLabels() )
  {
    case Layer::Upright:
      uprightLabel = true;
      break;
    case Layer::ShowDefined:
      // upright only dynamic labels
      if ( !hasFixedRotation() || ( !hasFixedPosition() && fixedAngle() == 0.0 ) )
      {
        uprightLabel = true;
      }
      break;
    case Layer::ShowAll:
      break;
    default:
      uprightLabel = true;
  }
  return uprightLabel;
}

