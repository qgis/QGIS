
// $Id$

#include <QIcon>
#include <QPixmap>
#include <QTableWidgetItem>

#include "qgspgutil.h"
#include "spit_icons.h"

#include "qgseditreservedwordsdialog.h"
QgsEditReservedWordsDialog::QgsEditReservedWordsDialog(QWidget *parent, Qt::WFlags fl)
  : QDialog(parent, fl)
{
  setupUi(this);
  // buggered if I know why setting the columns and their labels in
  // designer doesn't last until here, hence it's done manually...
  lvColumns->setColumnCount(3);
  QStringList headerText;
  headerText << tr("Status") << tr("Column Name") << tr("Index");
  lvColumns->setHorizontalHeaderLabels(headerText);
  lvColumns->resizeColumnsToContents();
}

QgsEditReservedWordsDialog::~QgsEditReservedWordsDialog()
{
}
void QgsEditReservedWordsDialog::setReservedWords(const QStringList &words)
{
  lstReservedWords->addItems(words);
}
void QgsEditReservedWordsDialog::checkWord(QTableWidgetItem* item)
{
  // Column 1 is the one that the user can edit. However we can get
  // itemChanged signals from any item in the table, so ignore other
  // columns.
  if (lvColumns->column(item) != 1)
    return;

  QgsPgUtil *pgu = QgsPgUtil::instance();
  int row = lvColumns->row(item);

  // Column 0 is the one with the tick/cross pixmap
  if(pgu->isReserved(item->text()))
  {
    lvColumns->item(row, 0)->setIcon(QIcon(QPixmap(icon_reserved)));
  }
  else
  {
    lvColumns->item(row, 0)->setIcon(QIcon(QPixmap(icon_ok)));
  }
}
void QgsEditReservedWordsDialog::addColumn(QString column, bool isReserved, int index)
{
  QString indexNumber;
  indexNumber = indexNumber.setNum(index);

  QTableWidgetItem *reservedItem = new QTableWidgetItem();
  QTableWidgetItem *nameItem = new QTableWidgetItem(column);
  QTableWidgetItem *indexItem = new QTableWidgetItem(indexNumber);

  // Two of the columns shouldn't be editable
  reservedItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
  indexItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

  // Set the appropriate icon
  if(isReserved)
  {
    reservedItem->setIcon(QIcon(QPixmap(icon_reserved)));
  }
  else
  {
    reservedItem->setIcon(QIcon(QPixmap(icon_ok)));
  }

  // Insert a new row into the table
  int rows = lvColumns->rowCount();
  lvColumns->insertRow(rows);
  lvColumns->setItem(rows, 0, reservedItem);
  lvColumns->setItem(rows, 1, nameItem);
  lvColumns->setItem(rows, 2, indexItem);
}
QStringList QgsEditReservedWordsDialog::columnNames()
{
  // Extract and return the renamed columns from the table
  QStringList cols;
  lvColumns->sortItems(2);

  for (int i = 0; i < lvColumns->rowCount(); ++i)
  {
    cols << lvColumns->item(0, 2)->text();
  }
  return QStringList(cols);
}
void QgsEditReservedWordsDialog::setDescription(const QString &description)
{
  txtExplanation->setPlainText(description);
}

