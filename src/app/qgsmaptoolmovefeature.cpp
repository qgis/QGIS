/***************************************************************************
    qgsmaptoolmovefeature.cpp  -  map tool for translating features by mouse drag
    ---------------------
    begin                : Juli 2007
    copyright            : (C) 2007 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgisapp.h"
#include "qgsadvanceddigitizingdockwidget.h"
#include "qgsavoidintersectionsoperation.h"
#include "qgsfeatureiterator.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmaptoolmovefeature.h"
#include "moc_qgsmaptoolmovefeature.cpp"
#include "qgsrubberband.h"
#include "qgstolerance.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayertools.h"
#include "qgssnapindicator.h"
#include "qgsmapmouseevent.h"

#include <QMessageBox>
#include <QSettings>
#include <limits>


QgsMapToolMoveFeature::QgsMapToolMoveFeature( QgsMapCanvas *canvas, MoveMode mode )
  : QgsMapToolAdvancedDigitizing( canvas, QgisApp::instance()->cadDockWidget() )
  , mSnapIndicator( std::make_unique<QgsSnapIndicator>( canvas ) )
  , mMode( mode )
{
  mToolName = tr( "Move feature" );
}

QgsMapToolMoveFeature::~QgsMapToolMoveFeature()
{
  deleteRubberband();
}

void QgsMapToolMoveFeature::cadCanvasMoveEvent( QgsMapMouseEvent *e )
{
  if ( mRubberBand )
  {
    if ( QgsVectorLayer *vlayer = currentVectorLayer() )
    {
      // When MapCanvas crs == layer crs, fast rubberband translation
      if ( vlayer->crs() == canvas()->mapSettings().destinationCrs() )
      {
        const QgsPointXY pointCanvasCoords = e->mapPoint();
        const double offsetX = pointCanvasCoords.x() - mStartPointMapCoords.x();
        const double offsetY = pointCanvasCoords.y() - mStartPointMapCoords.y();
        mRubberBand->setTranslationOffset( offsetX, offsetY );
      }

      // Else, recreate the rubber band from the translated geometries
      else
      {
        const QgsPointXY startPointLayerCoords = toLayerCoordinates( ( QgsMapLayer * ) vlayer, mStartPointMapCoords );
        const QgsPointXY stopPointLayerCoords = toLayerCoordinates( ( QgsMapLayer * ) vlayer, e->mapPoint() );

        const double dx = stopPointLayerCoords.x() - startPointLayerCoords.x();
        const double dy = stopPointLayerCoords.y() - startPointLayerCoords.y();

        QgsGeometry geom = mGeom;

        if ( geom.translate( dx, dy ) == Qgis::GeometryOperationResult::Success )
        {
          mRubberBand->setToGeometry( geom, vlayer );
        }
        else
        {
          mRubberBand->reset( vlayer->geometryType() );
        }
      }
    }
  }

  mSnapIndicator->setMatch( e->mapPointMatch() );
}

void QgsMapToolMoveFeature::cadCanvasReleaseEvent( QgsMapMouseEvent *e )
{
  QgsVectorLayer *vlayer = currentVectorLayer();
  if ( !vlayer || !vlayer->isEditable() )
  {
    deleteRubberband();
    mSnapIndicator->setMatch( QgsPointLocator::Match() );
    cadDockWidget()->clear();
    notifyNotEditableLayer();
    return;
  }

  if ( !mRubberBand )
  {
    //find first geometry under mouse cursor and store iterator to it
    const QgsPointXY layerCoords = toLayerCoordinates( vlayer, e->mapPoint() );
    const double searchRadius = QgsTolerance::vertexSearchRadius( mCanvas->currentLayer(), mCanvas->mapSettings() );
    const QgsRectangle selectRect( layerCoords.x() - searchRadius, layerCoords.y() - searchRadius, layerCoords.x() + searchRadius, layerCoords.y() + searchRadius );

    if ( vlayer->selectedFeatureCount() == 0 )
    {
      QgsFeatureIterator fit = vlayer->getFeatures( QgsFeatureRequest().setFilterRect( selectRect ).setNoAttributes() );

      //find the closest feature
      const QgsGeometry pointGeometry = QgsGeometry::fromPointXY( layerCoords );
      if ( pointGeometry.isNull() )
      {
        cadDockWidget()->clear();
        return;
      }

      double minDistance = std::numeric_limits<double>::max();

      QgsFeature cf;
      QgsFeature f;
      while ( fit.nextFeature( f ) )
      {
        if ( f.hasGeometry() )
        {
          const double currentDistance = pointGeometry.distance( f.geometry() );
          if ( currentDistance < minDistance )
          {
            minDistance = currentDistance;
            cf = f;
          }
        }
      }

      if ( minDistance == std::numeric_limits<double>::max() )
      {
        cadDockWidget()->clear();
        return;
      }

      mMovedFeatures.clear();
      mMovedFeatures << cf.id(); //todo: take the closest feature, not the first one...

      mRubberBand = createRubberBand( vlayer->geometryType() );
      mGeom = cf.geometry();
      mRubberBand->setToGeometry( mGeom, vlayer );
    }
    else
    {
      mMovedFeatures = vlayer->selectedFeatureIds();

      mRubberBand = createRubberBand( vlayer->geometryType() );
      QgsFeature feat;
      QgsFeatureIterator it = vlayer->getSelectedFeatures( QgsFeatureRequest().setNoAttributes() );

      bool allFeaturesInView = true;
      const QgsRectangle viewRect = mCanvas->mapSettings().mapToLayerCoordinates( vlayer, mCanvas->extent() );

      QVector<QgsGeometry> selectedGeometries;
      while ( it.nextFeature( feat ) )
      {
        selectedGeometries << feat.geometry();

        if ( allFeaturesInView && !viewRect.intersects( feat.geometry().boundingBox() ) )
          allFeaturesInView = false;
      }
      mGeom = QgsGeometry::collectGeometry( selectedGeometries );
      mRubberBand->setToGeometry( mGeom, vlayer );

      if ( !allFeaturesInView )
      {
        // for extra safety to make sure we are not modifying geometries by accident

        const int res = QMessageBox::warning( mCanvas, tr( "Move features" ), tr( "Some of the selected features are outside of the current map view. Would you still like to continue?" ), QMessageBox::Yes | QMessageBox::No );
        if ( res != QMessageBox::Yes )
        {
          mMovedFeatures.clear();
          deleteRubberband();
          mSnapIndicator->setMatch( QgsPointLocator::Match() );
          return;
        }
      }
    }

    mStartPointMapCoords = e->mapPoint();
    mRubberBand->show();
  }
  else
  {
    // copy and move mode
    if ( e->button() != Qt::LeftButton )
    {
      cadDockWidget()->clear();
      deleteRubberband();
      mSnapIndicator->setMatch( QgsPointLocator::Match() );
      return;
    }

    const QgsPointXY startPointLayerCoords = toLayerCoordinates( ( QgsMapLayer * ) vlayer, mStartPointMapCoords );
    const QgsPointXY stopPointLayerCoords = toLayerCoordinates( ( QgsMapLayer * ) vlayer, e->mapPoint() );

    const double dx = stopPointLayerCoords.x() - startPointLayerCoords.x();
    const double dy = stopPointLayerCoords.y() - startPointLayerCoords.y();

    vlayer->beginEditCommand( mMode == Move ? tr( "Feature moved" ) : tr( "Feature copied and moved" ) );

    switch ( mMode )
    {
      case Move:
      {
        QgsFeatureRequest request;
        request.setFilterFids( mMovedFeatures ).setNoAttributes();
        QgsFeatureIterator fi = vlayer->getFeatures( request );
        QgsFeature f;

        QgsAvoidIntersectionsOperation avoidIntersections;
        connect( &avoidIntersections, &QgsAvoidIntersectionsOperation::messageEmitted, this, &QgsMapTool::messageEmitted );

        // when removing intersections don't check for intersections with selected features
        const QHash<QgsVectorLayer *, QSet<QgsFeatureId>> ignoreFeatures { { vlayer, mMovedFeatures } };

        while ( fi.nextFeature( f ) )
        {
          if ( !f.hasGeometry() )
            continue;

          QgsGeometry geom = f.geometry();
          if ( geom.translate( dx, dy ) != Qgis::GeometryOperationResult::Success )
            continue;

          const QgsFeatureId id = f.id();

          if ( vlayer->geometryType() == Qgis::GeometryType::Polygon )
          {
            const QgsAvoidIntersectionsOperation::Result res = avoidIntersections.apply( vlayer, id, geom, ignoreFeatures );

            if ( res.operationResult == Qgis::GeometryOperationResult::InvalidInputGeometryType || geom.isEmpty() )
            {
              const QString errorMessage = ( geom.isEmpty() ) ? tr( "The feature cannot be moved because the resulting geometry would be empty" ) : tr( "An error was reported during intersection removal" );

              emit messageEmitted( errorMessage, Qgis::MessageLevel::Warning );
              vlayer->destroyEditCommand();
              return;
            }
          }

          vlayer->changeGeometry( id, geom );

          if ( QgsProject::instance()->topologicalEditing() )
          {
            if ( mSnapIndicator && ( mSnapIndicator->match().layer() ) )
            {
              mSnapIndicator->match().layer()->addTopologicalPoints( vlayer->getGeometry( id ) );
            }
            vlayer->addTopologicalPoints( vlayer->getGeometry( id ) );
          }
        }
        deleteRubberband();
        mSnapIndicator->setMatch( QgsPointLocator::Match() );
        cadDockWidget()->clear();
        break;
      }
      case CopyMove:
        QgsFeatureRequest request;
        request.setFilterFids( mMovedFeatures );
        QString errorMsg;
        QString childrenInfoMsg;
        if ( !QgisApp::instance()->vectorLayerTools()->copyMoveFeatures( vlayer, request, dx, dy, &errorMsg, QgsProject::instance()->topologicalEditing(), mSnapIndicator->match().layer(), &childrenInfoMsg ) )
        {
          emit messageEmitted( errorMsg, Qgis::MessageLevel::Critical );
          deleteRubberband();
          vlayer->deleteFeatures( request.filterFids() );
          vlayer->destroyEditCommand();
          mSnapIndicator->setMatch( QgsPointLocator::Match() );
          return;
        }
        if ( !childrenInfoMsg.isEmpty() )
        {
          emit messageEmitted( childrenInfoMsg, Qgis::MessageLevel::Info );
        }
        break;
    }

    vlayer->endEditCommand();
    vlayer->triggerRepaint();
  }
}

void QgsMapToolMoveFeature::deactivate()
{
  deleteRubberband();
  mSnapIndicator->setMatch( QgsPointLocator::Match() );

  QgsMapToolAdvancedDigitizing::deactivate();
}

void QgsMapToolMoveFeature::keyReleaseEvent( QKeyEvent *e )
{
  if ( mRubberBand && e->key() == Qt::Key_Escape )
  {
    cadDockWidget()->clear();
    deleteRubberband();
    mSnapIndicator->setMatch( QgsPointLocator::Match() );
  }
}

void QgsMapToolMoveFeature::deleteRubberband()
{
  delete mRubberBand;
  mRubberBand = nullptr;
  mGeom = QgsGeometry();
}
