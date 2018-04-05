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

#include "qgsapplication.h"
#include "qgsdistancearea.h"
#include "qgsfeature.h"
#include "qgsfeatureiterator.h"
#include "qgsfeaturestore.h"
#include "qgsfields.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmaptoolidentify.h"
#include "qgsmaptopixel.h"
#include "qgsmessageviewer.h"
#include "qgsmaplayer.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterlayer.h"
#include "qgsrasteridentifyresult.h"
#include "qgsrubberband.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgsrenderer.h"
#include "qgsgeometryutils.h"
#include "qgsgeometrycollection.h"
#include "qgscurve.h"
#include "qgscoordinateutils.h"
#include "qgsexception.h"
#include "qgssettings.h"
#include "qgsmaptoolselectionhandler.h"

#include <QMouseEvent>
#include <QCursor>
#include <QPixmap>
#include <QStatusBar>
#include <QVariant>
#include <QMenu>
#include <qlabel.h>
#include <qgsdoublespinbox.h>
#include <qgisinterface.h>

class QgsMapToolSelectUtils;
class QgisInterface;


QgsDistanceWidgetNew::QgsDistanceWidgetNew( const QString &label, QWidget *parent )
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
  connect( mDistanceSpinBox, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), this, &QgsDistanceWidgetNew::distanceSpinBoxValueChanged );

  // config focus
  setFocusProxy( mDistanceSpinBox );
}

void QgsDistanceWidgetNew::setDistance( double distance )
{
  mDistanceSpinBox->setValue( distance );
}

double QgsDistanceWidgetNew::distance()
{
  return mDistanceSpinBox->value();
}

bool QgsDistanceWidgetNew::eventFilter( QObject *obj, QEvent *ev )
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

void QgsDistanceWidgetNew::distanceSpinBoxValueChanged( double distance )
{
  emit distanceChanged( distance );
}

QgsMapToolSelectionHandler::QgsMapToolSelectionHandler( QgsMapCanvas *canvas, QgisInterface *iface )
  : QObject()
  , mLastMapUnitsPerPixel( -1.0 )
  , mCoordinatePrecision( 6 )
  , mCanvas( canvas )
  , mSelectionMode( QgsMapToolSelectionHandler::SelectSimple )
{
  mFillColor = QColor( 254, 178, 76, 63 );
  mQgisInterface = iface;
  mStrokeColor = QColor( 254, 58, 29, 100 );
}

QgsMapToolSelectionHandler::~QgsMapToolSelectionHandler()
{
  deleteRotationWidget();
}

void QgsMapToolSelectionHandler::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  switch ( mSelectionMode )
  {
    case QgsMapToolSelectionHandler::SelectSimple:
      selectFeaturesReleaseEvent( e );
      break;
    case QgsMapToolSelectionHandler::SelectPolygon:
      selectPolygonReleaseEvent( e );
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

void QgsMapToolSelectionHandler::selectFeaturesPressEvent( QgsMapMouseEvent *e )
{
  mSelectionRubberBand.reset();
  initRubberBand();
  mInitDragPos = e -> pos();
}

void QgsMapToolSelectionHandler::canvasPressEvent( QgsMapMouseEvent *e )
{
  switch ( mSelectionMode )
  {
    case QgsMapToolSelectionHandler::SelectSimple:
      selectFeaturesPressEvent( e );
      break;
    case QgsMapToolSelectionHandler::SelectPolygon:
      break;
    case QgsMapToolSelectionHandler::SelectFreehand:
      break;
    case QgsMapToolSelectionHandler::SelectRadius:
      break;
  }
}

bool QgsMapToolSelectionHandler::escapeSelection( QKeyEvent *e )
{
  if ( mSelectionActive && e->key() == Qt::Key_Escape )
  {
    mSelectionRubberBand.reset();
    mSelectionActive = false;
    deleteRotationWidget();
    return true;
  }
  return false;
}

void QgsMapToolSelectionHandler::deactivate()
{
  deleteRotationWidget();
  mSelectionRubberBand.reset();
  mSelectionActive = false;
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
  // TODO @vsklencar refactor
  this->setRubberBand( mCanvas, rect, mSelectionRubberBand.get() );
}

void QgsMapToolSelectionHandler::selectFeaturesReleaseEvent( QgsMapMouseEvent *e )
{
  mSelectFeatures = false;
  QPoint point = e->pos() - mInitDragPos;
  if ( !mSelectionActive || ( point.manhattanLength() < QApplication::startDragDistance() ) )
  {
    mSelectionGeometry = QgsGeometry::fromPointXY( toMapCoordinates( e ->pos() ) );
    mSelectionActive = false;
    mSelectFeatures = true;
  }

  if ( mSelectionRubberBand && mSelectionActive )
  {
    mSelectionGeometry = mSelectionRubberBand->asGeometry();
    mSelectFeatures = true;
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
    mSelectionRubberBand->movePoint( this->toMapCoordinates( e->pos() ) );
  }
}

void QgsMapToolSelectionHandler::selectPolygonReleaseEvent( QgsMapMouseEvent *e )
{
  mSelectFeatures = false;
  if ( !mSelectionRubberBand )
  {
    initRubberBand();
  }
  if ( e->button() == Qt::LeftButton )
  {
    mSelectionRubberBand->addPoint( toMapCoordinates( e->pos() ) );
    mSelectionActive = true;
  }
  else
  {
    if ( mSelectionRubberBand->numberOfVertices() > 2 )
    {
      mSelectionGeometry = mSelectionRubberBand->asGeometry();
      mSelectFeatures = true;
    }
    mSelectionRubberBand.reset();
    mJustFinishedSelection = mSelectionActive;
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
  mSelectFeatures = false;
  if ( !mSelectionActive )
  {
    if ( e->button() != Qt::LeftButton )
      return;

    if ( !mSelectionRubberBand )
    {
      initRubberBand();
    }
    else
    {
      mSelectionRubberBand->reset( QgsWkbTypes::PolygonGeometry );
    }
    mSelectionRubberBand->addPoint( toMapCoordinates( e->pos() ) );
    mSelectionActive = true;
  }
  else
  {
    if ( e->button() == Qt::LeftButton )
    {
      if ( mSelectionRubberBand && mSelectionRubberBand->numberOfVertices() > 2 )
      {
        mSelectionGeometry = mSelectionRubberBand->asGeometry();
        mSelectFeatures = true;
      }
    }

    mSelectionRubberBand.reset();
    mSelectionActive = false;
  }
}

void QgsMapToolSelectionHandler::selectRadiusMoveEvent( QgsMapMouseEvent *e )
{
  QgsPointXY radiusEdge = e->snapPoint();

  if ( !mSelectionActive )
  {
    mSelectFeatures = true;
    return;
  }

  if ( !mSelectionRubberBand )
  {
    initRubberBand();
    mSelectFeatures = false;
  }

  updateRadiusFromEdge( radiusEdge );
}

void QgsMapToolSelectionHandler::selectRadiusReleaseEvent( QgsMapMouseEvent *e )
{

  if ( e->button() == Qt::RightButton )
  {
    mSelectionActive = false;
    mSelectFeatures = false;
    deleteRotationWidget();
    mSelectionRubberBand.reset();
    return;
  }

  if ( e->button() != Qt::LeftButton )
    return;

  if ( !mSelectionActive )
  {
    mSelectionActive = true;
    mSelectFeatures = true;
    mRadiusCenter = e->snapPoint();
    createRotationWidget();
  }
  else
  {
    if ( !mSelectionRubberBand )
    {
      initRubberBand();
    }
    mSelectionGeometry = mSelectionRubberBand->asGeometry();
    mSelectionRubberBand->reset( QgsWkbTypes::PolygonGeometry );
    mSelectionActive = false;
    mSelectFeatures = true;
    deleteRotationWidget();
  }
}


void QgsMapToolSelectionHandler::initRubberBand()
{
  mSelectionRubberBand = qgis::make_unique< QgsRubberBand>( mCanvas, QgsWkbTypes::PolygonGeometry );
  mSelectionRubberBand->setFillColor( mFillColor );
  mSelectionRubberBand->setStrokeColor( mStrokeColor );
}

void QgsMapToolSelectionHandler::setIface( QgisInterface *iface )
{
  mQgisInterface = iface;
}

void QgsMapToolSelectionHandler::createRotationWidget()
{
  if ( !mCanvas || !mQgisInterface )
  {
    return;
  }

  deleteRotationWidget();

  mDistanceWidget = new QgsDistanceWidgetNew( QStringLiteral( "Selection radius:" ) );
  if ( mQgisInterface )
  {
    mQgisInterface->addUserInputWidget( mDistanceWidget );
    mDistanceWidget->setFocus( Qt::TabFocusReason );
  }

  connect( mDistanceWidget, &QgsDistanceWidgetNew::distanceChanged, this, &QgsMapToolSelectionHandler::updateRubberband );
  connect( mDistanceWidget, &QgsDistanceWidgetNew::distanceEditingFinished, this, &QgsMapToolSelectionHandler::radiusValueEntered );
  connect( mDistanceWidget, &QgsDistanceWidgetNew::distanceEditingCanceled, this, &QgsMapToolSelectionHandler::cancel );
}

void QgsMapToolSelectionHandler::deleteRotationWidget()
{
  if ( mDistanceWidget )
  {
    disconnect( mDistanceWidget, &QgsDistanceWidgetNew::distanceChanged, this, &QgsMapToolSelectionHandler::updateRubberband );
    disconnect( mDistanceWidget, &QgsDistanceWidgetNew::distanceEditingFinished, this, &QgsMapToolSelectionHandler::radiusValueEntered );
    disconnect( mDistanceWidget, &QgsDistanceWidgetNew::distanceEditingCanceled, this, &QgsMapToolSelectionHandler::cancel );
    mDistanceWidget->releaseKeyboard();
    mDistanceWidget->deleteLater();
  }
  mDistanceWidget = nullptr;
}

void QgsMapToolSelectionHandler::selectFromRubberband( const Qt::KeyboardModifiers &modifiers )
{
  // TODO @vsklencar
  //QgsMapToolSelectUtils::selectMultipleFeatures( mCanvas, mSelectionRubberBand->asGeometry(), modifiers );
  mSelectionRubberBand->reset( QgsWkbTypes::PolygonGeometry );
  deleteRotationWidget();
  mSelectionActive = false;
  mSelectFeatures = true;
}

void QgsMapToolSelectionHandler::radiusValueEntered( const double &radius, const Qt::KeyboardModifiers &modifiers )
{
  updateRubberband( radius );
  selectFromRubberband( modifiers );
}

void QgsMapToolSelectionHandler::cancel()
{
  deleteRotationWidget();
  mSelectionRubberBand.reset();
  mSelectionActive = false;
}

void QgsMapToolSelectionHandler::updateRubberband( const double &radius )
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
    mDistanceWidget->editor()->selectAll();
  }
  else
  {
    updateRubberband( radius );
  }
}

QgsGeometry QgsMapToolSelectionHandler::selectedGeometry()
{
  return mSelectionGeometry;
}

void QgsMapToolSelectionHandler::setSelectedGeometry( QgsGeometry geometry )
{
  mSelectionGeometry = geometry;
}

void QgsMapToolSelectionHandler::setSelectionMode( SelectionMode mode )
{
  mSelectionMode = mode;
}

QgsMapToolSelectionHandler::SelectionMode QgsMapToolSelectionHandler::selectionMode()
{
  return mSelectionMode;
}

void QgsMapToolSelectionHandler::setRubberBand( QgsMapCanvas *canvas, QRect &selectRect, QgsRubberBand *rubberBand )
{
  const QgsMapToPixel *transform = canvas->getCoordinateTransform();
  QgsPointXY ll = transform->toMapCoordinates( selectRect.left(), selectRect.bottom() );
  QgsPointXY lr = transform->toMapCoordinates( selectRect.right(), selectRect.bottom() );
  QgsPointXY ul = transform->toMapCoordinates( selectRect.left(), selectRect.top() );
  QgsPointXY ur = transform->toMapCoordinates( selectRect.right(), selectRect.top() );

  if ( rubberBand )
  {
    rubberBand->reset( QgsWkbTypes::PolygonGeometry );
    rubberBand->addPoint( ll, false );
    rubberBand->addPoint( lr, false );
    rubberBand->addPoint( ur, false );
    rubberBand->addPoint( ul, true );
  }
}

