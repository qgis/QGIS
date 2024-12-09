/***************************************************************************
    qgsmaptoolselectionhandler.cpp
    ---------------------
    begin                : March 2018
    copyright            : (C) 2018 by Viktor Sklencar
    email                : vsklencar at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptoolselectionhandler.h"
#include "moc_qgsmaptoolselectionhandler.cpp"

#include <QBoxLayout>
#include <QKeyEvent>
#include <QLabel>

#include "qgisapp.h"
#include "qgsdoublespinbox.h"
#include "qgsmapcanvas.h"
#include "qgsmapmouseevent.h"
#include "qgsrubberband.h"
#include "qgssnapindicator.h"
#include "qgsidentifymenu.h"

/// @cond private

QgsDistanceWidget::QgsDistanceWidget( const QString &label, QWidget *parent )
  : QWidget( parent )
{
  mLayout = new QHBoxLayout( this );
  mLayout->setContentsMargins( 0, 0, 0, 0 );
  mLayout->setAlignment( Qt::AlignLeft );
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
  connect( mDistanceSpinBox, static_cast<void ( QgsDoubleSpinBox::* )( double )>( &QgsDoubleSpinBox::valueChanged ), this, &QgsDistanceWidget::distanceChanged );

  // config focus
  setFocusProxy( mDistanceSpinBox );
}

void QgsDistanceWidget::setDistance( double distance )
{
  mDistanceSpinBox->setValue( distance );
  mDistanceSpinBox->selectAll();
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

/// @endcond


QgsMapToolSelectionHandler::QgsMapToolSelectionHandler( QgsMapCanvas *canvas, QgsMapToolSelectionHandler::SelectionMode selectionMode )
  : mCanvas( canvas )
  , mSelectionMode( selectionMode )
  , mSnapIndicator( std::make_unique<QgsSnapIndicator>( canvas ) )
  , mIdentifyMenu( new QgsIdentifyMenu( mCanvas ) )
{
  mIdentifyMenu->setAllowMultipleReturn( false );
  mIdentifyMenu->setExecWithSingleResult( true );
}

QgsMapToolSelectionHandler::~QgsMapToolSelectionHandler()
{
  cancel();
}

void QgsMapToolSelectionHandler::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  switch ( mSelectionMode )
  {
    case QgsMapToolSelectionHandler::SelectSimple:
    case QgsMapToolSelectionHandler::SelectOnMouseOver:
      selectFeaturesReleaseEvent( e );
      break;
    case QgsMapToolSelectionHandler::SelectPolygon:
      break;
    case QgsMapToolSelectionHandler::SelectFreehand:
      selectFreehandReleaseEvent( e );
      break;
    case QgsMapToolSelectionHandler::SelectRadius:
      selectRadiusReleaseEvent( e );
      break;
  }
}


void QgsMapToolSelectionHandler::canvasMoveEvent( QgsMapMouseEvent *e )
{
  switch ( mSelectionMode )
  {
    case QgsMapToolSelectionHandler::SelectOnMouseOver:
    case QgsMapToolSelectionHandler::SelectSimple:
      selectFeaturesMoveEvent( e );
      break;
    case QgsMapToolSelectionHandler::SelectPolygon:
      selectPolygonMoveEvent( e );
      break;
    case QgsMapToolSelectionHandler::SelectFreehand:
      selectFreehandMoveEvent( e );
      break;
    case QgsMapToolSelectionHandler::SelectRadius:
      selectRadiusMoveEvent( e );
      break;
  }
}

void QgsMapToolSelectionHandler::canvasPressEvent( QgsMapMouseEvent *e )
{
  switch ( mSelectionMode )
  {
    case QgsMapToolSelectionHandler::SelectOnMouseOver:
    case QgsMapToolSelectionHandler::SelectSimple:
      selectFeaturesPressEvent( e );
      break;
    case QgsMapToolSelectionHandler::SelectPolygon:
      selectPolygonPressEvent( e );
      break;
    case QgsMapToolSelectionHandler::SelectFreehand:
      break;
    case QgsMapToolSelectionHandler::SelectRadius:
      break;
  }
}

bool QgsMapToolSelectionHandler::keyReleaseEvent( QKeyEvent *e )
{
  if ( mSelectionActive && e->key() == Qt::Key_Escape )
  {
    cancel();
    return true;
  }
  return false;
}

void QgsMapToolSelectionHandler::deactivate()
{
  cancel();
}

void QgsMapToolSelectionHandler::selectFeaturesPressEvent( QgsMapMouseEvent *e )
{
  if ( !mSelectionRubberBand )
    initRubberBand();

  mInitDragPos = e->pos();
}

void QgsMapToolSelectionHandler::selectFeaturesMoveEvent( QgsMapMouseEvent *e )
{
  if ( mSelectionMode == QgsMapToolSelectionHandler::SelectOnMouseOver && mCanvas->underMouse() )
  {
    mMoveLastCursorPos = e->pos();
    // This is a (well known, according to google) false positive,
    // I tried all possible NOLINT placements without success, this
    // ugly ifdef seems to do the trick with silencing the warning.
#ifndef __clang_analyzer__
    if ( !mOnMouseMoveDelayTimer || !mOnMouseMoveDelayTimer->isActive() )
    {
      setSelectedGeometry( QgsGeometry::fromPointXY( toMapCoordinates( e->pos() ) ), e->modifiers() );
      mOnMouseMoveDelayTimer = std::make_unique<QTimer>();
      mOnMouseMoveDelayTimer->setSingleShot( true );
      connect( mOnMouseMoveDelayTimer.get(), &QTimer::timeout, this, [=] {
        if ( !mMoveLastCursorPos.isNull() )
        {
          setSelectedGeometry( QgsGeometry::fromPointXY( toMapCoordinates( mMoveLastCursorPos ) ), e->modifiers() );
        }
      } );
      mOnMouseMoveDelayTimer->start( 300 );
    }
#endif
    return;
  }

  if ( e->buttons() != Qt::LeftButton )
    return;

  QRect rect;
  if ( !mSelectionActive )
  {
    mSelectionActive = true;
    rect = QRect( e->pos(), e->pos() );
  }
  else
  {
    rect = QRect( e->pos(), mInitDragPos );
  }

  if ( mSelectionRubberBand )
    mSelectionRubberBand->setToCanvasRectangle( rect );
}

void QgsMapToolSelectionHandler::selectFeaturesReleaseEvent( QgsMapMouseEvent *e )
{
  const QPoint point = e->pos() - mInitDragPos;
  if ( !mSelectionActive || ( point.manhattanLength() < QApplication::startDragDistance() ) )
  {
    mSelectionActive = false;
    setSelectedGeometry( QgsGeometry::fromPointXY( toMapCoordinates( e->pos() ) ), e->modifiers() );
  }

  if ( mSelectionRubberBand && mSelectionActive )
    setSelectedGeometry( mSelectionRubberBand->asGeometry(), e->modifiers() );
  if ( mSelectionRubberBand )
    mSelectionRubberBand.reset();

  mSelectionActive = false;
}

QgsPointXY QgsMapToolSelectionHandler::toMapCoordinates( QPoint point )
{
  return mCanvas->getCoordinateTransform()->toMapCoordinates( point );
}

void QgsMapToolSelectionHandler::selectPolygonMoveEvent( QgsMapMouseEvent *e )
{
  if ( !mSelectionRubberBand )
    return;

  if ( mSelectionRubberBand->numberOfVertices() > 0 )
  {
    mSelectionRubberBand->movePoint( toMapCoordinates( e->pos() ) );
  }
}

void QgsMapToolSelectionHandler::selectPolygonPressEvent( QgsMapMouseEvent *e )
{
  // Handle immediate right-click on feature to show context menu
  if ( !mSelectionRubberBand && ( e->button() == Qt::RightButton ) )
  {
    const QList<QgsMapToolIdentify::IdentifyResult> results = QgsIdentifyMenu::findFeaturesOnCanvas( e, mCanvas, { Qgis::GeometryType::Polygon } );

    const QPoint globalPos = mCanvas->mapToGlobal( QPoint( e->pos().x() + 5, e->pos().y() + 5 ) );
    const QList<QgsMapToolIdentify::IdentifyResult> selectedFeatures = mIdentifyMenu->exec( results, globalPos );
    if ( !selectedFeatures.empty() && selectedFeatures[0].mFeature.hasGeometry() )
    {
      QgsCoordinateTransform transform = mCanvas->mapSettings().layerTransform( selectedFeatures.at( 0 ).mLayer );
      QgsGeometry geom = selectedFeatures[0].mFeature.geometry();
      try
      {
        geom.transform( transform );
      }
      catch ( QgsCsException & )
      {
        QgsDebugError( QStringLiteral( "Could not transform geometry to map CRS" ) );
      }

      setSelectedGeometry( geom, e->modifiers() );
    }

    return;
  }

  // Handle definition of polygon by clicking points on cancas
  if ( !mSelectionRubberBand )
    initRubberBand();

  if ( e->button() == Qt::LeftButton )
  {
    mSelectionRubberBand->addPoint( toMapCoordinates( e->pos() ) );
    mSelectionActive = true;
  }
  else
  {
    if ( mSelectionRubberBand->numberOfVertices() > 2 )
    {
      setSelectedGeometry( mSelectionRubberBand->asGeometry(), e->modifiers() );
    }
    mSelectionRubberBand.reset();
    mSelectionActive = false;
  }
}

void QgsMapToolSelectionHandler::selectFreehandMoveEvent( QgsMapMouseEvent *e )
{
  if ( !mSelectionActive || !mSelectionRubberBand )
    return;

  mSelectionRubberBand->addPoint( toMapCoordinates( e->pos() ) );
}

void QgsMapToolSelectionHandler::selectFreehandReleaseEvent( QgsMapMouseEvent *e )
{
  if ( !mSelectionActive )
  {
    if ( e->button() != Qt::LeftButton )
      return;

    if ( !mSelectionRubberBand )
      initRubberBand();

    mSelectionRubberBand->addPoint( toMapCoordinates( e->pos() ) );
    mSelectionActive = true;
  }
  else
  {
    if ( e->button() == Qt::LeftButton )
    {
      if ( mSelectionRubberBand && mSelectionRubberBand->numberOfVertices() > 2 )
      {
        setSelectedGeometry( mSelectionRubberBand->asGeometry(), e->modifiers() );
      }
    }

    mSelectionRubberBand.reset();
    mSelectionActive = false;
  }
}

void QgsMapToolSelectionHandler::selectRadiusMoveEvent( QgsMapMouseEvent *e )
{
  QgsPointXY radiusEdge = e->snapPoint();

  mSnapIndicator->setMatch( e->mapPointMatch() );

  if ( !mSelectionActive )
  {
    return;
  }

  if ( !mSelectionRubberBand )
  {
    initRubberBand();
  }

  updateRadiusFromEdge( radiusEdge );
}

void QgsMapToolSelectionHandler::selectRadiusReleaseEvent( QgsMapMouseEvent *e )
{
  if ( e->button() == Qt::RightButton )
  {
    cancel();
    return;
  }

  if ( e->button() != Qt::LeftButton )
    return;

  if ( !mSelectionActive )
  {
    mSelectionActive = true;
    mRadiusCenter = e->snapPoint();
    createDistanceWidget();
  }
  else
  {
    if ( mSelectionRubberBand )
    {
      setSelectedGeometry( mSelectionRubberBand->asGeometry(), e->modifiers() );
    }
    cancel();
  }
}


void QgsMapToolSelectionHandler::initRubberBand()
{
  mSelectionRubberBand = std::make_unique<QgsRubberBand>( mCanvas, Qgis::GeometryType::Polygon );
  mSelectionRubberBand->setFillColor( mFillColor );
  mSelectionRubberBand->setStrokeColor( mStrokeColor );
}

void QgsMapToolSelectionHandler::createDistanceWidget()
{
  if ( !mCanvas )
  {
    return;
  }

  deleteDistanceWidget();

  mDistanceWidget = new QgsDistanceWidget( tr( "Selection radius:" ) );
  QgisApp::instance()->addUserInputWidget( mDistanceWidget );
  mDistanceWidget->setFocus( Qt::TabFocusReason );

  connect( mDistanceWidget, &QgsDistanceWidget::distanceChanged, this, &QgsMapToolSelectionHandler::updateRadiusRubberband );
  connect( mDistanceWidget, &QgsDistanceWidget::distanceEditingFinished, this, &QgsMapToolSelectionHandler::radiusValueEntered );
  connect( mDistanceWidget, &QgsDistanceWidget::distanceEditingCanceled, this, &QgsMapToolSelectionHandler::cancel );
}

void QgsMapToolSelectionHandler::deleteDistanceWidget()
{
  if ( mDistanceWidget )
  {
    mDistanceWidget->releaseKeyboard();
    mDistanceWidget->deleteLater();
  }
  mDistanceWidget = nullptr;
}

void QgsMapToolSelectionHandler::radiusValueEntered( double radius, Qt::KeyboardModifiers modifiers )
{
  if ( !mSelectionRubberBand )
    return;

  updateRadiusRubberband( radius );
  setSelectedGeometry( mSelectionRubberBand->asGeometry(), modifiers );
  cancel();
}

void QgsMapToolSelectionHandler::cancel()
{
  deleteDistanceWidget();
  mSnapIndicator->setMatch( QgsPointLocator::Match() );
  mSelectionRubberBand.reset();
  mSelectionActive = false;
}

void QgsMapToolSelectionHandler::updateRadiusRubberband( double radius )
{
  if ( !mSelectionRubberBand )
    initRubberBand();

  const int RADIUS_SEGMENTS = 80;

  mSelectionRubberBand->reset( Qgis::GeometryType::Polygon );
  for ( int i = 0; i <= RADIUS_SEGMENTS; ++i )
  {
    const double theta = i * ( 2.0 * M_PI / RADIUS_SEGMENTS );
    const QgsPointXY radiusPoint( mRadiusCenter.x() + radius * std::cos( theta ), mRadiusCenter.y() + radius * std::sin( theta ) );
    mSelectionRubberBand->addPoint( radiusPoint, false );
  }
  mSelectionRubberBand->closePoints( true );
}

void QgsMapToolSelectionHandler::updateRadiusFromEdge( QgsPointXY &radiusEdge )
{
  const double radius = std::sqrt( mRadiusCenter.sqrDist( radiusEdge ) );
  if ( mDistanceWidget )
  {
    mDistanceWidget->setDistance( radius );
    mDistanceWidget->setFocus( Qt::TabFocusReason );
  }
  else
  {
    updateRadiusRubberband( radius );
  }
}

QgsGeometry QgsMapToolSelectionHandler::selectedGeometry() const
{
  return mSelectionGeometry;
}

void QgsMapToolSelectionHandler::setSelectedGeometry( const QgsGeometry &geometry, Qt::KeyboardModifiers modifiers )
{
  mSelectionGeometry = geometry;
  mMoveLastCursorPos = QPoint();
  emit geometryChanged( modifiers );
}

void QgsMapToolSelectionHandler::setSelectionMode( SelectionMode mode )
{
  mSelectionMode = mode;
}

QgsMapToolSelectionHandler::SelectionMode QgsMapToolSelectionHandler::selectionMode() const
{
  return mSelectionMode;
}
