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
#include "moc_qgsmaptooleditmeshframe.cpp"

#include <QMessageBox>
#include <QLocale>

#include "qgis.h"
#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgsstatusbar.h"

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
#include "qgsmaptoolselectionhandler.h"
#include "qgsvectorlayer.h"
#include "qgsunitselectionwidget.h"
#include "qgsmeshselectbyexpressiondialog.h"
#include "qgsmaptoolidentify.h"
#include "qgsidentifymenu.h"
#include "qgsprojectelevationproperties.h"
#include "qgscoordinatetransform.h"
#include "qgsterrainprovider.h"
#include "qgsdistancearea.h"


//
// QgsZValueWidget
//


QgsZValueWidget::QgsZValueWidget( const QString &label, QWidget *parent )
  : QWidget( parent )
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
  mZValueSpinBox->interpretText();
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

double QgsZValueWidget::getDefaultValue()
{
  return mZValueSpinBox->clearValue();
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
  double toleranceValue = settings.value( QStringLiteral( "UI/Mesh/ForceByLineToleranceValue" ), 1.0 ).toDouble( &ok );
  if ( !ok )
    toleranceValue = 1.0;
  mToleranceSpinBox->setValue( toleranceValue );
  mToleranceSpinBox->setKeyboardTracking( false );
  mToleranceSpinBox->setWrapping( false );
  mToleranceSpinBox->setSingleStep( 0.1 );
  mToleranceSpinBox->setClearValue( 1.0 );

  mUnitSelecionWidget = new QgsUnitSelectionWidget();
  mUnitSelecionWidget->setUnits(
    { Qgis::RenderUnit::MetersInMapUnits,
      Qgis::RenderUnit::MapUnits
    }
  );

  Qgis::RenderUnit toleranceUnit = settings.enumValue( QStringLiteral( "UI/Mesh/ForceByLineToleranceUnit" ), Qgis::RenderUnit::MapUnits );
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

Qgis::RenderUnit QgsMeshEditForceByLineAction::toleranceUnit() const
{
  return mUnitSelecionWidget->unit();
}

void QgsMeshEditForceByLineAction::updateSettings()
{
  QgsSettings settings;

  settings.setValue( QStringLiteral( "UI/Mesh/ForceByLineNewVertex" ), mCheckBoxNewVertex->isChecked() );
  settings.setEnumValue( QStringLiteral( "UI/Mesh/ForceByLineInterpolateFrom" ), static_cast<IntepolationMode>( mComboInterpolateFrom->currentData().toInt() ) );
  settings.setValue( QStringLiteral( "UI/Mesh/ForceByLineToleranceValue" ), mToleranceSpinBox->value() );
  settings.setEnumValue( QStringLiteral( "UI/Mesh/ForceByLineToleranceUnit" ), mUnitSelecionWidget->unit() );
}

//
// QgsMeshEditDigitizingAction
//

QgsMeshEditDigitizingAction::QgsMeshEditDigitizingAction( QObject *parent )
  : QWidgetAction( parent )
{
  QGridLayout *gLayout = new QGridLayout();
  gLayout->setContentsMargins( 3, 2, 3, 2 );

  QgsSettings settings;

  QLabel *labelZValueType = new QLabel( tr( "New vertex Z value" ) );
  labelZValueType->setToolTip( tr( "Sets the source of Z values when adding new vertices" ) );
  mComboZValueType = new QComboBox();
  mComboZValueType->addItem( tr( "Prefer mesh, then Z Widget" ), PreferMeshThenZWidget );
  mComboZValueType->setItemData( 0, tr( "Get interpolated value from the existing mesh.\nIf vertex lies outside the mesh, get the value from the Z value widget instead." ), Qt::ToolTipRole );
  mComboZValueType->addItem( tr( "Prefer mesh, then terrain" ), PreferMeshThenTerrain );
  mComboZValueType->setItemData( 1, tr( "Get interpolated value from the existing mesh.\nIf vertex lies outside the mesh, get the value from the project terrain instead." ), Qt::ToolTipRole );
  mComboZValueType->addItem( tr( "Project terrain" ), Terrain );
  mComboZValueType->setItemData( 2, tr( "Get the value from the project's terrain.\nIf the terrain has no value for the point, the default elevation will be used." ), Qt::ToolTipRole );
  mComboZValueType->addItem( tr( "Z Widget" ), ZWidget );
  mComboZValueType->setItemData( 3, tr( "Always use the value set in the Z value widget" ), Qt::ToolTipRole );

  int interpolateFromValue = settings.enumValue( QStringLiteral( "UI/Mesh/zValueFrom" ), PreferMeshThenZWidget );
  mComboZValueType->setCurrentIndex( interpolateFromValue );

  mCheckBoxRefineNeighboringFaces = new QCheckBox( tr( "Refine neighboring faces when adding vertices" ) );
  mCheckBoxRefineNeighboringFaces->setToolTip( "Flip edges that do not fulfil delaunay rule on triangular faces that share at least one vertex with the face that new vertex was added to." );

  bool refineNeighboringFaces = settings.value( QStringLiteral( "UI/Mesh/refineNeighboringFaces" ) ).toBool();
  mCheckBoxRefineNeighboringFaces->setChecked( refineNeighboringFaces );

  gLayout->addWidget( labelZValueType, 1, 0, 1, 1 );
  gLayout->addWidget( mComboZValueType, 1, 1, 1, 1 );
  gLayout->addWidget( mCheckBoxRefineNeighboringFaces, 2, 0, 1, 2 );

  QWidget *w = new QWidget();
  w->setLayout( gLayout );
  setDefaultWidget( w );

  connect( mCheckBoxRefineNeighboringFaces, &QCheckBox::toggled, this, &QgsMeshEditDigitizingAction::updateSettings );
}

void QgsMeshEditDigitizingAction::updateSettings()
{
  QgsSettings settings;

  settings.setEnumValue( QStringLiteral( "UI/Mesh/zValueFrom" ), static_cast<ZValueSource>( mComboZValueType->currentData().toInt() ) );
  settings.setValue( QStringLiteral( "UI/Mesh/refineNeighboringFaces" ), mCheckBoxRefineNeighboringFaces->isChecked() );
}

QgsMeshEditDigitizingAction::ZValueSource QgsMeshEditDigitizingAction::zValueSourceType() const
{
  return static_cast<ZValueSource>( mComboZValueType->currentData().toInt() );
}

void QgsMeshEditDigitizingAction::setZValueType( QgsMeshEditDigitizingAction::ZValueSource zValueSource )
{
  mComboZValueType->setCurrentIndex( mComboZValueType->findData( zValueSource ) );
}

bool QgsMeshEditDigitizingAction::refineNeighboringFaces() const
{
  return mCheckBoxRefineNeighboringFaces->isChecked();
}

//
// QgsMapToolEditMeshFrame
//

QgsMapToolEditMeshFrame::QgsMapToolEditMeshFrame( QgsMapCanvas *canvas )
  : QgsMapToolAdvancedDigitizing( canvas, QgisApp::instance()->cadDockWidget() )
  , mSnapIndicator( new QgsSnapIndicator( canvas ) )
{
  mActionDigitizing = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionMeshDigitizing.svg" ) ), tr( "Digitize Mesh Elements" ), this );
  mActionDigitizing->setCheckable( true );

  mActionSelectByPolygon = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionMeshSelectPolygon.svg" ) ), tr( "Select Mesh Elements by Polygon" ), this );
  mActionSelectByPolygon->setCheckable( true );
  mActionSelectByPolygon->setObjectName( QStringLiteral( "ActionMeshSelectByPolygon" ) );
  mActionSelectByExpression = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionMeshSelectExpression.svg" ) ), tr( "Select Mesh Elements by Expression" ), this );
  mActionSelectByExpression->setObjectName( QStringLiteral( "ActionMeshSelectByExpression" ) );

  mSelectionHandler = std::make_unique<QgsMapToolSelectionHandler>( canvas, QgsMapToolSelectionHandler::SelectPolygon );

  mActionSelectIsolatedVertices = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionMeshSelectIsolatedVertices.svg" ) ), tr( "Select Isolated Vertices" ), this );
  mActionSelectAllVertices = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionMeshSelectAll.svg" ) ), tr( "Select All Vertices" ), this );

  mSelectActions << mActionSelectByPolygon
                 << mActionSelectByExpression
                 << mActionSelectIsolatedVertices
                 << mActionSelectAllVertices;

  mActionTransformCoordinates = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionMeshTransformByExpression.svg" ) ), tr( "Transform Vertices Coordinates" ), this );
  mActionTransformCoordinates->setCheckable( true );

  mActionForceByLines = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionMeshEditForceByVectorLines.svg" ) ), tr( "Force by Selected Geometries" ), this );
  mActionForceByLines->setCheckable( true );

  mWidgetActionForceByLine = new QgsMeshEditForceByLineAction( this );
  mWidgetActionForceByLine->setMapCanvas( canvas );

  mWidgetActionDigitizing = new QgsMeshEditDigitizingAction( this );

  mActionReindexMesh = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionMeshReindex.svg" ) ), tr( "Reindex Faces and Vertices" ), this );

  mActionRemoveVerticesFillingHole = new QAction( this );
  mActionDelaunayTriangulation = new QAction( tr( "Delaunay Triangulation with Selected Vertices" ), this );
  mActionFacesRefinement = new QAction( tr( "Refine Current Face" ), this );
  mActionRemoveVerticesWithoutFillingHole = new QAction( this );
  mActionRemoveFaces = new QAction( tr( "Remove Current Face" ), this );
  mActionSplitFaces = new QAction( tr( "Split Current Face" ), this );

  connect( mActionRemoveVerticesFillingHole, &QAction::triggered, this, [this] { removeSelectedVerticesFromMesh( true ); } );
  connect( mActionRemoveVerticesWithoutFillingHole, &QAction::triggered, this, [this] { removeSelectedVerticesFromMesh( false ); } );
  connect( mActionRemoveFaces, &QAction::triggered, this, &QgsMapToolEditMeshFrame::removeFacesFromMesh );
  connect( mActionSplitFaces, &QAction::triggered, this, &QgsMapToolEditMeshFrame::splitSelectedFaces );

  connect( mActionDigitizing, &QAction::toggled, this, [this]( bool checked ) {
    if ( checked )
      activateWithState( Digitizing );
  } );

  for ( int i = 0; i < mSelectActions.count(); ++i )
  {
    connect( mSelectActions.at( i ), &QAction::triggered, this, [i] {
      QgsSettings settings;
      settings.setValue( QStringLiteral( "UI/Mesh/defaultSelection" ), i );
    } );
  }

  connect( mActionSelectByPolygon, &QAction::triggered, this, [this] {
    if ( mActionSelectByPolygon->isChecked() )
    {
      activateWithState( SelectingByPolygon );
    }
    else
      mSelectionBand->reset( Qgis::GeometryType::Polygon );
  } );

  connect( mActionSelectIsolatedVertices, &QAction::triggered, this, [this] {
    onEditingStarted();
    setSelectedVertices( mCurrentEditor->freeVerticesIndexes(), Qgis::SelectBehavior::SetSelection );
  } );

  connect( mActionSelectAllVertices, &QAction::triggered, this, [this] {
    onEditingStarted();
    QList<int> verticesIndexes = mCurrentLayer->selectVerticesByExpression( QgsExpression( "true" ) );
    setSelectedVertices( verticesIndexes, Qgis::SelectBehavior::SetSelection );
  } );

  connect( mActionSelectByExpression, &QAction::triggered, this, &QgsMapToolEditMeshFrame::showSelectByExpressionDialog );
  connect( mActionTransformCoordinates, &QAction::triggered, this, &QgsMapToolEditMeshFrame::triggerTransformCoordinatesDockWidget );
  connect( mActionReindexMesh, &QAction::triggered, this, &QgsMapToolEditMeshFrame::reindexMesh );
  connect( mActionDelaunayTriangulation, &QAction::triggered, this, [this] {
    if ( mCurrentEditor && mSelectedVertices.count() >= 3 )
    {
      QgsTemporaryCursorOverride waitCursor( Qt::WaitCursor );
      QgsMeshEditingDelaunayTriangulation triangulation;
      triangulation.setInputVertices( mSelectedVertices.keys() );
      mCurrentEditor->advancedEdit( &triangulation );

      if ( !triangulation.message().isEmpty() )
        QgisApp::instance()->messageBar()->pushInfo( tr( "Delaunay triangulation" ), triangulation.message() );
    }
  } );
  connect( mActionFacesRefinement, &QAction::triggered, this, [this] {
    QgsTemporaryCursorOverride waitCursor( Qt::WaitCursor );
    QgsMeshEditRefineFaces refinement;
    if ( mCurrentEditor && mSelectedFaces.count() > 0 )
    {
      refinement.setInputFaces( mSelectedFaces.values() );
      mCurrentEditor->advancedEdit( &refinement );
    }
    else if ( mCurrentFaceIndex != -1 )
    {
      refinement.setInputFaces( { mCurrentFaceIndex } );
      mCurrentEditor->advancedEdit( &refinement );
    }
  } );

  connect( mSelectionHandler.get(), &QgsMapToolSelectionHandler::geometryChanged, this, [this]( Qt::KeyboardModifiers modifiers ) {
    mIsSelectingPolygonInProgress = false;
    selectByGeometry( mSelectionHandler->selectedGeometry(), modifiers );
  } );

  connect( mActionForceByLines, &QAction::toggled, this, [this]( bool checked ) {
    if ( mIsInitialized )
      mForceByLineRubberBand->reset( Qgis::GeometryType::Line );
    mForcingLineZValue.clear();
    if ( checked )
    {
      onEditingStarted();
      clearCanvasHelpers();
      activateWithState( ForceByLines );
    }
  } );

  connect( cadDockWidget(), &QgsAdvancedDigitizingDockWidget::cadEnabledChanged, this, [this]( bool enable ) {
    if ( !isActive() || !mCurrentEditor )
      return;

    if ( enable && mSelectedVertices.isEmpty() )
      deleteZValueWidget();
    else if ( !mZValueWidget )
      createZValueWidget();
  } );

  connect( this, &QgsMapToolEditMeshFrame::selectionChange, this, &QgsMapToolEditMeshFrame::updateStatusBarMessage );

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
    << mActionForceByLines
    << mActionReindexMesh
    << mActionSelectIsolatedVertices
    << mActionSelectAllVertices;

  for ( QAction *action : std::as_const( actions ) )
    action->setEnabled( enable );
}


QList<QAction *> QgsMapToolEditMeshFrame::mapToolActions()
{
  return QList<QAction *>()
         << mActionDigitizing
         << mActionSelectByPolygon
         << mActionForceByLines;
}

QAction *QgsMapToolEditMeshFrame::digitizeAction() const
{
  return mActionDigitizing;
}

QList<QAction *> QgsMapToolEditMeshFrame::selectActions() const
{
  return mSelectActions;
}

QAction *QgsMapToolEditMeshFrame::defaultSelectActions() const
{
  const QgsSettings settings;
  bool ok = false;
  int defaultIndex = settings.value( QStringLiteral( "UI/Mesh/defaultSelection" ) ).toInt( &ok );

  if ( ok )
    return mSelectActions.at( defaultIndex );

  return mActionSelectByPolygon;
}

QAction *QgsMapToolEditMeshFrame::transformAction() const
{
  return mActionTransformCoordinates;
}

QList<QAction *> QgsMapToolEditMeshFrame::forceByLinesActions() const
{
  return QList<QAction *>()
         << mActionForceByLines;
}

QAction *QgsMapToolEditMeshFrame::defaultForceAction() const
{
  return mActionForceByLines;
}

QWidgetAction *QgsMapToolEditMeshFrame::forceByLineWidgetActionSettings() const
{
  return mWidgetActionForceByLine;
}

QWidgetAction *QgsMapToolEditMeshFrame::digitizingWidgetActionSettings() const
{
  return mWidgetActionDigitizing;
}

QAction *QgsMapToolEditMeshFrame::reindexAction() const
{
  return mActionReindexMesh;
}

void QgsMapToolEditMeshFrame::initialize()
{
  if ( !mFaceRubberBand )
    mFaceRubberBand = createRubberBand( Qgis::GeometryType::Polygon );
  mFaceRubberBand->setVisible( false );
  mFaceRubberBand->setZValue( 5 );

  const QColor color = digitizingStrokeColor();
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
    mNewFaceBand = createRubberBand( Qgis::GeometryType::Polygon );
  mInvalidFaceColor = QColor( 255, 0, 0, mNewFaceBand->fillColor().alpha() ); //override color and keep only the transparency
  mValidFaceColor = QColor( 0, 255, 0, mNewFaceBand->fillColor().alpha() );   //override color and keep only the transparency
  mNewFaceBand->setFillColor( mInvalidFaceColor );
  mNewFaceBand->setVisible( false );
  mNewFaceBand->setZValue( 10 );

  if ( !mSelectionBand )
    mSelectionBand = new QgsRubberBand( mCanvas, Qgis::GeometryType::Polygon );
  mSelectionBand->setFillColor( QColor( 254, 178, 76, 63 ) );
  mSelectionBand->setStrokeColor( QColor( 254, 58, 29, 100 ) );
  mSelectionBand->setZValue( 10 );

  if ( !mSelectedFacesRubberband )
    mSelectedFacesRubberband = new QgsRubberBand( mCanvas, Qgis::GeometryType::Polygon );
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
    mMovingEdgesRubberband = createRubberBand( Qgis::GeometryType::Line );

  if ( !mMovingFacesRubberband )
    mMovingFacesRubberband = createRubberBand( Qgis::GeometryType::Polygon );

  if ( !mMovingFreeVertexRubberband )
  {
    mMovingFreeVertexRubberband = createRubberBand( Qgis::GeometryType::Point );
    mMovingFreeVertexRubberband->setIcon( QgsRubberBand::ICON_X );
    mMovingFreeVertexRubberband->setIconSize( QgsGuiUtils::scaleIconSize( 10 ) );
    mMovingFreeVertexRubberband->setWidth( QgsGuiUtils::scaleIconSize( 3 ) );
    mMovingFreeVertexRubberband->setVisible( true );
  }

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

  if ( !mForceByLineRubberBand )
    mForceByLineRubberBand = createRubberBand( Qgis::GeometryType::Line );

  connect( mCanvas, &QgsMapCanvas::currentLayerChanged, this, &QgsMapToolEditMeshFrame::setCurrentLayer );

  mUserZValue = defaultZValue();
  createZValueWidget();
  updateFreeVertices();

  mIsInitialized = true;
}

void QgsMapToolEditMeshFrame::deactivate()
{
  QgsMapToolAdvancedDigitizing::deactivate();
  clearSelection();
  clearCanvasHelpers();
  deleteZValueWidget();
  qDeleteAll( mFreeVertexMarker );
  mFreeVertexMarker.clear();
  mNewFaceCandidate.clear();
  mNewVerticesForNewFaceCandidate.clear();
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

  mFaceVerticesBand->deleteLater();
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
  if ( !cadDockWidget()->cadEnabled() )
    createZValueWidget();
}

bool QgsMapToolEditMeshFrame::populateContextMenuWithEvent( QMenu *menu, QgsMapMouseEvent *event )
{
  Q_UNUSED( event );

  switch ( mCurrentState )
  {
    case Digitizing:
    case SelectingByPolygon:
    {
      QList<QAction *> newActions;
      QList<QAction *> lastActions;

      if ( !mSelectedVertices.isEmpty() )
      {
        if ( mSelectedVertices.count() >= 3 )
          lastActions << mActionDelaunayTriangulation;

        newActions << mActionRemoveVerticesFillingHole << mActionRemoveVerticesWithoutFillingHole;
      }

      if ( !mSelectedFaces.isEmpty() || ( mCurrentFaceIndex != -1 && mCurrentState == Digitizing ) )
      {
        newActions << mActionRemoveFaces;
      }

      if ( mSplittableFaceCount > 0 || ( mCurrentFaceIndex != -1 && mCurrentEditor->faceCanBeSplit( mCurrentFaceIndex ) ) )
        newActions << mActionSplitFaces;

      int currentFaceSize = mCurrentFaceIndex != -1 ? nativeFace( mCurrentFaceIndex ).size() : 0;
      if ( mRefinableFaceCount > 0 || currentFaceSize == 3 || currentFaceSize == 4 )
        lastActions << mActionFacesRefinement;

      const QList<QAction *> existingActions = menu->actions();
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
    case MovingSelection:
    case ForceByLines:
      return false;
  }

  return false;
}

QgsMapTool::Flags QgsMapToolEditMeshFrame::flags() const
{
  switch ( mCurrentState )
  {
    case Digitizing:
      if ( !mCadDockWidget->cadEnabled() || !mSelectedVertices.isEmpty() || mCurrentFaceIndex != -1 )
        return QgsMapTool::Flags() | QgsMapTool::ShowContextMenu;
      [[fallthrough]];
    case AddingNewFace:
    case Selecting:
    case MovingSelection:
    case SelectingByPolygon:
    case ForceByLines:
      return QgsMapTool::Flags();
  }

  return QgsMapTool::Flags();
}

void QgsMapToolEditMeshFrame::forceByLineBySelectedFeature( QgsMapMouseEvent *e )
{
  const QList<QgsMapToolIdentify::IdentifyResult> &results = QgsIdentifyMenu::findFeaturesOnCanvas( e, mCanvas, QList<Qgis::GeometryType>() << Qgis::GeometryType::Polygon << Qgis::GeometryType::Line );

  QgsIdentifyMenu *menu = new QgsIdentifyMenu( mCanvas );
  menu->setExecWithSingleResult( true );
  menu->setAllowMultipleReturn( false );
  const QPoint globalPos = mCanvas->mapToGlobal( QPoint( e->pos().x() + 5, e->pos().y() + 5 ) );
  const QList<QgsMapToolIdentify::IdentifyResult> selectedFeatures = menu->exec( results, globalPos );
  menu->deleteLater();

  if ( !selectedFeatures.empty() && selectedFeatures[0].mFeature.hasGeometry() )
  {
    const QgsCoordinateTransform transform = mCanvas->mapSettings().layerTransform( selectedFeatures.at( 0 ).mLayer );
    QgsGeometry geom = selectedFeatures[0].mFeature.geometry();
    try
    {
      geom.transform( transform );
    }
    catch ( QgsCsException & )
    {
      QgsDebugError( QStringLiteral( "Could not transform geometry to layer CRS" ) );
    }
    forceByLine( geom );
  }

  return;
}

void QgsMapToolEditMeshFrame::cadCanvasPressEvent( QgsMapMouseEvent *e )
{
  if ( !mCurrentEditor )
    return;

  if ( e->button() == Qt::LeftButton && ( !mCadDockWidget->cadEnabled() || mCadDockWidget->betweenLineConstraint() == Qgis::BetweenLineConstraint::NoConstraint ) )
    mLeftButtonPressed = true;

  switch ( mCurrentState )
  {
    case Digitizing:
      if ( e->button() == Qt::LeftButton )
        mStartSelectionPos = e->pos();
      break;
    case AddingNewFace:
    case Selecting:
    case MovingSelection:
    case ForceByLines:
      if ( e->button() == Qt::LeftButton )
        mSelectionBand->reset( Qgis::GeometryType::Polygon );
      break;
    case SelectingByPolygon:
      if ( mSelectionHandler )
      {
        if ( e->button() == Qt::RightButton )
        {
          // here, quite tricky because 3 possibilities:
          // - a polygon has started to be digitized for selection -> right click validate the selection
          // - right click on an existing vector layer feature -> a menu is executed to choose a feature
          // - other case -> context menu of mesh editing to apply an edit on selected element
          // The last case is launched only if the other cases do not appears
          // With the selection handler, if we can know if the selecting polygon change, that means the first case appears,
          // we can't know if a feature is found or not (if the user do not choose a feature, nothing happen like if no feature was found).
          // The workaround is to check if a feature exist under the mouse before sending the event to the selection handler.
          // This is not ideal because that leads to a double search but no better idea for now to allow the editing context menu with selecting by polygon

          bool hasSelectableFeature = !QgsIdentifyMenu::findFeaturesOnCanvas( e, mCanvas, QList<Qgis::GeometryType>() << Qgis::GeometryType::Polygon ).isEmpty();

          if ( hasSelectableFeature || mIsSelectingPolygonInProgress )
            mSelectionHandler->canvasPressEvent( e );
          else
          {
            QMenu menu;
            populateContextMenuWithEvent( &menu, e );
            menu.exec( e->globalPos() );
          }
        }
        else
        {
          mIsSelectingPolygonInProgress = true;
          mSelectionHandler->canvasPressEvent( e );
        }
      }

      break;
  }

  QgsMapToolAdvancedDigitizing::cadCanvasPressEvent( e );
}

void QgsMapToolEditMeshFrame::cadCanvasMoveEvent( QgsMapMouseEvent *e )
{
  if ( !mCurrentEditor )
    return;

  const QgsPointXY &mapPoint = e->mapPoint();

  mSnapIndicator->setMatch( e->mapPointMatch() );

  if ( mLeftButtonPressed && mCurrentState == Digitizing )
  {
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
      if ( testNewVertexInFaceCanditate( true, mCurrentVertexIndex, mapPoint ) )
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
    case MovingSelection:
    {
      moveSelection( mapPoint );
    }
    break;
    case SelectingByPolygon:
      if ( mSelectionHandler )
        mSelectionHandler->canvasMoveEvent( e );
      break;
    case ForceByLines:
      searchFace( mapPoint );
      searchEdge( mapPoint );
      highlightCloseVertex( mapPoint );

      const QgsPointLocator::Match &matchPoint = e->mapPointMatch();

      if ( mCurrentVertexIndex != -1 )
      {
        mForceByLineRubberBand->movePoint( mapVertexXY( mCurrentVertexIndex ) );
        if ( mZValueWidget )
          mZValueWidget->setZValue( mapVertex( mCurrentVertexIndex ).z() );
      }
      else if ( matchPoint.isValid() && matchPoint.layer() && QgsWkbTypes::hasZ( matchPoint.layer()->wkbType() ) )
      {
        mForceByLineRubberBand->movePoint( mapPoint );
        if ( mZValueWidget )
          mZValueWidget->setZValue( e->mapPointMatch().interpolatedPoint( mCanvas->mapSettings().destinationCrs() ).z() );
      }
      else
      {
        mForceByLineRubberBand->movePoint( mapPoint );
        if ( mZValueWidget )
          mZValueWidget->setZValue( mUserZValue );
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

  QgsPointXY mapPoint = e->mapPoint();

  // advanced digitizing constraint only the first release of double clicks
  // so we need to store the click point for the next one that could be a double clicks
  if ( !mDoubleClicks )
  {
    mFirstClickPoint = mapPoint;
    mFirstClickZValue = currentZValue();
  }

  if ( e->button() == Qt::LeftButton )
    mLeftButtonPressed = false;

  switch ( mCurrentState )
  {
    case Digitizing:
      if ( e->button() == Qt::LeftButton )
      {
        if ( mDoubleClicks ) //double clicks --> add a vertex
        {
          addVertex( mFirstClickPoint, e->mapPointMatch() );
          mCadDockWidget->setPoints( QList<QgsPointXY>() << mFirstClickPoint << mFirstClickPoint );

          // after addition select the vertex for easier editing of Z value
          mCurrentVertexIndex = closeVertex( mapPoint );
          select( mapPoint, e->modifiers(), tolerance );
        }
        else if ( mNewFaceMarker->isVisible() && mapPoint.distance( mNewFaceMarker->center() ) < tolerance
                  && mCurrentVertexIndex >= 0 ) //new face marker clicked --> start adding a new face
        {
          clearSelection();
          mCurrentState = AddingNewFace;
          mNewFaceMarker->setVisible( false );
          mNewFaceBand->setVisible( true );
          mNewFaceBand->reset( Qgis::GeometryType::Polygon );
          addVertexToFaceCanditate( mCurrentVertexIndex );
          const QgsPointXY &currentPoint = mapVertexXY( mCurrentVertexIndex );
          cadDockWidget()->setPoints( QList<QgsPointXY>() << currentPoint << currentPoint );
        }
        else if ( isSelectionGrapped( mapPoint ) &&           //click on a selected vertex, an edge or face box
                  !( e->modifiers() & Qt::ControlModifier ) ) // without control modifier that is used to remove from the selection
        {
          mCurrentState = MovingSelection;
          mCadDockWidget->setEnabledZ( false );
          mStartMovingPoint = mapPoint;
          cadDockWidget()->setPoints( QList<QgsPointXY>() << mapPoint << mapPoint );
        }
        else if ( mFlipEdgeMarker->isVisible() && e->mapPoint().distance( mFlipEdgeMarker->center() ) < tolerance && mCurrentEdge.first != -1 && mCurrentEdge.second != -1 ) // flip edge
        {
          clearSelection();
          mCadDockWidget->clearPoints();
          const QVector<int> edgeVert = edgeVertices( mCurrentEdge );
          mCurrentEditor->flipEdge( edgeVert.at( 0 ), edgeVert.at( 1 ) );
          mCurrentEdge = { -1, -1 };
          highLight( mapPoint );
        }
        else if ( mMergeFaceMarker->isVisible() && e->mapPoint().distance( mMergeFaceMarker->center() ) < tolerance && mCurrentEdge.first != -1 && mCurrentEdge.second != -1 ) // merge two faces
        {
          clearSelection();
          mCadDockWidget->clearPoints();
          const QVector<int> edgeVert = edgeVertices( mCurrentEdge );
          mCurrentEditor->merge( edgeVert.at( 0 ), edgeVert.at( 1 ) );
          mCurrentEdge = { -1, -1 };
          highLight( mapPoint );
        }
        else
        {
          select( mapPoint, e->modifiers(), tolerance );
          mCadDockWidget->clearPoints();
        }
      }
      break;
    case AddingNewFace:
      if ( e->button() == Qt::LeftButton ) //eventually add a vertex to the face
      {
        if ( mCurrentVertexIndex != -1 )
        {
          addVertexToFaceCanditate( mCurrentVertexIndex );
          // Advanced digitizing base class adds a point at map point not at vertex position
          // so we need to replace it by the position of the vertex
          const QgsPoint &currentPoint = mapVertex( mCurrentVertexIndex );
          cadDockWidget()->updateCurrentPoint( currentPoint );
          cadDockWidget()->removePreviousPoint();
          cadDockWidget()->addPoint( currentPoint );
        }
        else
        {
          bool acceptPoint = true;
          if ( !mNewFaceCandidate.isEmpty() && mNewFaceCandidate.last() == -1 && !mNewVerticesForNewFaceCandidate.isEmpty() ) //avoid duplicate new vertex
          {
            acceptPoint = mapPoint.distance( mNewVerticesForNewFaceCandidate.last() ) > tolerance;
          }

          if ( acceptPoint )
          {
            addVertexToFaceCanditate( mapPoint );
          }
        }
      }
      else if ( e->button() == Qt::RightButton ) //if possible validate and add the face to the mesh
      {
        if ( testNewVertexInFaceCanditate( false, -1, QgsPointXY() ) )
        {
          mCurrentEditor->addFaceWithNewVertices( mNewFaceCandidate, mNewVerticesForNewFaceCandidate );
          mNewFaceBand->reset( Qgis::GeometryType::Polygon );
          mNewFaceCandidate.clear();
          mNewVerticesForNewFaceCandidate.clear();
          mCurrentState = Digitizing;
        }
      }
      break;
    case Selecting:
    {
      const QgsGeometry selectionGeom = mSelectionBand->asGeometry();
      selectByGeometry( selectionGeom, e->modifiers() );
      mSelectionBand->reset( Qgis::GeometryType::Polygon );
      mCadDockWidget->clearPoints();
      mCurrentState = Digitizing;
    }
    break;
    case MovingSelection:
      if ( mIsMovingAllowed )
      {
        const QList<int> verticesIndexes = mSelectedVertices.keys();
        QList<QgsPointXY> newPosition;
        newPosition.reserve( verticesIndexes.count() );

        const QgsMeshVertex &mapPointInNativeCoordinate = mCurrentLayer->triangularMesh()->triangularToNativeCoordinates( QgsMeshVertex( mapPoint.x(), mapPoint.y() ) );
        const QgsMeshVertex &startingPointInNativeCoordinate = mCurrentLayer->triangularMesh()->triangularToNativeCoordinates( QgsMeshVertex( mStartMovingPoint.x(), mStartMovingPoint.y() ) );
        const QgsVector &translationInLayerCoordinate = mapPointInNativeCoordinate - startingPointInNativeCoordinate;

        const QgsMesh &mesh = *mCurrentLayer->nativeMesh();
        mKeepSelectionOnEdit = true;
        if ( verticesIndexes.count() != 1 )
        {
          for ( int i = 0; i < verticesIndexes.count(); ++i )
            newPosition.append( QgsPointXY( mesh.vertex( verticesIndexes.at( i ) ) ) + translationInLayerCoordinate );
          mCurrentEditor->changeXYValues( verticesIndexes, newPosition );
        }
        else
        {
          //only one vertex, change also the Z value if snap on a 3D vector layer
          if ( e->mapPointMatch().isValid() && QgsWkbTypes::hasZ( e->mapPointMatch().layer()->wkbType() ) )
          {
            const QgsMeshVertex mapPointInMapCoordinate = QgsMeshVertex( mapPoint.x(), mapPoint.y(), e->mapPointMatch().interpolatedPoint( mCanvas->mapSettings().destinationCrs() ).z() );

            const QgsMeshVertex &mapPointInNativeCoordinate = mCurrentLayer->triangularMesh()->triangularToNativeCoordinates( mapPointInMapCoordinate );
            mCurrentEditor->changeCoordinates( verticesIndexes, QList<QgsPoint>() << mapPointInNativeCoordinate );
          }
          else
            mCurrentEditor->changeXYValues( verticesIndexes, QList<QgsPointXY>() << QgsPointXY( mesh.vertex( verticesIndexes.at( 0 ) ) ) + translationInLayerCoordinate );
        }
      }
      updateSelectecVerticesMarker();
      prepareSelection();
      clearCanvasHelpers();
      mMovingEdgesRubberband->reset();
      mMovingFacesRubberband->reset();
      mMovingFreeVertexRubberband->reset();
      mCadDockWidget->setEnabledZ( mCadDockWidget->cadEnabled() );
      mCurrentState = Digitizing;
      break;
    case SelectingByPolygon:
      if ( mSelectionHandler )
        mSelectionHandler->canvasReleaseEvent( e );
      break;
    case ForceByLines:
      forceByLineReleaseEvent( e );
      break;
  }
  mDoubleClicks = false;

  QgsMapToolAdvancedDigitizing::cadCanvasReleaseEvent( e );
}

void QgsMapToolEditMeshFrame::moveSelection( const QgsPointXY &destinationPoint )
{
  const QgsVector &translation = destinationPoint - mStartMovingPoint;
  mMovingEdgesRubberband->reset( Qgis::GeometryType::Line );
  mMovingFacesRubberband->reset( Qgis::GeometryType::Polygon );
  mMovingFreeVertexRubberband->reset( Qgis::GeometryType::Point );
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
      const QgsGeometry edge( new QgsLineString( { point1, point2 } ) );
      mMovingEdgesRubberband->addGeometry( edge );
      int associateFace = vertexData.meshFixedEdges.at( i ).first;
      if ( associateFace != -1 )
        borderMovingFace.insert( associateFace );
    }

    for ( int i = 0; i < vertexData.borderEdges.count(); ++i )
    {
      const QgsPointXY point2 = mapVertexXY( vertexData.borderEdges.at( i ).second ) + translation;
      const QgsGeometry edge( new QgsLineString( { point1, point2 } ) );
      mMovingEdgesRubberband->addGeometry( edge );
    }

    if ( mCurrentEditor->isVertexFree( it.key() ) )
      mMovingFreeVertexRubberband->addPoint( mapVertexXY( it.key() ) + translation, false );
  }

  mMovingFreeVertexRubberband->setVisible( true );
  mMovingFreeVertexRubberband->updatePosition();
  mMovingFreeVertexRubberband->update();

  const QgsMeshVertex &mapPointInNativeCoordinate = mCurrentLayer->triangularMesh()->triangularToNativeCoordinates( QgsMeshVertex( destinationPoint.x(), destinationPoint.y() ) );
  const QgsMeshVertex &startingPointInNativeCoordinate = mCurrentLayer->triangularMesh()->triangularToNativeCoordinates( QgsMeshVertex( mStartMovingPoint.x(), mStartMovingPoint.y() ) );
  const QgsVector &translationInLayerCoordinate = mapPointInNativeCoordinate - startingPointInNativeCoordinate;

  auto transformFunction = [translationInLayerCoordinate, this]( int vi ) -> const QgsMeshVertex {
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
        const QgsMeshVertex transformedVertex = transformFunction( vertexIndex );
        const QgsMeshVertex &mapTransformedVertex = mCurrentLayer->triangularMesh()->nativeToTriangularCoordinates( transformedVertex );
        const QgsPointXY pointInMap( mapTransformedVertex );
        mIsMovingAllowed &= !movingFacesGeometry.contains( &pointInMap );
        if ( !mIsMovingAllowed )
          break;
      }
    }
  }

  setMovingRubberBandValidity( mIsMovingAllowed );
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

  QgsPointXY currentPoint = mapPoint;

  if ( mSelectFaceMarker->isVisible() && mapPoint.distance( mSelectFaceMarker->center() ) < tolerance
       && mCurrentFaceIndex >= 0 )
  {
    setSelectedVertices( nativeFace( mCurrentFaceIndex ).toList(), behavior );
    currentPoint = mCurrentLayer->triangularMesh()->faceCentroids().at( mCurrentFaceIndex );
  }
  else if ( mCurrentVertexIndex != -1 )
  {
    setSelectedVertices( QList<int>() << mCurrentVertexIndex, behavior );
    currentPoint = mCurrentLayer->triangularMesh()->vertices().at( mCurrentVertexIndex );
  }
  else if ( mSelectEdgeMarker->isVisible() && mapPoint.distance( mSelectEdgeMarker->center() ) < tolerance && mCurrentEdge.first != -1 && mCurrentEdge.second != -1 )
  {
    const QVector<int> edgeVert = edgeVertices( mCurrentEdge );
    setSelectedVertices( edgeVert.toList(), behavior );
    const QgsMeshVertex v1 = mCurrentLayer->triangularMesh()->vertices().at( edgeVert.at( 0 ) );
    const QgsMeshVertex v2 = mCurrentLayer->triangularMesh()->vertices().at( edgeVert.at( 1 ) );
    currentPoint = QgsPointXY( ( v1.x() + v2.x() ) / 2, ( v1.y() + v2.y() ) / 2 );
  }
  else
    setSelectedVertices( QList<int>(), behavior );
  mCadDockWidget->setPoints( QList<QgsPointXY>() << currentPoint << currentPoint );
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
        {
          if ( mNewFaceCandidate.last() == -1 && !mNewVerticesForNewFaceCandidate.isEmpty() )
            mNewVerticesForNewFaceCandidate.removeLast();
          mNewFaceCandidate.removeLast();
        }
        if ( mNewFaceCandidate.isEmpty() )
          mCurrentState = Digitizing;

        mCadDockWidget->removePreviousPoint();

        consumned = true;
      }

      if ( e->key() == Qt::Key_Escape )
      {
        mNewFaceBand->reset( Qgis::GeometryType::Polygon );
        mNewFaceCandidate.clear();
        mNewVerticesForNewFaceCandidate.clear();
        mCurrentState = Digitizing;
        consumned = true;
      }
    }
    break;
    case MovingSelection:
      if ( e->key() == Qt::Key_Escape )
      {
        mCurrentState = Digitizing;
        mMovingEdgesRubberband->reset( Qgis::GeometryType::Line );
        mMovingFacesRubberband->reset( Qgis::GeometryType::Polygon );
        mMovingFreeVertexRubberband->reset( Qgis::GeometryType::Point );
        mCadDockWidget->setEnabledZ( mCadDockWidget->cadEnabled() );
      }
      break;
    case ForceByLines:
      if ( e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter )
      {
        if ( !mCadDockWidget->cadEnabled() )
          mUserZValue = currentZValue();
      }

      if ( e->key() == Qt::Key_Escape )
      {
        mForceByLineRubberBand->reset( Qgis::GeometryType::Line );
        mForcingLineZValue.clear();
      }
      break;
    case Selecting:
    case SelectingByPolygon:
      if ( e->key() == Qt::Key_Escape )
      {
        clearSelection();
        consumned = true;
      }
      break;
  }

  if ( e->key() == Qt::Key_Escape )
    mCadDockWidget->clearPoints();

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
    case SelectingByPolygon:
      if ( mSelectionHandler )
        mSelectionHandler->keyReleaseEvent( e );
      break;
    case Selecting:
    case MovingSelection:
    case ForceByLines:
      break;
  }

  if ( !consumned && mZValueWidget )
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
  if ( mCurrentLayer.isNull() || !mCurrentLayer->triangularMesh() )
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
  if ( mCurrentLayer.isNull() || !mCurrentLayer->nativeMesh() )
    return QgsMeshFace();

  return mCurrentLayer->nativeMesh()->face( index );
}

double QgsMapToolEditMeshFrame::currentZValue()
{
  if ( mDoubleClicks )
    return mFirstClickZValue;
  else if ( mZValueWidget )
    return mZValueWidget->zValue();
  else if ( mCadDockWidget->cadEnabled() )
    return mCadDockWidget->currentPointV2().z();

  return defaultZValue();
}

void QgsMapToolEditMeshFrame::searchFace( const QgsPointXY &mapPoint )
{
  if ( !mCurrentLayer.isNull() && mCurrentLayer->triangularMesh() )
    mCurrentFaceIndex = mCurrentLayer->triangularMesh()->nativeFaceIndexForPoint( mapPoint );
}

void QgsMapToolEditMeshFrame::searchEdge( const QgsPointXY &mapPoint )
{
  mCurrentEdge = { -1, -1 };
  double tolerance = QgsTolerance::vertexSearchRadius( canvas()->mapSettings() );

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

      const QgsPointXY pt1 = mapVertexXY( iv1 );
      const QgsPointXY pt2 = mapVertexXY( iv2 );

      QgsPointXY pointOneEdge;
      double distance = sqrt( mapPoint.sqrDistToSegment( pt1.x(), pt1.y(), pt2.x(), pt2.y(), pointOneEdge, 0 ) );
      if ( distance < tolerance && distance < minimumDistance && edgeCanBeInteractive( iv1, iv2 ) )
      {
        mCurrentEdge = { faceIndex, iv2 };
        minimumDistance = distance;
      }
    }
  }
}

void QgsMapToolEditMeshFrame::highLight( const QgsPointXY &mapPoint )
{
  highlightCurrentHoveredFace( mapPoint );
  //  searchEdge( mapPoint );
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

  if ( mCurrentLayer == meshLayer && mCurrentEditor )
  {
    activate();
    updateFreeVertices();
    return;
  }

  if ( mCurrentEditor )
    deactivate();

  if ( mIsInitialized )
    clearSelection(); //TODO later: implement a mechanism to retrieve selection if the layer is again selected

  if ( mCurrentLayer )
  {
    disconnect( mCurrentLayer, &QgsMeshLayer::editingStarted, this, &QgsMapToolEditMeshFrame::onEditingStarted );
    disconnect( mCurrentLayer, &QgsMeshLayer::editingStopped, this, &QgsMapToolEditMeshFrame::onEditingStopped );
    disconnect( mCurrentLayer->undoStack(), &QUndoStack::indexChanged, this, &QgsMapToolEditMeshFrame::onUndoRedo );
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
    connect( mCurrentLayer->undoStack(), &QUndoStack::indexChanged, this, &QgsMapToolEditMeshFrame::onUndoRedo );

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
    activate();
    updateFreeVertices();
  }

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

  return { mapVertexXY( vertexIndexes.at( 0 ) ), mapVertexXY( vertexIndexes.at( 1 ) ) };
}

QVector<int> QgsMapToolEditMeshFrame::edgeVertices( const QgsMapToolEditMeshFrame::Edge &edge ) const
{
  const QgsMeshFace &face = nativeFace( edge.first );
  int faceSize = face.count();
  int posInface = ( face.indexOf( edge.second ) + faceSize - 1 ) % faceSize;

  return { face.at( posInface ), edge.second };
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
    const QgsMeshVertexCirculator circulator = mCurrentEditor->vertexCirculator( vertexIndex );
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

    if ( crossProduct < -1e-8 )
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

void QgsMapToolEditMeshFrame::addVertexToFaceCanditate( const QgsPointXY &vertexPosition )
{
  mNewFaceBand->movePoint( vertexPosition );
  mNewFaceBand->addPoint( vertexPosition );
  QgsMeshVertex vert = QgsMeshVertex( vertexPosition.x(), vertexPosition.y(), currentZValue() );
  mNewVerticesForNewFaceCandidate.append( vert );
  mNewFaceCandidate.append( -1 );
}

bool QgsMapToolEditMeshFrame::testNewVertexInFaceCanditate( bool testLast, int vertexIndex, const QgsPointXY &mapPoint ) const
{
  QList<int> faceToTest = mNewFaceCandidate;
  QList<QgsMeshVertex> newVertices = mNewVerticesForNewFaceCandidate;

  if ( testLast )
  {
    if ( vertexIndex != -1 && !mNewFaceCandidate.empty() && vertexIndex != mNewFaceCandidate.last() && vertexIndex != mNewFaceCandidate.first() )
      faceToTest.append( vertexIndex );
    else if ( vertexIndex == -1 )
    {
      faceToTest.append( -1 );
      newVertices.append( QgsMeshVertex( mapPoint ) );
    }
  }

  return mCurrentEditor->faceCanBeAddedWithNewVertices( faceToTest, newVertices );
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

void QgsMapToolEditMeshFrame::setSelectedVertices( const QList<int> &newSelectedVertices, Qgis::SelectBehavior behavior )
{
  if ( mSelectedVertices.isEmpty() )
  {
    mUserZValue = currentZValue();
  }

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
    if ( contained && removeVertices )
      removeFromSelection( vertexIndex );
    else if ( !removeVertices && !contained )
      addNewSelectedVertex( vertexIndex );
  }

  prepareSelection();
}

void QgsMapToolEditMeshFrame::setSelectedFaces( const QList<int> &newSelectedFaces, Qgis::SelectBehavior behavior )
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
  if ( fillHole )
  {
    const QList<int> remainingVertex = mCurrentEditor->removeVerticesFillHoles( mSelectedVertices.keys() );

    if ( !remainingVertex.isEmpty() )
    {
      QgisApp::instance()->messageBar()->pushWarning(
        tr( "Mesh editing" ),
        tr( "%n vertices were not removed", nullptr, remainingVertex.count() )
      );
    }
  }
  else
  {
    const QgsMeshEditingError error = mCurrentEditor->removeVerticesWithoutFillHoles( mSelectedVertices.keys() );
    if ( error != QgsMeshEditingError() )
    {
      QgisApp::instance()->messageBar()->pushWarning(
        tr( "Mesh editing" ),
        tr( "removing the vertex %1 leads to a topological error, operation canceled." ).arg( error.elementIndex )
      );
    }
  }
}

void QgsMapToolEditMeshFrame::removeFacesFromMesh()
{
  QgsMeshEditingError error;
  if ( !mSelectedFaces.isEmpty() )
    error = mCurrentEditor->removeFaces( mSelectedFaces.values() );
  else if ( mCurrentFaceIndex != -1 )
    error = mCurrentEditor->removeFaces( { mCurrentFaceIndex } );
  else
    return;

  if ( error != QgsMeshEditingError() )
  {
    QgisApp::instance()->messageBar()->pushWarning(
      tr( "Mesh editing" ),
      tr( "removing the faces %1 leads to a topological error, operation canceled." ).arg( error.elementIndex )
    );
  }
  else
  {
    clearSelection();
  }
}

void QgsMapToolEditMeshFrame::splitSelectedFaces()
{
  QgsTemporaryCursorOverride waitCursor( Qt::WaitCursor );
  if ( mSplittableFaceCount > 0 )
    mCurrentEditor->splitFaces( mSelectedFaces.values() );
  else if ( mCurrentFaceIndex != -1 && mCurrentEditor->faceCanBeSplit( mCurrentFaceIndex ) )
    mCurrentEditor->splitFaces( { mCurrentFaceIndex } );
}

void QgsMapToolEditMeshFrame::triggerTransformCoordinatesDockWidget( bool checked )
{
  if ( mTransformDockWidget )
  {
    mTransformDockWidget->setUserVisible( checked );
    return;
  }

  onEditingStarted();
  mTransformDockWidget = new QgsMeshTransformCoordinatesDockWidget( QgisApp::instance() );
  mTransformDockWidget->setToggleVisibilityAction( mActionTransformCoordinates );
  mTransformDockWidget->setWindowTitle( tr( "Transform Mesh Vertices" ) );
  mTransformDockWidget->setObjectName( QStringLiteral( "TransformMeshVerticesDockWidget" ) );
  const QList<int> &inputVertices = mSelectedVertices.keys();
  mTransformDockWidget->setInput( mCurrentLayer, inputVertices );

  if ( !QgisApp::instance()->restoreDockWidget( mTransformDockWidget ) )
    QgisApp::instance()->addDockWidget( Qt::LeftDockWidgetArea, mTransformDockWidget );
  else
    QgisApp::instance()->panelMenu()->addAction( mTransformDockWidget->toggleViewAction() );

  mTransformDockWidget->show();

  connect( this, &QgsMapToolEditMeshFrame::selectionChange, mTransformDockWidget, &QgsMeshTransformCoordinatesDockWidget::setInput );

  connect( mTransformDockWidget, &QgsMeshTransformCoordinatesDockWidget::calculationUpdated, this, [this] {
    mMovingFacesRubberband->reset( Qgis::GeometryType::Polygon );
    mMovingEdgesRubberband->reset( Qgis::GeometryType::Line );
    mMovingFreeVertexRubberband->reset( Qgis::GeometryType::Point );
    setMovingRubberBandValidity( mTransformDockWidget->isResultValid() );

    if ( !mCurrentLayer || !mCurrentEditor )
      return;

    const QList<int> faceList = qgis::setToList( mSelectedFaces );
    QgsGeometry faceGeometry;
    if ( faceList.count() == 1 )
    {
      const QgsMeshFace &face = mCurrentLayer->nativeMesh()->face( faceList.at( 0 ) );
      const int faceSize = face.size();
      QVector<QgsPointXY> faceVertices( faceSize );
      for ( int j = 0; j < faceSize; ++j )
        faceVertices[j] = mTransformDockWidget->transformedVertex( face.at( j ) );

      faceGeometry = QgsGeometry::fromPolygonXY( { faceVertices } );
    }
    else
    {
      std::unique_ptr<QgsGeometryEngine> geomEngine( QgsGeometry::createGeometryEngine( faceGeometry.constGet() ) );
      geomEngine->prepareGeometry();
      QVector<QgsGeometry> faces( mSelectedFaces.count() );
      for ( int i = 0; i < faceList.count(); ++i )
      {
        const QgsMeshFace &face = mCurrentLayer->nativeMesh()->face( faceList.at( i ) );
        const int faceSize = face.size();
        QVector<QgsPointXY> faceVertices( faceSize );
        for ( int j = 0; j < faceSize; ++j )
          faceVertices[j] = mTransformDockWidget->transformedVertex( face.at( j ) );

        faces[i] = QgsGeometry::fromPolygonXY( { faceVertices } );
      }
      QString error;
      faceGeometry = QgsGeometry( geomEngine->combine( faces, &error ) );
    }

    QgsGeometry edgesGeom = QgsGeometry::fromMultiPolylineXY( QgsMultiPolylineXY() );
    for ( QMap<int, SelectedVertexData>::const_iterator it = mSelectedVertices.constBegin(); it != mSelectedVertices.constEnd(); ++it )
    {
      const QgsPointXY &point1 = mTransformDockWidget->transformedVertex( it.key() );
      const SelectedVertexData &vertexData = it.value();
      for ( int i = 0; i < vertexData.meshFixedEdges.count(); ++i )
      {
        const QgsPointXY point2 = mTransformDockWidget->transformedVertex( vertexData.meshFixedEdges.at( i ).second );
        QgsGeometry edge( new QgsLineString( { point1, point2 } ) );
        edgesGeom.addPart( edge );
      }

      for ( int i = 0; i < vertexData.borderEdges.count(); ++i )
      {
        const QgsPointXY point2 = mTransformDockWidget->transformedVertex( vertexData.borderEdges.at( i ).second );
        const QgsGeometry edge( new QgsLineString( { point1, point2 } ) );
        edgesGeom.addPart( edge );
      }
    }

    const QgsCoordinateTransform coordinateTransform( mCurrentLayer->crs(), canvas()->mapSettings().destinationCrs(), QgsProject::instance() );

    try
    {
      faceGeometry.transform( coordinateTransform );
    }
    catch ( QgsCsException & )
    {}

    try
    {
      edgesGeom.transform( coordinateTransform );
    }
    catch ( QgsCsException & )
    {}

    mMovingFacesRubberband->setToGeometry( faceGeometry );
    mMovingEdgesRubberband->setToGeometry( edgesGeom );
    const QList<int> vertexIndexes = mSelectedVertices.keys();
    for ( const int vertexIndex : vertexIndexes )
      if ( mCurrentEditor->isVertexFree( vertexIndex ) )
      {
        QgsMeshVertex transformedVertex = mTransformDockWidget->transformedVertex( vertexIndex );
        mMovingFreeVertexRubberband->addPoint( mCurrentLayer->triangularMesh()->nativeToTriangularCoordinates( transformedVertex ), false );
      }

    mMovingFreeVertexRubberband->setVisible( true );
    mMovingFreeVertexRubberband->updatePosition();
    mMovingFreeVertexRubberband->update();

    setMovingRubberBandValidity( mTransformDockWidget->isResultValid() );
  } );

  connect( mTransformDockWidget, &QgsMeshTransformCoordinatesDockWidget::aboutToBeApplied, this, [this] {
    mKeepSelectionOnEdit = true;
  } );

  connect( mTransformDockWidget, &QgsMeshTransformCoordinatesDockWidget::applied, this, [this] {
    mTransformDockWidget->setInput( mCurrentLayer, mSelectedVertices.keys() );
    updateSelectecVerticesMarker();
    prepareSelection();
  } );

  connect( mTransformDockWidget, &QgsDockWidget::closed, this, [this] {
    mActionTransformCoordinates->setChecked( false );
    if ( !mIsInitialized )
      return;
    mMovingFacesRubberband->reset( Qgis::GeometryType::Polygon );
    mMovingEdgesRubberband->reset( Qgis::GeometryType::Line );
    mMovingFreeVertexRubberband->reset( Qgis::GeometryType::Point );
    setMovingRubberBandValidity( false );
  } );
}

void QgsMapToolEditMeshFrame::reindexMesh()
{
  onEditingStarted();

  if ( !mCurrentLayer || !mCurrentLayer->isEditable() )
    return;

  if ( QMessageBox::question( canvas(), tr( "Reindex Mesh" ), tr( "Do you want to reindex the faces and vertices of the mesh layer %1?" ).arg( mCurrentLayer->name() ), QMessageBox::Yes | QMessageBox::No, QMessageBox::No )
       == QMessageBox::No )
    return;


  const QgsCoordinateTransform transform( mCurrentLayer->crs(), canvas()->mapSettings().destinationCrs(), QgsProject::instance() );

  QgsTemporaryCursorOverride waitCursor( Qt::WaitCursor );
  mCurrentLayer->reindex( transform, true );
}

void QgsMapToolEditMeshFrame::onUndoRedo()
{
  switch ( mCurrentState )
  {
    case Digitizing:
      break;
    case AddingNewFace:
      mNewFaceBand->reset( Qgis::GeometryType::Polygon );
      mNewFaceCandidate.clear();
      mNewVerticesForNewFaceCandidate.clear();
      mCurrentState = Digitizing;
      mCadDockWidget->clearPoints();
      break;
    case MovingSelection:
      mCurrentState = Digitizing;
      mMovingEdgesRubberband->reset( Qgis::GeometryType::Line );
      mMovingFacesRubberband->reset( Qgis::GeometryType::Polygon );
      mMovingFreeVertexRubberband->reset( Qgis::GeometryType::Point );
      mCadDockWidget->setEnabledZ( mCadDockWidget->cadEnabled() );
      mCadDockWidget->clearPoints();
      break;
    case ForceByLines:
    case Selecting:
    case SelectingByPolygon:
      break;
  }
}

void QgsMapToolEditMeshFrame::selectByGeometry( const QgsGeometry &geometry, Qt::KeyboardModifiers modifiers )
{
  if ( mCurrentLayer.isNull() || !mCurrentLayer->triangularMesh() || mCurrentEditor.isNull() )
    return;

  Qgis::SelectBehavior behavior;
  if ( modifiers & Qt::ShiftModifier )
    behavior = Qgis::SelectBehavior::AddToSelection;
  else if ( modifiers & Qt::ControlModifier )
    behavior = Qgis::SelectBehavior::RemoveFromSelection;
  else
    behavior = Qgis::SelectBehavior::SetSelection;

  if ( modifiers & Qt::AltModifier )
    selectContainedByGeometry( geometry, behavior );
  else
    selectTouchedByGeometry( geometry, behavior );
}

void QgsMapToolEditMeshFrame::selectTouchedByGeometry( const QgsGeometry &geometry, Qgis::SelectBehavior behavior )
{
  QSet<int> selectedVertices;
  const QList<int> nativeFaceIndexes = mCurrentLayer->triangularMesh()->nativeFaceIndexForRectangle( geometry.boundingBox() );

  std::unique_ptr<QgsGeometryEngine> engine( QgsGeometry::createGeometryEngine( geometry.constGet() ) );
  engine->prepareGeometry();

  for ( const int faceIndex : nativeFaceIndexes )
  {
    const QgsMeshFace &face = nativeFace( faceIndex );
    auto faceGeom = std::make_unique<QgsPolygon>( new QgsLineString( nativeFaceGeometry( faceIndex ) ) );
    if ( engine->intersects( faceGeom.get() ) )
    {
      QSet<int> faceToAdd = qgis::listToSet( face.toList() );
      selectedVertices.unite( faceToAdd );
    }
  }

  const QList<int> &freeVerticesIndexes = mCurrentEditor->freeVerticesIndexes();
  for ( const int freeVertexIndex : freeVerticesIndexes )
  {
    const QgsMeshVertex &vertex = mapVertex( freeVertexIndex );
    if ( engine->contains( &vertex ) )
      selectedVertices.insert( freeVertexIndex );
  }

  setSelectedVertices( selectedVertices.values(), behavior );
}

void QgsMapToolEditMeshFrame::selectContainedByGeometry( const QgsGeometry &geometry, Qgis::SelectBehavior behavior )
{
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

  const QList<int> &freeVerticesIndexes = mCurrentEditor->freeVerticesIndexes();
  for ( const int freeVertexIndex : freeVerticesIndexes )
  {
    const QgsMeshVertex &vertex = mapVertex( freeVertexIndex );
    if ( engine->contains( &vertex ) )
      selectedVertices.insert( freeVertexIndex );
  }

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
  mUserZValue = currentZValue();
  for ( int i = 0; i < mSelectedVertices.count(); ++i )
    zValues.append( mUserZValue );

  mCurrentEditor->changeZValues( mSelectedVertices.keys(), zValues );
}

void QgsMapToolEditMeshFrame::prepareSelection()
{
  if ( !mSelectedVertices.isEmpty() )
  {
    double vertexZValue = 0;
    for ( auto i = mSelectedVertices.keyBegin(); i != mSelectedVertices.keyEnd(); i++ )
      vertexZValue += mapVertex( *i ).z();
    vertexZValue /= mSelectedVertices.count();

    if ( !mZValueWidget )
      createZValueWidget();

    mZValueWidget->setZValue( vertexZValue );
  }
  else
  {
    if ( cadDockWidget()->cadEnabled() && mZValueWidget )
      deleteZValueWidget();
    else if ( mZValueWidget )
      mZValueWidget->setZValue( mUserZValue );
  }

  mConcernedFaceBySelection.clear();

  double xMin = std::numeric_limits<double>::max();
  double xMax = -std::numeric_limits<double>::max();
  double yMin = std::numeric_limits<double>::max();
  double yMax = -std::numeric_limits<double>::max();

  // search for moving edges and mesh fixed edges
  for ( QMap<int, SelectedVertexData>::iterator it = mSelectedVertices.begin(); it != mSelectedVertices.end(); ++it )
  {
    SelectedVertexData &vertexData = it.value();
    int vertexIndex = it.key();

    const QgsPointXY vert = mapVertex( vertexIndex );
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
    const QgsMeshVertexCirculator circulator = mCurrentEditor->vertexCirculator( vertexIndex );

    if ( !circulator.isValid() )
      continue;

    circulator.goBoundaryClockwise();
    int firstface = circulator.currentFaceIndex();
    do
    {
      int oppositeVertex = circulator.oppositeVertexClockwise();
      if ( mSelectedVertices.contains( oppositeVertex ) )
        vertexData.borderEdges.append( { circulator.currentFaceIndex(), oppositeVertex } );
      else
        vertexData.meshFixedEdges.append( { circulator.currentFaceIndex(), oppositeVertex } );

      mConcernedFaceBySelection.insert( circulator.currentFaceIndex() );
    } while ( circulator.turnCounterClockwise() != firstface && circulator.currentFaceIndex() != -1 );

    if ( circulator.currentFaceIndex() == -1 )
    {
      circulator.turnClockwise();
      int oppositeVertex = circulator.oppositeVertexCounterClockwise();
      if ( mSelectedVertices.contains( oppositeVertex ) )
        vertexData.borderEdges.append( { -1, oppositeVertex } );
      else
        vertexData.meshFixedEdges.append( { -1, oppositeVertex } );
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

  const QList<int> facesList = qgis::setToList( mSelectedFaces );
  if ( !facesList.isEmpty() )
  {
    const QgsGeometry faceGeometry( new QgsPolygon( new QgsLineString( nativeFaceGeometry( facesList.at( 0 ) ) ) ) );
    if ( mSelectedFaces.count() == 1 )
    {
      mSelectedFacesRubberband->setToGeometry( faceGeometry );
    }
    else
    {
      std::unique_ptr<QgsGeometryEngine> geomEngine( QgsGeometry::createGeometryEngine( faceGeometry.constGet() ) );
      geomEngine->prepareGeometry();

      QVector<QgsGeometry> otherFaces( mSelectedFaces.count() );
      for ( int i = 0; i < facesList.count(); ++i )
        otherFaces[i] = QgsGeometry( new QgsPolygon( new QgsLineString( nativeFaceGeometry( facesList.at( i ) ) ) ) );
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
    mSelectedFacesRubberband->reset( Qgis::GeometryType::Polygon );

  if ( mSelectedVertices.count() == 1 )
  {
    mActionRemoveVerticesFillingHole->setText( tr( "Remove Selected Vertex and Fill Hole" ) );
    mActionRemoveVerticesWithoutFillingHole->setText( tr( "Remove Selected Vertex without Filling Hole" ) );
  }
  else if ( mSelectedVertices.count() > 1 )
  {
    mActionRemoveVerticesFillingHole->setText( tr( "Remove Selected Vertices and Fill Hole(s)" ) );
    mActionRemoveVerticesWithoutFillingHole->setText( tr( "Remove Selected Vertices without Filling Hole(s)" ) );
  }

  if ( facesList.count() == 1 )
    mActionRemoveFaces->setText( tr( "Remove Selected Face" ) );
  else if ( facesList.count() > 1 )
    mActionRemoveFaces->setText( tr( "Remove %n Selected Face(s)", nullptr, mSelectedFaces.count() ) );
  else
    mActionRemoveFaces->setText( tr( "Remove Current Face" ) );

  mSplittableFaceCount = 0;
  mRefinableFaceCount = 0;
  for ( const int faceIndex : std::as_const( mSelectedFaces ) )
  {
    if ( mCurrentEditor->faceCanBeSplit( faceIndex ) )
      mSplittableFaceCount++;
    int faceSize = nativeFace( faceIndex ).size();
    if ( faceSize == 3 || faceSize == 4 )
      mRefinableFaceCount++;
  }

  if ( mSplittableFaceCount == 1 )
    mActionSplitFaces->setText( tr( "Split Selected Face" ) );
  else if ( mSplittableFaceCount > 1 )
    mActionSplitFaces->setText( tr( "Split %n Selected Face(s)", nullptr, mSplittableFaceCount ) );
  else
    mActionSplitFaces->setText( tr( "Split Current Face" ) );

  if ( mRefinableFaceCount == 1 )
    mActionFacesRefinement->setText( tr( "Refine Selected Face" ) );
  else if ( mRefinableFaceCount > 1 )
    mActionFacesRefinement->setText( tr( "Refine %n Selected Face(s)", nullptr, mRefinableFaceCount ) );
  else
    mActionFacesRefinement->setText( tr( "Refine Current Face" ) );

  emit selectionChange( mCurrentLayer, mSelectedVertices.keys() );
}

void QgsMapToolEditMeshFrame::updateSelectecVerticesMarker()
{
  qDeleteAll( mSelectedVerticesMarker );
  mSelectedVerticesMarker.clear();
  for ( auto it = mSelectedVertices.keyBegin(); it != mSelectedVertices.keyEnd(); it++ )
  {
    const int vertexIndex = *it;
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
    mMovingFreeVertexRubberband->setColor( QColor( 0, 200, 0 ) );
  }
  else
  {
    mMovingFacesRubberband->setFillColor( QColor( 200, 0, 0, 100 ) );
    mMovingFacesRubberband->setStrokeColor( QColor( 200, 0, 0 ) );
    mMovingEdgesRubberband->setColor( QColor( 200, 0, 0 ) );
    mMovingFreeVertexRubberband->setColor( QColor( 200, 0, 0 ) );
  }
}

bool QgsMapToolEditMeshFrame::isSelectionGrapped( QgsPointXY &grappedPoint ) const
{
  if ( mCurrentVertexIndex != -1 && mSelectedVertices.contains( mCurrentVertexIndex ) )
  {
    grappedPoint = mapVertexXY( mCurrentVertexIndex );
    return true;
  }

  double tolerance = QgsTolerance::vertexSearchRadius( canvas()->mapSettings() );

  if ( mCurrentEdge.first != -1 && mCurrentEdge.second != -1 && mSelectEdgeMarker->isVisible() && grappedPoint.distance( mSelectEdgeMarker->center() ) < tolerance )
  {
    const QVector<int> vertices = edgeVertices( mCurrentEdge );
    if ( mSelectedVertices.contains( vertices.at( 0 ) ) && mSelectedVertices.contains( vertices.at( 1 ) ) )
    {
      const QgsPointXY &point1 = mapVertexXY( vertices.at( 0 ) );
      const QgsPointXY &point2 = mapVertexXY( vertices.at( 1 ) );
      grappedPoint = QgsPointXY( point1.x() + point2.x(), point1.y() + point2.y() ) / 2;
      return true;
    }
  }


  if ( ( mSelectFaceMarker->isVisible() && grappedPoint.distance( mSelectFaceMarker->center() ) < tolerance
         && mCurrentFaceIndex >= 0
         && mSelectedFaces.contains( mCurrentFaceIndex ) ) )
  {
    grappedPoint = mCurrentLayer->triangularMesh()->faceCentroids().at( mCurrentFaceIndex );
    return true;
  }

  return false;
}

void QgsMapToolEditMeshFrame::forceByLineReleaseEvent( QgsMapMouseEvent *e )
{
  const QgsPointXY mapPoint = e->mapPoint();

  if ( e->button() == Qt::LeftButton )
  {
    double zValue = currentZValue();

    if ( mCurrentVertexIndex != -1 )
    {
      const QgsPointXY currentPoint = mapVertexXY( mCurrentVertexIndex );
      mForceByLineRubberBand->addPoint( currentPoint );
      mCadDockWidget->setZ( QString::number( mapVertex( mCurrentVertexIndex ).z(), 'f' ), QgsAdvancedDigitizingDockWidget::WidgetSetMode::TextEdited );
      mCadDockWidget->setPoints( QList<QgsPointXY>() << currentPoint << currentPoint );
    }
    else
    {
      if ( e->mapPointMatch().isValid() )
      {
        const QgsPoint layerPoint = e->mapPointMatch().interpolatedPoint( mCanvas->mapSettings().destinationCrs() );
        zValue = layerPoint.z();
      }

      mForceByLineRubberBand->addPoint( mapPoint );
    }

    mForcingLineZValue.append( zValue );
  }
  else if ( e->button() == Qt::RightButton )
  {
    if ( mForceByLineRubberBand->numberOfVertices() == 0 )
    {
      forceByLineBySelectedFeature( e );
      return;
    }

    QVector<QgsPoint> points;
    const QgsPolylineXY rubbergandLines = mForceByLineRubberBand->asGeometry().asPolyline();
    double defaultValue = currentZValue();

    if ( std::isnan( defaultValue ) )
      defaultValue = defaultZValue();

    for ( int i = 0; i < rubbergandLines.count() - 1; ++i )
    {
      points.append( QgsPoint( rubbergandLines.at( i ).x(), rubbergandLines.at( i ).y(), mForcingLineZValue.isEmpty() ? defaultValue : mForcingLineZValue.at( i ) ) );
    }
    auto forcingLine = std::make_unique<QgsLineString>( points );
    forceByLine( QgsGeometry( forcingLine.release() ) );
    mForceByLineRubberBand->reset( Qgis::GeometryType::Line );
    mForcingLineZValue.clear();
  }
}

void QgsMapToolEditMeshFrame::forceByLine( const QgsGeometry &lineGeometry )
{
  QgsMeshEditForceByPolylines forceByPolyline;

  double defaultValue = currentZValue();
  if ( std::isnan( defaultValue ) )
    defaultValue = defaultZValue();

  forceByPolyline.setDefaultZValue( defaultValue );
  forceByPolyline.addLineFromGeometry( lineGeometry );
  forceByPolyline.setAddVertexOnIntersection( mWidgetActionForceByLine->newVertexOnIntersectingEdge() );
  forceByPolyline.setInterpolateZValueOnMesh( mWidgetActionForceByLine->interpolationMode() == QgsMeshEditForceByLineAction::Mesh );
  forceByPolyline.setTolerance( mWidgetActionForceByLine->toleranceValue() );

  mCurrentEditor->advancedEdit( &forceByPolyline );
}


void QgsMapToolEditMeshFrame::highlightCurrentHoveredFace( const QgsPointXY &mapPoint )
{
  searchFace( mapPoint );
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

  const QgsPointSequence faceGeometry = nativeFaceGeometry( mCurrentFaceIndex );
  mFaceRubberBand->reset( Qgis::GeometryType::Polygon );
  mFaceVerticesBand->reset( Qgis::GeometryType::Point );
  for ( const QgsPoint &pt : faceGeometry )
  {
    mFaceRubberBand->addPoint( pt );
    mFaceVerticesBand->addPoint( pt );
  }

  if ( mCurrentFaceIndex != -1 && faceCanBeInteractive( mCurrentFaceIndex ) )
  {
    mSelectFaceMarker->setCenter( mCurrentLayer->triangularMesh()->faceCentroids().at( mCurrentFaceIndex ) );
    mSelectFaceMarker->setVisible( true );
  }
  else
    mSelectFaceMarker->setVisible( false );
}

void QgsMapToolEditMeshFrame::highlightCloseVertex( const QgsPointXY &mapPoint )
{
  if ( !mCurrentEditor )
    return;

  if ( mCurrentState == Digitizing && mNewFaceMarker->isVisible() )
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
      mVertexBand->reset( Qgis::GeometryType::Point );
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
    mVertexBand->reset( Qgis::GeometryType::Point );

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

  if ( mCadDockWidget->cadEnabled() && !mCadDockWidget->constraintZ()->isLocked() && mCurrentVertexIndex != -1 )
    mCadDockWidget->setZ( QString::number( mapVertex( mCurrentVertexIndex ).z(), 'f' ), QgsAdvancedDigitizingDockWidget::WidgetSetMode::TextEdited );
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

  searchEdge( mapPoint );

  if ( mMergeFaceMarker->isVisible() )
  {
    if ( mapPoint.distance( mMergeFaceMarker->center() ) < tolerance )
      mMergeFaceMarker->setColor( Qt::red );
    else
      mMergeFaceMarker->setColor( Qt::gray );
  }

  mEdgeBand->reset();
  mFlipEdgeMarker->setVisible( false );
  mMergeFaceMarker->setVisible( false );
  mSelectEdgeMarker->setVisible( false );
  if ( mCurrentEdge.first != -1 && mCurrentEdge.second != -1 && mCurrentState == Digitizing )
  {
    const QVector<QgsPointXY> &edgeGeom = edgeGeometry( mCurrentEdge );
    mEdgeBand->addPoint( edgeGeom.at( 0 ) );
    mEdgeBand->addPoint( edgeGeom.at( 1 ) );

    if ( mCurrentFaceIndex == -1 )
    {
      mFaceVerticesBand->reset( Qgis::GeometryType::Point );
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
  // avoid the user to click on a marker if he doesn't want
  double mapUnitPerPixel = mCanvas->mapSettings().mapUnitsPerPixel();
  return mapVertexXY( vertexIndex1 ).distance( mapVertexXY( vertexIndex2 ) ) / mapUnitPerPixel > 90;
}

bool QgsMapToolEditMeshFrame::faceCanBeInteractive( int faceIndex ) const
{
  // If both side of the face boundng box is less than 60px width, the interactive marker will not be displayed to avoid too close marker and
  // avoid the user to click on a marker if he doesn't want

  double mapUnitPerPixel = mCanvas->mapSettings().mapUnitsPerPixel();
  const QgsGeometry faceGeom( new QgsLineString( nativeFaceGeometry( faceIndex ) ) );
  QgsRectangle bbox = faceGeom.boundingBox();

  return bbox.width() / mapUnitPerPixel > 60 || bbox.height() / mapUnitPerPixel > 60;
}

void QgsMapToolEditMeshFrame::createZValueWidget()
{
  if ( !mCanvas )
    return;

  deleteZValueWidget();

  mZValueWidget = new QgsZValueWidget( tr( "Vertex Z value:" ) );
  mZValueWidget->setDefaultValue( defaultZValue() );
  mZValueWidget->setZValue( mUserZValue );
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
  mSelectionBand->reset( Qgis::GeometryType::Polygon );
  mSelectedFacesRubberband->reset( Qgis::GeometryType::Polygon );
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
  mNewFaceBand->reset();

  mSnapIndicator->setMatch( QgsPointLocator::Match() );
  mSelectFaceMarker->setVisible( false );
  clearEdgeHelpers();
}

void QgsMapToolEditMeshFrame::clearEdgeHelpers()
{
  mCurrentEdge = { -1, -1 };
  mEdgeBand->reset();
  mSelectEdgeMarker->setVisible( false );
  mFlipEdgeMarker->setVisible( false );
  mMergeFaceMarker->setVisible( false );
}

void QgsMapToolEditMeshFrame::addVertex(
  const QgsPointXY &mapPoint,
  const QgsPointLocator::Match &mapPointMatch
)
{
  QgsTemporaryCursorOverride waitCursor( Qt::WaitCursor );

  double zValue;
  QgsPointXY effectivePoint = mapPoint;

  // initial Z value that can be modified
  zValue = currentZValue();

  bool isOnMesh = mCurrentFaceIndex != -1 || ( mCurrentEdge.first != -1 && mCurrentEdge.second != -1 );

  if ( mWidgetActionDigitizing->zValueSourceType() == QgsMeshEditDigitizingAction::Terrain || ( mWidgetActionDigitizing->zValueSourceType() == QgsMeshEditDigitizingAction::PreferMeshThenTerrain && !isOnMesh ) )
  {
    const QgsAbstractTerrainProvider *terrainProvider = QgsProject::instance()->elevationProperties()->terrainProvider();
    const QgsCoordinateTransform transformation = QgsCoordinateTransform( mCurrentLayer->crs(), terrainProvider->crs(), QgsProject::instance() );

    try
    {
      const QgsPointXY point = transformation.transform( effectivePoint.x(), effectivePoint.y() );
      zValue = terrainProvider->heightAt( point.x(), point.y() );
    }
    catch ( const QgsCsException & )
    {
      zValue = std::numeric_limits<double>::quiet_NaN();
    }

    // either outside of terrain or the point cannot be transformed to terrainProvider CRS, use currentZValue
    if ( std::isnan( zValue ) )
    {
      QgisApp::instance()->messageBar()->pushMessage(
        tr( "Terrain Z Value" ),
        tr( "Z Value from project terrain could not be obtained, setting default value %1." ).arg( mZValueWidget->getDefaultValue() ),
        Qgis::MessageLevel::Warning
      );

      zValue = mZValueWidget->getDefaultValue();
    }
  }
  else if ( isOnMesh && ( mWidgetActionDigitizing->zValueSourceType() == QgsMeshEditDigitizingAction::PreferMeshThenZWidget || mWidgetActionDigitizing->zValueSourceType() == QgsMeshEditDigitizingAction::PreferMeshThenTerrain ) )
  {
    if ( mCurrentEdge.first != -1 && mCurrentEdge.second != -1 ) //we are on a edge -->interpolate the z value
    {
      const QVector<int> &edge = edgeVertices( mCurrentEdge );
      const QgsTriangularMesh &triangularMesh = *mCurrentLayer->triangularMesh();
      const QgsMeshVertex &v1 = triangularMesh.vertices().at( edge.at( 0 ) );
      const QgsMeshVertex &v2 = triangularMesh.vertices().at( edge.at( 1 ) );
      const QgsPoint projectedPoint = QgsGeometryUtils::projectPointOnSegment( QgsPoint( mapPoint ), v1, v2 );
      zValue = v1.z() + ( v2.z() - v1.z() ) * projectedPoint.distance( v1 ) / v1.distance( v2 );
      effectivePoint = QgsPointXY( projectedPoint );
    }
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
  }
  else
  {
    if ( mapPointMatch.isValid() && mapPointMatch.layer() && QgsWkbTypes::hasZ( mapPointMatch.layer()->wkbType() ) )
    {
      const QgsPoint layerPoint = mapPointMatch.interpolatedPoint( mCanvas->mapSettings().destinationCrs() );
      zValue = layerPoint.z();
    }
  }

  const QVector<QgsMeshVertex> points( 1, QgsMeshVertex( effectivePoint.x(), effectivePoint.y(), zValue ) );

  if ( mCurrentEditor )
  {
    double tolerance = QgsTolerance::vertexSearchRadius( canvas()->mapSettings() );
    if ( mWidgetActionDigitizing->refineNeighboringFaces() && mCurrentFaceIndex != -1 )
    {
      mCurrentEditor->addVertexWithDelaunayRefinement( points.first(), tolerance );
    }
    else
    {
      mCurrentEditor->addVertices( points, tolerance );
    }
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

  const QColor freeVertexColor = digitizingStrokeColor();
  const QColor fillFreeVertexColor = freeVertexColor.lighter();

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

  if ( mCurrentEdge.first != -1 && mCurrentEdge.second != -1 )
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

  QgsDistanceArea distArea;
  distArea.setSourceCrs( mCurrentLayer->crs(), QgsProject::instance()->transformContext() );
  distArea.setEllipsoid( QgsProject::instance()->ellipsoid() );
  expression.setAreaUnits( QgsProject::instance()->areaUnits() );
  expression.setDistanceUnits( QgsProject::instance()->distanceUnits() );
  expression.setGeomCalculator( &distArea );

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

void QgsMapToolEditMeshFrame::setZValueSourceType( QgsMeshEditDigitizingAction::ZValueSource zValueSource )
{
  mWidgetActionDigitizing->setZValueType( zValueSource );
}

void QgsMapToolEditMeshFrame::updateStatusBarMessage() const
{
  if ( !mSelectedVertices.isEmpty() )
  {
    QString message;
    if ( mSelectedVertices.count() == 1 )
    {
      const QgsMesh &mesh = *mCurrentLayer->nativeMesh();
      const int vertexId = mSelectedVertices.firstKey();
      const QgsMeshVertex vertex = mesh.vertex( vertexId );

      message = tr( "Selected mesh vertex ID: %1 at x: %2 y: %3 z: %4." ).arg( vertexId ).arg( QLocale().toString( vertex.x(), 'f' ) ).arg( QLocale().toString( vertex.y(), 'f' ) ).arg( QLocale().toString( vertex.z(), 'f' ) );
    }
    else if ( mSelectedVertices.count() == 2 )
    {
      const QgsMesh &mesh = *mCurrentLayer->nativeMesh();
      const int vertexId1 = mSelectedVertices.firstKey();
      const int vertexId2 = mSelectedVertices.lastKey();
      const QgsMeshVertex vertex1 = mesh.vertex( vertexId1 );
      const QgsMeshVertex vertex2 = mesh.vertex( vertexId2 );

      QString formattedDistance;
      double distance;

      // if crs is valid calculate using QgsDistanceArea otherwise calculate just as distance
      if ( mCurrentLayer->crs().isValid() )
      {
        QgsDistanceArea distArea = QgsDistanceArea();
        distArea.setSourceCrs( mCurrentLayer->crs(), QgsProject::instance()->transformContext() );
        distArea.setEllipsoid( QgsProject::instance()->ellipsoid() );
        try
        {
          distance = distArea.measureLine( QgsPointXY( vertex1 ), QgsPointXY( vertex2 ) );
          distance = distArea.convertLengthMeasurement( distance, QgsProject::instance()->distanceUnits() );
          formattedDistance = distArea.formatDistance( distance, 6, QgsProject::instance()->distanceUnits() );
        }
        catch ( QgsCsException & )
        {}
      }

      if ( formattedDistance.isEmpty() )
      {
        distance = vertex1.distance( vertex2 );
        formattedDistance = QLocale().toString( distance, 'f' );
      }

      const double zDiff = vertex2.z() - vertex1.z();

      message = tr( "Selected mesh vertices IDs: %1 and %2 with distance %3 and dZ %4." ).arg( vertexId1 ).arg( vertexId2 ).arg( formattedDistance ).arg( QLocale().toString( zDiff, 'f' ) );
    }
    else if ( mSelectedVertices.count() > 2 )
    {
      message = tr( "Selected %1 mesh vertices." ).arg( mSelectedVertices.count() );
    }

    QgisApp::instance()->statusBarIface()->showMessage( message );
  }
  else
  {
    QgisApp::instance()->statusBarIface()->clearMessage();
  }
}
