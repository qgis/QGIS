#include "qgsgraduatedsymbolrendererv2widget.h"

#include "qgssymbolv2.h"
#include "qgssymbollayerv2utils.h"
#include "qgsvectorcolorrampv2.h"
#include "qgsstylev2.h"

#include "qgsvectorlayer.h"

#include "qgsgraduatedsymbolrendererv2.h"

#include "qgssymbolv2selectordialog.h"

#include <QMessageBox>
#include <QStandardItemModel>
#include <QStandardItem>


QgsRendererV2Widget* QgsGraduatedSymbolRendererV2Widget::create(QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer)
{
  return new QgsGraduatedSymbolRendererV2Widget(layer, style, renderer);
}

QgsGraduatedSymbolRendererV2Widget::QgsGraduatedSymbolRendererV2Widget(QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer)
 : QgsRendererV2Widget(layer, style)
{

  // try to recognize the previous renderer
  // (null renderer means "no previous renderer")
  if (!renderer || renderer->type() != "graduatedSymbol")
  {
    // we're not going to use it - so let's delete the renderer
    delete renderer;

    mRenderer = new QgsGraduatedSymbolRendererV2("", QgsRangeList());
  }
  else
  {
    mRenderer = static_cast<QgsGraduatedSymbolRendererV2*>(renderer);
  }

  // setup user interface
  setupUi(this);

  populateColumns();
  populateColorRamps();
  QStandardItemModel* mg = new QStandardItemModel(this);
  QStringList labels;
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

QgsGraduatedSymbolRendererV2Widget::~QgsGraduatedSymbolRendererV2Widget()
{
  delete mRenderer;
}

QgsFeatureRendererV2* QgsGraduatedSymbolRendererV2Widget::renderer()
{
  return mRenderer;
}


void QgsGraduatedSymbolRendererV2Widget::updateUiFromRenderer()
{

  updateGraduatedSymbolIcon();
  populateRanges();

  // update UI from the graduated renderer (update combo boxes, view)
  if (mRenderer->mode() < cboGraduatedMode->count())
    cboGraduatedMode->setCurrentIndex( mRenderer->mode() );
  if (mRenderer->ranges().count())
    spinGraduatedClasses->setValue( mRenderer->ranges().count() );

  // set column
  //disconnect(cboGraduatedColumn, SIGNAL(currentIndexChanged(int)), this, SLOT(graduatedColumnChanged()));
  QString attrName = mRenderer->classAttribute();
  int idx = cboGraduatedColumn->findText(attrName, Qt::MatchExactly);
  cboGraduatedColumn->setCurrentIndex(idx >= 0 ? idx : 0);
  //connect(cboGraduatedColumn, SIGNAL(currentIndexChanged(int)), this, SLOT(graduatedColumnChanged()));

  // set source symbol
  if (mRenderer->sourceSymbol())
  {
    delete mGraduatedSymbol;
    mGraduatedSymbol = mRenderer->sourceSymbol()->clone();
    updateGraduatedSymbolIcon();
  }

  // set source color ramp
  if (mRenderer->sourceColorRamp())
  {
    QSize rampIconSize(50,16);
    QIcon icon = QgsSymbolLayerV2Utils::colorRampPreviewIcon(mRenderer->sourceColorRamp(), rampIconSize);
    if (cboGraduatedColorRamp->itemText(0) == "[source]")
      cboGraduatedColorRamp->setItemIcon(0, icon);
    else
      cboGraduatedColorRamp->insertItem(0, icon, "[source]");
    cboGraduatedColorRamp->setCurrentIndex(0);
  }

}



void QgsGraduatedSymbolRendererV2Widget::populateColumns()
{
  cboGraduatedColumn->clear();
  const QgsFieldMap& flds = mLayer->pendingFields();
  QgsFieldMap::ConstIterator it = flds.begin();
  for ( ; it != flds.end(); ++it)
  {
    if (it->type() == QVariant::Double || it->type() == QVariant::Int)
      cboGraduatedColumn->addItem(it->name());
  }
}


void QgsGraduatedSymbolRendererV2Widget::populateColorRamps()
{
  QSize rampIconSize(50,16);
  cboGraduatedColorRamp->setIconSize(rampIconSize);

  QStringList rampNames = mStyle->colorRampNames();
  for (QStringList::iterator it = rampNames.begin(); it != rampNames.end(); ++it)
  {
    QgsVectorColorRampV2* ramp = mStyle->colorRamp(*it);
    QIcon icon = QgsSymbolLayerV2Utils::colorRampPreviewIcon(ramp, rampIconSize);
    cboGraduatedColorRamp->addItem(icon, *it);
    delete ramp;
  }
}



void QgsGraduatedSymbolRendererV2Widget::classifyGraduated()
{
  QString attrName = cboGraduatedColumn->currentText();

  int classes = spinGraduatedClasses->value();

  QgsVectorColorRampV2* ramp = NULL;
  QString rampName = cboGraduatedColorRamp->currentText();
  if (rampName == "[source]")
    ramp = mRenderer->sourceColorRamp()->clone();
  else
    ramp = mStyle->colorRamp( rampName );

  if ( ramp == NULL )
  {
    if ( cboGraduatedColorRamp->count() == 0 )
      QMessageBox::critical( this, tr("Error"), tr("There are no available color ramps. You can add them in Style Manager.") );
    else
      QMessageBox::critical( this, tr("Error"), tr("The selected color ramp is not available.") );
    return;
  }

  QgsGraduatedSymbolRendererV2::Mode mode;
  if (cboGraduatedMode->currentIndex() == 0)
    mode = QgsGraduatedSymbolRendererV2::EqualInterval;
  else
    mode = QgsGraduatedSymbolRendererV2::Quantile;

  // create and set new renderer
  delete mRenderer;
  mRenderer = QgsGraduatedSymbolRendererV2::createRenderer(
      mLayer, attrName, classes, mode, mGraduatedSymbol, ramp);

  populateRanges();
}

void QgsGraduatedSymbolRendererV2Widget::changeGraduatedSymbol()
{
  QgsSymbolV2SelectorDialog dlg(mGraduatedSymbol, mStyle, this);
  if (!dlg.exec())
    return;

  updateGraduatedSymbolIcon();
}

void QgsGraduatedSymbolRendererV2Widget::updateGraduatedSymbolIcon()
{
  QIcon icon = QgsSymbolLayerV2Utils::symbolPreviewIcon(mGraduatedSymbol, btnChangeGraduatedSymbol->iconSize());
  btnChangeGraduatedSymbol->setIcon(icon);
}


void QgsGraduatedSymbolRendererV2Widget::populateRanges()
{

  QStandardItemModel* m = qobject_cast<QStandardItemModel*>(viewGraduated->model());
  m->clear();

  QStringList labels;
  labels << "Range" << "Label";
  m->setHorizontalHeaderLabels(labels);

  QSize iconSize(16,16);

  int i, count = mRenderer->ranges().count();

  for (i = 0; i < count; i++)
  {
    const QgsRendererRangeV2& range = mRenderer->ranges()[i];
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

void QgsGraduatedSymbolRendererV2Widget::rangesDoubleClicked(const QModelIndex & idx)
{
  if (idx.isValid() && idx.column() == 0)
    changeRangeSymbol(idx.row());
}

void QgsGraduatedSymbolRendererV2Widget::changeRangeSymbol(int rangeIdx)
{

  QgsSymbolV2* newSymbol = mRenderer->ranges()[rangeIdx].symbol()->clone();

  QgsSymbolV2SelectorDialog dlg(newSymbol, mStyle, this);
  if (!dlg.exec())
  {
    delete newSymbol;
    return;
  }

  mRenderer->updateRangeSymbol(rangeIdx, newSymbol);

  populateRanges();
}

