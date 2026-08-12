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

#include "qgsexpressioncontextutils.h"
#include "qgsscreenhelper.h"
#include "qgsstyle.h"
#include "qgssymbol.h"
#include "qgssymbollayer.h"
#include "qgssymbollayermodel.h"
#include "qgssymbollayerregistry.h"
#include "qgssymbollayerutils.h"

#include <QString>

#include "moc_qgssymbolselectordialog.cpp"

using namespace Qt::StringLiterals;

// the widgets
#include "qgssymbolslistwidget.h"
#include "qgslayerpropertieswidget.h"
#include "qgsapplication.h"
#include "qgsvectorlayer.h"
#include "qgssvgcache.h"
#include "qgsimagecache.h"
#include "qgsproject.h"
#include "qgsgui.h"
#include "qgsmarkersymbol.h"
#include "qgslinesymbol.h"
#include "qscreen.h"

#include <QColorDialog>
#include <QPainter>
#include <QInputDialog>
#include <QMessageBox>
#include <QKeyEvent>
#include <QMenu>

#include <QWidget>
#include <QFile>
#include <memory>

/// @cond PRIVATE


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
  if ( mMarker && mMarker->symbolLayerCount() > 1 )
  {
    if ( mDDSize && ( mSize != mMarkerSymbolLayer->size() || mMarkerOffset != mMarkerSymbolLayer->offset() ) )
      mMarker->setDataDefinedSize( mDDSize );
    if ( mDDAngle && mAngle != mMarkerSymbolLayer->angle() )
      mMarker->setDataDefinedAngle( mDDAngle );
  }
  else if ( mLine && mLine->symbolLayerCount() > 1 )
  {
    if ( mDDWidth && ( mWidth != mLineSymbolLayer->width() || mLineOffset != mLineSymbolLayer->offset() ) )
      mLine->setDataDefinedWidth( mDDWidth );
  }
  save();
}

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

  layersTree->setMaximumHeight( static_cast<int>( Qgis::UI_SCALE_FACTOR * fontMetrics().height() * 7 ) );
  layersTree->setMinimumHeight( layersTree->maximumHeight() );
  lblPreview->setMaximumWidth( layersTree->maximumHeight() );

  // setup icons
  btnAddLayer->setIcon( QIcon( QgsApplication::iconPath( "symbologyAdd.svg" ) ) );
  btnRemoveLayer->setIcon( QIcon( QgsApplication::iconPath( "symbologyRemove.svg" ) ) );
  QIcon iconLock;
  iconLock.addFile( QgsApplication::iconPath( u"locked.svg"_s ), QSize(), QIcon::Normal, QIcon::On );
  iconLock.addFile( QgsApplication::iconPath( u"locked.svg"_s ), QSize(), QIcon::Active, QIcon::On );
  iconLock.addFile( QgsApplication::iconPath( u"unlocked.svg"_s ), QSize(), QIcon::Normal, QIcon::Off );
  iconLock.addFile( QgsApplication::iconPath( u"unlocked.svg"_s ), QSize(), QIcon::Active, QIcon::Off );

  QIcon iconColorLock;
  iconColorLock.addFile( QgsApplication::iconPath( u"mIconColorLocked.svg"_s ), QSize(), QIcon::Normal, QIcon::On );
  iconColorLock.addFile( QgsApplication::iconPath( u"mIconColorLocked.svg"_s ), QSize(), QIcon::Active, QIcon::On );
  iconColorLock.addFile( QgsApplication::iconPath( u"mIconColorUnlocked.svg"_s ), QSize(), QIcon::Normal, QIcon::Off );
  iconColorLock.addFile( QgsApplication::iconPath( u"mIconColorUnlocked.svg"_s ), QSize(), QIcon::Active, QIcon::Off );

  mLockColorAction = new QAction( tr( "Lock Color" ), this );
  mLockColorAction->setToolTip( tr( "Avoid changing the color of the layer when the symbol color is changed" ) );
  mLockColorAction->setCheckable( true );
  mLockColorAction->setIcon( iconColorLock );

  QIcon iconSelectLock;
  iconSelectLock.addFile( QgsApplication::iconPath( u"mIconSelectLocked.svg"_s ), QSize(), QIcon::Normal, QIcon::On );
  iconSelectLock.addFile( QgsApplication::iconPath( u"mIconSelectLocked.svg"_s ), QSize(), QIcon::Active, QIcon::On );
  iconSelectLock.addFile( QgsApplication::iconPath( u"mIconSelectUnlocked.svg"_s ), QSize(), QIcon::Normal, QIcon::Off );
  iconSelectLock.addFile( QgsApplication::iconPath( u"mIconSelectUnlocked.svg"_s ), QSize(), QIcon::Active, QIcon::Off );

  mLockSelectionColorAction = new QAction( tr( "Lock Color When Selected" ), this );
  mLockSelectionColorAction->setToolTip( tr( "Avoid changing the color of the layer when a feature is selected" ) );
  mLockSelectionColorAction->setCheckable( true );
  mLockSelectionColorAction->setIcon( iconSelectLock );

  QMenu *lockMenu = new QMenu( this );
  lockMenu->addAction( mLockColorAction );
  lockMenu->addAction( mLockSelectionColorAction );
  btnLock->setMenu( lockMenu );
  btnLock->setPopupMode( QToolButton::InstantPopup );

  btnDuplicate->setIcon( QIcon( QgsApplication::iconPath( "mActionDuplicateLayer.svg" ) ) );
  btnUp->setIcon( QIcon( QgsApplication::iconPath( "mActionArrowUp.svg" ) ) );
  btnDown->setIcon( QIcon( QgsApplication::iconPath( "mActionArrowDown.svg" ) ) );

  mSymbolLayersModel = new QgsSymbolLayerModel( mVectorLayer, layersTree, screen() );

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

  mSymbolLayersModel->setSymbol( mSymbol );

  layersTree->expandAll();
  updatePreview();

  connect( btnUp, &QAbstractButton::clicked, this, &QgsSymbolSelectorWidget::moveLayerUp );
  connect( btnDown, &QAbstractButton::clicked, this, &QgsSymbolSelectorWidget::moveLayerDown );
  connect( btnAddLayer, &QAbstractButton::clicked, this, &QgsSymbolSelectorWidget::addLayer );
  connect( btnRemoveLayer, &QAbstractButton::clicked, this, &QgsSymbolSelectorWidget::removeLayer );
  connect( mLockColorAction, &QAction::toggled, this, &QgsSymbolSelectorWidget::lockLayer );
  connect( mLockSelectionColorAction, &QAction::toggled, this, [this]( bool checked ) {
    QgsSymbolLayer *layer = currentLayer();
    if ( !layer )
      return;

    Qgis::SymbolLayerUserFlags flags = layer->userFlags();
    flags.setFlag( Qgis::SymbolLayerUserFlag::DisableSelectionRecoloring, checked );
    layer->setUserFlags( flags );
    updateLockButtonIcon();
    emit symbolModified();
  } );
  connect( btnDuplicate, &QAbstractButton::clicked, this, &QgsSymbolSelectorWidget::duplicateLayer );
  connect( this, &QgsSymbolSelectorWidget::symbolModified, this, &QgsPanelWidget::widgetChanged );

  updateLockButtonIcon();

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

  connect( QgsProject::instance(), static_cast<void ( QgsProject::* )( const QList<QgsMapLayer *> &layers )>( &QgsProject::layersWillBeRemoved ), this, &QgsSymbolSelectorWidget::layersAboutToBeRemoved );

  auto screenHelper = new QgsScreenHelper( this );
  connect( screenHelper, &QgsScreenHelper::screenDpiChanged, this, &QgsSymbolSelectorWidget::updatePreview );
  connect( screenHelper, &QgsScreenHelper::screenDpiChanged, this, &QgsSymbolSelectorWidget::updateListIcons );
}

QgsSymbolSelectorWidget *QgsSymbolSelectorWidget::createWidgetWithSymbolOwnership( std::unique_ptr<QgsSymbol> symbol, QgsStyle *style, QgsVectorLayer *vl, QWidget *parent )
{
  QgsSymbolSelectorWidget *widget = new QgsSymbolSelectorWidget( symbol.get(), style, vl, parent );
  // transfer ownership of symbol to widget, so that we are guaranteed it will last for the duration of the widget
  widget->mOwnedSymbol = std::move( symbol );
  return widget;
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
  if ( QgsLayerPropertiesWidget *layerProp = qobject_cast<QgsLayerPropertiesWidget *>( widget ) )
  {
    layerProp->setContext( context );
  }
  else if ( QgsSymbolsListWidget *listWidget = qobject_cast<QgsSymbolsListWidget *>( widget ) )
  {
    listWidget->setContext( context );
  }

  layerChanged();
  updatePreview();
}

QgsSymbolWidgetContext QgsSymbolSelectorWidget::context() const
{
  return mContext;
}

void QgsSymbolSelectorWidget::loadSymbol( QgsSymbol *symbol )
{
  if ( !symbol )
    return;

  mSymbol = symbol;
  mSymbolLayersModel->setSymbol( symbol );
}

void QgsSymbolSelectorWidget::reloadSymbol()
{
  mSymbolLayersModel->setSymbol( mSymbol );
  layersTree->expandAll();
}

void QgsSymbolSelectorWidget::updateUi()
{
  const QModelIndex currentIdx = layersTree->currentIndex();
  if ( !currentIdx.isValid() )
    return;

  QgsSymbolLayerModelNode *node = mSymbolLayersModel->index2node( currentIdx );
  if ( !node->isLayer() )
  {
    btnUp->setEnabled( false );
    btnDown->setEnabled( false );
    btnRemoveLayer->setEnabled( false );
    btnLock->setEnabled( false );
    btnDuplicate->setEnabled( false );
    return;
  }

  const int rowCount = node->parent()->rowCount();
  const int currentRow = node->rowIndex();

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

  std::unique_ptr<QgsSymbol> symbolClone( mSymbol->clone() );
  const QImage preview = symbolClone->bigSymbolPreviewImage( &mPreviewExpressionContext, Qgis::SymbolPreviewFlag::FlagIncludeCrosshairsForMarkerSymbols, QgsScreenProperties( screen() ) );
  lblPreview->setPixmap( QPixmap::fromImage( preview ) );
}

void QgsSymbolSelectorWidget::updateLayerPreview()
{
  // get current layer item and update its icon

  QgsSymbolLayerModelNode *node = currentLayerNode();
  if ( node )
    mSymbolLayersModel->updatePreview( node );
  // update also preview of the whole symbol
  updatePreview();
}

QgsSymbolLayerModelNode *QgsSymbolSelectorWidget::currentLayerNode()
{
  const QModelIndex idx = layersTree->currentIndex();
  if ( !idx.isValid() )
    return nullptr;

  QgsSymbolLayerModelNode *node = mSymbolLayersModel->index2node( idx );
  if ( !node->isLayer() )
    return nullptr;

  return node;
}

QgsSymbolLayer *QgsSymbolSelectorWidget::currentLayer()
{
  const QModelIndex idx = layersTree->currentIndex();
  if ( !idx.isValid() )
    return nullptr;

  QgsSymbolLayerModelNode *node = mSymbolLayersModel->index2node( idx );
  if ( node->isLayer() )
    return node->layer();

  return nullptr;
}

void QgsSymbolSelectorWidget::layerChanged()
{
  updateUi();

  QgsSymbolLayerModelNode *currentNode = mSymbolLayersModel->index2node( layersTree->currentIndex() );
  if ( !currentNode )
    return;

  if ( currentNode->isLayer() )
  {
    QgsSymbolLayerModelNode *parent = currentNode->parent();
    mDataDefineRestorer = std::make_unique<DataDefinedRestorer>( parent->symbol(), currentNode->layer() );
    QgsLayerPropertiesWidget *layerProp = new QgsLayerPropertiesWidget( currentNode->layer(), parent->symbol(), mVectorLayer );
    layerProp->setDockMode( this->dockMode() );
    layerProp->setContext( mContext );
    setWidget( layerProp );
    connect( layerProp, &QgsLayerPropertiesWidget::changed, mDataDefineRestorer.get(), &DataDefinedRestorer::restore );
    connect( layerProp, &QgsLayerPropertiesWidget::changed, this, &QgsSymbolSelectorWidget::updateLayerPreview );
    connect( layerProp, &QgsLayerPropertiesWidget::changed, this, &QgsSymbolSelectorWidget::emitSymbolModified );
    // This connection when layer type is changed
    connect( layerProp, &QgsLayerPropertiesWidget::changeLayer, this, &QgsSymbolSelectorWidget::changeLayer );

    connectChildPanel( layerProp );
  }
  else
  {
    // then it must be a symbol
    mDataDefineRestorer.reset();
    Q_NOWARN_DEPRECATED_PUSH
    currentNode->symbol()->setLayer( mVectorLayer );
    Q_NOWARN_DEPRECATED_POP
    // Now populate symbols of that type using the symbols list widget:
    QgsSymbolsListWidget *symbolsList = new QgsSymbolsListWidget( currentNode->symbol(), mStyle, mAdvancedMenu, this, mVectorLayer );
    symbolsList->setContext( mContext );

    setWidget( symbolsList );
    connect( symbolsList, &QgsSymbolsListWidget::changed, this, &QgsSymbolSelectorWidget::symbolChanged );
  }
  updateLockButton();
}

void QgsSymbolSelectorWidget::symbolChanged()
{
  QgsSymbolLayerModelNode *currentNode = mSymbolLayersModel->index2node( layersTree->currentIndex() );
  if ( !currentNode || currentNode->isLayer() )
    return;
  // disconnect to avoid recreating widget
  disconnect( layersTree->selectionModel(), &QItemSelectionModel::currentChanged, this, &QgsSymbolSelectorWidget::layerChanged );
  if ( currentNode->parent() && !currentNode->parent()->isRootNode() )
  {
    // it is a sub-symbol

    QgsSymbol *symbol = currentNode->symbol();
    QgsSymbolLayerModelNode *parent = currentNode->parent();

    mSymbolLayersModel->updateNode( symbol, parent );

    layersTree->expandRecursively( mSymbolLayersModel->node2index( parent->childAt( 0 ) ) );
    layersTree->setCurrentIndex( mSymbolLayersModel->node2index( parent->childAt( 0 ) ) );
  }
  else
  {
    //it is the symbol itself
    reloadSymbol();
    const QModelIndex newIndex = layersTree->model()->index( 0, 0 );
    layersTree->setCurrentIndex( newIndex );
  }
  updatePreview();
  emitSymbolModified();
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
  mLockColorAction->setChecked( layer->isLocked() );
  mLockSelectionColorAction->setChecked( layer->userFlags() & Qgis::SymbolLayerUserFlag::DisableSelectionRecoloring );

  updateLockButtonIcon();
}

void QgsSymbolSelectorWidget::updateLockButtonIcon()
{
  if ( mLockColorAction->isChecked() && mLockSelectionColorAction->isChecked() )
    btnLock->setIcon( QgsApplication::getThemeIcon( u"locked.svg"_s ) );
  else if ( mLockColorAction->isChecked() )
    btnLock->setIcon( QgsApplication::getThemeIcon( u"mIconColorLocked.svg"_s ) );
  else if ( mLockSelectionColorAction->isChecked() )
    btnLock->setIcon( QgsApplication::getThemeIcon( u"mIconSelectLocked.svg"_s ) );
  else
    btnLock->setIcon( QgsApplication::getThemeIcon( u"unlocked.svg"_s ) );
}

void QgsSymbolSelectorWidget::addLayer()
{
  const QModelIndex idx = layersTree->currentIndex();

  QgsSymbolLayerModelNode *newNode = mSymbolLayersModel->addLayer( idx );

  layersTree->expandRecursively( mSymbolLayersModel->node2index( newNode ) );
  layersTree->setCurrentIndex( mSymbolLayersModel->node2index( newNode ) );
  updateUi();
  updatePreview();
  emitSymbolModified();
}

void QgsSymbolSelectorWidget::removeLayer()
{
  QgsSymbolLayerModelNode *node = currentLayerNode();

  mSymbolLayersModel->removeLayer( node );

  updateUi();
  updatePreview();
  emitSymbolModified();
}

void QgsSymbolSelectorWidget::moveLayerDown()
{
  moveLayerByOffset( +1 );
}

void QgsSymbolSelectorWidget::moveLayerUp()
{
  moveLayerByOffset( -1 );
}

void QgsSymbolSelectorWidget::moveLayerByOffset( int offset )
{
  QgsSymbolLayerModelNode *node = currentLayerNode();
  if ( !node )
    return;

  mSymbolLayersModel->moveLayerByOffset( node, offset );

  layersTree->setCurrentIndex( mSymbolLayersModel->node2index( node ) );

  updatePreview();
  emitSymbolModified();
  updateUi();
}

void QgsSymbolSelectorWidget::lockLayer()
{
  QgsSymbolLayer *layer = currentLayer();
  if ( !layer )
    return;
  layer->setLocked( mLockColorAction->isChecked() );
  updateLockButtonIcon();
  emit symbolModified();
}

void QgsSymbolSelectorWidget::duplicateLayer()
{
  const QModelIndex idx = layersTree->currentIndex();
  if ( !idx.isValid() )
    return;

  QgsSymbolLayerModelNode *node = mSymbolLayersModel->index2node( idx );
  if ( !node->isLayer() )
    return;

  QgsSymbolLayerModelNode *newNode = mSymbolLayersModel->duplicateLayer( node );

  if ( newNode )
  {
    layersTree->expandRecursively( mSymbolLayersModel->node2index( newNode ) );
    layersTree->setCurrentIndex( mSymbolLayersModel->node2index( newNode ) );
  }

  updateUi();
  updatePreview();
  emitSymbolModified();
}

void QgsSymbolSelectorWidget::changeLayer( QgsSymbolLayer *newLayer )
{
  QgsSymbolLayerModelNode *node = currentLayerNode();

  mSymbolLayersModel->changeLayer( node, newLayer );

  QModelIndex nodeIndex = mSymbolLayersModel->node2index( node );
  layersTree->expandRecursively( nodeIndex );
  layersTree->setCurrentIndex( nodeIndex );

  updatePreview();
  emitSymbolModified();
  // Important: This lets the layer have its own layer properties widget
  layerChanged();
}

void QgsSymbolSelectorWidget::updateListIcons()
{
  mSymbolLayersModel->setScreen( screen() );
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
  setObjectName( u"SymbolSelectorDialog"_s );
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
  mSelectorWidget->setContext( context );
}

QgsSymbolWidgetContext QgsSymbolSelectorDialog::context() const
{
  return mSelectorWidget->context();
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

void QgsSymbolSelectorDialog::loadSymbol( QgsSymbol *symbol )
{
  mSelectorWidget->loadSymbol( symbol );
}

void QgsSymbolSelectorDialog::updateUi()
{
  mSelectorWidget->updateUi();
}

void QgsSymbolSelectorDialog::updateLockButton()
{
  mSelectorWidget->updateLockButton();
}

QgsSymbolLayerModelNode *QgsSymbolSelectorDialog::currentLayerNode()
{
  return mSelectorWidget->currentLayerNode();
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
  QgsHelp::openHelp( u"style_library/symbol_selector.html"_s );
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

void QgsSymbolSelectorWidget::emitSymbolModified()
{
  if ( !mBlockModified )
  {
    emit symbolModified();
  }
}
