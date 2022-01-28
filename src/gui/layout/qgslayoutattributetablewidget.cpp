/***************************************************************************
                         qgslayoutattributetablewidget.cpp
                         ---------------------------------
    begin                : November 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutattributetablewidget.h"
#include "qgslayoutatlas.h"
#include "qgslayout.h"
#include "qgslayoutframe.h"
#include "qgslayoutattributeselectiondialog.h"
#include "qgslayoutitemwidget.h"
#include "qgslayoutitemattributetable.h"
#include "qgslayouttablecolumn.h"
#include "qgslayoutitemmap.h"
#include "qgsvectorlayer.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgsproject.h"
#include "qgsrelationmanager.h"
#include "qgsguiutils.h"
#include "qgslayouttablebackgroundcolorsdialog.h"

QgsLayoutAttributeTableWidget::QgsLayoutAttributeTableWidget( QgsLayoutFrame *frame )
  : QgsLayoutItemBaseWidget( nullptr, frame ? qobject_cast< QgsLayoutItemAttributeTable* >( frame->multiFrame() ) : nullptr )
  , mTable( frame ? qobject_cast< QgsLayoutItemAttributeTable* >( frame->multiFrame() ) : nullptr )
  , mFrame( frame )
{
  setupUi( this );
  connect( mRefreshPushButton, &QPushButton::clicked, this, &QgsLayoutAttributeTableWidget::mRefreshPushButton_clicked );
  connect( mAttributesPushButton, &QPushButton::clicked, this, &QgsLayoutAttributeTableWidget::mAttributesPushButton_clicked );
  connect( mMaximumRowsSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsLayoutAttributeTableWidget::mMaximumRowsSpinBox_valueChanged );
  connect( mMarginSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutAttributeTableWidget::mMarginSpinBox_valueChanged );
  connect( mGridStrokeWidthSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutAttributeTableWidget::mGridStrokeWidthSpinBox_valueChanged );
  connect( mGridColorButton, &QgsColorButton::colorChanged, this, &QgsLayoutAttributeTableWidget::mGridColorButton_colorChanged );
  connect( mBackgroundColorButton, &QgsColorButton::colorChanged, this, &QgsLayoutAttributeTableWidget::mBackgroundColorButton_colorChanged );
  connect( mDrawHorizontalGrid, &QCheckBox::toggled, this, &QgsLayoutAttributeTableWidget::mDrawHorizontalGrid_toggled );
  connect( mDrawVerticalGrid, &QCheckBox::toggled, this, &QgsLayoutAttributeTableWidget::mDrawVerticalGrid_toggled );
  connect( mShowGridGroupCheckBox, &QgsCollapsibleGroupBoxBasic::toggled, this, &QgsLayoutAttributeTableWidget::mShowGridGroupCheckBox_toggled );
  connect( mShowOnlyVisibleFeaturesCheckBox, &QCheckBox::stateChanged, this, &QgsLayoutAttributeTableWidget::mShowOnlyVisibleFeaturesCheckBox_stateChanged );
  connect( mFeatureFilterCheckBox, &QCheckBox::stateChanged, this, &QgsLayoutAttributeTableWidget::mFeatureFilterCheckBox_stateChanged );
  connect( mFeatureFilterEdit, &QLineEdit::editingFinished, this, &QgsLayoutAttributeTableWidget::mFeatureFilterEdit_editingFinished );
  connect( mFeatureFilterButton, &QToolButton::clicked, this, &QgsLayoutAttributeTableWidget::mFeatureFilterButton_clicked );
  connect( mHeaderHAlignmentComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutAttributeTableWidget::mHeaderHAlignmentComboBox_currentIndexChanged );
  connect( mHeaderModeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutAttributeTableWidget::mHeaderModeComboBox_currentIndexChanged );
  connect( mWrapStringLineEdit, &QLineEdit::editingFinished, this, &QgsLayoutAttributeTableWidget::mWrapStringLineEdit_editingFinished );
  connect( mAddFramePushButton, &QPushButton::clicked, this, &QgsLayoutAttributeTableWidget::mAddFramePushButton_clicked );
  connect( mResizeModeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutAttributeTableWidget::mResizeModeComboBox_currentIndexChanged );
  connect( mSourceComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutAttributeTableWidget::mSourceComboBox_currentIndexChanged );
  connect( mRelationsComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutAttributeTableWidget::mRelationsComboBox_currentIndexChanged );
  connect( mEmptyModeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutAttributeTableWidget::mEmptyModeComboBox_currentIndexChanged );
  connect( mDrawEmptyCheckBox, &QCheckBox::toggled, this, &QgsLayoutAttributeTableWidget::mDrawEmptyCheckBox_toggled );
  connect( mEmptyMessageLineEdit, &QLineEdit::editingFinished, this, &QgsLayoutAttributeTableWidget::mEmptyMessageLineEdit_editingFinished );
  connect( mIntersectAtlasCheckBox, &QCheckBox::stateChanged, this, &QgsLayoutAttributeTableWidget::mIntersectAtlasCheckBox_stateChanged );
  connect( mUniqueOnlyCheckBox, &QCheckBox::stateChanged, this, &QgsLayoutAttributeTableWidget::mUniqueOnlyCheckBox_stateChanged );
  connect( mEmptyFrameCheckBox, &QCheckBox::toggled, this, &QgsLayoutAttributeTableWidget::mEmptyFrameCheckBox_toggled );
  connect( mHideEmptyBgCheckBox, &QCheckBox::toggled, this, &QgsLayoutAttributeTableWidget::mHideEmptyBgCheckBox_toggled );
  connect( mWrapBehaviorComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutAttributeTableWidget::mWrapBehaviorComboBox_currentIndexChanged );
  connect( mAdvancedCustomizationButton, &QPushButton::clicked, this, &QgsLayoutAttributeTableWidget::mAdvancedCustomizationButton_clicked );
  connect( mUseConditionalStylingCheckBox, &QCheckBox::stateChanged, this, &QgsLayoutAttributeTableWidget::useConditionalStylingChanged );
  setPanelTitle( tr( "Table Properties" ) );

  mContentFontToolButton->setMode( QgsFontButton::ModeTextRenderer );
  mHeaderFontToolButton->setMode( QgsFontButton::ModeTextRenderer );

  mContentFontToolButton->registerExpressionContextGenerator( this );
  mContentFontToolButton->setLayer( mTable->sourceLayer() );
  mHeaderFontToolButton->registerExpressionContextGenerator( this );
  mHeaderFontToolButton->setLayer( mTable->sourceLayer() );

  blockAllSignals( true );

  mResizeModeComboBox->addItem( tr( "Use Existing Frames" ), QgsLayoutMultiFrame::UseExistingFrames );
  mResizeModeComboBox->addItem( tr( "Extend to Next Page" ), QgsLayoutMultiFrame::ExtendToNextPage );
  mResizeModeComboBox->addItem( tr( "Repeat Until Finished" ), QgsLayoutMultiFrame::RepeatUntilFinished );

  mEmptyModeComboBox->addItem( tr( "Draw Headers Only" ), QgsLayoutTable::HeadersOnly );
  mEmptyModeComboBox->addItem( tr( "Hide Entire Table" ), QgsLayoutTable::HideTable );
  mEmptyModeComboBox->addItem( tr( "Show Set Message" ), QgsLayoutTable::ShowMessage );

  mWrapBehaviorComboBox->addItem( tr( "Truncate Text" ), QgsLayoutTable::TruncateText );
  mWrapBehaviorComboBox->addItem( tr( "Wrap Text" ), QgsLayoutTable::WrapText );

  mHeaderModeComboBox->addItem( tr( "On First Frame" ), QgsLayoutTable::FirstFrame );
  mHeaderModeComboBox->addItem( tr( "On All Frames" ), QgsLayoutTable::AllFrames );
  mHeaderModeComboBox->addItem( tr( "No Header" ), QgsLayoutTable::NoHeaders );

  mHeaderHAlignmentComboBox->addItem( tr( "Follow Column Alignment" ), QgsLayoutTable::FollowColumn );
  mHeaderHAlignmentComboBox->addItem( tr( "Left" ), QgsLayoutTable::HeaderLeft );
  mHeaderHAlignmentComboBox->addItem( tr( "Center" ), QgsLayoutTable::HeaderCenter );
  mHeaderHAlignmentComboBox->addItem( tr( "Right" ), QgsLayoutTable::HeaderRight );

  mSourceComboBox->addItem( tr( "Layer Features" ), QgsLayoutItemAttributeTable::LayerAttributes );
  toggleAtlasSpecificControls( static_cast< bool >( coverageLayer() ) );

  //update relations combo when relations modified in project
  connect( QgsProject::instance()->relationManager(), &QgsRelationManager::changed, this, &QgsLayoutAttributeTableWidget::updateRelationsCombo );

  mLayerComboBox->setFilters( QgsMapLayerProxyModel::VectorLayer );
  connect( mLayerComboBox, &QgsMapLayerComboBox::layerChanged, this, &QgsLayoutAttributeTableWidget::changeLayer );

  mComposerMapComboBox->setCurrentLayout( mTable->layout() );
  mComposerMapComboBox->setItemType( QgsLayoutItemRegistry::LayoutMap );
  connect( mComposerMapComboBox, &QgsLayoutItemComboBox::itemChanged, this, &QgsLayoutAttributeTableWidget::composerMapChanged );

  mGridColorButton->setColorDialogTitle( tr( "Select Grid Color" ) );
  mGridColorButton->setAllowOpacity( true );
  mGridColorButton->setContext( QStringLiteral( "composer" ) );
  mGridColorButton->setDefaultColor( Qt::black );
  mBackgroundColorButton->setColorDialogTitle( tr( "Select Background Color" ) );
  mBackgroundColorButton->setAllowOpacity( true );
  mBackgroundColorButton->setContext( QStringLiteral( "composer" ) );
  mBackgroundColorButton->setShowNoColor( true );
  mBackgroundColorButton->setNoColorString( tr( "No Background" ) );

  updateGuiElements();

  if ( mTable )
  {
    connect( mTable, &QgsLayoutMultiFrame::changed, this, &QgsLayoutAttributeTableWidget::updateGuiElements );

    // repopulate relations combo box if atlas layer changes
    connect( &mTable->layout()->reportContext(), &QgsLayoutReportContext::layerChanged,
             this, &QgsLayoutAttributeTableWidget::atlasToggled );

    if ( QgsLayoutAtlas *atlas = layoutAtlas() )
    {
      connect( atlas, &QgsLayoutAtlas::toggled, this, &QgsLayoutAttributeTableWidget::atlasToggled );
      atlasToggled();
    }

    mLayerSourceDDBtn->registerExpressionContextGenerator( mTable );
  }

  registerDataDefinedButton( mLayerSourceDDBtn, QgsLayoutObject::AttributeTableSourceLayer );

  //embed widget for general options
  if ( mFrame )
  {
    //add widget for general composer item properties
    mItemPropertiesWidget = new QgsLayoutItemPropertiesWidget( this, mFrame );
    mainLayout->addWidget( mItemPropertiesWidget );
  }

  connect( mHeaderFontToolButton, &QgsFontButton::changed, this, &QgsLayoutAttributeTableWidget::headerFontChanged );
  connect( mContentFontToolButton, &QgsFontButton::changed, this, &QgsLayoutAttributeTableWidget::contentFontChanged );
}

void QgsLayoutAttributeTableWidget::setReportTypeString( const QString &string )
{
  mIntersectAtlasCheckBox->setText( tr( "Show only features intersecting %1 feature" ).arg( string ) );
  const int atlasFeatureIndex = mSourceComboBox->findData( QgsLayoutItemAttributeTable::AtlasFeature );
  if ( atlasFeatureIndex != -1 )
  {
    mSourceComboBox->setItemText( atlasFeatureIndex,  tr( "Current %1 Feature" ).arg( string ) );
  }
}

void QgsLayoutAttributeTableWidget::setMasterLayout( QgsMasterLayoutInterface *masterLayout )
{
  if ( mItemPropertiesWidget )
    mItemPropertiesWidget->setMasterLayout( masterLayout );
}

QgsExpressionContext QgsLayoutAttributeTableWidget::createExpressionContext() const
{
  QgsExpressionContext context;

  // frames include their parent multiframe's context, so prefer that if possible
  if ( mFrame )
    context = mFrame->createExpressionContext();
  else if ( mTable )
    context = mTable->createExpressionContext();

  std::unique_ptr< QgsExpressionContextScope > cellScope = std::make_unique< QgsExpressionContextScope >();
  cellScope->setVariable( QStringLiteral( "row_number" ), 1, true );
  cellScope->setVariable( QStringLiteral( "column_number" ), 1, true );
  context.appendScope( cellScope.release() );

  context.setHighlightedVariables( { QStringLiteral( "row_number" ),
                                     QStringLiteral( "column_number" )} );

  return context;
}

bool QgsLayoutAttributeTableWidget::setNewItem( QgsLayoutItem *item )
{
  QgsLayoutFrame *frame = qobject_cast< QgsLayoutFrame * >( item );
  if ( !frame )
    return false;

  QgsLayoutMultiFrame *multiFrame = frame->multiFrame();
  if ( !multiFrame )
    return false;

  if ( multiFrame->type() != QgsLayoutItemRegistry::LayoutAttributeTable )
    return false;

  if ( mTable )
  {
    disconnect( mTable, &QgsLayoutObject::changed, this, &QgsLayoutAttributeTableWidget::updateGuiElements );
  }

  mTable = qobject_cast< QgsLayoutItemAttributeTable * >( multiFrame );
  mFrame = frame;
  mItemPropertiesWidget->setItem( frame );

  if ( mTable )
  {
    connect( mTable, &QgsLayoutObject::changed, this, &QgsLayoutAttributeTableWidget::updateGuiElements );
  }

  updateGuiElements();

  return true;
}


void QgsLayoutAttributeTableWidget::mRefreshPushButton_clicked()
{
  if ( !mTable )
  {
    return;
  }

  mTable->refreshAttributes();
}

void QgsLayoutAttributeTableWidget::mAttributesPushButton_clicked()
{
  if ( !mTable )
  {
    return;
  }

  //make deep copy of current columns, so we can restore them in case of cancellation
  QVector<QgsLayoutTableColumn> currentColumns = mTable->columns();
  QVector<QgsLayoutTableColumn> currentSortColumns = mTable->sortColumns();

  mTable->beginCommand( tr( "Change Table Attributes" ) );

  //temporarily block updates for the window, to stop table trying to repaint under windows (#11462)
  window()->setUpdatesEnabled( false );

  QgsLayoutAttributeSelectionDialog d( mTable, mTable->sourceLayer(), this );
  if ( d.exec() == QDialog::Accepted )
  {
    mTable->refreshAttributes();
    //safe to unblock updates
    window()->setUpdatesEnabled( true );
    mTable->update();
    mTable->endCommand();

    //clear currentColumns to free memory
    currentColumns.clear();
    currentSortColumns.clear();
  }
  else
  {
    //undo changes
    mTable->setColumns( currentColumns );
    mTable->setSortColumns( currentSortColumns );
    window()->setUpdatesEnabled( true );
    mTable->cancelCommand();
  }
}

void QgsLayoutAttributeTableWidget::composerMapChanged( QgsLayoutItem *item )
{
  if ( !mTable )
  {
    return;
  }

  mTable->beginCommand( tr( "Change Table Map" ) );
  mTable->setMap( qobject_cast< QgsLayoutItemMap * >( item ) );
  mTable->update();
  mTable->endCommand();
}

void QgsLayoutAttributeTableWidget::mMaximumRowsSpinBox_valueChanged( int i )
{
  if ( !mTable )
  {
    return;
  }

  mTable->beginCommand( tr( "Change Table Rows" ), QgsLayoutMultiFrame::UndoTableMaximumFeatures );
  mTable->setMaximumNumberOfFeatures( i );
  mTable->update();
  mTable->endCommand();
}

void QgsLayoutAttributeTableWidget::mMarginSpinBox_valueChanged( double d )
{
  if ( !mTable )
  {
    return;
  }

  mTable->beginCommand( tr( "Change Table Margin" ), QgsLayoutMultiFrame::UndoTableMargin );
  mTable->setCellMargin( d );
  mTable->endCommand();
}

void QgsLayoutAttributeTableWidget::headerFontChanged()
{
  if ( !mTable )
    return;

  mTable->beginCommand( tr( "Change Table Text Format" ) );
  mTable->setHeaderTextFormat( mHeaderFontToolButton->textFormat() );
  mTable->endCommand();
}

void QgsLayoutAttributeTableWidget::contentFontChanged()
{
  if ( !mTable )
  {
    return;
  }

  mTable->beginCommand( tr( "Change Table Text Format" ) );
  mTable->setContentTextFormat( mContentFontToolButton->textFormat() );
  mTable->endCommand();
}

void QgsLayoutAttributeTableWidget::mGridStrokeWidthSpinBox_valueChanged( double d )
{
  if ( !mTable )
  {
    return;
  }

  mTable->beginCommand( tr( "Change Table Line Width" ), QgsLayoutMultiFrame::UndoTableGridStrokeWidth );
  mTable->setGridStrokeWidth( d );
  mTable->endCommand();
}

void QgsLayoutAttributeTableWidget::mGridColorButton_colorChanged( const QColor &newColor )
{
  if ( !mTable )
  {
    return;
  }

  mTable->beginCommand( tr( "Change Table Grid Color" ), QgsLayoutMultiFrame::UndoTableGridColor );
  mTable->setGridColor( newColor );
  mTable->endCommand();
}

void QgsLayoutAttributeTableWidget::mDrawHorizontalGrid_toggled( bool state )
{
  if ( !mTable )
  {
    return;
  }

  mTable->beginCommand( tr( "Toggle Table Grid" ) );
  mTable->setHorizontalGrid( state );
  mTable->endCommand();
}

void QgsLayoutAttributeTableWidget::mDrawVerticalGrid_toggled( bool state )
{
  if ( !mTable )
  {
    return;
  }

  mTable->beginCommand( tr( "Toggled Table Grid" ) );
  mTable->setVerticalGrid( state );
  mTable->endCommand();
}

void QgsLayoutAttributeTableWidget::mShowGridGroupCheckBox_toggled( bool state )
{
  if ( !mTable )
  {
    return;
  }

  mTable->beginCommand( tr( "Toggle Table Grid" ) );
  mTable->setShowGrid( state );
  mTable->endCommand();
}

void QgsLayoutAttributeTableWidget::mBackgroundColorButton_colorChanged( const QColor &newColor )
{
  if ( !mTable )
  {
    return;
  }

  mTable->beginCommand( tr( "Change Table Color" ), QgsLayoutMultiFrame::UndoTableBackgroundColor );
  mTable->setBackgroundColor( newColor );
  mTable->endCommand();
}

void QgsLayoutAttributeTableWidget::updateGuiElements()
{
  if ( !mTable || !mFrame )
  {
    return;
  }

  blockAllSignals( true );

  mSourceComboBox->setCurrentIndex( mSourceComboBox->findData( mTable->source() ) );
  mRelationsComboBox->setCurrentIndex( mRelationsComboBox->findData( mTable->relationId() ) );

  //layer combo box
  if ( mTable->vectorLayer() )
  {
    mLayerComboBox->setLayer( mTable->vectorLayer() );
    if ( mTable->vectorLayer()->geometryType() == QgsWkbTypes::NullGeometry )
    {
      //layer has no geometry, so uncheck & disable controls which require geometry
      mShowOnlyVisibleFeaturesCheckBox->setChecked( false );
      mShowOnlyVisibleFeaturesCheckBox->setEnabled( false );
      mComposerMapComboBox->setEnabled( false );
      mComposerMapLabel->setEnabled( false );
      mIntersectAtlasCheckBox->setEnabled( false );
    }
    else
    {
      mShowOnlyVisibleFeaturesCheckBox->setEnabled( true );
      mComposerMapComboBox->setEnabled( mShowOnlyVisibleFeaturesCheckBox->isChecked() );
      mComposerMapLabel->setEnabled( mShowOnlyVisibleFeaturesCheckBox->isChecked() );
      mIntersectAtlasCheckBox->setEnabled( mSourceComboBox->findData( QgsLayoutItemAttributeTable::AtlasFeature ) != -1  && mTable->layout()->reportContext().layer() && mTable->layout()->reportContext().layer()->geometryType() != QgsWkbTypes::NullGeometry );
    }
  }

  mComposerMapComboBox->setItem( mTable->map() );
  mMaximumRowsSpinBox->setValue( mTable->maximumNumberOfFeatures() );
  mMarginSpinBox->setValue( mTable->cellMargin() );
  mGridStrokeWidthSpinBox->setValue( mTable->gridStrokeWidth() );
  mGridColorButton->setColor( mTable->gridColor() );
  mDrawHorizontalGrid->setChecked( mTable->horizontalGrid() );
  mDrawVerticalGrid->setChecked( mTable->verticalGrid() );
  if ( mTable->showGrid() )
  {
    mShowGridGroupCheckBox->setChecked( true );
  }
  else
  {
    mShowGridGroupCheckBox->setChecked( false );
  }
  mBackgroundColorButton->setColor( mTable->backgroundColor() );

  mHeaderFontToolButton->setTextFormat( mTable->headerTextFormat() );
  mContentFontToolButton->setTextFormat( mTable->contentTextFormat() );

  if ( mTable->displayOnlyVisibleFeatures() && mShowOnlyVisibleFeaturesCheckBox->isEnabled() )
  {
    mShowOnlyVisibleFeaturesCheckBox->setCheckState( Qt::Checked );
    mComposerMapComboBox->setEnabled( true );
    mComposerMapLabel->setEnabled( true );
  }
  else
  {
    mShowOnlyVisibleFeaturesCheckBox->setCheckState( Qt::Unchecked );
    mComposerMapComboBox->setEnabled( false );
    mComposerMapLabel->setEnabled( false );
  }

  mUniqueOnlyCheckBox->setChecked( mTable->uniqueRowsOnly() );
  mIntersectAtlasCheckBox->setChecked( mTable->filterToAtlasFeature() );
  mFeatureFilterEdit->setText( mTable->featureFilter() );
  mFeatureFilterCheckBox->setCheckState( mTable->filterFeatures() ? Qt::Checked : Qt::Unchecked );
  mFeatureFilterEdit->setEnabled( mTable->filterFeatures() );
  mFeatureFilterButton->setEnabled( mTable->filterFeatures() );
  mUseConditionalStylingCheckBox->setChecked( mTable->useConditionalStyling() );

  mHeaderHAlignmentComboBox->setCurrentIndex( mHeaderHAlignmentComboBox->findData( mTable->headerHAlignment() ) );
  mHeaderModeComboBox->setCurrentIndex( mHeaderModeComboBox->findData( mTable->headerMode() ) );

  mEmptyModeComboBox->setCurrentIndex( mEmptyModeComboBox->findData( mTable->emptyTableBehavior() ) );
  mEmptyMessageLineEdit->setText( mTable->emptyTableMessage() );
  mEmptyMessageLineEdit->setEnabled( mTable->emptyTableBehavior() == QgsLayoutTable::ShowMessage );
  mEmptyMessageLabel->setEnabled( mTable->emptyTableBehavior() == QgsLayoutTable::ShowMessage );
  mDrawEmptyCheckBox->setChecked( mTable->showEmptyRows() );
  mWrapStringLineEdit->setText( mTable->wrapString() );
  mWrapBehaviorComboBox->setCurrentIndex( mWrapBehaviorComboBox->findData( mTable->wrapBehavior() ) );

  mResizeModeComboBox->setCurrentIndex( mResizeModeComboBox->findData( mTable->resizeMode() ) );
  mAddFramePushButton->setEnabled( mTable->resizeMode() == QgsLayoutMultiFrame::UseExistingFrames );

  mEmptyFrameCheckBox->setChecked( mFrame->hidePageIfEmpty() );
  mHideEmptyBgCheckBox->setChecked( mFrame->hideBackgroundIfEmpty() );

  updateDataDefinedButton( mLayerSourceDDBtn );

  toggleSourceControls();

  blockAllSignals( false );
}

void QgsLayoutAttributeTableWidget::atlasToggled()
{
  // display/hide atlas options in source combobox depending on atlas status
  // if there's no atlas but there IS a coverageLayer, it's a report export and we should enable the controls
  const bool atlasEnabled = ( layoutAtlas() && layoutAtlas()->enabled() ) || ( !layoutAtlas() && coverageLayer() );


  toggleAtlasSpecificControls( atlasEnabled );

  if ( !mTable )
    return;

  whileBlocking( mSourceComboBox )->setCurrentIndex( mSourceComboBox->findData( mTable->source() ) );

  if ( !atlasEnabled && mTable->filterToAtlasFeature() )
  {
    mTable->setFilterToAtlasFeature( false );
  }
}

void QgsLayoutAttributeTableWidget::updateRelationsCombo()
{
  mRelationsComboBox->blockSignals( true );
  mRelationsComboBox->clear();

  QgsVectorLayer *atlasLayer = coverageLayer();
  if ( atlasLayer )
  {
    const QList<QgsRelation> relations = QgsProject::instance()->relationManager()->referencedRelations( atlasLayer );
    for ( const QgsRelation &relation : relations )
    {
      mRelationsComboBox->addItem( relation.name(), relation.id() );
    }
    if ( mTable )
    {
      mRelationsComboBox->setCurrentIndex( mRelationsComboBox->findData( mTable->relationId() ) );
    }
  }

  mRelationsComboBox->blockSignals( false );
}

void QgsLayoutAttributeTableWidget::toggleAtlasSpecificControls( const bool atlasEnabled )
{
  if ( !atlasEnabled )
  {
    if ( mTable->source() == QgsLayoutItemAttributeTable::AtlasFeature )
    {
      mTable->setSource( QgsLayoutItemAttributeTable::LayerAttributes );
    }
    mSourceComboBox->removeItem( mSourceComboBox->findData( QgsLayoutItemAttributeTable::AtlasFeature ) );
    mSourceComboBox->removeItem( mSourceComboBox->findData( QgsLayoutItemAttributeTable::RelationChildren ) );
    mRelationsComboBox->blockSignals( true );
    mRelationsComboBox->setEnabled( false );
    mRelationsComboBox->clear();
    mRelationsComboBox->blockSignals( false );
    mIntersectAtlasCheckBox->setEnabled( false );
  }
  else
  {
    if ( mSourceComboBox->findData( QgsLayoutItemAttributeTable::AtlasFeature ) == -1 )
    {
      //add missing atlasfeature option to combobox
      mSourceComboBox->addItem( tr( "Current Atlas Feature" ), QgsLayoutItemAttributeTable::AtlasFeature );
    }
    if ( mSourceComboBox->findData( QgsLayoutItemAttributeTable::RelationChildren ) == -1 )
    {
      //add missing relation children option to combobox
      mSourceComboBox->addItem( tr( "Relation Children" ), QgsLayoutItemAttributeTable::RelationChildren );
    }

    //add relations for coverage layer
    updateRelationsCombo();
    mRelationsComboBox->setEnabled( true );
    mIntersectAtlasCheckBox->setEnabled( mTable->layout()->reportContext().layer() && mTable->layout()->reportContext().layer()->geometryType() != QgsWkbTypes::NullGeometry );
  }
}

void QgsLayoutAttributeTableWidget::blockAllSignals( bool b )
{
  mSourceComboBox->blockSignals( b );
  mLayerComboBox->blockSignals( b );
  mComposerMapComboBox->blockSignals( b );
  mMaximumRowsSpinBox->blockSignals( b );
  mMarginSpinBox->blockSignals( b );
  mGridColorButton->blockSignals( b );
  mGridStrokeWidthSpinBox->blockSignals( b );
  mBackgroundColorButton->blockSignals( b );
  mDrawHorizontalGrid->blockSignals( b );
  mDrawVerticalGrid->blockSignals( b );
  mShowGridGroupCheckBox->blockSignals( b );
  mShowOnlyVisibleFeaturesCheckBox->blockSignals( b );
  mUniqueOnlyCheckBox->blockSignals( b );
  mIntersectAtlasCheckBox->blockSignals( b );
  mFeatureFilterEdit->blockSignals( b );
  mFeatureFilterCheckBox->blockSignals( b );
  mHeaderHAlignmentComboBox->blockSignals( b );
  mHeaderModeComboBox->blockSignals( b );
  mResizeModeComboBox->blockSignals( b );
  mRelationsComboBox->blockSignals( b );
  mEmptyModeComboBox->blockSignals( b );
  mEmptyMessageLineEdit->blockSignals( b );
  mEmptyFrameCheckBox->blockSignals( b );
  mHideEmptyBgCheckBox->blockSignals( b );
  mDrawEmptyCheckBox->blockSignals( b );
  mWrapStringLineEdit->blockSignals( b );
  mWrapBehaviorComboBox->blockSignals( b );
  mContentFontToolButton->blockSignals( b );
  mHeaderFontToolButton->blockSignals( b );
}

void QgsLayoutAttributeTableWidget::setMaximumNumberOfFeatures( int n )
{
  whileBlocking( mMaximumRowsSpinBox )->setValue( n );
}

void QgsLayoutAttributeTableWidget::mShowOnlyVisibleFeaturesCheckBox_stateChanged( int state )
{
  if ( !mTable )
  {
    return;
  }

  mTable->beginCommand( tr( "Toggle Visible Features Only" ) );
  const bool showOnlyVisibleFeatures = ( state == Qt::Checked );
  mTable->setDisplayOnlyVisibleFeatures( showOnlyVisibleFeatures );
  mTable->update();
  mTable->endCommand();

  //enable/disable map combobox based on state of checkbox
  mComposerMapComboBox->setEnabled( state == Qt::Checked );
  mComposerMapLabel->setEnabled( state == Qt::Checked );
}

void QgsLayoutAttributeTableWidget::mUniqueOnlyCheckBox_stateChanged( int state )
{
  if ( !mTable )
  {
    return;
  }

  mTable->beginCommand( tr( "Toggle Table Filter Duplicates" ) );
  mTable->setUniqueRowsOnly( state == Qt::Checked );
  mTable->update();
  mTable->endCommand();
}

void QgsLayoutAttributeTableWidget::mEmptyFrameCheckBox_toggled( bool checked )
{
  if ( !mFrame )
  {
    return;
  }

  mFrame->beginCommand( tr( "Toggle Empty Frame Mode" ) );
  mFrame->setHidePageIfEmpty( checked );
  mFrame->endCommand();
}

void QgsLayoutAttributeTableWidget::mHideEmptyBgCheckBox_toggled( bool checked )
{
  if ( !mFrame )
  {
    return;
  }

  mFrame->beginCommand( tr( "Toggle Background Display" ) );
  mFrame->setHideBackgroundIfEmpty( checked );
  mFrame->endCommand();
}

void QgsLayoutAttributeTableWidget::mIntersectAtlasCheckBox_stateChanged( int state )
{
  if ( !mTable )
  {
    return;
  }

  mTable->beginCommand( tr( "Toggle Table Atlas Filter" ) );
  const bool filterToAtlas = ( state == Qt::Checked );
  mTable->setFilterToAtlasFeature( filterToAtlas );
  mTable->update();
  mTable->endCommand();
}

void QgsLayoutAttributeTableWidget::mFeatureFilterCheckBox_stateChanged( int state )
{
  if ( !mTable )
  {
    return;
  }

  if ( state == Qt::Checked )
  {
    mFeatureFilterEdit->setEnabled( true );
    mFeatureFilterButton->setEnabled( true );
  }
  else
  {
    mFeatureFilterEdit->setEnabled( false );
    mFeatureFilterButton->setEnabled( false );
  }

  mTable->beginCommand( tr( "Toggle Table Feature Filter" ) );
  mTable->setFilterFeatures( state == Qt::Checked );
  mTable->update();
  mTable->endCommand();
}

void QgsLayoutAttributeTableWidget::mFeatureFilterEdit_editingFinished()
{
  if ( !mTable )
  {
    return;
  }

  mTable->beginCommand( tr( "Change Table Feature Filter" ) );
  mTable->setFeatureFilter( mFeatureFilterEdit->text() );
  mTable->update();
  mTable->endCommand();
}

void QgsLayoutAttributeTableWidget::mFeatureFilterButton_clicked()
{
  if ( !mTable )
  {
    return;
  }

  const QgsExpressionContext context = mTable->createExpressionContext();
  QgsExpressionBuilderDialog exprDlg( mTable->sourceLayer(), mFeatureFilterEdit->text(), this, QStringLiteral( "generic" ), context );
  exprDlg.setWindowTitle( tr( "Expression Based Filter" ) );
  if ( exprDlg.exec() == QDialog::Accepted )
  {
    const QString expression = exprDlg.expressionText();
    if ( !expression.isEmpty() )
    {
      mFeatureFilterEdit->setText( expression );
      mTable->beginCommand( tr( "Change Table Feature Filter" ) );
      mTable->setFeatureFilter( mFeatureFilterEdit->text() );
      mTable->update();
      mTable->endCommand();
    }
  }
}

void QgsLayoutAttributeTableWidget::mHeaderHAlignmentComboBox_currentIndexChanged( int )
{
  if ( !mTable )
  {
    return;
  }

  mTable->beginCommand( tr( "Change Table Alignment" ) );
  mTable->setHeaderHAlignment( static_cast<  QgsLayoutTable::HeaderHAlignment >( mHeaderHAlignmentComboBox->currentData().toInt() ) );
  mTable->endCommand();
}

void QgsLayoutAttributeTableWidget::mHeaderModeComboBox_currentIndexChanged( int )
{
  if ( !mTable )
  {
    return;
  }

  mTable->beginCommand( tr( "Change Table Header Mode" ) );
  mTable->setHeaderMode( static_cast< QgsLayoutTable::HeaderMode >( mHeaderModeComboBox->currentData().toInt() ) );
  mTable->endCommand();
}

void QgsLayoutAttributeTableWidget::mWrapStringLineEdit_editingFinished()
{
  if ( !mTable )
  {
    return;
  }

  mTable->beginCommand( tr( "Change Table Wrap String" ) );
  mTable->setWrapString( mWrapStringLineEdit->text() );
  mTable->endCommand();
}

void QgsLayoutAttributeTableWidget::changeLayer( QgsMapLayer *layer )
{
  if ( !mTable )
  {
    return;
  }

  QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layer );
  if ( !vl )
  {
    return;
  }

  mTable->beginCommand( tr( "Change Table Layer" ) );
  mTable->setVectorLayer( vl );
  mTable->update();
  mTable->endCommand();

  mContentFontToolButton->setLayer( vl );
  mHeaderFontToolButton->setLayer( vl );

  if ( vl->geometryType() == QgsWkbTypes::NullGeometry )
  {
    //layer has no geometry, so uncheck & disable controls which require geometry
    mShowOnlyVisibleFeaturesCheckBox->setChecked( false );
    mShowOnlyVisibleFeaturesCheckBox->setEnabled( false );
    mComposerMapComboBox->setEnabled( false );
    mComposerMapLabel->setEnabled( false );
    mIntersectAtlasCheckBox->setEnabled( false );
  }
  else
  {
    mShowOnlyVisibleFeaturesCheckBox->setEnabled( true );
    mComposerMapComboBox->setEnabled( mShowOnlyVisibleFeaturesCheckBox->isChecked() );
    mComposerMapLabel->setEnabled( mShowOnlyVisibleFeaturesCheckBox->isChecked() );
    mIntersectAtlasCheckBox->setEnabled( mSourceComboBox->findData( QgsLayoutItemAttributeTable::AtlasFeature ) != -1  && mTable->layout()->reportContext().layer() && mTable->layout()->reportContext().layer()->geometryType() != QgsWkbTypes::NullGeometry );
  }
}

void QgsLayoutAttributeTableWidget::mAddFramePushButton_clicked()
{
  if ( !mTable || !mFrame )
  {
    return;
  }

  //create a new frame based on the current frame
  QPointF pos = mFrame->pos();
  //shift new frame so that it sits 10 units below current frame
  pos.ry() += mFrame->rect().height() + 10;

  QgsLayoutFrame *newFrame = mTable->createNewFrame( mFrame, pos, mFrame->rect().size() );
  mTable->recalculateFrameSizes();

  //set new frame as selection
  if ( QgsLayout *layout = mTable->layout() )
  {
    layout->setSelectedItem( newFrame );
  }
}

void QgsLayoutAttributeTableWidget::mResizeModeComboBox_currentIndexChanged( int index )
{
  if ( !mTable )
  {
    return;
  }

  mTable->beginCommand( tr( "Change Resize Mode" ) );
  mTable->setResizeMode( static_cast< QgsLayoutMultiFrame::ResizeMode >( mResizeModeComboBox->itemData( index ).toInt() ) );
  mTable->endCommand();

  mAddFramePushButton->setEnabled( mTable->resizeMode() == QgsLayoutMultiFrame::UseExistingFrames );
}

void QgsLayoutAttributeTableWidget::mSourceComboBox_currentIndexChanged( int index )
{
  if ( !mTable )
  {
    return;
  }

  mTable->beginCommand( tr( "Change Table Source" ) );
  mTable->setSource( static_cast< QgsLayoutItemAttributeTable::ContentSource >( mSourceComboBox->itemData( index ).toInt() ) );
  mTable->endCommand();

  toggleSourceControls();
}

void QgsLayoutAttributeTableWidget::mRelationsComboBox_currentIndexChanged( int index )
{
  if ( !mTable )
  {
    return;
  }

  mTable->beginCommand( tr( "Change Table Source Relation" ) );
  mTable->setRelationId( mRelationsComboBox->itemData( index ).toString() );
  mTable->endCommand();
}

void QgsLayoutAttributeTableWidget::mEmptyModeComboBox_currentIndexChanged( int index )
{
  if ( !mTable )
  {
    return;
  }

  mTable->beginCommand( tr( "Change Empty Table Behavior" ) );
  mTable->setEmptyTableBehavior( static_cast< QgsLayoutTable::EmptyTableMode >( mEmptyModeComboBox->itemData( index ).toInt() ) );
  mTable->endCommand();
  mEmptyMessageLineEdit->setEnabled( mTable->emptyTableBehavior() == QgsLayoutTable::ShowMessage );
  mEmptyMessageLabel->setEnabled( mTable->emptyTableBehavior() == QgsLayoutTable::ShowMessage );
}

void QgsLayoutAttributeTableWidget::mWrapBehaviorComboBox_currentIndexChanged( int index )
{
  if ( !mTable )
  {
    return;
  }

  mTable->beginCommand( tr( "Change Table Wrap Mode" ) );
  mTable->setWrapBehavior( static_cast< QgsLayoutTable::WrapBehavior >( mWrapBehaviorComboBox->itemData( index ).toInt() ) );
  mTable->endCommand();
}

void QgsLayoutAttributeTableWidget::mAdvancedCustomizationButton_clicked()
{
  if ( !mTable )
  {
    return;
  }

  QgsLayoutTableBackgroundColorsDialog d( mTable, this );
  d.exec();
}

void QgsLayoutAttributeTableWidget::useConditionalStylingChanged( bool checked )
{
  if ( !mTable )
  {
    return;
  }

  mTable->beginCommand( tr( "Toggle Table Conditional Styling" ) );
  mTable->setUseConditionalStyling( checked );
  mTable->update();
  mTable->endCommand();
}

void QgsLayoutAttributeTableWidget::mDrawEmptyCheckBox_toggled( bool checked )
{
  if ( !mTable )
  {
    return;
  }

  mTable->beginCommand( tr( "Change Show Empty Rows" ) );
  mTable->setShowEmptyRows( checked );
  mTable->endCommand();
}

void QgsLayoutAttributeTableWidget::mEmptyMessageLineEdit_editingFinished()
{
  if ( !mTable )
  {
    return;
  }

  mTable->beginCommand( tr( "Change Empty Table Message" ) );
  mTable->setEmptyTableMessage( mEmptyMessageLineEdit->text() );
  mTable->endCommand();
}

void QgsLayoutAttributeTableWidget::toggleSourceControls()
{
  switch ( mTable->source() )
  {
    case QgsLayoutItemAttributeTable::LayerAttributes:
      mLayerComboBox->setEnabled( true );
      mLayerComboBox->setVisible( true );
      mLayerSourceDDBtn->setVisible( true );
      mLayerLabel->setVisible( true );
      mRelationsComboBox->setEnabled( false );
      mRelationsComboBox->setVisible( false );
      mRelationLabel->setVisible( false );
      mMaximumRowsSpinBox->setEnabled( true );
      mMaxNumFeaturesLabel->setEnabled( true );
      mShowOnlyVisibleFeaturesCheckBox->setEnabled( mTable->vectorLayer() && mTable->vectorLayer()->geometryType() != QgsWkbTypes::NullGeometry );
      mShowOnlyVisibleFeaturesCheckBox->setChecked( mTable->vectorLayer() && mTable->vectorLayer()->geometryType() != QgsWkbTypes::NullGeometry && mTable->displayOnlyVisibleFeatures() );
      mComposerMapComboBox->setEnabled( mShowOnlyVisibleFeaturesCheckBox->isChecked() );
      mComposerMapLabel->setEnabled( mShowOnlyVisibleFeaturesCheckBox->isChecked() );
      mIntersectAtlasCheckBox->setEnabled( mTable->vectorLayer() && mTable->vectorLayer()->geometryType() != QgsWkbTypes::NullGeometry
                                           && mSourceComboBox->findData( QgsLayoutItemAttributeTable::AtlasFeature ) != -1 && mTable->layout()->reportContext().layer() && mTable->layout()->reportContext().layer()->geometryType() != QgsWkbTypes::NullGeometry );
      break;
    case QgsLayoutItemAttributeTable::AtlasFeature:
      mLayerComboBox->setEnabled( false );
      mLayerComboBox->setVisible( false );
      mLayerSourceDDBtn->setVisible( false );
      mLayerLabel->setVisible( false );
      mRelationsComboBox->setEnabled( false );
      mRelationsComboBox->setVisible( false );
      mRelationLabel->setVisible( false );
      mMaximumRowsSpinBox->setEnabled( false );
      mMaxNumFeaturesLabel->setEnabled( false );
      mShowOnlyVisibleFeaturesCheckBox->setEnabled( mTable->sourceLayer() && mTable->sourceLayer()->geometryType() != QgsWkbTypes::NullGeometry );
      mShowOnlyVisibleFeaturesCheckBox->setChecked( mTable->sourceLayer() && mTable->sourceLayer()->geometryType() != QgsWkbTypes::NullGeometry && mTable->displayOnlyVisibleFeatures() );
      mComposerMapComboBox->setEnabled( mShowOnlyVisibleFeaturesCheckBox->isChecked() );
      mComposerMapLabel->setEnabled( mShowOnlyVisibleFeaturesCheckBox->isChecked() );
      mIntersectAtlasCheckBox->setEnabled( false );
      break;
    case QgsLayoutItemAttributeTable::RelationChildren:
      mLayerComboBox->setEnabled( false );
      mLayerComboBox->setVisible( false );
      mLayerLabel->setVisible( false );
      mLayerSourceDDBtn->setVisible( false );
      mRelationsComboBox->setEnabled( true );
      mRelationsComboBox->setVisible( true );
      mRelationLabel->setVisible( true );
      mMaximumRowsSpinBox->setEnabled( true );
      mMaxNumFeaturesLabel->setEnabled( true );
      //it's missing the check for null geometry of the referencing layer
      mShowOnlyVisibleFeaturesCheckBox->setEnabled( true );
      mComposerMapComboBox->setEnabled( mShowOnlyVisibleFeaturesCheckBox->isChecked() );
      mComposerMapLabel->setEnabled( mShowOnlyVisibleFeaturesCheckBox->isChecked() );
      mIntersectAtlasCheckBox->setEnabled( mSourceComboBox->findData( QgsLayoutItemAttributeTable::AtlasFeature ) != -1 && mTable->layout()->reportContext().layer() && mTable->layout()->reportContext().layer()->geometryType() != QgsWkbTypes::NullGeometry );
      break;
  }
}
