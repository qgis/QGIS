/***************************************************************************
    qgmaptooltrimextendfeature.cpp  -  map tool to trim or extend feature
    ---------------------
    begin                : October 2018
    copyright            : (C) 2018 by Loïc Bartoletti
    email                : loic dot bartoletti at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptooltrimextendfeature.h"
#include "moc_qgsmaptooltrimextendfeature.cpp"
#include "qgsmapcanvas.h"
#include "qgsvectorlayer.h"
#include "qgsgeometry.h"
#include "qgssnappingutils.h"
#include "qgsgeometryutils.h"
#include "qgsmapmouseevent.h"

class QgsRubberBand;

class FeatureFilter : public QgsPointLocator::MatchFilter
{
  public:
    FeatureFilter()
    {}

    bool acceptMatch( const QgsPointLocator::Match &match ) override
    {
      if ( mLayer )
        return match.layer() == mLayer && match.hasEdge();

      return match.hasEdge();
    }
    // We only want to modify the current layer. When geometries are overlapped, this makes it possible to snap onto the current layer.
    void setLayer( QgsVectorLayer *layer ) { mLayer = layer; }

  private:
    const QgsVectorLayer *mLayer = nullptr;
};

QgsMapToolTrimExtendFeature::QgsMapToolTrimExtendFeature( QgsMapCanvas *canvas )
  : QgsMapToolEdit( canvas )
{
  mToolName = tr( "Trim/Extend feature" );
  connect( mCanvas, &QgsMapCanvas::extentsChanged, this, &QgsMapToolTrimExtendFeature::extendLimit );
}

static bool getPoints( const QgsPointLocator::Match &match, QgsPoint &p1, QgsPoint &p2 )
{
  bool ret = false;
  const QgsFeatureId fid = match.featureId();
  const int vertex = match.vertexIndex();

  if ( auto *lLayer = match.layer() )
  {
    const QgsGeometry geom = lLayer->getGeometry( fid );

    if ( !( geom.isNull() || geom.isEmpty() ) )
    {
      p1 = geom.vertexAt( vertex );
      p2 = geom.vertexAt( vertex + 1 );
      ret = true;
    }
  }

  return ret;
}

void QgsMapToolTrimExtendFeature::canvasMoveEvent( QgsMapMouseEvent *e )
{
  mMapPoint = e->mapPoint();

  FeatureFilter filter;
  QgsPointLocator::Match match;

  switch ( mStep )
  {
    case StepLimit:

      match = mCanvas->snappingUtils()->snapToMap( mMapPoint, &filter, true );
      if ( match.isValid() && match.layer() )
      {
        mIs3DLayer = QgsWkbTypes::hasZ( match.layer()->wkbType() );

        QgsPointXY p1, p2;
        match.edgePoints( p1, p2 );
        mLimitLayer = match.layer();
        mRubberBandLimit.reset( createRubberBand( Qgis::GeometryType::Line ) );
        mRubberBandLimitExtend.reset( createRubberBand( Qgis::GeometryType::Line ) );
        mRubberBandLimitExtend->setLineStyle( Qt::DotLine );
        mRubberBandLimit->addPoint( p1 );
        mRubberBandLimit->addPoint( p2 );
        mRubberBandLimit->show();
        extendLimit();
      }
      else if ( mRubberBandLimit )
      {
        mRubberBandLimit->hide();
        mRubberBandLimitExtend.reset();
      }
      break;
    case StepExtend:

      QgsMapLayer *currentLayer = mCanvas->currentLayer();
      if ( !currentLayer )
      {
        mIsModified = false;
        break;
      }

      mVlayer = qobject_cast<QgsVectorLayer *>( currentLayer );
      if ( !mVlayer )
      {
        mIsModified = false;
        break;
      }

      if ( !mVlayer->isEditable() )
      {
        mIsModified = false;
        break;
      }

      filter.setLayer( mVlayer );
      match = mCanvas->snappingUtils()->snapToMap( mMapPoint, &filter, true );

      if ( match.isValid() )
      {
        if ( match.layer() != mVlayer )
          break;

        QgsPointXY p1, p2;
        match.edgePoints( p1, p2 );

        if ( !getPoints( match, pExtend1, pExtend2 ) )
          break;

        QgsCoordinateTransform transform( mLimitLayer->crs(), mVlayer->crs(), mCanvas->mapSettings().transformContext() );

        QgsPoint pLimit1Projected( pLimit1 );
        QgsPoint pLimit2Projected( pLimit2 );
        if ( !transform.isShortCircuited() )
        {
          const QgsPointXY transformedP1 = transform.transform( pLimit1 );
          const QgsPointXY transformedP2 = transform.transform( pLimit2 );
          pLimit1Projected.setX( transformedP1.x() );
          pLimit1Projected.setY( transformedP1.y() );
          pLimit2Projected.setX( transformedP2.x() );
          pLimit2Projected.setY( transformedP2.y() );
        }

        // No need to trim/extend if segments are continuous
        if ( ( ( pLimit1Projected == pExtend1 ) || ( pLimit1Projected == pExtend2 ) ) || ( ( pLimit2Projected == pExtend1 ) || ( pLimit2Projected == pExtend2 ) ) )
          break;

        mSegmentIntersects = QgsGeometryUtils::segmentIntersection( pLimit1Projected, pLimit2Projected, pExtend1, pExtend2, mIntersection, mIsIntersection, 1e-8, true );

        if ( mIs3DLayer && QgsWkbTypes::hasZ( match.layer()->wkbType() ) )
        {
          /* Z Interpolation */
          const QgsLineString line( pLimit1Projected, pLimit2Projected );

          mIntersection = QgsGeometryUtils::closestPoint( line, QgsPoint( mIntersection ) );
        }

        if ( mIsIntersection )
        {
          mRubberBandIntersection.reset( createRubberBand( Qgis::GeometryType::Point ) );
          mRubberBandIntersection->addPoint( toMapCoordinates( mVlayer, QgsPointXY( mIntersection ) ) );
          mRubberBandIntersection->show();

          mRubberBandExtend.reset( createRubberBand( match.layer()->geometryType() ) );

          mGeom = match.layer()->getGeometry( match.featureId() );
          int index = match.vertexIndex();

          if ( !mSegmentIntersects )
          {
            const QgsPoint ptInter( mIntersection.x(), mIntersection.y() );
            if ( pExtend2.distance( ptInter ) < pExtend1.distance( ptInter ) )
              index += 1;
          }
          // TRIM PART
          else if ( QgsGeometryUtils::leftOfLine( QgsPoint( mMapPoint ), pLimit1Projected, pLimit2Projected ) != QgsGeometryUtils::leftOfLine( pExtend1, pLimit1Projected, pLimit2Projected ) )
          {
            // Part where the mouse is (+) will be trimmed
            /*     |
             *     +
             *     |
             *   -----    -->   -----
             *     |              |
             *     |              |
             */

            /*     |              |
             *     |              |
             *   -----    -->   -----
             *     |
             *     +
             *     |
             */
            index += 1;
          }

          mIsModified = mGeom.moveVertex( mIntersection, index );

          if ( mIsModified )
          {
            mGeom.removeDuplicateNodes();
            // Densify by count to better display the intersection when layer crs != map crs
            mRubberBandExtend->setToGeometry( mGeom.densifyByCount( 10 ), mVlayer );
            mRubberBandExtend->show();
            extendLimit();
          }
        }
        else
        {
          if ( mRubberBandExtend )
            mRubberBandExtend->hide();
          if ( mRubberBandIntersection )
            mRubberBandIntersection->hide();
        }
      }
      else
      {
        if ( mRubberBandExtend )
          mRubberBandExtend->hide();
        if ( mRubberBandIntersection )
          mRubberBandIntersection->hide();
      }
      break;
  }
}

void QgsMapToolTrimExtendFeature::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  mMapPoint = e->mapPoint();

  FeatureFilter filter;
  QgsPointLocator::Match match;

  if ( e->button() == Qt::LeftButton )
  {
    switch ( mStep )
    {
      case StepLimit:
        match = mCanvas->snappingUtils()->snapToMap( mMapPoint, &filter, true );
        if ( mRubberBandLimit && mRubberBandLimit->isVisible() )
        {
          if ( getPoints( match, pLimit1, pLimit2 ) )
          {
            mStep = StepExtend;
            mLimitLayer = match.layer();
          }
        }
        break;
      case StepExtend:
        if ( mIsModified )
        {
          filter.setLayer( mVlayer );
          match = mCanvas->snappingUtils()->snapToMap( mMapPoint, &filter, true );

          if ( auto *lLayer = match.layer() )
          {
            lLayer->beginEditCommand( tr( "Trim/Extend feature" ) );
            lLayer->changeGeometry( match.featureId(), mGeom );
            if ( QgsProject::instance()->topologicalEditing() )
            {
              lLayer->addTopologicalPoints( mIntersection );
              mLimitLayer->addTopologicalPoints( mIntersection );
            }
            lLayer->endEditCommand();
            lLayer->triggerRepaint();

            emit messageEmitted( tr( "Feature trimmed/extended." ) );
          }
        }
        else
        {
          emit messageEmitted( tr( "Couldn't trim or extend the feature." ) );
        }

        // If Shift is pressed, keep the tool active with its reference feature
        if ( !( e->modifiers() & Qt::ShiftModifier ) )
          reset();
        break;
    }
  }
  else if ( e->button() == Qt::RightButton )
  {
    reset();
  }
}

void QgsMapToolTrimExtendFeature::keyPressEvent( QKeyEvent *e )
{
  if ( e && e->isAutoRepeat() )
  {
    return;
  }

  if ( e && e->key() == Qt::Key_Escape )
  {
    reset();
  }
}


void QgsMapToolTrimExtendFeature::extendLimit()
{
  if ( !mRubberBandLimitExtend )
  {
    return;
  }

  QgsVectorLayer *refLayer = qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() );
  refLayer = refLayer ? refLayer : mVlayer ? mVlayer
                                           : mLimitLayer;

  // Compute intersection between the line that extends the limit segment and the
  // edges of the map canvas
  QgsPointXY p1 = toLayerCoordinates( refLayer, *mRubberBandLimit->getPoint( 0, 0 ) );
  QgsPointXY p2 = toLayerCoordinates( refLayer, *mRubberBandLimit->getPoint( 0, 1 ) );
  QgsPoint canvasTopLeft = QgsPoint( toLayerCoordinates( refLayer, QPoint( 0, 0 ) ) );
  QgsPoint canvasTopRight = QgsPoint( toLayerCoordinates( refLayer, QPoint( mCanvas->width(), 0 ) ) );
  QgsPoint canvasBottomLeft = QgsPoint( toLayerCoordinates( refLayer, QPoint( 0, mCanvas->height() ) ) );
  QgsPoint canvasBottomRight = QgsPoint( toLayerCoordinates( refLayer, QPoint( mCanvas->width(), mCanvas->height() ) ) );

  QList<QgsPointXY> points;
  points << p1 << p2;

  QgsPoint intersection;
  if ( QgsGeometryUtils::lineIntersection( QgsPoint( p1 ), QgsPoint( p2 ) - QgsPoint( p1 ), canvasTopLeft, canvasTopRight - canvasTopLeft, intersection ) )
  {
    points << QgsPointXY( intersection );
  }
  if ( QgsGeometryUtils::lineIntersection( QgsPoint( p1 ), QgsPoint( p2 ) - QgsPoint( p1 ), canvasTopRight, canvasBottomRight - canvasTopRight, intersection ) )
  {
    points << QgsPointXY( intersection );
  }
  if ( QgsGeometryUtils::lineIntersection( QgsPoint( p1 ), QgsPoint( p2 ) - QgsPoint( p1 ), canvasBottomRight, canvasBottomLeft - canvasBottomRight, intersection ) )
  {
    points << QgsPointXY( intersection );
  }
  if ( QgsGeometryUtils::lineIntersection( QgsPoint( p1 ), QgsPoint( p2 ) - QgsPoint( p1 ), canvasBottomLeft, canvasTopLeft - canvasBottomLeft, intersection ) )
  {
    points << QgsPointXY( intersection );
  }

  // Reorder the points by x/y coordinates
  std::sort( points.begin(), points.end(), []( const QgsPointXY &a, const QgsPointXY &b ) -> bool {
    if ( a.x() == b.x() )
      return a.y() < b.y();
    return a.x() < b.x();
  } );

  // Keep only the closest intersection points from the original points
  const int p1Idx = points.indexOf( p1 );
  const int p2Idx = points.indexOf( p2 );
  const int first = std::max( 0, std::min( p1Idx, p2Idx ) - 1 );
  const int last = std::min( static_cast<int>( points.size() ) - 1, std::max( p1Idx, p2Idx ) + 1 );
  const QgsPolylineXY polyline = points.mid( first, last - first + 1 ).toVector();

  // Densify the polyline to display a more accurate prediction when layer crs != canvas crs
  QgsGeometry geom = QgsGeometry::fromPolylineXY( polyline ).densifyByCount( 10 );

  mRubberBandLimitExtend->setToGeometry( geom, refLayer );
  mRubberBandLimitExtend->show();
}
void QgsMapToolTrimExtendFeature::reset()
{
  mStep = StepLimit;
  mIsModified = false;
  mIs3DLayer = false;
  mIsIntersection = false;
  mSegmentIntersects = false;
  mRubberBandLimit.reset();
  mRubberBandLimitExtend.reset();
  mRubberBandExtend.reset();
  mRubberBandIntersection.reset();
  mVlayer = nullptr;
  mLimitLayer = nullptr;
}
void QgsMapToolTrimExtendFeature::activate()
{
  QgsMapTool::activate();

  // Save the original snapping configuration
  mOriginalSnappingConfig = mCanvas->snappingUtils()->config();

  // Enable Snapping & Snapping on Segment
  QgsSnappingConfig snappingConfig = mOriginalSnappingConfig;
  snappingConfig.setEnabled( true );
  Qgis::SnappingTypes flags = snappingConfig.typeFlag();
  flags |= Qgis::SnappingType::Segment;
  snappingConfig.setTypeFlag( flags );
  mCanvas->snappingUtils()->setConfig( snappingConfig );
}
void QgsMapToolTrimExtendFeature::deactivate()
{
  reset();
  // Restore the original snapping configuration
  mCanvas->snappingUtils()->setConfig( mOriginalSnappingConfig );
  QgsMapTool::deactivate();
}
