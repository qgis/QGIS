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
#include "qgscomposerlegenditem.h"
#include "qgscomposerlegenditemdialog.h"
#include "qgscomposerlegendlayersdialog.h"
#include "qgscomposeritemwidget.h"
#include "qgscomposermap.h"
#include "qgscomposition.h"
#include "qgisgui.h"

#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgslayertree.h"
#include "qgslayertreemodel.h"
#include "qgslayertreemodellegendnode.h"
#include "qgslegendrenderer.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayerlegend.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaprenderer.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"

#include <QMessageBox>
#include <QInputDialog>



class QgsComposerLegendMenuProvider : public QObject, public QgsLayerTreeViewMenuProvider
{
  public:
    QgsComposerLegendMenuProvider( QgsLayerTreeView* view, QgsComposerLegendWidget* w ) : mView( view ), mWidget( w ) {}

    virtual QMenu* createContextMenu() override
    {
      if ( !mView->currentNode() )
        return 0;

      if ( mWidget->legend()->autoUpdateModel() )
        return 0; // no editing allowed

      QMenu* menu = new QMenu();

      if ( QgsLayerTree::isLayer( mView->currentNode() ) )
      {
        menu->addAction( tr( "Reset to defaults" ), mWidget, SLOT( resetLayerNodeToDefaults() ) );
        menu->addSeparator();
      }

      QgsComposerLegendStyle::Style currentStyle = QgsLegendRenderer::nodeLegendStyle( mView->currentNode(), mView->layerTreeModel() );

      QList<QgsComposerLegendStyle::Style> lst;
      lst << QgsComposerLegendStyle::Hidden << QgsComposerLegendStyle::Group << QgsComposerLegendStyle::Subgroup;
      foreach ( QgsComposerLegendStyle::Style style, lst )
      {
        QAction* action = menu->addAction( QgsComposerLegendStyle::styleLabel( style ), mWidget, SLOT( setCurrentNodeStyleFromAction() ) );
        action->setCheckable( true );
        action->setChecked( currentStyle == style );
        action->setData(( int ) style );
      }

      return menu;
    }

  protected:
    QgsLayerTreeView* mView;
    QgsComposerLegendWidget* mWidget;
};



QgsComposerLegendWidget::QgsComposerLegendWidget( QgsComposerLegend* legend )
    : QgsComposerItemBaseWidget( 0, legend )
    , mLegend( legend )
{
  setupUi( this );

  // setup icons
  mAddToolButton->setIcon( QIcon( QgsApplication::iconPath( "symbologyAdd.png" ) ) );
  mEditPushButton->setIcon( QIcon( QgsApplication::iconPath( "symbologyEdit.png" ) ) );
  mRemoveToolButton->setIcon( QIcon( QgsApplication::iconPath( "symbologyRemove.png" ) ) );
  mMoveUpToolButton->setIcon( QIcon( QgsApplication::iconPath( "symbologyUp.png" ) ) );
  mMoveDownToolButton->setIcon( QIcon( QgsApplication::iconPath( "symbologyDown.png" ) ) );
  mCountToolButton->setIcon( QIcon( QgsApplication::iconPath( "mActionSum.png" ) ) );

  mFontColorButton->setColorDialogTitle( tr( "Select font color" ) );
  mFontColorButton->setContext( "composer" );

  //add widget for item properties
  QgsComposerItemWidget* itemPropertiesWidget = new QgsComposerItemWidget( this, legend );
  mainLayout->addWidget( itemPropertiesWidget );

  mItemTreeView->setHeaderHidden( true );

  if ( legend )
  {
    mItemTreeView->setModel( legend->modelV2() );
    mItemTreeView->setMenuProvider( new QgsComposerLegendMenuProvider( mItemTreeView, this ) );
    connect( legend, SIGNAL( itemChanged() ), this, SLOT( setGuiElements() ) );
    mWrapCharLineEdit->setText( legend->wrapChar() );
  }

  setGuiElements();

  connect( mItemTreeView->selectionModel(), SIGNAL( currentChanged( const QModelIndex &, const QModelIndex & ) ),
           this, SLOT( selectedChanged( const QModelIndex &, const QModelIndex & ) ) );
}

QgsComposerLegendWidget::QgsComposerLegendWidget(): QgsComposerItemBaseWidget( 0, 0 ), mLegend( 0 )
{
  setupUi( this );
}

QgsComposerLegendWidget::~QgsComposerLegendWidget()
{

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
  mTitleSpaceBottomSpinBox->setValue( mLegend->style( QgsComposerLegendStyle::Title ).margin( QgsComposerLegendStyle::Bottom ) );
  mGroupSpaceSpinBox->setValue( mLegend->style( QgsComposerLegendStyle::Group ).margin( QgsComposerLegendStyle::Top ) );
  mLayerSpaceSpinBox->setValue( mLegend->style( QgsComposerLegendStyle::Subgroup ).margin( QgsComposerLegendStyle::Top ) );
  // We keep Symbol and SymbolLabel Top in sync for now
  mSymbolSpaceSpinBox->setValue( mLegend->style( QgsComposerLegendStyle::Symbol ).margin( QgsComposerLegendStyle::Top ) );
  mIconLabelSpaceSpinBox->setValue( mLegend->style( QgsComposerLegendStyle::SymbolLabel ).margin( QgsComposerLegendStyle::Left ) );
  mBoxSpaceSpinBox->setValue( mLegend->boxSpace() );
  mColumnSpaceSpinBox->setValue( mLegend->columnSpace() );
  mCheckBoxAutoUpdate->setChecked( mLegend->autoUpdateModel() );
  refreshMapComboBox();

  const QgsComposerMap* map = mLegend->composerMap();
  if ( map )
  {
    mMapComboBox->setCurrentIndex( mMapComboBox->findData( map->id() ) );
  }
  else
  {
    mMapComboBox->setCurrentIndex( mMapComboBox->findData( -1 ) );
  }
  mFontColorButton->setColor( mLegend->fontColor() );
  blockAllSignals( false );

  on_mCheckBoxAutoUpdate_stateChanged( mLegend->autoUpdateModel() ? Qt::Checked : Qt::Unchecked );
}

void QgsComposerLegendWidget::on_mWrapCharLineEdit_textChanged( const QString &text )
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

void QgsComposerLegendWidget::on_mTitleLineEdit_textChanged( const QString& text )
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

void QgsComposerLegendWidget::on_mTitleAlignCombo_currentIndexChanged( int index )
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

void QgsComposerLegendWidget::on_mColumnCountSpinBox_valueChanged( int c )
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

void QgsComposerLegendWidget::on_mSplitLayerCheckBox_toggled( bool checked )
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

void QgsComposerLegendWidget::on_mEqualColumnWidthCheckBox_toggled( bool checked )
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

void QgsComposerLegendWidget::on_mSymbolWidthSpinBox_valueChanged( double d )
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

void QgsComposerLegendWidget::on_mSymbolHeightSpinBox_valueChanged( double d )
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

void QgsComposerLegendWidget::on_mWmsLegendWidthSpinBox_valueChanged( double d )
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

void QgsComposerLegendWidget::on_mWmsLegendHeightSpinBox_valueChanged( double d )
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

void QgsComposerLegendWidget::on_mTitleSpaceBottomSpinBox_valueChanged( double d )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Legend title space bottom" ), QgsComposerMergeCommand::LegendTitleSpaceBottom );
    mLegend->rstyle( QgsComposerLegendStyle::Title ).setMargin( QgsComposerLegendStyle::Bottom, d );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsComposerLegendWidget::on_mGroupSpaceSpinBox_valueChanged( double d )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Legend group space" ), QgsComposerMergeCommand::LegendGroupSpace );
    mLegend->rstyle( QgsComposerLegendStyle::Group ).setMargin( QgsComposerLegendStyle::Top, d );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsComposerLegendWidget::on_mLayerSpaceSpinBox_valueChanged( double d )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Legend layer space" ), QgsComposerMergeCommand::LegendLayerSpace );
    mLegend->rstyle( QgsComposerLegendStyle::Subgroup ).setMargin( QgsComposerLegendStyle::Top, d );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsComposerLegendWidget::on_mSymbolSpaceSpinBox_valueChanged( double d )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Legend symbol space" ), QgsComposerMergeCommand::LegendSymbolSpace );
    // We keep Symbol and SymbolLabel Top in sync for now
    mLegend->rstyle( QgsComposerLegendStyle::Symbol ).setMargin( QgsComposerLegendStyle::Top, d );
    mLegend->rstyle( QgsComposerLegendStyle::SymbolLabel ).setMargin( QgsComposerLegendStyle::Top, d );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsComposerLegendWidget::on_mIconLabelSpaceSpinBox_valueChanged( double d )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Legend icon label space" ), QgsComposerMergeCommand::LegendIconSymbolSpace );
    mLegend->rstyle( QgsComposerLegendStyle::SymbolLabel ).setMargin( QgsComposerLegendStyle::Left, d );
    mLegend->adjustBoxSize();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsComposerLegendWidget::on_mTitleFontButton_clicked()
{
  if ( mLegend )
  {
    bool ok;
    QFont newFont = QgisGui::getFont( ok, mLegend->style( QgsComposerLegendStyle::Title ).font() );
    if ( ok )
    {
      mLegend->beginCommand( tr( "Title font changed" ) );
      mLegend->setStyleFont( QgsComposerLegendStyle::Title, newFont );
      mLegend->adjustBoxSize();
      mLegend->update();
      mLegend->endCommand();
    }
  }
}

void QgsComposerLegendWidget::on_mGroupFontButton_clicked()
{
  if ( mLegend )
  {
    bool ok;
    QFont newFont = QgisGui::getFont( ok, mLegend->style( QgsComposerLegendStyle::Group ).font() );
    if ( ok )
    {
      mLegend->beginCommand( tr( "Legend group font changed" ) );
      mLegend->setStyleFont( QgsComposerLegendStyle::Group, newFont );
      mLegend->adjustBoxSize();
      mLegend->update();
      mLegend->endCommand();
    }
  }
}

void QgsComposerLegendWidget::on_mLayerFontButton_clicked()
{
  if ( mLegend )
  {
    bool ok;
    QFont newFont = QgisGui::getFont( ok, mLegend->style( QgsComposerLegendStyle::Subgroup ).font() );
    if ( ok )
    {
      mLegend->beginCommand( tr( "Legend layer font changed" ) );
      mLegend->setStyleFont( QgsComposerLegendStyle::Subgroup, newFont );
      mLegend->adjustBoxSize();
      mLegend->update();
      mLegend->endCommand();
    }
  }
}

void QgsComposerLegendWidget::on_mItemFontButton_clicked()
{
  if ( mLegend )
  {
    bool ok;
    QFont newFont = QgisGui::getFont( ok, mLegend->style( QgsComposerLegendStyle::SymbolLabel ).font() );
    if ( ok )
    {
      mLegend->beginCommand( tr( "Legend item font changed" ) );
      mLegend->setStyleFont( QgsComposerLegendStyle::SymbolLabel, newFont );
      mLegend->adjustBoxSize();
      mLegend->update();
      mLegend->endCommand();
    }
  }
}

void QgsComposerLegendWidget::on_mFontColorButton_colorChanged( const QColor& newFontColor )
{
  if ( !mLegend )
  {
    return;
  }

  mLegend->beginCommand( tr( "Legend font color changed" ) );
  mLegend->setFontColor( newFontColor );
  mLegend->update();
  mLegend->endCommand();
}

void QgsComposerLegendWidget::on_mBoxSpaceSpinBox_valueChanged( double d )
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

void QgsComposerLegendWidget::on_mColumnSpaceSpinBox_valueChanged( double d )
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


static void _moveLegendNode( QgsLayerTreeLayer* nodeLayer, int legendNodeIndex, int offset )
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


void QgsComposerLegendWidget::on_mMoveDownToolButton_clicked()
{
  if ( !mLegend )
  {
    return;
  }

  QModelIndex index = mItemTreeView->selectionModel()->currentIndex();
  QModelIndex parentIndex = index.parent();
  if ( !index.isValid() || index.row() == mItemTreeView->model()->rowCount( parentIndex ) - 1 )
    return;

  QgsLayerTreeNode* node = mItemTreeView->layerTreeModel()->index2node( index );
  QgsLayerTreeModelLegendNode* legendNode = mItemTreeView->layerTreeModel()->index2legendNode( index );
  if ( !node && !legendNode )
    return;

  mLegend->beginCommand( "Moved legend item down" );

  if ( node )
  {
    QgsLayerTreeGroup* parentGroup = QgsLayerTree::toGroup( node->parent() );
    parentGroup->insertChildNode( index.row() + 2, node->clone() );
    parentGroup->removeChildNode( node );
  }
  else // legend node
  {
    _moveLegendNode( legendNode->layerNode(), index.row(), 1 );
    mItemTreeView->layerTreeModel()->refreshLayerLegend( legendNode->layerNode() );
  }

  mItemTreeView->setCurrentIndex( mItemTreeView->layerTreeModel()->index( index.row() + 1, 0, parentIndex ) );

  mLegend->update();
  mLegend->endCommand();
}

void QgsComposerLegendWidget::on_mMoveUpToolButton_clicked()
{
  if ( !mLegend )
  {
    return;
  }

  QModelIndex index = mItemTreeView->selectionModel()->currentIndex();
  QModelIndex parentIndex = index.parent();
  if ( !index.isValid() || index.row() == 0 )
    return;

  QgsLayerTreeNode* node = mItemTreeView->layerTreeModel()->index2node( index );
  QgsLayerTreeModelLegendNode* legendNode = mItemTreeView->layerTreeModel()->index2legendNode( index );
  if ( !node && !legendNode )
    return;

  mLegend->beginCommand( "Moved legend item up" );

  if ( node )
  {
    QgsLayerTreeGroup* parentGroup = QgsLayerTree::toGroup( node->parent() );
    parentGroup->insertChildNode( index.row() - 1, node->clone() );
    parentGroup->removeChildNode( node );
  }
  else // legend node
  {
    _moveLegendNode( legendNode->layerNode(), index.row(), -1 );
    mItemTreeView->layerTreeModel()->refreshLayerLegend( legendNode->layerNode() );
  }

  mItemTreeView->setCurrentIndex( mItemTreeView->layerTreeModel()->index( index.row() - 1, 0, parentIndex ) );

  mLegend->update();
  mLegend->endCommand();
}

void QgsComposerLegendWidget::on_mCheckBoxAutoUpdate_stateChanged( int state )
{
  mLegend->beginCommand( "Auto update changed" );

  mLegend->setAutoUpdateModel( state == Qt::Checked );

  mLegend->update();
  mLegend->endCommand();

  // do not allow editing of model if auto update is on - we would modify project's layer tree
  QList<QWidget*> widgets;
  widgets << mMoveDownToolButton << mMoveUpToolButton << mRemoveToolButton << mAddToolButton
  << mEditPushButton << mCountToolButton << mUpdateAllPushButton << mAddGroupToolButton;
  foreach ( QWidget* w, widgets )
    w->setEnabled( state != Qt::Checked );
}

void QgsComposerLegendWidget::on_mMapComboBox_currentIndexChanged( int index )
{
  if ( !mLegend )
  {
    return;
  }

  QVariant itemData = mMapComboBox->itemData( index );
  if ( itemData.type() == QVariant::Invalid )
  {
    return;
  }

  const QgsComposition* comp = mLegend->composition();
  if ( !comp )
  {
    return;
  }

  int mapNr = itemData.toInt();
  if ( mapNr < 0 )
  {
    mLegend->setComposerMap( 0 );
  }
  else
  {
    const QgsComposerMap* map = comp->getComposerMapById( mapNr );
    if ( map )
    {
      mLegend->beginCommand( tr( "Legend map changed" ) );
      mLegend->setComposerMap( map );
      mLegend->update();
      mLegend->endCommand();
    }
  }
}

void QgsComposerLegendWidget::on_mAddToolButton_clicked()
{
  if ( !mLegend )
  {
    return;
  }

  QgisApp* app = QgisApp::instance();
  if ( !app )
  {
    return;
  }

  QgsMapCanvas* canvas = app->mapCanvas();
  if ( canvas )
  {
    QList<QgsMapLayer*> layers = canvas->layers();

    QgsComposerLegendLayersDialog addDialog( layers, this );
    if ( addDialog.exec() == QDialog::Accepted )
    {
      QgsMapLayer* layer = addDialog.selectedLayer();
      if ( layer )
      {
        mLegend->beginCommand( "Legend item added" );
        mLegend->modelV2()->rootGroup()->addLayer( layer );
        mLegend->endCommand();
      }
    }
  }
}

void QgsComposerLegendWidget::on_mRemoveToolButton_clicked()
{
  if ( !mLegend )
  {
    return;
  }

  QItemSelectionModel* selectionModel = mItemTreeView->selectionModel();
  if ( !selectionModel )
  {
    return;
  }

  mLegend->beginCommand( "Legend item removed" );

  QList<QPersistentModelIndex> indexes;
  foreach ( const QModelIndex &index, selectionModel->selectedIndexes() )
    indexes << index;

  // first try to remove legend nodes
  QHash<QgsLayerTreeLayer*, QList<int> > nodesWithRemoval;
  foreach ( const QPersistentModelIndex index, indexes )
  {
    if ( QgsLayerTreeModelLegendNode* legendNode = mItemTreeView->layerTreeModel()->index2legendNode( index ) )
    {
      QgsLayerTreeLayer* nodeLayer = legendNode->layerNode();
      nodesWithRemoval[nodeLayer].append( index.row() );
    }
  }
  foreach ( QgsLayerTreeLayer* nodeLayer, nodesWithRemoval.keys() )
  {
    QList<int> toDelete = nodesWithRemoval[nodeLayer];
    qSort( toDelete.begin(), toDelete.end(), qGreater<int>() );
    QList<int> order = QgsMapLayerLegendUtils::legendNodeOrder( nodeLayer );

    foreach ( int i, toDelete )
    {
      if ( i >= 0 && i < order.count() )
        order.removeAt( i );
    }

    QgsMapLayerLegendUtils::setLegendNodeOrder( nodeLayer, order );
    mItemTreeView->layerTreeModel()->refreshLayerLegend( nodeLayer );
  }

  // then remove layer tree nodes
  foreach ( const QPersistentModelIndex index, indexes )
  {
    if ( index.isValid() && mItemTreeView->layerTreeModel()->index2node( index ) )
      mLegend->modelV2()->removeRow( index.row(), index.parent() );
  }

  mLegend->adjustBoxSize();
  mLegend->update();
  mLegend->endCommand();
}

void QgsComposerLegendWidget::on_mEditPushButton_clicked()
{
  if ( !mLegend )
  {
    return;
  }

  QModelIndex idx = mItemTreeView->selectionModel()->currentIndex();
  QgsLayerTreeModel* model = mItemTreeView->layerTreeModel();
  QgsLayerTreeNode* currentNode = model->index2node( idx );
  QgsLayerTreeModelLegendNode* legendNode = model->index2legendNode( idx );
  QString currentText;

  if ( QgsLayerTree::isGroup( currentNode ) )
  {
    currentText = QgsLayerTree::toGroup( currentNode )->name();
  }
  else if ( QgsLayerTree::isLayer( currentNode ) )
  {
    currentText = QgsLayerTree::toLayer( currentNode )->layerName();
    QVariant v = currentNode->customProperty( "legend/title-label" );
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
    currentNode->setCustomProperty( "legend/title-label", newText );

    // force update of label of the legend node with embedded icon (a bit clumsy i know)
    QList<QgsLayerTreeModelLegendNode*> nodes = model->layerLegendNodes( QgsLayerTree::toLayer( currentNode ) );
    if ( nodes.count() == 1 && nodes[0]->isEmbeddedInParent() )
      nodes[0]->setUserLabel( QString() );
  }
  else if ( legendNode )
  {
    QList<int> order = QgsMapLayerLegendUtils::legendNodeOrder( legendNode->layerNode() );
    int originalIndex = ( idx.row() >= 0 && idx.row() < order.count() ? order[idx.row()] : -1 );
    QgsMapLayerLegendUtils::setLegendNodeUserLabel( legendNode->layerNode(), originalIndex, newText );
    model->refreshLayerLegend( legendNode->layerNode() );
  }

  mLegend->adjustBoxSize();
  mLegend->update();
  mLegend->endCommand();
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

  QgsLayerTreeLayer* nodeLayer = 0;
  if ( QgsLayerTreeNode* node = mItemTreeView->layerTreeModel()->index2node( currentIndex ) )
  {
    if ( QgsLayerTree::isLayer( node ) )
      nodeLayer = QgsLayerTree::toLayer( node );
  }
  if ( QgsLayerTreeModelLegendNode* legendNode = mItemTreeView->layerTreeModel()->index2legendNode( currentIndex ) )
  {
    nodeLayer = legendNode->layerNode();
  }

  if ( !nodeLayer )
    return;

  mLegend->beginCommand( tr( "Legend updated" ) );

  foreach ( QString key, nodeLayer->customProperties() )
  {
    if ( key.startsWith( "legend/" ) )
      nodeLayer->removeCustomProperty( key );
  }

  mItemTreeView->layerTreeModel()->refreshLayerLegend( nodeLayer );

  mLegend->update();
  mLegend->adjustBoxSize();
  mLegend->endCommand();
}

void QgsComposerLegendWidget::on_mCountToolButton_clicked( bool checked )
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

  QgsLayerTreeNode* currentNode = mItemTreeView->currentNode();
  if ( !QgsLayerTree::isLayer( currentNode ) )
    return;

  mLegend->beginCommand( tr( "Legend updated" ) );
  currentNode->setCustomProperty( "showFeatureCount", checked ? 1 : 0 );
  mLegend->update();
  mLegend->adjustBoxSize();
  mLegend->endCommand();
}

void QgsComposerLegendWidget::on_mFilterByMapToolButton_clicked( bool checked )
{
  mLegend->beginCommand( tr( "Legend updated" ) );
  mLegend->setLegendFilterByMapEnabled( checked );
  mLegend->update();
  mLegend->adjustBoxSize();
  mLegend->endCommand();
}

void QgsComposerLegendWidget::on_mUpdateAllPushButton_clicked()
{
  updateLegend();
}

void QgsComposerLegendWidget::on_mAddGroupToolButton_clicked()
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Legend group added" ) );
    mLegend->modelV2()->rootGroup()->addGroup( tr( "Group" ) );
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsComposerLegendWidget::updateLegend()
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Legend updated" ) );

    // this will reset the model completely, loosing any changes
    mLegend->setAutoUpdateModel( true );
    mLegend->setAutoUpdateModel( false );

    mLegend->update();
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
}

void QgsComposerLegendWidget::refreshMapComboBox()
{
  if ( !mLegend )
  {
    return;
  }

  const QgsComposition* composition = mLegend->composition();
  if ( !composition )
  {
    return;
  }

  //save current entry
  int currentMapId = mMapComboBox->itemData( mMapComboBox->currentIndex() ).toInt();
  mMapComboBox->clear();

  QList<const QgsComposerMap*> availableMaps = composition->composerMapItems();
  QList<const QgsComposerMap*>::const_iterator mapItemIt = availableMaps.constBegin();
  for ( ; mapItemIt != availableMaps.constEnd(); ++mapItemIt )
  {
    mMapComboBox->addItem( tr( "Map %1" ).arg(( *mapItemIt )->id() ), ( *mapItemIt )->id() );
  }
  mMapComboBox->addItem( tr( "None" ), -1 );

  //the former entry is not there anymore
  int entry = mMapComboBox->findData( currentMapId );
  if ( entry == -1 )
  {
  }
  else
  {
    mMapComboBox->setCurrentIndex( entry );
  }
}

void QgsComposerLegendWidget::showEvent( QShowEvent * event )
{
  refreshMapComboBox();
  QWidget::showEvent( event );
}

void QgsComposerLegendWidget::selectedChanged( const QModelIndex & current, const QModelIndex & previous )
{
  Q_UNUSED( current );
  Q_UNUSED( previous );
  QgsDebugMsg( "Entered" );

  mCountToolButton->setChecked( false );
  mCountToolButton->setEnabled( false );

  if ( mLegend && mLegend->autoUpdateModel() )
    return;

  QgsLayerTreeNode* currentNode = mItemTreeView->currentNode();
  if ( !QgsLayerTree::isLayer( currentNode ) )
    return;

  QgsLayerTreeLayer* currentLayerNode = QgsLayerTree::toLayer( currentNode );
  QgsVectorLayer* vl = qobject_cast<QgsVectorLayer*>( currentLayerNode->layer() );
  if ( !vl )
    return;

  mCountToolButton->setChecked( currentNode->customProperty( "showFeatureCount", 0 ).toInt() );
  mCountToolButton->setEnabled( true );
}

void QgsComposerLegendWidget::setCurrentNodeStyleFromAction()
{
  QAction* a = qobject_cast<QAction*>( sender() );
  if ( !a || !mItemTreeView->currentNode() )
    return;

  QgsLegendRenderer::setNodeLegendStyle( mItemTreeView->currentNode(), ( QgsComposerLegendStyle::Style ) a->data().toInt() );
  mLegend->update();
}
