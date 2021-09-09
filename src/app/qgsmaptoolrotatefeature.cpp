/***************************************************************************
    qgsmaptoolrotatefeature.cpp  -  map tool for rotating features by mouse drag
    ---------------------
    begin                : January 2012
    copyright            : (C) 2012 by Vinayan Parameswaran
    email                : vinayan123 at gmail dot com
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
#include "qgsmaptoolrotatefeature.h"
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


QgsAngleMagnetWidget::QgsAngleMagnetWidget( const QString &label, QWidget *parent )
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

  mAngleSpinBox = new QgsDoubleSpinBox( this );
  mAngleSpinBox->setMinimum( -360 );
  mAngleSpinBox->setMaximum( 360 );
  mAngleSpinBox->setSuffix( tr( "°" ) );
  mAngleSpinBox->setSingleStep( 1 );
  mAngleSpinBox->setValue( 0 );
  mAngleSpinBox->setClearValue( 0 );
  mAngleSpinBox->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Preferred );
  mLayout->addWidget( mAngleSpinBox );

  mMagnetSpinBox = new QgsSpinBox( this );
  mMagnetSpinBox->setMinimum( 0 );
  mMagnetSpinBox->setMaximum( 180 );
  mMagnetSpinBox->setPrefix( tr( "Snap to " ) );
  mMagnetSpinBox->setSuffix( tr( "°" ) );
  mMagnetSpinBox->setSingleStep( 15 );
  mMagnetSpinBox->setValue( 0 );
  mMagnetSpinBox->setClearValue( 0, tr( "No snapping" ) );
  //mMagnetSpinBox->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Preferred );
  mMagnetSpinBox->setMinimumWidth( 120 );
  mLayout->addWidget( mMagnetSpinBox );

  // connect signals
  mAngleSpinBox->installEventFilter( this );
  connect( mAngleSpinBox, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), this, &QgsAngleMagnetWidget::angleSpinBoxValueChanged );

  // config focus
  setFocusProxy( mAngleSpinBox );
}

void QgsAngleMagnetWidget::setAngle( double angle )
{
  const int m = magnet();
  if ( m )
  {
    mAngleSpinBox->setValue( std::round( angle / m ) * m );
  }
  else
  {
    mAngleSpinBox->setValue( angle );
  }
}

double QgsAngleMagnetWidget::angle() const
{
  return mAngleSpinBox->value();
}

void QgsAngleMagnetWidget::setMagnet( int magnet )
{
  mMagnetSpinBox->setValue( magnet );
}

int QgsAngleMagnetWidget::magnet() const
{
  return mMagnetSpinBox->value();
}

bool QgsAngleMagnetWidget::eventFilter( QObject *obj, QEvent *ev )
{
  if ( obj == mAngleSpinBox && ev->type() == QEvent::KeyPress )
  {
    QKeyEvent *event = static_cast<QKeyEvent *>( ev );
    if ( event->key() == Qt::Key_Escape )
    {
      emit angleEditingCanceled();
      return true;
    }
    if ( event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return )
    {
      emit angleEditingFinished( angle() );
      return true;
    }
  }

  return false;
}

void QgsAngleMagnetWidget::angleSpinBoxValueChanged( double angle )
{
  emit angleChanged( angle );
}

//
// QgsMapToolRotateFeature
//

QgsMapToolRotateFeature::QgsMapToolRotateFeature( QgsMapCanvas *canvas )
  : QgsMapToolAdvancedDigitizing( canvas, QgisApp::instance()->cadDockWidget() )
  , mSnapIndicator( std::make_unique< QgsSnapIndicator>( canvas ) )
{
  mToolName = tr( "Rotate feature" );
}

QgsMapToolRotateFeature::~QgsMapToolRotateFeature()
{
  deleteRotationWidget();
  mAnchorPoint.reset();
  deleteRubberband();
  mSnapIndicator->setMatch( QgsPointLocator::Match() );
}

void QgsMapToolRotateFeature::cadCanvasMoveEvent( QgsMapMouseEvent *e )
{
  mSnapIndicator->setMatch( e->mapPointMatch() );

  if ( mRotationActive )
  {
    const QgsPointXY pt = e->mapPoint();
    const double XDistance = pt.x() - mStartPointMapCoords.x();
    const double YDistance = pt.y() - mStartPointMapCoords.y();
    double rotation = std::atan2( XDistance, YDistance ) * ( 180 / M_PI ) - mRotationOffset;

    if ( mRotationWidget )
    {
      disconnect( mRotationWidget, &QgsAngleMagnetWidget::angleChanged, this, &QgsMapToolRotateFeature::updateRubberband );
      mRotationWidget->setAngle( rotation );
      mRotationWidget->setFocus( Qt::TabFocusReason );
      mRotationWidget->editor()->selectAll();
      connect( mRotationWidget, &QgsAngleMagnetWidget::angleChanged, this, &QgsMapToolRotateFeature::updateRubberband );
      if ( mRotationWidget->magnet() )
      {
        rotation = mRotationWidget->angle();
      }
    }
    updateRubberband( rotation );
  }
}

void QgsMapToolRotateFeature::cadCanvasReleaseEvent( QgsMapMouseEvent *e )
{
  if ( !mCanvas )
  {
    return;
  }

  QgsVectorLayer *vlayer = currentVectorLayer();
  if ( !vlayer )
  {
    deleteRotationWidget();
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
    mStartPointMapCoords = e->mapPoint();
    mStPoint = e->pos();
    cadDockWidget()->clear();
    return;
  }

  deleteRotationWidget();

  // Initialize rotation if not yet active
  if ( !mRotationActive )
  {
    mRotation = 0;
    mRotationOffset = 0;

    deleteRubberband();

    mInitialPos = e->mapPoint();

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
      QgsFeatureIterator fit = vlayer->getFeatures( QgsFeatureRequest().setFilterRect( selectRect ).setNoAttributes() );

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
        return;
      }

      const QgsRectangle bound = cf.geometry().boundingBox();
      if ( mAutoSetAnchorPoint )
      {
        mStartPointMapCoords = toMapCoordinates( vlayer, bound.center() );
        mAnchorPoint->setCenter( mStartPointMapCoords );
      }
      else
      {
        mStartPointMapCoords = mAnchorPoint->center();
      }

      mStPoint = toCanvasCoordinates( mStartPointMapCoords );

      mRotatedFeatures.clear();
      mRotatedFeatures << cf.id(); //todo: take the closest feature, not the first one...

      mRubberBand = createRubberBand( vlayer->geometryType() );
      mRubberBand->setToGeometry( cf.geometry(), vlayer );
    }
    else
    {
      mRotatedFeatures = vlayer->selectedFeatureIds();

      mRubberBand = createRubberBand( vlayer->geometryType() );

      QgsFeature feat;
      QgsFeatureIterator it = vlayer->getSelectedFeatures();
      while ( it.nextFeature( feat ) )
      {
        mRubberBand->addGeometry( feat.geometry(), vlayer, false );
      }
      mRubberBand->updatePosition();
      mRubberBand->update();
    }

    mRubberBand->show();

    const double XDistance = mInitialPos.x() - mAnchorPoint->center().x();
    const double YDistance = mInitialPos.y() - mAnchorPoint->center().y();
    mRotationOffset = std::atan2( XDistance, YDistance ) * ( 180 / M_PI );

    createRotationWidget();
    if ( e->modifiers() & Qt::ShiftModifier )
    {
      if ( mRotationWidget )
      {
        mRotationWidget->setMagnet( 45 );
      }
    }

    mRotationActive = true;

    return;
  }

  applyRotation( mRotation );
}

void QgsMapToolRotateFeature::cancel()
{
  deleteRotationWidget();
  deleteRubberband();
  QgsVectorLayer *vlayer = currentVectorLayer();
  if ( vlayer->selectedFeatureCount() == 0 || mAutoSetAnchorPoint )
  {
    mAnchorPoint.reset();
  }
  mRotationActive = false;
  mSnapIndicator->setMatch( QgsPointLocator::Match() );
  mCadDockWidget->clear();
}

void QgsMapToolRotateFeature::updateRubberband( double rotation )
{
  if ( mRotationActive )
  {
    mRotation = rotation;

    mStPoint = toCanvasCoordinates( mStartPointMapCoords );
    const double offsetX = mStPoint.x() - mRubberBand->x();
    const double offsetY = mStPoint.y() - mRubberBand->y();

    if ( mRubberBand )
    {
      mRubberBand->setTransform( QTransform().translate( offsetX, offsetY ).rotate( mRotation ).translate( -1 * offsetX, -1 * offsetY ) );
      mRubberBand->update();
    }
  }
}

void QgsMapToolRotateFeature::applyRotation( double rotation )
{
  mRotation = rotation;
  mRotationActive = false;

  QgsVectorLayer *vlayer = currentVectorLayer();
  if ( !vlayer )
  {
    deleteRubberband();
    notifyNotVectorLayer();
    mSnapIndicator->setMatch( QgsPointLocator::Match() );
    mCadDockWidget->clear();
    return;
  }

  const QgsPointXY anchorPoint = toLayerCoordinates( vlayer, mStartPointMapCoords );

  vlayer->beginEditCommand( tr( "Features Rotated" ) );

  QgsFeatureRequest request;
  request.setFilterFids( mRotatedFeatures ).setNoAttributes();
  QgsFeatureIterator fi = vlayer->getFeatures( request );
  QgsFeature f;
  while ( fi.nextFeature( f ) )
  {
    const QgsFeatureId id = f.id();
    QgsGeometry geom = f.geometry();
    geom.rotate( mRotation, anchorPoint );
    vlayer->changeGeometry( id, geom );
  }

  deleteRotationWidget();
  deleteRubberband();
  mSnapIndicator->setMatch( QgsPointLocator::Match() );
  mCadDockWidget->clear();

  if ( mAutoSetAnchorPoint )
    mAnchorPoint.reset();

  vlayer->endEditCommand();
  vlayer->triggerRepaint();
}

void QgsMapToolRotateFeature::keyReleaseEvent( QKeyEvent *e )
{
  if ( mRotationActive && e->key() == Qt::Key_Escape )
  {
    cancel();
    return;
  }
  QgsMapToolAdvancedDigitizing::keyReleaseEvent( e );
}

void QgsMapToolRotateFeature::activate()
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
    const QgsRectangle bound = vlayer->boundingBoxOfSelected();
    mStartPointMapCoords = toMapCoordinates( vlayer, bound.center() );

    mAnchorPoint = std::make_unique<QgsVertexMarker>( mCanvas );
    mAnchorPoint->setIconType( QgsVertexMarker::ICON_CROSS );
    mAnchorPoint->setCenter( mStartPointMapCoords );

    mStPoint = toCanvasCoordinates( mStartPointMapCoords );
  }
  QgsMapToolAdvancedDigitizing::activate();
}

void QgsMapToolRotateFeature::deleteRubberband()
{
  delete mRubberBand;
  mRubberBand = nullptr;
}

void QgsMapToolRotateFeature::deactivate()
{
  deleteRotationWidget();
  mRotationActive = false;
  mRotationOffset = 0;
  mAnchorPoint.reset();
  deleteRubberband();
  mSnapIndicator->setMatch( QgsPointLocator::Match() );
  QgsMapToolAdvancedDigitizing::deactivate();
}

void QgsMapToolRotateFeature::createRotationWidget()
{
  if ( !mCanvas )
  {
    return;
  }

  deleteRotationWidget();

  mRotationWidget = new QgsAngleMagnetWidget( QStringLiteral( "Rotation:" ) );
  QgisApp::instance()->addUserInputWidget( mRotationWidget );
  mRotationWidget->setFocus( Qt::TabFocusReason );

  connect( mRotationWidget, &QgsAngleMagnetWidget::angleChanged, this, &QgsMapToolRotateFeature::updateRubberband );
  connect( mRotationWidget, &QgsAngleMagnetWidget::angleEditingFinished, this, &QgsMapToolRotateFeature::applyRotation );
  connect( mRotationWidget, &QgsAngleMagnetWidget::angleEditingCanceled, this, &QgsMapToolRotateFeature::cancel );
}

void QgsMapToolRotateFeature::deleteRotationWidget()
{
  if ( mRotationWidget )
  {
    disconnect( mRotationWidget, &QgsAngleMagnetWidget::angleChanged, this, &QgsMapToolRotateFeature::updateRubberband );
    disconnect( mRotationWidget, &QgsAngleMagnetWidget::angleEditingFinished, this, &QgsMapToolRotateFeature::applyRotation );
    disconnect( mRotationWidget, &QgsAngleMagnetWidget::angleEditingCanceled, this, &QgsMapToolRotateFeature::cancel );

    mRotationWidget->releaseKeyboard();
    mRotationWidget->deleteLater();
  }
  mRotationWidget = nullptr;
}


