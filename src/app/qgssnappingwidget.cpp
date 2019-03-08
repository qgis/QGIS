/***************************************************************************
                         qgssnappingwidget.cpp
    begin                : August 2016
    copyright            : (C) 2016 Denis Rouzaud
    email                : denis.rouzaud@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QAction>
#include <QComboBox>
#include <QFont>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QToolBar>
#include <QToolButton>
#include <QWidgetAction>

#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgsdoublespinbox.h"
#include "qgsfloatingwidget.h"
#include "qgslayertreegroup.h"
#include "qgslayertree.h"
#include "qgslayertreeview.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsproject.h"
#include "qgssnappingconfig.h"
#include "qgssnappinglayertreemodel.h"
#include "qgssnappingwidget.h"
#include "qgsunittypes.h"
#include "qgssettings.h"


QgsSnappingWidget::QgsSnappingWidget( QgsProject *project, QgsMapCanvas *canvas, QWidget *parent )
  : QWidget( parent )
  , mProject( project )
  , mConfig( project )
  , mCanvas( canvas )
{
  // detect the type of display
  QToolBar *tb = qobject_cast<QToolBar *>( parent );
  if ( tb )
  {
    mDisplayMode = ToolBar;
    setObjectName( QStringLiteral( "SnappingOptionToolBar" ) );
  }
  else
  {
    mDisplayMode = Widget;
    setObjectName( QStringLiteral( "SnappingOptionDialog" ) );
  }

  // Advanced config layer tree view
  mAdvancedConfigWidget = new QWidget( this );
  QVBoxLayout *advancedLayout = new QVBoxLayout();
  if ( mDisplayMode == Widget )
    advancedLayout->setContentsMargins( 0, 0, 0, 0 );
  // tree view
  mLayerTreeView = new QTreeView();
  QgsSnappingLayerTreeModel *model = new QgsSnappingLayerTreeModel( mProject, this );
  model->setLayerTreeModel( new QgsLayerTreeModel( mProject->layerTreeRoot(), model ) );
  // connections
  connect( model, &QgsSnappingLayerTreeModel::rowsInserted, this, &QgsSnappingWidget::onSnappingTreeLayersChanged );
  connect( model, &QgsSnappingLayerTreeModel::modelReset, this, &QgsSnappingWidget::onSnappingTreeLayersChanged );
  connect( model, &QgsSnappingLayerTreeModel::rowsRemoved, this, &QgsSnappingWidget::onSnappingTreeLayersChanged );
  connect( mProject, &QgsProject::readProject, this, [ = ] {model->resetLayerTreeModel();} );
  connect( mProject, &QObject::destroyed, this, [ = ] {mLayerTreeView->setModel( nullptr );} );
  // model->setFlags( 0 );
  mLayerTreeView->setModel( model );
  mLayerTreeView->resizeColumnToContents( 0 );
  mLayerTreeView->header()->show();
  mLayerTreeView->setSelectionMode( QAbstractItemView::NoSelection );
  // item delegates
  mLayerTreeView->setEditTriggers( QAbstractItemView::AllEditTriggers );
  mLayerTreeView->setItemDelegate( new QgsSnappingLayerDelegate( mCanvas, this ) );
  mLayerTreeView->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding );
  mLayerTreeView->setMinimumWidth( 500 );
  mLayerTreeView->resizeColumnToContents( 0 );
  // filter line edit
  QHBoxLayout *filterLayout = new QHBoxLayout();
  filterLayout->setContentsMargins( 0, 0, 0, 0 );
  filterLayout->addStretch();
  QgsFilterLineEdit *filterLineEdit = new QgsFilterLineEdit();
  filterLineEdit->setShowClearButton( true );
  filterLineEdit->setShowSearchIcon( true );
  filterLineEdit->setPlaceholderText( tr( "Filter layers…" ) );
  connect( filterLineEdit, &QgsFilterLineEdit::textChanged, model, &QgsSnappingLayerTreeModel::setFilterText );
  filterLayout->addStretch();
  filterLayout->addWidget( filterLineEdit );
  advancedLayout->addWidget( mLayerTreeView );
  advancedLayout->addLayout( filterLayout );
  mAdvancedConfigWidget->setLayout( advancedLayout );

  // enable button
  mEnabledAction = new QAction( tr( "Toggle Snapping" ), this );
  mEnabledAction->setCheckable( true );
  mEnabledAction->setIcon( QIcon( QgsApplication::getThemeIcon( "/mIconSnapping.svg" ) ) );
  mEnabledAction->setToolTip( tr( "Enable Snapping (S)" ) );
  mEnabledAction->setShortcut( tr( "S", "Keyboard shortcut: toggle snapping" ) );
  mEnabledAction->setObjectName( QStringLiteral( "EnableSnappingAction" ) );
  connect( mEnabledAction, &QAction::toggled, this, &QgsSnappingWidget::enableSnapping );

  // mode button
  mModeButton = new QToolButton();
  mModeButton->setToolTip( tr( "Snapping Mode" ) );
  mModeButton->setPopupMode( QToolButton::InstantPopup );
  QMenu *modeMenu = new QMenu( tr( "Set Snapping Mode" ), this );
  mAllLayersAction = new QAction( QIcon( QgsApplication::getThemeIcon( "/mIconSnappingAllLayers.svg" ) ), tr( "All Layers" ), modeMenu );
  mActiveLayerAction = new QAction( QIcon( QgsApplication::getThemeIcon( "/mIconSnappingActiveLayer.svg" ) ), tr( "Active Layer" ), modeMenu );
  mAdvancedModeAction = new QAction( QIcon( QgsApplication::getThemeIcon( "/mIconSnappingAdvanced.svg" ) ), tr( "Advanced Configuration" ), modeMenu );
  modeMenu->addAction( mAllLayersAction );
  modeMenu->addAction( mActiveLayerAction );
  modeMenu->addAction( mAdvancedModeAction );
  if ( mDisplayMode == ToolBar )
  {
    modeMenu->addSeparator();
    QAction *openDialogAction = new QAction( tr( "Open Snapping Options…" ), modeMenu );
    connect( openDialogAction, &QAction::triggered, QgisApp::instance(), &QgisApp::snappingOptions );
    modeMenu->addAction( openDialogAction );
  }
  mModeButton->setMenu( modeMenu );
  mModeButton->setObjectName( QStringLiteral( "SnappingModeButton" ) );
  if ( mDisplayMode == Widget )
  {
    mModeButton->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );
  }
  connect( mModeButton, &QToolButton::triggered, this, &QgsSnappingWidget::modeButtonTriggered );

  // type button
  mTypeButton = new QToolButton();
  mTypeButton->setToolTip( tr( "Snapping Type" ) );
  mTypeButton->setPopupMode( QToolButton::InstantPopup );
  QMenu *typeMenu = new QMenu( tr( "Set Snapping Mode" ), this );
  mVertexAction = new QAction( QIcon( QgsApplication::getThemeIcon( "/mIconSnappingVertex.svg" ) ), tr( "Vertex" ), typeMenu );
  mVertexAndSegmentAction = new QAction( QIcon( QgsApplication::getThemeIcon( "/mIconSnappingVertexAndSegment.svg" ) ), tr( "Vertex and Segment" ), typeMenu );
  mSegmentAction = new QAction( QIcon( QgsApplication::getThemeIcon( "/mIconSnappingSegment.svg" ) ), tr( "Segment" ), typeMenu );
  typeMenu->addAction( mVertexAction );
  typeMenu->addAction( mVertexAndSegmentAction );
  typeMenu->addAction( mSegmentAction );
  mTypeButton->setMenu( typeMenu );
  mTypeButton->setObjectName( QStringLiteral( "SnappingTypeButton" ) );
  if ( mDisplayMode == Widget )
  {
    mTypeButton->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );
  }
  connect( mTypeButton, &QToolButton::triggered, this, &QgsSnappingWidget::typeButtonTriggered );

  // tolerance
  mToleranceSpinBox = new QDoubleSpinBox();
  mToleranceSpinBox->setDecimals( 5 );
  mToleranceSpinBox->setMaximum( 99999999.990000 );
  mToleranceSpinBox->setToolTip( tr( "Snapping Tolerance in Defined Units" ) );
  mToleranceSpinBox->setObjectName( QStringLiteral( "SnappingToleranceSpinBox" ) );
  connect( mToleranceSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsSnappingWidget::changeTolerance );

  // units
  mUnitsComboBox = new QComboBox();
  mUnitsComboBox->addItem( tr( "px" ), QgsTolerance::Pixels );
  mUnitsComboBox->addItem( QgsUnitTypes::toString( mProject->distanceUnits() ), QgsTolerance::ProjectUnits );
  mUnitsComboBox->setToolTip( tr( "Snapping Unit Type: Pixels (px) or Map Units (mu)" ) );
  mUnitsComboBox->setObjectName( QStringLiteral( "SnappingUnitComboBox" ) );
  connect( mUnitsComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsSnappingWidget::changeUnit );

  // topological editing button
  mTopologicalEditingAction = new QAction( tr( "Topological Editing" ), this );
  mTopologicalEditingAction->setCheckable( true );
  mTopologicalEditingAction->setIcon( QIcon( QgsApplication::getThemeIcon( "/mIconTopologicalEditing.svg" ) ) );
  mTopologicalEditingAction->setToolTip( tr( "Enable Topological Editing" ) );
  mTopologicalEditingAction->setObjectName( QStringLiteral( "TopologicalEditingAction" ) );
  connect( mTopologicalEditingAction, &QAction::toggled, this, &QgsSnappingWidget::enableTopologicalEditing );

  // snapping on intersection button
  mIntersectionSnappingAction = new QAction( tr( "Snapping on Intersection" ), this );
  mIntersectionSnappingAction->setCheckable( true );
  mIntersectionSnappingAction->setIcon( QIcon( QgsApplication::getThemeIcon( "/mIconSnappingIntersection.svg" ) ) );
  mIntersectionSnappingAction->setToolTip( tr( "Enable Snapping on Intersection" ) );
  mIntersectionSnappingAction->setObjectName( QStringLiteral( "IntersectionSnappingAction" ) );
  connect( mIntersectionSnappingAction, &QAction::toggled, this, &QgsSnappingWidget::enableIntersectionSnapping );

  // snapping on intersection button
  mEnableTracingAction = new QAction( tr( "Enable Tracing" ), this );
  mEnableTracingAction->setCheckable( true );
  mEnableTracingAction->setIcon( QIcon( QgsApplication::getThemeIcon( "/mActionTracing.svg" ) ) );
  mEnableTracingAction->setToolTip( tr( "Enable Tracing (T)" ) );
  mEnableTracingAction->setShortcut( tr( "T", "Keyboard shortcut: Enable tracing" ) );
  mEnableTracingAction->setObjectName( QStringLiteral( "EnableTracingAction" ) );

  // tracing offset
  mTracingOffsetSpinBox = new QgsDoubleSpinBox;
  mTracingOffsetSpinBox->setRange( -1000000, 1000000 );
  mTracingOffsetSpinBox->setDecimals( 6 );
  mTracingOffsetSpinBox->setClearValue( 0 );
  mTracingOffsetSpinBox->setClearValueMode( QgsDoubleSpinBox::ClearValueMode::CustomValue );
  // Note: set to false to "fix" a crash
  // See https://github.com/qgis/QGIS/pull/8266 for some more context
  mTracingOffsetSpinBox->setShowClearButton( false );
  QMenu *tracingMenu = new QMenu( this );
  QWidgetAction *widgetAction = new QWidgetAction( tracingMenu );
  QVBoxLayout *tracingWidgetLayout = new QVBoxLayout;
  tracingWidgetLayout->addWidget( new QLabel( "Offset" ) );
  tracingWidgetLayout->addWidget( mTracingOffsetSpinBox );
  QWidget *tracingWidget = new QWidget;
  tracingWidget->setLayout( tracingWidgetLayout );
  widgetAction->setDefaultWidget( tracingWidget );
  tracingMenu->addAction( widgetAction );
  mEnableTracingAction->setMenu( tracingMenu );

  // layout
  if ( mDisplayMode == ToolBar )
  {
    // hiding widget in a toolbar is not possible, actions are required
    tb->addAction( mEnabledAction );
    mModeAction = tb->addWidget( mModeButton );

    // edit advanced config button
    QToolButton *advConfigButton = new QToolButton( this );
    advConfigButton->setPopupMode( QToolButton::InstantPopup );
    QMenu *advConfigMenu = new QMenu( this );
    QWidgetAction *advConfigWidgetAction = new QWidgetAction( advConfigMenu );
    advConfigWidgetAction->setDefaultWidget( mAdvancedConfigWidget );
    advConfigMenu->addAction( advConfigWidgetAction );
    advConfigButton->setIcon( QIcon( QgsApplication::getThemeIcon( "/mActionShowAllLayers.svg" ) ) );
    advConfigButton->setToolTip( tr( "Edit advanced configuration" ) );
    advConfigButton->setObjectName( QStringLiteral( "EditAdvancedConfigurationButton" ) );
    advConfigButton->setMenu( advConfigMenu );
    mEditAdvancedConfigAction = tb->addWidget( advConfigButton );

    // other buttons / actions
    mTypeAction = tb->addWidget( mTypeButton );
    mToleranceAction = tb->addWidget( mToleranceSpinBox );
    mUnitAction = tb->addWidget( mUnitsComboBox );
    tb->addAction( mTopologicalEditingAction );
    tb->addAction( mIntersectionSnappingAction );
    tb->addAction( mEnableTracingAction );
  }
  else
  {
    // mode = widget
    QHBoxLayout *layout = new QHBoxLayout();

    QToolButton *enabledButton = new QToolButton();
    enabledButton->addAction( mEnabledAction );
    enabledButton->setDefaultAction( mEnabledAction );
    layout->addWidget( enabledButton );

    layout->addWidget( mModeButton );
    layout->addWidget( mTypeButton );
    layout->addWidget( mToleranceSpinBox );
    layout->addWidget( mUnitsComboBox );

    QToolButton *topoButton = new QToolButton();
    topoButton->addAction( mTopologicalEditingAction );
    topoButton->setDefaultAction( mTopologicalEditingAction );
    topoButton->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );
    layout->addWidget( topoButton );

    QToolButton *interButton = new QToolButton();
    interButton->addAction( mIntersectionSnappingAction );
    interButton->setDefaultAction( mIntersectionSnappingAction );
    interButton->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );
    layout->addWidget( interButton );

    layout->setContentsMargins( 0, 0, 0, 0 );
    layout->setAlignment( Qt::AlignRight );
    layout->setSpacing( mDisplayMode == Widget ? 3 : 0 );

    QGridLayout *topLayout = new QGridLayout();
    topLayout->addLayout( layout, 0, 0, Qt::AlignLeft | Qt::AlignTop );
    topLayout->addWidget( mAdvancedConfigWidget, 1, 0 );
    setLayout( topLayout );
  }

  // connect settings changed and map units changed to properly update the widget
  connect( project, &QgsProject::snappingConfigChanged, this, &QgsSnappingWidget::projectSnapSettingsChanged );
  connect( project, &QgsProject::topologicalEditingChanged, this, &QgsSnappingWidget::projectTopologicalEditingChanged );
  connect( mCanvas, &QgsMapCanvas::destinationCrsChanged, this, &QgsSnappingWidget::updateToleranceDecimals );

  // Slightly modify the config so the settings changed code doesn't early exit
  mConfig = project->snappingConfig();
  mConfig.setEnabled( !mConfig.enabled() );
  projectSnapSettingsChanged();

  // modeChanged determines if widget are visible or not based on mode
  modeChanged();
  updateToleranceDecimals();

  bool defaultSnapEnabled = QgsSettings().value( QStringLiteral( "/qgis/digitizing/default_snap_enabled" ), false ).toBool();
  enableSnapping( defaultSnapEnabled );

  restoreGeometry( QgsSettings().value( QStringLiteral( "/Windows/SnappingWidget/geometry" ) ).toByteArray() );
}

QgsSnappingWidget::~QgsSnappingWidget()
{
  if ( mDisplayMode == Widget )
  {
    QgsSettings().setValue( QStringLiteral( "/Windows/SnappingWidget/geometry" ), saveGeometry() );
  }
}

void QgsSnappingWidget::projectSnapSettingsChanged()
{
  QgsSnappingConfig config = mProject->snappingConfig();
  if ( mConfig == config )
    return;
  mConfig = config;

  mEnabledAction->setChecked( config.enabled() );

  if ( config.mode() == QgsSnappingConfig::AllLayers && mModeButton->defaultAction() != mAllLayersAction )
  {
    mModeButton->setDefaultAction( mAllLayersAction );
    modeChanged();
    updateToleranceDecimals();
  }
  if ( config.mode() == QgsSnappingConfig::ActiveLayer && mModeButton->defaultAction() != mActiveLayerAction )
  {
    mModeButton->setDefaultAction( mActiveLayerAction );
    modeChanged();
    updateToleranceDecimals();
  }
  if ( config.mode() == QgsSnappingConfig::AdvancedConfiguration && mModeButton->defaultAction() != mAdvancedModeAction )
  {
    mModeButton->setDefaultAction( mAdvancedModeAction );
    modeChanged();
    updateToleranceDecimals();
  }

  if ( config.type() == QgsSnappingConfig::Vertex && mTypeButton->defaultAction() != mVertexAction )
  {
    mTypeButton->setDefaultAction( mVertexAction );
  }
  if ( config.type() == QgsSnappingConfig::VertexAndSegment && mTypeButton->defaultAction() != mVertexAndSegmentAction )
  {
    mTypeButton->setDefaultAction( mVertexAndSegmentAction );
  }
  if ( config.type() == QgsSnappingConfig::Segment && mTypeButton->defaultAction() != mSegmentAction )
  {
    mTypeButton->setDefaultAction( mSegmentAction );
  }

  if ( static_cast<QgsTolerance::UnitType>( mUnitsComboBox->currentData().toInt() ) != config.units() )
  {
    mUnitsComboBox->setCurrentIndex( mUnitsComboBox->findData( config.units() ) );
  }

  if ( mToleranceSpinBox->value() != config.tolerance() )
  {
    mToleranceSpinBox->setValue( config.tolerance() );
  }

  if ( config.intersectionSnapping() != mIntersectionSnappingAction->isChecked() )
  {
    mIntersectionSnappingAction->setChecked( config.intersectionSnapping() );
  }

  toggleSnappingWidgets( config.enabled() );
}

void QgsSnappingWidget::projectTopologicalEditingChanged()
{
  if ( mProject->topologicalEditing() != mTopologicalEditingAction->isChecked() )
  {
    mTopologicalEditingAction->setChecked( mProject->topologicalEditing() );
  }
}

void QgsSnappingWidget::enableSnapping( bool checked )
{
  toggleSnappingWidgets( checked );
  mConfig.setEnabled( checked );
  mProject->setSnappingConfig( mConfig );
}

void QgsSnappingWidget::toggleSnappingWidgets( bool enabled )
{
  mModeButton->setEnabled( enabled );
  mTypeButton->setEnabled( enabled );
  mToleranceSpinBox->setEnabled( enabled );
  mUnitsComboBox->setEnabled( enabled );
  if ( mAdvancedConfigWidget )
  {
    mAdvancedConfigWidget->setEnabled( enabled );
  }
  mIntersectionSnappingAction->setEnabled( enabled );
  mEnableTracingAction->setEnabled( enabled );
}

void QgsSnappingWidget::changeTolerance( double tolerance )
{
  mConfig.setTolerance( tolerance );
  mProject->setSnappingConfig( mConfig );
}

void QgsSnappingWidget::changeUnit( int idx )
{
  QgsTolerance::UnitType unit = static_cast<QgsTolerance::UnitType>( mUnitsComboBox->itemData( idx ).toInt() );
  mConfig.setUnits( unit );
  mProject->setSnappingConfig( mConfig );

  updateToleranceDecimals();
}

void QgsSnappingWidget::enableTopologicalEditing( bool enabled )
{
  mProject->setTopologicalEditing( enabled );
}

void QgsSnappingWidget::enableIntersectionSnapping( bool enabled )
{
  mConfig.setIntersectionSnapping( enabled );
  mProject->setSnappingConfig( mConfig );
}

void QgsSnappingWidget::onSnappingTreeLayersChanged()
{
  mLayerTreeView->expandAll();
  mLayerTreeView->resizeColumnToContents( 0 );
}

void QgsSnappingWidget::modeButtonTriggered( QAction *action )
{
  if ( action != mAllLayersAction &&
       action != mActiveLayerAction &&
       action != mAdvancedModeAction )
  {
    return;
  }

  if ( action != mModeButton->defaultAction() )
  {
    mModeButton->setDefaultAction( action );
    if ( action == mAllLayersAction )
    {
      mConfig.setMode( QgsSnappingConfig::AllLayers );
    }
    else if ( action == mActiveLayerAction )
    {
      mConfig.setMode( QgsSnappingConfig::ActiveLayer );
    }
    else if ( action == mAdvancedModeAction )
    {
      mConfig.setMode( QgsSnappingConfig::AdvancedConfiguration );
    }
    mProject->setSnappingConfig( mConfig );
    updateToleranceDecimals();
    modeChanged();
  }
}

void QgsSnappingWidget::typeButtonTriggered( QAction *action )
{
  if ( action != mTypeButton->defaultAction() )
  {
    mTypeButton->setDefaultAction( action );
    if ( action == mVertexAction )
    {
      mConfig.setType( QgsSnappingConfig::Vertex );
    }
    else if ( action == mVertexAndSegmentAction )
    {
      mConfig.setType( QgsSnappingConfig::VertexAndSegment );
    }
    else if ( action == mSegmentAction )
    {
      mConfig.setType( QgsSnappingConfig::Segment );
    }
    mProject->setSnappingConfig( mConfig );
  }
}

void QgsSnappingWidget::updateToleranceDecimals()
{
  if ( mConfig.units() == QgsTolerance::Pixels )
  {
    mToleranceSpinBox->setDecimals( 0 );
  }
  else
  {
    QgsUnitTypes::DistanceUnit mapUnit = mCanvas->mapUnits();
    QgsUnitTypes::DistanceUnitType type = QgsUnitTypes::unitType( mapUnit );
    if ( type == QgsUnitTypes::Standard )
    {
      mToleranceSpinBox->setDecimals( 2 );
    }
    else
    {
      mToleranceSpinBox->setDecimals( 5 );
    }
  }
}

void QgsSnappingWidget::modeChanged()
{
  bool advanced = mConfig.mode() == QgsSnappingConfig::AdvancedConfiguration;

  if ( mDisplayMode == ToolBar )
  {
    mTypeAction->setVisible( !advanced );
    mToleranceAction->setVisible( !advanced );
    mUnitAction->setVisible( !advanced );
    mEditAdvancedConfigAction->setVisible( advanced );
  }
  else
  {
    mTypeButton->setVisible( !advanced );
    mToleranceSpinBox->setVisible( !advanced );
    mUnitsComboBox->setVisible( !advanced );
    if ( mDisplayMode == Widget && mAdvancedConfigWidget )
    {
      mAdvancedConfigWidget->setVisible( advanced );
    }
  }
}

QgsSnappingConfig QgsSnappingWidget::config() const
{
  return mConfig;
}

void QgsSnappingWidget::setConfig( const QgsSnappingConfig &config )
{
  if ( mConfig == config )
    return;

  mConfig = config;
}



void QgsSnappingWidget::cleanGroup( QgsLayerTreeNode *node )
{
  QgsLayerTreeGroup *group = QgsLayerTree::isGroup( node ) ? QgsLayerTree::toGroup( node ) : nullptr;
  if ( !group )
    return;

  QList<QgsLayerTreeNode *> toRemove;
  Q_FOREACH ( QgsLayerTreeNode *child, node->children() )
  {
    if ( QgsLayerTree::isLayer( child ) && ( !QgsLayerTree::toLayer( child )->layer() || QgsLayerTree::toLayer( child )->layer()->type() != QgsMapLayerType::VectorLayer ) )
    {
      toRemove << child;
      continue;
    }

    cleanGroup( child );

    if ( QgsLayerTree::isGroup( child ) && child->children().isEmpty() )
      toRemove << child;
  }

  Q_FOREACH ( QgsLayerTreeNode *child, toRemove )
    group->removeChildNode( child );
}
