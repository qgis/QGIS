#include "qgscategorizedsymbolrendererv2widget.h"

#include "qgscategorizedsymbolrendererv2.h"

#include "qgssymbolv2.h"
#include "qgssymbollayerv2utils.h"
#include "qgsvectorcolorrampv2.h"
#include "qgsstylev2.h"

#include "qgssymbolv2selectordialog.h"

#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h" // for uniqueValues

#include <QMessageBox>
#include <QStandardItemModel>
#include <QStandardItem>


QgsRendererV2Widget* QgsCategorizedSymbolRendererV2Widget::create( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer )
{
  return new QgsCategorizedSymbolRendererV2Widget( layer, style, renderer );
}

QgsCategorizedSymbolRendererV2Widget::QgsCategorizedSymbolRendererV2Widget( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer )
    : QgsRendererV2Widget( layer, style )
{

  // try to recognize the previous renderer
  // (null renderer means "no previous renderer")
  if ( !renderer || renderer->type() != "categorizedSymbol" )
  {
    // we're not going to use it - so let's delete the renderer
    delete renderer;

    mRenderer = new QgsCategorizedSymbolRendererV2( "", QgsCategoryList() );
  }
  else
  {
    mRenderer = static_cast<QgsCategorizedSymbolRendererV2*>( renderer );
  }

  // setup user interface
  setupUi( this );

  populateColumns();
  populateColorRamps();
  QStandardItemModel* m = new QStandardItemModel( this );
  QStringList labels;
  labels << "Value" << "Label";
  m->setHorizontalHeaderLabels( labels );
  viewCategories->setModel( m );

  mCategorizedSymbol = QgsSymbolV2::defaultSymbol( mLayer->geometryType() );

  connect( cboCategorizedColumn, SIGNAL( currentIndexChanged( int ) ), this, SLOT( categoryColumnChanged() ) );

  connect( viewCategories, SIGNAL( doubleClicked( const QModelIndex & ) ), this, SLOT( categoriesDoubleClicked( const QModelIndex & ) ) );

  connect( btnChangeCategorizedSymbol, SIGNAL( clicked() ), this, SLOT( changeCategorizedSymbol() ) );
  connect( btnAddCategories, SIGNAL( clicked() ), this, SLOT( addCategories() ) );
  connect( btnDeleteCategory, SIGNAL( clicked() ), this, SLOT( deleteCategory() ) );
  connect( btnDeleteAllCategories, SIGNAL( clicked() ), this, SLOT( deleteAllCategories() ) );
  connect( m, SIGNAL( itemChanged( QStandardItem * ) ), this, SLOT( changeCurrentValue( QStandardItem * ) ) );

  // update GUI from renderer
  updateUiFromRenderer();
}

QgsCategorizedSymbolRendererV2Widget::~QgsCategorizedSymbolRendererV2Widget()
{
  delete mRenderer;
}

void QgsCategorizedSymbolRendererV2Widget::updateUiFromRenderer()
{

  updateCategorizedSymbolIcon();
  populateCategories();

  // set column
  disconnect( cboCategorizedColumn, SIGNAL( currentIndexChanged( int ) ), this, SLOT( categoryColumnChanged() ) );
  QString attrName = mRenderer->classAttribute();
  int idx = cboCategorizedColumn->findText( attrName, Qt::MatchExactly );
  cboCategorizedColumn->setCurrentIndex( idx >= 0 ? idx : 0 );
  connect( cboCategorizedColumn, SIGNAL( currentIndexChanged( int ) ), this, SLOT( categoryColumnChanged() ) );

  // set source symbol
  if ( mRenderer->sourceSymbol() )
  {
    delete mCategorizedSymbol;
    mCategorizedSymbol = mRenderer->sourceSymbol()->clone();
    updateCategorizedSymbolIcon();
  }

  // set source color ramp
  if ( mRenderer->sourceColorRamp() )
  {
    QSize rampIconSize( 50, 16 );
    QIcon icon = QgsSymbolLayerV2Utils::colorRampPreviewIcon( mRenderer->sourceColorRamp(), rampIconSize );
    if ( cboCategorizedColorRamp->itemText( 0 ) == "[source]" )
      cboCategorizedColorRamp->setItemIcon( 0, icon );
    else
      cboCategorizedColorRamp->insertItem( 0, icon, "[source]" );
    cboCategorizedColorRamp->setCurrentIndex( 0 );
  }

}

QgsFeatureRendererV2* QgsCategorizedSymbolRendererV2Widget::renderer()
{
  return mRenderer;
}

void QgsCategorizedSymbolRendererV2Widget::changeCategorizedSymbol()
{
  QgsSymbolV2SelectorDialog dlg( mCategorizedSymbol, mStyle, this );
  if ( !dlg.exec() )
    return;

  updateCategorizedSymbolIcon();
}

void QgsCategorizedSymbolRendererV2Widget::updateCategorizedSymbolIcon()
{
  QIcon icon = QgsSymbolLayerV2Utils::symbolPreviewIcon( mCategorizedSymbol, btnChangeCategorizedSymbol->iconSize() );
  btnChangeCategorizedSymbol->setIcon( icon );
}


void QgsCategorizedSymbolRendererV2Widget::populateCategories()
{
  QStandardItemModel* m = qobject_cast<QStandardItemModel*>( viewCategories->model() );
  m->clear();

  QStringList labels;
  labels << "Value" << "Label";
  m->setHorizontalHeaderLabels( labels );

  QSize iconSize( 16, 16 );

  int i, count = mRenderer->categories().count();

  // TODO: sort?? utils.sortVariantList(keys);

  for ( i = 0; i < count; i++ )
  {
    const QgsRendererCategoryV2& cat = mRenderer->categories()[i];
    QVariant k = cat.value();

    QIcon icon = QgsSymbolLayerV2Utils::symbolPreviewIcon( cat.symbol(), iconSize );
    QStandardItem* item = new QStandardItem( icon, k.toString() );
    item->setData( k ); // set attribute value as user role
    item->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );

    QList<QStandardItem *> list;
    list << item << new QStandardItem( cat.label() );
    m->appendRow( list );
  }
}

void QgsCategorizedSymbolRendererV2Widget::populateColumns()
{
  cboCategorizedColumn->clear();
  const QgsFieldMap& flds = mLayer->pendingFields();
  QgsFieldMap::ConstIterator it = flds.begin();
  for ( ; it != flds.end(); ++it )
  {
    cboCategorizedColumn->addItem( it->name() );
  }
}

void QgsCategorizedSymbolRendererV2Widget::populateColorRamps()
{
  QSize rampIconSize( 50, 16 );
  cboCategorizedColorRamp->setIconSize( rampIconSize );

  QStringList rampNames = mStyle->colorRampNames();
  for ( QStringList::iterator it = rampNames.begin(); it != rampNames.end(); ++it )
  {
    QgsVectorColorRampV2* ramp = mStyle->colorRamp( *it );
    QIcon icon = QgsSymbolLayerV2Utils::colorRampPreviewIcon( ramp, rampIconSize );
    cboCategorizedColorRamp->addItem( icon, *it );
    delete ramp;
  }
}


void QgsCategorizedSymbolRendererV2Widget::categoryColumnChanged()
{
  deleteAllCategories();
}

void QgsCategorizedSymbolRendererV2Widget::categoriesDoubleClicked( const QModelIndex & idx )
{
  if ( idx.isValid() && idx.column() == 0 )
    changeCategorySymbol();
}

void QgsCategorizedSymbolRendererV2Widget::changeCategorySymbol()
{
  QVariant k = currentCategory();
  if ( !k.isValid() )
    return;

  int catIdx = mRenderer->categoryIndexForValue( k );
  QgsSymbolV2* newSymbol = mRenderer->categories()[catIdx].symbol()->clone();

  QgsSymbolV2SelectorDialog dlg( newSymbol, mStyle, this );
  if ( !dlg.exec() )
  {
    delete newSymbol;
    return;
  }

  mRenderer->updateCategorySymbol( catIdx, newSymbol );

  populateCategories();
}




static void _createCategories( QgsCategoryList& cats, QList<QVariant>& values, QgsSymbolV2* symbol, QgsVectorColorRampV2* ramp )
{
  // sort the categories first
  // TODO: sortVariantList(values);

  int num = values.count();

  for ( int i = 0; i < num; i++ )
  {
    QVariant value = values[i];
    double x = i / ( double ) num;
    QgsSymbolV2* newSymbol = symbol->clone();
    newSymbol->setColor( ramp->color( x ) );

    cats.append( QgsRendererCategoryV2( value, newSymbol, value.toString() ) );
  }

}

void QgsCategorizedSymbolRendererV2Widget::addCategories()
{
  QString attrName = cboCategorizedColumn->currentText();
  int idx = mLayer->fieldNameIndex( attrName );
  QList<QVariant> unique_vals;
  mLayer->dataProvider()->uniqueValues( idx, unique_vals );

  //DlgAddCategories dlg(mStyle, createDefaultSymbol(), unique_vals, this);
  //if (!dlg.exec())
  //  return;

  QgsVectorColorRampV2* ramp = NULL;
  QString rampName = cboCategorizedColorRamp->currentText();
  if ( rampName == "[source]" )
    ramp = mRenderer->sourceColorRamp()->clone();
  else
    ramp = mStyle->colorRamp( rampName );

  if ( ramp == NULL )
  {
    if ( cboCategorizedColorRamp->count() == 0 )
      QMessageBox::critical( this, tr( "Error" ), tr( "There are no available color ramps. You can add them in Style Manager." ) );
    else
      QMessageBox::critical( this, tr( "Error" ), tr( "The selected color ramp is not available." ) );
    return;
  }

  QgsCategoryList cats;
  _createCategories( cats, unique_vals, mCategorizedSymbol, ramp );

  // TODO: if not all categories are desired, delete some!
  /*
  if (not dlg.radAllCats.isChecked())
  {
    cats2 = {}
    for item in dlg.listCategories.selectedItems():
      for k,c in cats.iteritems():
        if item.text() == k.toString():
          break
      cats2[k] = c
    cats = cats2
  }
  */

  // recreate renderer
  delete mRenderer;
  mRenderer = new QgsCategorizedSymbolRendererV2( attrName, cats );
  mRenderer->setSourceSymbol( mCategorizedSymbol->clone() );
  mRenderer->setSourceColorRamp( ramp->clone() );

  populateCategories();
}

int QgsCategorizedSymbolRendererV2Widget::currentCategoryRow()
{
  QModelIndex idx = viewCategories->selectionModel()->currentIndex();
  if ( !idx.isValid() )
    return -1;
  return idx.row();
}

QVariant QgsCategorizedSymbolRendererV2Widget::currentCategory()
{
  int row = currentCategoryRow();
  if ( row == -1 )
    return QVariant();
  QStandardItemModel* m = qobject_cast<QStandardItemModel*>( viewCategories->model() );
  return m->item( row )->data();
}

void QgsCategorizedSymbolRendererV2Widget::deleteCategory()
{
  QVariant k = currentCategory();
  if ( !k.isValid() )
    return;

  int idx = mRenderer->categoryIndexForValue( k );
  if ( idx < 0 )
    return;

  mRenderer->deleteCategory( idx );

  populateCategories();
}

void QgsCategorizedSymbolRendererV2Widget::deleteAllCategories()
{
  mRenderer->deleteAllCategories();
  populateCategories();
}

void QgsCategorizedSymbolRendererV2Widget::changeCurrentValue( QStandardItem * item )
{
  if ( item->column() == 1 )
  {
    QString label = item->text();
    int idx = item->row();
    mRenderer->updateCategoryLabel( idx, label );
  }
}
