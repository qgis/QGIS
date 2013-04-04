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
#include <QFontDialog>
#include <QColorDialog>

#include "qgsapplegendinterface.h"
#include "qgisapp.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaprenderer.h"
#include "qgsapplication.h"
#include "qgsvectorlayer.h"

#include <QMessageBox>

QgsComposerLegendWidgetStyleDelegate::QgsComposerLegendWidgetStyleDelegate( QObject *parent ): QItemDelegate( parent )
{
}

QWidget *QgsComposerLegendWidgetStyleDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem & item, const QModelIndex & index ) const
{
  Q_UNUSED( item );
  Q_UNUSED( index );
  QgsDebugMsg( "Entered" );

  QComboBox *editor = new QComboBox( parent );
  QList<QgsComposerLegendStyle::Style> list;
  list << QgsComposerLegendStyle::Group << QgsComposerLegendStyle::Subgroup << QgsComposerLegendStyle::Hidden;
  foreach ( QgsComposerLegendStyle::Style s, list )
  {
    editor->addItem( QgsComposerLegendStyle::styleLabel( s ), s );
  }

  return editor;
}

void QgsComposerLegendWidgetStyleDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
  QgsDebugMsg( "Entered" );
  //int s = index.model()->data(index, Qt::UserRole).toInt();
  QComboBox *comboBox = static_cast<QComboBox*>( editor );

  const QStandardItemModel * standardModel = dynamic_cast<const QStandardItemModel*>( index.model() );
  if ( standardModel )
  {
    QModelIndex firstColumnIndex = standardModel->index( index.row(), 0, index.parent() );
    QStandardItem* firstColumnItem = standardModel->itemFromIndex( firstColumnIndex );

    QgsComposerLegendItem* cItem = dynamic_cast<QgsComposerLegendItem*>( firstColumnItem );
    if ( cItem )
    {
      comboBox->setCurrentIndex( comboBox->findData( cItem->style() ) );
    }
  }
}

void QgsComposerLegendWidgetStyleDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  QgsDebugMsg( "Entered" );
  QComboBox *comboBox = static_cast<QComboBox*>( editor );
  QgsComposerLegendStyle::Style s = ( QgsComposerLegendStyle::Style ) comboBox->itemData( comboBox->currentIndex() ).toInt();
  model->setData( index, s, Qt::UserRole );
  model->setData( index, QgsComposerLegendStyle::styleLabel( s ), Qt::DisplayRole );
  QStandardItemModel * standardModel = dynamic_cast<QStandardItemModel*>( model );
  if ( standardModel )
  {
    QModelIndex firstColumnIndex = standardModel->index( index.row(), 0, index.parent() );
    QStandardItem* firstColumnItem = standardModel->itemFromIndex( firstColumnIndex );

    QgsComposerLegendItem* cItem = dynamic_cast<QgsComposerLegendItem*>( firstColumnItem );
    if ( cItem )
    {
      cItem->setStyle( s );
    }
  }
}

void QgsComposerLegendWidgetStyleDelegate::updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex & index ) const
{
  Q_UNUSED( index );
  editor->setGeometry( option.rect );
}


QgsComposerLegendWidget::QgsComposerLegendWidget( QgsComposerLegend* legend ): mLegend( legend )
{
  setupUi( this );

  // setup icons
  mAddToolButton->setIcon( QIcon( QgsApplication::iconPath( "symbologyAdd.png" ) ) );
  mEditPushButton->setIcon( QIcon( QgsApplication::iconPath( "symbologyEdit.png" ) ) );
  mRemoveToolButton->setIcon( QIcon( QgsApplication::iconPath( "symbologyRemove.png" ) ) );
  mMoveUpToolButton->setIcon( QIcon( QgsApplication::iconPath( "symbologyUp.png" ) ) );
  mMoveDownToolButton->setIcon( QIcon( QgsApplication::iconPath( "symbologyDown.png" ) ) );
  mCountToolButton->setIcon( QIcon( QgsApplication::iconPath( "mActionSum.png" ) ) );

  //add widget for item properties
  QgsComposerItemWidget* itemPropertiesWidget = new QgsComposerItemWidget( this, legend );
  mainLayout->addWidget( itemPropertiesWidget );

  if ( legend )
  {
    legend->model()->setHorizontalHeaderLabels( QStringList() << tr( "Item" ) << tr( "Title style" ) );
    mItemTreeView->setModel( legend->model() );
  }

  QgsComposerLegendWidgetStyleDelegate* styleDelegate = new QgsComposerLegendWidgetStyleDelegate();
  mItemTreeView->setItemDelegateForColumn( 0, styleDelegate );
  mItemTreeView->setItemDelegateForColumn( 1, styleDelegate );
  mItemTreeView->setEditTriggers( QAbstractItemView::AllEditTriggers );

  mItemTreeView->setDragEnabled( true );
  mItemTreeView->setAcceptDrops( true );
  mItemTreeView->setDropIndicatorShown( true );
  mItemTreeView->setDragDropMode( QAbstractItemView::InternalMove );
  mWrapCharLineEdit->setText( legend->wrapChar() );

  setGuiElements();

  connect( mItemTreeView->selectionModel(), SIGNAL( currentChanged( const QModelIndex &, const QModelIndex & ) ),
           this, SLOT( selectedChanged( const QModelIndex &, const QModelIndex & ) ) );
}

QgsComposerLegendWidget::QgsComposerLegendWidget(): mLegend( 0 )
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

  blockAllSignals( true );
  mTitleLineEdit->setText( mLegend->title() );
  mColumnCountSpinBox->setValue( mLegend->columnCount() );
  mSplitLayerCheckBox->setChecked( mLegend->splitLayer() );
  mEqualColumnWidthCheckBox->setChecked( mLegend->equalColumnWidth() );
  mSymbolWidthSpinBox->setValue( mLegend->symbolWidth() );
  mSymbolHeightSpinBox->setValue( mLegend->symbolHeight() );
  mGroupSpaceSpinBox->setValue( mLegend->style( QgsComposerLegendStyle::Group ).margin( QgsComposerLegendStyle::Top ) );
  mLayerSpaceSpinBox->setValue( mLegend->style( QgsComposerLegendStyle::Subgroup ).margin( QgsComposerLegendStyle::Top ) );
  // We keep Symbol and SymbolLabel Top in sync for now
  mSymbolSpaceSpinBox->setValue( mLegend->style( QgsComposerLegendStyle::Symbol ).margin( QgsComposerLegendStyle::Top ) );
  mIconLabelSpaceSpinBox->setValue( mLegend->style( QgsComposerLegendStyle::SymbolLabel ).margin( QgsComposerLegendStyle::Left ) );
  mBoxSpaceSpinBox->setValue( mLegend->boxSpace() );
  mColumnSpaceSpinBox->setValue( mLegend->columnSpace() );
  if ( mLegend->model() )
  {
    mCheckBoxAutoUpdate->setChecked( mLegend->model()->autoUpdate() );
  }
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
  blockAllSignals( false );
}

void QgsComposerLegendWidget::on_mWrapCharLineEdit_textChanged( const QString &text )
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Item wrapping changed" ), QgsComposerMergeCommand::ComposerLegendText );
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
#if defined(Q_WS_MAC) && QT_VERSION >= 0x040500 && defined(QT_MAC_USE_COCOA)
    // Native Mac dialog works only for Qt Carbon
    QFont newFont = QFontDialog::getFont( &ok, mLegend->style( QgsComposerLegendStyle::Title ).font(), 0, QString(), QFontDialog::DontUseNativeDialog );
#else
    QFont newFont = QFontDialog::getFont( &ok, mLegend->style( QgsComposerLegendStyle::Title ).font() );
#endif
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
#if defined(Q_WS_MAC) && QT_VERSION >= 0x040500 && defined(QT_MAC_USE_COCOA)
    // Native Mac dialog works only for Qt Carbon
    QFont newFont = QFontDialog::getFont( &ok, mLegend->style( QgsComposerLegendStyle::Group ).font(), 0, QString(), QFontDialog::DontUseNativeDialog );
#else
    QFont newFont = QFontDialog::getFont( &ok, mLegend->style( QgsComposerLegendStyle::Group ).font() );
#endif
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
#if defined(Q_WS_MAC) && QT_VERSION >= 0x040500 && defined(QT_MAC_USE_COCOA)
    // Native Mac dialog works only for Qt Carbon
    QFont newFont = QFontDialog::getFont( &ok, mLegend->style( QgsComposerLegendStyle::Subgroup ).font(), 0, QString(), QFontDialog::DontUseNativeDialog );
#else
    QFont newFont = QFontDialog::getFont( &ok, mLegend->style( QgsComposerLegendStyle::Subgroup ).font() );
#endif
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
#if defined(Q_WS_MAC) && QT_VERSION >= 0x040500 && defined(QT_MAC_USE_COCOA)
    // Native Mac dialog works only for Qt Carbon
    QFont newFont = QFontDialog::getFont( &ok, mLegend->style( QgsComposerLegendStyle::SymbolLabel ).font(), 0, QString(), QFontDialog::DontUseNativeDialog );
#else
    QFont newFont = QFontDialog::getFont( &ok, mLegend->style( QgsComposerLegendStyle::SymbolLabel ).font() );
#endif
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

void QgsComposerLegendWidget::on_mFontColorPushButton_clicked()
{
  if ( !mLegend )
  {
    return;
  }

  QColor oldColor = mLegend->fontColor();
  QColor newColor = QColorDialog::getColor( oldColor, 0 );

  if ( !newColor.isValid() ) //user canceled the dialog
  {
    return;
  }

  mLegend->beginCommand( tr( "Legend font color changed" ) );
  mLegend->setFontColor( newColor );
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

void QgsComposerLegendWidget::on_mMoveDownToolButton_clicked()
{
  if ( !mLegend )
  {
    return;
  }

  QStandardItemModel* itemModel = qobject_cast<QStandardItemModel *>( mItemTreeView->model() );
  if ( !itemModel )
  {
    return;
  }

  QModelIndex currentIndex = mItemTreeView->currentIndex();
  if ( !currentIndex.isValid() )
  {
    return;
  }

  //is there an older sibling?
  int row = currentIndex.row();
  QModelIndex youngerSibling = currentIndex.sibling( row + 1, 0 );

  if ( !youngerSibling.isValid() )
  {
    return;
  }

  mLegend->beginCommand( "Moved legend item down" );
  QModelIndex parentIndex = currentIndex.parent();
  QList<QStandardItem*> itemToMove;
  QList<QStandardItem*> youngerSiblingItem;

  if ( !parentIndex.isValid() ) //move toplevel (layer) item
  {
    youngerSiblingItem = itemModel->takeRow( row + 1 );
    itemToMove = itemModel->takeRow( row );
    itemModel->insertRow( row, youngerSiblingItem );
    itemModel->insertRow( row + 1, itemToMove );
  }
  else //move child (classification) item
  {
    QStandardItem* parentItem = itemModel->itemFromIndex( parentIndex );
    youngerSiblingItem = parentItem->takeRow( row + 1 );
    itemToMove = parentItem->takeRow( row );
    parentItem->insertRow( row, youngerSiblingItem );
    parentItem->insertRow( row + 1, itemToMove );
  }

  mItemTreeView->setCurrentIndex( itemModel->indexFromItem( itemToMove.at( 0 ) ) );
  mLegend->update();
  mLegend->endCommand();
}

void QgsComposerLegendWidget::on_mMoveUpToolButton_clicked()
{
  if ( !mLegend )
  {
    return;
  }

  QStandardItemModel* itemModel = qobject_cast<QStandardItemModel *>( mItemTreeView->model() );
  if ( !itemModel )
  {
    return;
  }

  QModelIndex currentIndex = mItemTreeView->currentIndex();
  if ( !currentIndex.isValid() )
  {
    return;
  }

  mLegend->beginCommand( "Moved legend item up" );
  //is there an older sibling?
  int row = currentIndex.row();
  QModelIndex olderSibling = currentIndex.sibling( row - 1, 0 );

  if ( !olderSibling.isValid() )
  {
    return;
  }

  QModelIndex parentIndex = currentIndex.parent();
  QList<QStandardItem*> itemToMove;
  QList<QStandardItem*> olderSiblingItem;

  if ( !parentIndex.isValid() ) //move toplevel item
  {
    itemToMove = itemModel->takeRow( row );
    olderSiblingItem = itemModel->takeRow( row - 1 );
    itemModel->insertRow( row - 1, itemToMove );
    itemModel->insertRow( row, olderSiblingItem );

  }
  else //move classification items
  {
    QStandardItem* parentItem = itemModel->itemFromIndex( parentIndex );
    itemToMove = parentItem->takeRow( row );
    olderSiblingItem = parentItem->takeRow( row - 1 );
    parentItem->insertRow( row - 1, itemToMove );
    parentItem->insertRow( row, olderSiblingItem );
  }

  mItemTreeView->setCurrentIndex( itemModel->indexFromItem( itemToMove.at( 0 ) ) );
  mLegend->update();
  mLegend->endCommand();
}

void QgsComposerLegendWidget::on_mCheckBoxAutoUpdate_stateChanged( int state )
{
  if ( !mLegend->model() )
  {
    return;
  }

  if ( state == Qt::Checked )
  {
    mLegend->model()->setAutoUpdate( true );
  }
  else
  {
    mLegend->model()->setAutoUpdate( false );
  }
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

  QStandardItemModel* itemModel = qobject_cast<QStandardItemModel *>( mItemTreeView->model() );
  if ( !itemModel )
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

    QgsComposerLegendLayersDialog addDialog( layers );
    if ( addDialog.exec() == QDialog::Accepted )
    {
      QgsMapLayer* layer = addDialog.selectedLayer();
      if ( layer )
      {
        mLegend->beginCommand( "Legend item added" );
        mLegend->model()->addLayer( layer );
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

  QStandardItemModel* itemModel = qobject_cast<QStandardItemModel *>( mItemTreeView->model() );
  if ( !itemModel )
  {
    return;
  }

  mLegend->beginCommand( "Legend item removed" );

  QItemSelectionModel* selectionModel = mItemTreeView->selectionModel();
  if ( !selectionModel )
  {
    return;
  }

  QModelIndexList selection = selectionModel->selectedIndexes();
  for ( int i = selection.size() - 1; i >= 0; --i )
  {
    QModelIndex parentIndex = selection.at( i ).parent();
    itemModel->removeRow( selection.at( i ).row(), parentIndex );
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

  QStandardItemModel* itemModel = qobject_cast<QStandardItemModel *>( mItemTreeView->model() );
  if ( !itemModel )
  {
    return;
  }

  //get current item
  QModelIndex currentIndex = mItemTreeView->currentIndex();
  if ( !currentIndex.isValid() )
  {
    return;
  }

  QStandardItem* currentItem = itemModel->itemFromIndex( currentIndex );
  if ( !currentItem )
  {
    return;
  }

  QgsComposerLegendItemDialog itemDialog( currentItem );
  if ( itemDialog.exec() == QDialog::Accepted )
  {
    currentItem->setText( itemDialog.itemText() );
  }

  mLegend->beginCommand( tr( "Legend item edited" ) );
  mLegend->adjustBoxSize();
  mLegend->update();
  mLegend->endCommand();
}

void QgsComposerLegendWidget::on_mUpdatePushButton_clicked()
{
  if ( !mLegend )
  {
    return;
  }

  //get current item
  QStandardItemModel* itemModel = qobject_cast<QStandardItemModel *>( mItemTreeView->model() );
  if ( !itemModel )
  {
    return;
  }

  //get current item
  QModelIndex currentIndex = mItemTreeView->currentIndex();
  if ( !currentIndex.isValid() )
  {
    return;
  }

  QStandardItem* currentItem = itemModel->itemFromIndex( currentIndex );
  if ( !currentItem )
  {
    return;
  }

  mLegend->beginCommand( tr( "Legend updated" ) );
  if ( mLegend->model() )
  {
    mLegend->model()->updateItem( currentItem );
  }
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
  QStandardItemModel* itemModel = qobject_cast<QStandardItemModel *>( mItemTreeView->model() );
  if ( !itemModel )
  {
    return;
  }

  //get current item
  QModelIndex currentIndex = mItemTreeView->currentIndex();
  if ( !currentIndex.isValid() )
  {
    return;
  }

  QStandardItem* currentItem = itemModel->itemFromIndex( currentIndex );
  if ( !currentItem )
  {
    return;
  }

  QgsComposerLayerItem* layerItem = dynamic_cast<QgsComposerLayerItem *>( currentItem );

  if ( !layerItem )
  {
    return;
  }

  mLegend->beginCommand( tr( "Legend updated" ) );
  layerItem->setShowFeatureCount( checked );
  if ( mLegend->model() )
  {
    mLegend->model()->updateItem( currentItem );
  }
  mLegend->update();
  mLegend->adjustBoxSize();
  mLegend->endCommand();
}

void QgsComposerLegendWidget::on_mUpdateAllPushButton_clicked()
{
  updateLegend();
}

void QgsComposerLegendWidget::on_mAddGroupButton_clicked()
{
  if ( mLegend && mLegend->model() )
  {
    mLegend->beginCommand( tr( "Legend group added" ) );
    mLegend->model()->addGroup();
    mLegend->update();
    mLegend->endCommand();
  }
}

void QgsComposerLegendWidget::updateLegend()
{
  if ( mLegend )
  {
    mLegend->beginCommand( tr( "Legend updated" ) );
    QgisApp* app = QgisApp::instance();
    if ( !app )
    {
      return;
    }

    //get layer id list
    QStringList layerIdList;
    QgsMapCanvas* canvas = app->mapCanvas();
    if ( canvas )
    {
      QgsMapRenderer* renderer = canvas->mapRenderer();
      if ( renderer )
      {
        layerIdList = renderer->layerSet();
      }
    }

    //and also group info
    QgsAppLegendInterface legendIface( app->legend() );
    QList< GroupLayerInfo > groupInfo = legendIface.groupLayerRelationship();
    mLegend->model()->setLayerSetAndGroups( layerIdList, groupInfo );
    mLegend->endCommand();
  }
}

void QgsComposerLegendWidget::blockAllSignals( bool b )
{
  mTitleLineEdit->blockSignals( b );
  mItemTreeView->blockSignals( b );
  mCheckBoxAutoUpdate->blockSignals( b );
  mMapComboBox->blockSignals( b );
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
  Q_UNUSED( previous );
  QgsDebugMsg( "Entered" );

  mCountToolButton->setChecked( false );
  mCountToolButton->setEnabled( false );

  QStandardItemModel* itemModel = qobject_cast<QStandardItemModel *>( mItemTreeView->model() );
  if ( !itemModel ) return;

  QStandardItem* currentItem = itemModel->itemFromIndex( current );
  if ( !currentItem ) return;

  QgsComposerLayerItem* layerItem = dynamic_cast<QgsComposerLayerItem *>( currentItem );
  if ( !layerItem ) return;

  QgsVectorLayer* vectorLayer = dynamic_cast<QgsVectorLayer*>( QgsMapLayerRegistry::instance()->mapLayer( layerItem->layerID() ) );
  if ( !vectorLayer ) return;

  mCountToolButton->setChecked( layerItem->showFeatureCount() );
  mCountToolButton->setEnabled( true );
}
