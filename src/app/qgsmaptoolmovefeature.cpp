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
#include "qgsfeatureiterator.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmaptoolmovefeature.h"
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
  , mSnapIndicator( std::make_unique< QgsSnapIndicator>( canvas ) )
  , mMode( mode )
{
  mToolName = tr( "Move feature" );
}

QgsMapToolMoveFeature::~QgsMapToolMoveFeature()
{
  delete mRubberBand;
}

void QgsMapToolMoveFeature::cadCanvasMoveEvent( QgsMapMouseEvent *e )
{
  if ( mRubberBand )
  {
    const QgsPointXY pointCanvasCoords = e->mapPoint();
    const double offsetX = pointCanvasCoords.x() - mStartPointMapCoords.x();
    const double offsetY = pointCanvasCoords.y() - mStartPointMapCoords.y();
    mRubberBand->setTranslationOffset( offsetX, offsetY );
    mRubberBand->updatePosition();
    mRubberBand->update();
    mSnapIndicator->setMatch( e->mapPointMatch() );
  }
  else
  {
    mSnapIndicator->setMatch( e->mapPointMatch() );
  }
}

void QgsMapToolMoveFeature::cadCanvasReleaseEvent( QgsMapMouseEvent *e )
{
  QgsVectorLayer *vlayer = currentVectorLayer();
  if ( !vlayer || !vlayer->isEditable() )
  {
    delete mRubberBand;
    mRubberBand = nullptr;
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
    const QgsRectangle selectRect( layerCoords.x() - searchRadius, layerCoords.y() - searchRadius,
                                   layerCoords.x() + searchRadius, layerCoords.y() + searchRadius );

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
      mRubberBand->setToGeometry( cf.geometry(), vlayer );
    }
    else
    {
      mMovedFeatures = vlayer->selectedFeatureIds();

      mRubberBand = createRubberBand( vlayer->geometryType() );
      QgsFeature feat;
      QgsFeatureIterator it = vlayer->getSelectedFeatures( QgsFeatureRequest().setNoAttributes() );

      bool allFeaturesInView = true;
      const QgsRectangle viewRect = mCanvas->mapSettings().mapToLayerCoordinates( vlayer, mCanvas->extent() );

      while ( it.nextFeature( feat ) )
      {
        mRubberBand->addGeometry( feat.geometry(), vlayer, false );

        if ( allFeaturesInView && !viewRect.intersects( feat.geometry().boundingBox() ) )
          allFeaturesInView = false;
      }
      mRubberBand->updatePosition();
      mRubberBand->update();

      if ( !allFeaturesInView )
      {
        // for extra safety to make sure we are not modifying geometries by accident

        const int res = QMessageBox::warning( mCanvas, tr( "Move features" ),
                                              tr( "Some of the selected features are outside of the current map view. Would you still like to continue?" ),
                                              QMessageBox::Yes | QMessageBox::No );
        if ( res != QMessageBox::Yes )
        {
          mMovedFeatures.clear();
          delete mRubberBand;
          mRubberBand = nullptr;
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
      delete mRubberBand;
      mRubberBand = nullptr;
      mSnapIndicator->setMatch( QgsPointLocator::Match() );
      return;
    }

    const QgsPointXY startPointLayerCoords = toLayerCoordinates( ( QgsMapLayer * )vlayer, mStartPointMapCoords );
    const QgsPointXY stopPointLayerCoords = toLayerCoordinates( ( QgsMapLayer * )vlayer, e->mapPoint() );

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
        while ( fi.nextFeature( f ) )
        {
          if ( !f.hasGeometry() )
            continue;

          QgsGeometry geom = f.geometry();
          if ( geom.translate( dx, dy ) != Qgis::GeometryOperationResult::Success )
            continue;

          const QgsFeatureId id = f.id();
          vlayer->changeGeometry( id, geom );

          if ( QgsProject::instance()->topologicalEditing() )
          {
            if ( mSnapIndicator && ( mSnapIndicator->match().layer() != nullptr ) )
            {
              mSnapIndicator->match().layer()->addTopologicalPoints( vlayer->getGeometry( id ) );
            }
            vlayer->addTopologicalPoints( vlayer->getGeometry( id ) );
          }
        }
        delete mRubberBand;
        mRubberBand = nullptr;
        mSnapIndicator->setMatch( QgsPointLocator::Match() );
        cadDockWidget()->clear();
        break;
      }
      case CopyMove:
        QgsFeatureRequest request;
        request.setFilterFids( mMovedFeatures );
        QString *errorMsg = new QString();
        if ( !QgisApp::instance()->vectorLayerTools()->copyMoveFeatures( vlayer, request, dx, dy, errorMsg, QgsProject::instance()->topologicalEditing(), mSnapIndicator->match().layer() ) )
        {
          emit messageEmitted( *errorMsg, Qgis::MessageLevel::Critical );
          delete mRubberBand;
          mRubberBand = nullptr;
          mSnapIndicator->setMatch( QgsPointLocator::Match() );
        }
        break;
    }

    vlayer->endEditCommand();
    vlayer->triggerRepaint();
  }
}

void QgsMapToolMoveFeature::deactivate()
{
  //delete rubber band
  delete mRubberBand;
  mRubberBand = nullptr;
  mSnapIndicator->setMatch( QgsPointLocator::Match() );

  QgsMapToolAdvancedDigitizing::deactivate();
}

void QgsMapToolMoveFeature::keyReleaseEvent( QKeyEvent *e )
{
  if ( mRubberBand && e->key() == Qt::Key_Escape )
  {
    cadDockWidget()->clear();
    delete mRubberBand;
    mRubberBand = nullptr;
    mSnapIndicator->setMatch( QgsPointLocator::Match() );
  }
}
