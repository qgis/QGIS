/***************************************************************************
    qgsmaptoolscalefeature.cpp  -  map tool for scaling features by mouse drag
    ---------------------
    begin                :
    copyright            :
    email                :
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

  mMagnetSpinBox = new QgsSpinBox( this );
  mMagnetSpinBox->setMinimum( 0 );
  mMagnetSpinBox->setMaximum( 180 );
  mMagnetSpinBox->setPrefix( tr( "Snap to " ) );
  mMagnetSpinBox->setSuffix( tr( "x" ) );
  mMagnetSpinBox->setSingleStep( 5 );
  mMagnetSpinBox->setValue( 0 );
  mMagnetSpinBox->setClearValue( 0, tr( "No snapping" ) );
  //mMagnetSpinBox->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Preferred );
  mMagnetSpinBox->setMinimumWidth( 120 );
  mLayout->addWidget( mMagnetSpinBox );

  // connect signals
  mScaleSpinBox->installEventFilter( this );
  connect( mScaleSpinBox, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), this, &QgsScaleMagnetWidget::scaleSpinBoxValueChanged );

  // config focus
  setFocusProxy( mScaleSpinBox );
}

void QgsScaleMagnetWidget::setScale( double scale )
{
  const int m = magnet();
  if ( m )
  {
    mScaleSpinBox->setValue( std::round( scale / m ) * m );
  }
  else
  {
    mScaleSpinBox->setValue( scale );
  }
}

double QgsScaleMagnetWidget::scale() const
{
  return mScaleSpinBox->value();
}

void QgsScaleMagnetWidget::setMagnet( int magnet )
{
  mMagnetSpinBox->setValue( magnet );
}

int QgsScaleMagnetWidget::magnet() const
{
  return mMagnetSpinBox->value();
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

QgsMapToolScaleFeature::QgsMapToolScaleFeature( QgsMapCanvas *canvas )
  : QgsMapToolEdit( canvas )
  , mScaling( 0 )
  , mScalingActive( false )
{
}

QgsMapToolScaleFeature::~QgsMapToolScaleFeature()
{
  deleteScalingWidget();
  mAnchorPoint.reset();
  deleteRubberband();
}

void QgsMapToolScaleFeature::canvasMoveEvent( QgsMapMouseEvent *e )
{
  if ( mBaseDistance == 0)
  {

  }
  if ( mScalingActive )
  {
    const double distance = mFeatureCenter.distance( toLayerCoordinates( mLayer, e->mapPoint() ) );
    double scale =  distance / mBaseDistance; // min 0 or no limit?

    if ( mScalingWidget )
    {
      disconnect( mScalingWidget, &QgsScaleMagnetWidget::scaleChanged, this, &QgsMapToolScaleFeature::updateRubberband );
      mScalingWidget->setScale( scale );
      mScalingWidget->setFocus( Qt::TabFocusReason );
      mScalingWidget->editor()->selectAll();
      connect( mScalingWidget, &QgsScaleMagnetWidget::scaleChanged, this, &QgsMapToolScaleFeature::updateRubberband );
      if ( mScalingWidget->magnet() )
      {
        scale = mScalingWidget->scale();
      }
    }
    updateRubberband( scale );
  }
}


void QgsMapToolScaleFeature::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  if ( !mCanvas )
  {
    return;
  }

  //QgsVectorLayer *mLayer = currentVectorLayer();
  if ( !mLayer )
  {
    deleteScalingWidget();
    deleteRubberband();
    notifyNotVectorLayer();
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
      return;
    }
    mFeatureCenter = toLayerCoordinates( mLayer, e->mapPoint() );
    mAnchorPoint->setCenter( mFeatureCenter );
    return;
  }

  deleteScalingWidget();

  // Initialize scaling if not yet active
  if ( !mScalingActive )
  {
    mScaling = 1;

    deleteRubberband();

    mInitialCanvasPos = e->pos();

    if ( !mLayer->isEditable() )
    {
      notifyNotEditableLayer();
      return;
    }

    QgsPointXY layerInitCoords = toLayerCoordinates( mLayer, e->mapPoint() );
    double searchRadius = QgsTolerance::vertexSearchRadius( mCanvas->currentLayer(), mCanvas->mapSettings() );
    QgsRectangle selectRect( layerInitCoords.x() - searchRadius, layerInitCoords.y() - searchRadius,
                             layerInitCoords.x() + searchRadius, layerInitCoords.y() + searchRadius );

    if ( !mAnchorPoint )
    {
      mAnchorPoint = qgis::make_unique<QgsVertexMarker>( mCanvas );
      mAnchorPoint->setIconType( QgsVertexMarker::ICON_CROSS );
    }

    if ( mLayer->selectedFeatureCount() == 0 )
    {
      QgsFeatureIterator fit = mLayer->getFeatures( QgsFeatureRequest().setNoAttributes().setFilterRect( selectRect ) );

      //find the closest feature
      QgsGeometry pointGeometry = QgsGeometry().fromPointXY( layerInitCoords );// layerInitCoords )
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

      mExtent = cf.geometry().boundingBox();
      mFeatureCenter = mExtent.center();

      mScaledFeatures.clear();
      mScaledFeatures << cf.id(); //todo: take the closest feature, not the first one...

      mRubberBand = createRubberBand( mLayer->geometryType() );
      mRubberBand->setToGeometry( cf.geometry(), mLayer );
    }
    else
    {
      mScaledFeatures = mLayer->selectedFeatureIds();

      mRubberBand = createRubberBand( mLayer->geometryType() );

      QgsFeature feat;
      QgsFeatureIterator it = mLayer->getSelectedFeatures();
      while ( it.nextFeature( feat ) )
      {
        mRubberBand->addGeometry( feat.geometry(), mLayer );
      }
    }
    mScalingActive = true;
    mMapAnchor = toMapCoordinates( mLayer,mFeatureCenter );
    recenterRubberband( 0.0 );
    mBaseDistance = toLayerCoordinates( mLayer, e->mapPoint() ).distance( mFeatureCenter );
    mScaling = 1.0;
    connect( mCanvas, &QgsMapCanvas::scaleChanged, this, &QgsMapToolScaleFeature::recenterRubberband );

    createScalingWidget();


    return;
  }

  applyScaling( mScaling );
}

void QgsMapToolScaleFeature::cancel()
{
  deleteScalingWidget();
  deleteRubberband();
  QgsVectorLayer *mLayer = currentVectorLayer();
  if ( mLayer->selectedFeatureCount() == 0 )
  {
    mAnchorPoint.reset();
  }
  mScalingActive = false;
}

void QgsMapToolScaleFeature::updateRubberband( double scale )
{
  if ( mScalingActive )
  {
    mScaling = scale;

    double offsetx = ( 1 - mScaling ) * mRubberScale.x();
    double offsety = ( 1 - mScaling ) *  mRubberScale.y();

    if ( mRubberBand )
    {
      mRubberBand->setTransform( QTransform( mScaling, 0, 0, mScaling, offsetx, offsety ) );
      mRubberBand->update();
    }
  }
}


void QgsMapToolScaleFeature::applyScaling( double scale )
{
  mScaling = scale;
  mScalingActive = false;

  if ( !mLayer )
  {
    deleteRubberband();
    notifyNotVectorLayer();
    return;
  }

  //calculations for affine transformation

  mLayer->beginEditCommand( tr( "Features Scaled" ) );

  int start = ( mLayer->geometryType() == 2 )? 1 : 0;

  for ( QgsFeatureId id : qgis::as_const( mScaledFeatures ) )
  {
    QgsFeature feat;
    mLayer->getFeatures( QgsFeatureRequest().setFilterFid( id ) ).nextFeature( feat );
    QgsGeometry geom = feat.geometry();
    int i = start;
    QgsPointXY vertex = geom.vertexAt( i );
    while ( !vertex.isEmpty() )
    {
      // for to maintain feature position use the center of the feature bbox and not the whole selection
      double newX = vertex.x() + ( ( vertex.x() - mFeatureCenter.x() ) * (scale - 1) );
      double newY = vertex.y() + ( ( vertex.y() - mFeatureCenter.y() ) * (scale - 1) );

      mLayer->moveVertex( newX, newY, id, i );
      i = i + 1;
      vertex = geom.vertexAt( i );
    }
    //double offsetx = ( 1 - mScaling ) * mRubberScale.x();
    //double offsety = ( 1 - mScaling ) *  mRubberScale.y();
    //QgsGeometry::OperationResult res = geom.transform( QTransform( mScaling, 0, 0, mScaling, offsetx, offsety ) );
    //QString::number( res );
  }

  deleteScalingWidget();
  deleteRubberband();

  mLayer->endEditCommand();
  mLayer->triggerRepaint();
}

void QgsMapToolScaleFeature::keyReleaseEvent( QKeyEvent *e )
{
  if ( mScalingActive && e->key() == Qt::Key_Escape )
  {
    cancel();
    return;
  }
  QgsMapTool::keyReleaseEvent( e );
}

void QgsMapToolScaleFeature::activate()
{
  mLayer = currentVectorLayer();
  if ( !mLayer )
  {
    return;
  }

  if ( !mLayer->isEditable() )
  {
    return;
  }

  if ( mLayer->selectedFeatureCount() > 0 )
  {
    mExtent = mLayer->boundingBoxOfSelected();
    mFeatureCenter = mExtent.center();

    mAnchorPoint = qgis::make_unique<QgsVertexMarker>( mCanvas );
    mAnchorPoint->setIconType( QgsVertexMarker::ICON_CROSS );
    mAnchorPoint->setCenter( mFeatureCenter );
  }
  QgsMapTool::activate();

}

void QgsMapToolScaleFeature::recenterRubberband( double )
{
    if ( !mScalingActive )
        return;
    QPoint rubberAnchor = toCanvasCoordinates( mMapAnchor );
    mAnchorPoint->setCenter( mMapAnchor );
    mRubberScale = QPointF( rubberAnchor.x() - mRubberBand->x(), rubberAnchor.y() - mRubberBand->y() );
    mRubberBand->setTransformOriginPoint( rubberAnchor );
    mRubberBand->show();
}

void QgsMapToolScaleFeature::deleteRubberband()
{
  delete mRubberBand;
  mRubberBand = nullptr;
}

void QgsMapToolScaleFeature::deactivate()
{
  deleteScalingWidget();
  mScalingActive = false;
  mAnchorPoint.reset();
  deleteRubberband();
  disconnect(mCanvas, &QgsMapCanvas::scaleChanged, this, &QgsMapToolScaleFeature::recenterRubberband);
  QgsMapTool::deactivate();
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


