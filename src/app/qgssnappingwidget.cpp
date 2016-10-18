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
#include <QDoubleSpinBox>
#include <QFont>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMenu>
#include <QSettings>
#include <QStatusBar>
#include <QToolBar>
#include <QToolButton>

#include "qgsapplication.h"
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



QgsSnappingWidget::QgsSnappingWidget( QgsProject* project, QgsMapCanvas* canvas, QWidget* parent )
    : QWidget( parent )
    , mProject( project )
    , mCanvas( canvas )
    , mModeAction( nullptr )
    , mTypeAction( nullptr )
    , mToleranceAction( nullptr )
    , mUnitAction( nullptr )
    , mLayerTreeView( nullptr )
{
  // detect the type of display
  QToolBar* tb = qobject_cast<QToolBar*>( parent );
  if ( tb )
  {
    mDisplayMode = ToolBar;
    setObjectName( "SnappingOptionToolBar" );
  }
  else
  {
    QStatusBar *sb = qobject_cast<QStatusBar*>( parent );
    if ( sb )
    {
      mDisplayMode = StatusBar;
      setObjectName( "SnappingOptionStatusBar" );
    }
    else
    {
      mDisplayMode = Widget;
      setObjectName( "SnappingOptionDialog" );
    }
  }

  // enable button
  mEnabledAction = new QAction( this );
  mEnabledAction->setCheckable( true );
  mEnabledAction->setIcon( QIcon( QgsApplication::getThemeIcon( "/mIconSnapping.svg" ) ) );
  mEnabledAction->setToolTip( tr( "Enable snapping" ) );
  mEnabledAction->setObjectName( "EnableSnappingAction" );
  connect( mEnabledAction, SIGNAL( toggled( bool ) ) , this, SLOT( enableSnapping( bool ) ) );

  // mode button
  mModeButton = new QToolButton();
  mModeButton->setToolTip( tr( "Snapping mode" ) );
  mModeButton->setPopupMode( QToolButton::InstantPopup );
  QMenu *modeMenu = new QMenu( tr( "Set snapping mode" ), this );
  mAllLayersAction = new QAction( QIcon( QgsApplication::getThemeIcon( "/mIconSnappingAllLayers.svg" ) ), "All layers", modeMenu );
  mActiveLayerAction = new QAction( QIcon( QgsApplication::getThemeIcon( "/mIconSnappingActiveLayer.svg" ) ), "Active layer", modeMenu );
  mAdvancedModeAction = new QAction( QIcon( QgsApplication::getThemeIcon( "/mIconSnappingAdvanced.svg" ) ), "Advanced configuration", modeMenu );
  modeMenu->addAction( mAllLayersAction );
  modeMenu->addAction( mActiveLayerAction );
  modeMenu->addAction( mAdvancedModeAction );
  mModeButton->setMenu( modeMenu );
  mModeButton->setObjectName( "SnappingModeButton" );
  if ( mDisplayMode == Widget )
  {
    mModeButton->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );
  }
  connect( mModeButton, SIGNAL( triggered( QAction* ) ), this, SLOT( modeButtonTriggered( QAction* ) ) );

  // type button
  mTypeButton = new QToolButton();
  mTypeButton->setToolTip( tr( "Snapping type" ) );
  mTypeButton->setPopupMode( QToolButton::InstantPopup );
  QMenu *typeMenu = new QMenu( tr( "Set snapping mode" ), this );
  mVertexAction = new QAction( QIcon( QgsApplication::getThemeIcon( "/mIconSnappingVertex.svg" ) ), "Vertex", typeMenu );
  mVertexAndSegmentAction = new QAction( QIcon( QgsApplication::getThemeIcon( "/mIconSnappingVertexAndSegment.svg" ) ), "Vertex and segment", typeMenu );
  mSegmentAction = new QAction( QIcon( QgsApplication::getThemeIcon( "/mIconSnappingSegment.svg" ) ), "Segment", typeMenu );
  typeMenu->addAction( mVertexAction );
  typeMenu->addAction( mVertexAndSegmentAction );
  typeMenu->addAction( mSegmentAction );
  mTypeButton->setMenu( typeMenu );
  mTypeButton->setObjectName( "SnappingTypeButton" );
  if ( mDisplayMode == Widget )
  {
    mTypeButton->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );
  }
  connect( mTypeButton, SIGNAL( triggered( QAction* ) ), this, SLOT( typeButtonTriggered( QAction* ) ) );

  // tolerance
  mToleranceSpinBox = new QDoubleSpinBox();
  mToleranceSpinBox->setToolTip( tr( "Snapping tolerance in defined units" ) );
  mToleranceSpinBox->setObjectName( "SnappingToleranceSpinBox" );
  connect( mToleranceSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( changeTolerance( double ) ) );

  // units
  mUnitsComboBox = new QComboBox();
  mUnitsComboBox->addItem( tr( "px" ), QgsTolerance::Pixels );
  mUnitsComboBox->addItem( QgsUnitTypes::toString( QgsProject::instance()->distanceUnits() ), QgsTolerance::ProjectUnits );
  mUnitsComboBox->setToolTip( tr( "Snapping unit type: pixels (px) or map units (mu)" ) );
  mUnitsComboBox->setObjectName( "SnappingUnitComboBox" );
  connect( mUnitsComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( changeUnit( int ) ) );

  // topological editing button
  mTopologicalEditingAction = new QAction( tr( "topological editing" ), this );
  mTopologicalEditingAction->setCheckable( true );
  mTopologicalEditingAction->setIcon( QIcon( QgsApplication::getThemeIcon( "/mIconTopologicalEditing.svg" ) ) );
  mTopologicalEditingAction->setToolTip( tr( "Enable topological editing" ) );
  mTopologicalEditingAction->setObjectName( "TopologicalEditingAction" );
  connect( mTopologicalEditingAction, SIGNAL( toggled( bool ) ) , this, SLOT( enableTopologicalEditing( bool ) ) );

  // snapping on intersection button
  mIntersectionSnappingAction = new QAction( tr( "snapping on intersection" ), this );
  mIntersectionSnappingAction->setCheckable( true );
  mIntersectionSnappingAction->setIcon( QIcon( QgsApplication::getThemeIcon( "/mIconSnappingIntersection.svg" ) ) );
  mIntersectionSnappingAction->setToolTip( tr( "Enable snapping on intersection" ) );
  mIntersectionSnappingAction->setObjectName( "IntersectionSnappingAction" );
  connect( mIntersectionSnappingAction, SIGNAL( toggled( bool ) ) , this, SLOT( enableIntersectionSnapping( bool ) ) );

  // layout
  if ( mDisplayMode == ToolBar )
  {
    // hiding widget in a toolbar is not possible, actions are required
    tb->addAction( mEnabledAction );
    mModeAction = tb->addWidget( mModeButton );
    mTypeAction = tb->addWidget( mTypeButton );
    mToleranceAction = tb->addWidget( mToleranceSpinBox );
    mUnitAction = tb->addWidget( mUnitsComboBox );
    tb->addAction( mTopologicalEditingAction );
    tb->addAction( mIntersectionSnappingAction );
  }
  else
  {
    // mode = widget or status bar
    QHBoxLayout* layout = new QHBoxLayout();

    QToolButton* enabledButton = new QToolButton();
    enabledButton->addAction( mEnabledAction );
    enabledButton->setDefaultAction( mEnabledAction );
    layout->addWidget( enabledButton );

    layout->addWidget( mModeButton );
    layout->addWidget( mTypeButton );
    layout->addWidget( mToleranceSpinBox ) ;
    layout->addWidget( mUnitsComboBox ) ;

    QToolButton* topoButton = new QToolButton();
    topoButton->addAction( mTopologicalEditingAction );
    topoButton->setDefaultAction( mTopologicalEditingAction );
    if ( mDisplayMode == Widget )
    {
      topoButton->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );
    }
    layout->addWidget( topoButton );
    QToolButton* interButton = new QToolButton();
    interButton->addAction( mIntersectionSnappingAction );
    interButton->setDefaultAction( mIntersectionSnappingAction );
    if ( mDisplayMode == Widget )
    {
      interButton->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );
    }
    layout->addWidget( interButton );

    layout->setContentsMargins( 0, 0, 0, 0 );
    layout->setAlignment( Qt::AlignRight );
    layout->setSpacing( mDisplayMode == Widget ? 3 : 0 );

    if ( mDisplayMode == Widget )
    {
      mLayerTreeView = new QTreeView();
      QgsSnappingLayerTreeModel* model = new QgsSnappingLayerTreeModel( mProject, this );
      model->setLayerTreeModel( new QgsLayerTreeModel( QgsProject::instance()->layerTreeRoot(), model ) );
      // model->setFlags( 0 );
      mLayerTreeView->setModel( model );
      mLayerTreeView->resizeColumnToContents( 0 );
      mLayerTreeView->header()->show();
      mLayerTreeView->header()->setSectionResizeMode( QHeaderView::ResizeToContents );
      mLayerTreeView->header()->setSectionResizeMode( QHeaderView::Interactive );
      mLayerTreeView->setSelectionMode( QAbstractItemView::NoSelection );

      // item delegates
      mLayerTreeView->setEditTriggers( QAbstractItemView::AllEditTriggers );
      mLayerTreeView->setItemDelegate( new QgsSnappingLayerDelegate( mCanvas, this ) );

      QGridLayout* topLayout = new QGridLayout();
      topLayout->addLayout( layout, 0, 0, Qt::AlignLeft | Qt::AlignTop );
      topLayout->addWidget( mLayerTreeView, 1, 0 );
      setLayout( topLayout );
    }
    else
    {
      // mode = status bar
      setLayout( layout );
    }
  }

  // connect settings changed and map units changed to properly update the widget
  connect( project, &QgsProject::snappingConfigChanged, this, &QgsSnappingWidget::projectSnapSettingsChanged );
  connect( project, &QgsProject::topologicalEditingChanged, this, &QgsSnappingWidget::projectTopologicalEditingChanged );
  connect( mCanvas, SIGNAL( mapUnitsChanged() ), this, SLOT( updateToleranceDecimals() ) );

  // modeChanged determines if widget are visible or not based on mode
  modeChanged();
  updateToleranceDecimals();
}

QgsSnappingWidget::~QgsSnappingWidget()
{
  if ( mDisplayMode == Widget )
  {
    QSettings().setValue( "/Windows/SnappingWidget/geometry", saveGeometry() );
  }
}

void QgsSnappingWidget::projectSnapSettingsChanged()
{
  QgsSnappingConfig config = mProject->snappingConfig();
  if ( mConfig == config )
    return;
  mConfig = config;

  mEnabledAction->setChecked( config.enabled() );

  if ( config.mode() == QgsSnappingConfig::AllLayers && mModeButton->defaultAction() != mActiveLayerAction )
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

  if ( mToleranceSpinBox->value() != config.tolerance() )
  {
    mToleranceSpinBox->setValue( config.tolerance() );
  }

  if (( QgsTolerance::UnitType )mUnitsComboBox->currentData().toInt() != config.units() )
  {
    mUnitsComboBox->setCurrentIndex( mUnitsComboBox->findData( config.units() ) );
  }

  if ( config.intersectionSnapping() != mIntersectionSnappingAction->isChecked() )
  {
    mIntersectionSnappingAction->setChecked( config.intersectionSnapping() );
  }
}

void QgsSnappingWidget::projectTopologicalEditingChanged()
{
  if ( QgsProject::instance()->topologicalEditing() != mTopologicalEditingAction->isChecked() )
  {
    mTopologicalEditingAction->setChecked( QgsProject::instance()->topologicalEditing() );
  }
}

void QgsSnappingWidget::enableSnapping( bool checked )
{
  mModeButton->setEnabled( checked );
  mTypeButton->setEnabled( checked );
  mToleranceSpinBox->setEnabled( checked );
  mUnitsComboBox->setEnabled( checked );
  if ( mLayerTreeView )
  {
    mLayerTreeView->setEnabled( checked );
  }
  mTopologicalEditingAction->setEnabled( checked );
  mIntersectionSnappingAction->setEnabled( checked );

  mConfig.setEnabled( checked );
  mProject->setSnappingConfig( mConfig );
}

void QgsSnappingWidget::changeTolerance( double tolerance )
{
  mConfig.setTolerance( tolerance );
  mProject->setSnappingConfig( mConfig );
}

void QgsSnappingWidget::changeUnit( int idx )
{
  QgsTolerance::UnitType unit = ( QgsTolerance::UnitType )mUnitsComboBox->itemData( idx ).toInt();
  mConfig.setUnits( unit );
  mProject->setSnappingConfig( mConfig );

  updateToleranceDecimals();
}

void QgsSnappingWidget::enableTopologicalEditing( bool enabled )
{
  QgsProject::instance()->setTopologicalEditing( enabled );
}

void QgsSnappingWidget::enableIntersectionSnapping( bool enabled )
{
  mConfig.setIntersectionSnapping( enabled );
  mProject->setSnappingConfig( mConfig );
}

void QgsSnappingWidget::modeButtonTriggered( QAction* action )
{
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

void QgsSnappingWidget::typeButtonTriggered( QAction* action )
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
  }
  else
  {
    mTypeButton->setVisible( !advanced );
    mToleranceSpinBox->setVisible( !advanced );
    mUnitsComboBox->setVisible( !advanced );
    if ( mDisplayMode == Widget && mLayerTreeView )
    {
      mLayerTreeView->setVisible( advanced );
    }
  }
}

QgsSnappingConfig QgsSnappingWidget::config() const
{
  return mConfig;
}

void QgsSnappingWidget::setConfig( const QgsSnappingConfig& config )
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
    if ( QgsLayerTree::isLayer( child ) && QgsLayerTree::toLayer( child )->layer()->type() != QgsMapLayer::VectorLayer )
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
