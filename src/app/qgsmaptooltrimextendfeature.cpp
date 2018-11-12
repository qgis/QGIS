/***************************************************************************
    qgmaptoolextendfeature.cpp  -  map tool for extending feature
    ---------------------
    begin                : October 2018
    copyright            : (C) 2018 by Loïc Bartoletti
    email                : loic dot bartoletti at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptooltrimextendfeature.h"

#include "qgsmapcanvas.h"
#include "qgsvertexmarker.h"
#include "qgsvectorlayer.h"
#include "qgsgeometry.h"
#include "qgsrubberband.h"
#include "qgssnappingutils.h"
#include "qgstolerance.h"
#include "qgisapp.h"
#include "qgsgeometryutils.h"
#include "qgsmapmouseevent.h"
#include "qgssnapindicator.h"

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
    void setLayer( QgsVectorLayer *vlayer ) { mLayer = vlayer; }

  private:
    const QgsVectorLayer *mLayer = nullptr;
};

QgsMapToolTrimExtendFeature::QgsMapToolTrimExtendFeature( QgsMapCanvas *canvas )
  : QgsMapToolEdit( canvas )
{
  mToolName = tr( "Trim/Extend feature" );
}

QgsMapToolTrimExtendFeature::~QgsMapToolTrimExtendFeature()
{
}

static void getPoints( const QgsPointLocator::Match &match, QgsPoint &p1, QgsPoint &p2 )
{
  const QgsFeatureId fid = match.featureId();
  const int vertex = match.vertexIndex();

  const QgsGeometry geom = match.layer()->getGeometry( fid );

  if ( !( geom.isNull() || geom.isEmpty() ) )
  {
    p1 = geom.vertexAt( vertex );
    p2 = geom.vertexAt( vertex + 1 );
  }
}

void QgsMapToolTrimExtendFeature::canvasMoveEvent( QgsMapMouseEvent *e )
{
  mapPoint = e->mapPoint();

  FeatureFilter filter;
  QgsPointLocator::Match match;

  switch ( step )
  {
    case 0:

      match = mCanvas->snappingUtils()->snapToMap( mapPoint, &filter );
      if ( match.isValid() )
      {
        is3DLayer = QgsWkbTypes::hasZ( match.layer()->wkbType() );

        QgsPointXY p1, p2;
        match.edgePoints( p1, p2 );

        mRubberBandLimit.reset( createRubberBand( QgsWkbTypes::LineGeometry ) );
        mRubberBandLimit->addPoint( p1 );
        mRubberBandLimit->addPoint( p2 );
        mRubberBandLimit->show();

      }
      else
      {
        if ( mRubberBandLimit )
          mRubberBandLimit->hide();
      }
      break;
    case 1:

      QgsMapLayer *currentLayer = mCanvas->currentLayer();
      if ( !currentLayer )
        break;

      vlayer = qobject_cast<QgsVectorLayer *>( currentLayer );
      if ( !vlayer )
        break;

      if ( !vlayer->isEditable() )
        break;

      filter.setLayer( vlayer );
      match = mCanvas->snappingUtils()->snapToMap( mapPoint, &filter );

      if ( match.isValid() )
      {
        if ( match.layer() != vlayer )
          break;

        QgsPointXY p1, p2;
        match.edgePoints( p1, p2 );

        getPoints( match, pExtend1, pExtend2 );

        // No need to trim/extend if segments are continuous
        if ( ( ( pLimit1 == pExtend1 ) || ( pLimit1 == pExtend2 ) ) || ( ( pLimit2 == pExtend1 ) || ( pLimit2 == pExtend2 ) ) )
          break;

        segmentIntersects = QgsGeometryUtils::segmentIntersection( pLimit1, pLimit2, pExtend1, pExtend2, intersection, isIntersection, 1e-8, true );

        if ( is3DLayer && QgsWkbTypes::hasZ( match.layer()->wkbType() ) )
        {
          /* Z Interpolation */
          QgsLineString line( pLimit1, pLimit2 );

          intersection = QgsGeometryUtils::closestPoint( line, QgsPoint( intersection ) );
        }

        if ( isIntersection )
        {
          mRubberBandIntersection.reset( createRubberBand( QgsWkbTypes::PointGeometry ) );
          mRubberBandIntersection->addPoint( QgsPointXY( intersection ) );
          mRubberBandIntersection->show();

          mRubberBandExtend.reset( createRubberBand( match.layer()->geometryType() ) );

          geom = match.layer()->getGeometry( match.featureId() );
          int index = match.vertexIndex();

          if ( !segmentIntersects )
          {
            QgsPoint ptInter( intersection.x(), intersection.y() );
            if ( pExtend2.distance( ptInter ) < pExtend1.distance( ptInter ) )
              index += 1;
          }
          else // TRIM PART
          {
            // Part where the mouse is (+) will be trimed
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
            if ( QgsGeometryUtils::leftOfLine( QgsPoint( mapPoint ), pLimit1, pLimit2 ) != QgsGeometryUtils::leftOfLine( pExtend1, pLimit1, pLimit2 ) )
              index += 1;
          }

          isModified = geom.moveVertex( intersection, index );

          if ( isModified )
          {
            mRubberBandExtend->setToGeometry( geom );
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
  mapPoint = e->mapPoint();

  FeatureFilter filter;
  QgsPointLocator::Match match;

  if ( e->button() == Qt::LeftButton )
  {
    switch ( step )
    {
      case 0:
        match = mCanvas->snappingUtils()->snapToMap( mapPoint, &filter );
        if ( mRubberBandLimit && mRubberBandLimit->isVisible() )
        {
          getPoints( match, pLimit1, pLimit2 );
          step += 1;
        }
        break;
      case 1:
        if ( isModified )
        {
          filter.setLayer( vlayer );
          match = mCanvas->snappingUtils()->snapToMap( mapPoint, &filter );

          match.layer()->beginEditCommand( tr( "Trim/Extend feature" ) );
          match.layer()->changeGeometry( match.featureId(), geom );
          match.layer()->endEditCommand();
          match.layer()->triggerRepaint();

          emit messageEmitted( tr( "Feature trimed/extended." ) );
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
  step = 0;
  isModified = false;
  is3DLayer = false;
  isIntersection = false;
  segmentIntersects = false;
  mRubberBandLimit.reset();
  mRubberBandExtend.reset();
  mRubberBandIntersection.reset();
  QgsMapTool::deactivate();
}

