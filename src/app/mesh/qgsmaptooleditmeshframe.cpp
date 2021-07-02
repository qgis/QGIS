/***************************************************************************
  qgsmaptooleditmeshframe.cpp - QgsMapToolEditMeshFrame

 ---------------------
 begin                : 24.6.2021
 copyright            : (C) 2021 by Vincent Cloarec
 email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsmaptooleditmeshframe.h"

#include "qgisapp.h"

#include "qgsdoublespinbox.h"
#include "qgsmeshdataprovider.h"
#include "qgsmapcanvas.h"
#include "qgsmapmouseevent.h"
#include "qgsmessagebar.h"
#include "qgsmeshlayer.h"
#include "qgsmesheditor.h"
#include "qgstriangularmesh.h"
#include "qgsrubberband.h"
#include "qgssnapindicator.h"
#include "qgsvertexmarker.h"
#include "qgsguiutils.h"


QgsZValueWidget::QgsZValueWidget( const QString &label, QWidget *parent ): QWidget( parent )
{
  QHBoxLayout *layout = new QHBoxLayout( this );
  layout->setContentsMargins( 0, 0, 0, 0 );
  setLayout( layout );

  if ( !label.isEmpty() )
  {
    QLabel *lbl = new QLabel( label, this );
    lbl->setAlignment( Qt::AlignRight | Qt::AlignCenter );
    layout->addWidget( lbl );
  }

  mZValueSpinBox = new QgsDoubleSpinBox( this );
  mZValueSpinBox->setSingleStep( 1 );

  mZValueSpinBox->setMinimum( -std::numeric_limits<double>::max() );
  mZValueSpinBox->setMaximum( std::numeric_limits<double>::max() );
  mZValueSpinBox->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Preferred );
  layout->addWidget( mZValueSpinBox );
  mZValueSpinBox->setClearValue( 0.0 );
  mZValueSpinBox->clear();

  setFocusProxy( mZValueSpinBox );
}

double QgsZValueWidget::zValue() const
{
  return mZValueSpinBox->value();
}



void QgsZValueWidget::setZValue( double z )
{
  mZValueSpinBox->setValue( z );
  mZValueSpinBox->selectAll();
}

void QgsZValueWidget::setDefaultValue( double z )
{
  mZValueSpinBox->setClearValue( z );
  mZValueSpinBox->clear();
  mZValueSpinBox->selectAll();
}

void QgsZValueWidget::setEventFilterOnValueSpinbox( QObject *filter )
{
  if ( mZValueSpinBox )
    mZValueSpinBox->installEventFilter( filter );
}

QgsMapToolEditMeshFrame::QgsMapToolEditMeshFrame( QgsMapCanvas *canvas )
  : QgsMapToolAdvancedDigitizing( canvas, QgisApp::instance()->cadDockWidget() )
  , mSnapIndicator( new QgsSnapIndicator( canvas ) )
{
  mFaceRubberBand = createRubberBand( QgsWkbTypes::PolygonGeometry );
  mFaceRubberBand->setVisible( false );

  QColor color = digitizingStrokeColor();
  mFaceVerticesBand = new QgsRubberBand( canvas );
  mFaceVerticesBand->setIcon( QgsRubberBand::ICON_CIRCLE );
  mFaceVerticesBand->setColor( color );
  mFaceVerticesBand->setWidth( QgsGuiUtils::scaleIconSize( 2 ) );
  mFaceVerticesBand->setBrushStyle( Qt::NoBrush );
  mFaceVerticesBand->setIconSize( QgsGuiUtils::scaleIconSize( 6 ) );
  mFaceVerticesBand->setVisible( false );

  mVertexBand = new QgsRubberBand( canvas );
  mVertexBand->setIcon( QgsRubberBand::ICON_CIRCLE );
  mVertexBand->setColor( color );
  mVertexBand->setWidth( QgsGuiUtils::scaleIconSize( 2 ) );
  mVertexBand->setBrushStyle( Qt::NoBrush );
  mVertexBand->setIconSize( QgsGuiUtils::scaleIconSize( 15 ) );
  mVertexBand->setVisible( false );

  mNewFaceBand = createRubberBand( QgsWkbTypes::PolygonGeometry );
  mInvalidFaceColor = QColor( 255, 0, 0, mNewFaceBand->fillColor().alpha() ); //override color and keep only the transparency
  mValidFaceColor = QColor( 0, 255, 0, mNewFaceBand->fillColor().alpha() ); //override color and keep only the transparency
  mNewFaceBand->setFillColor( mInvalidFaceColor );
  mNewFaceBand->setVisible( false );

  mSelectionBand = new QgsRubberBand( mCanvas, QgsWkbTypes::PolygonGeometry );
  QColor fillColor = QColor( 0, 120, 215, 63 );
  QColor strokeColor = QColor( 0, 102, 204, 100 );
  mSelectionBand->setFillColor( fillColor );
  mSelectionBand->setStrokeColor( strokeColor );

  mActionRemoveVerticesFillingHole = new QAction( this );
  mActionRemoveVerticesWithoutFillingHole = new QAction( this );
  connect( mActionRemoveVerticesFillingHole, &QAction::triggered, this, [this] {removeSelectedVerticesFromMesh( true );} );
  connect( mActionRemoveVerticesWithoutFillingHole, &QAction::triggered, this, [this] {removeSelectedVerticesFromMesh( false );} );

  setAutoSnapEnabled( true );
}

QgsMapToolEditMeshFrame::~QgsMapToolEditMeshFrame()
{
  deleteZvalueWidget();
}

void QgsMapToolEditMeshFrame::deactivate()
{
  disconnect( canvas(), &QgsMapCanvas::currentLayerChanged, this, &QgsMapToolEditMeshFrame::setCurrentLayer );
  clear();

  mNewFaceMarker.reset();

  QgsMapToolAdvancedDigitizing::deactivate();
}

void QgsMapToolEditMeshFrame::activate()
{
  mNewFaceMarker.reset( new QgsVertexMarker( canvas() ) );
  mNewFaceMarker->setIconType( QgsVertexMarker::ICON_TRIANGLE );
  mNewFaceMarker->setIconSize( QgsGuiUtils::scaleIconSize( 12 ) );
  mNewFaceMarker->setColor( Qt::gray );
  mNewFaceMarker->setVisible( false );
  mNewFaceMarker->setPenWidth( 3 );

  connect( canvas(), &QgsMapCanvas::currentLayerChanged, this, &QgsMapToolEditMeshFrame::setCurrentLayer );
  setCurrentLayer( canvas()->currentLayer() );
  createZValueWidget();
  updateFreeVertices();
  QgsMapToolAdvancedDigitizing::activate();
}

bool QgsMapToolEditMeshFrame::populateContextMenuWithEvent( QMenu *menu, QgsMapMouseEvent *event )
{
  Q_UNUSED( event );

  switch ( mCurrentState )
  {
    case Default:
    {
      if ( !mSelectedVertex.isEmpty() )
      {
        QList<QAction * >  newActions;
        newActions << mActionRemoveVerticesFillingHole << mActionRemoveVerticesWithoutFillingHole;
        QList<QAction * > existingActions = menu->actions();
        if ( existingActions.isEmpty() )
          menu->addActions( newActions );
        else
          menu->insertActions( existingActions.first(), newActions );
      }
      case AddingNewFace:
        return false;
      case Selecting:
        return false;
      }
  }
}

QgsMapTool::Flags QgsMapToolEditMeshFrame::flags() const
{
  switch ( mCurrentState )
  {
    case Default:
      return QgsMapTool::Flags() | QgsMapTool::ShowContextMenu;
    case AddingNewFace:
      return QgsMapTool::Flags();
    case Selecting:
      return QgsMapTool::Flags();
  }
}

bool QgsMapToolEditMeshFrame::eventFilter( QObject *obj, QEvent *ev )
{
  if ( mZValueWidget && obj )
  {
    switch ( ev->type() )
    {
      case QEvent::KeyPress:
        keyPressEvent( static_cast<QKeyEvent *>( ev ) );
        if ( ev->isAccepted() )
          return true;
        break;
      case QEvent::KeyRelease:
        keyReleaseEvent( static_cast<QKeyEvent *>( ev ) );
        if ( ev->isAccepted() )
          return true;
        break;
      default:
        return false;
        break;
    }
  }
  else
    QgsMapToolAdvancedDigitizing::eventFilter( obj, ev );

  return false;
}

void QgsMapToolEditMeshFrame::cadCanvasPressEvent( QgsMapMouseEvent *e )
{
  if ( !mCurrentEditor )
    return;

  switch ( mCurrentState )
  {
    case Default:
      if ( e->button() == Qt::LeftButton )
      {
        if ( mCurrentVertexIndex == -1 )
        {
          //start dragging a selection rubberband
          mCurrentState = Selecting;
          mStartSelectionPos = e->pos();
          mSelectionBand->reset( QgsWkbTypes::PolygonGeometry );
        }
      }
      break;
    case AddingNewFace:
      break;
    case Selecting:
      break;
  }
}

void QgsMapToolEditMeshFrame::cadCanvasReleaseEvent( QgsMapMouseEvent *e )
{
  if ( !mCurrentEditor )
    return;
  double tolerance = QgsTolerance::vertexSearchRadius( canvas()->mapSettings() );

  switch ( mCurrentState )
  {
    case Default:
    {
      if ( e->button() == Qt::LeftButton )
      {
        if ( mDoubleClick )
        {
          addVertex( e->mapPoint(), e->mapPointMatch() );
        }
        else if ( mNewFaceMarker->isVisible() &&
                  e->mapPoint().distance( mNewFaceMarker->center() ) < tolerance
                  && mCurrentVertexIndex >= 0 )
        {
          clearSelectedvertex();
          mCurrentState = AddingNewFace;
          mNewFaceMarker->setVisible( false );
          mNewFaceBand->setVisible( true );
          mNewFaceBand->reset( QgsWkbTypes::PolygonGeometry );
          addVertexToFaceCanditate( mCurrentVertexIndex );
        }
        else if ( mCurrentVertexIndex != -1 )
          setSelectedVertex( QList<int>() << mCurrentVertexIndex, e->modifiers() & Qt::ControlModifier );
        else
          setSelectedVertex( QList<int>(), false );
      }
    }
    break;
    case AddingNewFace:
      if ( e->button() == Qt::LeftButton )
      {
        if ( mDoubleClick )
        {
          addVertex( e->mapPoint(), e->mapPointMatch() );
          highlightCloseVertex( e->mapPoint() );
        }

        if ( mCurrentVertexIndex != -1 )
          addVertexToFaceCanditate( mCurrentVertexIndex );
      }
      else if ( e->button() == Qt::RightButton )
      {
        if ( testNewVertexInFaceCanditate( -1 ) )
        {
          mCurrentEditor->addFace( mNewFaceCandidate.toVector() );
          mNewFaceBand->reset( QgsWkbTypes::PolygonGeometry );
          mNewFaceCandidate.clear();
          mCurrentState = Default;
        }
      }
      break;
    case Selecting:
      mCurrentState = Default;
      QgsGeometry selectionGeom = mSelectionBand->asGeometry();
      selectVerticesInGeometry( selectionGeom, e->modifiers() & Qt::ControlModifier );
      mSelectionBand->reset( QgsWkbTypes::PolygonGeometry );
      break;
  }
  mDoubleClick = false;
}

void QgsMapToolEditMeshFrame::cadCanvasMoveEvent( QgsMapMouseEvent *e )
{
  mSnapIndicator->setMatch( e->mapPointMatch() );
  if ( mZValueWidget )
    mZValueWidget->setFocus( Qt::TabFocusReason );
  switch ( mCurrentState )
  {
    case Default:
      highlightCurrentHoveredFace( e->mapPoint() );
      highlightCloseVertex( e->mapPoint() );
      break;
    case AddingNewFace:
      mNewFaceBand->movePoint( e->mapPoint() );
      highlightCurrentHoveredFace( e->mapPoint() );
      highlightCloseVertex( e->mapPoint() );
      if ( testNewVertexInFaceCanditate( mCurrentVertexIndex ) )
        mNewFaceBand->setColor( mValidFaceColor );
      else
        mNewFaceBand->setColor( mInvalidFaceColor );
      break;
    case Selecting:
    {
      QRect rect = QRect( e->pos(), mStartSelectionPos );
      mSelectionBand->setToCanvasRectangle( rect );
    }
    break;
  }

  QgsMapToolAdvancedDigitizing::cadCanvasMoveEvent( e );
}

void QgsMapToolEditMeshFrame::keyPressEvent( QKeyEvent *e )
{
  e->ignore();

  switch ( mCurrentState )
  {
    case Default:
    {
      if ( e->key() == Qt::Key_Delete && ( e->modifiers() & Qt::ControlModifier ) )
      {
        removeSelectedVerticesFromMesh( !( e->modifiers() & Qt::ShiftModifier ) );
        e->accept();
      }

      if ( e->key() == Qt::Key_Escape )
      {
        clearSelectedvertex();
      }

      if ( e->key() == Qt::Key_Enter )
      {
        applyZValueOnSelectedVertices();
        e->accept();
      }
    }
    break;
    case AddingNewFace:
    {
      if ( e->key() == Qt::Key_Backspace )
      {
        mNewFaceBand->removeLastPoint();
        if ( !mNewFaceCandidate.isEmpty() )
          mNewFaceCandidate.removeLast();
        if ( mNewFaceCandidate.isEmpty() )
          mCurrentState = Default;
        e->accept();
      }

      if ( e->key() == Qt::Key_Escape )
      {
        mNewFaceBand->reset( QgsWkbTypes::PolygonGeometry );
        mNewFaceCandidate.clear();
        mCurrentState = Default;
        e->accept();
      }
    }
    break;
    case Selecting:
      break;
  }
}

void QgsMapToolEditMeshFrame::keyReleaseEvent( QKeyEvent *e )
{
  e->ignore();

  switch ( mCurrentState )
  {
    case Default:
      break;
    case AddingNewFace:
      if ( e->key() == Qt::Key_Backspace )
        e->accept(); //to avoid removing the value of the ZvalueWidget
      break;
    case Selecting:
      break;
  }
}

void QgsMapToolEditMeshFrame::canvasDoubleClickEvent( QgsMapMouseEvent *e )
{
  //canvasReleseaseEvent() will be called just after the last click, so just flag the double click
  Q_UNUSED( e )
  mDoubleClick = true;
}

void QgsMapToolEditMeshFrame::onEditingStopped()
{
  mCurrentEditor = nullptr;
  updateFreeVertices();
  deleteZvalueWidget();
}


const QgsMeshVertex QgsMapToolEditMeshFrame::mapVertex( int index ) const
{
  if ( mCurrentLayer.isNull() || ! mCurrentLayer->triangularMesh() )
    return QgsMeshVertex();

  return mCurrentLayer->triangularMesh()->vertices().at( index );
}

const QgsMeshFace QgsMapToolEditMeshFrame::nativeFace( int index ) const
{
  if ( mCurrentLayer.isNull() || ! mCurrentLayer->nativeMesh() )
    return QgsMeshFace();

  return mCurrentLayer->nativeMesh()->face( index );
}

void QgsMapToolEditMeshFrame::onEdidingStarted()
{
  setCurrentLayer( canvas()->currentLayer() );
}


void QgsMapToolEditMeshFrame::setCurrentLayer( QgsMapLayer *layer )
{
  QgsMeshLayer *meshLayer = qobject_cast<QgsMeshLayer *>( layer );

  if ( mCurrentLayer == meshLayer && mCurrentEditor != nullptr )
    return;

  if ( mCurrentLayer )
  {
    disconnect( mCurrentLayer, &QgsMeshLayer::editingStarted, this, &QgsMapToolEditMeshFrame::onEdidingStarted );
    disconnect( mCurrentLayer, &QgsMeshLayer::editingStopped, this, &QgsMapToolEditMeshFrame::onEditingStopped );
  }

  mCurrentLayer = meshLayer;
  mCurrentFaceIndex = -1;

  if ( mCurrentEditor )
  {
    disconnect( mCurrentEditor, &QgsMeshEditor::meshEdited, this, &QgsMapToolEditMeshFrame::onEdit );
  }

  mCurrentEditor = nullptr;
  if ( mCurrentLayer )
  {
    connect( mCurrentLayer, &QgsMeshLayer::editingStarted, this, &QgsMapToolEditMeshFrame::onEdidingStarted );
    connect( mCurrentLayer, &QgsMeshLayer::editingStopped, this, &QgsMapToolEditMeshFrame::onEditingStopped );

    if ( mCurrentLayer->isEditable() )
    {
      mCurrentEditor = mCurrentLayer->meshEditor();
      connect( mCurrentEditor, &QgsMeshEditor::meshEdited, this, &QgsMapToolEditMeshFrame::onEdit );
    }
  }

  if ( mCurrentEditor && !mZValueWidget )
  {
    createZValueWidget();
    updateFreeVertices();
  }

  if ( !mCurrentEditor && mZValueWidget )
  {
    deleteZvalueWidget();
    mFaceRubberBand->reset( QgsWkbTypes::PolygonGeometry );
    mVertexBand->reset( QgsWkbTypes::PointGeometry );
    clear();
  }
}

void QgsMapToolEditMeshFrame::onEdit()
{
  updateFreeVertices();
}

QgsPointSequence QgsMapToolEditMeshFrame::nativeFaceGeometry( int faceIndex ) const
{
  QgsPointSequence faceGeometry;
  const QgsMeshFace &face = mCurrentLayer->nativeMesh()->face( faceIndex );

  for ( const int index : face )
  {
    faceGeometry.append( mapVertex( index ) );
  }

  return faceGeometry;
}

QgsPointXY QgsMapToolEditMeshFrame::newFaceMarkerPosition( int vertexIndex )
{
  QgsVector directionVector;

  const QgsMeshVertex &v = mapVertex( vertexIndex );

  if ( mCurrentEditor->isVertexFree( vertexIndex ) )
  {
    directionVector = QgsVector( 1, 0 );
  }
  else
  {
    QgsMeshVertexCirculator circulator = mCurrentEditor->vertexCirculator( vertexIndex );
    circulator.goBoundaryClockwise();
    int indexPt1 = circulator.oppositeVertexClockWise();
    circulator.goBoundaryCounterClockwise();
    int indexPt2 = circulator.oppositeVertexCounterClockWise();

    const QgsMeshVertex &v1 = mapVertex( indexPt1 );
    const QgsMeshVertex &v2 = mapVertex( indexPt2 );

    QgsVector vector1 = v - v2;
    QgsVector vector2 = v - v1;

    vector1 = vector1.normalized();
    vector2 = vector2.normalized();

    double crossProduct = vector1.crossProduct( vector2 );

    if ( crossProduct < - 1e-8 )
      directionVector = ( vector1 + vector2 ).normalized();
    else if ( crossProduct > 1e-8 )
      directionVector = -( vector1 + vector2 ).normalized();
    else
      directionVector = vector2.perpVector();
  }

  double dist = 15 * canvas()->mapSettings().mapUnitsPerPixel();
  return v + directionVector * dist;
}

void QgsMapToolEditMeshFrame::addVertexToFaceCanditate( int vertexIndex )
{
  if ( vertexIndex == -1 && vertexIndex != mNewFaceCandidate.last() )
    return;

  mNewFaceBand->movePoint( mapVertex( vertexIndex ) );
  mNewFaceBand->addPoint( mapVertex( vertexIndex ) );
  mNewFaceCandidate.append( vertexIndex );
}

bool QgsMapToolEditMeshFrame::testNewVertexInFaceCanditate( int vertexIndex )
{
  QgsMeshFace face = mNewFaceCandidate.toVector();
  if ( vertexIndex != -1 && !face.empty() && vertexIndex != mNewFaceCandidate.last() )
    face.append( vertexIndex );
  return mCurrentEditor->faceCanBeAdded( face );
}

void QgsMapToolEditMeshFrame::setSelectedVertex( const QList<int> newSelectedVertex, bool ctrl )
{
  if ( mSelectedVertex.isEmpty() )
    mCurrentZValue = mZValueWidget->zValue();

  if ( !ctrl )
    clearSelectedvertex();

  for ( const int vertexIndex : newSelectedVertex )
  {
    if ( mSelectedVertex.contains( vertexIndex ) )
    {
      int pos = mSelectedVertex.indexOf( vertexIndex );
      mSelectedVertex.removeAt( pos );
      delete mSelectedVerticesMarker.at( pos );
      mSelectedVerticesMarker.removeAt( pos );
    }
    else
    {
      mSelectedVertex.append( vertexIndex );
      QgsVertexMarker *marker = new QgsVertexMarker( canvas() );
      marker->setIconType( QgsVertexMarker::ICON_CIRCLE );
      marker->setIconSize( QgsGuiUtils::scaleIconSize( 10 ) );
      marker->setPenWidth( QgsGuiUtils::scaleIconSize( 2 ) );
      marker->setColor( Qt::blue );
      marker->setCenter( mapVertex( vertexIndex ) );
      mSelectedVerticesMarker.append( marker );
    }
  }

  if ( !mSelectedVertex.isEmpty() )
  {
    double vertexZValue = 0;
    for ( int i = 0; i < mSelectedVertex.count(); ++i )
      vertexZValue += mapVertex( mSelectedVertex.at( i ) ).z();
    vertexZValue /= mSelectedVertex.count();

    mZValueWidget->setDefaultValue( vertexZValue );

    if ( mSelectedVertex.count() == 1 )
    {
      mActionRemoveVerticesFillingHole->setText( tr( "Remove selected vertex and fill hole" ) );
      mActionRemoveVerticesWithoutFillingHole->setText( tr( "Remove selected vertex without filling hole" ) );
    }
    else
    {
      mActionRemoveVerticesFillingHole->setText( tr( "Remove selected vertices and fill hole(s)" ) );
      mActionRemoveVerticesWithoutFillingHole->setText( tr( "Remove selected vertices without filling hole(s)" ) );
    }
  }
}

void QgsMapToolEditMeshFrame::clearSelectedvertex()
{
  mSelectedVertex.clear();
  qDeleteAll( mSelectedVerticesMarker );
  mSelectedVerticesMarker.clear();
  if ( mZValueWidget )
    mZValueWidget->setZValue( mCurrentZValue );
}

void QgsMapToolEditMeshFrame::removeSelectedVerticesFromMesh( bool fillHole )
{
  QgsMeshEditingError error = mCurrentEditor->removeVertices( mSelectedVertex, fillHole );
  if ( error != QgsMeshEditingError() )
  {
    QgisApp::instance()->messageBar()->pushWarning(
      tr( "Mesh editing" ),
      tr( "removing the vertex %1 leads to a topological error, operation cancelled." ).arg( error.elementIndex ) );
  }
  else
    clearSelectedvertex();
}

void QgsMapToolEditMeshFrame::selectVerticesInGeometry( const QgsGeometry &geometry, bool ctrl )
{
  if ( mCurrentLayer.isNull() || !mCurrentLayer->triangularMesh() || mCurrentEditor.isNull() )
    return;

  QSet<int> selectedVertices;
  const QList<int> nativeFaceIndexes = mCurrentLayer->triangularMesh()->nativeFaceIndexForRectangle( geometry.boundingBox() );

  std::unique_ptr<QgsGeometryEngine> engine( QgsGeometry::createGeometryEngine( geometry.constGet() ) );
  engine->prepareGeometry();

  for ( const int faceIndex : nativeFaceIndexes )
  {
    const QgsMeshFace &face = nativeFace( faceIndex );
    for ( const int vertexIndex : face )
    {
      const QgsMeshVertex &vertex = mapVertex( vertexIndex );
      if ( engine->contains( &vertex ) )
        selectedVertices.insert( vertexIndex );
    }
  }
  //free vertices
  const QList<int> &freeVerticesIndexes = mCurrentEditor->freeVerticesIndexes();
  for ( const int freeVertexIndex : freeVerticesIndexes )
  {
    const QgsMeshVertex &vertex = mapVertex( freeVertexIndex );
    if ( engine->contains( &vertex ) )
      selectedVertices.insert( freeVertexIndex );
  }
  setSelectedVertex( selectedVertices.values(), ctrl );
}

void QgsMapToolEditMeshFrame::applyZValueOnSelectedVertices()
{
  if ( !mZValueWidget )
    return;
  QList<double> zValues;
  zValues.reserve( mSelectedVertex.count() );
  mCurrentZValue = mZValueWidget->zValue();
  for ( int i = 0; i < mSelectedVertex.count(); ++i )
    zValues.append( mCurrentZValue );

  mCurrentEditor->changeZValues( mSelectedVertex, zValues );
}

void QgsMapToolEditMeshFrame::highlightCurrentHoveredFace( const QgsPointXY &mapPoint )
{
  int faceIndex = -1;
  if ( !mCurrentLayer.isNull() && mCurrentLayer->triangularMesh() )
    faceIndex = mCurrentLayer->triangularMesh()->nativeFaceIndexForPoint( mapPoint );

  if ( faceIndex == mCurrentFaceIndex )
    return;

  mCurrentFaceIndex = faceIndex;

  QgsPointSequence faceGeometry = nativeFaceGeometry( faceIndex );

  mFaceRubberBand->reset( QgsWkbTypes::PolygonGeometry );
  mFaceVerticesBand->reset( QgsWkbTypes::PointGeometry );
  for ( const QgsPoint &pt : faceGeometry )
  {
    mFaceRubberBand->addPoint( pt );
    mFaceVerticesBand->addPoint( pt );
  }

  return;
}

void QgsMapToolEditMeshFrame::highlightCloseVertex( const QgsPointXY &mapPoint )
{
  if ( !mCurrentEditor )
    return;

  if ( mNewFaceMarker->isVisible() )
  {
    double tol = QgsTolerance::vertexSearchRadius( canvas()->mapSettings() );
    if ( mapPoint.distance( mNewFaceMarker->center() ) < tol )
    {
      mNewFaceMarker->setColor( Qt::red );
      mVertexBand->setVisible( false );
      return;
    }
    else if ( !mVertexBand->isVisible() )
    {
      mNewFaceMarker->setVisible( false );
      mCurrentVertexIndex = -1;
    }
  }

  if ( mVertexBand->isVisible() && mVertexBand->numberOfVertices() > 0 )
  {
    double tol = QgsTolerance::vertexSearchRadius( canvas()->mapSettings() );
    if ( mVertexBand->getPoint( 0 )->distance( mapPoint ) > tol )
    {
      mVertexBand->reset( QgsWkbTypes::PointGeometry );
      mVertexBand->setVisible( false );
      mNewFaceMarker->setVisible( false );
      mCurrentVertexIndex = -1;
    }
  }
  else
  {
    int closeVert = closeVertex( mapPoint );
    mCurrentVertexIndex = -1;
    bool isBoundary = mCurrentEditor->isVertexOnBoundary( closeVert );
    bool isFree = mCurrentEditor->isVertexFree( closeVert );
    mVertexBand->reset( QgsWkbTypes::PointGeometry );

    if ( closeVert >= 0 && ( mCurrentState != AddingNewFace || isBoundary || isFree ) )
    {
      mCurrentVertexIndex = closeVert;
      mVertexBand->addPoint( mapVertex( closeVert ) );
      if ( mCurrentState == Default )
      {
        if ( isBoundary || isFree )
        {
          mNewFaceMarker->setCenter( newFaceMarkerPosition( closeVert ) );
          mNewFaceMarker->setVisible( true );
          mNewFaceMarker->setColor( Qt::gray );
        }
      }

    }
  }
}

void QgsMapToolEditMeshFrame::createZValueWidget()
{
  if ( !mCanvas )
  {
    return;
  }

  deleteZvalueWidget();

  mZValueWidget = new QgsZValueWidget( tr( "Vertex elevation:" ) );
  QgisApp::instance()->addUserInputWidget( mZValueWidget );
  mZValueWidget->setFocus( Qt::TabFocusReason );
  mZValueWidget->setEventFilterOnValueSpinbox( this );
}

void QgsMapToolEditMeshFrame::deleteZvalueWidget()
{
  if ( mZValueWidget )
  {
    mZValueWidget->releaseKeyboard();
    mZValueWidget->deleteLater();
  }

  mZValueWidget = nullptr;
}

void QgsMapToolEditMeshFrame::clear()
{
  mFaceRubberBand->reset( QgsWkbTypes::PolygonGeometry );
  mFaceVerticesBand->setVisible( false );
  mVertexBand->reset( QgsWkbTypes::PointGeometry );
  mNewFaceBand->reset( QgsWkbTypes::PolygonGeometry );
  qDeleteAll( mFreeVertexMarker );
  mFreeVertexMarker.clear();
  qDeleteAll( mSelectedVerticesMarker );
  mSelectedVerticesMarker.clear();
  deleteZvalueWidget();
  mSnapIndicator->setMatch( QgsPointLocator::Match() );
}

void QgsMapToolEditMeshFrame::addVertex( const QgsPointXY &mapPoint, const QgsPointLocator::Match &mapPointMatch )
{
  double zValue = mZValueWidget ? mZValueWidget->zValue() : std::numeric_limits<double>::quiet_NaN();

  if ( mapPointMatch.isValid() )
  {
    QgsPoint layerPoint = mapPointMatch.interpolatedPoint();
    zValue = layerPoint.z();
  }

  QVector<QgsMeshVertex> points( 1, QgsMeshVertex( mapPoint.x(), mapPoint.y(), zValue ) );
  if ( mCurrentEditor )
  {
    double tolerance = QgsTolerance::vertexSearchRadius( canvas()->mapSettings() );
    mCurrentEditor->addVertices( points, tolerance );
  }
}

void QgsMapToolEditMeshFrame::updateFreeVertices()
{
  qDeleteAll( mFreeVertexMarker );
  mFreeVertexMarker.clear();

  if ( mCurrentLayer.isNull() || mCurrentEditor.isNull() )
    return;

  const QList<int> &freeVertexIndexes = mCurrentEditor->freeVerticesIndexes();
  int freeVertexCount = freeVertexIndexes.count();
  mFreeVertexMarker.reserve( freeVertexCount );

  QColor freeVertexColor = digitizingStrokeColor();
  QColor fillFreeVertexColor = freeVertexColor.lighter();

  for ( const int freeVertexIndex : freeVertexIndexes )
  {
    mFreeVertexMarker.append( new QgsVertexMarker( canvas() ) );
    QgsVertexMarker *marker = mFreeVertexMarker.last();
    marker->setCenter( mapVertex( freeVertexIndex ) );
    marker->setIconType( QgsVertexMarker::ICON_CIRCLE );
    marker->setIconSize( QgsGuiUtils::scaleIconSize( 7 ) );
    marker->setColor( freeVertexColor );
    marker->setFillColor( fillFreeVertexColor );
  }
}

int QgsMapToolEditMeshFrame::closeVertex( const QgsPointXY &mapPoint ) const
{
  if ( !mCurrentEditor )
    return -1;


  double tolerance = QgsTolerance::vertexSearchRadius( canvas()->mapSettings() );
  if ( mCurrentFaceIndex >= 0 )
  {
    const QgsMeshFace &face = mCurrentLayer->nativeMesh()->face( mCurrentFaceIndex );
    for ( const int vertexIndex : face )
    {
      const QgsPointXY &meshVertex = mapVertex( vertexIndex );
      if ( meshVertex.distance( mapPoint ) < tolerance )
        return vertexIndex;
    }

    //nothing found int the face --> return -1;
    return -1;
  }

  //if we are here, we are outside faces, need to search for free vertices;
  const QList<int> &freeVertexIndexes = mCurrentEditor->freeVerticesIndexes();
  for ( const int vertexIndex : freeVertexIndexes )
  {
    const QgsPointXY &meshVertex = mapVertex( vertexIndex );
    if ( std::sqrt( meshVertex.sqrDist( mapPoint ) < tolerance ) )
      return vertexIndex;
  }

  return -1;
}
