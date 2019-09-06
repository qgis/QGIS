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

#include "qgsmapcanvas.h"
#include "qgsvertexmarker.h"
#include "qgsvectorlayer.h"
#include "qgsgeometry.h"
#include "qgssnappingutils.h"
#include "qgstolerance.h"
#include "qgisapp.h"
#include "qgsgeometryutils.h"
#include "qgsmapmouseevent.h"
#include "qgssnapindicator.h"

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
}

static bool getPoints( const QgsPointLocator::Match &match, QgsPoint &p1, QgsPoint &p2 )
{
  bool ret = false;
  const QgsFeatureId fid = match.featureId();
  const int vertex = match.vertexIndex();

  if ( match.layer() )
  {
    const QgsGeometry geom = match.layer()->getGeometry( fid );

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

      match = mCanvas->snappingUtils()->snapToMapRelaxed( mMapPoint, &filter );
      if ( match.isValid() )
      {
        mIs3DLayer = QgsWkbTypes::hasZ( match.layer()->wkbType() );

        QgsPointXY p1, p2;
        match.edgePoints( p1, p2 );

        mRubberBandLimit.reset( createRubberBand( QgsWkbTypes::LineGeometry ) );
        mRubberBandLimit->addPoint( p1 );
        mRubberBandLimit->addPoint( p2 );
        mRubberBandLimit->show();

      }
      else if ( mRubberBandLimit )
      {
        mRubberBandLimit->hide();
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
      match = mCanvas->snappingUtils()->snapToMapRelaxed( mMapPoint, &filter );

      if ( match.isValid() )
      {
        if ( match.layer() != mVlayer )
          break;

        QgsPointXY p1, p2;
        match.edgePoints( p1, p2 );

        if ( !getPoints( match, pExtend1, pExtend2 ) )
          break;

        // No need to trim/extend if segments are continuous
        if ( ( ( pLimit1 == pExtend1 ) || ( pLimit1 == pExtend2 ) ) || ( ( pLimit2 == pExtend1 ) || ( pLimit2 == pExtend2 ) ) )
          break;

        mSegmentIntersects = QgsGeometryUtils::segmentIntersection( pLimit1, pLimit2, pExtend1, pExtend2, mIntersection, mIsIntersection, 1e-8, true );

        if ( mIs3DLayer && QgsWkbTypes::hasZ( match.layer()->wkbType() ) )
        {
          /* Z Interpolation */
          QgsLineString line( pLimit1, pLimit2 );

          mIntersection = QgsGeometryUtils::closestPoint( line, QgsPoint( mIntersection ) );
        }

        if ( mIsIntersection )
        {
          mRubberBandIntersection.reset( createRubberBand( QgsWkbTypes::PointGeometry ) );
          mRubberBandIntersection->addPoint( QgsPointXY( mIntersection ) );
          mRubberBandIntersection->show();

          mRubberBandExtend.reset( createRubberBand( match.layer()->geometryType() ) );

          mGeom = match.layer()->getGeometry( match.featureId() );
          int index = match.vertexIndex();

          if ( !mSegmentIntersects )
          {
            QgsPoint ptInter( mIntersection.x(), mIntersection.y() );
            if ( pExtend2.distance( ptInter ) < pExtend1.distance( ptInter ) )
              index += 1;
          }
          // TRIM PART
          else if ( QgsGeometryUtils::leftOfLine( QgsPoint( mMapPoint ), pLimit1, pLimit2 ) != QgsGeometryUtils::leftOfLine( pExtend1, pLimit1, pLimit2 ) )
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
            mRubberBandExtend->setToGeometry( mGeom );
            mRubberBandExtend->show();
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
        match = mCanvas->snappingUtils()->snapToMapRelaxed( mMapPoint, &filter );
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
          match = mCanvas->snappingUtils()->snapToMapRelaxed( mMapPoint, &filter );

          if ( match.layer() )
          {

            match.layer()->beginEditCommand( tr( "Trim/Extend feature" ) );
            match.layer()->changeGeometry( match.featureId(), mGeom );
            if ( QgsProject::instance()->topologicalEditing() )
            {
              match.layer()->addTopologicalPoints( mIntersection );
              mLimitLayer->addTopologicalPoints( mIntersection );
            }
            match.layer()->endEditCommand();
            match.layer()->triggerRepaint();

            emit messageEmitted( tr( "Feature trimmed/extended." ) );
          }
        }
        else
        {
          emit messageEmitted( tr( "Couldn't trim or extend the feature." ) );
        }
        deactivate();
        break;
    }
  }
  else if ( e->button() == Qt::RightButton )
  {
    deactivate();
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
    deactivate();
  }
}

void QgsMapToolTrimExtendFeature::deactivate()
{
  mStep = StepLimit;
  mIsModified = false;
  mIs3DLayer = false;
  mIsIntersection = false;
  mSegmentIntersects = false;
  mRubberBandLimit.reset();
  mRubberBandExtend.reset();
  mRubberBandIntersection.reset();
  QgsMapTool::deactivate();
  mVlayer = nullptr;
  mLimitLayer = nullptr;
}
