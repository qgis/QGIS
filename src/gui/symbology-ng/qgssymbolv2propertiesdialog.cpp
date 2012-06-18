/***************************************************************************
    qgssymbolv2propertiesdialog.cpp
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

#include "qgssymbolv2propertiesdialog.h"

#include <QFile>
#include <QStandardItem>
#include <QKeyEvent>
#include <QMessageBox>

#include "qgssymbollayerv2.h"
#include "qgssymbollayerv2registry.h"

#include "qgsapplication.h"
#include "qgslogger.h"

#include "qgssymbollayerv2widget.h"
#include "qgsellipsesymbollayerv2widget.h"
#include "qgsvectorfieldsymbollayerwidget.h"
#include "qgssymbolv2.h" //for the unit


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
      if ( role == Qt::DisplayRole )
      {
        if ( mIsLayer )
          return QgsSymbolLayerV2Registry::instance()->symbolLayerMetadata( mLayer->layerType() )->visibleName();
        else
        {
          switch( mSymbol->type() )
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


static bool _initWidgetFunction( QString name, QgsSymbolLayerV2WidgetFunc f )
{
  QgsSymbolLayerV2Registry* reg = QgsSymbolLayerV2Registry::instance();

  QgsSymbolLayerV2AbstractMetadata* abstractMetadata = reg->symbolLayerMetadata( name );
  if ( abstractMetadata == NULL )
  {
    QgsDebugMsg( "Failed to find symbol layer's entry in registry: " + name );
    return false;
  }
  QgsSymbolLayerV2Metadata* metadata = dynamic_cast<QgsSymbolLayerV2Metadata*>( abstractMetadata );
  if ( metadata == NULL )
  {
    QgsDebugMsg( "Failed to cast symbol layer's metadata: " + name );
    return false;
  }
  metadata->setWidgetFunction( f );
  return true;
}

static void _initWidgetFunctions()
{
  static bool initialized = false;
  if ( initialized )
    return;

  _initWidgetFunction( "SimpleLine", QgsSimpleLineSymbolLayerV2Widget::create );
  _initWidgetFunction( "MarkerLine", QgsMarkerLineSymbolLayerV2Widget::create );
  _initWidgetFunction( "LineDecoration", QgsLineDecorationSymbolLayerV2Widget::create );

  _initWidgetFunction( "SimpleMarker", QgsSimpleMarkerSymbolLayerV2Widget::create );
  _initWidgetFunction( "SvgMarker", QgsSvgMarkerSymbolLayerV2Widget::create );
  _initWidgetFunction( "FontMarker", QgsFontMarkerSymbolLayerV2Widget::create );
  _initWidgetFunction( "EllipseMarker", QgsEllipseSymbolLayerV2Widget::create );
  _initWidgetFunction( "VectorField", QgsVectorFieldSymbolLayerWidget::create );

  _initWidgetFunction( "SimpleFill", QgsSimpleFillSymbolLayerV2Widget::create );
  _initWidgetFunction( "SVGFill", QgsSVGFillSymbolLayerWidget::create );
  _initWidgetFunction( "CentroidFill", QgsCentroidFillSymbolLayerV2Widget::create );
  _initWidgetFunction( "LinePatternFill", QgsLinePatternFillSymbolLayerWidget::create );
  _initWidgetFunction( "PointPatternFill", QgsPointPatternFillSymbolLayerWidget::create );

  initialized = true;
}


//////////

QgsSymbolV2PropertiesDialog::QgsSymbolV2PropertiesDialog( QgsSymbolV2* symbol, const QgsVectorLayer* vl, QWidget* parent )
    : QDialog( parent ), mSymbol( symbol ), mVectorLayer( vl )
{
  setupUi( this );

  // setup icons
  btnAddLayer->setIcon( QIcon( QgsApplication::iconPath( "symbologyAdd.png" ) ) );
  btnRemoveLayer->setIcon( QIcon( QgsApplication::iconPath( "symbologyRemove.png" ) ) );
  QIcon iconLock;
  iconLock.addFile( QgsApplication::iconPath( "locked.png" ), QSize(), QIcon::Normal, QIcon::On );
  iconLock.addFile( QgsApplication::iconPath( "unlocked.png" ), QSize(), QIcon::Normal, QIcon::Off );
  btnLock->setIcon( iconLock );
  btnUp->setIcon( QIcon( QgsApplication::iconPath( "symbologyUp.png" ) ) );
  btnDown->setIcon( QIcon( QgsApplication::iconPath( "symbologyDown.png" ) ) );

  // set widget functions
  // (should be probably moved somewhere else)
  _initWidgetFunctions();

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


void QgsSymbolV2PropertiesDialog::loadSymbol( QgsSymbolV2* symbol, SymbolLayerItem* parent )
{
  SymbolLayerItem* symbolItem = new SymbolLayerItem( symbol );
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
  layersTree->setExpanded( symbolItem->index(), true);
}


void QgsSymbolV2PropertiesDialog::loadSymbol()
{
  model->clear();
  loadSymbol( mSymbol, static_cast<SymbolLayerItem*>( model->invisibleRootItem() ) );
}


void QgsSymbolV2PropertiesDialog::populateLayerTypes( QgsSymbolV2 *symbol )
{
  QStringList types = QgsSymbolLayerV2Registry::instance()->symbolLayersForType( symbol->type() );

  cboLayerType->clear();
  for ( int i = 0; i < types.count(); i++ )
    cboLayerType->addItem( QgsSymbolLayerV2Registry::instance()->symbolLayerMetadata( types[i] )->visibleName(), types[i] );

  if ( mSymbol->type() == QgsSymbolV2::Fill )
  {
    QStringList typesLine = QgsSymbolLayerV2Registry::instance()->symbolLayersForType( QgsSymbolV2::Line );
    for ( int i = 0; i < typesLine.count(); i++ )
    {
      QString visibleName = QgsSymbolLayerV2Registry::instance()->symbolLayerMetadata( typesLine[i] )->visibleName();
      QString name = QString( tr( "Outline: %1" ) ).arg( visibleName );
      cboLayerType->addItem( name, typesLine[i] );
    }
  }
}


void QgsSymbolV2PropertiesDialog::updateUi()
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
    return;
  }

  int rowCount = item->parent()->rowCount();
  int currentRow = item->row();

  btnUp->setEnabled( currentRow > 0 );
  btnDown->setEnabled( currentRow < rowCount - 1 );
  btnRemoveLayer->setEnabled( rowCount > 1 );
  btnLock->setEnabled( true );
}

void QgsSymbolV2PropertiesDialog::updatePreview()
{
  QImage preview = mSymbol->bigSymbolPreviewImage();
  lblPreview->setPixmap( QPixmap::fromImage( preview ) );
}

void QgsSymbolV2PropertiesDialog::updateLayerPreview()
{
  // get current layer item and update its icon
  SymbolLayerItem* item = currentLayerItem();
  if ( item )
    item->updatePreview();
  // update also preview of the whole symbol
  updatePreview();
}

void QgsSymbolV2PropertiesDialog::updateSymbolLayerWidget( QgsSymbolLayerV2* layer )
{
  if ( stackedWidget->currentWidget() != pageDummy )
  {
    // stop updating from the original widget
    disconnect( stackedWidget->currentWidget(), SIGNAL( changed() ), this, SLOT( updateLayerPreview() ) );
    stackedWidget->removeWidget( stackedWidget->currentWidget() );
  }

  QgsSymbolLayerV2Registry* pReg = QgsSymbolLayerV2Registry::instance();

  QString layerType = layer->layerType();

  QgsSymbolLayerV2AbstractMetadata* am = pReg->symbolLayerMetadata( layerType );
  if ( am )
  {
    QgsSymbolLayerV2Widget* w = am->createSymbolLayerWidget( mVectorLayer );
    if ( w )
    {
      w->setSymbolLayer( layer );
      stackedWidget->addWidget( w );
      stackedWidget->setCurrentWidget( w );
      // start recieving updates from widget
      connect( w , SIGNAL( changed() ), this, SLOT( updateLayerPreview() ) );
      return;
    }
  }

  // When anything is not right
  stackedWidget->setCurrentWidget( pageDummy );
}

SymbolLayerItem* QgsSymbolV2PropertiesDialog::currentLayerItem()
{
  QModelIndex idx = layersTree->currentIndex();
  if ( !idx.isValid() )
    return NULL;

  SymbolLayerItem *item = static_cast<SymbolLayerItem*>( model->itemFromIndex( idx ) );
  if ( !item->isLayer() )
    return NULL;

  return item;
}

QgsSymbolLayerV2* QgsSymbolV2PropertiesDialog::currentLayer()
{
  QModelIndex idx = layersTree->currentIndex();
  if ( !idx.isValid() )
    return NULL;

  SymbolLayerItem *item = static_cast<SymbolLayerItem*>( model->itemFromIndex( idx ) );
  if ( item->isLayer() )
    return item->layer();

  return NULL;
}


void QgsSymbolV2PropertiesDialog::layerChanged()
{
  // We donot want slot to fire while we load compatible layertypes
  disconnect( cboLayerType, SIGNAL( currentIndexChanged( int ) ), this, SLOT( layerTypeChanged() ) );
  updateUi();

  SymbolLayerItem *currentItem = static_cast<SymbolLayerItem*>( model->itemFromIndex( layersTree->currentIndex() ) );
  if ( currentItem == NULL )
    return;

  // FIXME get the current symbol and populate the saved symbols of that type in the widget area
  if ( !currentItem->isLayer() )
  {
    cboLayerType->clear();
    stackedWidget->removeWidget( stackedWidget->currentWidget() );
    return;
  }

  SymbolLayerItem *symbolItem = static_cast<SymbolLayerItem*>( currentItem->parent() );
  populateLayerTypes( symbolItem->symbol() );

  QgsSymbolLayerV2 *layer = currentItem->layer();
  // update layer type combo box
  int idx = cboLayerType->findData( layer->layerType() );
  cboLayerType->setCurrentIndex( idx );

  updateSymbolLayerWidget( layer );
  connect( cboLayerType, SIGNAL( currentIndexChanged( int ) ), this, SLOT( layerTypeChanged() ) );

  updateLockButton();
}


void QgsSymbolV2PropertiesDialog::updateLockButton()
{
  QgsSymbolLayerV2* layer = currentLayer();
  if ( !layer )
    return;
  btnLock->setChecked( layer->isLocked() );
}


void QgsSymbolV2PropertiesDialog::layerTypeChanged()
{
  QgsSymbolLayerV2* layer = currentLayer();
  if ( !layer )
    return;
  QString newLayerType = cboLayerType->itemData( cboLayerType->currentIndex() ).toString();
  if ( layer->layerType() == newLayerType )
    return;

  // get creation function for new layer from registry
  QgsSymbolLayerV2Registry* pReg = QgsSymbolLayerV2Registry::instance();
  QgsSymbolLayerV2AbstractMetadata* am = pReg->symbolLayerMetadata( newLayerType );
  if ( am == NULL ) // check whether the metadata is assigned
    return;

  // change layer to a new (with different type)
  QgsSymbolLayerV2* newLayer = am->createSymbolLayer( QgsStringMap() );
  if ( newLayer == NULL )
    return;

  SymbolLayerItem *item = currentLayerItem();
  // remove previos childs if any
  if ( layer->subSymbol() )
  {
    item->removeRow( 0 );
  }
  // update symbol layer item
  item->setLayer( newLayer );
  // When it is a marker symbol
  if ( newLayer->subSymbol() )
  {
    SymbolLayerItem *subsymbol = new SymbolLayerItem( newLayer->subSymbol() );
    SymbolLayerItem *sublayer = new SymbolLayerItem( newLayer->subSymbol()->symbolLayer( 0 ) );
    subsymbol->appendRow( sublayer );
    item->appendRow( subsymbol );
  }

  // Change the symbol at last to avoid deleting item's layer
  QgsSymbolV2* symbol = static_cast<SymbolLayerItem*>( item->parent() )->symbol();
  int layerIdx = item->parent()->rowCount() - item->row() - 1;
  symbol->changeSymbolLayer( layerIdx, newLayer );

  updateSymbolLayerWidget( newLayer );

  item->updatePreview();
  updatePreview();
}


void QgsSymbolV2PropertiesDialog::addLayer()
{
  QModelIndex idx = layersTree->currentIndex();
  if ( !idx.isValid() )
    return;

  SymbolLayerItem *item = static_cast<SymbolLayerItem*>( model->itemFromIndex( idx ) );
  if ( item->isLayer() )
  {
    QMessageBox::critical( this, tr( "Invalid Selection!" ), tr( "Kindly select a symbol to add layer.") );
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


void QgsSymbolV2PropertiesDialog::removeLayer()
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

void QgsSymbolV2PropertiesDialog::moveLayerDown()
{
  moveLayerByOffset( + 1 );
}

void QgsSymbolV2PropertiesDialog::moveLayerUp()
{
  moveLayerByOffset( -1 );
}

void QgsSymbolV2PropertiesDialog::moveLayerByOffset( int offset )
{
  SymbolLayerItem *item = currentLayerItem();
  if( item == NULL )
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

void QgsSymbolV2PropertiesDialog::lockLayer()
{
  QgsSymbolLayerV2* layer = currentLayer();
  if ( !layer )
    return;
  layer->setLocked( btnLock->isChecked() );
}

void QgsSymbolV2PropertiesDialog::keyPressEvent( QKeyEvent * e )
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
