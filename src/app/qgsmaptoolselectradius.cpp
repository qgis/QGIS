/***************************************************************************
qgsmaptoolselectradius.cpp  -  map tool for selecting features by radius
---------------------
begin                : May 2010
copyright            : (C) 2010 by Jeremy Palmer
email                : jpalmer at linz dot govt dot nz
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/


#include <cmath>
#include <QMouseEvent>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>

#include "qgisapp.h"
#include "qgsmaptoolselectradius.h"
#include "qgsmaptoolselectutils.h"
#include "qgsgeometry.h"
#include "qgsrubberband.h"
#include "qgsmapcanvas.h"
#include "qgis.h"
#include "qgslogger.h"
#include "qgsdoublespinbox.h"
#include "qgssnapindicator.h"
#include "qgsmaptoolselectionhandler.h"


const int RADIUS_SEGMENTS = 80;

QgsDistanceWidget::QgsDistanceWidget( const QString &label, QWidget *parent )
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

  mDistanceSpinBox = new QgsDoubleSpinBox( this );
  mDistanceSpinBox->setSingleStep( 1 );
  mDistanceSpinBox->setValue( 0 );
  mDistanceSpinBox->setMinimum( 0 );
  mDistanceSpinBox->setMaximum( 1000000000 );
  mDistanceSpinBox->setDecimals( 6 );
  mDistanceSpinBox->setShowClearButton( false );
  mDistanceSpinBox->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Preferred );
  mLayout->addWidget( mDistanceSpinBox );

  // connect signals
  mDistanceSpinBox->installEventFilter( this );
  connect( mDistanceSpinBox, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), this, &QgsDistanceWidget::distanceSpinBoxValueChanged );

  // config focus
  setFocusProxy( mDistanceSpinBox );
}

void QgsDistanceWidget::setDistance( double distance )
{
  mDistanceSpinBox->setValue( distance );
}

double QgsDistanceWidget::distance()
{
  return mDistanceSpinBox->value();
}

bool QgsDistanceWidget::eventFilter( QObject *obj, QEvent *ev )
{
  if ( obj == mDistanceSpinBox && ev->type() == QEvent::KeyPress )
  {
    QKeyEvent *event = static_cast<QKeyEvent *>( ev );
    if ( event->key() == Qt::Key_Escape )
    {
      emit distanceEditingCanceled();
      return true;
    }
    if ( event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return )
    {
      emit distanceEditingFinished( distance(), event->modifiers() );
      return true;
    }
  }

  return false;
}

void QgsDistanceWidget::distanceSpinBoxValueChanged( double distance )
{
  emit distanceChanged( distance );
}

QgsMapToolSelectRadius::QgsMapToolSelectRadius( QgsMapCanvas *canvas )
  : QgsMapTool( canvas )
  , mSnapIndicator( qgis::make_unique< QgsSnapIndicator >( canvas ) )
{
  mCursor = Qt::ArrowCursor;
  mSelectionHandler = new QgsMapToolSelectionHandler( canvas );
  //mSelectionHandler->setIface( QgisApp::instance()->mQgisInterface);
  mSelectionHandler->setIface(reinterpret_cast<QgisInterface*> (QgisApp::instance()->getInterface()));
}

QgsMapToolSelectRadius::~QgsMapToolSelectRadius()
{
  deleteRotationWidget();
}

void QgsMapToolSelectRadius::canvasMoveEvent( QgsMapMouseEvent *e )
{
  QgsPointXY radiusEdge = e->snapPoint();
  mSnapIndicator->setMatch( e->mapPointMatch() );

  if ( !mActive )
    return;

  if ( !mRubberBand )
  {
    createRubberBand();
  }

  updateRadiusFromEdge( radiusEdge );
}


void QgsMapToolSelectRadius::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  if ( e->button() == Qt::RightButton )
  {
    deleteRotationWidget();
    deleteRubberband();
    mActive = false;
    return;
  }

  if ( e->button() != Qt::LeftButton )
    return;

  if ( !mActive )
  {
    mActive = true;
    mRadiusCenter = e->snapPoint();
    createRotationWidget();
  }
  else
  {
    if ( !mRubberBand )
    {
      createRubberBand();
    }
    QgsPointXY radiusEdge = e->snapPoint();
    updateRadiusFromEdge( radiusEdge );
    selectFromRubberband( e->modifiers() );
  }
}


void QgsMapToolSelectRadius::updateRadiusFromEdge( QgsPointXY &radiusEdge )
{
  double radius = std::sqrt( mRadiusCenter.sqrDist( radiusEdge ) );
  if ( mDistanceWidget )
  {
    mDistanceWidget->setDistance( radius );
    mDistanceWidget->setFocus( Qt::TabFocusReason );
    mDistanceWidget->editor()->selectAll();
  }
  else
  {
    updateRubberband( radius );
  }
}

void QgsMapToolSelectRadius::deactivate()
{
  deleteRotationWidget();
  deleteRubberband();
  mActive = false;

  mSnapIndicator->setMatch( QgsPointLocator::Match() );

  QgsMapTool::deactivate();
}

void QgsMapToolSelectRadius::keyReleaseEvent( QKeyEvent *e )
{
  if ( mActive && e->key() == Qt::Key_Escape )
  {
    cancel();
    return;
  }
  QgsMapTool::keyReleaseEvent( e );
}

void QgsMapToolSelectRadius::updateRubberband( const double &radius )
{
  if ( !mRubberBand )
    createRubberBand();

  mRubberBand->reset( QgsWkbTypes::PolygonGeometry );
  for ( int i = 0; i <= RADIUS_SEGMENTS; ++i )
  {
    double theta = i * ( 2.0 * M_PI / RADIUS_SEGMENTS );
    QgsPointXY radiusPoint( mRadiusCenter.x() + radius * std::cos( theta ),
                            mRadiusCenter.y() + radius * std::sin( theta ) );
    mRubberBand->addPoint( radiusPoint, false );
  }
  mRubberBand->closePoints( true );
}

void QgsMapToolSelectRadius::selectFromRubberband( const Qt::KeyboardModifiers &modifiers )
{
  QgsGeometry radiusGeometry = mRubberBand->asGeometry();
  QgsMapToolSelectUtils::selectMultipleFeatures( mCanvas, radiusGeometry, modifiers );
  mRubberBand->reset( QgsWkbTypes::PolygonGeometry );
  deleteRotationWidget();
  deleteRubberband();
  mActive = false;
}

void QgsMapToolSelectRadius::radiusValueEntered( const double &radius, const Qt::KeyboardModifiers &modifiers )
{
  updateRubberband( radius );
  selectFromRubberband( modifiers );
}

void QgsMapToolSelectRadius::cancel()
{
  deleteRotationWidget();
  deleteRubberband();
  mActive = false;
}

void QgsMapToolSelectRadius::deleteRubberband()
{
  mRubberBand.reset();
}

void QgsMapToolSelectRadius::createRotationWidget()
{
  if ( !mCanvas )
  {
    return;
  }

  deleteRotationWidget();

  mDistanceWidget = new QgsDistanceWidget( QStringLiteral( "Selection radius:" ) );
  QgisApp::instance()->addUserInputWidget( mDistanceWidget );
  mDistanceWidget->setFocus( Qt::TabFocusReason );

  connect( mDistanceWidget, &QgsDistanceWidget::distanceChanged, this, &QgsMapToolSelectRadius::updateRubberband );
  connect( mDistanceWidget, &QgsDistanceWidget::distanceEditingFinished, this, &QgsMapToolSelectRadius::radiusValueEntered );
  connect( mDistanceWidget, &QgsDistanceWidget::distanceEditingCanceled, this, &QgsMapToolSelectRadius::cancel );
}

void QgsMapToolSelectRadius::createRubberBand()
{
  mRubberBand = qgis::make_unique< QgsRubberBand >( mCanvas, QgsWkbTypes::PolygonGeometry );
  mRubberBand->setFillColor( mFillColor );
  mRubberBand->setStrokeColor( mStrokeColor );
}

void QgsMapToolSelectRadius::deleteRotationWidget()
{
  if ( mDistanceWidget )
  {
    disconnect( mDistanceWidget, &QgsDistanceWidget::distanceChanged, this, &QgsMapToolSelectRadius::updateRubberband );
    disconnect( mDistanceWidget, &QgsDistanceWidget::distanceEditingFinished, this, &QgsMapToolSelectRadius::radiusValueEntered );
    disconnect( mDistanceWidget, &QgsDistanceWidget::distanceEditingCanceled, this, &QgsMapToolSelectRadius::cancel );
    mDistanceWidget->releaseKeyboard();
    mDistanceWidget->deleteLater();
  }
  mDistanceWidget = nullptr;
}
