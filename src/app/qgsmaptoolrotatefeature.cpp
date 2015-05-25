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
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsrubberband.h"
#include "qgsvectordataprovider.h"
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
#include <math.h>

#define PI 3.14159265

QgsAngleMagnetWidget::QgsAngleMagnetWidget( QString label , QWidget *parent )
    : QWidget( parent )
{
  mLayout = new QHBoxLayout( this );
  mLayout->setContentsMargins( 0, 0, 0, 0 );
  //mLayout->setAlignment( Qt::AlignLeft );
  setLayout( mLayout );

  if ( !label.isNull() )
  {
    QLabel* lbl = new QLabel( label, this );
    lbl->setAlignment( Qt::AlignRight | Qt::AlignCenter );
    mLayout->addWidget( lbl );
  }

  mAngleSpinBox = new QgsDoubleSpinBox( this );
  mAngleSpinBox->setMinimum( -360 );
  mAngleSpinBox->setMaximum( 360 );
  mAngleSpinBox->setSuffix( QString::fromUtf8( "°" ) );
  mAngleSpinBox->setSingleStep( 1 );
  mAngleSpinBox->setValue( 0 );
  mAngleSpinBox->setShowClearButton( false );
  mLayout->addWidget( mAngleSpinBox );

  mMagnetSpinBox = new QgsSpinBox( this );
  mMagnetSpinBox->setMinimum( 0 );
  mMagnetSpinBox->setMaximum( 180 );
  mMagnetSpinBox->setPrefix( tr( "Snap to " ) );
  mMagnetSpinBox->setSuffix( QString::fromUtf8( "°" ) );
  mMagnetSpinBox->setSingleStep( 15 );
  mMagnetSpinBox->setValue( 0 );
  mMagnetSpinBox->setClearValue( 0, tr( "No snapping" ) );
  mLayout->addWidget( mMagnetSpinBox );

  // connect signals
  mAngleSpinBox->installEventFilter( this );
  connect( mAngleSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( angleSpinBoxValueChanged( double ) ) );

  // config focus
  setFocusProxy( mAngleSpinBox );
}

QgsAngleMagnetWidget::~QgsAngleMagnetWidget()
{
}

void QgsAngleMagnetWidget::setAngle( double angle )
{
  const int magnet = mMagnetSpinBox->value();
  if ( magnet )
  {
    mAngleSpinBox->setValue( qRound( angle / magnet ) * magnet );
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

QgsMapToolRotateFeature::QgsMapToolRotateFeature( QgsMapCanvas* canvas )
    : QgsMapToolEdit( canvas )
    , mRubberBand( 0 )
    , mRotation( 0 )
    , mRotationOffset( 0 )
    , mAnchorPoint( 0 )
    , mRotationActive( false )
    , mRotationWidget( 0 )
{
}

QgsMapToolRotateFeature::~QgsMapToolRotateFeature()
{
  deleteRotationWidget();
  delete mAnchorPoint;
  deleteRubberband();
}

void QgsMapToolRotateFeature::canvasMoveEvent( QMouseEvent * e )
{
  if ( mRotationActive )
  {
    const double XDistance = e->pos().x() - mStPoint.x();
    const double YDistance = e->pos().y() - mStPoint.y();
    double rotation = atan2( YDistance, XDistance ) * ( 180 / PI );

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


void QgsMapToolRotateFeature::canvasReleaseEvent( QMouseEvent * e )
{
  deleteRotationWidget();

  if ( !mCanvas )
  {
    return;
  }

  QgsVectorLayer* vlayer = currentVectorLayer();
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

    QgsPoint layerCoords = toLayerCoordinates( vlayer, e->pos() );
    double searchRadius = QgsTolerance::vertexSearchRadius( mCanvas->currentLayer(), mCanvas->mapSettings() );
    QgsRectangle selectRect( layerCoords.x() - searchRadius, layerCoords.y() - searchRadius,
                             layerCoords.x() + searchRadius, layerCoords.y() + searchRadius );

    if ( vlayer->selectedFeatureCount() == 0 )
    {
      QgsFeatureIterator fit = vlayer->getFeatures( QgsFeatureRequest().setFilterRect( selectRect ).setSubsetOfAttributes( QgsAttributeList() ) );

      //find the closest feature
      QgsGeometry* pointGeometry = QgsGeometry::fromPoint( layerCoords );
      if ( !pointGeometry )
      {
        return;
      }

      double minDistance = std::numeric_limits<double>::max();

      QgsFeature cf;
      QgsFeature f;
      while ( fit.nextFeature( f ) )
      {
        if ( f.constGeometry() )
        {
          double currentDistance = pointGeometry->distance( *f.constGeometry() );
          if ( currentDistance < minDistance )
          {
            minDistance = currentDistance;
            cf = f;
          }
        }
      }

      delete pointGeometry;

      if ( minDistance == std::numeric_limits<double>::max() )
      {
        return;
      }

      QgsRectangle bound = cf.constGeometry()->boundingBox();
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
      mRubberBand->setToGeometry( cf.constGeometry(), vlayer );
    }
    else
    {
      mRotatedFeatures = vlayer->selectedFeaturesIds();

      mRubberBand = createRubberBand( vlayer->geometryType() );

      QgsFeature feat;
      QgsFeatureIterator it = vlayer->selectedFeaturesIterator();
      while ( it.nextFeature( feat ) )
      {
        mRubberBand->addGeometry( feat.constGeometry(), vlayer );
      }
    }

    mRubberBand->setColor( QColor( 255, 0, 0, 65 ) );
    mRubberBand->setWidth( 2 );
    mRubberBand->show();

    double XDistance = mInitialPos.x() - mAnchorPoint->x();
    double YDistance = mInitialPos.y() - mAnchorPoint->y() ;
    mRotationOffset = atan2( YDistance, XDistance ) * ( 180 / PI );

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

  QgsVectorLayer* vlayer = currentVectorLayer();
  if ( !vlayer )
  {
    deleteRubberband();
    notifyNotVectorLayer();
    return;
  }

  //calculations for affine transformation
  double angle = -1 * mRotation * ( PI / 180 );
  QgsPoint anchorPoint = toLayerCoordinates( vlayer, mStartPointMapCoords );
  double a = cos( angle );
  double b = -1 * sin( angle );
  double c = anchorPoint.x() - cos( angle ) * anchorPoint.x() + sin( angle ) * anchorPoint.y();
  double d = sin( angle );
  double ee = cos( angle );
  double f = anchorPoint.y() - sin( angle ) * anchorPoint.x() - cos( angle ) * anchorPoint.y();

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
  foreach ( QgsFeatureId id, mRotatedFeatures )
  {
    QgsFeature feat;
    vlayer->getFeatures( QgsFeatureRequest().setFilterFid( id ) ).nextFeature( feat );
    const QgsGeometry* geom = feat.constGeometry();
    i = start;

    QgsPoint vertex = geom->vertexAt( i );
    while ( vertex != QgsPoint( 0, 0 ) )
    {
      double newX = a * vertex.x() + b * vertex.y() + c;
      double newY = d * vertex.x() + ee * vertex.y() + f;

      vlayer->moveVertex( newX, newY, id, i );
      i = i + 1;
      vertex = geom->vertexAt( i );
    }

  }

  double anchorX = a * anchorPoint.x() + b * anchorPoint.y() + c;
  double anchorY = d * anchorPoint.x() + ee * anchorPoint.y() + f;

  mAnchorPoint->setCenter( QgsPoint( anchorX, anchorY ) );

  deleteRotationWidget();
  deleteRubberband();

  mCanvas->refresh();
  vlayer->endEditCommand();
}

void QgsMapToolRotateFeature::activate()
{

  QgsVectorLayer* vlayer = currentVectorLayer();
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
  mRubberBand = 0;
}

void QgsMapToolRotateFeature::deactivate()
{
  deleteRotationWidget();
  mRotationActive = false;
  delete mAnchorPoint;
  mAnchorPoint = 0;
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

  mRotationWidget = new QgsAngleMagnetWidget( "Rotation:" );
  QgisApp::instance()->addUserInputWidget( mRotationWidget );
  mRotationWidget->setFocus( Qt::TabFocusReason );

  QObject::connect( mRotationWidget, SIGNAL( angleChanged( double ) ), this, SLOT( updateRubberband( double ) ) );
  QObject::connect( mRotationWidget, SIGNAL( angleEditingFinished( double ) ), this, SLOT( applyRotation( double ) ) );
}

void QgsMapToolRotateFeature::deleteRotationWidget()
{
  if ( mRotationWidget )
  {
    QObject::disconnect( mRotationWidget, SIGNAL( angleChanged( double ) ), this, SLOT( updateRubberband( double ) ) );
    QObject::disconnect( mRotationWidget, SIGNAL( angleEditingFinished( double ) ), this, SLOT( applyRotation( double ) ) );
    mRotationWidget->releaseKeyboard();
    mRotationWidget->deleteLater();
  }
  mRotationWidget = 0;
}


