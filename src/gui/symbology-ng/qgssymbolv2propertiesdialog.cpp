
#include "qgssymbolv2propertiesdialog.h"

#include <QFile>
#include <QStandardItem>
#include <QKeyEvent>

#include "qgssymbollayerv2.h"
#include "qgssymbollayerv2registry.h"

#include "qgsapplication.h"
#include "qgslogger.h"

#include "qgssymbollayerv2widget.h"
#include "qgsellipsesymbollayerv2widget.h"
#include "qgssymbolv2.h" //for the unit


static const int SymbolLayerItemType = QStandardItem::UserType + 1;

class SymbolLayerItem : public QStandardItem
{
  public:
    SymbolLayerItem( QgsSymbolLayerV2* layer )
    {
      setLayer( layer );
    }

    void setLayer( QgsSymbolLayerV2* layer )
    {
      mLayer = layer;
      updatePreview();
    }

    void updatePreview()
    {
      QIcon icon = QgsSymbolLayerV2Utils::symbolLayerPreviewIcon( mLayer, QgsSymbolV2::MM, QSize( 16, 16 ) ); //todo: make unit a parameter
      setIcon( icon );
    }

    int type() const { return SymbolLayerItemType; }

    QVariant data( int role ) const
    {
      if ( role == Qt::DisplayRole )
        return QgsSymbolLayerV2Registry::instance()->symbolLayerMetadata( mLayer->layerType() )->visibleName();
      if ( role == Qt::SizeHintRole )
        return QVariant( QSize( 32, 32 ) );
      if ( role == Qt::CheckStateRole )
        return QVariant(); // could be true/false
      return QStandardItem::data( role );
    }

  protected:
    QgsSymbolLayerV2* mLayer;
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

  _initWidgetFunction( "SimpleFill", QgsSimpleFillSymbolLayerV2Widget::create );
  _initWidgetFunction( "SVGFill", QgsSVGFillSymbolLayerWidget::create );
  _initWidgetFunction( "CentroidFill", QgsCentroidFillSymbolLayerV2Widget::create );

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

  loadSymbol();

  connect( btnUp, SIGNAL( clicked() ), this, SLOT( moveLayerUp() ) );
  connect( btnDown, SIGNAL( clicked() ), this, SLOT( moveLayerDown() ) );
  connect( btnAddLayer, SIGNAL( clicked() ), this, SLOT( addLayer() ) );
  connect( btnRemoveLayer, SIGNAL( clicked() ), this, SLOT( removeLayer() ) );
  connect( btnLock, SIGNAL( clicked() ), this, SLOT( lockLayer() ) );

  populateLayerTypes();
  connect( cboLayerType, SIGNAL( currentIndexChanged( int ) ), this, SLOT( layerTypeChanged() ) );

  loadPropertyWidgets();

  updateUi();

  // set first layer as active
  QModelIndex newIndex = listLayers->model()->index( 0, 0 );
  listLayers->setCurrentIndex( newIndex );
}


void QgsSymbolV2PropertiesDialog::loadSymbol()
{
  QStandardItemModel* model = new QStandardItemModel( this );
  listLayers->setModel( model );

  QItemSelectionModel* selModel = listLayers->selectionModel();
  connect( selModel, SIGNAL( currentChanged( const QModelIndex&, const QModelIndex& ) ), this, SLOT( layerChanged() ) );

  int count = mSymbol->symbolLayerCount();
  for ( int i = count - 1; i >= 0; i-- )
  {
    model->appendRow( new SymbolLayerItem( mSymbol->symbolLayer( i ) ) );
  }

  updatePreview();
}


void QgsSymbolV2PropertiesDialog::populateLayerTypes()
{
  QStringList types = QgsSymbolLayerV2Registry::instance()->symbolLayersForType( mSymbol->type() );

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
  int row = currentRowIndex();
  int count = listLayers->model()->rowCount();
  btnUp->setEnabled( row > 0 );
  btnDown->setEnabled( row < count - 1 && row != -1 );
  btnRemoveLayer->setEnabled( count > 1 && row != -1 );
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
  QString layerType = layer->layerType();

  // stop updating from the original widget
  if ( stackedWidget->currentWidget() != pageDummy )
    disconnect( stackedWidget->currentWidget(), SIGNAL( changed() ), this, SLOT( updateLayerPreview() ) );

  // update active properties widget
  if ( mWidgets.contains( layerType ) )
  {
    stackedWidget->setCurrentWidget( mWidgets[layerType] );
    mWidgets[layerType]->setSymbolLayer( layer );

    // start recieving updates from widget
    connect( mWidgets[layerType], SIGNAL( changed() ), this, SLOT( updateLayerPreview() ) );
  }
  else
  {
    // use dummy widget instead
    stackedWidget->setCurrentWidget( pageDummy );
  }
}

void QgsSymbolV2PropertiesDialog::loadPropertyWidgets()
{
  QgsSymbolLayerV2Registry* pReg = QgsSymbolLayerV2Registry::instance();

  QStringList layerTypes = pReg->symbolLayersForType( mSymbol->type() );

  // also load line symbol layers for fill symbols
  if ( mSymbol->type() == QgsSymbolV2::Fill )
    layerTypes += pReg->symbolLayersForType( QgsSymbolV2::Line );

  for ( int i = 0; i < layerTypes.count(); i++ )
  {
    QString layerType = layerTypes[i];
    QgsSymbolLayerV2AbstractMetadata* am = pReg->symbolLayerMetadata( layerType );
    if ( am == NULL ) // check whether the metadata is assigned
      continue;

    QgsSymbolLayerV2Widget* w = am->createSymbolLayerWidget( mVectorLayer );
    if ( w == NULL ) // check whether the function returns correct widget
      continue;

    mWidgets[layerType] = w;
    stackedWidget->addWidget( w );
  }
}

int QgsSymbolV2PropertiesDialog::currentRowIndex()
{
  QModelIndex idx = listLayers->selectionModel()->currentIndex();
  if ( !idx.isValid() )
    return -1;
  return idx.row();
}

int QgsSymbolV2PropertiesDialog::currentLayerIndex()
{
  return listLayers->model()->rowCount() - currentRowIndex() - 1;
}

SymbolLayerItem* QgsSymbolV2PropertiesDialog::currentLayerItem()
{
  int index = currentRowIndex();
  if ( index < 0 )
    return NULL;

  QStandardItemModel* model = qobject_cast<QStandardItemModel*>( listLayers->model() );
  if ( model == NULL )
    return NULL;
  QStandardItem* item = model->item( index );
  if ( item->type() != SymbolLayerItemType )
    return NULL;
  return static_cast<SymbolLayerItem*>( item );
}

QgsSymbolLayerV2* QgsSymbolV2PropertiesDialog::currentLayer()
{
  int idx = currentLayerIndex();
  if ( idx < 0 )
    return NULL;

  return mSymbol->symbolLayer( idx );
}


void QgsSymbolV2PropertiesDialog::layerChanged()
{
  updateUi();

  // get layer info
  QgsSymbolLayerV2* layer = currentLayer();
  if ( !layer )
    return;

  // update layer type combo box
  int idx = cboLayerType->findData( layer->layerType() );
  cboLayerType->setCurrentIndex( idx );

  updateSymbolLayerWidget( layer );

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
  mSymbol->changeSymbolLayer( currentLayerIndex(), newLayer );

  updateSymbolLayerWidget( newLayer );

  // update symbol layer item
  SymbolLayerItem* item = currentLayerItem();
  item->setLayer( newLayer );
  item->updatePreview();

  updatePreview();
}


void QgsSymbolV2PropertiesDialog::addLayer()
{
  QgsSymbolLayerV2* newLayer = QgsSymbolLayerV2Registry::instance()->defaultSymbolLayer( mSymbol->type() );

  mSymbol->appendSymbolLayer( newLayer );

  loadSymbol();

  QModelIndex newIndex = listLayers->model()->index( 0, 0 );
  listLayers->setCurrentIndex( newIndex );

  updateUi();
}


void QgsSymbolV2PropertiesDialog::removeLayer()
{
  int idx = currentLayerIndex();
  if ( idx < 0 )
    return;
  int row = currentRowIndex();
  mSymbol->deleteSymbolLayer( idx );

  loadSymbol();

  updateUi();

  // set previous layer as active
  QModelIndex newIndex = listLayers->model()->index( qMin( row, mSymbol->symbolLayerCount() - 1 ), 0 );
  listLayers->setCurrentIndex( newIndex );
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
  int rowIdx = currentRowIndex();
  int layerIdx = currentLayerIndex();

  // switch layers
  QgsSymbolLayerV2* tmpLayer = mSymbol->takeSymbolLayer( layerIdx );
  mSymbol->insertSymbolLayer( layerIdx - offset, tmpLayer );

  loadSymbol();

  QModelIndex newIndex = listLayers->model()->index( rowIdx + offset, 0 );
  listLayers->setCurrentIndex( newIndex );

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
