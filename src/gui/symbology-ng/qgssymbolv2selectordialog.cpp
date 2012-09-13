/***************************************************************************
    qgssymbolv2selectordialog.cpp
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder.sk at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssymbolv2selectordialog.h"

#include "qgsstylev2.h"
#include "qgssymbolv2.h"
#include "qgssymbollayerv2.h"
#include "qgssymbollayerv2utils.h"
#include "qgssymbollayerv2registry.h"

// the widgets
#include "qgssymbolslistwidget.h"
#include "qgslayerpropertieswidget.h"
#include "qgssymbollayerv2widget.h"
#include "qgsellipsesymbollayerv2widget.h"
#include "qgsvectorfieldsymbollayerwidget.h"

#include "qgslogger.h"
#include "qgsapplication.h"

#include <QColorDialog>
#include <QPainter>
#include <QStandardItemModel>
#include <QInputDialog>
#include <QMessageBox>
#include <QKeyEvent>
#include <QMenu>

#include <QWidget>
#include <QFile>
#include <QStandardItem>

static const int SymbolLayerItemType = QStandardItem::UserType + 1;

// Hybrid item which may represent a symbol or a layer
// Check using item->isLayer()
class SymbolLayerItem : public QStandardItem
{
  public:
    SymbolLayerItem( QgsSymbolLayerV2* layer )
    {
      setLayer( layer );
    }

    SymbolLayerItem( QgsSymbolV2* symbol )
    {
      setSymbol( symbol );
    }

    void setLayer( QgsSymbolLayerV2* layer )
    {
      mLayer = layer;
      mIsLayer = true;
      mSymbol = NULL;
      updatePreview();
    }

    void setSymbol( QgsSymbolV2* symbol )
    {
      mSymbol = symbol;
      mIsLayer = false;
      mLayer = NULL;
      updatePreview();
    }

    void updatePreview()
    {
      QIcon icon;
      if ( mIsLayer )
        icon = QgsSymbolLayerV2Utils::symbolLayerPreviewIcon( mLayer, QgsSymbolV2::MM, QSize( 16, 16 ) ); //todo: make unit a parameter
      else
        icon = QgsSymbolLayerV2Utils::symbolPreviewIcon( mSymbol, QSize( 16, 16 ) );
      setIcon( icon );

      if ( parent() )
        static_cast<SymbolLayerItem*>( parent() )->updatePreview();
    }

    int type() const { return SymbolLayerItemType; }
    bool isLayer() { return mIsLayer; }

    // returns the symbol pointer; helpful in determining a layer's parent symbol
    QgsSymbolV2* symbol()
    {
      if ( mIsLayer )
        return NULL;
      return mSymbol;
    }

    QgsSymbolLayerV2* layer()
    {
      if ( mIsLayer )
        return mLayer;
      return NULL;
    }

    QVariant data( int role ) const
    {
      if ( role == Qt::DisplayRole || role == Qt::EditRole )
      {
        if ( mIsLayer )
          return QgsSymbolLayerV2Registry::instance()->symbolLayerMetadata( mLayer->layerType() )->visibleName();
        else
        {
          switch ( mSymbol->type() )
          {
            case QgsSymbolV2::Marker : return "Symbol: Marker";
            case QgsSymbolV2::Fill   : return "Symbol: Fill";
            case QgsSymbolV2::Line   : return "Symbol: Line";
            default: return "Symbol";
          }
        }
      }
      if ( role == Qt::SizeHintRole )
        return QVariant( QSize( 32, 32 ) );
      if ( role == Qt::CheckStateRole )
        return QVariant(); // could be true/false
      return QStandardItem::data( role );
    }

  protected:
    QgsSymbolLayerV2* mLayer;
    QgsSymbolV2* mSymbol;
    bool mIsLayer;
};

//////////

QgsSymbolV2SelectorDialog::QgsSymbolV2SelectorDialog( QgsSymbolV2* symbol, QgsStyleV2* style, const QgsVectorLayer* vl, QWidget* parent, bool embedded )
    : QDialog( parent ), mAdvancedMenu( NULL ), mVectorLayer( vl )
{
  mStyle = style;
  mSymbol = symbol;
  mPresentWidget = NULL;

  setupUi( this );
  // can be embedded in renderer properties dialog
  if ( embedded )
  {
    buttonBox->hide();
    layout()->setContentsMargins( 0, 0, 0, 0 );
  }
  // setup icons
  btnAddLayer->setIcon( QIcon( QgsApplication::iconPath( "symbologyAdd.png" ) ) );
  btnRemoveLayer->setIcon( QIcon( QgsApplication::iconPath( "symbologyRemove.png" ) ) );
  QIcon iconLock;
  iconLock.addFile( QgsApplication::iconPath( "locked.png" ), QSize(), QIcon::Normal, QIcon::On );
  iconLock.addFile( QgsApplication::iconPath( "unlocked.png" ), QSize(), QIcon::Normal, QIcon::Off );
  btnLock->setIcon( iconLock );
  btnUp->setIcon( QIcon( QgsApplication::iconPath( "symbologyUp.png" ) ) );
  btnDown->setIcon( QIcon( QgsApplication::iconPath( "symbologyDown.png" ) ) );

  model = new QStandardItemModel();
  // Set the symbol
  layersTree->setModel( model );
  layersTree->setHeaderHidden( true );

  QItemSelectionModel* selModel = layersTree->selectionModel();
  connect( selModel, SIGNAL( currentChanged( const QModelIndex&, const QModelIndex& ) ), this, SLOT( layerChanged() ) );

  loadSymbol( symbol, static_cast<SymbolLayerItem*>( model->invisibleRootItem() ) );
  updatePreview();

  connect( btnUp, SIGNAL( clicked() ), this, SLOT( moveLayerUp() ) );
  connect( btnDown, SIGNAL( clicked() ), this, SLOT( moveLayerDown() ) );
  connect( btnAddLayer, SIGNAL( clicked() ), this, SLOT( addLayer() ) );
  connect( btnRemoveLayer, SIGNAL( clicked() ), this, SLOT( removeLayer() ) );
  connect( btnLock, SIGNAL( clicked() ), this, SLOT( lockLayer() ) );

  updateUi();

  // set symbol as active item in the tree
  QModelIndex newIndex = layersTree->model()->index( 0, 0 );
  layersTree->setCurrentIndex( newIndex );
}

void QgsSymbolV2SelectorDialog::keyPressEvent( QKeyEvent * e )
{
  // Ignore the ESC key to avoid close the dialog without the properties window
  if ( !isWindow() && e->key() == Qt::Key_Escape )
  {
    e->ignore();
  }
  else
  {
    QDialog::keyPressEvent( e );
  }
}

QMenu* QgsSymbolV2SelectorDialog::advancedMenu()
{
  if ( mAdvancedMenu == NULL )
  {
    mAdvancedMenu = new QMenu;
    // Brute force method to activate the Advanced menu
    layerChanged();
  }
  return mAdvancedMenu;
}

void QgsSymbolV2SelectorDialog::loadSymbol( QgsSymbolV2* symbol, SymbolLayerItem* parent )
{
  SymbolLayerItem* symbolItem = new SymbolLayerItem( symbol );
  QFont boldFont = symbolItem->font();
  boldFont.setBold( true );
  symbolItem->setFont( boldFont );
  parent->appendRow( symbolItem );

  int count = symbol->symbolLayerCount();
  for ( int i = count - 1; i >= 0; i-- )
  {
    SymbolLayerItem *layerItem = new SymbolLayerItem( symbol->symbolLayer( i ) );
    layerItem->setEditable( false );
    symbolItem->appendRow( layerItem );
    if ( symbol->symbolLayer( i )->subSymbol() )
    {
      loadSymbol( symbol->symbolLayer( i )->subSymbol(), layerItem );
    }
  }
  layersTree->setExpanded( symbolItem->index(), true );
}


void QgsSymbolV2SelectorDialog::loadSymbol()
{
  model->clear();
  loadSymbol( mSymbol, static_cast<SymbolLayerItem*>( model->invisibleRootItem() ) );
}

void QgsSymbolV2SelectorDialog::updateUi()
{
  QModelIndex currentIdx =  layersTree->currentIndex();
  if ( !currentIdx.isValid() )
    return;

  SymbolLayerItem *item = static_cast<SymbolLayerItem*>( model->itemFromIndex( currentIdx ) );
  if ( !item->isLayer() )
  {
    btnUp->setEnabled( false );
    btnDown->setEnabled( false );
    btnRemoveLayer->setEnabled( false );
    btnLock->setEnabled( false );
    btnAddLayer->setEnabled( true );
    return;
  }

  int rowCount = item->parent()->rowCount();
  int currentRow = item->row();

  btnUp->setEnabled( currentRow > 0 );
  btnDown->setEnabled( currentRow < rowCount - 1 );
  btnRemoveLayer->setEnabled( rowCount > 1 );
  btnLock->setEnabled( true );
  btnAddLayer->setEnabled( false );
}

void QgsSymbolV2SelectorDialog::updatePreview()
{
  QImage preview = mSymbol->bigSymbolPreviewImage();
  lblPreview->setPixmap( QPixmap::fromImage( preview ) );
  // Hope this is a appropriate place
  emit symbolModified();
}

void QgsSymbolV2SelectorDialog::updateLayerPreview()
{
  // get current layer item and update its icon
  SymbolLayerItem* item = currentLayerItem();
  if ( item )
    item->updatePreview();
  // update also preview of the whole symbol
  updatePreview();
}

SymbolLayerItem* QgsSymbolV2SelectorDialog::currentLayerItem()
{
  QModelIndex idx = layersTree->currentIndex();
  if ( !idx.isValid() )
    return NULL;

  SymbolLayerItem *item = static_cast<SymbolLayerItem*>( model->itemFromIndex( idx ) );
  if ( !item->isLayer() )
    return NULL;

  return item;
}

QgsSymbolLayerV2* QgsSymbolV2SelectorDialog::currentLayer()
{
  QModelIndex idx = layersTree->currentIndex();
  if ( !idx.isValid() )
    return NULL;

  SymbolLayerItem *item = static_cast<SymbolLayerItem*>( model->itemFromIndex( idx ) );
  if ( item->isLayer() )
    return item->layer();

  return NULL;
}

void QgsSymbolV2SelectorDialog::layerChanged()
{
  updateUi();

  SymbolLayerItem *currentItem = static_cast<SymbolLayerItem*>( model->itemFromIndex( layersTree->currentIndex() ) );
  if ( currentItem == NULL )
    return;

  if ( currentItem->isLayer() )
  {
    SymbolLayerItem *parent = static_cast<SymbolLayerItem*>( currentItem->parent() );
    QWidget *layerProp = new QgsLayerPropertiesWidget( currentItem->layer(), parent->symbol(), mVectorLayer );
    setWidget( layerProp );
    connect( layerProp, SIGNAL( changed() ), this, SLOT( updateLayerPreview() ) );
    // This connection when layer type is changed
    connect( layerProp, SIGNAL( changeLayer( QgsSymbolLayerV2* ) ), this, SLOT( changeLayer( QgsSymbolLayerV2* ) ) );
  }
  else
  {
    // then it must be a symbol
    // Now populate symbols of that type using the symbols list widget:
    QWidget *symbolsList = new QgsSymbolsListWidget( currentItem->symbol(), mStyle, mAdvancedMenu );
    setWidget( symbolsList );
    connect( symbolsList, SIGNAL( changed() ), this, SLOT( symbolChanged() ) );
  }
  updateLockButton();
}

void QgsSymbolV2SelectorDialog::symbolChanged()
{
  SymbolLayerItem *currentItem = static_cast<SymbolLayerItem*>( model->itemFromIndex( layersTree->currentIndex() ) );
  if ( currentItem == NULL || currentItem->isLayer() )
    return;
  // disconnect to avoid recreating widget
  disconnect( layersTree->selectionModel(), SIGNAL( currentChanged( const QModelIndex&, const QModelIndex& ) ), this, SLOT( layerChanged() ) );
  if ( currentItem->parent() )
  {
    // it is a sub-symbol
    QgsSymbolV2* symbol = currentItem->symbol();
    SymbolLayerItem *parent = static_cast<SymbolLayerItem*>( currentItem->parent() );
    parent->removeRow( 0 );
    loadSymbol( symbol, parent );
    layersTree->setCurrentIndex( parent->child( 0 )->index() );
    parent->updatePreview();
  }
  else
  {
    //it is the symbol itself
    loadSymbol();
    QModelIndex newIndex = layersTree->model()->index( 0, 0 );
    layersTree->setCurrentIndex( newIndex );
  }
  updatePreview();
  // connect it back once things are set
  connect( layersTree->selectionModel(), SIGNAL( currentChanged( const QModelIndex&, const QModelIndex& ) ), this, SLOT( layerChanged() ) );
}

void QgsSymbolV2SelectorDialog::setWidget( QWidget* widget )
{
  int index = stackedWidget->addWidget( widget );
  stackedWidget->setCurrentIndex( index );
  if ( mPresentWidget )
  {
    stackedWidget->removeWidget( mPresentWidget );
    QWidget *dummy = mPresentWidget;
    mPresentWidget = widget;
    delete dummy; // auto disconnects all signals
  }
}

void QgsSymbolV2SelectorDialog::updateLockButton()
{
  QgsSymbolLayerV2* layer = currentLayer();
  if ( !layer )
    return;
  btnLock->setChecked( layer->isLocked() );
}

void QgsSymbolV2SelectorDialog::addLayer()
{
  QModelIndex idx = layersTree->currentIndex();
  if ( !idx.isValid() )
    return;

  SymbolLayerItem *item = static_cast<SymbolLayerItem*>( model->itemFromIndex( idx ) );
  if ( item->isLayer() )
  {
    QMessageBox::critical( this, tr( "Invalid Selection!" ), tr( "Kindly select a symbol to add layer." ) );
    return;
  }

  QgsSymbolV2* parentSymbol = item->symbol();
  QgsSymbolLayerV2* newLayer = QgsSymbolLayerV2Registry::instance()->defaultSymbolLayer( parentSymbol->type() );
  parentSymbol->appendSymbolLayer( newLayer );
  // XXX Insane behaviour of the appendSymbolLayer, it actually "pushes" into the list
  SymbolLayerItem *newLayerItem = new SymbolLayerItem( newLayer );
  item->insertRow( 0, newLayerItem );
  item->updatePreview();

  layersTree->setCurrentIndex( model->indexFromItem( newLayerItem ) );
  updateUi();
  updatePreview();
}

void QgsSymbolV2SelectorDialog::removeLayer()
{
  SymbolLayerItem *item = currentLayerItem();
  int row = item->row();
  SymbolLayerItem *parent = static_cast<SymbolLayerItem*>( item->parent() );

  int layerIdx = parent->rowCount() - row - 1; // IMPORTANT
  QgsSymbolV2* parentSymbol = parent->symbol();
  QgsSymbolLayerV2 *tmpLayer = parentSymbol->takeSymbolLayer( layerIdx );

  parent->removeRow( row );
  parent->updatePreview();

  QModelIndex newIdx = parent->child( 0 )->index();
  layersTree->setCurrentIndex( newIdx );

  updateUi();
  updatePreview();
  //finally delete the removed layer pointer
  delete tmpLayer;
}

void QgsSymbolV2SelectorDialog::moveLayerDown()
{
  moveLayerByOffset( + 1 );
}

void QgsSymbolV2SelectorDialog::moveLayerUp()
{
  moveLayerByOffset( -1 );
}

void QgsSymbolV2SelectorDialog::moveLayerByOffset( int offset )
{
  SymbolLayerItem *item = currentLayerItem();
  if ( item == NULL )
    return;
  int row = item->row();

  SymbolLayerItem *parent = static_cast<SymbolLayerItem*>( item->parent() );
  QgsSymbolV2* parentSymbol = parent->symbol();

  int layerIdx = parent->rowCount() - row - 1;
  // switch layers
  QgsSymbolLayerV2* tmpLayer = parentSymbol->takeSymbolLayer( layerIdx );
  parentSymbol->insertSymbolLayer( layerIdx - offset, tmpLayer );

  QList<QStandardItem*> rowItems = parent->takeRow( row );
  parent->insertRows( row + offset, rowItems );
  parent->updatePreview();

  QModelIndex newIdx = rowItems[ 0 ]->index();
  layersTree->setCurrentIndex( newIdx );

  updatePreview();
  updateUi();
}

void QgsSymbolV2SelectorDialog::lockLayer()
{
  QgsSymbolLayerV2* layer = currentLayer();
  if ( !layer )
    return;
  layer->setLocked( btnLock->isChecked() );
}

void QgsSymbolV2SelectorDialog::changeLayer( QgsSymbolLayerV2* newLayer )
{
  SymbolLayerItem *item = currentLayerItem();
  QgsSymbolLayerV2* layer = item->layer();

  if ( layer->subSymbol() )
  {
    item->removeRow( 0 );
  }
  // update symbol layer item
  item->setLayer( newLayer );
  // When it is a marker symbol
  if ( newLayer->subSymbol() )
  {
    loadSymbol( newLayer->subSymbol(), item );
    layersTree->setExpanded( item->index(), true );
  }

  // Change the symbol at last to avoid deleting item's layer
  QgsSymbolV2* symbol = static_cast<SymbolLayerItem*>( item->parent() )->symbol();
  int layerIdx = item->parent()->rowCount() - item->row() - 1;
  symbol->changeSymbolLayer( layerIdx, newLayer );

  item->updatePreview();
  updatePreview();
  // Important: This lets the layer to have its own layer properties widget
  layerChanged();
}
