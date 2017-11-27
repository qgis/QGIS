/***************************************************************************
                         qgscomposerlegendwidget.cpp
                         ---------------------------
    begin                : July 2008
    copyright            : (C) 2008 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscomposerlegendwidget.h"
#include "qgscomposerlegend.h"
#include "qgscomposerlegendlayersdialog.h"
#include "qgscomposeritemwidget.h"
#include "qgscomposermap.h"
#include "qgscomposition.h"
#include "qgsguiutils.h"

#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgslayertree.h"
#include "qgslayertreeutils.h"
#include "qgslayertreemodel.h"
#include "qgslayertreemodellegendnode.h"
#include "qgslegendrenderer.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayerlegend.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgslayoutitemlegend.h"

#include <QMessageBox>
#include <QInputDialog>


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


QgsComposerLegendWidget::QgsComposerLegendWidget( QgsComposerLegend *legend )
  : QgsComposerItemBaseWidget( nullptr, legend )
  , mLegend( legend )
{
  setupUi( this );
  connect( mWrapCharLineEdit, &QLineEdit::textChanged, this, &QgsComposerLegendWidget::mWrapCharLineEdit_textChanged );
  connect( mTitleLineEdit, &QLineEdit::textChanged, this, &QgsComposerLegendWidget::mTitleLineEdit_textChanged );
  connect( mTitleAlignCombo, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsComposerLegendWidget::mTitleAlignCombo_currentIndexChanged );
  connect( mColumnCountSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsComposerLegendWidget::mColumnCountSpinBox_valueChanged );
  connect( mSplitLayerCheckBox, &QCheckBox::toggled, this, &QgsComposerLegendWidget::mSplitLayerCheckBox_toggled );
  connect( mEqualColumnWidthCheckBox, &QCheckBox::toggled, this, &QgsComposerLegendWidget::mEqualColumnWidthCheckBox_toggled );
  connect( mSymbolWidthSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerLegendWidget::mSymbolWidthSpinBox_valueChanged );
  connect( mSymbolHeightSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerLegendWidget::mSymbolHeightSpinBox_valueChanged );
  connect( mWmsLegendWidthSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerLegendWidget::mWmsLegendWidthSpinBox_valueChanged );
  connect( mWmsLegendHeightSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerLegendWidget::mWmsLegendHeightSpinBox_valueChanged );
  connect( mTitleSpaceBottomSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerLegendWidget::mTitleSpaceBottomSpinBox_valueChanged );
  connect( mGroupSpaceSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerLegendWidget::mGroupSpaceSpinBox_valueChanged );
  connect( mLayerSpaceSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerLegendWidget::mLayerSpaceSpinBox_valueChanged );
  connect( mSymbolSpaceSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerLegendWidget::mSymbolSpaceSpinBox_valueChanged );
  connect( mIconLabelSpaceSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerLegendWidget::mIconLabelSpaceSpinBox_valueChanged );
  connect( mFontColorButton, &QgsColorButton::colorChanged, this, &QgsComposerLegendWidget::mFontColorButton_colorChanged );
  connect( mBoxSpaceSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerLegendWidget::mBoxSpaceSpinBox_valueChanged );
  connect( mColumnSpaceSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerLegendWidget::mColumnSpaceSpinBox_valueChanged );
  connect( mLineSpacingSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerLegendWidget::mLineSpacingSpinBox_valueChanged );
  connect( mCheckBoxAutoUpdate, &QCheckBox::stateChanged, this, &QgsComposerLegendWidget::mCheckBoxAutoUpdate_stateChanged );
  connect( mCheckboxResizeContents, &QCheckBox::toggled, this, &QgsComposerLegendWidget::mCheckboxResizeContents_toggled );
  connect( mRasterStrokeGroupBox, &QgsCollapsibleGroupBoxBasic::toggled, this, &QgsComposerLegendWidget::mRasterStrokeGroupBox_toggled );
  connect( mRasterStrokeWidthSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerLegendWidget::mRasterStrokeWidthSpinBox_valueChanged );
  connect( mRasterStrokeColorButton, &QgsColorButton::colorChanged, this, &QgsComposerLegendWidget::mRasterStrokeColorButton_colorChanged );
  connect( mMoveDownToolButton, &QToolButton::clicked, this, &QgsComposerLegendWidget::mMoveDownToolButton_clicked );
  connect( mMoveUpToolButton, &QToolButton::clicked, this, &QgsComposerLegendWidget::mMoveUpToolButton_clicked );
  connect( mRemoveToolButton, &QToolButton::clicked, this, &QgsComposerLegendWidget::mRemoveToolButton_clicked );
  connect( mAddToolButton, &QToolButton::clicked, this, &QgsComposerLegendWidget::mAddToolButton_clicked );
  connect( mEditPushButton, &QToolButton::clicked, this, &QgsComposerLegendWidget::mEditPushButton_clicked );
  connect( mCountToolButton, &QToolButton::clicked, this, &QgsComposerLegendWidget::mCountToolButton_clicked );
  connect( mExpressionFilterButton, &QgsLegendFilterButton::toggled, this, &QgsComposerLegendWidget::mExpressionFilterButton_toggled );
  connect( mFilterByMapToolButton, &QToolButton::toggled, this, &QgsComposerLegendWidget::mFilterByMapToolButton_toggled );
  connect( mUpdateAllPushButton, &QToolButton::clicked, this, &QgsComposerLegendWidget::mUpdateAllPushButton_clicked );
  connect( mAddGroupToolButton, &QToolButton::clicked, this, &QgsComposerLegendWidget::mAddGroupToolButton_clicked );
  connect( mFilterLegendByAtlasCheckBox, &QCheckBox::toggled, this, &QgsComposerLegendWidget::mFilterLegendByAtlasCheckBox_toggled );
  connect( mItemTreeView, &QgsLayerTreeView::doubleClicked, this, &QgsComposerLegendWidget::mItemTreeView_doubleClicked );
  setPanelTitle( tr( "Legend properties" ) );

  mTitleFontButton->setMode( QgsFontButton::ModeQFont );
  mGroupFontButton->setMode( QgsFontButton::ModeQFont );
  mLayerFontButton->setMode( QgsFontButton::ModeQFont );
  mItemFontButton->setMode( QgsFontButton::ModeQFont );

  // setup icons
  mAddToolButton->setIcon( QIcon( QgsApplication::iconPath( "symbologyAdd.svg" ) ) );
  mEditPushButton->setIcon( QIcon( QgsApplication::iconPath( "symbologyEdit.png" ) ) );
  mRemoveToolButton->setIcon( QIcon( QgsApplication::iconPath( "symbologyRemove.svg" ) ) );
  mMoveUpToolButton->setIcon( QIcon( QgsApplication::iconPath( "mActionArrowUp.svg" ) ) );
  mMoveDownToolButton->setIcon( QIcon( QgsApplication::iconPath( "mActionArrowDown.svg" ) ) );
  mCountToolButton->setIcon( QIcon( QgsApplication::iconPath( "mActionSum.svg" ) ) );

  mFontColorButton->setColorDialogTitle( tr( "Select Font Color" ) );
  mFontColorButton->setContext( QStringLiteral( "composer" ) );

  mRasterStrokeColorButton->setColorDialogTitle( tr( "Select Stroke Color" ) );
  mRasterStrokeColorButton->setAllowOpacity( true );
  mRasterStrokeColorButton->setContext( QStringLiteral( "composer " ) );

  if ( legend )
    mMapComboBox->setComposition( legend->composition() );
  mMapComboBox->setItemType( QgsComposerItem::ComposerMap );
  connect( mMapComboBox, &QgsComposerItemComboBox::itemChanged, this, &QgsComposerLegendWidget::composerMapChanged );

  //add widget for item properties
  QgsComposerItemWidget *itemPropertiesWidget = new QgsComposerItemWidget( this, legend );
  mainLayout->addWidget( itemPropertiesWidget );

  mItemTreeView->setHeaderHidden( true );

  if ( legend )
  {
    mItemTreeView->setModel( legend->model() );
    mItemTreeView->setMenuProvider( new QgsComposerLegendMenuProvider( mItemTreeView, this ) );
    connect( legend, &QgsComposerObject::itemChanged, this, &QgsComposerLegendWidget::setGuiElements );
    mWrapCharLineEdit->setText( legend->wrapChar() );

    // connect atlas state to the filter legend by atlas checkbox
    connect( &legend->composition()->atlasComposition(), &QgsAtlasComposition::toggled, this, &QgsComposerLegendWidget::updateFilterLegendByAtlasButton );
    connect( &legend->composition()->atlasComposition(), &QgsAtlasComposition::coverageLayerChanged, this, &QgsComposerLegendWidget::updateFilterLegendByAtlasButton );
  }

  registerDataDefinedButton( mLegendTitleDDBtn, QgsComposerObject::LegendTitle );
  registerDataDefinedButton( mColumnsDDBtn, QgsComposerObject::LegendColumnCount );

  setGuiElements();

  connect( mItemTreeView->selectionModel(), &QItemSelectionModel::currentChanged,
           this, &QgsComposerLegendWidget::selectedChanged );
  connect( mTitleFontButton, &QgsFontButton::changed, this, &QgsComposerLegendWidget::titleFontChanged );
  connect( mGroupFontButton, &QgsFontButton::changed, this, &QgsComposerLegendWidget::groupFontChanged );
  connect( mLayerFontButton, &QgsFontButton::changed, this, &QgsComposerLegendWidget::layerFontChanged );
  connect( mItemFontButton, &QgsFontButton::changed, this, &QgsComposerLegendWidget::itemFontChanged );
}

QgsComposerLegendWidget::QgsComposerLegendWidget(): QgsComposerItemBaseWidget( nullptr, nullptr )
{
  setupUi( this );
  connect( mWrapCharLineEdit, &QLineEdit::textChanged, this, &QgsComposerLegendWidget::mWrapCharLineEdit_textChanged );
  connect( mTitleLineEdit, &QLineEdit::textChanged, this, &QgsComposerLegendWidget::mTitleLineEdit_textChanged );
  connect( mTitleAlignCombo, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsComposerLegendWidget::mTitleAlignCombo_currentIndexChanged );
  connect( mColumnCountSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsComposerLegendWidget::mColumnCountSpinBox_valueChanged );
  connect( mSplitLayerCheckBox, &QCheckBox::toggled, this, &QgsComposerLegendWidget::mSplitLayerCheckBox_toggled );
  connect( mEqualColumnWidthCheckBox, &QCheckBox::toggled, this, &QgsComposerLegendWidget::mEqualColumnWidthCheckBox_toggled );
  connect( mSymbolWidthSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerLegendWidget::mSymbolWidthSpinBox_valueChanged );
  connect( mSymbolHeightSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerLegendWidget::mSymbolHeightSpinBox_valueChanged );
  connect( mWmsLegendWidthSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerLegendWidget::mWmsLegendWidthSpinBox_valueChanged );
  connect( mWmsLegendHeightSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerLegendWidget::mWmsLegendHeightSpinBox_valueChanged );
  connect( mTitleSpaceBottomSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerLegendWidget::mTitleSpaceBottomSpinBox_valueChanged );
  connect( mGroupSpaceSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerLegendWidget::mGroupSpaceSpinBox_valueChanged );
  connect( mLayerSpaceSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerLegendWidget::mLayerSpaceSpinBox_valueChanged );
  connect( mSymbolSpaceSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerLegendWidget::mSymbolSpaceSpinBox_valueChanged );
  connect( mIconLabelSpaceSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerLegendWidget::mIconLabelSpaceSpinBox_valueChanged );
  connect( mFontColorButton, &QgsColorButton::colorChanged, this, &QgsComposerLegendWidget::mFontColorButton_colorChanged );
  connect( mBoxSpaceSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerLegendWidget::mBoxSpaceSpinBox_valueChanged );
  connect( mColumnSpaceSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerLegendWidget::mColumnSpaceSpinBox_valueChanged );
  connect( mLineSpacingSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerLegendWidget::mLineSpacingSpinBox_valueChanged );
  connect( mCheckBoxAutoUpdate, &QCheckBox::stateChanged, this, &QgsComposerLegendWidget::mCheckBoxAutoUpdate_stateChanged );
  connect( mCheckboxResizeContents, &QCheckBox::toggled, this, &QgsComposerLegendWidget::mCheckboxResizeContents_toggled );
  connect( mRasterStrokeGroupBox, &QgsCollapsibleGroupBoxBasic::toggled, this, &QgsComposerLegendWidget::mRasterStrokeGroupBox_toggled );
  connect( mRasterStrokeWidthSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerLegendWidget::mRasterStrokeWidthSpinBox_valueChanged );
  connect( mRasterStrokeColorButton, &QgsColorButton::colorChanged, this, &QgsComposerLegendWidget::mRasterStrokeColorButton_colorChanged );
  connect( mMoveDownToolButton, &QToolButton::clicked, this, &QgsComposerLegendWidget::mMoveDownToolButton_clicked );
  connect( mMoveUpToolButton, &QToolButton::clicked, this, &QgsComposerLegendWidget::mMoveUpToolButton_clicked );
  connect( mRemoveToolButton, &QToolButton::clicked, this, &QgsComposerLegendWidget::mRemoveToolButton_clicked );
  connect( mAddToolButton, &QToolButton::clicked, this, &QgsComposerLegendWidget::mAddToolButton_clicked );
  connect( mEditPushButton, &QToolButton::clicked, this, &QgsComposerLegendWidget::mEditPushButton_clicked );
  connect( mCountToolButton, &QToolButton::clicked, this, &QgsComposerLegendWidget::mCountToolButton_clicked );
  connect( mExpressionFilterButton, &QgsLegendFilterButton::toggled, this, &QgsComposerLegendWidget::mExpressionFilterButton_toggled );
  connect( mFilterByMapToolButton, &QToolButton::toggled, this, &QgsComposerLegendWidget::mFilterByMapToolButton_toggled );
  connect( mUpdateAllPushButton, &QToolButton::clicked, this, &QgsComposerLegendWidget::mUpdateAllPushButton_clicked );
  connect( mAddGroupToolButton, &QToolButton::clicked, this, &QgsComposerLegendWidget::mAddGroupToolButton_clicked );
  connect( mFilterLegendByAtlasCheckBox, &QCheckBox::toggled, this, &QgsComposerLegendWidget::mFilterLegendByAtlasCheckBox_toggled );
  connect( mItemTreeView, &QgsLayerTreeView::doubleClicked, this, &QgsComposerLegendWidget::mItemTreeView_doubleClicked );
}

void QgsComposerLegendWidget::setGuiElements()
{
  if ( !mLegend )
  {
    return;
  }

  int alignment = mLegend->titleAlignment() == Qt::AlignLeft ? 0 : mLegend->titleAlignment() == Qt::AlignHCenter ? 1 : 2;

  blockAllSignals( true );
  mTitleLineEdit->setText( mLegend->title() );
  mTitleAlignCombo->setCurrentIndex( alignment );
  mFilterByMapToolButton->setChecked( mLegend->legendFilterByMapEnabled() );
  mColumnCountSpinBox->setValue( mLegend->columnCount() );
  mSplitLayerCheckBox->setChecked( mLegend->splitLayer() );
  mEqualColumnWidthCheckBox->setChecked( mLegend->equalColumnWidth() );
  mSymbolWidthSpinBox->setValue( mLegend->symbolWidth() );
  mSymbolHeightSpinBox->setValue( mLegend->symbolHeight() );
  mWmsLegendWidthSpinBox->setValue( mLegend->wmsLegendWidth() );
  mWmsLegendHeightSpinBox->setValue( mLegend->wmsLegendHeight() );
  mTitleSpaceBottomSpinBox->setValue( mLegend->style( QgsLegendStyle::Title ).margin( QgsLegendStyle::Bottom ) );
  mGroupSpaceSpinBox->setValue( mLegend->style( QgsLegendStyle::Group ).margin( QgsLegendStyle::Top ) );
  mLayerSpaceSpinBox->setValue( mLegend->style( QgsLegendStyle::Subgroup ).margin( QgsLegendStyle::Top ) );
  // We keep Symbol and SymbolLabel Top in sync for now
  mSymbolSpaceSpinBox->setValue( mLegend->style( QgsLegendStyle::Symbol ).margin( QgsLegendStyle::Top ) );
  mIconLabelSpaceSpinBox->setValue( mLegend->style( QgsLegendStyle::SymbolLabel ).margin( QgsLegendStyle::Left ) );
  mBoxSpaceSpinBox->setValue( mLegend->boxSpace() );
  mColumnSpaceSpinBox->setValue( mLegend->columnSpace() );
  mLineSpacingSpinBox->setValue( mLegend->lineSpacing() );

  mRasterStrokeGroupBox->setChecked( mLegend->drawRasterStroke() );
  mRasterStrokeWidthSpinBox->setValue( mLegend->rasterStrokeWidth() );
  mRasterStrokeColorButton->setColor( mLegend->rasterStrokeColor() );

  mCheckBoxAutoUpdate->setChecked( mLegend->autoUpdateModel() );

  mCheckboxResizeContents->setChecked( mLegend->resizeToContents() );
  mFilterLegendByAtlasCheckBox->setChecked( mLegend->legendFilterOutAtlas() );

  const QgsComposerMap *map = mLegend->composerMap();
  mMapComboBox->setItem( map );
  mFontColorButton->setColor( mLegend->fontColor() );
  mTitleFontButton->setCurrentFont( mLegend->style( QgsLegendStyle::Title ).font() );
  mGroupFontButton->setCurrentFont( mLegend->style( QgsLegendStyle::Group ).font() );
  mLayerFontButton->setCurrentFont( mLegend->style( QgsLegendStyle::Subgroup ).font() );
  mItemFontButton->setCurrentFont( mLegend->style( QgsLegendStyle::SymbolLabel ).font() );

  blockAllSignals( false );

  mCheckBoxAutoUpdate_stateChanged( mLegend->autoUpdateModel() ? Qt::Checked : Qt::Unchecked );
  updateDataDefinedButton( mLegendTitleDDBtn );
  updateDataDefinedButton( mColumnsDDBtn );
}

void QgsComposerLegendWidget::mWrapCharLineEdit_textChanged( const QString &text )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Item wrapping changed" ) );
    mLegend->setWrapChar( text );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsComposerLegendWidget::mTitleLineEdit_textChanged( const QString &text )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Legend title changed" ), QgsComposerMergeCommand::ComposerLegendText );
    mLegend->setTitle( text );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsComposerLegendWidget::mTitleAlignCombo_currentIndexChanged( int index )
{
  if ( mLegend )
  {
    Qt::AlignmentFlag alignment = index == 0 ? Qt::AlignLeft : index == 1 ? Qt::AlignHCenter : Qt::AlignRight;
    mLegend->beginCommand( tr( "Legend title alignment changed" ) );
    mLegend->setTitleAlignment( alignment );
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsComposerLegendWidget::mColumnCountSpinBox_valueChanged( int c )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Legend column count" ), QgsComposerMergeCommand::LegendColumnCount );
    mLegend->setColumnCount( c );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
  mSplitLayerCheckBox->setEnabled( c > 1 );
  mEqualColumnWidthCheckBox->setEnabled( c > 1 );
}

void QgsComposerLegendWidget::mSplitLayerCheckBox_toggled( bool checked )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Legend split layers" ), QgsComposerMergeCommand::LegendSplitLayer );
    mLegend->setSplitLayer( checked );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsComposerLegendWidget::mEqualColumnWidthCheckBox_toggled( bool checked )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Legend equal column width" ), QgsComposerMergeCommand::LegendEqualColumnWidth );
    mLegend->setEqualColumnWidth( checked );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsComposerLegendWidget::mSymbolWidthSpinBox_valueChanged( double d )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Legend symbol width" ), QgsComposerMergeCommand::LegendSymbolWidth );
    mLegend->setSymbolWidth( d );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsComposerLegendWidget::mSymbolHeightSpinBox_valueChanged( double d )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Legend symbol height" ), QgsComposerMergeCommand::LegendSymbolHeight );
    mLegend->setSymbolHeight( d );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsComposerLegendWidget::mWmsLegendWidthSpinBox_valueChanged( double d )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Wms Legend width" ), QgsComposerMergeCommand::LegendWmsLegendWidth );
    mLegend->setWmsLegendWidth( d );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsComposerLegendWidget::mWmsLegendHeightSpinBox_valueChanged( double d )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Wms Legend height" ), QgsComposerMergeCommand::LegendWmsLegendHeight );
    mLegend->setWmsLegendHeight( d );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsComposerLegendWidget::mTitleSpaceBottomSpinBox_valueChanged( double d )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Legend title space bottom" ), QgsComposerMergeCommand::LegendTitleSpaceBottom );
    mLegend->rstyle( QgsLegendStyle::Title ).setMargin( QgsLegendStyle::Bottom, d );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsComposerLegendWidget::mGroupSpaceSpinBox_valueChanged( double d )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Legend group space" ), QgsComposerMergeCommand::LegendGroupSpace );
    mLegend->rstyle( QgsLegendStyle::Group ).setMargin( QgsLegendStyle::Top, d );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsComposerLegendWidget::mLayerSpaceSpinBox_valueChanged( double d )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Legend layer space" ), QgsComposerMergeCommand::LegendLayerSpace );
    mLegend->rstyle( QgsLegendStyle::Subgroup ).setMargin( QgsLegendStyle::Top, d );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsComposerLegendWidget::mSymbolSpaceSpinBox_valueChanged( double d )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Legend symbol space" ), QgsComposerMergeCommand::LegendSymbolSpace );
    // We keep Symbol and SymbolLabel Top in sync for now
    mLegend->rstyle( QgsLegendStyle::Symbol ).setMargin( QgsLegendStyle::Top, d );
    mLegend->rstyle( QgsLegendStyle::SymbolLabel ).setMargin( QgsLegendStyle::Top, d );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsComposerLegendWidget::mIconLabelSpaceSpinBox_valueChanged( double d )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Legend icon label space" ), QgsComposerMergeCommand::LegendIconSymbolSpace );
    mLegend->rstyle( QgsLegendStyle::SymbolLabel ).setMargin( QgsLegendStyle::Left, d );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsComposerLegendWidget::titleFontChanged()
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Title font changed" ) );
    mLegend->setStyleFont( QgsLegendStyle::Title, mTitleFontButton->currentFont() );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsComposerLegendWidget::groupFontChanged()
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Legend group font changed" ) );
    mLegend->setStyleFont( QgsLegendStyle::Group, mGroupFontButton->currentFont() );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsComposerLegendWidget::layerFontChanged()
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Legend layer font changed" ) );
    mLegend->setStyleFont( QgsLegendStyle::Subgroup, mLayerFontButton->currentFont() );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsComposerLegendWidget::itemFontChanged()
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Legend item font changed" ) );
    mLegend->setStyleFont( QgsLegendStyle::SymbolLabel, mItemFontButton->currentFont() );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsComposerLegendWidget::mFontColorButton_colorChanged( const QColor &newFontColor )
{
  if ( !mLegend )
  {
    return;
  }

  mLegend->beginCommand( tr( "Legend font color changed" ), QgsComposerMergeCommand::LegendFontColor );
  mLegend->setFontColor( newFontColor );
  mLegend->update();
  mLegend->endCommand();
}

void QgsComposerLegendWidget::mBoxSpaceSpinBox_valueChanged( double d )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Legend box space" ), QgsComposerMergeCommand::LegendBoxSpace );
    mLegend->setBoxSpace( d );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsComposerLegendWidget::mColumnSpaceSpinBox_valueChanged( double d )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Legend box space" ), QgsComposerMergeCommand::LegendColumnSpace );
    mLegend->setColumnSpace( d );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsComposerLegendWidget::mLineSpacingSpinBox_valueChanged( double d )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Legend line space" ), QgsComposerMergeCommand::LegendLineSpacing );
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


void QgsComposerLegendWidget::mMoveDownToolButton_clicked()
{
  if ( !mLegend )
  {
    return;
  }

  QModelIndex index = mItemTreeView->selectionModel()->currentIndex();
  QModelIndex parentIndex = index.parent();
  if ( !index.isValid() || index.row() == mItemTreeView->model()->rowCount( parentIndex ) - 1 )
    return;

  QgsLayerTreeNode *node = mItemTreeView->layerTreeModel()->index2node( index );
  QgsLayerTreeModelLegendNode *legendNode = mItemTreeView->layerTreeModel()->index2legendNode( index );
  if ( !node && !legendNode )
    return;

  mLegend->beginCommand( QStringLiteral( "Moved legend item down" ) );

  if ( node )
  {
    QgsLayerTreeGroup *parentGroup = QgsLayerTree::toGroup( node->parent() );
    parentGroup->insertChildNode( index.row() + 2, node->clone() );
    parentGroup->removeChildNode( node );
  }
  else // legend node
  {
    _moveLegendNode( legendNode->layerNode(), _unfilteredLegendNodeIndex( legendNode ), 1 );
    mItemTreeView->layerTreeModel()->refreshLayerLegend( legendNode->layerNode() );
  }

  mItemTreeView->setCurrentIndex( mItemTreeView->layerTreeModel()->index( index.row() + 1, 0, parentIndex ) );

  mLegend->update();
  mLegend->endCommand();
}

void QgsComposerLegendWidget::mMoveUpToolButton_clicked()
{
  if ( !mLegend )
  {
    return;
  }

  QModelIndex index = mItemTreeView->selectionModel()->currentIndex();
  QModelIndex parentIndex = index.parent();
  if ( !index.isValid() || index.row() == 0 )
    return;

  QgsLayerTreeNode *node = mItemTreeView->layerTreeModel()->index2node( index );
  QgsLayerTreeModelLegendNode *legendNode = mItemTreeView->layerTreeModel()->index2legendNode( index );
  if ( !node && !legendNode )
    return;

  mLegend->beginCommand( QStringLiteral( "Moved legend item up" ) );

  if ( node )
  {
    QgsLayerTreeGroup *parentGroup = QgsLayerTree::toGroup( node->parent() );
    parentGroup->insertChildNode( index.row() - 1, node->clone() );
    parentGroup->removeChildNode( node );
  }
  else // legend node
  {
    _moveLegendNode( legendNode->layerNode(), _unfilteredLegendNodeIndex( legendNode ), -1 );
    mItemTreeView->layerTreeModel()->refreshLayerLegend( legendNode->layerNode() );
  }

  mItemTreeView->setCurrentIndex( mItemTreeView->layerTreeModel()->index( index.row() - 1, 0, parentIndex ) );

  mLegend->update();
  mLegend->endCommand();
}

void QgsComposerLegendWidget::mCheckBoxAutoUpdate_stateChanged( int state )
{
  mLegend->beginCommand( QStringLiteral( "Auto update changed" ) );

  mLegend->setAutoUpdateModel( state == Qt::Checked );

  mLegend->updateItem();
  mLegend->endCommand();

  // do not allow editing of model if auto update is on - we would modify project's layer tree
  QList<QWidget *> widgets;
  widgets << mMoveDownToolButton << mMoveUpToolButton << mRemoveToolButton << mAddToolButton
          << mEditPushButton << mCountToolButton << mUpdateAllPushButton << mAddGroupToolButton
          << mExpressionFilterButton;
  Q_FOREACH ( QWidget *w, widgets )
    w->setEnabled( state != Qt::Checked );

  if ( state == Qt::Unchecked )
  {
    // update widgets state based on current selection
    selectedChanged( QModelIndex(), QModelIndex() );
  }
}

void QgsComposerLegendWidget::composerMapChanged( QgsComposerItem *item )
{
  if ( !mLegend )
  {
    return;
  }

  const QgsComposition *comp = mLegend->composition();
  if ( !comp )
  {
    return;
  }

  QgsComposerMap *map = dynamic_cast< QgsComposerMap * >( item );
  if ( map )
  {
    mLegend->beginCommand( tr( "Legend map changed" ) );
    mLegend->setComposerMap( map );
    mLegend->updateItem();
    mLegend->endCommand();
  }
}

void QgsComposerLegendWidget::mCheckboxResizeContents_toggled( bool checked )
{
  if ( !mLegend )
  {
    return;
  }

  mLegend->beginCommand( tr( "Legend resize to contents" ) );
  mLegend->setResizeToContents( checked );
  if ( checked )
    mLegend->adjustBoxSize();
  mLegend->updateItem();
  mLegend->endCommand();
}

void QgsComposerLegendWidget::mRasterStrokeGroupBox_toggled( bool state )
{
  if ( !mLegend )
  {
    return;
  }

  mLegend->beginCommand( tr( "Legend raster borders" ) );
  mLegend->setDrawRasterStroke( state );
  mLegend->adjustBoxSize();
  mLegend->update();
  mLegend->endCommand();
}

void QgsComposerLegendWidget::mRasterStrokeWidthSpinBox_valueChanged( double d )
{
  if ( !mLegend )
  {
    return;
  }

  mLegend->beginCommand( tr( "Legend raster stroke width" ), QgsComposerMergeCommand::LegendRasterStrokeWidth );
  mLegend->setRasterStrokeWidth( d );
  mLegend->adjustBoxSize();
  mLegend->update();
  mLegend->endCommand();
}

void QgsComposerLegendWidget::mRasterStrokeColorButton_colorChanged( const QColor &newColor )
{
  if ( !mLegend )
  {
    return;
  }

  mLegend->beginCommand( tr( "Legend raster stroke color" ), QgsComposerMergeCommand::LegendRasterStrokeColor );
  mLegend->setRasterStrokeColor( newColor );
  mLegend->update();
  mLegend->endCommand();
}

void QgsComposerLegendWidget::mAddToolButton_clicked()
{
  if ( !mLegend )
  {
    return;
  }

  QgisApp *app = QgisApp::instance();
  if ( !app )
  {
    return;
  }

  QgsMapCanvas *canvas = app->mapCanvas();
  if ( canvas )
  {
    QList<QgsMapLayer *> layers = canvas->layers();

    QgsComposerLegendLayersDialog addDialog( layers, this );
    if ( addDialog.exec() == QDialog::Accepted )
    {
      QgsMapLayer *layer = addDialog.selectedLayer();
      if ( layer )
      {
        mLegend->beginCommand( QStringLiteral( "Legend item added" ) );
        mLegend->model()->rootGroup()->addLayer( layer );
        mLegend->endCommand();
      }
    }
  }
}

void QgsComposerLegendWidget::mRemoveToolButton_clicked()
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

  mLegend->beginCommand( QStringLiteral( "Legend item removed" ) );

  QList<QPersistentModelIndex> indexes;
  Q_FOREACH ( const QModelIndex &index, selectionModel->selectedIndexes() )
    indexes << index;

  // first try to remove legend nodes
  QHash<QgsLayerTreeLayer *, QList<int> > nodesWithRemoval;
  Q_FOREACH ( const QPersistentModelIndex &index, indexes )
  {
    if ( QgsLayerTreeModelLegendNode *legendNode = mItemTreeView->layerTreeModel()->index2legendNode( index ) )
    {
      QgsLayerTreeLayer *nodeLayer = legendNode->layerNode();
      nodesWithRemoval[nodeLayer].append( _unfilteredLegendNodeIndex( legendNode ) );
    }
  }
  Q_FOREACH ( QgsLayerTreeLayer *nodeLayer, nodesWithRemoval.keys() )
  {
    QList<int> toDelete = nodesWithRemoval[nodeLayer];
    std::sort( toDelete.begin(), toDelete.end(), std::greater<int>() );
    QList<int> order = QgsMapLayerLegendUtils::legendNodeOrder( nodeLayer );

    Q_FOREACH ( int i, toDelete )
    {
      if ( i >= 0 && i < order.count() )
        order.removeAt( i );
    }

    QgsMapLayerLegendUtils::setLegendNodeOrder( nodeLayer, order );
    mItemTreeView->layerTreeModel()->refreshLayerLegend( nodeLayer );
  }

  // then remove layer tree nodes
  Q_FOREACH ( const QPersistentModelIndex &index, indexes )
  {
    if ( index.isValid() && mItemTreeView->layerTreeModel()->index2node( index ) )
      mLegend->model()->removeRow( index.row(), index.parent() );
  }

  mLegend->adjustBoxSize();
  mLegend->updateItem();
  mLegend->endCommand();
}

void QgsComposerLegendWidget::mEditPushButton_clicked()
{
  if ( !mLegend )
  {
    return;
  }

  QModelIndex idx = mItemTreeView->selectionModel()->currentIndex();
  mItemTreeView_doubleClicked( idx );
}

void QgsComposerLegendWidget::resetLayerNodeToDefaults()
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
  if ( QgsLayerTreeNode *node = mItemTreeView->layerTreeModel()->index2node( currentIndex ) )
  {
    if ( QgsLayerTree::isLayer( node ) )
      nodeLayer = QgsLayerTree::toLayer( node );
  }
  if ( QgsLayerTreeModelLegendNode *legendNode = mItemTreeView->layerTreeModel()->index2legendNode( currentIndex ) )
  {
    nodeLayer = legendNode->layerNode();
  }

  if ( !nodeLayer )
    return;

  mLegend->beginCommand( tr( "Legend updated" ) );

  Q_FOREACH ( const QString &key, nodeLayer->customProperties() )
  {
    if ( key.startsWith( QLatin1String( "legend/" ) ) )
      nodeLayer->removeCustomProperty( key );
  }

  mItemTreeView->layerTreeModel()->refreshLayerLegend( nodeLayer );

  mLegend->updateItem();
  mLegend->adjustBoxSize();
  mLegend->endCommand();
}

void QgsComposerLegendWidget::mCountToolButton_clicked( bool checked )
{
  QgsDebugMsg( "Entered." );
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

  mLegend->beginCommand( tr( "Legend updated" ) );
  currentNode->setCustomProperty( QStringLiteral( "showFeatureCount" ), checked ? 1 : 0 );
  mLegend->updateItem();
  mLegend->adjustBoxSize();
  mLegend->endCommand();
}

void QgsComposerLegendWidget::mFilterByMapToolButton_toggled( bool checked )
{
  mLegend->beginCommand( tr( "Legend updated" ) );
  mLegend->setLegendFilterByMapEnabled( checked );
  mLegend->adjustBoxSize();
  mLegend->endCommand();
}

void QgsComposerLegendWidget::mExpressionFilterButton_toggled( bool checked )
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

  mLegend->beginCommand( tr( "Legend updated" ) );
  mLegend->updateItem();
  mLegend->adjustBoxSize();
  mLegend->endCommand();
}

void QgsComposerLegendWidget::mUpdateAllPushButton_clicked()
{
  updateLegend();
}

void QgsComposerLegendWidget::mAddGroupToolButton_clicked()
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Legend group added" ) );
    mLegend->model()->rootGroup()->addGroup( tr( "Group" ) );
    mLegend->updateItem();
    mLegend->endCommand();
  }
}

void QgsComposerLegendWidget::mFilterLegendByAtlasCheckBox_toggled( bool toggled )
{
  if ( mLegend )
  {
    mLegend->setLegendFilterOutAtlas( toggled );
    // force update of legend when in preview mode
    if ( mLegend->composition()->atlasMode() == QgsComposition::PreviewAtlas )
    {
      mLegend->composition()->atlasComposition().refreshFeature();
    }
  }
}

void QgsComposerLegendWidget::updateLegend()
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Legend updated" ) );

    // this will reset the model completely, losing any changes
    mLegend->setAutoUpdateModel( true );
    mLegend->setAutoUpdateModel( false );
    mLegend->updateItem();
    mLegend->endCommand();
  }
}

void QgsComposerLegendWidget::blockAllSignals( bool b )
{
  mTitleLineEdit->blockSignals( b );
  mTitleAlignCombo->blockSignals( b );
  mItemTreeView->blockSignals( b );
  mCheckBoxAutoUpdate->blockSignals( b );
  mMapComboBox->blockSignals( b );
  mFilterByMapToolButton->blockSignals( b );
  mColumnCountSpinBox->blockSignals( b );
  mSplitLayerCheckBox->blockSignals( b );
  mEqualColumnWidthCheckBox->blockSignals( b );
  mSymbolWidthSpinBox->blockSignals( b );
  mSymbolHeightSpinBox->blockSignals( b );
  mGroupSpaceSpinBox->blockSignals( b );
  mLayerSpaceSpinBox->blockSignals( b );
  mSymbolSpaceSpinBox->blockSignals( b );
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
}

void QgsComposerLegendWidget::selectedChanged( const QModelIndex &current, const QModelIndex &previous )
{
  Q_UNUSED( current );
  Q_UNUSED( previous );

  if ( mLegend && mLegend->autoUpdateModel() )
    return;

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

  bool exprEnabled;
  QString expr = QgsLayerTreeUtils::legendFilterByExpression( *qobject_cast<QgsLayerTreeLayer *>( currentNode ), &exprEnabled );
  mExpressionFilterButton->blockSignals( true );
  mExpressionFilterButton->setExpressionText( expr );
  mExpressionFilterButton->setVectorLayer( vl );
  mExpressionFilterButton->setEnabled( true );
  mExpressionFilterButton->setChecked( exprEnabled );
  mExpressionFilterButton->blockSignals( false );
}

void QgsComposerLegendWidget::setCurrentNodeStyleFromAction()
{
  QAction *a = qobject_cast<QAction *>( sender() );
  if ( !a || !mItemTreeView->currentNode() )
    return;

  QgsLegendRenderer::setNodeLegendStyle( mItemTreeView->currentNode(), ( QgsLegendStyle::Style ) a->data().toInt() );
  mLegend->updateItem();
}

void QgsComposerLegendWidget::updateFilterLegendByAtlasButton()
{
  const QgsAtlasComposition &atlas = mLegend->composition()->atlasComposition();
  mFilterLegendByAtlasCheckBox->setEnabled( atlas.enabled() && atlas.coverageLayer() && atlas.coverageLayer()->geometryType() == QgsWkbTypes::PolygonGeometry );
}

void QgsComposerLegendWidget::mItemTreeView_doubleClicked( const QModelIndex &idx )
{
  if ( !mLegend || !idx.isValid() )
  {
    return;
  }

  QgsLayerTreeModel *model = mItemTreeView->layerTreeModel();
  QgsLayerTreeNode *currentNode = model->index2node( idx );
  QgsLayerTreeModelLegendNode *legendNode = model->index2legendNode( idx );
  QString currentText;

  if ( QgsLayerTree::isGroup( currentNode ) )
  {
    currentText = QgsLayerTree::toGroup( currentNode )->name();
  }
  else if ( QgsLayerTree::isLayer( currentNode ) )
  {
    currentText = QgsLayerTree::toLayer( currentNode )->name();
    QVariant v = currentNode->customProperty( QStringLiteral( "legend/title-label" ) );
    if ( !v.isNull() )
      currentText = v.toString();
  }
  else if ( legendNode )
  {
    currentText = legendNode->data( Qt::EditRole ).toString();
  }

  bool ok;
  QString newText = QInputDialog::getText( this, tr( "Legend item properties" ), tr( "Item text" ),
                    QLineEdit::Normal, currentText, &ok );
  if ( !ok || newText == currentText )
    return;

  mLegend->beginCommand( tr( "Legend item edited" ) );

  if ( QgsLayerTree::isGroup( currentNode ) )
  {
    QgsLayerTree::toGroup( currentNode )->setName( newText );
  }
  else if ( QgsLayerTree::isLayer( currentNode ) )
  {
    currentNode->setCustomProperty( QStringLiteral( "legend/title-label" ), newText );

    // force update of label of the legend node with embedded icon (a bit clumsy i know)
    if ( QgsLayerTreeModelLegendNode *embeddedNode = model->legendNodeEmbeddedInParent( QgsLayerTree::toLayer( currentNode ) ) )
      embeddedNode->setUserLabel( QString() );
  }
  else if ( legendNode )
  {
    int originalIndex = _originalLegendNodeIndex( legendNode );
    QgsMapLayerLegendUtils::setLegendNodeUserLabel( legendNode->layerNode(), originalIndex, newText );
    model->refreshLayerLegend( legendNode->layerNode() );
  }

  mLegend->adjustBoxSize();
  mLegend->updateItem();
  mLegend->endCommand();
}


//
// QgsComposerLegendMenuProvider
//

QgsComposerLegendMenuProvider::QgsComposerLegendMenuProvider( QgsLayerTreeView *view, QgsComposerLegendWidget *w )
  : mView( view )
  , mWidget( w )
{}

QMenu *QgsComposerLegendMenuProvider::createContextMenu()
{
  if ( !mView->currentNode() )
    return nullptr;

  if ( mWidget->legend()->autoUpdateModel() )
    return nullptr; // no editing allowed

  QMenu *menu = new QMenu();

  if ( QgsLayerTree::isLayer( mView->currentNode() ) )
  {
    menu->addAction( QObject::tr( "Reset to defaults" ), mWidget, SLOT( resetLayerNodeToDefaults() ) );
    menu->addSeparator();
  }

  QgsLegendStyle::Style currentStyle = QgsLegendRenderer::nodeLegendStyle( mView->currentNode(), mView->layerTreeModel() );

  QList<QgsLegendStyle::Style> lst;
  lst << QgsLegendStyle::Hidden << QgsLegendStyle::Group << QgsLegendStyle::Subgroup;
  Q_FOREACH ( QgsLegendStyle::Style style, lst )
  {
    QAction *action = menu->addAction( QgsLegendStyle::styleLabel( style ), mWidget, SLOT( setCurrentNodeStyleFromAction() ) );
    action->setCheckable( true );
    action->setChecked( currentStyle == style );
    action->setData( ( int ) style );
  }

  return menu;
}
