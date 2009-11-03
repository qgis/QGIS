
#include "qgsrendererv2propertiesdialog.h"

#include "qgssymbollayerv2utils.h"
#include "qgsrendererv2.h"
#include "qgssymbolv2.h"
#include "qgsvectorcolorrampv2.h"
#include "qgsstylev2.h"

#include "qgssymbolv2selectordialog.h"
#include "qgssymbollevelsv2dialog.h"

#include "qgslogger.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"

#include <QStandardItemModel>
#include <QStandardItem>

QgsRendererV2PropertiesDialog::QgsRendererV2PropertiesDialog(QgsVectorLayer* layer, QgsStyleV2* style, QWidget* parent, bool embedded)
  : QDialog(parent), mStyle(style)
{
  mLayer = layer;
  
  // if the layer doesn't use renderer V2, let's start using it!
  if (!mLayer->isUsingRendererV2())
  {
    mLayer->setRendererV2( QgsFeatureRendererV2::defaultRenderer(mLayer->geometryType()) );
    mLayer->setUsingRendererV2(true);
  }

  mRenderer = mLayer->rendererV2()->clone();
  
  setupUi(this);

  // can be embedded in vector layer properties
  if (embedded)
  {
    buttonBox->hide();
  }

  connect(buttonBox, SIGNAL(accepted()), this, SLOT(onOK()));

  connect(btnSymbolLevels, SIGNAL(clicked()), this, SLOT(symbolLevels()));

  connect(radSingleSymbol, SIGNAL(clicked()), this, SLOT(updateRenderer()));
  connect(radCategorized, SIGNAL(clicked()), this, SLOT(updateRenderer()));
  connect(radGraduated, SIGNAL(clicked()), this, SLOT(updateRenderer()));
  
  // simple symbol page
  if (mRenderer->type() == QgsFeatureRendererV2::RendererSingleSymbol)
    mSingleSymbol = ((QgsSingleSymbolRendererV2*)mRenderer)->symbol()->clone();
  else
    mSingleSymbol = QgsSymbolV2::defaultSymbol(mLayer->geometryType());

  stackedWidget->removeWidget(pageSingleSymbol);
  delete pageSingleSymbol;
  pageSingleSymbol = new QgsSymbolV2SelectorDialog(mSingleSymbol, mStyle, NULL, true);
  stackedWidget->addWidget( pageSingleSymbol );
  connect(pageSingleSymbol, SIGNAL(symbolModified()), this, SLOT(changeSingleSymbol()));

  // categorized symbol page
  
  populateColumns(); // together with graduated
  QStandardItemModel* m = new QStandardItemModel(this);
  QStringList labels;
  labels << "Value" << "Label";
  m->setHorizontalHeaderLabels(labels);
  viewCategories->setModel(m);

  mCategorizedSymbol = QgsSymbolV2::defaultSymbol(mLayer->geometryType());

  connect(cboCategorizedColumn, SIGNAL(currentIndexChanged(int)), this, SLOT(categoryColumnChanged()));
  
  connect(viewCategories, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(categoriesDoubleClicked(const QModelIndex &)));
  
  connect(btnChangeCategorizedSymbol, SIGNAL(clicked()), this, SLOT(changeCategorizedSymbol()));
  connect(btnAddCategories, SIGNAL(clicked()), this, SLOT(addCategories()));
  connect(btnDeleteCategory, SIGNAL(clicked()), this, SLOT(deleteCategory()));
  connect(btnDeleteAllCategories, SIGNAL(clicked()), this, SLOT(deleteAllCategories()));
  
  // graduated symbol page
  populateColorRamps();
  QStandardItemModel* mg = new QStandardItemModel(this);
  QStringList labelsG;
  labels << "Range" << "Label";
  mg->setHorizontalHeaderLabels(labels);
  viewGraduated->setModel(mg);
  
  mGraduatedSymbol = QgsSymbolV2::defaultSymbol(mLayer->geometryType());
  
  connect(viewGraduated, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(rangesDoubleClicked(const QModelIndex &)));

  connect(btnGraduatedClassify, SIGNAL(clicked()), this, SLOT(classifyGraduated()));
  connect(btnChangeGraduatedSymbol, SIGNAL(clicked()), this, SLOT(changeGraduatedSymbol()));

  // initialize from previously set renderer
  updateUiFromRenderer();
}

QgsRendererV2PropertiesDialog::~QgsRendererV2PropertiesDialog()
{
  // delete the temporary renderer (if exists)
  delete mRenderer;

  delete mSingleSymbol;
  // TODO: delete categorized, graduated symbol?
}

void QgsRendererV2PropertiesDialog::apply()
{
  // use clone of our temporary renderer
  mLayer->setRendererV2( mRenderer->clone() );
}

void QgsRendererV2PropertiesDialog::onOK()
{
  apply();
  accept();
}

QgsSingleSymbolRendererV2* QgsRendererV2PropertiesDialog::rendererSingle()
{
  Q_ASSERT(mRenderer != NULL && mRenderer->type() == QgsFeatureRendererV2::RendererSingleSymbol);
  return static_cast<QgsSingleSymbolRendererV2*>(mRenderer);
}

QgsCategorizedSymbolRendererV2* QgsRendererV2PropertiesDialog::rendererCategorized()
{
  Q_ASSERT(mRenderer != NULL && mRenderer->type() == QgsFeatureRendererV2::RendererCategorizedSymbol);
  return static_cast<QgsCategorizedSymbolRendererV2*>(mRenderer);
}

QgsGraduatedSymbolRendererV2* QgsRendererV2PropertiesDialog::rendererGraduated()
{
  Q_ASSERT(mRenderer != NULL && mRenderer->type() == QgsFeatureRendererV2::RendererGraduatedSymbol);
  return static_cast<QgsGraduatedSymbolRendererV2*>(mRenderer);
}

void QgsRendererV2PropertiesDialog::changeSingleSymbol()
{
  // update symbol from the GUI
  rendererSingle()->setSymbol( mSingleSymbol->clone() );
}

void QgsRendererV2PropertiesDialog::updateRenderer()
{
  delete mRenderer;

  if (radSingleSymbol->isChecked())
    mRenderer = new QgsSingleSymbolRendererV2( mSingleSymbol->clone() );
  else if (radCategorized->isChecked())
    mRenderer = new QgsCategorizedSymbolRendererV2(QString(), QgsCategoryList());
  else if (radGraduated->isChecked())
    mRenderer = new QgsGraduatedSymbolRendererV2(QString(), QgsRangeList());
  else
    Q_ASSERT(false);
    
  updateUiFromRenderer();
}

void QgsRendererV2PropertiesDialog::updateUiFromRenderer()
{
  switch (mRenderer->type())
  {
    case QgsFeatureRendererV2::RendererSingleSymbol:
      radSingleSymbol->setChecked(true);
    
      stackedWidget->setCurrentWidget(pageSingleSymbol);
      //updateSingleSymbolIcon();
      break;
      
    case QgsFeatureRendererV2::RendererCategorizedSymbol:
      radCategorized->setChecked(true);
    
      stackedWidget->setCurrentWidget(pageCategorized);
      updateCategorizedSymbolIcon();

      disconnect(cboCategorizedColumn, SIGNAL(currentIndexChanged(int)), this, SLOT(categoryColumnChanged()));
      {
        QString attrName = rendererCategorized()->classAttribute();
        int idx = cboCategorizedColumn->findText(attrName, Qt::MatchExactly);
        cboCategorizedColumn->setCurrentIndex(idx >= 0 ? idx : 0);
      }
      connect(cboCategorizedColumn, SIGNAL(currentIndexChanged(int)), this, SLOT(categoryColumnChanged()));
      populateCategories();
      break;
  
    case QgsFeatureRendererV2::RendererGraduatedSymbol:
      radGraduated->setChecked(true);
      
      stackedWidget->setCurrentWidget(pageGraduated);
      updateGraduatedSymbolIcon();
      populateRanges();
      
      {
        // update UI from the graduated renderer (update combo boxes, view)
        QgsGraduatedSymbolRendererV2* r = rendererGraduated();
        if (r->mode() < cboGraduatedMode->count())
          cboGraduatedMode->setCurrentIndex( r->mode() );
        if (r->ranges().count())
          spinGraduatedClasses->setValue( r->ranges().count() );
        
        // set column
        //disconnect(cboGraduatedColumn, SIGNAL(currentIndexChanged(int)), this, SLOT(graduatedColumnChanged()));
        QString attrName = r->classAttribute();
        int idx = cboGraduatedColumn->findText(attrName, Qt::MatchExactly);
        cboGraduatedColumn->setCurrentIndex(idx >= 0 ? idx : 0);
        //connect(cboGraduatedColumn, SIGNAL(currentIndexChanged(int)), this, SLOT(graduatedColumnChanged()));
      }
      break;
      
    default: // custom renderer
      break;
  }
}

 
void QgsRendererV2PropertiesDialog::changeCategorizedSymbol()
{
  QgsSymbolV2SelectorDialog dlg(mCategorizedSymbol, mStyle, this);
  if (!dlg.exec())
    return;

  updateCategorizedSymbolIcon();
}

void QgsRendererV2PropertiesDialog::updateCategorizedSymbolIcon()
{
  QIcon icon = QgsSymbolLayerV2Utils::symbolPreviewIcon(mCategorizedSymbol, btnChangeCategorizedSymbol->iconSize());
  btnChangeCategorizedSymbol->setIcon(icon);
}

void QgsRendererV2PropertiesDialog::populateCategories()
{
  QStandardItemModel* m = qobject_cast<QStandardItemModel*>(viewCategories->model());
  m->clear();
  
  QStringList labels;
  labels << "Value" << "Label";
  m->setHorizontalHeaderLabels(labels);
  
  QSize iconSize(16,16);
  
  QgsCategorizedSymbolRendererV2* r = rendererCategorized();
  int i, count = r->categories().count();
  
  // TODO: sort?? utils.sortVariantList(keys);
  
  for (i = 0; i < count; i++)
  {
    const QgsRendererCategoryV2& cat = r->categories()[i];
    QVariant k = cat.value();
    
    QIcon icon = QgsSymbolLayerV2Utils::symbolPreviewIcon(cat.symbol(), iconSize);
    QStandardItem* item = new QStandardItem(icon, k.toString());
    item->setData(k); // set attribute value as user role
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    
    QList<QStandardItem *> list;
    list << item << new QStandardItem(cat.label());
    m->appendRow( list );
  }
}

void QgsRendererV2PropertiesDialog::populateColumns()
{
  cboCategorizedColumn->clear();
  const QgsFieldMap& flds = mLayer->dataProvider()->fields();
  QgsFieldMap::ConstIterator it = flds.begin();
  for ( ; it != flds.end(); ++it)
  {
    cboCategorizedColumn->addItem(it->name());
    
    if (it->type() == QVariant::Double || it->type() == QVariant::Int)
      cboGraduatedColumn->addItem(it->name());
  }
}


void QgsRendererV2PropertiesDialog::categoryColumnChanged()
{
  deleteAllCategories();
}

void QgsRendererV2PropertiesDialog::categoriesDoubleClicked(const QModelIndex & idx)
{
  if (idx.isValid() && idx.column() == 0)
    changeCategorySymbol();
}

void QgsRendererV2PropertiesDialog::changeCategorySymbol()
{
  QVariant k = currentCategory();
  if (!k.isValid())
    return;
  
  QgsCategorizedSymbolRendererV2* r = rendererCategorized();
  int catIdx = r->categoryIndexForValue(k);
  QgsSymbolV2* newSymbol = r->categories()[catIdx].symbol()->clone();
  
  QgsSymbolV2SelectorDialog dlg(newSymbol, mStyle, this);
  if (!dlg.exec())
  {
    delete newSymbol;
    return;
  }
  
  r->updateCategorySymbol(catIdx, newSymbol);
  
  populateCategories();
}

void createCategories(QgsCategoryList& cats, QList<QVariant>& values, QgsSymbolV2* symbol, QgsVectorColorRampV2* ramp)
{
  // sort the categories first
  // TODO: sortVariantList(values);
  
  int num = values.count();
  
  for (int i = 0; i < num; i++)
  {
    QVariant value = values[i];
    double x = i / (double) num;
    QgsSymbolV2* newSymbol = symbol->clone();
    newSymbol->setColor( ramp->color(x) );
    
    cats.append( QgsRendererCategoryV2(value, newSymbol, value.toString()) );
  }
  
}

void QgsRendererV2PropertiesDialog::addCategories()
{
  QString attrName = cboCategorizedColumn->currentText();
  int idx = QgsFeatureRendererV2::fieldNameIndex(mLayer->pendingFields(), attrName);
  QList<QVariant> unique_vals;
  mLayer->dataProvider()->uniqueValues(idx, unique_vals);
  
  //DlgAddCategories dlg(mStyle, createDefaultSymbol(), unique_vals, this);
  //if (!dlg.exec())
  //  return;
  
  QgsVectorColorRampV2* ramp = mStyle->colorRamp( cboCategorizedColorRamp->currentText() );
  
  QgsCategoryList cats;
  ::createCategories(cats, unique_vals, mCategorizedSymbol, ramp );
  
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
  mRenderer = new QgsCategorizedSymbolRendererV2(attrName, cats);
  
  populateCategories();
}

int QgsRendererV2PropertiesDialog::currentCategoryRow()
{
  QModelIndex idx = viewCategories->selectionModel()->currentIndex();
  if (!idx.isValid())
    return -1;
  return idx.row();
}

QVariant QgsRendererV2PropertiesDialog::currentCategory()
{
  int row = currentCategoryRow();
  if (row == -1)
    return QVariant();
  QStandardItemModel* m = qobject_cast<QStandardItemModel*>(viewCategories->model());
  return m->item(row)->data();
}

void QgsRendererV2PropertiesDialog::deleteCategory()
{
  QVariant k = currentCategory();
  if (!k.isValid())
    return;
  
  QgsCategorizedSymbolRendererV2* r = rendererCategorized();
  const QgsCategoryList& cats = r->categories();
  QgsCategoryList::ConstIterator it = cats.begin();
  
  for ( ; it != cats.end(); ++it)
  {
    if (k.toString() == it->value().toString())
    {
      int idx = r->categoryIndexForValue(k);
      r->deleteCategory(idx);
    }
  }
  
  populateCategories();
}

void QgsRendererV2PropertiesDialog::deleteAllCategories()
{
  rendererCategorized()->deleteAllCategories();
  populateCategories();
}

//////


void QgsRendererV2PropertiesDialog::populateColorRamps()
{
  QSize rampIconSize(50,16);
  cboGraduatedColorRamp->setIconSize(rampIconSize);
  cboCategorizedColorRamp->setIconSize(rampIconSize);

  QStringList rampNames = mStyle->colorRampNames();
  for (QStringList::iterator it = rampNames.begin(); it != rampNames.end(); ++it)
  {
    QgsVectorColorRampV2* ramp = mStyle->colorRamp(*it);
    QIcon icon = QgsSymbolLayerV2Utils::colorRampPreviewIcon(ramp, rampIconSize);
    cboGraduatedColorRamp->addItem(icon, *it);
    cboCategorizedColorRamp->addItem(icon, *it);
    delete ramp;
  }
}

void QgsRendererV2PropertiesDialog::classifyGraduated()
{
  QString attrName = cboGraduatedColumn->currentText();
  
  int classes = spinGraduatedClasses->value();
  
  QgsVectorColorRampV2* ramp = mStyle->colorRamp( cboGraduatedColorRamp->currentText() );
  
  QgsGraduatedSymbolRendererV2::Mode mode;
  if (cboGraduatedMode->currentIndex() == 0)
    mode = QgsGraduatedSymbolRendererV2::EqualInterval;
  else
    mode = QgsGraduatedSymbolRendererV2::Quantile;
  
  // create and set new renderer
  QgsGraduatedSymbolRendererV2* r = QgsGraduatedSymbolRendererV2::createRenderer(
      mLayer, attrName, classes, mode, mGraduatedSymbol, ramp);
  r->setMode(mode);
  
  delete mRenderer;
  mRenderer = r;
  
  populateRanges();
}

void QgsRendererV2PropertiesDialog::changeGraduatedSymbol()
{
  QgsSymbolV2SelectorDialog dlg(mGraduatedSymbol, mStyle, this);
  if (!dlg.exec())
    return;
  
  updateGraduatedSymbolIcon();
}

void QgsRendererV2PropertiesDialog::updateGraduatedSymbolIcon()
{
  QIcon icon = QgsSymbolLayerV2Utils::symbolPreviewIcon(mGraduatedSymbol, btnChangeGraduatedSymbol->iconSize());
  btnChangeGraduatedSymbol->setIcon(icon);
}

void QgsRendererV2PropertiesDialog::populateRanges()
{

  QStandardItemModel* m = qobject_cast<QStandardItemModel*>(viewGraduated->model());
  m->clear();
  
  QStringList labels;
  labels << "Range" << "Label";
  m->setHorizontalHeaderLabels(labels);
  
  QSize iconSize(16,16);
  
  QgsGraduatedSymbolRendererV2* r = rendererGraduated();
  int i, count = r->ranges().count();
  
  for (i = 0; i < count; i++)
  {
    const QgsRendererRangeV2& range = r->ranges()[i];
    QString rangeStr = QString::number(range.lowerValue(),'f',4) + " - " + QString::number(range.upperValue(),'f',4);
    
    QIcon icon = QgsSymbolLayerV2Utils::symbolPreviewIcon(range.symbol(), iconSize);
    QStandardItem* item = new QStandardItem(icon, rangeStr);
    //item->setData(k); // set attribute value as user role
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    
    QList<QStandardItem *> list;
    list << item << new QStandardItem(range.label());
    m->appendRow( list );
  }

  // make sure that the "range" column has visible context
  viewGraduated->resizeColumnToContents(0);
}

/*
int QgsRendererV2PropertiesDialog::currentRangeRow()
{
  QModelIndex idx = viewGraduated->selectionModel()->currentIndex();
  if (!idx.isValid())
    return -1;
  return idx.row();
}
*/

void QgsRendererV2PropertiesDialog::rangesDoubleClicked(const QModelIndex & idx)
{
  if (idx.isValid() && idx.column() == 0)
    changeRangeSymbol(idx.row());
}

void QgsRendererV2PropertiesDialog::changeRangeSymbol(int rangeIdx)
{

  QgsGraduatedSymbolRendererV2* r = rendererGraduated();
  QgsSymbolV2* newSymbol = r->ranges()[rangeIdx].symbol()->clone();

  QgsSymbolV2SelectorDialog dlg(newSymbol, mStyle, this);
  if (!dlg.exec())
  {
    delete newSymbol;
    return;
  }

  r->updateRangeSymbol(rangeIdx, newSymbol);

  populateRanges();
}


void QgsRendererV2PropertiesDialog::symbolLevels()
{
  QgsSymbolV2List symbols = mRenderer->symbols();

  QgsSymbolLevelsV2Dialog dlg(symbols, mRenderer->usingSymbolLevels(), this);
  if (dlg.exec())
  {
    mRenderer->setUsingSymbolLevels( dlg.usingLevels() );
  }
}
