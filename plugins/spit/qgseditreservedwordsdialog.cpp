#include <qlistbox.h>
#include <qpixmap.h>
#include <qlistview.h>
#include "../../src/qgspgutil.h"
#include "spit_icons.h"

#include "qgseditreservedwordsdialog.h"
QgsEditReservedWordsDialog::QgsEditReservedWordsDialog(QWidget *parent, const char *name)
  : QgsEditReservedWordsBase(parent, name)
{
  // set focus indicator to span all columns
 lvColumns->setAllColumnsShowFocus(true);
}

QgsEditReservedWordsDialog::~QgsEditReservedWordsDialog()
{
}
void QgsEditReservedWordsDialog::setReservedWords(const QStringList &words)
{
  lstReservedWords->insertStringList(words);
}
void QgsEditReservedWordsDialog::checkWord(QListViewItem *lvi, int col, const QString &word)
{
  QgsPgUtil *pgu = QgsPgUtil::instance();
  if(pgu->isReserved(word))
  {
    lvi->setPixmap(0, QPixmap(icon_reserved));
  }
  else
  {
    lvi->setPixmap(0, QPixmap(icon_ok));
  }

}
void QgsEditReservedWordsDialog::addColumn(QString column, bool isReserved, int index)
{
  QString indexNumber;
  indexNumber = indexNumber.setNum(index);
  QListViewItem *lvi = new QListViewItem(lvColumns,"",column, indexNumber);
//  lvi-setText(1, column);
  lvi->setRenameEnabled(1, true);
  if(isReserved)
  {
    lvi->setPixmap(0, QPixmap(icon_reserved));
  }
  else
  {
    lvi->setPixmap(0, QPixmap(icon_ok));
  }
}
void QgsEditReservedWordsDialog::editWord(QListViewItem *lvi)
{
  if(lvi)
  {
    lvi->startRename(1);
  }

}
QStringList QgsEditReservedWordsDialog::columnNames()
{
  QStringList cols;
  lvColumns->setSorting(2);
  lvColumns->sort();
  QListViewItem *lvi = lvColumns->firstChild();
  while(lvi)
  {
    cols << lvi->text(1); 
    lvi = lvi->nextSibling();
  }
  return QStringList(cols);
}
