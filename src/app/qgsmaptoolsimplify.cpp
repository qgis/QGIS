/***************************************************************************
    qgsmaptoolsimplify.cpp  - simplify vector layer features
    ---------------------
    begin                : April 2009
    copyright            : (C) 2009 by Richard Kostecky
    email                : csf dot kostej at mail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QPushButton>

#include "qgsmaptoolsimplify.h"

#include "qgsfeatureiterator.h"
#include "qgsgeometry.h"
#include "qgsmapcanvas.h"
#include "qgsrubberband.h"
#include "qgsvectorlayer.h"
#include "qgstolerance.h"
#include "qgisapp.h"
#include "qgssettings.h"
#include "qgsmaptopixelgeometrysimplifier.h"
#include "qgsmapmouseevent.h"

#include <cmath>
#include <cfloat>

QgsSimplifyUserInputWidget::QgsSimplifyUserInputWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  mMethodComboBox->addItem( tr( "Simplify by Distance" ), QgsMapToolSimplify::SimplifyDistance );
  mMethodComboBox->addItem( tr( "Simplify by Snapping to Grid" ), QgsMapToolSimplify::SimplifySnapToGrid );
  mMethodComboBox->addItem( tr( "Simplify by Area (Visvalingam)" ), QgsMapToolSimplify::SimplifyVisvalingam );
  mMethodComboBox->addItem( tr( "Smooth" ), QgsMapToolSimplify::Smooth );

  mToleranceUnitsComboBox->addItem( tr( "Layer Units" ), QgsTolerance::LayerUnits );
  mToleranceUnitsComboBox->addItem( tr( "Pixels" ), QgsTolerance::Pixels );
  mToleranceUnitsComboBox->addItem( tr( "Map Units" ), QgsTolerance::ProjectUnits );

  mToleranceSpinBox->setShowClearButton( false );

  mOffsetSpin->setClearValue( 25 );
  mIterationsSpin->setClearValue( 1 );
  if ( mMethodComboBox->currentData().toInt() != QgsMapToolSimplify::Smooth )
    mOptionsStackedWidget->setCurrentIndex( 0 );
  else
    mOptionsStackedWidget->setCurrentIndex( 1 );

  // communication with map tool
  connect( mToleranceSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsSimplifyUserInputWidget::toleranceChanged );
  connect( mToleranceUnitsComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, [ = ]( const int index ) {emit toleranceUnitsChanged( ( QgsTolerance::UnitType )index );} );
  connect( mMethodComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, [ = ]( const int method ) {emit methodChanged( ( QgsMapToolSimplify::Method )method );} );
  connect( mMethodComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, [ = ]
  {
    if ( mMethodComboBox->currentData().toInt() != QgsMapToolSimplify::Smooth )
      mOptionsStackedWidget->setCurrentIndex( 0 );
    else
      mOptionsStackedWidget->setCurrentIndex( 1 );
  } );

  connect( mOffsetSpin, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, [ = ]( const int offset ) {emit smoothOffsetChanged( offset / 100.0 );} );
  connect( mIterationsSpin, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsSimplifyUserInputWidget::smoothIterationsChanged );

  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QgsSimplifyUserInputWidget::accepted );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QgsSimplifyUserInputWidget::rejected );

  mToleranceSpinBox->installEventFilter( this );
  mOffsetSpin->installEventFilter( this );
  mIterationsSpin->installEventFilter( this );

  setFocusProxy( mButtonBox );
}

void QgsSimplifyUserInputWidget::setConfig( QgsMapToolSimplify::Method method,
    double tolerance,
    QgsTolerance::UnitType units,
    double smoothOffset,
    int smoothIterations )
{
  mMethodComboBox->setCurrentIndex( mMethodComboBox->findData( method ) );

  mToleranceSpinBox->setValue( tolerance );
  mToleranceUnitsComboBox->setCurrentIndex( mToleranceUnitsComboBox->findData( units ) );
  mOffsetSpin->setValue( 100 * smoothOffset );
  mIterationsSpin->setValue( smoothIterations );
}

void QgsSimplifyUserInputWidget::updateStatusText( const QString &text )
{
  labelStatus->setText( text );
}

void QgsSimplifyUserInputWidget::enableOkButton( bool enabled )
{
  mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( enabled );
}

bool QgsSimplifyUserInputWidget::eventFilter( QObject *object, QEvent *ev )
{
  Q_UNUSED( object )
  if ( ev->type() == QEvent::KeyPress )
  {
    QKeyEvent *event = static_cast<QKeyEvent *>( ev );
    if ( event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return )
    {
      emit accepted();
      return true;
    }
  }

  return false;
}

void QgsSimplifyUserInputWidget::keyReleaseEvent( QKeyEvent *event )
{
  if ( event->key() == Qt::Key_Escape )
  {
    emit rejected();
    return;
  }
  if ( event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return )
  {
    emit accepted();
    return;
  }
  QWidget::keyReleaseEvent( event );
}

////////////////////////////////////////////////////////////////////////////


QgsMapToolSimplify::QgsMapToolSimplify( QgsMapCanvas *canvas )
  : QgsMapToolEdit( canvas )
{
  const QgsSettings settings;
  mTolerance = settings.value( QStringLiteral( "digitizing/simplify_tolerance" ), 1 ).toDouble();
  mToleranceUnits = static_cast< QgsTolerance::UnitType >( settings.value( QStringLiteral( "digitizing/simplify_tolerance_units" ), 0 ).toInt() );
  mMethod = static_cast< QgsMapToolSimplify::Method >( settings.value( QStringLiteral( "digitizing/simplify_method" ), 0 ).toInt() );
  mSmoothIterations = settings.value( QStringLiteral( "digitizing/smooth_iterations" ), 1 ).toInt();
  mSmoothOffset = settings.value( QStringLiteral( "digitizing/smooth_offset" ), 0.25 ).toDouble();
}

QgsMapToolSimplify::~QgsMapToolSimplify()
{
  clearSelection();
}


void QgsMapToolSimplify::setTolerance( double tolerance )
{
  mTolerance = tolerance;

  QgsSettings settings;
  settings.setValue( QStringLiteral( "digitizing/simplify_tolerance" ), tolerance );

  if ( !mSelectedFeatures.isEmpty() )
    updateSimplificationPreview();
}

void QgsMapToolSimplify::setToleranceUnits( QgsTolerance::UnitType units )
{
  mToleranceUnits = units;

  QgsSettings settings;
  settings.setValue( QStringLiteral( "digitizing/simplify_tolerance_units" ), units );

  if ( !mSelectedFeatures.isEmpty() )
    updateSimplificationPreview();
}

void QgsMapToolSimplify::updateSimplificationPreview()
{
  QgsVectorLayer *vl = currentVectorLayer();

  const double layerTolerance = QgsTolerance::toleranceInMapUnits( mTolerance, vl, mCanvas->mapSettings(), mToleranceUnits );
  mReducedHasErrors = false;
  mReducedVertexCount = 0;
  int i = 0;

  const auto constMSelectedFeatures = mSelectedFeatures;
  for ( const QgsFeature &fSel : constMSelectedFeatures )
  {
    const QgsGeometry g = processGeometry( fSel.geometry(), layerTolerance );
    if ( !g.isNull() )
    {
      mReducedVertexCount += g.constGet()->nCoordinates();
      mRubberBands.at( i )->setToGeometry( g, vl );
    }
    else
      mReducedHasErrors = true;
    ++i;
  }

  if ( mSimplifyUserWidget )
  {
    mSimplifyUserWidget->updateStatusText( statusText() );
    mSimplifyUserWidget->enableOkButton( !mReducedHasErrors );
  }
}

void QgsMapToolSimplify::createUserInputWidget()
{
  mSimplifyUserWidget = new QgsSimplifyUserInputWidget( );
  mSimplifyUserWidget->setConfig( method(), tolerance(), toleranceUnits(), smoothOffset(), smoothIterations() );

  connect( mSimplifyUserWidget, &QgsSimplifyUserInputWidget::methodChanged, this, &QgsMapToolSimplify::setMethod );
  connect( mSimplifyUserWidget, &QgsSimplifyUserInputWidget::toleranceChanged, this, &QgsMapToolSimplify::setTolerance );
  connect( mSimplifyUserWidget, &QgsSimplifyUserInputWidget::toleranceUnitsChanged, this, &QgsMapToolSimplify::setToleranceUnits );
  connect( mSimplifyUserWidget, &QgsSimplifyUserInputWidget::smoothOffsetChanged, this, &QgsMapToolSimplify::setSmoothOffset );
  connect( mSimplifyUserWidget, &QgsSimplifyUserInputWidget::smoothIterationsChanged, this, &QgsMapToolSimplify::setSmoothIterations );
  connect( mSimplifyUserWidget, &QgsSimplifyUserInputWidget::accepted, this, &QgsMapToolSimplify::storeSimplified );
  connect( mSimplifyUserWidget, &QgsSimplifyUserInputWidget::rejected, this, &QgsMapToolSimplify::clearSelection );

  QgisApp::instance()->addUserInputWidget( mSimplifyUserWidget );
  mSimplifyUserWidget->setFocus( Qt::TabFocusReason );
}

QgsGeometry QgsMapToolSimplify::processGeometry( const QgsGeometry &geometry, double tolerance ) const
{
  switch ( mMethod )
  {
    case SimplifyDistance:
      return geometry.simplify( tolerance );

    case SimplifySnapToGrid:
    case SimplifyVisvalingam:
    {

      const QgsMapToPixelSimplifier simplifier( QgsMapToPixelSimplifier::SimplifyGeometry, tolerance, mMethod == SimplifySnapToGrid ? QgsMapToPixelSimplifier::SnapToGrid : QgsMapToPixelSimplifier::Visvalingam );
      return simplifier.simplify( geometry );
    }

    case Smooth:
      return geometry.smooth( mSmoothIterations, mSmoothOffset );

  }
  return QgsGeometry(); //no warnings
}

double QgsMapToolSimplify::smoothOffset() const
{
  return mSmoothOffset;
}

void QgsMapToolSimplify::setSmoothOffset( double smoothOffset )
{
  mSmoothOffset = smoothOffset;

  QgsSettings settings;
  settings.setValue( QStringLiteral( "digitizing/smooth_offset" ), smoothOffset );

  if ( !mSelectedFeatures.isEmpty() )
    updateSimplificationPreview();
}

int QgsMapToolSimplify::smoothIterations() const
{
  return mSmoothIterations;
}

void QgsMapToolSimplify::setSmoothIterations( int smoothIterations )
{
  mSmoothIterations = smoothIterations;

  QgsSettings settings;
  settings.setValue( QStringLiteral( "digitizing/smooth_iterations" ), smoothIterations );

  if ( !mSelectedFeatures.isEmpty() )
    updateSimplificationPreview();
}

QgsMapToolSimplify::Method QgsMapToolSimplify::method() const
{
  return mMethod;
}

void QgsMapToolSimplify::setMethod( QgsMapToolSimplify::Method method )
{
  mMethod = method;

  QgsSettings settings;
  settings.setValue( QStringLiteral( "digitizing/simplify_method" ), method );

  if ( !mSelectedFeatures.isEmpty() )
    updateSimplificationPreview();
}

void QgsMapToolSimplify::storeSimplified()
{
  QgsVectorLayer *vlayer = currentVectorLayer();
  const double layerTolerance = QgsTolerance::toleranceInMapUnits( mTolerance, vlayer, mCanvas->mapSettings(), mToleranceUnits );

  vlayer->beginEditCommand( tr( "Geometry simplified" ) );
  const auto constMSelectedFeatures = mSelectedFeatures;
  for ( const QgsFeature &feat : constMSelectedFeatures )
  {
    QgsGeometry g = processGeometry( feat.geometry(), layerTolerance );
    if ( !g.isNull() )
    {
      vlayer->changeGeometry( feat.id(), g );
    }
  }
  vlayer->endEditCommand();

  clearSelection();

  vlayer->triggerRepaint();
}

void QgsMapToolSimplify::canvasPressEvent( QgsMapMouseEvent *e )
{
  if ( e->button() != Qt::LeftButton )
    return;

  if ( !currentVectorLayer() )
  {
    notifyNotVectorLayer();
    return;
  }

  // delete previous rubberband (if any)
  clearSelection();

  mSelectionRect.setRect( 0, 0, 0, 0 );
}


void QgsMapToolSimplify::canvasMoveEvent( QgsMapMouseEvent *e )
{
  if ( !( e->buttons() & Qt::LeftButton ) )
    return;

  if ( !mDragging )
  {
    mDragging = true;
    delete mSelectionRubberBand;
    mSelectionRubberBand = new QgsRubberBand( mCanvas, QgsWkbTypes::PolygonGeometry );
    QColor color( Qt::blue );
    color.setAlpha( 63 );
    mSelectionRubberBand->setColor( color );
    mSelectionRect.setTopLeft( e->pos() );
  }
  mSelectionRect.setBottomRight( e->pos() );
  if ( mSelectionRubberBand )
  {
    mSelectionRubberBand->setToCanvasRectangle( mSelectionRect );
    mSelectionRubberBand->show();
  }
}


void QgsMapToolSimplify::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  if ( e->button() == Qt::RightButton )
  {
    clearSelection();
    return;
  }

  if ( e->button() != Qt::LeftButton || !currentVectorLayer() )
    return;

  delete mSelectionRubberBand;
  mSelectionRubberBand = nullptr;

  if ( mDragging && ( mSelectionRect.topLeft() != mSelectionRect.bottomRight() ) )
  {
    mDragging = false;

    // store the rectangle
    mSelectionRect.setRight( e->pos().x() );
    mSelectionRect.setBottom( e->pos().y() );

    selectFeaturesInRect();
  }
  else
  {
    selectOneFeature( e->pos() );
  }

  mDragging = false;

  if ( mSelectedFeatures.isEmpty() )
  {
    emit messageEmitted( tr( "Could not find a nearby feature in the current layer." ) );
    return;
  }

  // count vertices, prepare rubber bands
  mOriginalVertexCount = 0;
  const auto constMSelectedFeatures = mSelectedFeatures;
  for ( const QgsFeature &f : constMSelectedFeatures )
  {
    if ( f.hasGeometry() )
      mOriginalVertexCount += f.geometry().constGet()->nCoordinates();

    QgsRubberBand *rb = createRubberBand();
    rb->show();
    mRubberBands << rb;
  }
  createUserInputWidget();
  updateSimplificationPreview();
}

void QgsMapToolSimplify::keyReleaseEvent( QKeyEvent *e )
{
  if ( e->key() == Qt::Key_Escape )
  {
    clearSelection();
    return;
  }
  QgsMapTool::keyReleaseEvent( e );
}

void QgsMapToolSimplify::selectOneFeature( QPoint canvasPoint )
{
  QgsVectorLayer *vlayer = currentVectorLayer();
  const QgsPointXY layerCoords = toLayerCoordinates( vlayer, canvasPoint );
  const double r = QgsTolerance::vertexSearchRadius( vlayer, mCanvas->mapSettings() );
  const QgsRectangle selectRect = QgsRectangle( layerCoords.x() - r, layerCoords.y() - r,
                                  layerCoords.x() + r, layerCoords.y() + r );
  QgsFeatureIterator fit = vlayer->getFeatures( QgsFeatureRequest().setFilterRect( selectRect ).setNoAttributes() );

  const QgsGeometry geometry = QgsGeometry::fromPointXY( layerCoords );
  double minDistance = std::numeric_limits<double>::max();
  double currentDistance;
  QgsFeature minDistanceFeature;
  QgsFeature f;
  while ( fit.nextFeature( f ) )
  {
    currentDistance = geometry.distance( f.geometry() );
    if ( currentDistance < minDistance )
    {
      minDistance = currentDistance;
      minDistanceFeature = f;
    }
  }

  if ( minDistanceFeature.isValid() )
  {
    mSelectedFeatures << minDistanceFeature;
  }
}


void QgsMapToolSimplify::selectFeaturesInRect()
{
  QgsVectorLayer *vlayer = currentVectorLayer();
  const QgsPointXY pt1 = toMapCoordinates( mSelectionRect.topLeft() );
  const QgsPointXY pt2 = toMapCoordinates( mSelectionRect.bottomRight() );
  const QgsRectangle rect = toLayerCoordinates( vlayer, QgsRectangle( pt1, pt2 ) );

  QgsFeature f;
  QgsFeatureRequest request;
  request.setFilterRect( rect );
  request.setFlags( QgsFeatureRequest::ExactIntersect );
  request.setNoAttributes();
  QgsFeatureIterator fit = vlayer->getFeatures( request );
  while ( fit.nextFeature( f ) )
    mSelectedFeatures << f;
}


void QgsMapToolSimplify::clearSelection()
{
  mSelectedFeatures.clear();
  delete mSimplifyUserWidget;
  mSimplifyUserWidget = nullptr;
  qDeleteAll( mRubberBands );
  mRubberBands.clear();
}

void QgsMapToolSimplify::deactivate()
{
  delete mSelectionRubberBand;
  mSelectionRubberBand = nullptr;
  clearSelection();
  QgsMapTool::deactivate();
}

QString QgsMapToolSimplify::statusText() const
{
  const int percent = mOriginalVertexCount ? ( 100 * mReducedVertexCount / mOriginalVertexCount ) : 0;
  QString txt = tr( "%1 feature(s): %2 to %3 vertices (%4%)" )
                .arg( mSelectedFeatures.count() ).arg( mOriginalVertexCount ).arg( mReducedVertexCount ).arg( percent );
  if ( mReducedHasErrors )
    txt += '\n' + tr( "Simplification failed!" );
  return txt;
}
