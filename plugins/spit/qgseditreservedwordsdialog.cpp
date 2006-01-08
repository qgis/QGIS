#include <q3listbox.h>
#include <qpixmap.h>
#include <q3listview.h>
#include <q3textedit.h>
#include "qgspgutil.h"
#include "spit_icons.h"

#include "qgseditreservedwordsdialog.h"
QgsEditReservedWordsDialog::QgsEditReservedWordsDialog(QWidget *parent, const char *name)
  : QDialog(parent, name)
{
  setupUi(this);
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
void QgsEditReservedWordsDialog::checkWord(Q3ListViewItem *lvi, int col, const QString &word)
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
  Q3ListViewItem *lvi = new Q3ListViewItem(lvColumns,"",column, indexNumber);
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
void QgsEditReservedWordsDialog::editWord(Q3ListViewItem *lvi)
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
  Q3ListViewItem *lvi = lvColumns->firstChild();
  while(lvi)
  {
    cols << lvi->text(1); 
    lvi = lvi->nextSibling();
  }
  return QStringList(cols);
}
void QgsEditReservedWordsDialog::setDescription(const QString &description)
{
  txtExplanation->setText(description);
}

