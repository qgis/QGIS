
#include "qgssymbollevelsv2dialog.h"

#include "qgssymbollayerv2utils.h"
#include "qgssymbolv2.h"

#include <QTableWidgetItem>

QgsSymbolLevelsV2Dialog::QgsSymbolLevelsV2Dialog(QgsSymbolV2List symbols, QgsSymbolV2LevelOrder levels, QWidget* parent)
 : QDialog(parent), mSymbols(symbols), mLevels(levels)
{
  setupUi(this);

  chkEnable->setChecked( !levels.isEmpty() );

  connect(chkEnable, SIGNAL(clicked()), this, SLOT(updateUi()));

  int maxLayers = 0;
  tableLevels->setRowCount(symbols.count());
  for (int i = 0; i < symbols.count(); i++)
  {
    QgsSymbolV2* sym = symbols[i];

    // set icons for the rows
    QIcon icon = QgsSymbolLayerV2Utils::symbolPreviewIcon(sym, QSize(16,16));
    tableLevels->setVerticalHeaderItem(i, new QTableWidgetItem(icon, QString()) );

    // find out max. number of layers per symbol
    int layers = sym->symbolLayerCount();
    if (layers > maxLayers)
      maxLayers = layers;
  }

  tableLevels->setColumnCount(maxLayers);
  for (int i = 0; i < maxLayers; i++)
  {
    QString name = QString("Layer %1").arg(i);
    tableLevels->setHorizontalHeaderItem(i, new QTableWidgetItem(name));
  }

  mMaxLayers = maxLayers;

  updateUi();

  connect(tableLevels, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(updateLevels(QTableWidgetItem*)));
  populateTable();
}

void QgsSymbolLevelsV2Dialog::populateTable()
{
  if (mLevels.isEmpty())
    return;

  disconnect(tableLevels, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(updateLevels(QTableWidgetItem*)));

  for (int row = 0; row < mSymbols.count(); row++)
  {
    QgsSymbolV2* sym = mSymbols[row];
    for (int layer = 0; layer < mMaxLayers; layer++)
    {
      QTableWidgetItem* item;
      if (layer >= sym->symbolLayerCount())
      {
        item = new QTableWidgetItem();
        item->setFlags(Qt::ItemFlags());
      }
      else
      {
        item = new QTableWidgetItem( QString::number(levelForSymbolLayer(sym, layer)) );
      }
      tableLevels->setItem(row, layer, item);
    }
  }

  connect(tableLevels, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(updateLevels(QTableWidgetItem*)));

}

void QgsSymbolLevelsV2Dialog::updateUi()
{
  if (chkEnable->isChecked())
  {
    if (mLevels.isEmpty())
      setDefaultLevels();
    populateTable();
  }
  else
    mLevels.clear();

  tableLevels->setEnabled(chkEnable->isChecked());
}

void QgsSymbolLevelsV2Dialog::setDefaultLevels()
{
  mLevels.clear();
  for (int col = 0; col < tableLevels->columnCount(); col++)
  {
    QgsSymbolV2Level level;
    for (int i = 0; i < mSymbols.count(); i++)
    {
      QgsSymbolV2* sym = mSymbols[i];
      if (col < sym->symbolLayerCount())
        level.append(QgsSymbolV2LevelItem(sym, col));
    }
    mLevels.append(level);
  }
}

int QgsSymbolLevelsV2Dialog::levelForSymbolLayer(QgsSymbolV2* sym, int layer)
{
  for (int l = 0; l < mLevels.count(); l++)
  {
    QgsSymbolV2Level& level = mLevels[l];
    for (int i = 0; i < level.count(); i++)
    {
      QgsSymbolV2LevelItem& item = level[i];
      if (item.symbol() == sym && item.layer() == layer)
        return l;
    }
  }
  return -1;
}

void QgsSymbolLevelsV2Dialog::updateLevels(QTableWidgetItem* item)
{
  int num = item->text().toInt();
  if (num > 100)
  {
    item->setText("0");
    return;
  }

  mLevels.clear();
  for (int col = 0; col < tableLevels->columnCount(); col++)
  {
    for (int row = 0; row < mSymbols.count(); row++)
    {
      QgsSymbolV2* sym = mSymbols[row];
      int level = tableLevels->item(row,col)->text().toInt();
      QgsSymbolV2LevelItem item(sym,col);
      while (level >= mLevels.count()) // append new empty levels
        mLevels.append( QgsSymbolV2Level() );
      mLevels[level].append(item);
    }
  }
}
