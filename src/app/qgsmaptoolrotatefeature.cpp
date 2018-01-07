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

#include "qgsmaptoolrotatefeature.h"
#include "qgsfeatureiterator.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsrubberband.h"
#include "qgsvectorlayer.h"
#include "qgstolerance.h"
#include "qgsvertexmarker.h"
#include "qgisapp.h"
#include "qgsspinbox.h"
#include "qgsdoublespinbox.h"

#include <QMouseEvent>
#include <QSettings>
#include <QEvent>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>

#include <limits>
#include <cmath>

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
  mAngleSpinBox->setSuffix( trUtf8( "°" ) );
  mAngleSpinBox->setSingleStep( 1 );
  mAngleSpinBox->setValue( 0 );
  mAngleSpinBox->setShowClearButton( false );
  mLayout->addWidget( mAngleSpinBox );

  mMagnetSpinBox = new QgsSpinBox( this );
  mMagnetSpinBox->setMinimum( 0 );
  mMagnetSpinBox->setMaximum( 180 );
  mMagnetSpinBox->setPrefix( tr( "Snap to " ) );
  mMagnetSpinBox->setSuffix( trUtf8( "°" ) );
  mMagnetSpinBox->setSingleStep( 15 );
  mMagnetSpinBox->setValue( 0 );
  mMagnetSpinBox->setClearValue( 0, tr( "No snapping" ) );
  mLayout->addWidget( mMagnetSpinBox );

  // connect signals
  mAngleSpinBox->installEventFilter( this );
  connect( mAngleSpinBox, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), this, &QgsAngleMagnetWidget::angleSpinBoxValueChanged );

  // config focus
  setFocusProxy( mAngleSpinBox );
}

void QgsAngleMagnetWidget::setAngle( double angle )
{
  const int magnet = mMagnetSpinBox->value();
  if ( magnet )
  {
    mAngleSpinBox->setValue( std::round( angle / magnet ) * magnet );
  }
  else
  {
    mAngleSpinBox->setValue( angle );
  }
}

double QgsAngleMagnetWidget::angle()
{
  return mAngleSpinBox->value();
}

void QgsAngleMagnetWidget::setMagnet( int magnet )
{
  mMagnetSpinBox->setValue( magnet );
}


bool QgsAngleMagnetWidget::eventFilter( QObject *obj, QEvent *ev )
{
  if ( obj == mAngleSpinBox && ev->type() == QEvent::KeyPress )
  {
    QKeyEvent *event = static_cast<QKeyEvent *>( ev );
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

QgsMapToolRotateFeature::QgsMapToolRotateFeature( QgsMapCanvas *canvas )
  : QgsMapToolEdit( canvas )
  , mRotation( 0 )
  , mRotationOffset( 0 )
  , mRotationActive( false )
{
}

QgsMapToolRotateFeature::~QgsMapToolRotateFeature()
{
  deleteRotationWidget();
  delete mAnchorPoint;
  deleteRubberband();
}

void QgsMapToolRotateFeature::canvasMoveEvent( QgsMapMouseEvent *e )
{
  if ( mRotationActive )
  {
    const double XDistance = e->pos().x() - mStPoint.x();
    const double YDistance = e->pos().y() - mStPoint.y();
    double rotation = std::atan2( YDistance, XDistance ) * ( 180 / M_PI );

    if ( mRotationWidget )
    {
      mRotationWidget->setAngle( rotation - mRotationOffset );
      mRotationWidget->setFocus( Qt::TabFocusReason );
    }
    else
    {
      updateRubberband( rotation - mRotationOffset );
    }
  }
}


void QgsMapToolRotateFeature::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  deleteRotationWidget();

  if ( !mCanvas )
  {
    return;
  }

  QgsVectorLayer *vlayer = currentVectorLayer();
  if ( !vlayer )
  {
    deleteRubberband();
    notifyNotVectorLayer();
    return;
  }

  if ( e->button() == Qt::RightButton )
  {
    deleteRubberband();
    mRotationActive = false;
    return;
  }

  // place anchor point on CTRL + click
  if ( e->modifiers() & Qt::ControlModifier )
  {
    if ( !mAnchorPoint )
    {
      return;
    }
    mAnchorPoint->setCenter( toMapCoordinates( e->pos() ) );
    mStartPointMapCoords = toMapCoordinates( e->pos() );
    mStPoint = e->pos();
    return;
  }

  // Initialize rotation if not yet active
  if ( !mRotationActive )
  {
    mRotation = 0;
    mRotationOffset = 0;

    deleteRubberband();

    mInitialPos = e->pos();

    if ( !vlayer->isEditable() )
    {
      notifyNotEditableLayer();
      return;
    }

    QgsPointXY layerCoords = toLayerCoordinates( vlayer, e->pos() );
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
        emit messageEmitted( tr( "Could not find a nearby feature in the current layer." ) );
        return;
      }

      QgsRectangle bound = cf.geometry().boundingBox();
      mStartPointMapCoords = toMapCoordinates( vlayer, bound.center() );

      if ( !mAnchorPoint )
      {
        mAnchorPoint = new QgsVertexMarker( mCanvas );
      }
      mAnchorPoint->setIconType( QgsVertexMarker::ICON_CROSS );
      mAnchorPoint->setCenter( mStartPointMapCoords );

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
        mRubberBand->addGeometry( feat.geometry(), vlayer );
      }
    }

    mRubberBand->setColor( QColor( 255, 0, 0, 65 ) );
    mRubberBand->setWidth( 2 );
    mRubberBand->show();

    double XDistance = mInitialPos.x() - mAnchorPoint->x();
    double YDistance = mInitialPos.y() - mAnchorPoint->y();
    mRotationOffset = std::atan2( YDistance, XDistance ) * ( 180 / M_PI );

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

void QgsMapToolRotateFeature::updateRubberband( double rotation )
{
  if ( mRotationActive )
  {
    mRotation = rotation;

    mStPoint = toCanvasCoordinates( mStartPointMapCoords );
    double offsetX = mStPoint.x() - mRubberBand->x();
    double offsetY = mStPoint.y() - mRubberBand->y();

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
    return;
  }

  //calculations for affine transformation
  double angle = -1 * mRotation * ( M_PI / 180 );
  QgsPointXY anchorPoint = toLayerCoordinates( vlayer, mStartPointMapCoords );
  double a = std::cos( angle );
  double b = -1 * std::sin( angle );
  double c = anchorPoint.x() - std::cos( angle ) * anchorPoint.x() + std::sin( angle ) * anchorPoint.y();
  double d = std::sin( angle );
  double ee = std::cos( angle );
  double f = anchorPoint.y() - std::sin( angle ) * anchorPoint.x() - std::cos( angle ) * anchorPoint.y();

  vlayer->beginEditCommand( tr( "Features Rotated" ) );

  int start;
  if ( vlayer->geometryType() == 2 )
  {
    start = 1;
  }
  else
  {
    start = 0;
  }

  int i = 0;
  Q_FOREACH ( QgsFeatureId id, mRotatedFeatures )
  {
    QgsFeature feat;
    vlayer->getFeatures( QgsFeatureRequest().setFilterFid( id ) ).nextFeature( feat );
    QgsGeometry geom = feat.geometry();
    i = start;

    QgsPointXY vertex = geom.vertexAt( i );
    while ( vertex != QgsPointXY( 0, 0 ) )
    {
      double newX = a * vertex.x() + b * vertex.y() + c;
      double newY = d * vertex.x() + ee * vertex.y() + f;

      vlayer->moveVertex( newX, newY, id, i );
      i = i + 1;
      vertex = geom.vertexAt( i );
    }

  }

  double anchorX = a * anchorPoint.x() + b * anchorPoint.y() + c;
  double anchorY = d * anchorPoint.x() + ee * anchorPoint.y() + f;

  mAnchorPoint->setCenter( QgsPointXY( anchorX, anchorY ) );

  deleteRotationWidget();
  deleteRubberband();

  vlayer->endEditCommand();
  vlayer->triggerRepaint();
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

  if ( vlayer->selectedFeatureCount() == 0 )
  {
    return;
  }
  else
  {
    QgsRectangle bound = vlayer->boundingBoxOfSelected();
    mStartPointMapCoords = toMapCoordinates( vlayer, bound.center() );

    mAnchorPoint = new QgsVertexMarker( mCanvas );
    mAnchorPoint->setIconType( QgsVertexMarker::ICON_CROSS );
    mAnchorPoint->setCenter( mStartPointMapCoords );

    mStPoint = toCanvasCoordinates( mStartPointMapCoords );

    QgsMapTool::activate();
  }
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
  delete mAnchorPoint;
  mAnchorPoint = nullptr;
  mRotationOffset = 0;
  deleteRubberband();
  QgsMapTool::deactivate();
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
}

void QgsMapToolRotateFeature::deleteRotationWidget()
{
  if ( mRotationWidget )
  {
    disconnect( mRotationWidget, &QgsAngleMagnetWidget::angleChanged, this, &QgsMapToolRotateFeature::updateRubberband );
    disconnect( mRotationWidget, &QgsAngleMagnetWidget::angleEditingFinished, this, &QgsMapToolRotateFeature::applyRotation );
    mRotationWidget->releaseKeyboard();
    mRotationWidget->deleteLater();
  }
  mRotationWidget = nullptr;
}


