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
  connect( mDistanceSpinBox, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), this, &QgsDistanceWidget::distanceChanged );

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
  , mSnapIndicator( qgis::make_unique< QgsSnapIndicator >( canvas ) )
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
  QPoint point = e->pos() - mInitDragPos;
  if ( !mSelectionActive || ( point.manhattanLength() < QApplication::startDragDistance() ) )
  {
    mSelectionActive = false;
    setSelectedGeometry( QgsGeometry::fromPointXY( toMapCoordinates( e->pos() ) ), e->modifiers() );
  }

  if ( mSelectionRubberBand && mSelectionActive )
  {
    setSelectedGeometry( mSelectionRubberBand->asGeometry(), e->modifiers() );
    mSelectionRubberBand.reset();
  }

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
    QList<QgsMapToolIdentify::IdentifyResult> results;
    QMap< QString, QString > derivedAttributes;

    const QgsPointXY mapPoint = toMapCoordinates( e->pos() );
    double x = mapPoint.x(), y = mapPoint.y();
    double sr = QgsMapTool::searchRadiusMU( mCanvas );

    const QList<QgsMapLayer *> layers = mCanvas->layers();
    for ( auto layer : layers )
    {
      if ( layer->type() == QgsMapLayerType::VectorLayer )
      {
        auto vectorLayer = static_cast<QgsVectorLayer *>( layer );
        if ( vectorLayer->geometryType() == QgsWkbTypes::PolygonGeometry )
        {
          QgsFeatureIterator fit = vectorLayer->getFeatures( QgsFeatureRequest()
                                   .setDestinationCrs( mCanvas->mapSettings().destinationCrs(), mCanvas->mapSettings().transformContext() )
                                   .setFilterRect( QgsRectangle( x - sr, y - sr, x + sr, y + sr ) )
                                   .setFlags( QgsFeatureRequest::ExactIntersect ) );
          QgsFeature f;
          while ( fit.nextFeature( f ) )
          {
            results << QgsMapToolIdentify::IdentifyResult( vectorLayer, f, derivedAttributes );
          }
        }
      }
    }

    QPoint globalPos = mCanvas->mapToGlobal( QPoint( e->pos().x() + 5, e->pos().y() + 5 ) );
    const QList<QgsMapToolIdentify::IdentifyResult> selectedFeatures = mIdentifyMenu->exec( results, globalPos );
    if ( !selectedFeatures.empty() && selectedFeatures[0].mFeature.hasGeometry() )
      setSelectedGeometry( selectedFeatures[0].mFeature.geometry(), e->modifiers() );

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
  mSelectionRubberBand = qgis::make_unique<QgsRubberBand>( mCanvas, QgsWkbTypes::PolygonGeometry );
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

  mSelectionRubberBand->reset( QgsWkbTypes::PolygonGeometry );
  for ( int i = 0; i <= RADIUS_SEGMENTS; ++i )
  {
    double theta = i * ( 2.0 * M_PI / RADIUS_SEGMENTS );
    QgsPointXY radiusPoint( mRadiusCenter.x() + radius * std::cos( theta ),
                            mRadiusCenter.y() + radius * std::sin( theta ) );
    mSelectionRubberBand->addPoint( radiusPoint, false );
  }
  mSelectionRubberBand->closePoints( true );
}

void QgsMapToolSelectionHandler::updateRadiusFromEdge( QgsPointXY &radiusEdge )
{
  double radius = std::sqrt( mRadiusCenter.sqrDist( radiusEdge ) );
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
