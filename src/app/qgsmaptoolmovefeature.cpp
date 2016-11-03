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

#include "qgsmaptoolmovefeature.h"
#include "qgsfeatureiterator.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsrubberband.h"
#include "qgsvectorlayer.h"
#include "qgstolerance.h"
#include "qgisapp.h"
#include "qgsadvanceddigitizingdockwidget.h"

#include <QMouseEvent>
#include <QSettings>
#include <limits>


QgsMapToolMoveFeature::QgsMapToolMoveFeature( QgsMapCanvas* canvas , MoveMode mode )
    : QgsMapToolAdvancedDigitizing( canvas, QgisApp::instance()->cadDockWidget() )
    , mRubberBand( nullptr )
    , mMode( mode )
{
  mToolName = tr( "Move feature" );
  switch ( mode )
  {
    case Move:
      mCaptureMode = QgsMapToolAdvancedDigitizing::CaptureSegment;
      break;
    case CopyMove:
      mCaptureMode = QgsMapToolAdvancedDigitizing::CaptureLine; // we copy/move several times
      break;
  }
}

QgsMapToolMoveFeature::~QgsMapToolMoveFeature()
{
  delete mRubberBand;
}

void QgsMapToolMoveFeature::cadCanvasMoveEvent( QgsMapMouseEvent* e )
{
  if ( mRubberBand )
  {
    QgsPoint pointCanvasCoords = e->mapPoint();
    double offsetX = pointCanvasCoords.x() - mStartPointMapCoords.x();
    double offsetY = pointCanvasCoords.y() - mStartPointMapCoords.y();
    mRubberBand->setTranslationOffset( offsetX, offsetY );
    mRubberBand->updatePosition();
    mRubberBand->update();
  }
}

void QgsMapToolMoveFeature::cadCanvasReleaseEvent( QgsMapMouseEvent* e )
{
  QgsVectorLayer* vlayer = currentVectorLayer();
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

    //find first geometry under mouse cursor and store iterator to it
    QgsPoint layerCoords = toLayerCoordinates( vlayer, e->pos() );
    double searchRadius = QgsTolerance::vertexSearchRadius( mCanvas->currentLayer(), mCanvas->mapSettings() );
    QgsRectangle selectRect( layerCoords.x() - searchRadius, layerCoords.y() - searchRadius,
                             layerCoords.x() + searchRadius, layerCoords.y() + searchRadius );

    if ( vlayer->selectedFeatureCount() == 0 )
    {
      QgsFeatureIterator fit = vlayer->getFeatures( QgsFeatureRequest().setFilterRect( selectRect ).setSubsetOfAttributes( QgsAttributeList() ) );

      //find the closest feature
      QgsGeometry pointGeometry = QgsGeometry::fromPoint( layerCoords );
      if ( pointGeometry.isEmpty() )
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
      mMovedFeatures = vlayer->selectedFeaturesIds();

      mRubberBand = createRubberBand( vlayer->geometryType() );
      QgsFeature feat;
      QgsFeatureIterator it = vlayer->selectedFeaturesIterator( QgsFeatureRequest().setSubsetOfAttributes( QgsAttributeList() ) );

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

    QgsPoint startPointLayerCoords = toLayerCoordinates(( QgsMapLayer* )vlayer, mStartPointMapCoords );
    QgsPoint stopPointLayerCoords = toLayerCoordinates(( QgsMapLayer* )vlayer, e->mapPoint() );

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
        break;

      case CopyMove:
        int featureCount = mMovedFeatures.count();

        QgsFeatureRequest request;
        request.setFilterFids( mMovedFeatures );
        QgsFeatureIterator fi = vlayer->getFeatures( request );
        QgsFeature f;
        QgsAttributeList pkAttrList = vlayer->pkAttributeList();

        int browsedFeatureCount = 0;
        int addedFeatureCount = 0;
        while ( fi.nextFeature( f ) )
        {
          browsedFeatureCount++;
          // remove pkey values
          Q_FOREACH ( auto idx, pkAttrList )
          {
            f.setAttribute( idx, QVariant() );
          }
          // translate
          QgsGeometry geom = f.geometry();
          geom.translate( dx, dy );
          f.setGeometry( geom );
#ifdef QGISDEBUG
          const QgsFeatureId  fid = f.id();
#endif
          // paste feature
          if ( vlayer->addFeature( f, false ) )
          {
            addedFeatureCount++;
          }
          else
          {
            QgsDebugMsg( QString( "could not add new feature. copied feature had id: %1" ).arg( fid ) );
          }
        }
        if ( addedFeatureCount != featureCount )
        {
          QString msg = QString( tr( "Only %1 out of %2 features were copied." ) ).arg( addedFeatureCount, featureCount );
          msg.append( " " );
          if ( browsedFeatureCount == featureCount )
          {
            msg.append( tr( "Some features could not be created on the layer." ) );
          }
          else
          {
            msg.append( tr( "Some features could not be retrieved from the selection." ) );
          }
          emit messageEmitted( msg, QgsMessageBar::CRITICAL );
        }
    }

    vlayer->endEditCommand();
    vlayer->triggerRepaint();
  }
}

//! called when map tool is being deactivated
void QgsMapToolMoveFeature::deactivate()
{
  //delete rubber band
  delete mRubberBand;
  mRubberBand = nullptr;

  QgsMapTool::deactivate();
}
