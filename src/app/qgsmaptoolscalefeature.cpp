/***************************************************************************
    qgsmaptoolscalefeature.cpp  -  map tool for scaling features by mouse drag
    ---------------------
    Date                 : December 2020
    Copyright            : (C) 2020 by roya0045
    Contact              : ping me on github
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QSettings>
#include <QEvent>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>

#include <limits>
#include <cmath>

#include "qgsadvanceddigitizingdockwidget.h"
#include "qgsmaptoolscalefeature.h"
#include "qgsfeatureiterator.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsrubberband.h"
#include "qgsvectorlayer.h"
#include "qgstolerance.h"
#include "qgisapp.h"
#include "qgsspinbox.h"
#include "qgsdoublespinbox.h"
#include "qgssnapindicator.h"
#include "qgsmapmouseevent.h"


QgsScaleMagnetWidget::QgsScaleMagnetWidget( const QString &label, QWidget *parent )
  : QWidget( parent )
{
  mLayout = new QHBoxLayout( this );
  mLayout->setContentsMargins( 0, 0, 0, 0 );
  //mLayout->setAlignment( Qt::AlignLeft );
  setLayout( mLayout );

  if ( !label.isEmpty() )
  {
    QLabel *lbl = new QLabel( label, this );
    lbl->setAlignment( Qt::AlignRight | Qt::AlignCenter );
    mLayout->addWidget( lbl );
  }

  mScaleSpinBox = new QgsDoubleSpinBox( this );
  mScaleSpinBox->setSingleStep( 0.5 );
  mScaleSpinBox->setMinimum( 0 );
  mScaleSpinBox->setValue( 1 );
  mScaleSpinBox->setShowClearButton( false );
  mScaleSpinBox->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Preferred );
  mLayout->addWidget( mScaleSpinBox );

  // connect signals
  mScaleSpinBox->installEventFilter( this );
  connect( mScaleSpinBox, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), this, &QgsScaleMagnetWidget::scaleSpinBoxValueChanged );

  // config focus
  setFocusProxy( mScaleSpinBox );
}

void QgsScaleMagnetWidget::setScale( double scale )
{
  mScaleSpinBox->setValue( scale );
}

double QgsScaleMagnetWidget::scale() const
{
  return mScaleSpinBox->value();
}

bool QgsScaleMagnetWidget::eventFilter( QObject *obj, QEvent *ev )
{
  if ( obj == mScaleSpinBox && ev->type() == QEvent::KeyPress )
  {
    QKeyEvent *event = static_cast<QKeyEvent *>( ev );
    if ( event->key() == Qt::Key_Escape )
    {
      emit scaleEditingCanceled();
      return true;
    }
    if ( event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return )
    {
      emit scaleEditingFinished( scale() );
      return true;
    }
  }

  return false;
}

void QgsScaleMagnetWidget::scaleSpinBoxValueChanged( double scale )
{
  emit scaleChanged( scale );
}

//
// QgsMapToolScaleFeature
//

QgsMapToolScaleFeature::QgsMapToolScaleFeature( QgsMapCanvas *canvas )
  : QgsMapToolAdvancedDigitizing( canvas, QgisApp::instance()->cadDockWidget() )
  , mSnapIndicator( std::make_unique< QgsSnapIndicator>( canvas ) )
{
  mToolName = tr( "Scale feature" );
}

QgsMapToolScaleFeature::~QgsMapToolScaleFeature()
{
  deleteScalingWidget();
  mAnchorPoint.reset();
  deleteRubberband();
  mSnapIndicator->setMatch( QgsPointLocator::Match() );
}

void QgsMapToolScaleFeature::cadCanvasMoveEvent( QgsMapMouseEvent *e )
{
  mSnapIndicator->setMatch( e->mapPointMatch() );
  if ( mBaseDistance == 0 )
  {
    return;
  }
  if ( mScalingActive )
  {
    const double distance = mFeatureCenterMapCoords.distance( e->mapPoint() );
    const double scale = distance / mBaseDistance; // min 0 or no limit?

    if ( mScalingWidget )
    {
      disconnect( mScalingWidget, &QgsScaleMagnetWidget::scaleChanged, this, &QgsMapToolScaleFeature::updateRubberband );
      mScalingWidget->setScale( scale );
      mScalingWidget->setFocus( Qt::TabFocusReason );
      mScalingWidget->editor()->selectAll();
      connect( mScalingWidget, &QgsScaleMagnetWidget::scaleChanged, this, &QgsMapToolScaleFeature::updateRubberband );
    }
    updateRubberband( scale );
  }
}

void QgsMapToolScaleFeature::cadCanvasReleaseEvent( QgsMapMouseEvent *e )
{
  if ( !mCanvas )
  {
    return;
  }

  QgsVectorLayer *vlayer = currentVectorLayer();
  if ( !vlayer )
  {
    deleteScalingWidget();
    deleteRubberband();
    notifyNotVectorLayer();
    mSnapIndicator->setMatch( QgsPointLocator::Match() );
    mCadDockWidget->clear();
    return;
  }

  if ( e->button() == Qt::RightButton )
  {
    cancel();
    return;
  }

  // place anchor point on CTRL + click
  if ( e->modifiers() & Qt::ControlModifier )
  {
    if ( !mAnchorPoint )
    {
      mAnchorPoint = std::make_unique<QgsVertexMarker>( mCanvas );
      mAnchorPoint->setIconType( QgsVertexMarker::ICON_CROSS );
    }
    mAnchorPoint->setCenter( e->mapPoint() );
    mFeatureCenterMapCoords = e->mapPoint();
    cadDockWidget()->clear();
    return;
  }

  deleteScalingWidget();

  // Initialize scaling if not yet active
  if ( !mScalingActive )
  {
    mScaling = 1;

    deleteRubberband();

    if ( !vlayer->isEditable() )
    {
      notifyNotEditableLayer();
      return;
    }

    const QgsPointXY layerCoords = toLayerCoordinates( vlayer, e->mapPoint() );
    const double searchRadius = QgsTolerance::vertexSearchRadius( mCanvas->currentLayer(), mCanvas->mapSettings() );
    const QgsRectangle selectRect( layerCoords.x() - searchRadius, layerCoords.y() - searchRadius,
                                   layerCoords.x() + searchRadius, layerCoords.y() + searchRadius );

    mAutoSetAnchorPoint = false;
    if ( !mAnchorPoint )
    {
      mAnchorPoint = std::make_unique<QgsVertexMarker>( mCanvas );
      mAnchorPoint->setIconType( QgsVertexMarker::ICON_CROSS );
      mAutoSetAnchorPoint = true;
    }

    if ( vlayer->selectedFeatureCount() == 0 )
    {
      QgsFeatureIterator fit = vlayer->getFeatures( QgsFeatureRequest().setNoAttributes().setFilterRect( selectRect ) );

      //find the closest feature
      const QgsGeometry pointGeometry = QgsGeometry::fromPointXY( layerCoords );
      if ( pointGeometry.isNull() )
      {
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
        emit messageEmitted( tr( "Could not find a nearby feature in the current layer." ) );
        if ( mAutoSetAnchorPoint )
          mAnchorPoint.reset();
        return;
      }

      mExtent = cf.geometry().boundingBox();
      if ( mAutoSetAnchorPoint )
      {
        mFeatureCenterMapCoords = toMapCoordinates( vlayer, mExtent.center() );
        mAnchorPoint->setCenter( mFeatureCenterMapCoords );
      }
      else
      {
        mFeatureCenterMapCoords =  mAnchorPoint->center();
      }

      mScaledFeatures.clear();
      mScaledFeatures << cf.id(); //todo: take the closest feature, not the first one...
      mOriginalGeometries << cf.geometry();

      mRubberBand = createRubberBand( vlayer->geometryType() );
      mRubberBand->setToGeometry( cf.geometry(), vlayer );
    }
    else
    {
      mScaledFeatures = vlayer->selectedFeatureIds();

      mRubberBand = createRubberBand( vlayer->geometryType() );

      QgsFeature feat;
      QgsFeatureIterator it = vlayer->getSelectedFeatures();
      while ( it.nextFeature( feat ) )
      {
        mRubberBand->addGeometry( feat.geometry(), vlayer, false );
        mOriginalGeometries << feat.geometry();
      }
      mRubberBand->updatePosition();
      mRubberBand->update();
    }

    mScalingActive = true;

    mBaseDistance = e->mapPoint().distance( mFeatureCenterMapCoords );
    mScaling = 1.0;

    createScalingWidget();

    mScalingActive = true;

    return;
  }

  applyScaling( mScaling );
}

void QgsMapToolScaleFeature::cancel()
{
  deleteScalingWidget();
  deleteRubberband();
  QgsVectorLayer *vlayer = currentVectorLayer();
  if ( vlayer->selectedFeatureCount() == 0 || mAutoSetAnchorPoint )
  {
    mAnchorPoint.reset();
  }
  mScalingActive = false;
  mSnapIndicator->setMatch( QgsPointLocator::Match() );
  mCadDockWidget->clear();
}

void QgsMapToolScaleFeature::updateRubberband( double scale )
{
  if ( mScalingActive && mRubberBand )
  {
    mScaling = scale;

    QgsVectorLayer *vlayer = currentVectorLayer();
    if ( !vlayer )
      return;

    const QgsPointXY layerCoords = toLayerCoordinates( vlayer, mFeatureCenterMapCoords );
    QTransform t;
    t.translate( layerCoords.x(), layerCoords.y() );
    t.scale( mScaling, mScaling );
    t.translate( -layerCoords.x(), -layerCoords.y() );

    mRubberBand->reset( vlayer->geometryType() );
    for ( const QgsGeometry &originalGeometry : mOriginalGeometries )
    {
      QgsGeometry geom = originalGeometry;
      geom.transform( t );
      mRubberBand->addGeometry( geom, vlayer );
    }
  }
}

void QgsMapToolScaleFeature::applyScaling( double scale )
{
  mScaling = scale;
  mScalingActive = false;

  QgsVectorLayer *vlayer = currentVectorLayer();
  if ( !vlayer )
  {
    deleteRubberband();
    notifyNotVectorLayer();
    mSnapIndicator->setMatch( QgsPointLocator::Match() );
    mCadDockWidget->clear();
    return;
  }

  //calculations for affine transformation

  vlayer->beginEditCommand( tr( "Features Scaled" ) );

  const QgsPointXY layerCoords = toLayerCoordinates( vlayer, mFeatureCenterMapCoords );
  QTransform t;
  t.translate( layerCoords.x(), layerCoords.y() );
  t.scale( mScaling, mScaling );
  t.translate( -layerCoords.x(), -layerCoords.y() );

  QgsFeatureRequest request;
  request.setFilterFids( mScaledFeatures ).setNoAttributes();
  QgsFeatureIterator fi = vlayer->getFeatures( request );
  QgsFeature feat;
  while ( fi.nextFeature( feat ) )
  {
    if ( !feat.hasGeometry() )
      continue;

    QgsGeometry geom = feat.geometry();
    if ( !( geom.transform( t ) == Qgis::GeometryOperationResult::Success ) )
      continue;

    const QgsFeatureId id = feat.id();
    vlayer->changeGeometry( id, geom );
  }

  deleteScalingWidget();
  deleteRubberband();
  mSnapIndicator->setMatch( QgsPointLocator::Match() );
  mCadDockWidget->clear();

  if ( mAutoSetAnchorPoint )
    mAnchorPoint.reset();

  vlayer->endEditCommand();
  vlayer->triggerRepaint();
}

void QgsMapToolScaleFeature::keyReleaseEvent( QKeyEvent *e )
{
  if ( mScalingActive && e->key() == Qt::Key_Escape )
  {
    cancel();
    return;
  }
  QgsMapToolAdvancedDigitizing::keyReleaseEvent( e );
}

void QgsMapToolScaleFeature::activate()
{
  QgsVectorLayer *vlayer = currentVectorLayer();
  if ( !vlayer )
  {
    return;
  }

  if ( !vlayer->isEditable() )
  {
    return;
  }

  if ( vlayer->selectedFeatureCount() > 0 )
  {
    mExtent = vlayer->boundingBoxOfSelected();
    mFeatureCenterMapCoords = toMapCoordinates( vlayer, mExtent.center() );

    mAnchorPoint = std::make_unique<QgsVertexMarker>( mCanvas );
    mAnchorPoint->setIconType( QgsVertexMarker::ICON_CROSS );
    mAnchorPoint->setCenter( mFeatureCenterMapCoords );
  }
  QgsMapToolAdvancedDigitizing::activate();
}

void QgsMapToolScaleFeature::deleteRubberband()
{
  delete mRubberBand;
  mRubberBand = nullptr;

  mOriginalGeometries.clear();
}

void QgsMapToolScaleFeature::deactivate()
{
  deleteScalingWidget();
  mScalingActive = false;
  mAnchorPoint.reset();
  deleteRubberband();
  mSnapIndicator->setMatch( QgsPointLocator::Match() );
  QgsMapToolAdvancedDigitizing::deactivate();
}

void QgsMapToolScaleFeature::createScalingWidget()
{
  if ( !mCanvas )
  {
    return;
  }

  deleteScalingWidget();

  mScalingWidget = new QgsScaleMagnetWidget( QStringLiteral( "Scaling:" ) );
  QgisApp::instance()->addUserInputWidget( mScalingWidget );
  mScalingWidget->setFocus( Qt::TabFocusReason );

  connect( mScalingWidget, &QgsScaleMagnetWidget::scaleChanged, this, &QgsMapToolScaleFeature::updateRubberband );
  connect( mScalingWidget, &QgsScaleMagnetWidget::scaleEditingFinished, this, &QgsMapToolScaleFeature::applyScaling );
  connect( mScalingWidget, &QgsScaleMagnetWidget::scaleEditingCanceled, this, &QgsMapToolScaleFeature::cancel );
}

void QgsMapToolScaleFeature::deleteScalingWidget()
{
  if ( mScalingWidget )
  {
    disconnect( mScalingWidget, &QgsScaleMagnetWidget::scaleChanged, this, &QgsMapToolScaleFeature::updateRubberband );
    disconnect( mScalingWidget, &QgsScaleMagnetWidget::scaleEditingFinished, this, &QgsMapToolScaleFeature::applyScaling );
    disconnect( mScalingWidget, &QgsScaleMagnetWidget::scaleEditingCanceled, this, &QgsMapToolScaleFeature::cancel );

    mScalingWidget->releaseKeyboard();
    mScalingWidget->deleteLater();
  }
  mScalingWidget = nullptr;
}

