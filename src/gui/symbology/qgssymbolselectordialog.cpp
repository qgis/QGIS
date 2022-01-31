/***************************************************************************
    qgssymbolselectordialog.cpp
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

#include "qgssymbolselectordialog.h"

#include "qgsstyle.h"
#include "qgssymbol.h"
#include "qgssymbollayer.h"
#include "qgssymbollayerutils.h"
#include "qgssymbollayerregistry.h"
#include "qgsexpressioncontextutils.h"

// the widgets
#include "qgssymbolslistwidget.h"
#include "qgslayerpropertieswidget.h"
#include "qgssymbollayerwidget.h"
#include "qgsellipsesymbollayerwidget.h"
#include "qgsvectorfieldsymbollayerwidget.h"

#include "qgslogger.h"
#include "qgsapplication.h"
#include "qgssettings.h"
#include "qgsfeatureiterator.h"
#include "qgsvectorlayer.h"
#include "qgssvgcache.h"
#include "qgsimagecache.h"
#include "qgsproject.h"
#include "qgsguiutils.h"
#include "qgsgui.h"
#include "qgsmarkersymbol.h"
#include "qgsfillsymbol.h"
#include "qgslinesymbol.h"

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

static const int SYMBOL_LAYER_ITEM_TYPE = QStandardItem::UserType + 1;

DataDefinedRestorer::DataDefinedRestorer( QgsSymbol *symbol, const QgsSymbolLayer *symbolLayer )

{
  if ( symbolLayer->type() == Qgis::SymbolType::Marker && symbol->type() == Qgis::SymbolType::Marker )
  {
    Q_ASSERT( symbol->type() == Qgis::SymbolType::Marker );
    mMarker = static_cast<QgsMarkerSymbol *>( symbol );
    mMarkerSymbolLayer = static_cast<const QgsMarkerSymbolLayer *>( symbolLayer );
    mDDSize = mMarker->dataDefinedSize();
    mDDAngle = mMarker->dataDefinedAngle();
    // check if restore is actually needed
    if ( !mDDSize && !mDDAngle )
      mMarker = nullptr;
  }
  else if ( symbolLayer->type() == Qgis::SymbolType::Line && symbol->type() == Qgis::SymbolType::Line )
  {
    mLine = static_cast<QgsLineSymbol *>( symbol );
    mLineSymbolLayer = static_cast<const QgsLineSymbolLayer *>( symbolLayer );
    mDDWidth = mLine->dataDefinedWidth();
    // check if restore is actually needed
    if ( !mDDWidth )
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
    if ( mDDSize &&
         ( mSize != mMarkerSymbolLayer->size() || mMarkerOffset != mMarkerSymbolLayer->offset() ) )
      mMarker->setDataDefinedSize( mDDSize );
    if ( mDDAngle &&
         mAngle != mMarkerSymbolLayer->angle() )
      mMarker->setDataDefinedAngle( mDDAngle );
  }
  else if ( mLine )
  {
    if ( mDDWidth &&
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
    explicit SymbolLayerItem( QgsSymbolLayer *layer, Qgis::SymbolType symbolType )
    {
      setLayer( layer, symbolType );
    }

    explicit SymbolLayerItem( QgsSymbol *symbol )
    {
      setSymbol( symbol );
    }

    void setLayer( QgsSymbolLayer *layer, Qgis::SymbolType symbolType )
    {
      mLayer = layer;
      mIsLayer = true;
      mSymbol = nullptr;
      mSymbolType = symbolType;
      updatePreview();
    }

    void setSymbol( QgsSymbol *symbol )
    {
      mSymbol = symbol;
      mIsLayer = false;
      mLayer = nullptr;
      updatePreview();
    }

    void updatePreview()
    {
      if ( !mSize.isValid() )
      {
        const int size = QgsGuiUtils::scaleIconSize( 16 );
        mSize = QSize( size, size );
      }
      QIcon icon;
      if ( mIsLayer )
        icon = QgsSymbolLayerUtils::symbolLayerPreviewIcon( mLayer, QgsUnitTypes::RenderMillimeters, mSize, QgsMapUnitScale(), mSymbol ? mSymbol->type() : mSymbolType ); //todo: make unit a parameter
      else
        icon = QgsSymbolLayerUtils::symbolPreviewIcon( mSymbol, mSize );
      setIcon( icon );

      if ( auto *lParent = parent() )
        static_cast<SymbolLayerItem *>( lParent )->updatePreview();
    }

    int type() const override { return SYMBOL_LAYER_ITEM_TYPE; }
    bool isLayer() { return mIsLayer; }

    // returns the symbol pointer; helpful in determining a layer's parent symbol
    QgsSymbol *symbol()
    {
      return mSymbol;
    }

    QgsSymbolLayer *layer()
    {
      return mLayer;
    }

    QVariant data( int role ) const override
    {
      if ( role == Qt::DisplayRole || role == Qt::EditRole )
      {
        if ( mIsLayer )
        {
          QgsSymbolLayerAbstractMetadata *m = QgsApplication::symbolLayerRegistry()->symbolLayerMetadata( mLayer->layerType() );
          if ( m )
            return m->visibleName();
          else
            return QString();
        }
        else
        {
          switch ( mSymbol->type() )
          {
            case Qgis::SymbolType::Marker :
              return QCoreApplication::translate( "SymbolLayerItem", "Marker" );
            case Qgis::SymbolType::Fill   :
              return QCoreApplication::translate( "SymbolLayerItem", "Fill" );
            case Qgis::SymbolType::Line   :
              return QCoreApplication::translate( "SymbolLayerItem", "Line" );
            default:
              return "Symbol";
          }
        }
      }
      else if ( role == Qt::ForegroundRole && mIsLayer )
      {
        if ( !mLayer->enabled() )
        {
          QPalette pal = qApp->palette();
          QBrush brush = QStandardItem::data( role ).value< QBrush >();
          brush.setColor( pal.color( QPalette::Disabled, QPalette::WindowText ) );
          return brush;
        }
        else
        {
          return QVariant();
        }
      }

//      if ( role == Qt::SizeHintRole )
//        return QVariant( QSize( 32, 32 ) );
      if ( role == Qt::CheckStateRole )
        return QVariant(); // could be true/false
      return QStandardItem::data( role );
    }

  protected:
    QgsSymbolLayer *mLayer = nullptr;
    QgsSymbol *mSymbol = nullptr;
    bool mIsLayer;
    QSize mSize;
    Qgis::SymbolType mSymbolType = Qgis::SymbolType::Hybrid;
};

///@endcond

//////////

QgsSymbolSelectorWidget::QgsSymbolSelectorWidget( QgsSymbol *symbol, QgsStyle *style, QgsVectorLayer *vl, QWidget *parent )
  : QgsPanelWidget( parent )
  , mStyle( style )
  , mSymbol( symbol )
  , mVectorLayer( vl )
{
#ifdef Q_OS_MAC
  setWindowModality( Qt::WindowModal );
#endif

  setupUi( this );
  this->layout()->setContentsMargins( 0, 0, 0, 0 );

  layersTree->setMaximumHeight( static_cast< int >( Qgis::UI_SCALE_FACTOR * fontMetrics().height() * 7 ) );
  layersTree->setMinimumHeight( layersTree->maximumHeight() );
  lblPreview->setMaximumWidth( layersTree->maximumHeight() );

  // setup icons
  btnAddLayer->setIcon( QIcon( QgsApplication::iconPath( "symbologyAdd.svg" ) ) );
  btnRemoveLayer->setIcon( QIcon( QgsApplication::iconPath( "symbologyRemove.svg" ) ) );
  QIcon iconLock;
  iconLock.addFile( QgsApplication::iconPath( QStringLiteral( "locked.svg" ) ), QSize(), QIcon::Normal, QIcon::On );
  iconLock.addFile( QgsApplication::iconPath( QStringLiteral( "locked.svg" ) ), QSize(), QIcon::Active, QIcon::On );
  iconLock.addFile( QgsApplication::iconPath( QStringLiteral( "unlocked.svg" ) ), QSize(), QIcon::Normal, QIcon::Off );
  iconLock.addFile( QgsApplication::iconPath( QStringLiteral( "unlocked.svg" ) ), QSize(), QIcon::Active, QIcon::Off );
  btnLock->setIcon( iconLock );
  btnDuplicate->setIcon( QIcon( QgsApplication::iconPath( "mActionDuplicateLayer.svg" ) ) );
  btnUp->setIcon( QIcon( QgsApplication::iconPath( "mActionArrowUp.svg" ) ) );
  btnDown->setIcon( QIcon( QgsApplication::iconPath( "mActionArrowDown.svg" ) ) );

  mSymbolLayersModel = new QStandardItemModel( layersTree );
  // Set the symbol
  layersTree->setModel( mSymbolLayersModel );
  layersTree->setHeaderHidden( true );

  //get first feature from layer for previews
  if ( mVectorLayer )
  {
#if 0 // this is too expensive to do for many providers. TODO revisit when support for connection timeouts is complete across all providers
    // short timeout for request - it doesn't really matter if we don't get the feature, and this call is blocking UI
    QgsFeatureIterator it = mVectorLayer->getFeatures( QgsFeatureRequest().setLimit( 1 ).setConnectionTimeout( 100 ) );
    it.nextFeature( mPreviewFeature );
#endif
    mPreviewExpressionContext.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( mVectorLayer ) );
#if 0
    mPreviewExpressionContext.setFeature( mPreviewFeature );
#endif
  }
  else
  {
    mPreviewExpressionContext.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( nullptr ) );
  }

  QItemSelectionModel *selModel = layersTree->selectionModel();
  connect( selModel, &QItemSelectionModel::currentChanged, this, &QgsSymbolSelectorWidget::layerChanged );

  loadSymbol( mSymbol, static_cast<SymbolLayerItem *>( mSymbolLayersModel->invisibleRootItem() ) );
  updatePreview();

  connect( btnUp, &QAbstractButton::clicked, this, &QgsSymbolSelectorWidget::moveLayerUp );
  connect( btnDown, &QAbstractButton::clicked, this, &QgsSymbolSelectorWidget::moveLayerDown );
  connect( btnAddLayer, &QAbstractButton::clicked, this, &QgsSymbolSelectorWidget::addLayer );
  connect( btnRemoveLayer, &QAbstractButton::clicked, this, &QgsSymbolSelectorWidget::removeLayer );
  connect( btnLock, &QAbstractButton::clicked, this, &QgsSymbolSelectorWidget::lockLayer );
  connect( btnDuplicate, &QAbstractButton::clicked, this, &QgsSymbolSelectorWidget::duplicateLayer );
  connect( this, &QgsSymbolSelectorWidget::symbolModified, this, &QgsPanelWidget::widgetChanged );

  updateUi();

  // set symbol as active item in the tree
  const QModelIndex newIndex = layersTree->model()->index( 0, 0 );
  layersTree->setCurrentIndex( newIndex );

  setPanelTitle( tr( "Symbol Selector" ) );

  // when a remote svg has been fetched, update the widget's previews
  // this is required if the symbol utilizes remote svgs, and the current previews
  // have been generated using the temporary "downloading" svg. In this case
  // we require the preview to be regenerated to use the correct fetched
  // svg
  connect( QgsApplication::svgCache(), &QgsSvgCache::remoteSvgFetched, this, &QgsSymbolSelectorWidget::projectDataChanged );

  // when a remote image has been fetched, update the widget's previews
  // this is required if the symbol utilizes remote images, and the current previews
  // have been generated using the temporary "downloading" image. In this case
  // we require the preview to be regenerated to use the correct fetched
  // image
  connect( QgsApplication::imageCache(), &QgsImageCache::remoteImageFetched, this, &QgsSymbolSelectorWidget::projectDataChanged );

  // if project color scheme changes, we need to redraw symbols - they may use project colors and accordingly
  // need updating to reflect the new colors
  connect( QgsProject::instance(), &QgsProject::projectColorsChanged, this, &QgsSymbolSelectorWidget::projectDataChanged );

  connect( QgsProject::instance(), static_cast < void ( QgsProject::* )( const QList<QgsMapLayer *>& layers ) > ( &QgsProject::layersWillBeRemoved ), this, &QgsSymbolSelectorWidget::layersAboutToBeRemoved );
}

QMenu *QgsSymbolSelectorWidget::advancedMenu()
{
  if ( !mAdvancedMenu )
  {
    mAdvancedMenu = new QMenu( this );
    // Brute force method to activate the Advanced menu
    layerChanged();
  }
  return mAdvancedMenu;
}

void QgsSymbolSelectorWidget::setContext( const QgsSymbolWidgetContext &context )
{
  mContext = context;

  if ( auto *lExpressionContext = mContext.expressionContext() )
  {
    mPreviewExpressionContext = *lExpressionContext;
    if ( mVectorLayer )
      mPreviewExpressionContext.appendScope( QgsExpressionContextUtils::layerScope( mVectorLayer ) );

    mPreviewExpressionContext.setFeature( mPreviewFeature );
  }

  QWidget *widget = stackedWidget->currentWidget();
  if ( QgsLayerPropertiesWidget *layerProp = qobject_cast< QgsLayerPropertiesWidget * >( widget ) )
    layerProp->setContext( context );
  else if ( QgsSymbolsListWidget *listWidget = qobject_cast< QgsSymbolsListWidget * >( widget ) )
    listWidget->setContext( context );

  layerChanged();
  updatePreview();
}

QgsSymbolWidgetContext QgsSymbolSelectorWidget::context() const
{
  return mContext;
}

void QgsSymbolSelectorWidget::loadSymbol( QgsSymbol *symbol, SymbolLayerItem *parent )
{
  if ( !symbol )
    return;

  if ( !parent )
  {
    mSymbol = symbol;
    mSymbolLayersModel->clear();
    parent = static_cast<SymbolLayerItem *>( mSymbolLayersModel->invisibleRootItem() );
  }

  SymbolLayerItem *symbolItem = new SymbolLayerItem( symbol );
  QFont boldFont = symbolItem->font();
  boldFont.setBold( true );
  symbolItem->setFont( boldFont );
  parent->appendRow( symbolItem );

  const int count = symbol->symbolLayerCount();
  for ( int i = count - 1; i >= 0; i-- )
  {
    SymbolLayerItem *layerItem = new SymbolLayerItem( symbol->symbolLayer( i ), symbol->type() );
    layerItem->setEditable( false );
    symbolItem->appendRow( layerItem );
    if ( symbol->symbolLayer( i )->subSymbol() )
    {
      loadSymbol( symbol->symbolLayer( i )->subSymbol(), layerItem );
    }
    layersTree->setExpanded( layerItem->index(), true );
  }
  layersTree->setExpanded( symbolItem->index(), true );

  if ( mSymbol == symbol && !layersTree->currentIndex().isValid() )
  {
    // make sure root item for symbol is selected in tree
    layersTree->setCurrentIndex( symbolItem->index() );
  }
}

void QgsSymbolSelectorWidget::reloadSymbol()
{
  mSymbolLayersModel->clear();
  loadSymbol( mSymbol, static_cast<SymbolLayerItem *>( mSymbolLayersModel->invisibleRootItem() ) );
}

void QgsSymbolSelectorWidget::updateUi()
{
  const QModelIndex currentIdx = layersTree->currentIndex();
  if ( !currentIdx.isValid() )
    return;

  SymbolLayerItem *item = static_cast<SymbolLayerItem *>( mSymbolLayersModel->itemFromIndex( currentIdx ) );
  if ( !item->isLayer() )
  {
    btnUp->setEnabled( false );
    btnDown->setEnabled( false );
    btnRemoveLayer->setEnabled( false );
    btnLock->setEnabled( false );
    btnDuplicate->setEnabled( false );
    return;
  }

  const int rowCount = item->parent()->rowCount();
  const int currentRow = item->row();

  btnUp->setEnabled( currentRow > 0 );
  btnDown->setEnabled( currentRow < rowCount - 1 );
  btnRemoveLayer->setEnabled( rowCount > 1 );
  btnLock->setEnabled( true );
  btnDuplicate->setEnabled( true );
}

void QgsSymbolSelectorWidget::updatePreview()
{
  if ( !mSymbol )
    return;

  std::unique_ptr< QgsSymbol > symbolClone( mSymbol->clone() );
  const QImage preview = symbolClone->bigSymbolPreviewImage( &mPreviewExpressionContext, Qgis::SymbolPreviewFlags() );
  lblPreview->setPixmap( QPixmap::fromImage( preview ) );
  // Hope this is a appropriate place
  if ( !mBlockModified )
    emit symbolModified();
}

void QgsSymbolSelectorWidget::updateLayerPreview()
{
  // get current layer item and update its icon
  SymbolLayerItem *item = currentLayerItem();
  if ( item )
    item->updatePreview();
  // update also preview of the whole symbol
  updatePreview();
}

SymbolLayerItem *QgsSymbolSelectorWidget::currentLayerItem()
{
  const QModelIndex idx = layersTree->currentIndex();
  if ( !idx.isValid() )
    return nullptr;

  SymbolLayerItem *item = static_cast<SymbolLayerItem *>( mSymbolLayersModel->itemFromIndex( idx ) );
  if ( !item->isLayer() )
    return nullptr;

  return item;
}

QgsSymbolLayer *QgsSymbolSelectorWidget::currentLayer()
{
  const QModelIndex idx = layersTree->currentIndex();
  if ( !idx.isValid() )
    return nullptr;

  SymbolLayerItem *item = static_cast<SymbolLayerItem *>( mSymbolLayersModel->itemFromIndex( idx ) );
  if ( item->isLayer() )
    return item->layer();

  return nullptr;
}

void QgsSymbolSelectorWidget::layerChanged()
{
  updateUi();

  SymbolLayerItem *currentItem = static_cast<SymbolLayerItem *>( mSymbolLayersModel->itemFromIndex( layersTree->currentIndex() ) );
  if ( !currentItem )
    return;

  if ( currentItem->isLayer() )
  {
    SymbolLayerItem *parent = static_cast<SymbolLayerItem *>( currentItem->parent() );
    mDataDefineRestorer.reset( new DataDefinedRestorer( parent->symbol(), currentItem->layer() ) );
    QgsLayerPropertiesWidget *layerProp = new QgsLayerPropertiesWidget( currentItem->layer(), parent->symbol(), mVectorLayer );
    layerProp->setDockMode( this->dockMode() );
    layerProp->setContext( mContext );
    setWidget( layerProp );
    connect( layerProp, &QgsLayerPropertiesWidget::changed, mDataDefineRestorer.get(), &DataDefinedRestorer::restore );
    connect( layerProp, &QgsLayerPropertiesWidget::changed, this, &QgsSymbolSelectorWidget::updateLayerPreview );
    // This connection when layer type is changed
    connect( layerProp, &QgsLayerPropertiesWidget::changeLayer, this, &QgsSymbolSelectorWidget::changeLayer );

    connectChildPanel( layerProp );
  }
  else
  {
    // then it must be a symbol
    mDataDefineRestorer.reset();
    Q_NOWARN_DEPRECATED_PUSH
    currentItem->symbol()->setLayer( mVectorLayer );
    Q_NOWARN_DEPRECATED_POP
    // Now populate symbols of that type using the symbols list widget:
    QgsSymbolsListWidget *symbolsList = new QgsSymbolsListWidget( currentItem->symbol(), mStyle, mAdvancedMenu, this, mVectorLayer );
    symbolsList->setContext( mContext );

    setWidget( symbolsList );
    connect( symbolsList, &QgsSymbolsListWidget::changed, this, &QgsSymbolSelectorWidget::symbolChanged );
  }
  updateLockButton();
}

void QgsSymbolSelectorWidget::symbolChanged()
{
  SymbolLayerItem *currentItem = static_cast<SymbolLayerItem *>( mSymbolLayersModel->itemFromIndex( layersTree->currentIndex() ) );
  if ( !currentItem || currentItem->isLayer() )
    return;
  // disconnect to avoid recreating widget
  disconnect( layersTree->selectionModel(), &QItemSelectionModel::currentChanged, this, &QgsSymbolSelectorWidget::layerChanged );
  if ( currentItem->parent() )
  {
    // it is a sub-symbol
    QgsSymbol *symbol = currentItem->symbol();
    SymbolLayerItem *parent = static_cast<SymbolLayerItem *>( currentItem->parent() );
    parent->removeRow( 0 );
    loadSymbol( symbol, parent );
    layersTree->setCurrentIndex( parent->child( 0 )->index() );
    parent->updatePreview();
  }
  else
  {
    //it is the symbol itself
    reloadSymbol();
    const QModelIndex newIndex = layersTree->model()->index( 0, 0 );
    layersTree->setCurrentIndex( newIndex );
  }
  updatePreview();
  // connect it back once things are set
  connect( layersTree->selectionModel(), &QItemSelectionModel::currentChanged, this, &QgsSymbolSelectorWidget::layerChanged );
}

void QgsSymbolSelectorWidget::setWidget( QWidget *widget )
{
  const int index = stackedWidget->addWidget( widget );
  stackedWidget->setCurrentIndex( index );
  if ( mPresentWidget )
    mPresentWidget->deleteLater();
  mPresentWidget = widget;
}

void QgsSymbolSelectorWidget::updateLockButton()
{
  QgsSymbolLayer *layer = currentLayer();
  if ( !layer )
    return;
  btnLock->setChecked( layer->isLocked() );
}

void QgsSymbolSelectorWidget::addLayer()
{
  const QModelIndex idx = layersTree->currentIndex();
  if ( !idx.isValid() )
    return;

  int insertIdx = -1;
  SymbolLayerItem *item = static_cast<SymbolLayerItem *>( mSymbolLayersModel->itemFromIndex( idx ) );
  if ( item->isLayer() )
  {
    insertIdx = item->row();
    item = static_cast<SymbolLayerItem *>( item->parent() );
  }

  QgsSymbol *parentSymbol = item->symbol();

  // save data-defined values at marker level
  const QgsProperty ddSize( parentSymbol->type() == Qgis::SymbolType::Marker
                            ? static_cast<QgsMarkerSymbol *>( parentSymbol )->dataDefinedSize()
                            : QgsProperty() );
  const QgsProperty ddAngle( parentSymbol->type() == Qgis::SymbolType::Marker
                             ? static_cast<QgsMarkerSymbol *>( parentSymbol )->dataDefinedAngle()
                             : QgsProperty() );
  const QgsProperty ddWidth( parentSymbol->type() == Qgis::SymbolType::Line
                             ? static_cast<QgsLineSymbol *>( parentSymbol )->dataDefinedWidth()
                             : QgsProperty() );

  QgsSymbolLayer *newLayer = QgsSymbolLayerRegistry::defaultSymbolLayer( parentSymbol->type() );
  if ( insertIdx == -1 )
    parentSymbol->appendSymbolLayer( newLayer );
  else
    parentSymbol->insertSymbolLayer( item->rowCount() - insertIdx, newLayer );

  // restore data-defined values at marker level
  if ( ddSize )
    static_cast<QgsMarkerSymbol *>( parentSymbol )->setDataDefinedSize( ddSize );
  if ( ddAngle )
    static_cast<QgsMarkerSymbol *>( parentSymbol )->setDataDefinedAngle( ddAngle );
  if ( ddWidth )
    static_cast<QgsLineSymbol *>( parentSymbol )->setDataDefinedWidth( ddWidth );

  SymbolLayerItem *newLayerItem = new SymbolLayerItem( newLayer, parentSymbol->type() );
  item->insertRow( insertIdx == -1 ? 0 : insertIdx, newLayerItem );
  item->updatePreview();

  layersTree->setCurrentIndex( mSymbolLayersModel->indexFromItem( newLayerItem ) );
  updateUi();
  updatePreview();
}

void QgsSymbolSelectorWidget::removeLayer()
{
  SymbolLayerItem *item = currentLayerItem();
  const int row = item->row();
  SymbolLayerItem *parent = static_cast<SymbolLayerItem *>( item->parent() );

  const int layerIdx = parent->rowCount() - row - 1; // IMPORTANT
  QgsSymbol *parentSymbol = parent->symbol();
  QgsSymbolLayer *tmpLayer = parentSymbol->takeSymbolLayer( layerIdx );

  parent->removeRow( row );
  parent->updatePreview();

  const QModelIndex newIdx = parent->child( 0 )->index();
  layersTree->setCurrentIndex( newIdx );

  updateUi();
  updatePreview();
  //finally delete the removed layer pointer
  delete tmpLayer;
}

void QgsSymbolSelectorWidget::moveLayerDown()
{
  moveLayerByOffset( + 1 );
}

void QgsSymbolSelectorWidget::moveLayerUp()
{
  moveLayerByOffset( -1 );
}

void QgsSymbolSelectorWidget::moveLayerByOffset( int offset )
{
  SymbolLayerItem *item = currentLayerItem();
  if ( !item )
    return;
  const int row = item->row();

  SymbolLayerItem *parent = static_cast<SymbolLayerItem *>( item->parent() );
  QgsSymbol *parentSymbol = parent->symbol();

  const int layerIdx = parent->rowCount() - row - 1;
  // switch layers
  QgsSymbolLayer *tmpLayer = parentSymbol->takeSymbolLayer( layerIdx );
  parentSymbol->insertSymbolLayer( layerIdx - offset, tmpLayer );

  QList<QStandardItem *> rowItems = parent->takeRow( row );
  parent->insertRows( row + offset, rowItems );
  parent->updatePreview();

  const QModelIndex newIdx = rowItems[ 0 ]->index();
  layersTree->setCurrentIndex( newIdx );

  updatePreview();
  updateUi();
}

void QgsSymbolSelectorWidget::lockLayer()
{
  QgsSymbolLayer *layer = currentLayer();
  if ( !layer )
    return;
  layer->setLocked( btnLock->isChecked() );
  emit symbolModified();
}

void QgsSymbolSelectorWidget::duplicateLayer()
{
  const QModelIndex idx = layersTree->currentIndex();
  if ( !idx.isValid() )
    return;

  SymbolLayerItem *item = static_cast<SymbolLayerItem *>( mSymbolLayersModel->itemFromIndex( idx ) );
  if ( !item->isLayer() )
    return;

  QgsSymbolLayer *source = item->layer();

  const int insertIdx = item->row();
  item = static_cast<SymbolLayerItem *>( item->parent() );

  QgsSymbol *parentSymbol = item->symbol();

  QgsSymbolLayer *newLayer = source->clone();
  if ( insertIdx == -1 )
    parentSymbol->appendSymbolLayer( newLayer );
  else
    parentSymbol->insertSymbolLayer( item->rowCount() - insertIdx, newLayer );

  SymbolLayerItem *newLayerItem = new SymbolLayerItem( newLayer, parentSymbol->type() );
  item->insertRow( insertIdx == -1 ? 0 : insertIdx, newLayerItem );
  if ( newLayer->subSymbol() )
  {
    loadSymbol( newLayer->subSymbol(), newLayerItem );
    layersTree->setExpanded( newLayerItem->index(), true );
  }
  item->updatePreview();

  layersTree->setCurrentIndex( mSymbolLayersModel->indexFromItem( newLayerItem ) );
  updateUi();
  updatePreview();
}

void QgsSymbolSelectorWidget::changeLayer( QgsSymbolLayer *newLayer )
{
  SymbolLayerItem *item = currentLayerItem();
  QgsSymbolLayer *layer = item->layer();

  if ( layer->subSymbol() )
  {
    item->removeRow( 0 );
  }
  QgsSymbol *symbol = static_cast<SymbolLayerItem *>( item->parent() )->symbol();

  // update symbol layer item
  item->setLayer( newLayer, symbol->type() );
  // When it is a marker symbol
  if ( newLayer->subSymbol() )
  {
    loadSymbol( newLayer->subSymbol(), item );
    layersTree->setExpanded( item->index(), true );
  }

  // Change the symbol at last to avoid deleting item's layer
  const int layerIdx = item->parent()->rowCount() - item->row() - 1;
  symbol->changeSymbolLayer( layerIdx, newLayer );

  item->updatePreview();
  updatePreview();
  // Important: This lets the layer have its own layer properties widget
  layerChanged();
}

QgsSymbolSelectorDialog::QgsSymbolSelectorDialog( QgsSymbol *symbol, QgsStyle *style, QgsVectorLayer *vl, QWidget *parent, bool embedded )
  : QDialog( parent )
{
  setLayout( new QVBoxLayout() );

  mSelectorWidget = new QgsSymbolSelectorWidget( symbol, style, vl, this );
  mButtonBox = new QDialogButtonBox( QDialogButtonBox::Cancel | QDialogButtonBox::Help | QDialogButtonBox::Ok );

  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );
  connect( mButtonBox, &QDialogButtonBox::helpRequested, this, &QgsSymbolSelectorDialog::showHelp );

  layout()->addWidget( mSelectorWidget );
  layout()->addWidget( mButtonBox );

  connect( mSelectorWidget, &QgsPanelWidget::panelAccepted, this, &QDialog::reject );

  mSelectorWidget->setMinimumSize( 460, 560 );
  setObjectName( QStringLiteral( "SymbolSelectorDialog" ) );
  QgsGui::enableAutoGeometryRestore( this );

  // Can be embedded in renderer properties dialog
  if ( embedded )
  {
    mButtonBox->hide();
    layout()->setContentsMargins( 0, 0, 0, 0 );
  }
  else
  {
    setWindowTitle( tr( "Symbol Selector" ) );
  }
  mSelectorWidget->setDockMode( embedded );
}

QMenu *QgsSymbolSelectorDialog::advancedMenu()
{
  return mSelectorWidget->advancedMenu();
}

void QgsSymbolSelectorDialog::setContext( const QgsSymbolWidgetContext &context )
{
  mContext = context;
}

QgsSymbolWidgetContext QgsSymbolSelectorDialog::context() const
{
  return mContext;
}

QgsSymbol *QgsSymbolSelectorDialog::symbol()
{
  return mSelectorWidget->symbol();
}

void QgsSymbolSelectorDialog::keyPressEvent( QKeyEvent *e )
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

void QgsSymbolSelectorDialog::reloadSymbol()
{
  mSelectorWidget->reloadSymbol();
}

void QgsSymbolSelectorDialog::loadSymbol( QgsSymbol *symbol, SymbolLayerItem *parent )
{
  mSelectorWidget->loadSymbol( symbol, parent );
}

void QgsSymbolSelectorDialog::updateUi()
{
  mSelectorWidget->updateUi();
}

void QgsSymbolSelectorDialog::updateLockButton()
{
  mSelectorWidget->updateLockButton();
}

SymbolLayerItem *QgsSymbolSelectorDialog::currentLayerItem()
{
  return mSelectorWidget->currentLayerItem();
}

QgsSymbolLayer *QgsSymbolSelectorDialog::currentLayer()
{
  return mSelectorWidget->currentLayer();
}

void QgsSymbolSelectorDialog::moveLayerByOffset( int offset )
{
  mSelectorWidget->moveLayerByOffset( offset );
}

void QgsSymbolSelectorDialog::setWidget( QWidget *widget )
{
  mSelectorWidget->setWidget( widget );
}

void QgsSymbolSelectorDialog::moveLayerDown()
{
  mSelectorWidget->moveLayerDown();
}

void QgsSymbolSelectorDialog::moveLayerUp()
{
  mSelectorWidget->moveLayerUp();
}

void QgsSymbolSelectorDialog::addLayer()
{
  mSelectorWidget->addLayer();
}

void QgsSymbolSelectorDialog::removeLayer()
{
  mSelectorWidget->removeLayer();
}

void QgsSymbolSelectorDialog::lockLayer()
{
  mSelectorWidget->lockLayer();
}

void QgsSymbolSelectorDialog::duplicateLayer()
{
  mSelectorWidget->duplicateLayer();
}

void QgsSymbolSelectorDialog::layerChanged()
{
  mSelectorWidget->layerChanged();
}

void QgsSymbolSelectorDialog::updateLayerPreview()
{
  mSelectorWidget->updateLayerPreview();
}

void QgsSymbolSelectorDialog::updatePreview()
{
  mSelectorWidget->updatePreview();
}

void QgsSymbolSelectorDialog::symbolChanged()
{
  mSelectorWidget->symbolChanged();
}

void QgsSymbolSelectorDialog::changeLayer( QgsSymbolLayer *layer )
{
  mSelectorWidget->changeLayer( layer );
}

QDialogButtonBox *QgsSymbolSelectorDialog::buttonBox() const
{
  return mButtonBox;
}

void QgsSymbolSelectorDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "style_library/symbol_selector.html" ) );
}

void QgsSymbolSelectorWidget::projectDataChanged()
{
  mBlockModified = true;
  symbolChanged();
  updatePreview();
  mBlockModified = false;
}

void QgsSymbolSelectorWidget::layersAboutToBeRemoved( const QList<QgsMapLayer *> &layers )
{
  if ( mVectorLayer && layers.contains( mVectorLayer ) )
  {
    disconnect( QgsProject::instance(), &QgsProject::projectColorsChanged, this, &QgsSymbolSelectorWidget::projectDataChanged );
  }
}
