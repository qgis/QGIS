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

#include "qgis.h"
#include "qgisapp.h"
#include "qgsapplication.h"

#include "qgsadvanceddigitizingdockwidget.h"
#include "qgsdoublespinbox.h"
#include "qgsmeshdataprovider.h"
#include "qgsmapcanvas.h"
#include "qgsmapmouseevent.h"
#include "qgsmessagebar.h"
#include "qgsmeshlayer.h"
#include "qgsmeshlayerutils.h"
#include "qgsmesheditor.h"
#include "qgspolygon.h"
#include "qgstriangularmesh.h"
#include "qgsrubberband.h"
#include "qgssnapindicator.h"
#include "qgsvertexmarker.h"
#include "qgsguiutils.h"
#include "qgsmeshtriangulation.h"


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

  mZValueSpinBox->setFocusPolicy( Qt::NoFocus );
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

QWidget *QgsZValueWidget::keyboardEntryWidget() const
{
  return mZValueSpinBox;
}

QgsMapToolEditMeshFrame::QgsMapToolEditMeshFrame( QgsMapCanvas *canvas )
  : QgsMapToolAdvancedDigitizing( canvas, QgisApp::instance()->cadDockWidget() )
  , mSnapIndicator( new QgsSnapIndicator( canvas ) )
{
  mActionDigitizing = new QAction( QgsApplication::getThemePixmap( QStringLiteral( "/mActionMeshDigitizing.svg" ) ), tr( "Digitize mesh elements" ) );
  mActionDigitizing->setCheckable( true );

  mActionRemoveVerticesFillingHole = new QAction( this );
  mActionDelaunayTriangulation = new QAction( tr( "Delaunay triangulation with selected vertices" ), this );
  mActionFacesRefinement = new QAction( tr( "Refine current face" ), this );
  mActionRemoveVerticesWithoutFillingHole = new QAction( this );
  mActionRemoveFaces = new QAction( tr( "Remove current face" ), this );
  mActionSplitFaces = new QAction( tr( "Split current face" ), this );

  connect( mActionRemoveVerticesFillingHole, &QAction::triggered, this, [this] {removeSelectedVerticesFromMesh( true );} );
  connect( mActionRemoveVerticesWithoutFillingHole, &QAction::triggered, this, [this] {removeSelectedVerticesFromMesh( false );} );
  connect( mActionRemoveFaces, &QAction::triggered, this, &QgsMapToolEditMeshFrame::removeFacesFromMesh );
  connect( mActionSplitFaces, &QAction::triggered, this, &QgsMapToolEditMeshFrame::splitSelectedFaces );

  connect( mActionDigitizing, &QAction::toggled, this, [this]( bool checked )
  {
    if ( checked )
      activateWithState( Digitizing );
  } );

  connect( mActionDelaunayTriangulation, &QAction::triggered, this, [this]
  {
    if ( mCurrentEditor && mSelectedVertices.count() > 3 )
    {
      QgsMeshEditingDelaunayTriangulation triangulation;
      triangulation.setInputVertices( mSelectedVertices.keys() );
      mCurrentEditor->advancedEdit( &triangulation );

      if ( !triangulation.message().isEmpty() )
        QgisApp::instance()->messageBar()->pushInfo( tr( "Delaunay triangulation" ), triangulation.message() );
    }
  } );

  connect( mActionFacesRefinement, &QAction::triggered, this, [this]
  {
    QgsMeshEditRefineFaces refinement;
    if ( mCurrentEditor && mSelectedFaces.count() > 0 )
    {
      refinement.setInputFaces( mSelectedFaces.values() );
      mCurrentEditor->advancedEdit( &refinement );
    }
    else if ( mCurrentFaceIndex != -1 )
    {
      refinement.setInputFaces( {mCurrentFaceIndex} );
      mCurrentEditor->advancedEdit( &refinement );
    }
  } );

  setAutoSnapEnabled( true );
}

void QgsMapToolEditMeshFrame::activateWithState( State state )
{
  if ( mCanvas->mapTool() != this )
  {
    mCanvas->setMapTool( this );
    onEditingStarted();
  }
  mCurrentState = state;
}

QgsMapToolEditMeshFrame::~QgsMapToolEditMeshFrame()
{
  deleteZvalueWidget();
}

QList<QAction *> QgsMapToolEditMeshFrame::actions() const
{
  return  QList<QAction *>()
          << mActionDigitizing;
}

QList<QAction *> QgsMapToolEditMeshFrame::mapToolActions()
{
  return  QList<QAction *>()
          << mActionDigitizing;
}

void QgsMapToolEditMeshFrame::initialize()
{
  if ( !mFaceRubberBand )
    mFaceRubberBand = createRubberBand( QgsWkbTypes::PolygonGeometry );
  mFaceRubberBand->setVisible( false );
  mFaceRubberBand->setZValue( 5 );

  QColor color = digitizingStrokeColor();
  if ( !mFaceVerticesBand )
    mFaceVerticesBand = new QgsRubberBand( mCanvas );
  mFaceVerticesBand->setIcon( QgsRubberBand::ICON_CIRCLE );
  mFaceVerticesBand->setColor( color );
  mFaceVerticesBand->setWidth( QgsGuiUtils::scaleIconSize( 2 ) );
  mFaceVerticesBand->setBrushStyle( Qt::NoBrush );
  mFaceVerticesBand->setIconSize( QgsGuiUtils::scaleIconSize( 6 ) );
  mFaceVerticesBand->setVisible( false );
  mFaceVerticesBand->setZValue( 5 );

  if ( !mVertexBand )
    mVertexBand = new QgsRubberBand( mCanvas );
  mVertexBand->setIcon( QgsRubberBand::ICON_CIRCLE );
  mVertexBand->setColor( color );
  mVertexBand->setWidth( QgsGuiUtils::scaleIconSize( 2 ) );
  mVertexBand->setBrushStyle( Qt::NoBrush );
  mVertexBand->setIconSize( QgsGuiUtils::scaleIconSize( 15 ) );
  mVertexBand->setVisible( false );
  mVertexBand->setZValue( 5 );

  if ( !mEdgeBand )
    mEdgeBand = new QgsRubberBand( mCanvas );
  QColor color2( color );
  color2.setAlpha( color2.alpha() / 3 );
  mEdgeBand->setColor( color2 );
  mEdgeBand->setWidth( QgsGuiUtils::scaleIconSize( 10 ) );
  mEdgeBand->setVisible( false );

  if ( !mNewFaceBand )
    mNewFaceBand = createRubberBand( QgsWkbTypes::PolygonGeometry );
  mInvalidFaceColor = QColor( 255, 0, 0, mNewFaceBand->fillColor().alpha() ); //override color and keep only the transparency
  mValidFaceColor = QColor( 0, 255, 0, mNewFaceBand->fillColor().alpha() ); //override color and keep only the transparency
  mNewFaceBand->setFillColor( mInvalidFaceColor );
  mNewFaceBand->setVisible( false );
  mNewFaceBand->setZValue( 10 );

  if ( !mSelectionBand )
    mSelectionBand = new QgsRubberBand( mCanvas, QgsWkbTypes::PolygonGeometry );
  mSelectionBandPartiallyFillColor = QColor( 0, 215, 120, 63 );
  mSelectionBandPartiallyStrokeColor = QColor( 0, 204, 102, 100 );
  mSelectionBandTotalFillColor = QColor( 0, 120, 215, 63 );
  mSelectionBandTotalStrokeColor = QColor( 0, 102, 204, 100 );
  mSelectionBand->setFillColor( mSelectionBandTotalFillColor );
  mSelectionBand->setStrokeColor( mSelectionBandTotalStrokeColor );
  mSelectionBand->setZValue( 10 );

  if ( !mSelectedFacesRubberband )
    mSelectedFacesRubberband = new QgsRubberBand( mCanvas, QgsWkbTypes::PolygonGeometry );
  mSelectedFacesRubberband->setZValue( 1 );

  if ( !mNewFaceMarker )
    mNewFaceMarker = new QgsVertexMarker( canvas() );
  mNewFaceMarker->setIconType( QgsVertexMarker::ICON_TRIANGLE );
  mNewFaceMarker->setIconSize( QgsGuiUtils::scaleIconSize( 12 ) );
  mNewFaceMarker->setColor( Qt::gray );
  mNewFaceMarker->setVisible( false );
  mNewFaceMarker->setPenWidth( 3 );

  if ( !mSelectFaceMarker )
    mSelectFaceMarker = new QgsVertexMarker( canvas() );
  mSelectFaceMarker->setIconType( QgsVertexMarker::ICON_BOX );
  mSelectFaceMarker->setIconSize( QgsGuiUtils::scaleIconSize( 10 ) );
  mSelectFaceMarker->setColor( Qt::gray );
  mSelectFaceMarker->setFillColor( Qt::gray );
  mSelectFaceMarker->setVisible( false );
  mSelectFaceMarker->setPenWidth( 3 );
  mSelectFaceMarker->setZValue( 10 );

  if ( !mSelectEdgeMarker )
    mSelectEdgeMarker = new QgsVertexMarker( canvas() );
  mSelectEdgeMarker->setIconType( QgsVertexMarker::ICON_BOX );
  mSelectEdgeMarker->setIconSize( QgsGuiUtils::scaleIconSize( 10 ) );
  mSelectEdgeMarker->setColor( Qt::gray );
  mSelectEdgeMarker->setFillColor( Qt::gray );
  mSelectEdgeMarker->setVisible( false );
  mSelectEdgeMarker->setPenWidth( 3 );
  mSelectEdgeMarker->setZValue( 10 );

  if ( !mMovingEdgesRubberband )
    mMovingEdgesRubberband = createRubberBand( QgsWkbTypes::LineGeometry );

  if ( !mMovingFacesRubberband )
    mMovingFacesRubberband = createRubberBand( QgsWkbTypes::PolygonGeometry );

  if ( !mMovingVerticesRubberband )
    mMovingVerticesRubberband = createRubberBand( QgsWkbTypes::PointGeometry );
  mMovingEdgesRubberband->setWidth( QgsGuiUtils::scaleIconSize( 2 ) );
  mMovingEdgesRubberband->setBrushStyle( Qt::NoBrush );
  mMovingEdgesRubberband->setIconSize( QgsGuiUtils::scaleIconSize( 10 ) );
  mMovingEdgesRubberband->setVisible( false );
  mMovingEdgesRubberband->setZValue( 5 );

  if ( !mFlipEdgeMarker )
    mFlipEdgeMarker = new QgsVertexMarker( canvas() );
  mFlipEdgeMarker->setIconType( QgsVertexMarker::ICON_CIRCLE );
  mFlipEdgeMarker->setIconSize( QgsGuiUtils::scaleIconSize( 12 ) );
  mFlipEdgeMarker->setColor( Qt::gray );
  mFlipEdgeMarker->setVisible( false );
  mFlipEdgeMarker->setPenWidth( 3 );
  mFlipEdgeMarker->setZValue( 10 );

  if ( !mMergeFaceMarker )
    mMergeFaceMarker = new QgsVertexMarker( canvas() );
  mMergeFaceMarker->setIconType( QgsVertexMarker::ICON_X );
  mMergeFaceMarker->setIconSize( QgsGuiUtils::scaleIconSize( 12 ) );
  mMergeFaceMarker->setColor( Qt::gray );
  mMergeFaceMarker->setVisible( false );
  mMergeFaceMarker->setPenWidth( 3 );
  mMergeFaceMarker->setZValue( 10 );

  connect( mCanvas, &QgsMapCanvas::currentLayerChanged, this, &QgsMapToolEditMeshFrame::setCurrentLayer );

  createZValueWidget();
  updateFreeVertices();

  mIsInitialized = true;
}

void QgsMapToolEditMeshFrame::deactivate()
{
  clearSelection();
  clearCanvasHelpers();
  deleteZvalueWidget();
  qDeleteAll( mFreeVertexMarker );
  mFreeVertexMarker.clear();

  QgsMapToolAdvancedDigitizing::deactivate();
}

void QgsMapToolEditMeshFrame::clearAll()
{
  delete mNewFaceMarker;
  mNewFaceMarker = nullptr;

  delete mSelectFaceMarker;
  mSelectFaceMarker = nullptr;

  delete mSelectEdgeMarker;
  mSelectEdgeMarker = nullptr;

  delete mFlipEdgeMarker;
  mFlipEdgeMarker = nullptr;

  delete mMergeFaceMarker;
  mMergeFaceMarker = nullptr;

  mFaceRubberBand->deleteLater();
  mFaceRubberBand = nullptr;

  mFaceVerticesBand ->deleteLater();
  mFaceVerticesBand = nullptr;

  mVertexBand->deleteLater();
  mVertexBand = nullptr;

  mNewFaceBand->deleteLater();
  mNewFaceBand = nullptr;

  mSelectionBand->deleteLater();
  mSelectionBand = nullptr;

  mSelectedFacesRubberband->deleteLater();
  mSelectedFacesRubberband = nullptr;

  deleteZvalueWidget();
}

void QgsMapToolEditMeshFrame::activate()
{
  QgsMapToolAdvancedDigitizing::activate();
}

bool QgsMapToolEditMeshFrame::populateContextMenuWithEvent( QMenu *menu, QgsMapMouseEvent *event )
{
  Q_UNUSED( event );

  switch ( mCurrentState )
  {
    case Digitizing:
    {
      QList<QAction * >  newActions;

      QList<QAction * >  lastActions;

      if ( !mSelectedVertices.isEmpty() )
      {
        if ( mSelectedVertices.count() >= 3 )
          lastActions << mActionDelaunayTriangulation;

        newActions << mActionRemoveVerticesFillingHole << mActionRemoveVerticesWithoutFillingHole;
      }

      if ( !mSelectedFaces.isEmpty() || mCurrentFaceIndex != -1 )
      {
        newActions << mActionRemoveFaces;
        lastActions << mActionFacesRefinement;
      }

      if ( mSplittableFaceCount > 0 ||
           ( mCurrentFaceIndex != -1 && mCurrentEditor->faceCanBeSplit( mCurrentFaceIndex ) ) )
        newActions << mActionSplitFaces;

      QList<QAction * > existingActions = menu->actions();
      if ( !newActions.isEmpty() )
      {
        if ( existingActions.isEmpty() )
        {
          menu->addActions( newActions );
          if ( !lastActions.empty() )
          {
            menu->addSeparator();
            menu->addActions( lastActions );
          }
        }
        else
        {
          menu->insertActions( existingActions.first(), newActions );
          menu->insertSeparator( existingActions.first() );
          menu->insertActions( existingActions.first(), lastActions );
          menu->insertSeparator( existingActions.first() );
        }
        return true;
      }
      return false;
    }
    case AddingNewFace:
    case Selecting:
    case MovingVertex:
      return false;
  }

  return false;
}

QgsMapTool::Flags QgsMapToolEditMeshFrame::flags() const
{
  switch ( mCurrentState )
  {
    case Digitizing:
      return QgsMapTool::Flags() | QgsMapTool::ShowContextMenu;
    case AddingNewFace:
    case Selecting:
    case MovingVertex:
      return QgsMapTool::Flags();
  }

  return QgsMapTool::Flags();
}

void QgsMapToolEditMeshFrame::cadCanvasPressEvent( QgsMapMouseEvent *e )
{
  if ( !mCurrentEditor )
    return;

  if ( e->button() == Qt::LeftButton )
    mLeftButtonPressed = true;

  double tolerance = QgsTolerance::vertexSearchRadius( canvas()->mapSettings() );
  const QgsPointXY mapPoint = e->mapPoint();

  if ( e->button() == Qt::LeftButton )
  {
    mStartSelectionPos = e->pos();
    mSelectionBand->reset( QgsWkbTypes::PolygonGeometry );
    switch ( mCurrentState )
    {
      case Digitizing:
        mCanMovingStart = false;

        if ( ( mSelectFaceMarker->isVisible() &&
               mapPoint.distance( mSelectFaceMarker->center() ) < tolerance
               && mCurrentFaceIndex >= 0
               && mSelectedFaces.contains( mCurrentFaceIndex ) ) )
        {
          mStartMovingPoint = mCurrentLayer->triangularMesh()->faceCentroids().at( mCurrentFaceIndex );
          mCanMovingStart = true;
        }

        if ( mCurrentEdge.first != -1 && mCurrentEdge.second != -1 &&
             mSelectEdgeMarker->isVisible() &&
             mapPoint.distance( mSelectEdgeMarker->center() ) < tolerance )
        {
          QVector<int> edgeVert( edgeVertices( mCurrentEdge ) );
          if ( mSelectedVertices.contains( edgeVert.at( 0 ) ) && mSelectedVertices.contains( edgeVert.at( 1 ) ) )
          {
            QVector<QgsPointXY> edgeGeom = edgeGeometry( mCurrentEdge );
            mStartMovingPoint = QgsPointXY( ( edgeGeom.at( 0 ).x() + edgeGeom.at( 1 ).x() ) / 2,
                                            ( edgeGeom.at( 0 ).y() + edgeGeom.at( 1 ).y() ) / 2 );
            mCanMovingStart = true;
          }
        }

        if ( mSelectedVertices.contains( mCurrentVertexIndex ) )
        {
          mStartMovingPoint = mapVertexXY( mCurrentVertexIndex );
          mCanMovingStart = true;
        }
        mPreviousState = mCurrentState;
        break;
      case AddingNewFace:
      case Selecting:
      case MovingVertex:
        break;
    }
  }

  QgsMapToolAdvancedDigitizing::cadCanvasPressEvent( e );
}

void QgsMapToolEditMeshFrame::cadCanvasMoveEvent( QgsMapMouseEvent *e )
{
  if ( !mCurrentEditor )
    return;

  const QgsPointXY &mapPoint = e->mapPoint();

  mSnapIndicator->setMatch( e->mapPointMatch() );

  if ( mLeftButtonPressed &&
       mCurrentState != MovingVertex  &&
       mCurrentState != AddingNewFace )
  {
    if ( mCanMovingStart )
    {
      mCurrentState = MovingVertex;
      mCanMovingStart = false;
    }
    else
      mCurrentState = Selecting;
  }

  switch ( mCurrentState )
  {
    case Digitizing:
      highLight( mapPoint );
      break;
    case AddingNewFace:
      mNewFaceBand->movePoint( mapPoint );
      highLight( mapPoint );
      if ( testNewVertexInFaceCanditate( mCurrentVertexIndex ) )
        mNewFaceBand->setColor( mValidFaceColor );
      else
        mNewFaceBand->setColor( mInvalidFaceColor );
      break;
    case Selecting:
    {
      const QRect &rect = QRect( e->pos(), mStartSelectionPos );
      mSelectPartiallyContainedFace = e->pos().x() < mStartSelectionPos.x();
      if ( mSelectPartiallyContainedFace )
      {
        mSelectionBand->setFillColor( mSelectionBandPartiallyFillColor );
        mSelectionBand->setColor( mSelectionBandPartiallyStrokeColor );
      }
      else
      {
        mSelectionBand->setFillColor( mSelectionBandTotalFillColor );
        mSelectionBand->setColor( mSelectionBandTotalStrokeColor );
      }

      mSelectionBand->setToCanvasRectangle( rect );
    }
    break;
    case MovingVertex:
    {
      const QgsVector &translation = mapPoint - mStartMovingPoint;
      mMovingEdgesRubberband->reset( QgsWkbTypes::LineGeometry );
      mMovingVerticesRubberband->reset( QgsWkbTypes::PointGeometry );
      mMovingFacesRubberband->reset( QgsWkbTypes::PolygonGeometry );
      QgsGeometry faceGeom = mSelectedFacesRubberband->asGeometry();
      faceGeom.translate( translation.x(), translation.y() );
      mMovingFacesRubberband->setToGeometry( faceGeom );

      mIsMovingAllowed = true;
      for ( QMap<int, SelectedVertexData>::const_iterator it = mSelectedVertices.constBegin(); it != mSelectedVertices.constEnd(); ++it )
      {
        const QgsPointXY &point1 = mapVertexXY( it.key() ) + translation;
        const SelectedVertexData &vertexData = it.value();
        for ( int i = 0; i < vertexData.meshFixedEdges.count(); ++i )
        {
          const QgsPointXY point2 = mapVertexXY( vertexData.meshFixedEdges.at( i ).second );
          QgsGeometry edge( new QgsLineString( {point1, point2} ) );
          mMovingEdgesRubberband->addGeometry( edge );
          if ( mIsMovingAllowed )
            mIsMovingAllowed &= testBorderMovingFace( nativeFace( vertexData.meshFixedEdges.at( i ).first ), translation );
        }

        for ( int i = 0; i < vertexData.selectedEdges.count(); ++i )
        {
          const QgsPointXY point2 = mapVertexXY( vertexData.selectedEdges.at( i ).second ) + translation;
          const QgsPointXY middlePoint( ( point1.x() + point2.x() ) / 2, ( point1.y() + point2.y() ) / 2 );
          if ( !faceGeom.contains( &middlePoint ) )
          {
            QgsGeometry edge( new QgsLineString( {point1, point2} ) );
            mMovingEdgesRubberband->addGeometry( edge );
          }
        }
      }

      if ( mIsMovingAllowed )
      {
        //to finish test if the polygons formed by the moving faces contains something else
        const QList<int> &faceIndexesIntersect = mCurrentLayer->triangularMesh()->nativeFaceIndexForRectangle( faceGeom.boundingBox() );
        for ( const int faceIndex : faceIndexesIntersect )
        {
          if ( mConcernedFaceBySelection.contains( faceIndex ) )
            continue;
          const QgsGeometry otherFaceGeom( new QgsPolygon( new QgsLineString( nativeFaceGeometry( faceIndex ) ) ) );
          mIsMovingAllowed &= !faceGeom.intersects( otherFaceGeom );
          if ( !mIsMovingAllowed )
            break;
        }

        if ( mIsMovingAllowed ) //last check, the free vertices...
        {
          const QList<int> &freeVerticesIndexes = mCurrentEditor->freeVerticesIndexes();
          for ( const int vertexIndex : freeVerticesIndexes )
          {
            const QgsPointXY &pointInMap = mapVertexXY( vertexIndex );
            mIsMovingAllowed &= !faceGeom.contains( &pointInMap );
            if ( !mIsMovingAllowed )
              break;
          }
        }
      }

      if ( mIsMovingAllowed )
      {
        mMovingFacesRubberband->setFillColor( QColor( 0, 200, 0, 100 ) );
        mMovingEdgesRubberband->setColor( QColor( 0, 200, 0, 100 ) );
      }
      else
      {
        mMovingFacesRubberband->setFillColor( QColor( 200, 0, 0, 100 ) );
        mMovingEdgesRubberband->setColor( QColor( 200, 0, 0, 100 ) );
      }
    }
    break;
  }

  QgsMapToolAdvancedDigitizing::cadCanvasMoveEvent( e );
}

void QgsMapToolEditMeshFrame::cadCanvasReleaseEvent( QgsMapMouseEvent *e )
{
  if ( !mCurrentEditor )
    return;
  double tolerance = QgsTolerance::vertexSearchRadius( canvas()->mapSettings() );

  const QgsPointXY &mapPoint = e->mapPoint();

  // advanced digitizing constraint only the first release of double clicks
  // so we nned to store for the next one that could be a double clicks
  if ( !mDoubleClicks )
  {
    mLastClickPoint = mapPoint;

    mIsSelectedZValue = !mSelectedVertices.isEmpty() ||
                        ( mIsSelectedZValue && ( e->modifiers() &Qt::ControlModifier ) );
    if ( mIsSelectedZValue && mZValueWidget )
    {
      if ( !mSelectedVertices.isEmpty() )
        mSelectedZValue = mZValueWidget->zValue();
    }
    else if ( !mSelectedZValue && mZValueWidget )
      if ( mSelectedVertices.isEmpty() )
        mZValueWidget->setDefaultValue( mOrdinaryZValue );
  }

  if ( e->button() == Qt::LeftButton )
    mLeftButtonPressed = false;

  switch ( mCurrentState )
  {
    case Digitizing:
    {
      if ( e->button() == Qt::LeftButton )
      {
        if ( mDoubleClicks )  //double clicks --> add a vertex
        {
          addVertex( mapPoint, e->mapPointMatch(), e->modifiers() );
        }
        else if ( mNewFaceMarker->isVisible() &&
                  mapPoint.distance( mNewFaceMarker->center() ) < tolerance
                  && mCurrentVertexIndex >= 0 )  //new face marker clicked --> start adding a new face
        {
          clearSelection();
          mCurrentState = AddingNewFace;
          mNewFaceMarker->setVisible( false );
          mNewFaceBand->setVisible( true );
          mNewFaceBand->reset( QgsWkbTypes::PolygonGeometry );
          addVertexToFaceCanditate( mCurrentVertexIndex );
          const QgsPointXY &currentPoint = mapVertexXY( mCurrentVertexIndex );
          cadDockWidget()->setPoints( QList<QgsPointXY>() << currentPoint << currentPoint );
        }
        else if ( mFlipEdgeMarker->isVisible() &&
                  e->mapPoint().distance( mFlipEdgeMarker->center() ) < tolerance &&
                  mCurrentEdge.first != -1 && mCurrentEdge.second != -1 )
        {
          clearSelection();
          QVector<int> edgeVert = edgeVertices( mCurrentEdge );
          mCurrentEditor->flipEdge( edgeVert.at( 0 ), edgeVert.at( 1 ) );
          mCurrentEdge = {-1, -1};
          highLight( mapPoint );
        }
        else if ( mMergeFaceMarker->isVisible() &&
                  e->mapPoint().distance( mMergeFaceMarker->center() ) < tolerance &&
                  mCurrentEdge.first != -1 && mCurrentEdge.second != -1 )
        {
          clearSelection();
          QVector<int> edgeVert = edgeVertices( mCurrentEdge );
          mCurrentEditor->merge( edgeVert.at( 0 ), edgeVert.at( 1 ) );
          mCurrentEdge = {-1, -1};
          highLight( mapPoint );
        }
        else // try to select
        {
          select( mapPoint, e->modifiers(), tolerance );
        }
      }
    }
    break;
    case AddingNewFace:
      if ( e->button() == Qt::LeftButton ) //eventually add a vertex to the face
      {
        if ( mDoubleClicks )
        {
          addVertex( mLastClickPoint, e->mapPointMatch(), e->modifiers() );
          highlightCloseVertex( mLastClickPoint );
        }

        if ( mCurrentVertexIndex != -1 )
        {
          addVertexToFaceCanditate( mCurrentVertexIndex );
          QgsPointXY currentPoint = mapVertexXY( mCurrentVertexIndex );
          cadDockWidget()->setPoints( QList<QgsPointXY>() << currentPoint << currentPoint );
        }
      }
      else if ( e->button() == Qt::RightButton ) //if possible validate and add the face to the mesh
      {
        if ( testNewVertexInFaceCanditate( -1 ) )
        {
          mCurrentEditor->addFace( mNewFaceCandidate.toVector() );
          mNewFaceBand->reset( QgsWkbTypes::PolygonGeometry );
          mNewFaceCandidate.clear();
          mCurrentState = Digitizing;
        }
      }
      break;
    case Selecting:
    {
      mCurrentState = mPreviousState;
      QgsGeometry selectionGeom = mSelectionBand->asGeometry();
      selectInGeometry( selectionGeom, e->modifiers() );
      mSelectionBand->reset( QgsWkbTypes::PolygonGeometry );
    }
    break;
    case MovingVertex:
      if ( mIsMovingAllowed )
      {
        const QList<int> verticesIndexes = mSelectedVertices.keys();
        QList<QgsPointXY> newPosition;
        newPosition.reserve( verticesIndexes.count() );
        QgsVector translation = e->mapPoint() - mStartMovingPoint;
        for ( int i = 0; i < verticesIndexes.count(); ++i )
        {
          newPosition.append( mapVertexXY( verticesIndexes.at( i ) ) + translation );
        }
        mKeepSelectionOnEdit = true;
        mCurrentEditor->changeXYValues( mSelectedVertices.keys(), newPosition );
      }
      updateSelectecVerticesMarker();
      prepareSelection();
      clearCanvasHelpers();
      mMovingEdgesRubberband->reset();
      mMovingFacesRubberband->reset();
      mMovingVerticesRubberband->reset();
      mCurrentState = Digitizing;
      break;
  }
  mDoubleClicks = false;

  if ( !( e->modifiers() & Qt::ControlModifier ) &&
       mSelectedVertices.isEmpty() &&
       mCurrentVertexIndex == -1 &&
       mZValueWidget )
    mZValueWidget->setDefaultValue( mOrdinaryZValue );

  QgsMapToolAdvancedDigitizing::cadCanvasReleaseEvent( e );
}

void QgsMapToolEditMeshFrame::select( const QgsPointXY &mapPoint, Qt::KeyboardModifiers modifiers, double tolerance )
{
  if ( mSelectFaceMarker->isVisible() &&
       mapPoint.distance( mSelectFaceMarker->center() ) < tolerance
       && mCurrentFaceIndex >= 0 )
  {
    setSelectedVertices( nativeFace( mCurrentFaceIndex ).toList(), modifiers );
  }
  else if ( mCurrentVertexIndex != -1 )
  {
    setSelectedVertices( QList<int>() << mCurrentVertexIndex, modifiers );
  }
  else if ( mSelectEdgeMarker->isVisible() &&
            mapPoint.distance( mSelectEdgeMarker->center() ) < tolerance &&
            mCurrentEdge.first != -1 && mCurrentEdge.second != -1 )
  {
    QVector<int> edgeVert = edgeVertices( mCurrentEdge );
    setSelectedVertices( edgeVert.toList(), modifiers );
  }
  else
    setSelectedVertices( QList<int>(),  modifiers );
}

bool QgsMapToolEditMeshFrame::testBorderMovingFace( const QgsMeshFace &borderMovingfaces, const QgsVector &translation ) const
{
  int faceSize = borderMovingfaces.count();
  QgsPolygonXY polygon;
  QVector<QgsPointXY> points( faceSize );
  for ( int i = 0; i < faceSize; ++i )
  {
    int ip0 =  borderMovingfaces[i];
    int ip1 = borderMovingfaces[( i + 1 ) % faceSize];
    int ip2 = borderMovingfaces[( i + 2 ) % faceSize];

    QgsPointXY p0 = mCurrentLayer->nativeMesh()->vertices.at( ip0 ) + ( mSelectedVertices.contains( ip0 ) ? translation : QgsVector( 0, 0 ) );
    QgsPointXY p1 = mCurrentLayer->nativeMesh()->vertices.at( ip1 ) + ( mSelectedVertices.contains( ip1 ) ? translation : QgsVector( 0, 0 ) );
    QgsPointXY p2 = mCurrentLayer->nativeMesh()->vertices.at( ip2 ) + ( mSelectedVertices.contains( ip2 ) ? translation : QgsVector( 0, 0 ) );

    double ux = p0.x() - p1.x();
    double uy = p0.y() - p1.y();
    double vx = p2.x() - p1.x();
    double vy = p2.y() - p1.y();

    double crossProduct = ux * vy - uy * vx;
    if ( crossProduct >= 0 ) //if cross product>0, we have two edges clockwise
      return false;
    points[i] = p0;
  }
  polygon.append( points );

  const QgsGeometry &deformedFace = QgsGeometry::fromPolygonXY( polygon );

  // now test if the deformedface contain something else
  QList<int> otherFaceIndexes = mCurrentLayer->triangularMesh()->nativeFaceIndexForRectangle( deformedFace.boundingBox() );
  {
    for ( const int otherFaceIndex : otherFaceIndexes )
    {
      if ( mConcernedFaceBySelection.contains( otherFaceIndex ) )
        continue;

      const QgsMeshFace &otherFace = nativeFace( otherFaceIndex );
      int existingFaceSize = otherFace.count();
      bool shareVertex = false;
      for ( int i = 0; i < existingFaceSize; ++i )
      {
        if ( borderMovingfaces.contains( otherFace.at( i ) ) )
        {
          shareVertex = true;
          break;
        }
      }
      if ( shareVertex )
      {
        //only test the edge that not contain a shared vertex
        for ( int i = 0; i < existingFaceSize; ++i )
        {
          int index1 = otherFace.at( i );
          int index2 = otherFace.at( ( i + 1 ) % existingFaceSize );
          if ( ! borderMovingfaces.contains( index1 )  && !borderMovingfaces.contains( index2 ) )
          {
            const QgsPointXY &v1 = mapVertexXY( index1 );
            const QgsPointXY &v2 = mapVertexXY( index2 );
            QgsGeometry edgeGeom = QgsGeometry::fromPolylineXY( { v1, v2} );
            if ( deformedFace.intersects( edgeGeom ) )
              return false;
          }
        }
      }
      else
      {
        const QgsGeometry existingFaceGeom( new QgsPolygon( new QgsLineString( nativeFaceGeometry( otherFaceIndex ) ) ) );
        if ( deformedFace.intersects( existingFaceGeom ) )
          return false;
      }
    }
  }

  //finish with free vertices...
  const QList<int> freeVerticesIndex = mCurrentEditor->freeVerticesIndexes();
  for ( const int vertexIndex : freeVerticesIndex )
  {
    const QgsPointXY &mapPoint = mapVertexXY( vertexIndex );
    if ( deformedFace.contains( &mapPoint ) )
      return false;
  }

  return true;
}



void QgsMapToolEditMeshFrame::keyPressEvent( QKeyEvent *e )
{
  bool consumned = false;
  switch ( mCurrentState )
  {
    case Digitizing:
    {
      if ( e->key() == Qt::Key_Delete && ( e->modifiers() & Qt::ControlModifier ) )
      {
        removeSelectedVerticesFromMesh( !( e->modifiers() & Qt::ShiftModifier ) );
        consumned = true;
      }
      else if ( e->key() == Qt::Key_Delete && ( e->modifiers() & Qt::ShiftModifier ) )
      {
        removeFacesFromMesh();
        consumned = true;
      }

      if ( e->key() == Qt::Key_Escape )
      {
        clearSelection();
        consumned = true;
      }

      if ( e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter )
      {
        applyZValueOnSelectedVertices();
        consumned = true;
      }
    }
    break;
    case AddingNewFace:
    {
      if ( e->key() == Qt::Key_Backspace )
      {
        mNewFaceBand->removePoint( -2, true );
        if ( !mNewFaceCandidate.isEmpty() )
          mNewFaceCandidate.removeLast();
        if ( mNewFaceCandidate.isEmpty() )
          mCurrentState = Digitizing;
        consumned = true;
      }

      if ( e->key() == Qt::Key_Escape )
      {
        mNewFaceBand->reset( QgsWkbTypes::PolygonGeometry );
        mNewFaceCandidate.clear();
        mCurrentState = Digitizing;
        consumned = true;
      }
    }
    break;
    case Selecting:
    case MovingVertex:
      break;
  }

  if ( !consumned && mZValueWidget )
    QgsApplication::sendEvent( mZValueWidget->keyboardEntryWidget(), e );
  else
    e->ignore();

  if ( e->key() == Qt::Key_Backspace || e->key() == Qt::Key_Delete )
    e->ignore();

  QgsMapToolAdvancedDigitizing::keyPressEvent( e );
}

void QgsMapToolEditMeshFrame::keyReleaseEvent( QKeyEvent *e )
{
  bool consumned = false;
  switch ( mCurrentState )
  {
    case Digitizing:
      break;
    case AddingNewFace:
      if ( e->key() == Qt::Key_Backspace )
        consumned = true; //to avoid removing the value of the ZvalueWidget
      break;
    case Selecting:
    case MovingVertex:
      break;
  }

  if ( !consumned )
    QgsApplication::sendEvent( mZValueWidget->keyboardEntryWidget(), e );

  QgsMapToolAdvancedDigitizing::keyReleaseEvent( e );
}

void QgsMapToolEditMeshFrame::canvasDoubleClickEvent( QgsMapMouseEvent *e )
{
  Q_UNUSED( e )
  //canvasReleseaseEvent() will be called just after the last click, so just flag the double clicks
  mDoubleClicks = true;

  QgsMapToolAdvancedDigitizing::canvasDoubleClickEvent( e );
}

void QgsMapToolEditMeshFrame::onEditingStopped()
{
  mCurrentEditor = nullptr;
  deactivate();
}

const QgsMeshVertex QgsMapToolEditMeshFrame::mapVertex( int index ) const
{
  if ( mCurrentLayer.isNull() || ! mCurrentLayer->triangularMesh() )
    return QgsMeshVertex();

  return mCurrentLayer->triangularMesh()->vertices().at( index );
}

const QgsPointXY QgsMapToolEditMeshFrame::mapVertexXY( int index ) const
{
  const QgsMeshVertex &v = mapVertex( index );
  return QgsPointXY( v.x(), v.y() );
}

const QgsMeshFace QgsMapToolEditMeshFrame::nativeFace( int index ) const
{
  if ( mCurrentLayer.isNull() || ! mCurrentLayer->nativeMesh() )
    return QgsMeshFace();

  return mCurrentLayer->nativeMesh()->face( index );
}

void QgsMapToolEditMeshFrame::highLight( const QgsPointXY &mapPoint )
{
  highlightCurrentHoveredFace( mapPoint );
  highlightCloseVertex( mapPoint );
  highlightCloseEdge( mapPoint );
}

void QgsMapToolEditMeshFrame::onEditingStarted()
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
    disconnect( mCurrentLayer, &QgsMeshLayer::editingStarted, this, &QgsMapToolEditMeshFrame::onEditingStarted );
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
    connect( mCurrentLayer, &QgsMeshLayer::editingStarted, this, &QgsMapToolEditMeshFrame::onEditingStarted );
    connect( mCurrentLayer, &QgsMeshLayer::editingStopped, this, &QgsMapToolEditMeshFrame::onEditingStopped );

    if ( mCurrentLayer->isEditable() )
    {
      mCurrentEditor = mCurrentLayer->meshEditor();
      if ( !mIsInitialized )
        initialize();
      connect( mCurrentEditor, &QgsMeshEditor::meshEdited, this, &QgsMapToolEditMeshFrame::onEdit );
    }
  }

  if ( mCurrentEditor && !mZValueWidget )
  {
    createZValueWidget();
    updateFreeVertices();
  }

  if ( !mCurrentEditor )
  {
    deactivate();
  }
}

void QgsMapToolEditMeshFrame::onEdit()
{
  if ( !mKeepSelectionOnEdit )
    clearSelection();
  mKeepSelectionOnEdit = false;
  clearCanvasHelpers();
  updateFreeVertices();
}

QgsPointSequence QgsMapToolEditMeshFrame::nativeFaceGeometry( int faceIndex ) const
{
  QgsPointSequence faceGeometry;
  const QgsMeshFace &face = mCurrentLayer->nativeMesh()->face( faceIndex );

  for ( const int index : face )
    faceGeometry.append( mapVertex( index ) );

  return faceGeometry;
}

QVector<QgsPointXY> QgsMapToolEditMeshFrame::edgeGeometry( const QgsMapToolEditMeshFrame::Edge &edge ) const
{
  const QVector<int> &vertexIndexes = edgeVertices( edge );

  return {mapVertexXY( vertexIndexes.at( 0 ) ), mapVertexXY( vertexIndexes.at( 1 ) )};
}

QVector<int> QgsMapToolEditMeshFrame::edgeVertices( const QgsMapToolEditMeshFrame::Edge &edge ) const
{
  const QgsMeshFace &face = nativeFace( edge.first );
  int faceSize = face.count();
  int posInface = ( face.indexOf( edge.second ) + faceSize - 1 ) % faceSize;

  return {face.at( posInface ), edge.second};
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
    int indexPt1 = circulator.oppositeVertexClockwise();
    circulator.goBoundaryCounterClockwise();
    int indexPt2 = circulator.oppositeVertexCounterClockwise();

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
  if ( vertexIndex == -1 || ( !mNewFaceCandidate.isEmpty() && vertexIndex == mNewFaceCandidate.last() ) )
    return;

  mNewFaceBand->movePoint( mapVertexXY( vertexIndex ) );
  mNewFaceBand->addPoint( mapVertexXY( vertexIndex ) );
  mNewFaceCandidate.append( vertexIndex );
}

bool QgsMapToolEditMeshFrame::testNewVertexInFaceCanditate( int vertexIndex )
{
  QgsMeshFace face = mNewFaceCandidate.toVector();
  if ( vertexIndex != -1 && !face.empty() && vertexIndex != mNewFaceCandidate.last() )
    face.append( vertexIndex );
  return mCurrentEditor->faceCanBeAdded( face );
}

void QgsMapToolEditMeshFrame::setSelectedVertices( const QList<int> newSelectedVertex, Qt::KeyboardModifiers modifiers )
{
  if ( !( modifiers & Qt::ControlModifier ) && !( modifiers & Qt::ShiftModifier ) )
    clearSelection();

  bool removeVertices = modifiers & Qt::ControlModifier;

  for ( const int vertexIndex : newSelectedVertex )
  {
    if ( mSelectedVertices.contains( vertexIndex ) &&  removeVertices )
    {
      mSelectedVertices.remove( vertexIndex );
      delete mSelectedVerticesMarker.value( vertexIndex );
      mSelectedVerticesMarker.remove( vertexIndex );
    }
    else if ( ! removeVertices && !mSelectedVertices.contains( vertexIndex ) )
    {
      mSelectedVertices.insert( vertexIndex, SelectedVertexData() );
      QgsVertexMarker *marker = new QgsVertexMarker( canvas() );
      marker->setIconType( QgsVertexMarker::ICON_CIRCLE );
      marker->setIconSize( QgsGuiUtils::scaleIconSize( 10 ) );
      marker->setPenWidth( QgsGuiUtils::scaleIconSize( 2 ) );
      marker->setColor( Qt::blue );
      marker->setCenter( mapVertexXY( vertexIndex ) );
      marker->setZValue( 2 );
      mSelectedVerticesMarker[vertexIndex] = marker;
    }
  }

  if ( !mSelectedVertices.isEmpty() )
  {
    double vertexZValue = 0;
    for ( int i : mSelectedVertices.keys() )
      vertexZValue += mapVertex( i ).z();
    vertexZValue /= mSelectedVertices.count();

    mZValueWidget->setDefaultValue( vertexZValue );
  }

  prepareSelection();
}

void QgsMapToolEditMeshFrame::clearSelectedvertex()
{
  mSelectedVertices.clear();
  mSelectedFaces.clear();
  qDeleteAll( mSelectedVerticesMarker );
  mSelectedVerticesMarker.clear();
  mSelectedFacesRubberband->reset();
  prepareSelection();
}

void QgsMapToolEditMeshFrame::removeSelectedVerticesFromMesh( bool fillHole )
{
  QgsMeshEditingError error = mCurrentEditor->removeVertices( mSelectedVertices.keys(), fillHole );
  if ( error != QgsMeshEditingError() )
  {
    QgisApp::instance()->messageBar()->pushWarning(
      tr( "Mesh editing" ),
      tr( "removing the vertex %1 leads to a topological error, operation canceled." ).arg( error.elementIndex ) );
  }
}

void QgsMapToolEditMeshFrame::removeFacesFromMesh()
{
  QgsMeshEditingError error;
  if ( ! mSelectedFaces.isEmpty() )
    error = mCurrentEditor->removeFaces( mSelectedFaces.values() );
  else if ( mCurrentFaceIndex != -1 )
    error = mCurrentEditor->removeFaces( {mCurrentFaceIndex} );
  else
    return;

  if ( error != QgsMeshEditingError() )
  {
    QgisApp::instance()->messageBar()->pushWarning(
      tr( "Mesh editing" ),
      tr( "removing the faces %1 leads to a topological error, operation canceled." ).arg( error.elementIndex ) );
  }
  else
  {
    clearSelectedvertex();
  }
}

void QgsMapToolEditMeshFrame::splitSelectedFaces()
{
  if ( mSplittableFaceCount > 0 )
    mCurrentEditor->splitFaces( mSelectedFaces.values() );
  else if ( mCurrentFaceIndex != -1 && mCurrentEditor->faceCanBeSplit( mCurrentFaceIndex ) )
    mCurrentEditor->splitFaces( {mCurrentFaceIndex} );
}

void QgsMapToolEditMeshFrame::selectInGeometry( const QgsGeometry &geometry, Qt::KeyboardModifiers modifiers )
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
    if ( mSelectPartiallyContainedFace )
    {
      std::unique_ptr<QgsPolygon> faceGeom( new QgsPolygon( new QgsLineString( nativeFaceGeometry( faceIndex ) ) ) );
      if ( engine->intersects( faceGeom.get() ) )
      {
        QSet<int> faceToAdd = qgis::listToSet( face.toList() );
        selectedVertices.unite( faceToAdd );
      }
    }
    else
    {
      for ( const int vertexIndex : face )
      {
        const QgsMeshVertex &vertex = mapVertex( vertexIndex );
        if ( engine->contains( &vertex ) )
          selectedVertices.insert( vertexIndex );
      }
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
  setSelectedVertices( selectedVertices.values(), modifiers );
}

void QgsMapToolEditMeshFrame::applyZValueOnSelectedVertices()
{
  if ( !mZValueWidget )
    return;

  if ( mSelectedVertices.isEmpty() )
    return;

  QList<double> zValues;
  zValues.reserve( mSelectedVertices.count() );
  mOrdinaryZValue = mZValueWidget->zValue();
  for ( int i = 0; i < mSelectedVertices.count(); ++i )
    zValues.append( mOrdinaryZValue );

  mCurrentEditor->changeZValues( mSelectedVertices.keys(), zValues );
}

void QgsMapToolEditMeshFrame::prepareSelection()
{
  mConcernedFaceBySelection.clear();
  QSet<int> borderSelectionVertices;
  QMap<int, SelectedVertexData> movingVertices;

  // search for moving edges and mesh fixed edges
  for ( QMap<int, SelectedVertexData>::iterator it = mSelectedVertices.begin(); it != mSelectedVertices.end(); ++it )
  {
    SelectedVertexData &vertexData = it.value();
    int vertexIndex = it.key();

    vertexData.selectedEdges.clear();
    vertexData.meshFixedEdges.clear();
    QgsMeshVertexCirculator circulator = mCurrentEditor->vertexCirculator( vertexIndex );

    if ( !circulator.isValid() )
      continue;

    circulator.goBoundaryClockwise();
    int firstface = circulator.currentFaceIndex();
    do
    {
      int oppositeVertex = circulator.oppositeVertexClockwise();
      if ( mSelectedVertices.contains( oppositeVertex ) )
        vertexData.selectedEdges.append( {circulator.currentFaceIndex(), oppositeVertex} );
      else
      {
        vertexData.meshFixedEdges.append( {circulator.currentFaceIndex(), oppositeVertex} );
        borderSelectionVertices.insert( vertexIndex );
      }

      mConcernedFaceBySelection.insert( circulator.currentFaceIndex() );
    }
    while ( circulator.turnCounterClockwise() != firstface && circulator.currentFaceIndex() != -1 );

    if ( circulator.currentFaceIndex() == -1 )
    {
      circulator.turnClockwise();
      int oppositeVertex = circulator.oppositeVertexCounterClockwise();
      if ( mSelectedVertices.contains( oppositeVertex ) )
        vertexData.selectedEdges.append( {-1, oppositeVertex} );
      else
        vertexData.meshFixedEdges.append( {-1, oppositeVertex} );

      borderSelectionVertices.insert( vertexIndex );
    }
  }

  // remove faces that have at least one vertex not selected
  mSelectedFaces = mConcernedFaceBySelection;
  for ( const int faceIndex : std::as_const( mConcernedFaceBySelection ) )
  {
    const QgsMeshFace &face = nativeFace( faceIndex );
    for ( const int vi : std::as_const( face ) )
      if ( !mSelectedVertices.contains( vi ) )
      {
        mSelectedFaces.remove( faceIndex );
        continue;
      }
  }

  if ( !mSelectedFaces.isEmpty() )
  {
    QList<int> faceList = mSelectedFaces.values();
    QgsGeometry faceGeometrie( new QgsPolygon( new QgsLineString( nativeFaceGeometry( faceList.at( 0 ) ) ) ) );
    if ( mSelectedFaces.count() == 1 )
    {
      mSelectedFacesRubberband->setToGeometry( faceGeometrie );
    }
    else
    {
      std::unique_ptr<QgsGeometryEngine> geomEngine( QgsGeometry::createGeometryEngine( faceGeometrie.constGet() ) );
      geomEngine->prepareGeometry();

      QVector<QgsGeometry> otherFaces( mSelectedFaces.count() );
      for ( int i = 0; i < mSelectedFaces.count(); ++i )
        otherFaces[i] = QgsGeometry( new QgsPolygon( new QgsLineString( nativeFaceGeometry( faceList.at( i ) ) ) ) );
      QString error;
      QgsGeometry allFaces( geomEngine->combine( otherFaces, &error ) );
      mSelectedFacesRubberband->setToGeometry( allFaces );
      QColor fillColor = canvas()->mapSettings().selectionColor();

      if ( fillColor.alpha() > 100 ) //set alpha to 150 if the transparency is no enough to see the mesh
        fillColor.setAlpha( 100 );

      mSelectedFacesRubberband->setFillColor( fillColor );
    }
  }
  else
    mSelectedFacesRubberband->reset( QgsWkbTypes::PolygonGeometry );

  if ( mSelectedVertices.count() == 1 )
  {
    mActionRemoveVerticesFillingHole->setText( tr( "Remove selected vertex and fill hole" ) );
    mActionRemoveVerticesWithoutFillingHole->setText( tr( "Remove selected vertex without filling hole" ) );
  }
  else if ( mSelectedVertices.count() > 1 )
  {
    mActionRemoveVerticesFillingHole->setText( tr( "Remove selected vertices and fill hole(s)" ) );
    mActionRemoveVerticesWithoutFillingHole->setText( tr( "Remove selected vertices without filling hole(s)" ) );
  }

  if ( mSelectedFaces.count() == 1 )
  {
    mActionRemoveFaces->setText( tr( "Remove selected face" ) );
    mActionFacesRefinement->setText( tr( "Refine selected face" ) );
  }
  else if ( mSelectedFaces.count() > 1 )
  {
    mActionRemoveFaces->setText( tr( "Remove %1 selected faces" ).arg( mSelectedFaces.count() ) );
    mActionFacesRefinement->setText( tr( "Refine %1 selected faces" ).arg( mSelectedFaces.count() ) );
  }
  else
  {
    mActionRemoveFaces->setText( tr( "Remove current face" ) );
    mActionFacesRefinement->setText( tr( "Refine current face" ) );
  }

  mSplittableFaceCount = 0;
  for ( const int faceIndex : std::as_const( mSelectedFaces ) )
  {
    if ( mCurrentEditor->faceCanBeSplit( faceIndex ) )
      mSplittableFaceCount++;
  }

  if ( mSplittableFaceCount == 1 )
    mActionSplitFaces->setText( tr( "Split selected face" ) );
  else if ( mSplittableFaceCount > 1 )
    mActionSplitFaces->setText( tr( "Split %1 selected faces" ).arg( mSplittableFaceCount ) );
  else
    mActionSplitFaces->setText( tr( "Split current face" ) );
}

void QgsMapToolEditMeshFrame::updateSelectecVerticesMarker()
{
  qDeleteAll( mSelectedVerticesMarker );
  mSelectedVerticesMarker.clear();
  for ( const int vertexIndex : mSelectedVertices.keys() )
  {
    mSelectedVertices.insert( vertexIndex, SelectedVertexData() );
    QgsVertexMarker *marker = new QgsVertexMarker( canvas() );
    marker->setIconType( QgsVertexMarker::ICON_CIRCLE );
    marker->setIconSize( QgsGuiUtils::scaleIconSize( 10 ) );
    marker->setPenWidth( QgsGuiUtils::scaleIconSize( 2 ) );
    marker->setColor( Qt::blue );
    marker->setCenter( mapVertexXY( vertexIndex ) );
    marker->setZValue( 2 );
    mSelectedVerticesMarker[vertexIndex] = marker;
  }
}

void QgsMapToolEditMeshFrame::highlightCurrentHoveredFace( const QgsPointXY &mapPoint )
{
  int faceIndex = -1;
  if ( !mCurrentLayer.isNull() && mCurrentLayer->triangularMesh() )
    faceIndex = mCurrentLayer->triangularMesh()->nativeFaceIndexForPoint( mapPoint );

  if ( mSelectFaceMarker->isVisible() )
  {
    double tol = QgsTolerance::vertexSearchRadius( canvas()->mapSettings() );
    if ( mapPoint.distance( mSelectFaceMarker->center() ) < tol )
    {
      mSelectFaceMarker->setColor( Qt::red );
      mSelectFaceMarker->setFillColor( Qt::red );
    }
    else
    {
      mSelectFaceMarker->setColor( Qt::gray );
      mSelectFaceMarker->setFillColor( Qt::gray );
    }
  }

  mCurrentFaceIndex = faceIndex;

  QgsPointSequence faceGeometry = nativeFaceGeometry( faceIndex );
  mFaceRubberBand->reset( QgsWkbTypes::PolygonGeometry );
  mFaceVerticesBand->reset( QgsWkbTypes::PointGeometry );
  for ( const QgsPoint &pt : faceGeometry )
  {
    mFaceRubberBand->addPoint( pt );
    mFaceVerticesBand->addPoint( pt );
  }

  if ( faceIndex != -1 && faceCanBeInteractive( faceIndex ) )
  {
    mSelectFaceMarker->setCenter( mCurrentLayer->triangularMesh()->faceCentroids().at( faceIndex ) );
    mSelectFaceMarker->setVisible( true );
  }
  else
    mSelectFaceMarker->setVisible( false );

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
      mVertexBand->addPoint( mapVertexXY( closeVert ) );
      if ( mCurrentState == Digitizing )
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

void QgsMapToolEditMeshFrame::highlightCloseEdge( const QgsPointXY &mapPoint )
{
  if ( mCurrentLayer.isNull() )
    return;

  double tolerance = QgsTolerance::vertexSearchRadius( mCanvas->mapSettings() );

  if ( mFlipEdgeMarker->isVisible() )
  {
    if ( mapPoint.distance( mFlipEdgeMarker->center() ) < tolerance )
      mFlipEdgeMarker->setColor( Qt::red );
    else
      mFlipEdgeMarker->setColor( Qt::gray );
  }

  if ( mSelectEdgeMarker->isVisible() )
  {
    if ( mapPoint.distance( mSelectEdgeMarker->center() ) < tolerance )
    {
      mSelectEdgeMarker->setColor( Qt::red );
      mSelectEdgeMarker->setFillColor( Qt::red );
    }
    else
    {
      mSelectEdgeMarker->setColor( Qt::gray );
      mSelectEdgeMarker->setFillColor( Qt::gray );
    }
  }

  if ( mMergeFaceMarker->isVisible() )
  {
    if ( mapPoint.distance( mMergeFaceMarker->center() ) < tolerance )
      mMergeFaceMarker->setColor( Qt::red );
    else
      mMergeFaceMarker->setColor( Qt::gray );
  }

  mCurrentEdge = {-1, -1};

  QList<int> candidateFaceIndexes;

  if ( mCurrentFaceIndex != -1 )
  {
    candidateFaceIndexes.append( mCurrentFaceIndex );
  }
  else
  {
    const QgsRectangle searchRect( mapPoint.x() - tolerance, mapPoint.y() - tolerance, mapPoint.x() + tolerance, mapPoint.y() + tolerance );
    candidateFaceIndexes = mCurrentLayer->triangularMesh()->nativeFaceIndexForRectangle( searchRect );
  }

  double minimumDistance = std::numeric_limits<double>::max();
  for ( const int faceIndex : std::as_const( candidateFaceIndexes ) )
  {
    const QgsMeshFace &face = nativeFace( faceIndex );
    int faceSize = face.count();
    for ( int i = 0; i < faceSize; ++i )
    {
      int iv1 = face.at( i );
      int iv2 = face.at( ( i + 1 ) % faceSize );

      QgsPointXY pt1 = mapVertexXY( iv1 );
      QgsPointXY pt2 = mapVertexXY( iv2 );

      QgsPointXY pointOneEdge;
      double distance = sqrt( mapPoint.sqrDistToSegment( pt1.x(), pt1.y(), pt2.x(), pt2.y(), pointOneEdge ) );
      if ( distance < tolerance && distance < minimumDistance && edgeCanBeInteractive( iv1, iv2 ) )
      {
        mCurrentEdge = {faceIndex, iv2};
        minimumDistance = distance;
      }
    }
  }

  mEdgeBand->reset();
  mFlipEdgeMarker->setVisible( false );
  mMergeFaceMarker->setVisible( false );
  mSelectEdgeMarker->setVisible( false );
  if ( mCurrentEdge.first != -1 && mCurrentEdge.second != -1 &&  mCurrentState == Digitizing )
  {
    const QVector<QgsPointXY> &edgeGeom = edgeGeometry( mCurrentEdge );
    mEdgeBand->addPoint( edgeGeom.at( 0 ) );
    mEdgeBand->addPoint( edgeGeom.at( 1 ) );

    if ( mCurrentFaceIndex == -1 )
    {
      mFaceVerticesBand->reset( QgsWkbTypes::PointGeometry );
      mFaceVerticesBand->addPoint( edgeGeom.at( 0 ) );
      mFaceVerticesBand->addPoint( edgeGeom.at( 1 ) );
    }

    const QVector<int> edgeVert = edgeVertices( mCurrentEdge );
    QgsPointXY basePoint;
    QgsVector intervalOfMarkers;
    if ( std::fabs( edgeGeom.at( 0 ).x() - edgeGeom.at( 1 ).x() ) > std::fabs( edgeGeom.at( 0 ).y() - edgeGeom.at( 1 ).y() ) )
    {
      // edge are more horizontal than vertical, take the vertex on the left side
      if ( edgeGeom.at( 0 ).x() < edgeGeom.at( 1 ).x() )
      {
        basePoint = edgeGeom.at( 0 );
        intervalOfMarkers = ( edgeGeom.at( 1 ) - edgeGeom.at( 0 ) ) / 4;
      }
      else
      {
        basePoint = edgeGeom.at( 1 );
        intervalOfMarkers = ( edgeGeom.at( 0 ) - edgeGeom.at( 1 ) ) / 4;
      }
    }
    else
    {
      // edge are more vertical than horizontal, take the vertex on the bottom
      if ( edgeGeom.at( 0 ).y() < edgeGeom.at( 1 ).y() )
      {
        basePoint = edgeGeom.at( 0 );
        intervalOfMarkers = ( edgeGeom.at( 1 ) - edgeGeom.at( 0 ) ) / 4;
      }
      else
      {
        basePoint = edgeGeom.at( 1 );
        intervalOfMarkers = ( edgeGeom.at( 0 ) - edgeGeom.at( 1 ) ) / 4;
      }
    }

    if ( mCurrentEditor->edgeCanBeFlipped( edgeVert.at( 0 ), edgeVert.at( 1 ) ) )
    {
      mFlipEdgeMarker->setVisible( true );
      mFlipEdgeMarker->setCenter( basePoint + intervalOfMarkers );
    }
    else
      mFlipEdgeMarker->setVisible( false );

    mSelectEdgeMarker->setVisible( true );
    mSelectEdgeMarker->setCenter( basePoint + intervalOfMarkers * 2 );

    if ( mCurrentEditor->canBeMerged( edgeVert.at( 0 ), edgeVert.at( 1 ) ) )
    {
      mMergeFaceMarker->setVisible( true );
      mMergeFaceMarker->setCenter( basePoint + intervalOfMarkers * 3 );
    }
    else
      mMergeFaceMarker->setVisible( false );
  }
}

bool QgsMapToolEditMeshFrame::edgeCanBeInteractive( int vertexIndex1, int vertexIndex2 ) const
{
  // If the edge is less than 90px width, the interactive marker will not be displayed to avoid too close marker and
  // avoit the user to click on a marker if he doesn't want
  double mapUnitPerPixel = mCanvas->mapSettings().mapUnitsPerPixel();
  return mapVertexXY( vertexIndex1 ).distance( mapVertexXY( vertexIndex2 ) ) / mapUnitPerPixel > 90;
}

bool QgsMapToolEditMeshFrame::faceCanBeInteractive( int faceIndex ) const
{
  // If both side of the face boundng box is less than 60px width, the interactive marker will not be displayed to avoid too close marker and
  // avoit the user to click on a marker if he doesn't want

  double mapUnitPerPixel = mCanvas->mapSettings().mapUnitsPerPixel();
  QgsGeometry faceGeom( new QgsLineString( nativeFaceGeometry( faceIndex ) ) );
  QgsRectangle bbox = faceGeom.boundingBox();

  return bbox.width() / mapUnitPerPixel > 60 || bbox.height() / mapUnitPerPixel > 60;
}

void QgsMapToolEditMeshFrame::createZValueWidget()
{
  if ( !mCanvas )
  {
    return;
  }

  deleteZvalueWidget();

  mZValueWidget = new QgsZValueWidget( tr( "Vertex Z value:" ) );
  QgisApp::instance()->addUserInputWidget( mZValueWidget );
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

void QgsMapToolEditMeshFrame::clearSelection()
{
  mSelectedVertices.clear();
  mSelectedFaces.clear();
  mSelectionBand->reset( QgsWkbTypes::PolygonGeometry );
  mSelectedFacesRubberband->reset( QgsWkbTypes::PolygonGeometry );
  qDeleteAll( mSelectedVerticesMarker );
  mSelectedVerticesMarker.clear();
  prepareSelection();
}

void QgsMapToolEditMeshFrame::clearCanvasHelpers()
{
  mCurrentFaceIndex = -1;
  mCurrentVertexIndex = -1;
  mFaceRubberBand->reset();
  mFaceVerticesBand->reset();
  mVertexBand->reset();

  mSnapIndicator->setMatch( QgsPointLocator::Match() );
  mSelectFaceMarker->setVisible( false );
  clearEdgeHelpers();
}

void QgsMapToolEditMeshFrame::clearEdgeHelpers()
{
  mCurrentEdge = {-1, -1};
  mEdgeBand->reset();
  mSelectEdgeMarker->setVisible( false );
  mFlipEdgeMarker->setVisible( false );
  mMergeFaceMarker->setVisible( false );
}

void QgsMapToolEditMeshFrame::addVertex(
  const QgsPointXY &mapPoint,
  const QgsPointLocator::Match &mapPointMatch,
  Qt::KeyboardModifiers modifiers )
{
  double zValue = mZValueWidget ? mZValueWidget->zValue() : std::numeric_limits<double>::quiet_NaN();

  if ( mapPointMatch.isValid() )
  {
    QgsPoint layerPoint = mapPointMatch.interpolatedPoint();
    zValue = layerPoint.z();
  }
  else if ( mIsSelectedZValue )
    zValue = mSelectedZValue;
  else if ( mCurrentFaceIndex != -1 ) //we are on a face -->interpolate the z value
  {
    const QgsTriangularMesh &triangularMesh = *mCurrentLayer->triangularMesh();
    int triangleFaceIndex = triangularMesh.faceIndexForPoint_v2( mapPoint );
    const QgsMeshFace &triangleFace = triangularMesh.triangles().at( triangleFaceIndex );
    const QgsMeshVertex &v1 = triangularMesh.vertices().at( triangleFace.at( 0 ) );
    const QgsMeshVertex &v2 = triangularMesh.vertices().at( triangleFace.at( 1 ) );
    const QgsMeshVertex &v3 = triangularMesh.vertices().at( triangleFace.at( 2 ) );
    zValue = QgsMeshLayerUtils::interpolateFromVerticesData( v1, v2, v3, v1.z(), v2.z(), v3.z(), mapPoint );
  }

  if ( modifiers & Qt::ControlModifier )
    mOrdinaryZValue = mSelectedZValue;

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
    marker->setCenter( mapVertexXY( freeVertexIndex ) );
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

  if ( mCurrentEdge.first != -1 && mCurrentEdge.second  != -1 )
  {
    const QVector<int> &edge = edgeVertices( mCurrentEdge );

    for ( const int vertexIndex : edge )
    {
      const QgsPointXY &meshVertex = mapVertexXY( vertexIndex );
      if ( meshVertex.distance( mapPoint ) < tolerance )
        return vertexIndex;
    }
  }

  if ( mCurrentFaceIndex >= 0 )
  {
    const QgsMeshFace &face = mCurrentLayer->nativeMesh()->face( mCurrentFaceIndex );
    for ( const int vertexIndex : face )
    {
      const QgsPointXY &meshVertex = mapVertexXY( vertexIndex );
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
    const QgsPointXY &meshVertex = mapVertexXY( vertexIndex );
    if ( meshVertex.distance( mapPoint ) < tolerance )
      return vertexIndex;
  }

  return -1;
}
