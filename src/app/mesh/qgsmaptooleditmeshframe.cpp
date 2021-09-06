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
#include "qgsmeshtransformcoordinatesdockwidget.h"
#include "qgsmeshforcebypolylines.h"
#include "qgsvectorlayer.h"
#include "qgsunitselectionwidget.h"


//
// QgsZValueWidget
//

#include "qgsexpressionbuilderwidget.h"
#include "qgsmeshselectbyexpressiondialog.h"
#include "qgsexpressioncontext.h"
#include "qgsexpressioncontextutils.h"
#include "qgsexpressionutils.h"


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

QgsMeshEditForceByLineAction::QgsMeshEditForceByLineAction( QObject *parent )
  : QWidgetAction( parent )
{
  QGridLayout *gLayout = new QGridLayout();
  gLayout->setContentsMargins( 3, 2, 3, 2 );

  QgsSettings settings;

  mCheckBoxNewVertex = new QCheckBox( tr( "Add new vertex on intersecting edges" ) );

  bool newVertex = settings.value( QStringLiteral( "UI/Mesh/ForceByLineNewVertex" ) ).toBool();
  mCheckBoxNewVertex->setChecked( newVertex );

  QLabel *labelInterpolation = new QLabel( tr( "Interpolate Z value from" ) );
  mComboInterpolateFrom = new QComboBox();
  mComboInterpolateFrom->addItem( tr( "Mesh" ), Mesh );
  mComboInterpolateFrom->addItem( tr( "Forcing line" ), Lines );

  int interpolateFromValue = settings.enumValue( QStringLiteral( "UI/Mesh/ForceByLineInterpolateFrom" ), Mesh );
  mComboInterpolateFrom->setCurrentIndex( interpolateFromValue );

  QLabel *labelTolerance = new QLabel( tr( "Tolerance" ) );
  mToleranceSpinBox = new QgsDoubleSpinBox();

  bool ok;
  double toleranceValue = settings.value( QStringLiteral( "UI/Mesh/ForceByLineToleranceValue" ), QgsUnitTypes::RenderMapUnits ).toDouble( &ok );
  if ( !ok )
    toleranceValue = 1.0;
  mToleranceSpinBox->setValue( toleranceValue );
  mToleranceSpinBox->setKeyboardTracking( false );
  mToleranceSpinBox->setWrapping( false );
  mToleranceSpinBox->setSingleStep( 0.1 );
  mToleranceSpinBox->setClearValue( 1.0 );

  mUnitSelecionWidget = new QgsUnitSelectionWidget();
  mUnitSelecionWidget->setUnits( QgsUnitTypes::RenderUnitList() <<
                                 QgsUnitTypes::RenderMetersInMapUnits <<
                                 QgsUnitTypes::RenderMapUnits );

  QgsUnitTypes::RenderUnit toleranceUnit = settings.enumValue( QStringLiteral( "UI/Mesh/ForceByLineToleranceValue" ), QgsUnitTypes::RenderMapUnits );
  mUnitSelecionWidget->setUnit( toleranceUnit );

  gLayout->addWidget( mCheckBoxNewVertex, 1, 0, 1, 4 );
  gLayout->addWidget( labelInterpolation, 2, 0, 1, 3 );
  gLayout->addWidget( mComboInterpolateFrom, 2, 3, 1, 1 );
  gLayout->addWidget( labelTolerance, 3, 0, 1, 2 );
  gLayout->addWidget( mToleranceSpinBox, 3, 2, 1, 1 );
  gLayout->addWidget( mUnitSelecionWidget, 3, 3, 1, 1 );

  QWidget *w = new QWidget();
  w->setLayout( gLayout );
  setDefaultWidget( w );

  connect( mCheckBoxNewVertex, &QCheckBox::toggled, this, &QgsMeshEditForceByLineAction::updateSettings );
  connect( mComboInterpolateFrom, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsMeshEditForceByLineAction::updateSettings );
  connect( mToleranceSpinBox, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, &QgsMeshEditForceByLineAction::updateSettings );
  connect( mUnitSelecionWidget, &QgsUnitSelectionWidget::changed, this, &QgsMeshEditForceByLineAction::updateSettings );
}

void QgsMeshEditForceByLineAction::setMapCanvas( QgsMapCanvas *canvas )
{
  mUnitSelecionWidget->setMapCanvas( canvas );
}

QgsMeshEditForceByLineAction::IntepolationMode QgsMeshEditForceByLineAction::interpolationMode() const
{
  return static_cast<IntepolationMode>( mComboInterpolateFrom->currentData().toInt() );
}

bool QgsMeshEditForceByLineAction::newVertexOnIntersectingEdge() const
{
  return mCheckBoxNewVertex->isChecked();
}

double QgsMeshEditForceByLineAction::toleranceValue() const
{
  return mToleranceSpinBox->value();
}

QgsUnitTypes::RenderUnit QgsMeshEditForceByLineAction::toleranceUnit() const
{
  return mUnitSelecionWidget->unit();
}

void QgsMeshEditForceByLineAction::updateSettings()
{
  QgsSettings settings;

  settings.setValue( QStringLiteral( "UI/Mesh/ForceByLineNewVertex" ), mCheckBoxNewVertex->isChecked() );
  settings.setEnumValue( QStringLiteral( "UI/Mesh/ForceByLineInterpolateFrom" ),
                         static_cast<IntepolationMode>( mComboInterpolateFrom->currentData().toInt() ) );
  settings.setValue( QStringLiteral( "UI/Mesh/ForceByLineToleranceValue" ), mToleranceSpinBox->value() );
  settings.setEnumValue( QStringLiteral( "UI/Mesh/ForceByLineToleranceValue" ), mUnitSelecionWidget->unit() );
}

//
// QgsMapToolEditMeshFrame
//

QgsMapToolEditMeshFrame::QgsMapToolEditMeshFrame( QgsMapCanvas *canvas )
  : QgsMapToolAdvancedDigitizing( canvas, QgisApp::instance()->cadDockWidget() )
  , mSnapIndicator( new QgsSnapIndicator( canvas ) )
{
  mActionDigitizing = new QAction( QgsApplication::getThemePixmap( QStringLiteral( "/mActionMeshDigitizing.svg" ) ), tr( "Digitize mesh elements" ), this );
  mActionDigitizing->setCheckable( true );
  mActionSelectByPolygon = new QAction( QgsApplication::getThemePixmap( QStringLiteral( "/mActionMeshSelectPolygon.svg" ) ), tr( "Select mesh element by polygon" ), this );
  mActionSelectByPolygon = new QAction( QgsApplication::getThemePixmap( QStringLiteral( "/mActionMeshSelectPolygon.svg" ) ), tr( "Select mesh elements by polygon" ), this );
  mActionSelectByPolygon->setCheckable( true );
  mActionSelectByExpression = new QAction( QgsApplication::getThemePixmap( QStringLiteral( "/mActionMeshSelectExpression.svg" ) ), tr( "Select mesh elements by expression" ), this );

  mActionTransformCoordinates = new QAction( QgsApplication::getThemePixmap( QStringLiteral( "/mActionMeshTransformByExpression.svg" ) ), tr( "Transform vertices coordinates" ), this );
  mActionTransformCoordinates->setCheckable( true );

  mActionForceByVectorLayerGeometries = new QAction( QgsApplication::getThemePixmap( QStringLiteral( "/mActionMeshEditForceByVectorLines.svg" ) ), tr( "Force by selected geometries" ), this );
  mActionForceByVectorLayerGeometries->setEnabled( areGeometriesSelectedInVectorLayer() );

  mWidgetActionForceByLine = new QgsMeshEditForceByLineAction( this );
  mWidgetActionForceByLine->setMapCanvas( canvas );

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

  connect( mActionSelectByPolygon, &QAction::toggled, this, [this]( bool checked )
  {
    if ( checked )
      activateWithState( SelectingByPolygon );
    else
      mSelectionBand->reset( QgsWkbTypes::PolygonGeometry );
  } );

  connect( mActionDelaunayTriangulation, &QAction::triggered, this, [this]
  {
    if ( mCurrentEditor && mSelectedVertices.count() >= 3 )
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

  connect( mActionTransformCoordinates, &QAction::triggered, this, &QgsMapToolEditMeshFrame::triggerTransformCoordinatesDockWidget );
  connect( mActionSelectByExpression, &QAction::triggered, this, &QgsMapToolEditMeshFrame::showSelectByExpressionDialog );

  connect( canvas, &QgsMapCanvas::selectionChanged, this, [this]
  {
    mActionForceByVectorLayerGeometries->setEnabled( areGeometriesSelectedInVectorLayer() &&( mCurrentEditor != nullptr ) );
  } );

  connect( mActionForceByVectorLayerGeometries, &QAction::triggered, this, &QgsMapToolEditMeshFrame::forceBySelectedLayerPolyline );

  setAutoSnapEnabled( true );
}

void QgsMapToolEditMeshFrame::activateWithState( State state )
{
  if ( mCanvas->mapTool() != this )
  {
    mCanvas->setMapTool( this );
    mCanvas->setFocus();
    onEditingStarted();
  }
  mCurrentState = state;
}

void QgsMapToolEditMeshFrame::backToDigitizing()
{
  activateWithState( Digitizing );
  mActionDigitizing->setChecked( true );
}

QgsMapToolEditMeshFrame::~QgsMapToolEditMeshFrame()
{
  deleteZValueWidget();
}

void QgsMapToolEditMeshFrame::setActionsEnable( bool enable )
{
  QList<QAction *> actions;
  actions
      << mActionDigitizing
      << mActionSelectByPolygon
      << mActionSelectByExpression
      << mActionTransformCoordinates
      << mActionForceByVectorLayerGeometries;

  for ( QAction *action : std::as_const( actions ) )
    action->setEnabled( enable );

  mActionForceByVectorLayerGeometries->setEnabled( enable && areGeometriesSelectedInVectorLayer() );
}


QList<QAction *> QgsMapToolEditMeshFrame::mapToolActions()
{
  return  QList<QAction *>()
          << mActionDigitizing
          << mActionSelectByPolygon;
}

QAction *QgsMapToolEditMeshFrame::digitizeAction() const
{
  return  mActionDigitizing;
}

QList<QAction *> QgsMapToolEditMeshFrame::selectActions() const
{
  return  QList<QAction *>()
          << mActionSelectByPolygon;
}

QAction *QgsMapToolEditMeshFrame::defaultSelectActions() const
{
  return mActionSelectByPolygon;
}

QAction *QgsMapToolEditMeshFrame::transformAction() const
{
  return mActionTransformCoordinates;
}

QList<QAction *> QgsMapToolEditMeshFrame::forceByLinesActions() const
{
  return  QList<QAction *>()
          << mActionForceByVectorLayerGeometries;
}

QAction *QgsMapToolEditMeshFrame::defaultForceAction() const
{
  return mActionForceByVectorLayerGeometries;
}

QWidgetAction *QgsMapToolEditMeshFrame::forceByLineWidgetActionSettings() const
{
  return mWidgetActionForceByLine;
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
  mSelectionBand->setFillColor( QColor( 254, 178, 76, 63 ) );
  mSelectionBand->setStrokeColor( QColor( 254, 58, 29, 100 ) );
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
  deleteZValueWidget();
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

  deleteZValueWidget();
}

void QgsMapToolEditMeshFrame::activate()
{
  QgsMapToolAdvancedDigitizing::activate();
  createZValueWidget();
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
    case SelectingByPolygon:
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
    case SelectingByPolygon:
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
    if ( mCurrentState != SelectingByPolygon )
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
        break;
      case AddingNewFace:
      case Selecting:
      case MovingVertex:
      case SelectingByPolygon:
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
       mCurrentState != AddingNewFace &&
       mCurrentState != SelectingByPolygon )
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
      mSelectionBand->setToCanvasRectangle( rect );
    }
    break;
    case MovingVertex:
    {
      const QgsVector &translation = mapPoint - mStartMovingPoint;
      mMovingEdgesRubberband->reset( QgsWkbTypes::LineGeometry );
      mMovingFacesRubberband->reset( QgsWkbTypes::PolygonGeometry );
      QgsGeometry movingFacesGeometry = mSelectedFacesRubberband->asGeometry();
      movingFacesGeometry.translate( translation.x(), translation.y() );
      mMovingFacesRubberband->setToGeometry( movingFacesGeometry );

      QSet<int> borderMovingFace;

      for ( QMap<int, SelectedVertexData>::const_iterator it = mSelectedVertices.constBegin(); it != mSelectedVertices.constEnd(); ++it )
      {
        const QgsPointXY &point1 = mapVertexXY( it.key() ) + translation;
        const SelectedVertexData &vertexData = it.value();
        for ( int i = 0; i < vertexData.meshFixedEdges.count(); ++i )
        {
          const QgsPointXY point2 = mapVertexXY( vertexData.meshFixedEdges.at( i ).second );
          QgsGeometry edge( new QgsLineString( {point1, point2} ) );
          mMovingEdgesRubberband->addGeometry( edge );
          int associateFace = vertexData.meshFixedEdges.at( i ).first;
          if ( associateFace != -1 )
            borderMovingFace.insert( associateFace );
        }

        for ( int i = 0; i < vertexData.borderEdges.count(); ++i )
        {
          const QgsPointXY point2 = mapVertexXY( vertexData.borderEdges.at( i ).second ) + translation;
          const QgsGeometry edge( new QgsLineString( {point1, point2} ) );
          mMovingEdgesRubberband->addGeometry( edge );
        }
      }

      const QgsMeshVertex &mapPointInNativeCoordinate =
        mCurrentLayer->triangularMesh()->triangularToNativeCoordinates( QgsMeshVertex( mapPoint.x(), mapPoint.y() ) );
      const QgsMeshVertex &startingPointInNativeCoordinate =
        mCurrentLayer->triangularMesh()->triangularToNativeCoordinates( QgsMeshVertex( mStartMovingPoint.x(), mStartMovingPoint.y() ) );
      const QgsVector &translationInLayerCoordinate = mapPointInNativeCoordinate - startingPointInNativeCoordinate;

      auto transformFunction = [translationInLayerCoordinate, this ]( int vi )-> const QgsMeshVertex
      {
        if ( mSelectedVertices.contains( vi ) )
          return mCurrentLayer->nativeMesh()->vertex( vi ) + translationInLayerCoordinate;
        else
          return mCurrentLayer->nativeMesh()->vertex( vi );
      };

      // we test only the faces that are deformed on the border, moving and not deformed faces are tested later
      mIsMovingAllowed = mCurrentEditor->canBeTransformed( qgis::setToList( borderMovingFace ), transformFunction );

      if ( mIsMovingAllowed )
      {
        //to finish test if the polygons formed by the moving faces contains something else
        const QList<int> &faceIndexesIntersect = mCurrentLayer->triangularMesh()->nativeFaceIndexForRectangle( movingFacesGeometry.boundingBox() );
        for ( const int faceIndex : faceIndexesIntersect )
        {
          if ( mConcernedFaceBySelection.contains( faceIndex ) )
            continue;
          const QgsGeometry otherFaceGeom( new QgsPolygon( new QgsLineString( nativeFaceGeometry( faceIndex ) ) ) );
          mIsMovingAllowed &= !movingFacesGeometry.intersects( otherFaceGeom );
          if ( !mIsMovingAllowed )
            break;
        }

        if ( mIsMovingAllowed ) //last check, the free vertices...
        {
          const QList<int> &freeVerticesIndexes = mCurrentEditor->freeVerticesIndexes();
          for ( const int vertexIndex : freeVerticesIndexes )
          {
            const QgsPointXY &pointInMap = mapVertexXY( vertexIndex );
            mIsMovingAllowed &= !movingFacesGeometry.contains( &pointInMap );
            if ( !mIsMovingAllowed )
              break;
          }
        }
      }

      setMovingRubberBandValidity( mIsMovingAllowed );
    }
    break;
    case SelectingByPolygon:
      mSelectionBand->movePoint( mapPoint );
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
      QgsGeometry selectionGeom = mSelectionBand->asGeometry();
      selectInGeometry( selectionGeom, e->modifiers() );
      mSelectionBand->reset( QgsWkbTypes::PolygonGeometry );
      mCurrentState = Digitizing;
    }
    break;
    case MovingVertex:
      if ( mIsMovingAllowed )
      {
        const QList<int> verticesIndexes = mSelectedVertices.keys();
        QList<QgsPointXY> newPosition;
        newPosition.reserve( verticesIndexes.count() );

        const QgsMeshVertex &mapPointInNativeCoordinate =
          mCurrentLayer->triangularMesh()->triangularToNativeCoordinates( QgsMeshVertex( mapPoint.x(), mapPoint.y() ) );
        const QgsMeshVertex &startingPointInNativeCoordinate =
          mCurrentLayer->triangularMesh()->triangularToNativeCoordinates( QgsMeshVertex( mStartMovingPoint.x(), mStartMovingPoint.y() ) );
        const QgsVector &translationInLayerCoordinate = mapPointInNativeCoordinate - startingPointInNativeCoordinate;

        const QgsMesh &mesh = *mCurrentLayer->nativeMesh();
        for ( int i = 0; i < verticesIndexes.count(); ++i )
          newPosition.append( QgsPointXY( mesh.vertex( verticesIndexes.at( i ) ) ) + translationInLayerCoordinate );

        mKeepSelectionOnEdit = true;
        mCurrentEditor->changeXYValues( mSelectedVertices.keys(), newPosition );
      }
      updateSelectecVerticesMarker();
      prepareSelection();
      clearCanvasHelpers();
      mMovingEdgesRubberband->reset();
      mMovingFacesRubberband->reset();
      mCurrentState = Digitizing;
      break;
    case SelectingByPolygon:
      if ( e->button() == Qt::LeftButton )
      {
        mSelectionBand->movePoint( mapPoint );
        mSelectionBand->addPoint( mapPoint );
      }
      else if ( e->button() == Qt::RightButton )
      {
        QgsGeometry selectionGeom = mSelectionBand->asGeometry();
        selectInGeometry( selectionGeom, e->modifiers() );
        mSelectionBand->reset( QgsWkbTypes::PolygonGeometry );
      }
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
  Qgis::SelectBehavior behavior;
  if ( modifiers & Qt::ShiftModifier )
    behavior = Qgis::SelectBehavior::AddToSelection;
  else if ( modifiers & Qt::ControlModifier )
    behavior = Qgis::SelectBehavior::RemoveFromSelection;
  else
    behavior = Qgis::SelectBehavior::SetSelection;

  if ( mSelectFaceMarker->isVisible() &&
       mapPoint.distance( mSelectFaceMarker->center() ) < tolerance
       && mCurrentFaceIndex >= 0 )
  {
    setSelectedVertices( nativeFace( mCurrentFaceIndex ).toList(), behavior );
  }
  else if ( mCurrentVertexIndex != -1 )
  {
    setSelectedVertices( QList<int>() << mCurrentVertexIndex, behavior );
  }
  else if ( mSelectEdgeMarker->isVisible() &&
            mapPoint.distance( mSelectEdgeMarker->center() ) < tolerance &&
            mCurrentEdge.first != -1 && mCurrentEdge.second != -1 )
  {
    QVector<int> edgeVert = edgeVertices( mCurrentEdge );
    setSelectedVertices( edgeVert.toList(), behavior );
  }
  else
    setSelectedVertices( QList<int>(),  behavior );
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
    case SelectingByPolygon:
      if ( e->key() == Qt::Key_Escape )
      {
        mSelectionBand->reset( QgsWkbTypes::PolygonGeometry );
        backToDigitizing();
        consumned = true;
      }

      if ( e->key() == Qt::Key_Backspace )
      {
        mSelectionBand->removePoint( -2, true );
        consumned = true;
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
    case SelectingByPolygon:
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

  if ( mIsInitialized )
    clearSelection(); //TODO later: implement a mechanism to retrieve selection if the layer is again selected

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

  if ( mCurrentEditor )
  {
    if ( !mZValueWidget )
      createZValueWidget();
    updateFreeVertices();
  }
  else
    deactivate();

  emit selectionChange( mCurrentLayer, mSelectedVertices.keys() );
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


void QgsMapToolEditMeshFrame::addNewSelectedVertex( int vertexIndex )
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

void QgsMapToolEditMeshFrame::removeFromSelection( int vertexIndex )
{
  mSelectedVertices.remove( vertexIndex );
  delete mSelectedVerticesMarker.value( vertexIndex );
  mSelectedVerticesMarker.remove( vertexIndex );
}

bool QgsMapToolEditMeshFrame::isFaceSelected( int faceIndex )
{
  const QgsMeshFace &face = nativeFace( faceIndex );

  for ( int i = 0; i < face.size(); ++i )
  {
    if ( !mSelectedVertices.contains( face.at( i ) ) )
      return false;
  }
  return true;
}

void QgsMapToolEditMeshFrame::setSelectedVertices( const QList<int> newSelectedVertices, Qgis::SelectBehavior behavior )
{

  bool removeVertices = false;

  switch ( behavior )
  {
    case Qgis::SelectBehavior::SetSelection:
      clearSelection();
      break;
    case Qgis::SelectBehavior::AddToSelection:
      break;
    case Qgis::SelectBehavior::RemoveFromSelection:
      removeVertices = true;
      break;
    case Qgis::SelectBehavior::IntersectSelection:
      return;
      break;
  }

  for ( const int vertexIndex : newSelectedVertices )
  {
    bool contained = mSelectedVertices.contains( vertexIndex );
    if ( contained &&  removeVertices )
      removeFromSelection( vertexIndex );
    else if ( ! removeVertices && !contained )
      addNewSelectedVertex( vertexIndex );
  }

  prepareSelection();
}

void QgsMapToolEditMeshFrame::setSelectedFaces( const QList<int> newSelectedFaces, Qgis::SelectBehavior behavior )
{
  bool removeFaces = false;

  switch ( behavior )
  {
    case Qgis::SelectBehavior::SetSelection:
      clearSelection();
      break;
    case Qgis::SelectBehavior::AddToSelection:
      break;
    case Qgis::SelectBehavior::RemoveFromSelection:
      removeFaces = true;
      break;
    case Qgis::SelectBehavior::IntersectSelection:
      return;
      break;
  }

  const QSet<int> facesToTreat = qgis::listToSet( newSelectedFaces );

  for ( const int faceIndex : newSelectedFaces )
  {
    const QgsMeshFace &face = nativeFace( faceIndex );
    for ( int i = 0; i < face.size(); ++i )
    {
      int vertexIndex = face.at( i );
      bool vertexContained = mSelectedVertices.contains( vertexIndex );
      if ( vertexContained && removeFaces )
      {
        const QList<int> facesAround = mCurrentEditor->topologicalMesh().facesAroundVertex( vertexIndex );
        bool keepVertex = false;
        for ( const int faceAroundIndex : facesAround )
          keepVertex |= !facesToTreat.contains( faceAroundIndex ) && isFaceSelected( faceAroundIndex );
        if ( !keepVertex )
          removeFromSelection( vertexIndex );
      }
      else if ( !removeFaces && !vertexContained )
        addNewSelectedVertex( vertexIndex );
    }
  }

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
    clearSelection();
  }
}

void QgsMapToolEditMeshFrame::splitSelectedFaces()
{
  if ( mSplittableFaceCount > 0 )
    mCurrentEditor->splitFaces( mSelectedFaces.values() );
  else if ( mCurrentFaceIndex != -1 && mCurrentEditor->faceCanBeSplit( mCurrentFaceIndex ) )
    mCurrentEditor->splitFaces( {mCurrentFaceIndex} );
}

void QgsMapToolEditMeshFrame::triggerTransformCoordinatesDockWidget( bool checked )
{
  if ( !checked && mTransformDockWidget )
  {
    mTransformDockWidget->close();
    return;
  }
  else if ( mTransformDockWidget )
  {
    mTransformDockWidget->show();
    return;
  }

  onEditingStarted();
  mTransformDockWidget = new QgsMeshTransformCoordinatesDockWidget( QgisApp::instance() );
  mTransformDockWidget->setWindowTitle( tr( "Transform Mesh Vertices" ) );
  mTransformDockWidget->setObjectName( QStringLiteral( "TransformMeshVerticesDockWidget" ) );
  mTransformDockWidget->setInput( mCurrentLayer, mSelectedVertices.keys() );

  if ( !QgisApp::instance()->restoreDockWidget( mTransformDockWidget ) )
    QgisApp::instance()->addDockWidget( Qt::LeftDockWidgetArea, mTransformDockWidget );
  else
    QgisApp::instance()->panelMenu()->addAction( mTransformDockWidget->toggleViewAction() );

  mTransformDockWidget->show();

  connect( this, &QgsMapToolEditMeshFrame::selectionChange, mTransformDockWidget, &QgsMeshTransformCoordinatesDockWidget::setInput );

  connect( mTransformDockWidget, &QgsMeshTransformCoordinatesDockWidget::calculationUpdated, this, [this]
  {
    mMovingFacesRubberband->reset( QgsWkbTypes::PolygonGeometry );
    mMovingEdgesRubberband->reset( QgsWkbTypes::LineGeometry );
    setMovingRubberBandValidity( mTransformDockWidget->isResultValid() );

    if ( !mCurrentLayer || !mCurrentEditor )
      return;

    QList<int> faceList = qgis::setToList( mSelectedFaces );
    QgsGeometry faceGeometrie;
    if ( faceList.count() == 1 )
    {
      const QgsMeshFace &face = mCurrentLayer->nativeMesh()->face( faceList.at( 0 ) );
      const int faceSize = face.size();
      QVector<QgsPointXY> faceVertices( faceSize );
      for ( int j = 0; j < faceSize; ++j )
        faceVertices[j] = mTransformDockWidget->transformedVertex( face.at( j ) );

      faceGeometrie = QgsGeometry::fromPolygonXY( {faceVertices} );
    }
    else
    {
      std::unique_ptr<QgsGeometryEngine> geomEngine( QgsGeometry::createGeometryEngine( faceGeometrie.constGet() ) );
      geomEngine->prepareGeometry();
      QVector<QgsGeometry> faces( mSelectedFaces.count() );
      for ( int i = 0; i < faceList.count(); ++i )
      {
        const QgsMeshFace &face = mCurrentLayer->nativeMesh()->face( faceList.at( i ) );
        const int faceSize = face.size();
        QVector<QgsPointXY> faceVertices( faceSize );
        for ( int j = 0; j < faceSize; ++j )
          faceVertices[j] = mTransformDockWidget->transformedVertex( face.at( j ) );

        faces[i] = QgsGeometry::fromPolygonXY( {faceVertices} );
      }
      QString error;
      faceGeometrie = QgsGeometry( geomEngine->combine( faces, &error ) );
    }

    QSet<int> borderMovingFace;
    QgsGeometry edgesGeom = QgsGeometry::fromMultiPolylineXY( QgsMultiPolylineXY() );
    for ( QMap<int, SelectedVertexData>::const_iterator it = mSelectedVertices.constBegin(); it != mSelectedVertices.constEnd(); ++it )
    {
      const QgsPointXY &point1 = mTransformDockWidget->transformedVertex( it.key() ) ;
      const SelectedVertexData &vertexData = it.value();
      for ( int i = 0; i < vertexData.meshFixedEdges.count(); ++i )
      {
        const QgsPointXY point2 = mTransformDockWidget->transformedVertex( vertexData.meshFixedEdges.at( i ).second );
        QgsGeometry edge( new QgsLineString( {point1, point2} ) );
        edgesGeom.addPart( edge );
        int associateFace = vertexData.meshFixedEdges.at( i ).first;
        if ( associateFace != -1 )
          borderMovingFace.insert( associateFace );
      }

      for ( int i = 0; i < vertexData.borderEdges.count(); ++i )
      {
        const QgsPointXY point2 = mTransformDockWidget->transformedVertex( vertexData.borderEdges.at( i ).second );
        const QgsGeometry edge( new QgsLineString( {point1, point2} ) );
        edgesGeom.addPart( edge );
      }
    }

    QgsCoordinateTransform coordinateTransform( mCurrentLayer->crs(), canvas()->mapSettings().destinationCrs(), QgsProject::instance() );

    try
    {
      faceGeometrie.transform( coordinateTransform );
    }
    catch ( QgsCsException & )
    {}

    try
    {
      edgesGeom.transform( coordinateTransform );
    }
    catch ( QgsCsException & )
    {}

    mMovingFacesRubberband->setToGeometry( faceGeometrie );
    mMovingEdgesRubberband->setToGeometry( edgesGeom );
    setMovingRubberBandValidity( mTransformDockWidget->isResultValid() );
  } );

  connect( mTransformDockWidget, &QgsMeshTransformCoordinatesDockWidget::aboutToBeApplied, this, [this]
  {
    mKeepSelectionOnEdit = true;
  } );

  connect( mTransformDockWidget, &QgsMeshTransformCoordinatesDockWidget::applied, this, [this]
  {
    mTransformDockWidget->setInput( mCurrentLayer, mSelectedVertices.keys() );
    updateSelectecVerticesMarker();
    prepareSelection();
  } );

  connect( mTransformDockWidget, &QgsDockWidget::closed, this, [this]
  {
    mActionTransformCoordinates->setChecked( false );
    if ( !mIsInitialized )
      return;
    mMovingFacesRubberband->reset( QgsWkbTypes::PolygonGeometry );
    mMovingEdgesRubberband->reset( QgsWkbTypes::LineGeometry );
    setMovingRubberBandValidity( false );
  } );

}

void QgsMapToolEditMeshFrame::forceBySelectedLayerPolyline()
{
  const QList<QgsGeometry> geoms = selectedGeometriesInVectorLayers();
  if ( geoms.isEmpty() )
    return;

  onEditingStarted();

  QgsMeshEditForceByPolylines forceByPolylinesEdit;

  double tolerance = QgsTolerance::toleranceInMapUnits( mWidgetActionForceByLine->toleranceValue(),
                     nullptr,
                     canvas()->mapSettings(),
                     QgsTolerance::ProjectUnits );

  forceByPolylinesEdit.setTolerance( tolerance );
  forceByPolylinesEdit.setAddVertexOnIntersection( mWidgetActionForceByLine->newVertexOnIntersectingEdge() );
  forceByPolylinesEdit.setDefaultZvalue( mZValueWidget->zValue() );
  forceByPolylinesEdit.setInterpolateZValueOnMesh(
    mWidgetActionForceByLine->interpolationMode() == QgsMeshEditForceByLineAction::Mesh );

  for ( const QgsGeometry &geom : geoms )
    forceByPolylinesEdit.addLineFromGeometry( geom );

  mCurrentEditor->advancedEdit( &forceByPolylinesEdit );
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
    if ( !( modifiers & Qt::AltModifier ) )
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

  Qgis::SelectBehavior behavior;
  if ( modifiers & Qt::ShiftModifier )
    behavior = Qgis::SelectBehavior::RemoveFromSelection;
  else if ( modifiers & Qt::ControlModifier )
    behavior = Qgis::SelectBehavior::RemoveFromSelection;
  else
    behavior = Qgis::SelectBehavior::SetSelection;

  setSelectedVertices( selectedVertices.values(), behavior );
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
  if ( !mSelectedVertices.isEmpty() )
  {
    double vertexZValue = 0;
    for ( int i : mSelectedVertices.keys() )
      vertexZValue += mapVertex( i ).z();
    vertexZValue /= mSelectedVertices.count();

    mZValueWidget->setDefaultValue( vertexZValue );
  }

  mConcernedFaceBySelection.clear();
  QMap<int, SelectedVertexData> movingVertices;


  double xMin = std::numeric_limits<double>::max();
  double xMax = -std::numeric_limits<double>::max();
  double yMin = std::numeric_limits<double>::max();
  double yMax = -std::numeric_limits<double>::max();

  // search for moving edges and mesh fixed edges
  for ( QMap<int, SelectedVertexData>::iterator it = mSelectedVertices.begin(); it != mSelectedVertices.end(); ++it )
  {
    SelectedVertexData &vertexData = it.value();
    int vertexIndex = it.key();

    QgsPointXY vert = mapVertex( vertexIndex );
    if ( vert.x() < xMin )
      xMin = vert.x();
    if ( vert.x() > xMax )
      xMax = vert.x();
    if ( vert.y() < yMin )
      yMin = vert.y();
    if ( vert.y() > yMax )
      yMax = vert.y();

    vertexData.borderEdges.clear();
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
        vertexData.borderEdges.append( {circulator.currentFaceIndex(), oppositeVertex} );
      else
        vertexData.meshFixedEdges.append( {circulator.currentFaceIndex(), oppositeVertex} );

      mConcernedFaceBySelection.insert( circulator.currentFaceIndex() );
    }
    while ( circulator.turnCounterClockwise() != firstface && circulator.currentFaceIndex() != -1 );

    if ( circulator.currentFaceIndex() == -1 )
    {
      circulator.turnClockwise();
      int oppositeVertex = circulator.oppositeVertexCounterClockwise();
      if ( mSelectedVertices.contains( oppositeVertex ) )
        vertexData.borderEdges.append( {-1, oppositeVertex} );
      else
        vertexData.meshFixedEdges.append( {-1, oppositeVertex} );
    }
  }

  mSelectedMapExtent = QgsRectangle( xMin, yMin, xMax, yMax );

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

  // here, we search for border edges that have associate face in the selection and remove it
  for ( QMap<int, SelectedVertexData>::iterator it = mSelectedVertices.begin(); it != mSelectedVertices.end(); ++it )
  {
    SelectedVertexData &vertexData = it.value();
    int i = 0;
    while ( i < vertexData.borderEdges.count() )
    {
      int associateFace = vertexData.borderEdges.at( i ).first;
      if ( associateFace == -1 || mSelectedFaces.contains( associateFace ) )
        vertexData.borderEdges.removeAt( i );
      else
        i++;
    }
  }

  if ( !mSelectedFaces.isEmpty() )
  {
    const QList<int> faceList = qgis::setToList( mSelectedFaces );
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
      for ( int i = 0; i < faceList.count(); ++i )
        otherFaces[i] = QgsGeometry( new QgsPolygon( new QgsLineString( nativeFaceGeometry( faceList.at( i ) ) ) ) );
      QString error;
      const QgsGeometry allFaces( geomEngine->combine( otherFaces, &error ) );
      mSelectedFacesRubberband->setToGeometry( allFaces );
    }

    QColor fillColor = canvas()->mapSettings().selectionColor();

    if ( fillColor.alpha() > 100 ) //set alpha to 150 if the transparency is not enough to see the mesh
      fillColor.setAlpha( 100 );

    mSelectedFacesRubberband->setFillColor( fillColor );
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

  emit selectionChange( mCurrentLayer, mSelectedVertices.keys() );
}

void QgsMapToolEditMeshFrame::updateSelectecVerticesMarker()
{
  qDeleteAll( mSelectedVerticesMarker );
  mSelectedVerticesMarker.clear();
  for ( const int vertexIndex : mSelectedVertices.keys() )
  {
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

void QgsMapToolEditMeshFrame::setMovingRubberBandValidity( bool valid )
{
  if ( valid )
  {
    mMovingFacesRubberband->setFillColor( QColor( 0, 200, 0, 100 ) );
    mMovingFacesRubberband->setStrokeColor( QColor( 0, 200, 0 ) );
    mMovingEdgesRubberband->setColor( QColor( 0, 200, 0 ) );
  }
  else
  {
    mMovingFacesRubberband->setFillColor( QColor( 200, 0, 0, 100 ) );
    mMovingFacesRubberband->setStrokeColor( QColor( 200, 0, 0 ) );
    mMovingEdgesRubberband->setColor( QColor( 200, 0, 0 ) );
  }
}

QList<QgsGeometry> QgsMapToolEditMeshFrame::selectedGeometriesInVectorLayers() const
{
  const QList<QgsMapLayer *> layers = canvas()->layers();

  QList<QgsGeometry> geomList;

  for ( QgsMapLayer *layer : layers )
  {
    QgsVectorLayer *vectoLayer = qobject_cast<QgsVectorLayer *>( layer );

    if ( vectoLayer )
    {
      const QgsFeatureList &features = vectoLayer->selectedFeatures();

      QgsCoordinateTransform transform = canvas()->mapSettings().layerTransform( vectoLayer );
      for ( const QgsFeature &feat : features )
      {
        QgsGeometry geom = feat.geometry();

        try
        {
          geom.transform( transform );
        }
        catch ( QgsCsException &e )
        {
          Q_UNUSED( e );
          continue;
        }
        geomList.append( geom );
      }
    }
  }

  return geomList;
}

bool QgsMapToolEditMeshFrame::areGeometriesSelectedInVectorLayer() const
{
  const QList<QgsMapLayer *> layers = canvas()->layers();

  QList<QgsGeometry> geomList;

  for ( QgsMapLayer *layer : layers )
  {
    QgsVectorLayer *vectoLayer = qobject_cast<QgsVectorLayer *>( layer );

    if ( vectoLayer )
    {
      if ( vectoLayer->selectedFeatureCount() > 0 )
        return true;
    }
  }

  return false;
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
    return;

  deleteZValueWidget();

  mZValueWidget = new QgsZValueWidget( tr( "Vertex Z value:" ) );
  mZValueWidget->setDefaultValue( mOrdinaryZValue );
  QgisApp::instance()->addUserInputWidget( mZValueWidget );
}

void QgsMapToolEditMeshFrame::deleteZValueWidget()
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


void QgsMapToolEditMeshFrame::selectByExpression( const QString &textExpression, Qgis::SelectBehavior behavior, QgsMesh::ElementType elementType )
{
  if ( !mCurrentEditor || !mCurrentLayer )
    return;
  QgsExpression expression( textExpression );

  std::unique_ptr<QgsDistanceArea> distArea = std::make_unique<QgsDistanceArea>();
  distArea->setSourceCrs( mCurrentLayer->crs(), QgsProject::instance()->transformContext() );
  distArea->setEllipsoid( QgsProject::instance()->ellipsoid() );
  expression.setAreaUnits( QgsProject::instance()->areaUnits() );
  expression.setDistanceUnits( QgsProject::instance()->distanceUnits() );
  expression.setGeomCalculator( distArea.release() );

  switch ( elementType )
  {
    case QgsMesh::Vertex:
      setSelectedVertices( mCurrentLayer->selectVerticesByExpression( expression ), behavior );
      break;
    case QgsMesh::Face:
      setSelectedFaces( mCurrentLayer->selectFacesByExpression( expression ), behavior );
      break;
    case QgsMesh::Edge:
      //not supported
      break;
  }
}

void QgsMapToolEditMeshFrame::onZoomToSelected()
{
  canvas()->zoomToFeatureExtent( mSelectedMapExtent );
}

void QgsMapToolEditMeshFrame::showSelectByExpressionDialog()
{
  onEditingStarted();
  QgsMeshSelectByExpressionDialog *dialog = new QgsMeshSelectByExpressionDialog( canvas() );
  dialog->setAttribute( Qt::WA_DeleteOnClose );
  dialog->show();
  connect( dialog, &QgsMeshSelectByExpressionDialog::select, this, &QgsMapToolEditMeshFrame::selectByExpression );
  connect( dialog, &QgsMeshSelectByExpressionDialog::zoomToSelected, this, &QgsMapToolEditMeshFrame::onZoomToSelected );
}
