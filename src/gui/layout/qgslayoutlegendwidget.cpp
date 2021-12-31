/***************************************************************************
                         qgslayoutlegendwidget.cpp
                         -------------------------
    begin                : October 2017
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

#include "qgslayoutlegendwidget.h"
#include "qgslayoutitemlegend.h"
#include "qgslayoutlegendlayersdialog.h"
#include "qgslayoutitemwidget.h"
#include "qgslayoutitemmap.h"
#include "qgslayout.h"
#include "qgsguiutils.h"

#include "qgsapplication.h"
#include "qgslayertree.h"
#include "qgslayertreeutils.h"
#include "qgslayertreemodel.h"
#include "qgslayertreemodellegendnode.h"
#include "qgslegendrenderer.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayerlegend.h"
#include "qgsproject.h"
#include "qgsrenderer.h"
#include "qgsvectorlayer.h"
#include "qgslayoutatlas.h"
#include "qgslayoutitemlegend.h"
#include "qgslayoutmeasurementconverter.h"
#include "qgsunittypes.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgsexpressioncontextutils.h"
#include "qgslegendpatchshapewidget.h"
#include "qgslayertreefilterproxymodel.h"
#include "qgscolorramplegendnodewidget.h"

#include <QMenu>
#include <QMessageBox>
#include <QInputDialog>

///@cond PRIVATE

Q_GUI_EXPORT extern int qt_defaultDpiX();

static int _unfilteredLegendNodeIndex( QgsLayerTreeModelLegendNode *legendNode )
{
  return legendNode->model()->layerOriginalLegendNodes( legendNode->layerNode() ).indexOf( legendNode );
}

static int _originalLegendNodeIndex( QgsLayerTreeModelLegendNode *legendNode )
{
  // figure out index of the legend node as it comes out of the map layer legend.
  // first legend nodes may be reordered, output of that is available in layerOriginalLegendNodes().
  // next the nodes may be further filtered (by scale, map content etc).
  // so here we go in reverse order: 1. find index before filtering, 2. find index before reorder
  int unfilteredNodeIndex = _unfilteredLegendNodeIndex( legendNode );
  QList<int> order = QgsMapLayerLegendUtils::legendNodeOrder( legendNode->layerNode() );
  return ( unfilteredNodeIndex >= 0 && unfilteredNodeIndex < order.count() ? order[unfilteredNodeIndex] : -1 );
}


QgsLayoutLegendWidget::QgsLayoutLegendWidget( QgsLayoutItemLegend *legend, QgsMapCanvas *mapCanvas )
  : QgsLayoutItemBaseWidget( nullptr, legend )
  , mLegend( legend )
  , mMapCanvas( mapCanvas )
{
  Q_ASSERT( mLegend );

  setupUi( this );
  connect( mWrapCharLineEdit, &QLineEdit::textChanged, this, &QgsLayoutLegendWidget::mWrapCharLineEdit_textChanged );
  connect( mTitleLineEdit, &QLineEdit::textChanged, this, &QgsLayoutLegendWidget::mTitleLineEdit_textChanged );
  connect( mTitleAlignCombo, &QgsAlignmentComboBox::changed, this, &QgsLayoutLegendWidget::titleAlignmentChanged );
  connect( mGroupAlignCombo, &QgsAlignmentComboBox::changed, this, &QgsLayoutLegendWidget::groupAlignmentChanged );
  connect( mSubgroupAlignCombo, &QgsAlignmentComboBox::changed, this, &QgsLayoutLegendWidget::subgroupAlignmentChanged );
  connect( mItemAlignCombo, &QgsAlignmentComboBox::changed, this, &QgsLayoutLegendWidget::itemAlignmentChanged );
  connect( mColumnCountSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsLayoutLegendWidget::mColumnCountSpinBox_valueChanged );
  connect( mSplitLayerCheckBox, &QCheckBox::toggled, this, &QgsLayoutLegendWidget::mSplitLayerCheckBox_toggled );
  connect( mEqualColumnWidthCheckBox, &QCheckBox::toggled, this, &QgsLayoutLegendWidget::mEqualColumnWidthCheckBox_toggled );
  connect( mSymbolWidthSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutLegendWidget::mSymbolWidthSpinBox_valueChanged );
  connect( mSymbolHeightSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutLegendWidget::mSymbolHeightSpinBox_valueChanged );
  connect( mMaxSymbolSizeSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutLegendWidget::mMaxSymbolSizeSpinBox_valueChanged );
  connect( mMinSymbolSizeSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutLegendWidget::mMinSymbolSizeSpinBox_valueChanged );
  connect( mWmsLegendWidthSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutLegendWidget::mWmsLegendWidthSpinBox_valueChanged );
  connect( mWmsLegendHeightSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutLegendWidget::mWmsLegendHeightSpinBox_valueChanged );
  connect( mTitleSpaceBottomSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutLegendWidget::mTitleSpaceBottomSpinBox_valueChanged );
  connect( mGroupSpaceSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutLegendWidget::mGroupSpaceSpinBox_valueChanged );
  connect( mGroupIndentSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutLegendWidget::mGroupIndentSpinBox_valueChanged );
  connect( mSubgroupIndentSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutLegendWidget::mSubgroupIndentSpinBox_valueChanged );
  connect( mSpaceBelowGroupHeadingSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutLegendWidget::spaceBelowGroupHeadingChanged );
  connect( mGroupSideSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutLegendWidget::spaceGroupSideChanged );
  connect( mLayerSpaceSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutLegendWidget::mLayerSpaceSpinBox_valueChanged );
  connect( mSpaceBelowSubgroupHeadingSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutLegendWidget::spaceBelowSubGroupHeadingChanged );
  connect( mSubgroupSideSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutLegendWidget::spaceSubGroupSideChanged );
  connect( mSymbolSpaceSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutLegendWidget::mSymbolSpaceSpinBox_valueChanged );
  connect( mSymbolSideSpaceSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutLegendWidget::spaceSymbolSideChanged );
  connect( mIconLabelSpaceSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutLegendWidget::mIconLabelSpaceSpinBox_valueChanged );
  connect( mFontColorButton, &QgsColorButton::colorChanged, this, &QgsLayoutLegendWidget::mFontColorButton_colorChanged );
  connect( mBoxSpaceSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutLegendWidget::mBoxSpaceSpinBox_valueChanged );
  connect( mColumnSpaceSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutLegendWidget::mColumnSpaceSpinBox_valueChanged );
  connect( mLineSpacingSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutLegendWidget::mLineSpacingSpinBox_valueChanged );
  connect( mCheckBoxAutoUpdate, &QCheckBox::stateChanged, this, [ = ]( int state ) { mCheckBoxAutoUpdate_stateChanged( state ); } );
  connect( mCheckboxResizeContents, &QCheckBox::toggled, this, &QgsLayoutLegendWidget::mCheckboxResizeContents_toggled );
  connect( mRasterStrokeGroupBox, &QgsCollapsibleGroupBoxBasic::toggled, this, &QgsLayoutLegendWidget::mRasterStrokeGroupBox_toggled );
  connect( mRasterStrokeWidthSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutLegendWidget::mRasterStrokeWidthSpinBox_valueChanged );
  connect( mRasterStrokeColorButton, &QgsColorButton::colorChanged, this, &QgsLayoutLegendWidget::mRasterStrokeColorButton_colorChanged );
  connect( mMoveDownToolButton, &QToolButton::clicked, this, &QgsLayoutLegendWidget::mMoveDownToolButton_clicked );
  connect( mMoveUpToolButton, &QToolButton::clicked, this, &QgsLayoutLegendWidget::mMoveUpToolButton_clicked );
  connect( mRemoveToolButton, &QToolButton::clicked, this, &QgsLayoutLegendWidget::mRemoveToolButton_clicked );
  connect( mAddToolButton, &QToolButton::clicked, this, &QgsLayoutLegendWidget::mAddToolButton_clicked );
  connect( mEditPushButton, &QToolButton::clicked, this, &QgsLayoutLegendWidget::mEditPushButton_clicked );
  connect( mCountToolButton, &QToolButton::clicked, this, &QgsLayoutLegendWidget::mCountToolButton_clicked );
  connect( mExpressionFilterButton, &QgsLegendFilterButton::toggled, this, &QgsLayoutLegendWidget::mExpressionFilterButton_toggled );
  connect( mLayerExpressionButton, &QToolButton::clicked, this, &QgsLayoutLegendWidget::mLayerExpressionButton_clicked );
  connect( mFilterByMapCheckBox, &QCheckBox::toggled, this, &QgsLayoutLegendWidget::mFilterByMapCheckBox_toggled );
  connect( mUpdateAllPushButton, &QToolButton::clicked, this, &QgsLayoutLegendWidget::mUpdateAllPushButton_clicked );
  connect( mAddGroupToolButton, &QToolButton::clicked, this, &QgsLayoutLegendWidget::mAddGroupToolButton_clicked );
  connect( mFilterLegendByAtlasCheckBox, &QCheckBox::toggled, this, &QgsLayoutLegendWidget::mFilterLegendByAtlasCheckBox_toggled );
  connect( mItemTreeView, &QgsLayerTreeView::doubleClicked, this, &QgsLayoutLegendWidget::mItemTreeView_doubleClicked );
  setPanelTitle( tr( "Legend Properties" ) );

  mTitleFontButton->setMode( QgsFontButton::ModeQFont );
  mGroupFontButton->setMode( QgsFontButton::ModeQFont );
  mLayerFontButton->setMode( QgsFontButton::ModeQFont );
  mItemFontButton->setMode( QgsFontButton::ModeQFont );

  mTitleAlignCombo->setAvailableAlignments( Qt::AlignLeft | Qt::AlignHCenter | Qt::AlignRight );
  mGroupAlignCombo->setAvailableAlignments( Qt::AlignLeft | Qt::AlignHCenter | Qt::AlignRight );
  mSubgroupAlignCombo->setAvailableAlignments( Qt::AlignLeft | Qt::AlignHCenter | Qt::AlignRight );
  mItemAlignCombo->setAvailableAlignments( Qt::AlignLeft | Qt::AlignHCenter | Qt::AlignRight );

  mArrangementCombo->setAvailableAlignments( Qt::AlignLeft | Qt::AlignRight );
  connect( mArrangementCombo, &QgsAlignmentComboBox::changed, this, &QgsLayoutLegendWidget::arrangementChanged );
  mArrangementCombo->customizeAlignmentDisplay( Qt::AlignLeft, tr( "Symbols on Left" ), QgsApplication::getThemeIcon( QStringLiteral( "/mIconArrangeSymbolsLeft.svg" ) ) );
  mArrangementCombo->customizeAlignmentDisplay( Qt::AlignRight, tr( "Symbols on Right" ), QgsApplication::getThemeIcon( QStringLiteral( "/mIconArrangeSymbolsRight.svg" ) ) );

  mSpaceBelowGroupHeadingSpinBox->setClearValue( 0 );
  mGroupSideSpinBox->setClearValue( 0 );
  mSpaceBelowSubgroupHeadingSpinBox->setClearValue( 0 );
  mSubgroupSideSpinBox->setClearValue( 0 );
  mSymbolSideSpaceSpinBox->setClearValue( 0 );

  // setup icons
  mAddToolButton->setIcon( QIcon( QgsApplication::iconPath( "symbologyAdd.svg" ) ) );
  mEditPushButton->setIcon( QIcon( QgsApplication::iconPath( "symbologyEdit.svg" ) ) );
  mRemoveToolButton->setIcon( QIcon( QgsApplication::iconPath( "symbologyRemove.svg" ) ) );
  mMoveUpToolButton->setIcon( QIcon( QgsApplication::iconPath( "mActionArrowUp.svg" ) ) );
  mMoveDownToolButton->setIcon( QIcon( QgsApplication::iconPath( "mActionArrowDown.svg" ) ) );
  mCountToolButton->setIcon( QIcon( QgsApplication::iconPath( "mActionSum.svg" ) ) );
  mLayerExpressionButton->setIcon( QIcon( QgsApplication::iconPath( "mIconExpression.svg" ) ) );

  mMoveDownToolButton->setIconSize( QgsGuiUtils::iconSize( true ) );
  mMoveUpToolButton->setIconSize( QgsGuiUtils::iconSize( true ) );
  mAddGroupToolButton->setIconSize( QgsGuiUtils::iconSize( true ) );
  mAddToolButton->setIconSize( QgsGuiUtils::iconSize( true ) );
  mRemoveToolButton->setIconSize( QgsGuiUtils::iconSize( true ) );
  mEditPushButton->setIconSize( QgsGuiUtils::iconSize( true ) );
  mCountToolButton->setIconSize( QgsGuiUtils::iconSize( true ) );
  mExpressionFilterButton->setIconSize( QgsGuiUtils::iconSize( true ) );
  mLayerExpressionButton->setIconSize( QgsGuiUtils::iconSize( true ) );

  mFontColorButton->setColorDialogTitle( tr( "Select Font Color" ) );
  mFontColorButton->setContext( QStringLiteral( "composer" ) );

  mRasterStrokeColorButton->setColorDialogTitle( tr( "Select Stroke Color" ) );
  mRasterStrokeColorButton->setAllowOpacity( true );
  mRasterStrokeColorButton->setContext( QStringLiteral( "composer " ) );

  mMapComboBox->setCurrentLayout( legend->layout() );
  mMapComboBox->setItemType( QgsLayoutItemRegistry::LayoutMap );
  connect( mMapComboBox, &QgsLayoutItemComboBox::itemChanged, this, &QgsLayoutLegendWidget::composerMapChanged );

  //add widget for general composer item properties
  mItemPropertiesWidget = new QgsLayoutItemPropertiesWidget( this, legend );
  mainLayout->addWidget( mItemPropertiesWidget );

  mItemTreeView->setHeaderHidden( true );

  mItemTreeView->setModel( legend->model() );
  mItemTreeView->setMenuProvider( new QgsLayoutLegendMenuProvider( mItemTreeView, this ) );
  setLegendMapViewData();
  connect( legend, &QgsLayoutObject::changed, this, &QgsLayoutLegendWidget::setGuiElements );

  // connect atlas state to the filter legend by atlas checkbox
  if ( layoutAtlas() )
  {
    connect( layoutAtlas(), &QgsLayoutAtlas::toggled, this, &QgsLayoutLegendWidget::updateFilterLegendByAtlasButton );
  }
  connect( &legend->layout()->reportContext(), &QgsLayoutReportContext::layerChanged, this, &QgsLayoutLegendWidget::updateFilterLegendByAtlasButton );

  registerDataDefinedButton( mLegendTitleDDBtn, QgsLayoutObject::LegendTitle );
  registerDataDefinedButton( mColumnsDDBtn, QgsLayoutObject::LegendColumnCount );

  setGuiElements();

  connect( mItemTreeView->selectionModel(), &QItemSelectionModel::currentChanged,
           this, &QgsLayoutLegendWidget::selectedChanged );
  connect( mTitleFontButton, &QgsFontButton::changed, this, &QgsLayoutLegendWidget::titleFontChanged );
  connect( mGroupFontButton, &QgsFontButton::changed, this, &QgsLayoutLegendWidget::groupFontChanged );
  connect( mLayerFontButton, &QgsFontButton::changed, this, &QgsLayoutLegendWidget::layerFontChanged );
  connect( mItemFontButton, &QgsFontButton::changed, this, &QgsLayoutLegendWidget::itemFontChanged );
}

void QgsLayoutLegendWidget::setMasterLayout( QgsMasterLayoutInterface *masterLayout )
{
  if ( mItemPropertiesWidget )
    mItemPropertiesWidget->setMasterLayout( masterLayout );
}

void QgsLayoutLegendWidget::setGuiElements()
{
  if ( !mLegend )
  {
    return;
  }

  blockAllSignals( true );
  mTitleLineEdit->setText( mLegend->title() );
  whileBlocking( mTitleAlignCombo )->setCurrentAlignment( mLegend->titleAlignment() );
  whileBlocking( mGroupAlignCombo )->setCurrentAlignment( mLegend->style( QgsLegendStyle::Group ).alignment() );
  whileBlocking( mSubgroupAlignCombo )->setCurrentAlignment( mLegend->style( QgsLegendStyle::Subgroup ).alignment() );
  whileBlocking( mItemAlignCombo )->setCurrentAlignment( mLegend->style( QgsLegendStyle::SymbolLabel ).alignment() );
  whileBlocking( mArrangementCombo )->setCurrentAlignment( mLegend->symbolAlignment() );
  mFilterByMapCheckBox->setChecked( mLegend->legendFilterByMapEnabled() );
  mColumnCountSpinBox->setValue( mLegend->columnCount() );
  mSplitLayerCheckBox->setChecked( mLegend->splitLayer() );
  mEqualColumnWidthCheckBox->setChecked( mLegend->equalColumnWidth() );
  mSymbolWidthSpinBox->setValue( mLegend->symbolWidth() );
  mSymbolHeightSpinBox->setValue( mLegend->symbolHeight() );
  mMaxSymbolSizeSpinBox->setValue( mLegend->maximumSymbolSize() );
  mMinSymbolSizeSpinBox->setValue( mLegend->minimumSymbolSize() );
  mWmsLegendWidthSpinBox->setValue( mLegend->wmsLegendWidth() );
  mWmsLegendHeightSpinBox->setValue( mLegend->wmsLegendHeight() );
  mTitleSpaceBottomSpinBox->setValue( mLegend->style( QgsLegendStyle::Title ).margin( QgsLegendStyle::Bottom ) );
  mGroupSpaceSpinBox->setValue( mLegend->style( QgsLegendStyle::Group ).margin( QgsLegendStyle::Top ) );
  mGroupIndentSpinBox->setValue( mLegend->style( QgsLegendStyle::Group ).indent() );
  mSubgroupIndentSpinBox->setValue( mLegend->style( QgsLegendStyle::Subgroup ).indent() );
  mGroupSideSpinBox->setValue( mLegend->style( QgsLegendStyle::Group ).margin( QgsLegendStyle::Left ) );
  mSpaceBelowGroupHeadingSpinBox->setValue( mLegend->style( QgsLegendStyle::Group ).margin( QgsLegendStyle::Bottom ) );
  mLayerSpaceSpinBox->setValue( mLegend->style( QgsLegendStyle::Subgroup ).margin( QgsLegendStyle::Top ) );
  mSpaceBelowSubgroupHeadingSpinBox->setValue( mLegend->style( QgsLegendStyle::Subgroup ).margin( QgsLegendStyle::Bottom ) );
  mSubgroupSideSpinBox->setValue( mLegend->style( QgsLegendStyle::Subgroup ).margin( QgsLegendStyle::Left ) );
  // We keep Symbol and SymbolLabel Top in sync for now
  mSymbolSpaceSpinBox->setValue( mLegend->style( QgsLegendStyle::Symbol ).margin( QgsLegendStyle::Top ) );
  mIconLabelSpaceSpinBox->setValue( mLegend->style( QgsLegendStyle::SymbolLabel ).margin( QgsLegendStyle::Left ) );
  mSymbolSideSpaceSpinBox->setValue( mLegend->style( QgsLegendStyle::Symbol ).margin( QgsLegendStyle::Left ) );
  mBoxSpaceSpinBox->setValue( mLegend->boxSpace() );
  mColumnSpaceSpinBox->setValue( mLegend->columnSpace() );
  mLineSpacingSpinBox->setValue( mLegend->lineSpacing() );

  mRasterStrokeGroupBox->setChecked( mLegend->drawRasterStroke() );
  mRasterStrokeWidthSpinBox->setValue( mLegend->rasterStrokeWidth() );
  mRasterStrokeColorButton->setColor( mLegend->rasterStrokeColor() );

  mCheckBoxAutoUpdate->setChecked( mLegend->autoUpdateModel() );

  mCheckboxResizeContents->setChecked( mLegend->resizeToContents() );
  mFilterLegendByAtlasCheckBox->setChecked( mLegend->legendFilterOutAtlas() );
  mWrapCharLineEdit->setText( mLegend->wrapString() );

  QgsLayoutItemMap *map = mLegend->linkedMap();
  mMapComboBox->setItem( map );
  mFontColorButton->setColor( mLegend->fontColor() );
  mTitleFontButton->setCurrentFont( mLegend->style( QgsLegendStyle::Title ).font() );
  mGroupFontButton->setCurrentFont( mLegend->style( QgsLegendStyle::Group ).font() );
  mLayerFontButton->setCurrentFont( mLegend->style( QgsLegendStyle::Subgroup ).font() );
  mItemFontButton->setCurrentFont( mLegend->style( QgsLegendStyle::SymbolLabel ).font() );

  blockAllSignals( false );

  mCheckBoxAutoUpdate_stateChanged( mLegend->autoUpdateModel() ? Qt::Checked : Qt::Unchecked, false );
  updateDataDefinedButton( mLegendTitleDDBtn );
  updateDataDefinedButton( mColumnsDDBtn );
}

void QgsLayoutLegendWidget::mWrapCharLineEdit_textChanged( const QString &text )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Change Legend Wrap" ) );
    mLegend->setWrapString( text );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::mTitleLineEdit_textChanged( const QString &text )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Change Legend Title" ), QgsLayoutItem::UndoLegendText );
    mLegend->setTitle( text );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::titleAlignmentChanged()
{
  if ( mLegend )
  {
    Qt::AlignmentFlag alignment = static_cast< Qt::AlignmentFlag >( static_cast< int  >( mTitleAlignCombo->currentAlignment() & Qt::AlignHorizontal_Mask ) );
    mLegend->beginCommand( tr( "Change Title Alignment" ) );
    mLegend->setTitleAlignment( alignment );
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::groupAlignmentChanged()
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Change Group Alignment" ) );
    mLegend->rstyle( QgsLegendStyle::Group ).setAlignment( mGroupAlignCombo->currentAlignment() );
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::subgroupAlignmentChanged()
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Change Subgroup Alignment" ) );
    mLegend->rstyle( QgsLegendStyle::Subgroup ).setAlignment( mSubgroupAlignCombo->currentAlignment() );
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::itemAlignmentChanged()
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Change Item Alignment" ) );
    mLegend->rstyle( QgsLegendStyle::SymbolLabel ).setAlignment( mItemAlignCombo->currentAlignment() );
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::arrangementChanged()
{
  if ( mLegend )
  {
    Qt::AlignmentFlag alignment = static_cast< Qt::AlignmentFlag >( static_cast< int  >( mArrangementCombo->currentAlignment() & Qt::AlignHorizontal_Mask ) );
    mLegend->beginCommand( tr( "Change Legend Arrangement" ) );
    mLegend->setSymbolAlignment( alignment );
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::mColumnCountSpinBox_valueChanged( int c )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Change Column Count" ), QgsLayoutItem::UndoLegendColumnCount );
    mLegend->setColumnCount( c );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
  mSplitLayerCheckBox->setEnabled( c > 1 );
  mEqualColumnWidthCheckBox->setEnabled( c > 1 );
}

void QgsLayoutLegendWidget::mSplitLayerCheckBox_toggled( bool checked )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Split Legend Layers" ) );
    mLegend->setSplitLayer( checked );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::mEqualColumnWidthCheckBox_toggled( bool checked )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Legend Column Width" ) );
    mLegend->setEqualColumnWidth( checked );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::mSymbolWidthSpinBox_valueChanged( double d )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Resize Symbol Width" ), QgsLayoutItem::UndoLegendSymbolWidth );
    mLegend->setSymbolWidth( d );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::mMaxSymbolSizeSpinBox_valueChanged( double d )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Change Legend Maximum Symbol Size" ), QgsLayoutItem::UndoLegendMaxSymbolSize );
    mLegend->setMaximumSymbolSize( d );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::mMinSymbolSizeSpinBox_valueChanged( double d )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Change Legend Minimum Symbol Size" ), QgsLayoutItem::UndoLegendMinSymbolSize );
    mLegend->setMinimumSymbolSize( d );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::mSymbolHeightSpinBox_valueChanged( double d )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Resize Symbol Height" ), QgsLayoutItem::UndoLegendSymbolHeight );
    mLegend->setSymbolHeight( d );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::mWmsLegendWidthSpinBox_valueChanged( double d )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Resize WMS Width" ), QgsLayoutItem::UndoLegendWmsLegendWidth );
    mLegend->setWmsLegendWidth( d );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::mWmsLegendHeightSpinBox_valueChanged( double d )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Resize WMS Height" ), QgsLayoutItem::UndoLegendWmsLegendHeight );
    mLegend->setWmsLegendHeight( d );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::mTitleSpaceBottomSpinBox_valueChanged( double d )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Change Title Space" ), QgsLayoutItem::UndoLegendTitleSpaceBottom );
    mLegend->rstyle( QgsLegendStyle::Title ).setMargin( QgsLegendStyle::Bottom, d );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::mGroupSpaceSpinBox_valueChanged( double d )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Change Group Space" ), QgsLayoutItem::UndoLegendGroupSpace );
    mLegend->rstyle( QgsLegendStyle::Group ).setMargin( QgsLegendStyle::Top, d );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::mGroupIndentSpinBox_valueChanged( double d )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Change Group Indent" ), QgsLayoutItem::UndoLegendGroupIndent );
    mLegend->rstyle( QgsLegendStyle::Group ).setIndent( d );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::mSubgroupIndentSpinBox_valueChanged( double d )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Change Subgroup Indent" ), QgsLayoutItem::UndoLegendSubgroupIndent );
    mLegend->rstyle( QgsLegendStyle::Subgroup ).setIndent( d );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::spaceBelowGroupHeadingChanged( double space )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Change Group Space" ), QgsLayoutItem::UndoLegendGroupSpace );
    mLegend->rstyle( QgsLegendStyle::Group ).setMargin( QgsLegendStyle::Bottom, space );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::spaceGroupSideChanged( double space )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Change Side of Group Space" ), QgsLayoutItem::UndoLegendGroupSpace );
    mLegend->rstyle( QgsLegendStyle::Group ).setMargin( QgsLegendStyle::Left, space );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::spaceSubGroupSideChanged( double space )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Change Side of Subgroup Space" ), QgsLayoutItem::UndoLegendLayerSpace );
    mLegend->rstyle( QgsLegendStyle::Subgroup ).setMargin( QgsLegendStyle::Left, space );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::spaceSymbolSideChanged( double space )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Change Side of Symbol Space" ), QgsLayoutItem::UndoLegendSymbolSpace );
    mLegend->rstyle( QgsLegendStyle::Symbol ).setMargin( QgsLegendStyle::Left, space );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::mLayerSpaceSpinBox_valueChanged( double d )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Change Subgroup Space" ), QgsLayoutItem::UndoLegendLayerSpace );
    mLegend->rstyle( QgsLegendStyle::Subgroup ).setMargin( QgsLegendStyle::Top, d );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::mSymbolSpaceSpinBox_valueChanged( double d )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Change Symbol Space" ), QgsLayoutItem::UndoLegendSymbolSpace );
    // We keep Symbol and SymbolLabel Top in sync for now
    mLegend->rstyle( QgsLegendStyle::Symbol ).setMargin( QgsLegendStyle::Top, d );
    mLegend->rstyle( QgsLegendStyle::SymbolLabel ).setMargin( QgsLegendStyle::Top, d );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::mIconLabelSpaceSpinBox_valueChanged( double d )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Change Label Space" ), QgsLayoutItem::UndoLegendIconSymbolSpace );
    mLegend->rstyle( QgsLegendStyle::SymbolLabel ).setMargin( QgsLegendStyle::Left, d );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::titleFontChanged()
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Change Title Font" ), QgsLayoutItem::UndoLegendTitleFont );
    mLegend->setStyleFont( QgsLegendStyle::Title, mTitleFontButton->currentFont() );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::groupFontChanged()
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Change Group Font" ), QgsLayoutItem::UndoLegendGroupFont );
    mLegend->setStyleFont( QgsLegendStyle::Group, mGroupFontButton->currentFont() );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::layerFontChanged()
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Change Layer Font" ), QgsLayoutItem::UndoLegendLayerFont );
    mLegend->setStyleFont( QgsLegendStyle::Subgroup, mLayerFontButton->currentFont() );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::itemFontChanged()
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Change Item Font" ), QgsLayoutItem::UndoLegendItemFont );
    mLegend->setStyleFont( QgsLegendStyle::SymbolLabel, mItemFontButton->currentFont() );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::spaceBelowSubGroupHeadingChanged( double space )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Change Subgroup Space" ), QgsLayoutItem::UndoLegendLayerSpace );
    mLegend->rstyle( QgsLegendStyle::Subgroup ).setMargin( QgsLegendStyle::Bottom, space );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::mFontColorButton_colorChanged( const QColor &newFontColor )
{
  if ( !mLegend )
  {
    return;
  }

  mLegend->beginCommand( tr( "Change Font Color" ), QgsLayoutItem::UndoLegendFontColor );
  mLegend->setFontColor( newFontColor );
  mLegend->update();
  mLegend->endCommand();
}

void QgsLayoutLegendWidget::mBoxSpaceSpinBox_valueChanged( double d )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Change Box Space" ), QgsLayoutItem::UndoLegendBoxSpace );
    mLegend->setBoxSpace( d );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::mColumnSpaceSpinBox_valueChanged( double d )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Change Column Space" ), QgsLayoutItem::UndoLegendColumnSpace );
    mLegend->setColumnSpace( d );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::mLineSpacingSpinBox_valueChanged( double d )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Change Line Space" ), QgsLayoutItem::UndoLegendLineSpacing );
    mLegend->setLineSpacing( d );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

static void _moveLegendNode( QgsLayerTreeLayer *nodeLayer, int legendNodeIndex, int offset )
{
  QList<int> order = QgsMapLayerLegendUtils::legendNodeOrder( nodeLayer );

  if ( legendNodeIndex < 0 || legendNodeIndex >= order.count() )
    return;
  if ( legendNodeIndex + offset < 0 || legendNodeIndex + offset >= order.count() )
    return;

  int id = order.takeAt( legendNodeIndex );
  order.insert( legendNodeIndex + offset, id );

  QgsMapLayerLegendUtils::setLegendNodeOrder( nodeLayer, order );
}


void QgsLayoutLegendWidget::mMoveDownToolButton_clicked()
{
  if ( !mLegend )
  {
    return;
  }

  const QModelIndex index = mItemTreeView->selectionModel()->currentIndex();
  const QModelIndex sourceIndex = mItemTreeView->proxyModel()->mapToSource( index );
  const QModelIndex parentIndex = sourceIndex.parent();
  if ( !sourceIndex.isValid() || sourceIndex.row() == mItemTreeView->layerTreeModel()->rowCount( parentIndex ) - 1 )
    return;

  QgsLayerTreeNode *node = mItemTreeView->index2node( index );
  QgsLayerTreeModelLegendNode *legendNode = mItemTreeView->index2legendNode( index );
  if ( !node && !legendNode )
    return;

  mLegend->beginCommand( tr( "Moved Legend Item Down" ) );

  if ( node )
  {
    QgsLayerTreeGroup *parentGroup = QgsLayerTree::toGroup( node->parent() );
    parentGroup->insertChildNode( sourceIndex.row() + 2, node->clone() );
    parentGroup->removeChildNode( node );
  }
  else // legend node
  {
    _moveLegendNode( legendNode->layerNode(), _unfilteredLegendNodeIndex( legendNode ), 1 );
    mItemTreeView->layerTreeModel()->refreshLayerLegend( legendNode->layerNode() );
  }

  mItemTreeView->setCurrentIndex( mItemTreeView->proxyModel()->mapFromSource( mItemTreeView->layerTreeModel()->index( sourceIndex.row() + 1, 0, parentIndex ) ) );

  mLegend->update();
  mLegend->endCommand();
}

void QgsLayoutLegendWidget::mMoveUpToolButton_clicked()
{
  if ( !mLegend )
  {
    return;
  }

  const QModelIndex index = mItemTreeView->selectionModel()->currentIndex();
  const QModelIndex sourceIndex = mItemTreeView->proxyModel()->mapToSource( index );
  const QModelIndex parentIndex = sourceIndex.parent();
  if ( !sourceIndex.isValid() || sourceIndex.row() == 0 )
    return;

  QgsLayerTreeNode *node = mItemTreeView->index2node( index );
  QgsLayerTreeModelLegendNode *legendNode = mItemTreeView->index2legendNode( index );
  if ( !node && !legendNode )
    return;

  mLegend->beginCommand( tr( "Move Legend Item Up" ) );

  if ( node )
  {
    QgsLayerTreeGroup *parentGroup = QgsLayerTree::toGroup( node->parent() );
    parentGroup->insertChildNode( sourceIndex.row() - 1, node->clone() );
    parentGroup->removeChildNode( node );
  }
  else // legend node
  {
    _moveLegendNode( legendNode->layerNode(), _unfilteredLegendNodeIndex( legendNode ), -1 );
    mItemTreeView->layerTreeModel()->refreshLayerLegend( legendNode->layerNode() );
  }

  mItemTreeView->setCurrentIndex( mItemTreeView->proxyModel()->mapFromSource( mItemTreeView->layerTreeModel()->index( sourceIndex.row() - 1, 0, parentIndex ) ) );

  mLegend->update();
  mLegend->endCommand();
}

void QgsLayoutLegendWidget::mCheckBoxAutoUpdate_stateChanged( int state, bool userTriggered )
{
  if ( userTriggered )
  {
    mLegend->beginCommand( tr( "Change Auto Update" ) );

    mLegend->setAutoUpdateModel( state == Qt::Checked );

    mLegend->updateFilterByMap();
    mLegend->endCommand();
  }

  // do not allow editing of model if auto update is on - we would modify project's layer tree
  QList<QWidget *> widgets;
  widgets << mMoveDownToolButton << mMoveUpToolButton << mRemoveToolButton << mAddToolButton
          << mEditPushButton << mCountToolButton << mUpdateAllPushButton << mAddGroupToolButton
          << mExpressionFilterButton;
  for ( QWidget *w : std::as_const( widgets ) )
    w->setEnabled( state != Qt::Checked );

  if ( state == Qt::Unchecked )
  {
    // update widgets state based on current selection
    selectedChanged( QModelIndex(), QModelIndex() );
  }
}

void QgsLayoutLegendWidget::composerMapChanged( QgsLayoutItem *item )
{
  if ( !mLegend )
  {
    return;
  }

  QgsLayout *layout = mLegend->layout();
  if ( !layout )
  {
    return;
  }

  QgsLayoutItemMap *map = qobject_cast< QgsLayoutItemMap * >( item );
  if ( map )
  {
    mLegend->beginCommand( tr( "Change Legend Map" ) );
    mLegend->setLinkedMap( map );
    mLegend->updateFilterByMap();
    mLegend->endCommand();

    setLegendMapViewData();
  }
}

void QgsLayoutLegendWidget::mCheckboxResizeContents_toggled( bool checked )
{
  if ( !mLegend )
  {
    return;
  }

  mLegend->beginCommand( tr( "Resize Legend to Contents" ) );
  mLegend->setResizeToContents( checked );
  if ( checked )
    mLegend->adjustBoxSize();
  mLegend->updateFilterByMap();
  mLegend->endCommand();
}

void QgsLayoutLegendWidget::mRasterStrokeGroupBox_toggled( bool state )
{
  if ( !mLegend )
  {
    return;
  }

  mLegend->beginCommand( tr( "Change Legend Borders" ) );
  mLegend->setDrawRasterStroke( state );
  mLegend->adjustBoxSize();
  mLegend->update();
  mLegend->endCommand();
}

void QgsLayoutLegendWidget::mRasterStrokeWidthSpinBox_valueChanged( double d )
{
  if ( !mLegend )
  {
    return;
  }

  mLegend->beginCommand( tr( "Resize Legend Borders" ), QgsLayoutItem::UndoLegendRasterStrokeWidth );
  mLegend->setRasterStrokeWidth( d );
  mLegend->adjustBoxSize();
  mLegend->update();
  mLegend->endCommand();
}

void QgsLayoutLegendWidget::mRasterStrokeColorButton_colorChanged( const QColor &newColor )
{
  if ( !mLegend )
  {
    return;
  }

  mLegend->beginCommand( tr( "Change Legend Border Color" ), QgsLayoutItem::UndoLegendRasterStrokeColor );
  mLegend->setRasterStrokeColor( newColor );
  mLegend->update();
  mLegend->endCommand();
}

void QgsLayoutLegendWidget::mAddToolButton_clicked()
{
  if ( !mLegend )
  {
    return;
  }

  QList< QgsMapLayer * > visibleLayers;
  if ( mLegend->linkedMap() )
  {
    visibleLayers = mLegend->linkedMap()->layersToRender();
  }
  if ( visibleLayers.isEmpty() )
  {
    // just use current canvas layers as visible layers
    visibleLayers = mMapCanvas->layers( true );
  }

  QgsLayoutLegendLayersDialog addDialog( this );
  addDialog.setVisibleLayers( visibleLayers );
  if ( addDialog.exec() == QDialog::Accepted )
  {
    const QList<QgsMapLayer *> layers = addDialog.selectedLayers();
    if ( !layers.empty() )
    {
      mLegend->beginCommand( tr( "Add Legend Item(s)" ) );
      for ( QgsMapLayer *layer : layers )
      {
        mLegend->model()->rootGroup()->addLayer( layer );
      }
      mLegend->updateLegend();
      mLegend->update();
      mLegend->endCommand();
    }
  }
}

void QgsLayoutLegendWidget::mRemoveToolButton_clicked()
{
  if ( !mLegend )
  {
    return;
  }

  QItemSelectionModel *selectionModel = mItemTreeView->selectionModel();
  if ( !selectionModel )
  {
    return;
  }

  mLegend->beginCommand( tr( "Remove Legend Item" ) );

  QList<QPersistentModelIndex> proxyIndexes;
  const QModelIndexList viewSelection = selectionModel->selectedIndexes();
  for ( const QModelIndex &index : viewSelection )
    proxyIndexes << index;

  // first try to remove legend nodes
  QHash<QgsLayerTreeLayer *, QList<int> > nodesWithRemoval;
  for ( const QPersistentModelIndex &proxyIndex : std::as_const( proxyIndexes ) )
  {
    if ( QgsLayerTreeModelLegendNode *legendNode = mItemTreeView->index2legendNode( proxyIndex ) )
    {
      QgsLayerTreeLayer *nodeLayer = legendNode->layerNode();
      nodesWithRemoval[nodeLayer].append( _unfilteredLegendNodeIndex( legendNode ) );
    }
  }
  for ( auto it = nodesWithRemoval.constBegin(); it != nodesWithRemoval.constEnd(); ++it )
  {
    QList<int> toDelete = it.value();
    std::sort( toDelete.begin(), toDelete.end(), std::greater<int>() );
    QList<int> order = QgsMapLayerLegendUtils::legendNodeOrder( it.key() );

    for ( int i : std::as_const( toDelete ) )
    {
      if ( i >= 0 && i < order.count() )
        order.removeAt( i );
    }

    QgsMapLayerLegendUtils::setLegendNodeOrder( it.key(), order );
    mItemTreeView->layerTreeModel()->refreshLayerLegend( it.key() );
  }

  // then remove layer tree nodes
  for ( const QPersistentModelIndex &proxyIndex : std::as_const( proxyIndexes ) )
  {
    if ( proxyIndex.isValid() && mItemTreeView->index2node( proxyIndex ) )
    {
      const QModelIndex sourceIndex = mItemTreeView->proxyModel()->mapToSource( proxyIndex );
      mLegend->model()->removeRow( sourceIndex.row(), sourceIndex.parent() );
    }
  }

  mLegend->updateLegend();
  mLegend->update();
  mLegend->endCommand();
}

void QgsLayoutLegendWidget::mEditPushButton_clicked()
{
  if ( !mLegend )
  {
    return;
  }

  QModelIndex idx = mItemTreeView->selectionModel()->currentIndex();
  mItemTreeView_doubleClicked( idx );
}

void QgsLayoutLegendWidget::resetLayerNodeToDefaults()
{
  if ( !mLegend )
  {
    return;
  }

  //get current item
  QModelIndex currentIndex = mItemTreeView->currentIndex();
  if ( !currentIndex.isValid() )
  {
    return;
  }

  QgsLayerTreeLayer *nodeLayer = nullptr;
  if ( QgsLayerTreeNode *node = mItemTreeView->index2node( currentIndex ) )
  {
    if ( QgsLayerTree::isLayer( node ) )
      nodeLayer = QgsLayerTree::toLayer( node );
  }
  if ( QgsLayerTreeModelLegendNode *legendNode = mItemTreeView->index2legendNode( currentIndex ) )
  {
    nodeLayer = legendNode->layerNode();
  }

  if ( !nodeLayer )
    return;

  mLegend->beginCommand( tr( "Update Legend" ) );

  const auto constCustomProperties = nodeLayer->customProperties();
  for ( const QString &key : constCustomProperties )
  {
    if ( key.startsWith( QLatin1String( "legend/" ) ) )
      nodeLayer->removeCustomProperty( key );
  }

  nodeLayer->setPatchShape( QgsLegendPatchShape() );
  nodeLayer->setPatchSize( QSizeF() );

  mItemTreeView->layerTreeModel()->refreshLayerLegend( nodeLayer );

  mLegend->updateLegend();
  mLegend->update();
  mLegend->endCommand();
}

void QgsLayoutLegendWidget::mCountToolButton_clicked( bool checked )
{
  if ( !mLegend )
  {
    return;
  }

  const QList< QModelIndex > selectedIndexes = mItemTreeView->selectionModel()->selectedIndexes();
  if ( selectedIndexes.empty() )
    return;

  mLegend->beginCommand( tr( "Update Legend" ) );
  for ( const QModelIndex &index : selectedIndexes )
  {
    QgsLayerTreeNode *currentNode = mItemTreeView->index2node( index );
    if ( !QgsLayerTree::isLayer( currentNode ) )
      continue;

    currentNode->setCustomProperty( QStringLiteral( "showFeatureCount" ), checked ? 1 : 0 );
  }
  mLegend->updateFilterByMap();
  mLegend->adjustBoxSize();
  mLegend->endCommand();
}

void QgsLayoutLegendWidget::mFilterByMapCheckBox_toggled( bool checked )
{
  mLegend->beginCommand( tr( "Update Legend" ) );
  mLegend->setLegendFilterByMapEnabled( checked );
  mLegend->adjustBoxSize();
  mLegend->update();
  mLegend->endCommand();
}

void QgsLayoutLegendWidget::mExpressionFilterButton_toggled( bool checked )
{
  if ( !mLegend )
  {
    return;
  }

  //get current item
  QModelIndex currentIndex = mItemTreeView->currentIndex();
  if ( !currentIndex.isValid() )
  {
    return;
  }

  QgsLayerTreeNode *currentNode = mItemTreeView->currentNode();
  if ( !QgsLayerTree::isLayer( currentNode ) )
    return;

  QgsLayerTreeUtils::setLegendFilterByExpression( *qobject_cast<QgsLayerTreeLayer *>( currentNode ),
      mExpressionFilterButton->expressionText(),
      checked );

  mLegend->beginCommand( tr( "Update Legend" ) );
  mLegend->updateFilterByMap();
  mLegend->adjustBoxSize();
  mLegend->endCommand();
}

void QgsLayoutLegendWidget::mLayerExpressionButton_clicked()
{
  if ( !mLegend )
  {
    return;
  }

  QModelIndex currentIndex = mItemTreeView->currentIndex();
  if ( !currentIndex.isValid() )
    return;

  QgsLayerTreeNode *currentNode = mItemTreeView->currentNode();
  if ( !QgsLayerTree::isLayer( currentNode ) )
    return;

  QgsLayerTreeLayer *layerNode = qobject_cast<QgsLayerTreeLayer *>( currentNode );
  QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layerNode->layer() );

  if ( !vl )
    return;

  QString currentExpression;
  if ( layerNode->labelExpression().isEmpty() )
    currentExpression = QStringLiteral( "@symbol_label" );
  else
    currentExpression = layerNode->labelExpression();
  QgsExpressionContext legendContext = mLegend->createExpressionContext();
  legendContext.appendScope( vl->createExpressionContextScope() );

  QgsExpressionContextScope *symbolLegendScope = new QgsExpressionContextScope( tr( "Symbol scope" ) );

  QgsFeatureRenderer *r = vl->renderer();

  QStringList highlighted;
  if ( r )
  {
    const QgsLegendSymbolList legendSymbols = r->legendSymbolItems();

    if ( !legendSymbols.empty() )
    {
      QgsSymbolLegendNode legendNode( layerNode, legendSymbols.first() );

      symbolLegendScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "symbol_label" ), legendNode.symbolLabel().remove( QStringLiteral( "[%" ) ).remove( QStringLiteral( "%]" ) ), true ) );
      symbolLegendScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "symbol_id" ), legendSymbols.first().ruleKey(), true ) );
      highlighted << QStringLiteral( "symbol_label" ) << QStringLiteral( "symbol_id" );
      symbolLegendScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "symbol_count" ), QVariant::fromValue( vl->featureCount( legendSymbols.first().ruleKey() ) ), true ) );
      highlighted << QStringLiteral( "symbol_count" );
    }
  }

  legendContext.appendScope( symbolLegendScope );

  legendContext.setHighlightedVariables( highlighted );

  QgsExpressionBuilderDialog expressiondialog( vl, currentExpression, nullptr, QStringLiteral( "generic" ), legendContext );
  if ( expressiondialog.exec() )
    layerNode->setLabelExpression( expressiondialog.expressionText() );

  mLegend->beginCommand( tr( "Update Legend" ) );
  mLegend->refresh();
  mLegend->adjustBoxSize();
  mLegend->endCommand();
}

void QgsLayoutLegendWidget::mUpdateAllPushButton_clicked()
{
  updateLegend();
}

void QgsLayoutLegendWidget::mAddGroupToolButton_clicked()
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Add Legend Group" ) );
    mLegend->model()->rootGroup()->addGroup( tr( "Group" ) );
    mLegend->updateLegend();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::mFilterLegendByAtlasCheckBox_toggled( bool toggled )
{
  Q_UNUSED( toggled )
  if ( mLegend )
  {
    mLegend->setLegendFilterOutAtlas( toggled );
    // force update of legend when in preview mode
    mLegend->refresh();
  }
}

void QgsLayoutLegendWidget::updateLegend()
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Update Legend" ) );

    // this will reset the model completely, losing any changes
    mLegend->setAutoUpdateModel( true );
    mLegend->setAutoUpdateModel( false );
    mLegend->updateFilterByMap();
    mLegend->endCommand();
  }
}

void QgsLayoutLegendWidget::setReportTypeString( const QString &string )
{
  mFilterLegendByAtlasCheckBox->setText( tr( "Only show items inside current %1 feature" ).arg( string ) );
  mFilterLegendByAtlasCheckBox->setToolTip( tr( "Filter out legend elements that lie outside the current %1 feature." ).arg( string ) );
}

bool QgsLayoutLegendWidget::setNewItem( QgsLayoutItem *item )
{
  if ( item->type() != QgsLayoutItemRegistry::LayoutLegend )
    return false;

  if ( mLegend )
  {
    disconnect( mLegend, &QgsLayoutObject::changed, this, &QgsLayoutLegendWidget::setGuiElements );
  }

  mLegend = qobject_cast< QgsLayoutItemLegend * >( item );
  mItemPropertiesWidget->setItem( mLegend );

  if ( mLegend )
  {
    mItemTreeView->setModel( mLegend->model() );
    connect( mLegend, &QgsLayoutObject::changed, this, &QgsLayoutLegendWidget::setGuiElements );
  }

  setGuiElements();

  return true;
}

void QgsLayoutLegendWidget::blockAllSignals( bool b )
{
  mTitleLineEdit->blockSignals( b );
  mTitleAlignCombo->blockSignals( b );
  mItemTreeView->blockSignals( b );
  mCheckBoxAutoUpdate->blockSignals( b );
  mMapComboBox->blockSignals( b );
  mFilterByMapCheckBox->blockSignals( b );
  mColumnCountSpinBox->blockSignals( b );
  mSplitLayerCheckBox->blockSignals( b );
  mEqualColumnWidthCheckBox->blockSignals( b );
  mSymbolWidthSpinBox->blockSignals( b );
  mSymbolHeightSpinBox->blockSignals( b );
  mMaxSymbolSizeSpinBox->blockSignals( b );
  mMinSymbolSizeSpinBox->blockSignals( b );
  mGroupSpaceSpinBox->blockSignals( b );
  mGroupIndentSpinBox->blockSignals( b );
  mSubgroupIndentSpinBox->blockSignals( b );
  mSpaceBelowGroupHeadingSpinBox->blockSignals( b );
  mGroupSideSpinBox->blockSignals( b );
  mSpaceBelowSubgroupHeadingSpinBox->blockSignals( b );
  mSubgroupSideSpinBox->blockSignals( b );
  mLayerSpaceSpinBox->blockSignals( b );
  mSymbolSpaceSpinBox->blockSignals( b );
  mSymbolSideSpaceSpinBox->blockSignals( b );
  mIconLabelSpaceSpinBox->blockSignals( b );
  mBoxSpaceSpinBox->blockSignals( b );
  mColumnSpaceSpinBox->blockSignals( b );
  mFontColorButton->blockSignals( b );
  mRasterStrokeGroupBox->blockSignals( b );
  mRasterStrokeColorButton->blockSignals( b );
  mRasterStrokeWidthSpinBox->blockSignals( b );
  mWmsLegendWidthSpinBox->blockSignals( b );
  mWmsLegendHeightSpinBox->blockSignals( b );
  mCheckboxResizeContents->blockSignals( b );
  mTitleSpaceBottomSpinBox->blockSignals( b );
  mFilterLegendByAtlasCheckBox->blockSignals( b );
  mTitleFontButton->blockSignals( b );
  mGroupFontButton->blockSignals( b );
  mLayerFontButton->blockSignals( b );
  mItemFontButton->blockSignals( b );
  mWrapCharLineEdit->blockSignals( b );
  mLineSpacingSpinBox->blockSignals( b );
}

void QgsLayoutLegendWidget::selectedChanged( const QModelIndex &current, const QModelIndex &previous )
{
  Q_UNUSED( current )
  Q_UNUSED( previous )

  mLayerExpressionButton->setEnabled( false );

  if ( mLegend && mLegend->autoUpdateModel() )
  {
    QgsLayerTreeNode *currentNode = mItemTreeView->currentNode();
    if ( !QgsLayerTree::isLayer( currentNode ) )
      return;

    QgsLayerTreeLayer *currentLayerNode = QgsLayerTree::toLayer( currentNode );
    QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( currentLayerNode->layer() );
    if ( !vl )
      return;

    mLayerExpressionButton->setEnabled( true );
    return;
  }

  mCountToolButton->setChecked( false );
  mCountToolButton->setEnabled( false );


  mExpressionFilterButton->blockSignals( true );
  mExpressionFilterButton->setChecked( false );
  mExpressionFilterButton->setEnabled( false );
  mExpressionFilterButton->blockSignals( false );

  QgsLayerTreeNode *currentNode = mItemTreeView->currentNode();
  if ( !QgsLayerTree::isLayer( currentNode ) )
    return;

  QgsLayerTreeLayer *currentLayerNode = QgsLayerTree::toLayer( currentNode );
  QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( currentLayerNode->layer() );
  if ( !vl )
    return;

  mCountToolButton->setChecked( currentNode->customProperty( QStringLiteral( "showFeatureCount" ), 0 ).toInt() );
  mCountToolButton->setEnabled( true );
  mLayerExpressionButton->setEnabled( true );

  bool exprEnabled;
  QString expr = QgsLayerTreeUtils::legendFilterByExpression( *qobject_cast<QgsLayerTreeLayer *>( currentNode ), &exprEnabled );
  mExpressionFilterButton->blockSignals( true );
  mExpressionFilterButton->setExpressionText( expr );
  mExpressionFilterButton->setVectorLayer( vl );
  mExpressionFilterButton->setEnabled( true );
  mExpressionFilterButton->setChecked( exprEnabled );
  mExpressionFilterButton->blockSignals( false );
}

void QgsLayoutLegendWidget::setCurrentNodeStyleFromAction()
{
  QAction *a = qobject_cast<QAction *>( sender() );
  if ( !a || !mItemTreeView->currentNode() )
    return;

  QgsLegendRenderer::setNodeLegendStyle( mItemTreeView->currentNode(), static_cast< QgsLegendStyle::Style >( a->data().toInt() ) );
  mLegend->updateFilterByMap();
}

void QgsLayoutLegendWidget::setLegendMapViewData()
{
  if ( mLegend->linkedMap() )
  {
    int dpi = qt_defaultDpiX();
    QgsLayoutMeasurementConverter measurementConverter = QgsLayoutMeasurementConverter();
    measurementConverter.setDpi( dpi );
    double mapWidth = measurementConverter.convert( mLegend->linkedMap()->sizeWithUnits(), QgsUnitTypes::LayoutPixels ).width();
    double mapHeight = measurementConverter.convert( mLegend->linkedMap()->sizeWithUnits(), QgsUnitTypes::LayoutPixels ).height();
    double mapUnitsPerPixelX = mLegend->linkedMap()->extent().width() / mapWidth;
    double mapUnitsPerPixelY = mLegend->linkedMap()->extent().height() / mapHeight;
    mLegend->model()->setLegendMapViewData( ( mapUnitsPerPixelX > mapUnitsPerPixelY ? mapUnitsPerPixelX : mapUnitsPerPixelY ), dpi, mLegend->linkedMap()->scale() );
  }
}

void QgsLayoutLegendWidget::updateFilterLegendByAtlasButton()
{
  if ( QgsLayoutAtlas *atlas = layoutAtlas() )
  {
    mFilterLegendByAtlasCheckBox->setEnabled( atlas->enabled() && mLegend->layout()->reportContext().layer() && mLegend->layout()->reportContext().layer()->geometryType() == QgsWkbTypes::PolygonGeometry );
  }
}

void QgsLayoutLegendWidget::mItemTreeView_doubleClicked( const QModelIndex &idx )
{
  if ( !mLegend || !idx.isValid() )
  {
    return;
  }

  if ( mLegend->autoUpdateModel() )
    return;

  QgsLayerTreeNode *currentNode = mItemTreeView->index2node( idx );
  QgsLayerTreeModelLegendNode *legendNode = mItemTreeView->index2legendNode( idx );

  int originalIndex = -1;
  if ( legendNode )
  {
    originalIndex = _originalLegendNodeIndex( legendNode );
    currentNode = legendNode->layerNode();
  }

  QgsLayoutLegendNodeWidget *widget = new QgsLayoutLegendNodeWidget( mLegend, currentNode, legendNode, originalIndex );
  openPanel( widget );
}


//
// QgsComposerLegendMenuProvider
//

QgsLayoutLegendMenuProvider::QgsLayoutLegendMenuProvider( QgsLayerTreeView *view, QgsLayoutLegendWidget *w )
  : mView( view )
  , mWidget( w )
{}

QMenu *QgsLayoutLegendMenuProvider::createContextMenu()
{
  if ( !mView->currentNode() )
    return nullptr;

  if ( mWidget->legend()->autoUpdateModel() )
    return nullptr; // no editing allowed

  QMenu *menu = new QMenu();

  if ( QgsLayerTree::isLayer( mView->currentNode() ) )
  {
    menu->addAction( QObject::tr( "Reset to Defaults" ), mWidget, &QgsLayoutLegendWidget::resetLayerNodeToDefaults );
    menu->addSeparator();
  }

  QgsLegendStyle::Style currentStyle = QgsLegendRenderer::nodeLegendStyle( mView->currentNode(), mView->layerTreeModel() );

  QList<QgsLegendStyle::Style> lst;
  lst << QgsLegendStyle::Hidden << QgsLegendStyle::Group << QgsLegendStyle::Subgroup;
  for ( QgsLegendStyle::Style style : std::as_const( lst ) )
  {
    QAction *action = menu->addAction( QgsLegendStyle::styleLabel( style ), mWidget, &QgsLayoutLegendWidget::setCurrentNodeStyleFromAction );
    action->setCheckable( true );
    action->setChecked( currentStyle == style );
    action->setData( static_cast< int >( style ) );
  }

  return menu;
}

//
// QgsLayoutLegendNodeWidget
//
QgsLayoutLegendNodeWidget::QgsLayoutLegendNodeWidget( QgsLayoutItemLegend *legend, QgsLayerTreeNode *node, QgsLayerTreeModelLegendNode *legendNode, int originalLegendNodeIndex, QWidget *parent )
  : QgsPanelWidget( parent )
  , mLegend( legend )
  , mNode( node )
  , mLayer( qobject_cast< QgsLayerTreeLayer* >( node ) )
  , mLegendNode( legendNode )
  , mOriginalLegendNodeIndex( originalLegendNodeIndex )
{
  setupUi( this );
  setPanelTitle( tr( "Legend Item Properties" ) );

  // auto close panel if layer removed
  connect( node, &QObject::destroyed, this, &QgsPanelWidget::acceptPanel );

  mColumnSplitBehaviorComboBox->addItem( tr( "Follow Legend Default" ), QgsLayerTreeLayer::UseDefaultLegendSetting );
  mColumnSplitBehaviorComboBox->addItem( tr( "Allow Splitting Over Columns" ), QgsLayerTreeLayer::AllowSplittingLegendNodesOverMultipleColumns );
  mColumnSplitBehaviorComboBox->addItem( tr( "Prevent Splitting Over Columns" ), QgsLayerTreeLayer::PreventSplittingLegendNodesOverMultipleColumns );

  QString currentLabel;
  if ( mLegendNode )
  {
    currentLabel = mLegendNode->data( Qt::EditRole ).toString();
    mColumnBreakBeforeCheckBox->setChecked( mLegendNode->columnBreak() );
  }
  else if ( mLayer )
  {
    currentLabel = mLayer->name();
    QVariant v = mLayer->customProperty( QStringLiteral( "legend/title-label" ) );
    if ( !v.isNull() )
      currentLabel = v.toString();
    mColumnBreakBeforeCheckBox->setChecked( mLayer->customProperty( QStringLiteral( "legend/column-break" ) ).toInt() );

    mColumnSplitBehaviorComboBox->setCurrentIndex( mColumnSplitBehaviorComboBox->findData( mLayer->legendSplitBehavior() ) );
  }
  else
  {
    currentLabel = QgsLayerTree::toGroup( mNode )->name();
    mColumnBreakBeforeCheckBox->setChecked( mNode->customProperty( QStringLiteral( "legend/column-break" ) ).toInt() );
  }

  mWidthSpinBox->setClearValue( 0, tr( "Default" ) );
  mHeightSpinBox->setClearValue( 0, tr( "Default" ) );
  mWidthSpinBox->setVisible( mLegendNode || mLayer );
  mHeightSpinBox->setVisible( mLegendNode || mLayer );
  mPatchGroup->setVisible( mLegendNode || mLayer );
  mPatchWidthLabel->setVisible( mLegendNode || mLayer );
  mPatchHeightLabel->setVisible( mLegendNode || mLayer );
  mCustomSymbolCheckBox->setVisible( mLegendNode || mLegend->model()->legendNodeEmbeddedInParent( mLayer ) );
  mColumnSplitLabel->setVisible( mLayer && !mLegendNode );
  mColumnSplitBehaviorComboBox->setVisible( mLayer && !mLegendNode );
  if ( mLegendNode )
  {
    mWidthSpinBox->setValue( mLegendNode->userPatchSize().width() );
    mHeightSpinBox->setValue( mLegendNode->userPatchSize().height() );
  }
  else if ( mLayer )
  {
    mWidthSpinBox->setValue( mLayer->patchSize().width() );
    mHeightSpinBox->setValue( mLayer->patchSize().height() );
  }

  mCustomSymbolCheckBox->setChecked( false );

  QgsLegendPatchShape patchShape;
  if ( QgsSymbolLegendNode *symbolLegendNode = dynamic_cast< QgsSymbolLegendNode * >( mLegendNode ) )
  {
    patchShape = symbolLegendNode->patchShape();

    std::unique_ptr< QgsSymbol > customSymbol( symbolLegendNode->customSymbol() ? symbolLegendNode->customSymbol()->clone() : nullptr );
    mCustomSymbolCheckBox->setChecked( customSymbol.get() );
    if ( customSymbol )
    {
      mPatchShapeButton->setPreviewSymbol( customSymbol->clone() );
      mCustomSymbolButton->setSymbolType( customSymbol->type() );
      mCustomSymbolButton->setSymbol( customSymbol.release() );
    }
    else if ( symbolLegendNode->symbol() )
    {
      mPatchShapeButton->setPreviewSymbol( symbolLegendNode->symbol()->clone() );
      mCustomSymbolButton->setSymbolType( symbolLegendNode->symbol()->type() );
      mCustomSymbolButton->setSymbol( symbolLegendNode->symbol()->clone() );
    }
  }
  else if ( !mLegendNode && mLayer )
  {
    patchShape = mLayer->patchShape();
    if ( QgsSymbolLegendNode *symbolLegendNode = dynamic_cast< QgsSymbolLegendNode * >( mLegend->model()->legendNodeEmbeddedInParent( mLayer ) ) )
    {
      if ( QgsSymbol *customSymbol = symbolLegendNode->customSymbol() )
      {
        mCustomSymbolCheckBox->setChecked( true );
        mPatchShapeButton->setPreviewSymbol( customSymbol->clone() );
        mCustomSymbolButton->setSymbolType( customSymbol->type() );
        mCustomSymbolButton->setSymbol( customSymbol->clone() );
      }
      else
      {
        mPatchShapeButton->setPreviewSymbol( symbolLegendNode->symbol()->clone() );
        mCustomSymbolButton->setSymbolType( symbolLegendNode->symbol()->type() );
        mCustomSymbolButton->setSymbol( symbolLegendNode->symbol()->clone() );
      }
    }
  }

  if ( mLayer && mLayer->layer()  && mLayer->layer()->type() == QgsMapLayerType::VectorLayer )
  {
    switch ( qobject_cast< QgsVectorLayer * >( mLayer->layer() )->geometryType() )
    {
      case QgsWkbTypes::PolygonGeometry:
        mPatchShapeButton->setSymbolType( Qgis::SymbolType::Fill );
        break;

      case QgsWkbTypes::LineGeometry:
        mPatchShapeButton->setSymbolType( Qgis::SymbolType::Line );
        break;

      case QgsWkbTypes::PointGeometry:
        mPatchShapeButton->setSymbolType( Qgis::SymbolType::Marker );
        break;

      default:
        mPatchShapeLabel->hide();
        mPatchShapeButton->hide();
        break;
    }
    if ( !patchShape.isNull() )
      mPatchShapeButton->setShape( patchShape );

  }
  else if ( QgsSymbolLegendNode *symbolLegendNode = dynamic_cast< QgsSymbolLegendNode * >( mLegendNode ) )
  {
    if ( symbolLegendNode->symbol() )
    {
      mPatchShapeButton->setSymbolType( symbolLegendNode->symbol()->type() );
    }
    else
    {
      mPatchShapeLabel->hide();
      mPatchShapeButton->hide();
    }
  }
  else
  {
    mPatchShapeLabel->hide();
    mPatchShapeButton->hide();
  }

  if ( QgsColorRampLegendNode *colorRampNode = dynamic_cast< QgsColorRampLegendNode * >( mLegendNode ) )
  {
    mLabelGroup->hide();
    mColorRampLegendWidget->setSettings( colorRampNode->settings() );
  }
  else
  {
    mColorRampLegendWidget->hide();
  }

  if ( mLegendNode )
  {
    switch ( static_cast< QgsLayerTreeModelLegendNode::NodeTypes >( mLegendNode->data( QgsLayerTreeModelLegendNode::NodeTypeRole ).toInt() ) )
    {
      case QgsLayerTreeModelLegendNode::EmbeddedWidget:
      case QgsLayerTreeModelLegendNode::RasterSymbolLegend:
      case QgsLayerTreeModelLegendNode::ImageLegend:
      case QgsLayerTreeModelLegendNode::WmsLegend:
      case QgsLayerTreeModelLegendNode::DataDefinedSizeLegend:
      case QgsLayerTreeModelLegendNode::ColorRampLegend:
        mCustomSymbolCheckBox->hide();
        break;

      case QgsLayerTreeModelLegendNode::SimpleLegend:
      case QgsLayerTreeModelLegendNode::SymbolLegend:
        break;
    }
  }

  mLabelEdit->setPlainText( currentLabel );
  connect( mLabelEdit, &QPlainTextEdit::textChanged, this, &QgsLayoutLegendNodeWidget::labelChanged );
  connect( mPatchShapeButton, &QgsLegendPatchShapeButton::changed, this, &QgsLayoutLegendNodeWidget::patchChanged );
  connect( mInsertExpressionButton, &QPushButton::clicked, this, &QgsLayoutLegendNodeWidget::insertExpression );

  connect( mWidthSpinBox, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, &QgsLayoutLegendNodeWidget::sizeChanged );
  connect( mHeightSpinBox, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, &QgsLayoutLegendNodeWidget::sizeChanged );

  connect( mCustomSymbolCheckBox, &QGroupBox::toggled, this, &QgsLayoutLegendNodeWidget::customSymbolChanged );
  connect( mCustomSymbolButton, &QgsSymbolButton::changed, this, &QgsLayoutLegendNodeWidget::customSymbolChanged );

  connect( mColumnBreakBeforeCheckBox, &QCheckBox::toggled, this, &QgsLayoutLegendNodeWidget::columnBreakToggled );

  connect( mColumnSplitBehaviorComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsLayoutLegendNodeWidget::columnSplitChanged );

  connect( mColorRampLegendWidget, &QgsColorRampLegendNodeWidget::widgetChanged, this, &QgsLayoutLegendNodeWidget::colorRampLegendChanged );

  connectChildPanel( mColorRampLegendWidget );
}

void QgsLayoutLegendNodeWidget::setDockMode( bool dockMode )
{
  mColorRampLegendWidget->setDockMode( dockMode );
  QgsPanelWidget::setDockMode( dockMode );
}

void QgsLayoutLegendNodeWidget::labelChanged()
{
  mLegend->beginCommand( tr( "Edit Legend Item" ), QgsLayoutItem::UndoLegendText );

  const QString label = mLabelEdit->toPlainText();
  if ( QgsLayerTree::isGroup( mNode ) )
  {
    QgsLayerTree::toGroup( mNode )->setName( label );
  }
  else if ( mLegendNode )
  {
    QgsMapLayerLegendUtils::setLegendNodeUserLabel( mLayer, mOriginalLegendNodeIndex, label );
    mLegend->model()->refreshLayerLegend( mLayer );
  }
  else if ( mLayer )
  {
    mLayer->setCustomProperty( QStringLiteral( "legend/title-label" ), label );

    // force update of label of the legend node with embedded icon (a bit clumsy i know)
    if ( QgsLayerTreeModelLegendNode *embeddedNode = mLegend->model()->legendNodeEmbeddedInParent( mLayer ) )
      embeddedNode->setUserLabel( QString() );
  }

  mLegend->adjustBoxSize();
  mLegend->updateFilterByMap();
  mLegend->endCommand();
}

void QgsLayoutLegendNodeWidget::patchChanged()
{
  mLegend->beginCommand( tr( "Edit Legend Item" ) );

  QgsLegendPatchShape shape = mPatchShapeButton->shape();
  if ( mLegendNode )
  {
    QgsMapLayerLegendUtils::setLegendNodePatchShape( mLayer, mOriginalLegendNodeIndex, shape );
    mLegend->model()->refreshLayerLegend( mLayer );
  }
  else if ( mLayer )
  {
    mLayer->setPatchShape( shape );
    const QList<QgsLayerTreeModelLegendNode *> layerLegendNodes = mLegend->model()->layerLegendNodes( mLayer, false );
    for ( QgsLayerTreeModelLegendNode *node : layerLegendNodes )
    {
      QgsMapLayerLegendUtils::setLegendNodePatchShape( mLayer, _originalLegendNodeIndex( node ), shape );
    }
    mLegend->model()->refreshLayerLegend( mLayer );
  }

  mLegend->adjustBoxSize();
  mLegend->updateFilterByMap();
  mLegend->endCommand();
}

void QgsLayoutLegendNodeWidget::insertExpression()
{
  if ( !mLegend )
    return;

  QString selText = mLabelEdit->textCursor().selectedText();

  // html editor replaces newlines with Paragraph Separator characters - see https://github.com/qgis/QGIS/issues/27568
  selText = selText.replace( QChar( 0x2029 ), QChar( '\n' ) );

  // edit the selected expression if there's one
  if ( selText.startsWith( QLatin1String( "[%" ) ) && selText.endsWith( QLatin1String( "%]" ) ) )
    selText = selText.mid( 2, selText.size() - 4 );

  // use the atlas coverage layer, if any
  QgsVectorLayer *layer = mLegend->layout() ? mLegend->layout()->reportContext().layer() : nullptr;

  QgsExpressionContext context = mLegend->createExpressionContext();

  if ( mLayer && mLayer->layer() )
  {
    context.appendScope( QgsExpressionContextUtils::layerScope( mLayer->layer() ) );
  }

  context.setHighlightedVariables( QStringList() << QStringLiteral( "legend_title" )
                                   << QStringLiteral( "legend_column_count" )
                                   << QStringLiteral( "legend_split_layers" )
                                   << QStringLiteral( "legend_wrap_string" )
                                   << QStringLiteral( "legend_filter_by_map" )
                                   << QStringLiteral( "legend_filter_out_atlas" ) );

  QgsExpressionBuilderDialog exprDlg( layer, selText, this, QStringLiteral( "generic" ), context );

  exprDlg.setWindowTitle( tr( "Insert Expression" ) );
  if ( exprDlg.exec() == QDialog::Accepted )
  {
    QString expression = exprDlg.expressionText();
    if ( !expression.isEmpty() )
    {
      mLegend->beginCommand( tr( "Insert expression" ) );
      mLabelEdit->insertPlainText( "[%" + expression + "%]" );
      mLegend->endCommand();
    }
  }
}

void QgsLayoutLegendNodeWidget::sizeChanged( double )
{
  mLegend->beginCommand( tr( "Edit Legend Item" ) );
  const QSizeF size = QSizeF( mWidthSpinBox->value(), mHeightSpinBox->value() );

  if ( mLegendNode )
  {
    QgsMapLayerLegendUtils::setLegendNodeSymbolSize( mLayer, mOriginalLegendNodeIndex, size );
    mLegend->model()->refreshLayerLegend( mLayer );
  }
  else if ( mLayer )
  {
    mLayer->setPatchSize( size );
    const QList<QgsLayerTreeModelLegendNode *> layerLegendNodes = mLegend->model()->layerLegendNodes( mLayer, false );
    for ( QgsLayerTreeModelLegendNode *node : layerLegendNodes )
    {
      QgsMapLayerLegendUtils::setLegendNodeSymbolSize( mLayer, _originalLegendNodeIndex( node ), size );
    }
    mLegend->model()->refreshLayerLegend( mLayer );
  }

  mLegend->adjustBoxSize();
  mLegend->updateFilterByMap();
  mLegend->endCommand();
}

void QgsLayoutLegendNodeWidget::customSymbolChanged()
{
  mLegend->beginCommand( tr( "Edit Legend Item" ) );

  if ( mCustomSymbolCheckBox->isChecked() )
  {
    if ( mLegendNode )
    {
      QgsMapLayerLegendUtils::setLegendNodeCustomSymbol( mLayer, mOriginalLegendNodeIndex, mCustomSymbolButton->symbol() );
      mLegend->model()->refreshLayerLegend( mLayer );
    }
    else if ( mLayer )
    {
      const QList<QgsLayerTreeModelLegendNode *> layerLegendNodes = mLegend->model()->layerLegendNodes( mLayer, false );
      for ( QgsLayerTreeModelLegendNode *node : layerLegendNodes )
      {
        QgsMapLayerLegendUtils::setLegendNodeCustomSymbol( mLayer, _originalLegendNodeIndex( node ), mCustomSymbolButton->symbol() );
      }
      mLegend->model()->refreshLayerLegend( mLayer );
    }
  }
  else
  {
    if ( mLegendNode )
    {
      QgsMapLayerLegendUtils::setLegendNodeCustomSymbol( mLayer, mOriginalLegendNodeIndex, nullptr );
      mLegend->model()->refreshLayerLegend( mLayer );
    }
    else if ( mLayer )
    {
      const QList<QgsLayerTreeModelLegendNode *> layerLegendNodes = mLegend->model()->layerLegendNodes( mLayer, false );
      for ( QgsLayerTreeModelLegendNode *node : layerLegendNodes )
      {
        QgsMapLayerLegendUtils::setLegendNodeCustomSymbol( mLayer, _originalLegendNodeIndex( node ), nullptr );
      }
      mLegend->model()->refreshLayerLegend( mLayer );
    }
  }

  mLegend->adjustBoxSize();
  mLegend->updateFilterByMap();
  mLegend->endCommand();
}

void QgsLayoutLegendNodeWidget::colorRampLegendChanged()
{
  mLegend->beginCommand( tr( "Edit Legend Item" ) );

  QgsColorRampLegendNodeSettings settings = mColorRampLegendWidget->settings();
  QgsMapLayerLegendUtils::setLegendNodeColorRampSettings( mLayer, mOriginalLegendNodeIndex, &settings );
  mLegend->model()->refreshLayerLegend( mLayer );

  mLegend->adjustBoxSize();
  mLegend->updateFilterByMap();
  mLegend->endCommand();
}

void QgsLayoutLegendNodeWidget::columnBreakToggled( bool checked )
{
  mLegend->beginCommand( tr( "Edit Legend Columns" ) );

  if ( mLegendNode )
  {
    QgsMapLayerLegendUtils::setLegendNodeColumnBreak( mLayer, mOriginalLegendNodeIndex, checked );
    mLegend->model()->refreshLayerLegend( mLayer );
  }
  else if ( mLayer )
  {
    mLayer->setCustomProperty( QStringLiteral( "legend/column-break" ), QString( checked ? '1' : '0' ) );
  }
  else if ( mNode )
  {
    mNode->setCustomProperty( QStringLiteral( "legend/column-break" ), QString( checked ? '1' : '0' ) );
  }

  mLegend->adjustBoxSize();
  mLegend->updateFilterByMap();
  mLegend->endCommand();
}

void QgsLayoutLegendNodeWidget::columnSplitChanged()
{
  mLegend->beginCommand( tr( "Edit Legend Columns" ) );

  if ( mLayer && !mLegendNode )
  {
    mLayer->setLegendSplitBehavior( static_cast< QgsLayerTreeLayer::LegendNodesSplitBehavior >( mColumnSplitBehaviorComboBox->currentData().toInt() ) );
  }

  mLegend->adjustBoxSize();
  mLegend->updateFilterByMap();
  mLegend->endCommand();
}

///@endcond
