/***************************************************************************
    qgssymbolv2selectordialog.cpp
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
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
#include "qgsdatadefined.h"

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

/// @cond PRIVATE

static const int SymbolLayerItemType = QStandardItem::UserType + 1;

DataDefinedRestorer::DataDefinedRestorer( QgsSymbolV2* symbol, const QgsSymbolLayerV2* symbolLayer )
    : mMarker( nullptr )
    , mMarkerSymbolLayer( nullptr )
    , mLine( nullptr )
    , mLineSymbolLayer( nullptr )
{
  if ( symbolLayer->type() == QgsSymbolV2::Marker && symbol->type() == QgsSymbolV2::Marker )
  {
    Q_ASSERT( symbol->type() == QgsSymbolV2::Marker );
    mMarker = static_cast<QgsMarkerSymbolV2*>( symbol );
    mMarkerSymbolLayer = static_cast<const QgsMarkerSymbolLayerV2*>( symbolLayer );
    mDDSize = mMarker->dataDefinedSize();
    mDDAngle = mMarker->dataDefinedAngle();
    // check if restore is actually needed
    if ( mDDSize == QgsDataDefined() && mDDAngle == QgsDataDefined() )
      mMarker = nullptr;
  }
  else if ( symbolLayer->type() == QgsSymbolV2::Line && symbol->type() == QgsSymbolV2::Line )
  {
    mLine = static_cast<QgsLineSymbolV2*>( symbol );
    mLineSymbolLayer = static_cast<const QgsLineSymbolLayerV2*>( symbolLayer );
    mDDWidth = mLine->dataDefinedWidth();
    // check if restore is actually needed
    if ( mDDWidth == QgsDataDefined() )
      mLine = nullptr;
  }
  save();
}

void DataDefinedRestorer::save()
{
  if ( mMarker )
  {
    mSize = mMarkerSymbolLayer->size();
    mAngle = mMarkerSymbolLayer->angle();
    mMarkerOffset = mMarkerSymbolLayer->offset();
  }
  else if ( mLine )
  {
    mWidth = mLineSymbolLayer->width();
    mLineOffset = mLineSymbolLayer->offset();
  }
}

void DataDefinedRestorer::restore()
{
  if ( mMarker )
  {
    if ( mDDSize != QgsDataDefined() &&
         ( mSize != mMarkerSymbolLayer->size() || mMarkerOffset != mMarkerSymbolLayer->offset() ) )
      mMarker->setDataDefinedSize( mDDSize );
    if ( mDDAngle != QgsDataDefined() &&
         mAngle != mMarkerSymbolLayer->angle() )
      mMarker->setDataDefinedAngle( mDDAngle );
  }
  else if ( mLine )
  {
    if ( mDDWidth != QgsDataDefined() &&
         ( mWidth != mLineSymbolLayer->width() || mLineOffset != mLineSymbolLayer->offset() ) )
      mLine->setDataDefinedWidth( mDDWidth );
  }
  save();
}

// Hybrid item which may represent a symbol or a layer
// Check using item->isLayer()
class SymbolLayerItem : public QStandardItem
{
  public:
    explicit SymbolLayerItem( QgsSymbolLayerV2* layer )
    {
      setLayer( layer );
    }

    explicit SymbolLayerItem( QgsSymbolV2* symbol )
    {
      setSymbol( symbol );
    }

    void setLayer( QgsSymbolLayerV2* layer )
    {
      mLayer = layer;
      mIsLayer = true;
      mSymbol = nullptr;
      updatePreview();
    }

    void setSymbol( QgsSymbolV2* symbol )
    {
      mSymbol = symbol;
      mIsLayer = false;
      mLayer = nullptr;
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

    int type() const override { return SymbolLayerItemType; }
    bool isLayer() { return mIsLayer; }

    // returns the symbol pointer; helpful in determining a layer's parent symbol
    QgsSymbolV2* symbol()
    {
      return mSymbol;
    }

    QgsSymbolLayerV2* layer()
    {
      return mLayer;
    }

    QVariant data( int role ) const override
    {
      if ( role == Qt::DisplayRole || role == Qt::EditRole )
      {
        if ( mIsLayer )
          return QgsSymbolLayerV2Registry::instance()->symbolLayerMetadata( mLayer->layerType() )->visibleName();
        else
        {
          switch ( mSymbol->type() )
          {
            case QgsSymbolV2::Marker :
              return QCoreApplication::translate( "SymbolLayerItem", "Marker", nullptr, QCoreApplication::UnicodeUTF8 );
            case QgsSymbolV2::Fill   :
              return QCoreApplication::translate( "SymbolLayerItem", "Fill", nullptr, QCoreApplication::UnicodeUTF8 );
            case QgsSymbolV2::Line   :
              return QCoreApplication::translate( "SymbolLayerItem", "Line", nullptr, QCoreApplication::UnicodeUTF8 );
            default:
              return "Symbol";
          }
        }
      }
//      if ( role == Qt::SizeHintRole )
//        return QVariant( QSize( 32, 32 ) );
      if ( role == Qt::CheckStateRole )
        return QVariant(); // could be true/false
      return QStandardItem::data( role );
    }

  protected:
    QgsSymbolLayerV2* mLayer;
    QgsSymbolV2* mSymbol;
    bool mIsLayer;
};

///@endcond

//////////

QgsSymbolV2SelectorWidget::QgsSymbolV2SelectorWidget( QgsSymbolV2* symbol, QgsStyleV2* style, const QgsVectorLayer* vl, QWidget* parent )
    : QgsPanelWidget( parent )
    , mAdvancedMenu( nullptr )
    , mVectorLayer( vl )
    , mMapCanvas( nullptr )
{
#ifdef Q_OS_MAC
  setWindowModality( Qt::WindowModal );
#endif
  mStyle = style;
  mSymbol = symbol;
  mPresentWidget = nullptr;

  setupUi( this );

  // setup icons
  btnAddLayer->setIcon( QIcon( QgsApplication::iconPath( "symbologyAdd.svg" ) ) );
  btnRemoveLayer->setIcon( QIcon( QgsApplication::iconPath( "symbologyRemove.svg" ) ) );
  QIcon iconLock;
  iconLock.addFile( QgsApplication::iconPath( "locked.svg" ), QSize(), QIcon::Normal, QIcon::On );
  iconLock.addFile( QgsApplication::iconPath( "locked.svg" ), QSize(), QIcon::Active, QIcon::On );
  iconLock.addFile( QgsApplication::iconPath( "unlocked.svg" ), QSize(), QIcon::Normal, QIcon::Off );
  iconLock.addFile( QgsApplication::iconPath( "unlocked.svg" ), QSize(), QIcon::Active, QIcon::Off );
  btnLock->setIcon( iconLock );
  btnDuplicate->setIcon( QIcon( QgsApplication::iconPath( "mActionDuplicateLayer.svg" ) ) );
  btnUp->setIcon( QIcon( QgsApplication::iconPath( "symbologyUp.svg" ) ) );
  btnDown->setIcon( QIcon( QgsApplication::iconPath( "symbologyDown.svg" ) ) );

  model = new QStandardItemModel( layersTree );
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
  connect( btnDuplicate, SIGNAL( clicked() ), this, SLOT( duplicateLayer() ) );
  connect( this, SIGNAL( symbolModified() ), this, SIGNAL( widgetChanged() ) );

  updateUi();

  // set symbol as active item in the tree
  QModelIndex newIndex = layersTree->model()->index( 0, 0 );
  layersTree->setCurrentIndex( newIndex );

  setPanelTitle( tr( "Symbol selector" ) );
}

QgsSymbolV2SelectorWidget::~QgsSymbolV2SelectorWidget()
{
}

QMenu* QgsSymbolV2SelectorWidget::advancedMenu()
{
  if ( !mAdvancedMenu )
  {
    mAdvancedMenu = new QMenu( this );
    // Brute force method to activate the Advanced menu
    layerChanged();
  }
  return mAdvancedMenu;
}

void QgsSymbolV2SelectorWidget::setExpressionContext( QgsExpressionContext *context )
{
  mPresetExpressionContext.reset( context );
  layerChanged();
  updatePreview();
}

void QgsSymbolV2SelectorWidget::setMapCanvas( QgsMapCanvas *canvas )
{
  mMapCanvas = canvas;

  QWidget* widget = stackedWidget->currentWidget();
  QgsLayerPropertiesWidget* layerProp = dynamic_cast< QgsLayerPropertiesWidget* >( widget );
  QgsSymbolsListWidget* listWidget = dynamic_cast< QgsSymbolsListWidget* >( widget );

  if ( layerProp )
    layerProp->setMapCanvas( canvas );
  if ( listWidget )
    listWidget->setMapCanvas( canvas );
}

void QgsSymbolV2SelectorWidget::loadSymbol( QgsSymbolV2* symbol, SymbolLayerItem* parent )
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
    layersTree->setExpanded( layerItem->index(), true );
  }
  layersTree->setExpanded( symbolItem->index(), true );
}


void QgsSymbolV2SelectorWidget::loadSymbol()
{
  model->clear();
  loadSymbol( mSymbol, static_cast<SymbolLayerItem*>( model->invisibleRootItem() ) );
}

void QgsSymbolV2SelectorWidget::updateUi()
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
    btnDuplicate->setEnabled( false );
    return;
  }

  int rowCount = item->parent()->rowCount();
  int currentRow = item->row();

  btnUp->setEnabled( currentRow > 0 );
  btnDown->setEnabled( currentRow < rowCount - 1 );
  btnRemoveLayer->setEnabled( rowCount > 1 );
  btnLock->setEnabled( true );
  btnDuplicate->setEnabled( true );
}

void QgsSymbolV2SelectorWidget::updatePreview()
{
  QImage preview = mSymbol->bigSymbolPreviewImage( mPresetExpressionContext.data() );
  lblPreview->setPixmap( QPixmap::fromImage( preview ) );
  // Hope this is a appropriate place
  emit symbolModified();
}

void QgsSymbolV2SelectorWidget::updateLayerPreview()
{
  // get current layer item and update its icon
  SymbolLayerItem* item = currentLayerItem();
  if ( item )
    item->updatePreview();
  // update also preview of the whole symbol
  updatePreview();
}

SymbolLayerItem* QgsSymbolV2SelectorWidget::currentLayerItem()
{
  QModelIndex idx = layersTree->currentIndex();
  if ( !idx.isValid() )
    return nullptr;

  SymbolLayerItem *item = static_cast<SymbolLayerItem*>( model->itemFromIndex( idx ) );
  if ( !item->isLayer() )
    return nullptr;

  return item;
}

QgsSymbolLayerV2* QgsSymbolV2SelectorWidget::currentLayer()
{
  QModelIndex idx = layersTree->currentIndex();
  if ( !idx.isValid() )
    return nullptr;

  SymbolLayerItem *item = static_cast<SymbolLayerItem*>( model->itemFromIndex( idx ) );
  if ( item->isLayer() )
    return item->layer();

  return nullptr;
}

void QgsSymbolV2SelectorWidget::layerChanged()
{
  updateUi();

  SymbolLayerItem *currentItem = static_cast<SymbolLayerItem*>( model->itemFromIndex( layersTree->currentIndex() ) );
  if ( !currentItem )
    return;

  if ( currentItem->isLayer() )
  {
    SymbolLayerItem *parent = static_cast<SymbolLayerItem*>( currentItem->parent() );
    mDataDefineRestorer.reset( new DataDefinedRestorer( parent->symbol(), currentItem->layer() ) );
    QgsLayerPropertiesWidget *layerProp = new QgsLayerPropertiesWidget( currentItem->layer(), parent->symbol(), mVectorLayer );
    layerProp->setDockMode( this->dockMode() );
    layerProp->setExpressionContext( mPresetExpressionContext.data() );
    layerProp->setMapCanvas( mMapCanvas );
    setWidget( layerProp );
    connect( layerProp, SIGNAL( changed() ), mDataDefineRestorer.data(), SLOT( restore() ) );
    connect( layerProp, SIGNAL( changed() ), this, SLOT( updateLayerPreview() ) );
    // This connection when layer type is changed
    connect( layerProp, SIGNAL( changeLayer( QgsSymbolLayerV2* ) ), this, SLOT( changeLayer( QgsSymbolLayerV2* ) ) );

    connectChildPanel( layerProp );
  }
  else
  {
    mDataDefineRestorer.reset();
    // then it must be a symbol
    currentItem->symbol()->setLayer( mVectorLayer );
    // Now populate symbols of that type using the symbols list widget:
    QgsSymbolsListWidget *symbolsList = new QgsSymbolsListWidget( currentItem->symbol(), mStyle, mAdvancedMenu, this, mVectorLayer );
    symbolsList->setExpressionContext( mPresetExpressionContext.data() );
    symbolsList->setMapCanvas( mMapCanvas );

    setWidget( symbolsList );
    connect( symbolsList, SIGNAL( changed() ), this, SLOT( symbolChanged() ) );
  }
  updateLockButton();
}

void QgsSymbolV2SelectorWidget::symbolChanged()
{
  SymbolLayerItem *currentItem = static_cast<SymbolLayerItem*>( model->itemFromIndex( layersTree->currentIndex() ) );
  if ( !currentItem || currentItem->isLayer() )
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

void QgsSymbolV2SelectorWidget::setWidget( QWidget* widget )
{
  int index = stackedWidget->addWidget( widget );
  stackedWidget->setCurrentIndex( index );
  if ( mPresentWidget )
    mPresentWidget->deleteLater();
  mPresentWidget = widget;
}

void QgsSymbolV2SelectorWidget::updateLockButton()
{
  QgsSymbolLayerV2* layer = currentLayer();
  if ( !layer )
    return;
  btnLock->setChecked( layer->isLocked() );
}

void QgsSymbolV2SelectorWidget::addLayer()
{
  QModelIndex idx = layersTree->currentIndex();
  if ( !idx.isValid() )
    return;

  int insertIdx = -1;
  SymbolLayerItem *item = static_cast<SymbolLayerItem*>( model->itemFromIndex( idx ) );
  if ( item->isLayer() )
  {
    insertIdx = item->row();
    item = static_cast<SymbolLayerItem*>( item->parent() );
  }

  QgsSymbolV2* parentSymbol = item->symbol();

  // save data-defined values at marker level
  QgsDataDefined ddSize = parentSymbol->type() == QgsSymbolV2::Marker
                          ? static_cast<QgsMarkerSymbolV2 *>( parentSymbol )->dataDefinedSize()
                          : QgsDataDefined();
  QgsDataDefined ddAngle = parentSymbol->type() == QgsSymbolV2::Marker
                           ? static_cast<QgsMarkerSymbolV2 *>( parentSymbol )->dataDefinedAngle()
                           : QgsDataDefined();
  QgsDataDefined ddWidth = parentSymbol->type() == QgsSymbolV2::Line
                           ? static_cast<QgsLineSymbolV2 *>( parentSymbol )->dataDefinedWidth()
                           : QgsDataDefined() ;

  QgsSymbolLayerV2* newLayer = QgsSymbolLayerV2Registry::instance()->defaultSymbolLayer( parentSymbol->type() );
  if ( insertIdx == -1 )
    parentSymbol->appendSymbolLayer( newLayer );
  else
    parentSymbol->insertSymbolLayer( item->rowCount() - insertIdx, newLayer );

  // restore data-defined values at marker level
  if ( ddSize != QgsDataDefined() )
    static_cast<QgsMarkerSymbolV2 *>( parentSymbol )->setDataDefinedSize( ddSize );
  if ( ddAngle != QgsDataDefined() )
    static_cast<QgsMarkerSymbolV2 *>( parentSymbol )->setDataDefinedAngle( ddAngle );
  if ( ddWidth != QgsDataDefined() )
    static_cast<QgsLineSymbolV2 *>( parentSymbol )->setDataDefinedWidth( ddWidth );

  SymbolLayerItem *newLayerItem = new SymbolLayerItem( newLayer );
  item->insertRow( insertIdx == -1 ? 0 : insertIdx, newLayerItem );
  item->updatePreview();

  layersTree->setCurrentIndex( model->indexFromItem( newLayerItem ) );
  updateUi();
  updatePreview();
}

void QgsSymbolV2SelectorWidget::removeLayer()
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

void QgsSymbolV2SelectorWidget::moveLayerDown()
{
  moveLayerByOffset( + 1 );
}

void QgsSymbolV2SelectorWidget::moveLayerUp()
{
  moveLayerByOffset( -1 );
}

void QgsSymbolV2SelectorWidget::moveLayerByOffset( int offset )
{
  SymbolLayerItem *item = currentLayerItem();
  if ( !item )
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

void QgsSymbolV2SelectorWidget::lockLayer()
{
  QgsSymbolLayerV2* layer = currentLayer();
  if ( !layer )
    return;
  layer->setLocked( btnLock->isChecked() );
  emit symbolModified();
}

void QgsSymbolV2SelectorWidget::duplicateLayer()
{
  QModelIndex idx = layersTree->currentIndex();
  if ( !idx.isValid() )
    return;

  SymbolLayerItem *item = static_cast<SymbolLayerItem*>( model->itemFromIndex( idx ) );
  if ( !item->isLayer() )
    return;

  QgsSymbolLayerV2* source = item->layer();

  int insertIdx = item->row();
  item = static_cast<SymbolLayerItem*>( item->parent() );

  QgsSymbolV2* parentSymbol = item->symbol();

  QgsSymbolLayerV2* newLayer = source->clone();
  if ( insertIdx == -1 )
    parentSymbol->appendSymbolLayer( newLayer );
  else
    parentSymbol->insertSymbolLayer( item->rowCount() - insertIdx, newLayer );

  SymbolLayerItem *newLayerItem = new SymbolLayerItem( newLayer );
  item->insertRow( insertIdx == -1 ? 0 : insertIdx, newLayerItem );
  if ( newLayer->subSymbol() )
  {
    loadSymbol( newLayer->subSymbol(), newLayerItem );
    layersTree->setExpanded( newLayerItem->index(), true );
  }
  item->updatePreview();

  layersTree->setCurrentIndex( model->indexFromItem( newLayerItem ) );
  updateUi();
  updatePreview();
}

void QgsSymbolV2SelectorWidget::saveSymbol()
{
  bool ok;
  QString name = QInputDialog::getText( this, tr( "Symbol name" ),
                                        tr( "Please enter name for the symbol:" ), QLineEdit::Normal, tr( "New symbol" ), &ok );
  if ( !ok || name.isEmpty() )
    return;

  // check if there is no symbol with same name
  if ( mStyle->symbolNames().contains( name ) )
  {
    int res = QMessageBox::warning( this, tr( "Save symbol" ),
                                    tr( "Symbol with name '%1' already exists. Overwrite?" )
                                    .arg( name ),
                                    QMessageBox::Yes | QMessageBox::No );
    if ( res != QMessageBox::Yes )
    {
      return;
    }
  }

  // add new symbol to style and re-populate the list
  mStyle->addSymbol( name, mSymbol->clone() );

  // make sure the symbol is stored
  mStyle->saveSymbol( name, mSymbol->clone(), 0, QStringList() );
}

void QgsSymbolV2SelectorWidget::changeLayer( QgsSymbolLayerV2* newLayer )
{
  SymbolLayerItem* item = currentLayerItem();
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
  // Important: This lets the layer have its own layer properties widget
  layerChanged();
}

QgsSymbolV2SelectorDialog::QgsSymbolV2SelectorDialog( QgsSymbolV2 *symbol, QgsStyleV2 *style, const QgsVectorLayer *vl, QWidget *parent, bool embedded )
    : QDialog( parent )
{
  setLayout( new QVBoxLayout() );
  mSelectorWidget = new QgsSymbolV2SelectorWidget( symbol, style, vl, this );
  mButtonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );

  connect( mButtonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
  connect( mButtonBox, SIGNAL( rejected() ), this, SLOT( reject() ) );

  layout()->addWidget( mSelectorWidget );
  layout()->addWidget( mButtonBox );

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/SymbolSelectorWidget/geometry" ).toByteArray() );

  // can be embedded in renderer properties dialog
  if ( embedded )
  {
    mButtonBox->hide();
    layout()->setContentsMargins( 0, 0, 0, 0 );
  }
  mSelectorWidget->setDockMode( embedded );
}

QgsSymbolV2SelectorDialog::~QgsSymbolV2SelectorDialog()
{
  QSettings settings;
  settings.setValue( "/Windows/SymbolSelectorWidget/geometry", saveGeometry() );
}

QMenu *QgsSymbolV2SelectorDialog::advancedMenu()
{
  return mSelectorWidget->advancedMenu();
}

void QgsSymbolV2SelectorDialog::setExpressionContext( QgsExpressionContext *context )
{
  mSelectorWidget->setExpressionContext( context );
}

QgsExpressionContext *QgsSymbolV2SelectorDialog::expressionContext() const
{
  return mSelectorWidget->expressionContext();
}

void QgsSymbolV2SelectorDialog::setMapCanvas( QgsMapCanvas *canvas )
{
  mSelectorWidget->setMapCanvas( canvas );
}

QgsSymbolV2 *QgsSymbolV2SelectorDialog::symbol()
{
  return mSelectorWidget->symbol();
}

void QgsSymbolV2SelectorDialog::keyPressEvent( QKeyEvent *e )
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

void QgsSymbolV2SelectorDialog::loadSymbol()
{
  mSelectorWidget->loadSymbol();
}

void QgsSymbolV2SelectorDialog::loadSymbol( QgsSymbolV2 *symbol, SymbolLayerItem *parent )
{
  mSelectorWidget->loadSymbol( symbol, parent );
}

void QgsSymbolV2SelectorDialog::updateUi()
{
  mSelectorWidget->updateUi();
}

void QgsSymbolV2SelectorDialog::updateLockButton()
{
  mSelectorWidget->updateLockButton();
}

SymbolLayerItem *QgsSymbolV2SelectorDialog::currentLayerItem()
{
  return mSelectorWidget->currentLayerItem();
}

QgsSymbolLayerV2 *QgsSymbolV2SelectorDialog::currentLayer()
{
  return mSelectorWidget->currentLayer();
}

void QgsSymbolV2SelectorDialog::moveLayerByOffset( int offset )
{
  mSelectorWidget->moveLayerByOffset( offset );
}

void QgsSymbolV2SelectorDialog::setWidget( QWidget *widget )
{
  mSelectorWidget->setWidget( widget );
}

void QgsSymbolV2SelectorDialog::moveLayerDown()
{
  mSelectorWidget->moveLayerDown();
}

void QgsSymbolV2SelectorDialog::moveLayerUp()
{
  mSelectorWidget->moveLayerUp();
}

void QgsSymbolV2SelectorDialog::addLayer()
{
  mSelectorWidget->addLayer();
}

void QgsSymbolV2SelectorDialog::removeLayer()
{
  mSelectorWidget->removeLayer();
}

void QgsSymbolV2SelectorDialog::lockLayer()
{
  mSelectorWidget->lockLayer();
}

void QgsSymbolV2SelectorDialog::saveSymbol()
{
  Q_NOWARN_DEPRECATED_PUSH
  mSelectorWidget->saveSymbol();
  Q_NOWARN_DEPRECATED_POP
}

void QgsSymbolV2SelectorDialog::duplicateLayer()
{
  mSelectorWidget->duplicateLayer();
}

void QgsSymbolV2SelectorDialog::layerChanged()
{
  mSelectorWidget->layerChanged();
}

void QgsSymbolV2SelectorDialog::updateLayerPreview()
{
  mSelectorWidget->updateLayerPreview();
}

void QgsSymbolV2SelectorDialog::updatePreview()
{
  mSelectorWidget->updatePreview();
}

void QgsSymbolV2SelectorDialog::symbolChanged()
{
  mSelectorWidget->symbolChanged();
}

void QgsSymbolV2SelectorDialog::changeLayer( QgsSymbolLayerV2 *layer )
{
  mSelectorWidget->changeLayer( layer );
}
