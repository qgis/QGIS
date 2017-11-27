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


#include <QMouseEvent>
#include <QSettings>
#include <limits>


QgsMapToolMoveFeature::QgsMapToolMoveFeature( QgsMapCanvas *canvas, MoveMode mode )
  : QgsMapToolAdvancedDigitizing( canvas, QgisApp::instance()->cadDockWidget() )
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
    QgsPointXY pointCanvasCoords = e->mapPoint();
    double offsetX = pointCanvasCoords.x() - mStartPointMapCoords.x();
    double offsetY = pointCanvasCoords.y() - mStartPointMapCoords.y();
    mRubberBand->setTranslationOffset( offsetX, offsetY );
    mRubberBand->updatePosition();
    mRubberBand->update();
  }
}

void QgsMapToolMoveFeature::cadCanvasReleaseEvent( QgsMapMouseEvent *e )
{
  QgsVectorLayer *vlayer = currentVectorLayer();
  if ( !vlayer || !vlayer->isEditable() )
  {
    delete mRubberBand;
    mRubberBand = nullptr;
    cadDockWidget()->clear();
    notifyNotEditableLayer();
    return;
  }

  if ( !mRubberBand )
  {
    // ideally we would snap preferably on the moved feature
    e->snapPoint();

    //find first geometry under mouse cursor and store iterator to it
    QgsPointXY layerCoords = toLayerCoordinates( vlayer, e->mapPoint() );
    double searchRadius = QgsTolerance::vertexSearchRadius( mCanvas->currentLayer(), mCanvas->mapSettings() );
    QgsRectangle selectRect( layerCoords.x() - searchRadius, layerCoords.y() - searchRadius,
                             layerCoords.x() + searchRadius, layerCoords.y() + searchRadius );

    if ( vlayer->selectedFeatureCount() == 0 )
    {
      QgsFeatureIterator fit = vlayer->getFeatures( QgsFeatureRequest().setFilterRect( selectRect ).setSubsetOfAttributes( QgsAttributeList() ) );

      //find the closest feature
      QgsGeometry pointGeometry = QgsGeometry::fromPointXY( layerCoords );
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
          double currentDistance = pointGeometry.distance( f.geometry() );
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
      QgsFeatureIterator it = vlayer->getSelectedFeatures( QgsFeatureRequest().setSubsetOfAttributes( QgsAttributeList() ) );

      while ( it.nextFeature( feat ) )
      {
        mRubberBand->addGeometry( feat.geometry(), vlayer );
      }
    }

    mStartPointMapCoords = e->mapPoint();
    mRubberBand->setColor( QColor( 255, 0, 0, 65 ) );
    mRubberBand->setWidth( 2 );
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
      return;
    }
    e->snapPoint();

    QgsPointXY startPointLayerCoords = toLayerCoordinates( ( QgsMapLayer * )vlayer, mStartPointMapCoords );
    QgsPointXY stopPointLayerCoords = toLayerCoordinates( ( QgsMapLayer * )vlayer, e->mapPoint() );

    double dx = stopPointLayerCoords.x() - startPointLayerCoords.x();
    double dy = stopPointLayerCoords.y() - startPointLayerCoords.y();

    vlayer->beginEditCommand( mMode == Move ? tr( "Feature moved" ) : tr( "Feature copied and moved" ) );

    switch ( mMode )
    {
      case Move:
        Q_FOREACH ( QgsFeatureId id, mMovedFeatures )
        {
          vlayer->translateFeature( id, dx, dy );
        }
        delete mRubberBand;
        mRubberBand = nullptr;
        cadDockWidget()->clear();
        break;

      case CopyMove:
        QgsFeatureRequest request;
        request.setFilterFids( mMovedFeatures );
        QString *errorMsg = new QString();
        if ( !QgisApp::instance()->vectorLayerTools()->copyMoveFeatures( vlayer, request, dx, dy, errorMsg ) )
        {
          emit messageEmitted( *errorMsg, QgsMessageBar::CRITICAL );
          delete mRubberBand;
          mRubberBand = nullptr;
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

  QgsMapTool::deactivate();
}
