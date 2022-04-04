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
#include <QCheckBox>

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
#include "qgssettingsregistrycore.h"
#include "qgsscalewidget.h"

#ifdef ENABLE_MODELTEST
#include "modeltest.h"
#endif

class SnapTypeMenu: public QMenu
{
  public:
    SnapTypeMenu( const QString &title, QWidget *parent = nullptr )
      : QMenu( title, parent ) {}

    void mouseReleaseEvent( QMouseEvent *e )
    {
      QAction *action = activeAction();
      if ( action )
        action->trigger();
      else
        QMenu::mouseReleaseEvent( e );
    }
};

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
  QgsSnappingLayerTreeModel *model = new QgsSnappingLayerTreeModel( mProject, mCanvas, this );
  model->setLayerTreeModel( new QgsLayerTreeModel( mProject->layerTreeRoot(), model ) );
  mLayerTreeView->installEventFilter( this );

#ifdef ENABLE_MODELTEST
  new ModelTest( model, this );
  new ModelTest( model->layerTreeModel(), this );
#endif

  // connections
  connect( model, &QgsSnappingLayerTreeModel::rowsInserted, this, &QgsSnappingWidget::onSnappingTreeLayersChanged );
  connect( model, &QgsSnappingLayerTreeModel::modelReset, this, &QgsSnappingWidget::onSnappingTreeLayersChanged );
  connect( model, &QgsSnappingLayerTreeModel::rowsRemoved, this, &QgsSnappingWidget::onSnappingTreeLayersChanged );
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

  // avoid intersection mode button
  mAvoidIntersectionsModeButton = new QToolButton();
  mAvoidIntersectionsModeButton->setToolTip( tr( "When avoid overlap is enabled, digitized features will be clipped to not overlapped existing ones." ) );
  mAvoidIntersectionsModeButton->setPopupMode( QToolButton::InstantPopup );
  QMenu *avoidIntersectionsModeMenu = new QMenu( tr( "Set Avoid Overlap Mode" ), this );
  mAllowIntersectionsAction = new QAction( QIcon( QgsApplication::getThemeIcon( "/mActionAllowIntersections.svg" ) ), tr( "Allow Overlap" ), avoidIntersectionsModeMenu );
  mAvoidIntersectionsCurrentLayerAction = new QAction( QIcon( QgsApplication::getThemeIcon( "/mActionAvoidIntersectionsCurrentLayer.svg" ) ), tr( "Avoid Overlap on Active Layer" ), avoidIntersectionsModeMenu );
  mAvoidIntersectionsCurrentLayerAction->setToolTip( tr( "Avoid Overlap on Active Layer.\nBeware that this option will be applied on all vertices of the edited geometries, even if outside the current view extent" ) );
  mAvoidIntersectionsLayersAction = new QAction( QIcon( QgsApplication::getThemeIcon( "/mActionAvoidIntersectionsLayers.svg" ) ), tr( "Follow Advanced Configuration" ), avoidIntersectionsModeMenu );
  avoidIntersectionsModeMenu->addAction( mAllowIntersectionsAction );
  avoidIntersectionsModeMenu->addAction( mAvoidIntersectionsCurrentLayerAction );
  avoidIntersectionsModeMenu->addAction( mAvoidIntersectionsLayersAction );
  mAvoidIntersectionsModeButton->setMenu( avoidIntersectionsModeMenu );
  mAvoidIntersectionsModeButton->setObjectName( QStringLiteral( "AvoidIntersectionsModeButton" ) );
  if ( mDisplayMode == Widget )
  {
    mAvoidIntersectionsModeButton->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );
  }
  connect( mAvoidIntersectionsModeButton, &QToolButton::triggered, this, &QgsSnappingWidget::avoidIntersectionsModeButtonTriggered );

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
  SnapTypeMenu *typeMenu = new SnapTypeMenu( tr( "Set Snapping Mode" ), this );


  for ( Qgis::SnappingType type : qgsEnumList<Qgis::SnappingType>() )
  {
    if ( type == Qgis::SnappingType::NoSnap )
      continue;
    QAction *action = new QAction( QgsSnappingConfig::snappingTypeToIcon( type ), QgsSnappingConfig::snappingTypeToString( type ), typeMenu );
    action->setData( QVariant::fromValue( type ) );
    action->setCheckable( true );
    typeMenu->addAction( action );
    mSnappingFlagActions << action;
  }

  mTypeButton->setMenu( typeMenu );
  mTypeButton->setObjectName( QStringLiteral( "SnappingTypeButton" ) );
  if ( mDisplayMode == Widget )
  {
    mTypeButton->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );
  }
  connect( mTypeButton, &QToolButton::triggered, this, &QgsSnappingWidget::typeButtonTriggered );

  // tolerance
  mToleranceSpinBox = new QgsDoubleSpinBox();
  mToleranceSpinBox->setDecimals( 5 );
  mToleranceSpinBox->setMaximum( 99999999.990000 );
  mToleranceSpinBox->setToolTip( tr( "Snapping Tolerance in Defined Units" ) );
  mToleranceSpinBox->setObjectName( QStringLiteral( "SnappingToleranceSpinBox" ) );
  connect( mToleranceSpinBox, qOverload< double >( &QgsDoubleSpinBox::valueChanged ), this, &QgsSnappingWidget::changeTolerance );

  // units
  mUnitsComboBox = new QComboBox();
  mUnitsComboBox->addItem( tr( "px" ), QgsTolerance::Pixels );
  // Get canvas units
  const QString mapCanvasDistanceUnits { QgsUnitTypes::toString( mCanvas->mapSettings().mapUnits() ) };
  mUnitsComboBox->addItem( mapCanvasDistanceUnits, QgsTolerance::ProjectUnits );
  mUnitsComboBox->setToolTip( tr( "Snapping Unit Type: Pixels (px) or Project/Map Units (%1)" ).arg( mapCanvasDistanceUnits ) );
  mUnitsComboBox->setObjectName( QStringLiteral( "SnappingUnitComboBox" ) );
  connect( mUnitsComboBox, qOverload<int>( &QComboBox::currentIndexChanged ),
           this, &QgsSnappingWidget::changeUnit );

  connect( mCanvas, &QgsMapCanvas::destinationCrsChanged, this, [ = ]
  {
    // Update map units from canvas
    const QString mapCanvasDistanceUnits { QgsUnitTypes::toString( mCanvas->mapSettings().mapUnits() ) };
    mUnitsComboBox->setItemText( 1, mapCanvasDistanceUnits );
    mUnitsComboBox->setToolTip( tr( "Snapping Unit Type: Pixels (px) or Map Units (%1)" ).arg( mapCanvasDistanceUnits ) );
    model->resetLayerTreeModel();
  } );

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

  // self-snapping button
  mSelfSnappingAction = new QAction( tr( "Self-snapping" ), this );
  mSelfSnappingAction->setCheckable( true );
  mSelfSnappingAction->setIcon( QIcon( QgsApplication::getThemeIcon( "/mIconSnappingSelf.svg" ) ) );
  mSelfSnappingAction->setToolTip( tr( "If self snapping is enabled, snapping will also take the current state of the digitized feature into consideration." ) );
  mSelfSnappingAction->setObjectName( QStringLiteral( "SelfSnappingAction" ) );
  connect( mSelfSnappingAction, &QAction::toggled, this, &QgsSnappingWidget::enableSelfSnapping );

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
    mAvoidIntersectionsModeAction = tb->addWidget( mAvoidIntersectionsModeButton );
    tb->addAction( mIntersectionSnappingAction );
    tb->addAction( mEnableTracingAction );
    tb->addAction( mSelfSnappingAction );
  }
  else
  {
    mMinScaleWidget = new QgsScaleWidget();
    mMinScaleWidget->setToolTip( tr( "Minimum scale from which snapping is enabled (i.e. most \"zoomed out\" scale)" ) );
    mMinScaleWidget->setObjectName( QStringLiteral( "SnappingMinScaleSpinBox" ) );
    connect( mMinScaleWidget, &QgsScaleWidget::scaleChanged, this, &QgsSnappingWidget::changeMinScale );

    mMaxScaleWidget = new QgsScaleWidget();
    mMaxScaleWidget->setToolTip( tr( "Maximum scale up to which snapping is enabled (i.e. most \"zoomed in\" scale)" ) );
    mMaxScaleWidget->setObjectName( QStringLiteral( "SnappingMaxScaleSpinBox" ) );
    connect( mMaxScaleWidget, &QgsScaleWidget::scaleChanged, this, &QgsSnappingWidget::changeMaxScale );

    mSnappingScaleModeButton = new QToolButton();
    mSnappingScaleModeButton->setToolTip( tr( "Snapping scale mode" ) );
    mSnappingScaleModeButton->setPopupMode( QToolButton::InstantPopup );
    QMenu *scaleModeMenu = new QMenu( tr( "Set snapping scale mode" ), this );
    mDefaultSnappingScaleAct = new QAction( QIcon( QgsApplication::getThemeIcon( "/mIconSnappingOnScale.svg" ) ), tr( "Disabled" ), scaleModeMenu );
    mDefaultSnappingScaleAct->setToolTip( tr( "Scale dependency disabled" ) );
    mGlobalSnappingScaleAct = new QAction( QIcon( QgsApplication::getThemeIcon( "/mIconSnappingOnScale.svg" ) ), tr( "Global" ), scaleModeMenu );
    mGlobalSnappingScaleAct->setToolTip( tr( "Scale dependency global" ) );
    mPerLayerSnappingScaleAct = new QAction( QIcon( QgsApplication::getThemeIcon( "/mIconSnappingOnScale.svg" ) ), tr( "Per layer" ), scaleModeMenu );
    mPerLayerSnappingScaleAct->setToolTip( tr( "Scale dependency per layer" ) );
    scaleModeMenu->addAction( mDefaultSnappingScaleAct );
    scaleModeMenu->addAction( mGlobalSnappingScaleAct );
    scaleModeMenu->addAction( mPerLayerSnappingScaleAct );
    mSnappingScaleModeButton->setMenu( scaleModeMenu );
    mSnappingScaleModeButton->setObjectName( QStringLiteral( "SnappingScaleModeButton" ) );
    mSnappingScaleModeButton->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );
    connect( mSnappingScaleModeButton, &QToolButton::triggered, this, &QgsSnappingWidget::snappingScaleModeTriggered );

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
    mSnappingScaleModeButton->setDefaultAction( mDefaultSnappingScaleAct );
    layout->addWidget( mSnappingScaleModeButton );
    layout->addWidget( mMinScaleWidget );
    layout->addWidget( mMaxScaleWidget );

    QToolButton *topoButton = new QToolButton();
    topoButton->addAction( mTopologicalEditingAction );
    topoButton->setDefaultAction( mTopologicalEditingAction );
    topoButton->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );
    layout->addWidget( topoButton );

    layout->addWidget( mAvoidIntersectionsModeButton );

    QToolButton *interButton = new QToolButton();
    interButton->addAction( mIntersectionSnappingAction );
    interButton->setDefaultAction( mIntersectionSnappingAction );
    interButton->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );
    layout->addWidget( interButton );

    QToolButton *selfsnapButton = new QToolButton();
    selfsnapButton->addAction( mSelfSnappingAction );
    selfsnapButton->setDefaultAction( mSelfSnappingAction );
    selfsnapButton->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );
    layout->addWidget( selfsnapButton );

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
  connect( project, &QgsProject::avoidIntersectionsModeChanged, this, &QgsSnappingWidget::projectAvoidIntersectionModeChanged );
  connect( mCanvas, &QgsMapCanvas::destinationCrsChanged, this, &QgsSnappingWidget::updateToleranceDecimals );

  // Slightly modify the config so the settings changed code doesn't early exit
  mConfig = project->snappingConfig();
  mConfig.setEnabled( !mConfig.enabled() );
  projectSnapSettingsChanged();

  // modeChanged determines if widget are visible or not based on mode
  modeChanged();
  updateToleranceDecimals();

  enableSnapping( QgsSettingsRegistryCore::settingsDigitizingDefaultSnapEnabled.value() );

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

  if ( config.mode() == Qgis::SnappingMode::AllLayers && mModeButton->defaultAction() != mAllLayersAction )
  {
    mModeButton->setDefaultAction( mAllLayersAction );
    modeChanged();
    updateToleranceDecimals();
  }
  if ( config.mode() == Qgis::SnappingMode::ActiveLayer && mModeButton->defaultAction() != mActiveLayerAction )
  {
    mModeButton->setDefaultAction( mActiveLayerAction );
    modeChanged();
    updateToleranceDecimals();
  }
  if ( config.mode() == Qgis::SnappingMode::AdvancedConfiguration && mModeButton->defaultAction() != mAdvancedModeAction )
  {
    mModeButton->setDefaultAction( mAdvancedModeAction );
    modeChanged();
    updateToleranceDecimals();
  }

  // update snapping flag actions
  for ( QAction *action : std::as_const( mSnappingFlagActions ) )
  {
    const Qgis::SnappingTypes actionFlag = static_cast<Qgis::SnappingTypes>( action->data().toInt() );
    action->setChecked( config.typeFlag() & actionFlag );
    if ( action->isChecked() )
      mTypeButton->setDefaultAction( action );
  }

  if ( static_cast<QgsTolerance::UnitType>( mUnitsComboBox->currentData().toInt() ) != config.units() )
  {
    mUnitsComboBox->setCurrentIndex( mUnitsComboBox->findData( config.units() ) );
  }

  if ( mToleranceSpinBox->value() != config.tolerance() )
  {
    mToleranceSpinBox->setValue( config.tolerance() );
  }

  if ( mMinScaleWidget && mMinScaleWidget->scale() != config.minimumScale() )
  {
    mMinScaleWidget->setScale( config.minimumScale() );
  }

  if ( mMaxScaleWidget && mMaxScaleWidget->scale() != config.maximumScale() )
  {
    mMaxScaleWidget->setScale( config.maximumScale() );
  }

  if ( mSnappingScaleModeButton && config.scaleDependencyMode() == QgsSnappingConfig::Disabled )
  {
    mSnappingScaleModeButton->setDefaultAction( mDefaultSnappingScaleAct );
  }
  else if ( mSnappingScaleModeButton && config.scaleDependencyMode() == QgsSnappingConfig::Global )
  {
    mSnappingScaleModeButton->setDefaultAction( mGlobalSnappingScaleAct );
  }
  else if ( mSnappingScaleModeButton && config.scaleDependencyMode() == QgsSnappingConfig::PerLayer )
  {
    mSnappingScaleModeButton->setDefaultAction( mPerLayerSnappingScaleAct );
  }

  if ( config.intersectionSnapping() != mIntersectionSnappingAction->isChecked() )
  {
    mIntersectionSnappingAction->setChecked( config.intersectionSnapping() );
  }

  if ( config.selfSnapping() != mSelfSnappingAction->isChecked() )
  {
    mSelfSnappingAction->setChecked( config.selfSnapping() );
  }

  toggleSnappingWidgets( config.enabled() );

}

void QgsSnappingWidget::projectAvoidIntersectionModeChanged()
{
  switch ( mProject->avoidIntersectionsMode() )
  {
    case QgsProject::AvoidIntersectionsMode::AllowIntersections:
      mAvoidIntersectionsModeButton->setDefaultAction( mAllowIntersectionsAction );
      mAllowIntersectionsAction->setChecked( true );
      mAvoidIntersectionsCurrentLayerAction->setChecked( false );
      mAvoidIntersectionsLayersAction->setChecked( false );
      break;
    case QgsProject::AvoidIntersectionsMode::AvoidIntersectionsCurrentLayer:
      mAvoidIntersectionsModeButton->setDefaultAction( mAvoidIntersectionsCurrentLayerAction );
      mAllowIntersectionsAction->setChecked( false );
      mAvoidIntersectionsCurrentLayerAction->setChecked( true );
      mAvoidIntersectionsLayersAction->setChecked( false );
      break;
    case QgsProject::AvoidIntersectionsMode::AvoidIntersectionsLayers:
      mAvoidIntersectionsModeButton->setDefaultAction( mAvoidIntersectionsLayersAction );
      mAllowIntersectionsAction->setChecked( false );
      mAvoidIntersectionsCurrentLayerAction->setChecked( false );
      mAvoidIntersectionsLayersAction->setChecked( true );
      break;
  }
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
  if ( mSnappingScaleModeButton )
    mSnappingScaleModeButton->setEnabled( enabled );
  if ( mMinScaleWidget )
    mMinScaleWidget->setEnabled( enabled && mConfig.scaleDependencyMode() == QgsSnappingConfig::Global );
  if ( mMaxScaleWidget )
    mMaxScaleWidget->setEnabled( enabled && mConfig.scaleDependencyMode() == QgsSnappingConfig::Global );
  mUnitsComboBox->setEnabled( enabled );

  if ( mEditAdvancedConfigAction )
  {
    mEditAdvancedConfigAction->setEnabled( enabled );
  }

  if ( mAdvancedConfigWidget )
  {
    mAdvancedConfigWidget->setEnabled( enabled );
  }
  mIntersectionSnappingAction->setEnabled( enabled );
  mSelfSnappingAction->setEnabled( enabled );
  mEnableTracingAction->setEnabled( enabled );
}

void QgsSnappingWidget::changeTolerance( double tolerance )
{
  mConfig.setTolerance( tolerance );
  mProject->setSnappingConfig( mConfig );
}

void QgsSnappingWidget::changeMinScale( double minScale )
{
  mConfig.setMinimumScale( minScale );
  mProject->setSnappingConfig( mConfig );
}

void QgsSnappingWidget::changeMaxScale( double maxScale )
{
  mConfig.setMaximumScale( maxScale );
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

void QgsSnappingWidget::enableSelfSnapping( bool enabled )
{
  mConfig.setSelfSnapping( enabled );
  mProject->setSnappingConfig( mConfig );
}

void QgsSnappingWidget::onSnappingTreeLayersChanged()
{
  if ( mLayerTreeView->isVisible() )
  {
    mLayerTreeView->expandAll();
    mLayerTreeView->resizeColumnToContents( 0 );
    mRequireLayerTreeViewUpdate = false;
  }
  else
  {
    mRequireLayerTreeViewUpdate = true;
  }
}

void QgsSnappingWidget::avoidIntersectionsModeButtonTriggered( QAction *action )
{
  if ( action != mAllowIntersectionsAction &&
       action != mAvoidIntersectionsCurrentLayerAction &&
       action != mAvoidIntersectionsLayersAction )
  {
    return;
  }

  if ( action != mAvoidIntersectionsModeButton->defaultAction() )
  {
    mAvoidIntersectionsModeButton->setDefaultAction( action );
    if ( action == mAllowIntersectionsAction )
    {
      mProject->setAvoidIntersectionsMode( QgsProject::AvoidIntersectionsMode::AllowIntersections );
    }
    else if ( action == mAvoidIntersectionsCurrentLayerAction )
    {
      mProject->setAvoidIntersectionsMode( QgsProject::AvoidIntersectionsMode::AvoidIntersectionsCurrentLayer );
    }
    else if ( action == mAvoidIntersectionsLayersAction )
    {
      mProject->setAvoidIntersectionsMode( QgsProject::AvoidIntersectionsMode::AvoidIntersectionsLayers );
    }
  }
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
      mConfig.setMode( Qgis::SnappingMode::AllLayers );
    }
    else if ( action == mActiveLayerAction )
    {
      mConfig.setMode( Qgis::SnappingMode::ActiveLayer );
    }
    else if ( action == mAdvancedModeAction )
    {
      mConfig.setMode( Qgis::SnappingMode::AdvancedConfiguration );
    }
    mProject->setSnappingConfig( mConfig );
    updateToleranceDecimals();
    modeChanged();
  }
}

void QgsSnappingWidget::typeButtonTriggered( QAction *action )
{
  unsigned int type = static_cast<int>( mConfig.typeFlag() );

  const Qgis::SnappingTypes actionFlag = static_cast<Qgis::SnappingTypes>( action->data().toInt() );
  type ^= actionFlag;

  if ( type & actionFlag )
  {
    // user checked the action, set as new default
    mTypeButton->setDefaultAction( action );
  }
  else
  {
    // user unchecked the action -- find out which ones we should set as new default action
    for ( QAction *flagAction : std::as_const( mSnappingFlagActions ) )
    {
      if ( type & static_cast<Qgis::SnappingTypes>( flagAction->data().toInt() ) )
      {
        mTypeButton->setDefaultAction( flagAction );
        break;
      }
    }
  }

  mConfig.setTypeFlag( static_cast<Qgis::SnappingTypes>( type ) );
  mProject->setSnappingConfig( mConfig );
}

void QgsSnappingWidget::snappingScaleModeTriggered( QAction *action )
{
  mSnappingScaleModeButton->setDefaultAction( action );
  QgsSnappingConfig::ScaleDependencyMode mode = mConfig.scaleDependencyMode();

  if ( action == mDefaultSnappingScaleAct )
  {
    mode =  QgsSnappingConfig::Disabled;
  }
  else if ( action == mGlobalSnappingScaleAct )
  {
    mode = QgsSnappingConfig::Global;
  }
  else if ( action == mPerLayerSnappingScaleAct )
  {
    mode = QgsSnappingConfig::PerLayer;
  }

  mMinScaleWidget->setEnabled( mode == QgsSnappingConfig::Global );
  mMaxScaleWidget->setEnabled( mode == QgsSnappingConfig::Global );
  mConfig.setScaleDependencyMode( mode );
  mProject->setSnappingConfig( mConfig );

  mLayerTreeView->reset();
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
  bool advanced = mConfig.mode() == Qgis::SnappingMode::AdvancedConfiguration;

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
    if ( mSnappingScaleModeButton )
      mSnappingScaleModeButton->setVisible( advanced );
    if ( mMinScaleWidget )
      mMinScaleWidget->setVisible( advanced );
    if ( mMaxScaleWidget )
      mMaxScaleWidget->setVisible( advanced );
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

bool QgsSnappingWidget::eventFilter( QObject *watched, QEvent *event )
{
  if ( watched == mLayerTreeView  && event->type() == QEvent::Show )
  {
    if ( mRequireLayerTreeViewUpdate )
    {
      mLayerTreeView->expandAll();
      mLayerTreeView->resizeColumnToContents( 0 );
    }
  }
  return QWidget::eventFilter( watched, event );
}

void QgsSnappingWidget::cleanGroup( QgsLayerTreeNode *node )
{
  QgsLayerTreeGroup *group = QgsLayerTree::isGroup( node ) ? QgsLayerTree::toGroup( node ) : nullptr;
  if ( !group )
    return;

  QList<QgsLayerTreeNode *> toRemove;
  const auto constChildren = node->children();
  for ( QgsLayerTreeNode *child : constChildren )
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

  const auto constToRemove = toRemove;
  for ( QgsLayerTreeNode *child : constToRemove )
    group->removeChildNode( child );
}
