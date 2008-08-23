/***************************************************************************
                          qgsattributetable.cpp  -  description
                             -------------------
    begin                : Sat Nov 23 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc dot com
       Romans 3:23=>Romans 6:23=>Romans 5:8=>Romans 10:9,10=>Romans 12
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */
#include <QLineEdit>
#include <QValidator>

#include "qgsattributetable.h"
#include "qgsfield.h"
#include "qgslogger.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"

#include <QApplication>
#include <QClipboard>
#include <QHeaderView>
#include <QKeyEvent>
#include <QMenu>


QgsAttributeTableItemDelegate::QgsAttributeTableItemDelegate(QgsAttributeTable *table, QObject *parent)
  : mTable(table), QItemDelegate(parent)
{
}

QWidget *QgsAttributeTableItemDelegate::createEditor(
  QWidget *parent,
  const QStyleOptionViewItem &option,
  const QModelIndex &index ) const
{
  QWidget *editor = QItemDelegate::createEditor(parent, option, index);
  QLineEdit *le = dynamic_cast<QLineEdit*>(editor);
  if (!le)
    return editor;

  int col = index.column();
  QTableWidgetItem *twi = mTable->horizontalHeaderItem(col);
  if(!twi)
  {
    QgsDebugMsg( QString("horizontalHeaderItem %1 not found").arg(col) );
    return editor;
  }

  int type = twi->data(QgsAttributeTable::AttributeType).toInt();
  if( type==QVariant::Int )
  {
    le->setValidator( new QIntValidator(le) );
  }
  else if( type==QVariant::Double )
  {
    le->setValidator( new QDoubleValidator(le) );
  }

  return editor;
}


QgsAttributeTable::QgsAttributeTable(QWidget * parent) :
        QTableWidget(parent),
        lockKeyPressed(false),
        mEditable(false),
        mEdited(false),
        mActionPopup(0),
        mPreviousSortIndicatorColumn(-1)
{
  QFont f(font());
  f.setFamily("Helvetica");
  f.setPointSize(9);
  setFont(f);
  mDelegate = new QgsAttributeTableItemDelegate(this);
  setItemDelegate(mDelegate);
  setSelectionBehavior(QAbstractItemView::SelectRows);
  connect(this, SIGNAL(itemSelectionChanged()), this, SLOT(handleChangedSelections()));
  connect(horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(columnClicked(int)));
  connect(verticalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(rowClicked(int)));
  setFocus();
}

QgsAttributeTable::~QgsAttributeTable()
{
  delete mActionPopup;
  delete mDelegate;
}

void QgsAttributeTable::setReadOnly(bool b)
{
  setEditTriggers(b ? QAbstractItemView::NoEditTriggers :
    QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
  if(!b) {
    setColumnReadOnly(0, true);
  }
}

void QgsAttributeTable::setColumnReadOnly(int col, bool ro)
{
  for (int i = 0; i < rowCount(); ++i)
  {
    QTableWidgetItem *twi = item(i, col);
    twi->setFlags(ro ? twi->flags() & ~Qt::ItemIsEditable : twi->flags() | Qt::ItemIsEditable);
  }
}

void QgsAttributeTable::columnClicked(int col)
{
  QApplication::setOverrideCursor(Qt::WaitCursor);

  //store the ids of the selected rows in a list
  QList<int> idsOfSelected;
  QList<QTableWidgetSelectionRange> selection = selectedRanges();
  for (int i = 0; i < selection.count(); i++)
  {
    for (int j = selection.at(i).topRow(); j <= selection.at(i).bottomRow(); j++)
    {
      idsOfSelected.append(item(j, 0)->text().toInt());
    }
  }

  QHeaderView *header = horizontalHeader();
  if (!header->isSortIndicatorShown())
  {
    header->setSortIndicatorShown(true);
    header->setSortIndicator(col, Qt::AscendingOrder);
  }
  if (col != mPreviousSortIndicatorColumn)
  {
    // Workaround for QTableView sortIndicator displayed in wrong direction
    header->setSortIndicator(col, header->sortIndicatorOrder() == Qt::AscendingOrder ?
      Qt::DescendingOrder : Qt::AscendingOrder);
  }
  mPreviousSortIndicatorColumn = col;
  sortColumn(col, header->sortIndicatorOrder() == Qt::DescendingOrder);

  //clear and rebuild rowIdMap. Overwrite sortColumn later and sort rowIdMap there
  rowIdMap.clear();
  int id;
  for (int i = 0; i < rowCount(); i++)
  {
    id = item(i, 0)->text().toInt();
    rowIdMap.insert(id, i);
  }

  disconnect(this, SIGNAL(itemSelectionChanged()), this, SLOT(handleChangedSelections()));
  clearSelection();

  //select the rows again after sorting

  QList < int >::iterator it;
  for (it = idsOfSelected.begin(); it != idsOfSelected.end(); ++it)
  {
    selectRowWithId((*it));
  }
  connect(this, SIGNAL(itemSelectionChanged()), this, SLOT(handleChangedSelections()));

  QApplication::restoreOverrideCursor();
}

void QgsAttributeTable::keyPressEvent(QKeyEvent * ev)
{
  if (ev->key() == Qt::Key_Control || ev->key() == Qt::Key_Shift)
  {
    lockKeyPressed = true;
  }
}

void QgsAttributeTable::keyReleaseEvent(QKeyEvent * ev)
{
  if (ev->key() == Qt::Key_Control || ev->key() == Qt::Key_Shift)
  {
    lockKeyPressed = false;
  }
}

void QgsAttributeTable::handleChangedSelections()
{
  emit selectionRemoved(false);

  QList<QTableWidgetSelectionRange> selectedItemRanges = selectedRanges();
  QList<QTableWidgetSelectionRange>::const_iterator range_it = selectedItemRanges.constBegin();
  for (; range_it != selectedItemRanges.constEnd(); ++range_it)
  { 
    for (int index = range_it->topRow(); index <= range_it->bottomRow(); index++)
    {
      emit selected(item(index, 0)->text().toInt(), false);
    }
  }

  //don't send the signal repaintRequested() from here
  //but in mouseReleaseEvent() and rowClicked(int)
  //todo: don't repaint in case of double clicks
}

void QgsAttributeTable::insertFeatureId(int id, int row)
{
  rowIdMap.insert(id, row);
}

void QgsAttributeTable::selectRowWithId(int id)
{
  QMap < int, int >::iterator it = rowIdMap.find(id);
  setRangeSelected(QTableWidgetSelectionRange(it.value(), 0, it.value(), columnCount()-1), true);
}

void QgsAttributeTable::sortColumn(int col, bool ascending)
{
  int type = horizontalHeaderItem(col)->data(QgsAttributeTable::AttributeType).toInt();
  qsort(0, rowCount() - 1, col, ascending, type!=QVariant::Int && type!=QVariant::Double);
}


/**
  XXX Doesn't QString have something ilke this already?
*/
int QgsAttributeTable::compareItems(QString s1, QString s2, bool ascending, bool alphanumeric)
{
  if (alphanumeric)
  {
    if (s1 > s2)
    {
      if (ascending)
      {
        return 1;
      }
      else
      {
        return -1;
      }
    }
    else if (s1 < s2)
    {
      if (ascending)
      {
        return -1;
      }
      else
      {
        return 1;
      }
    }
    else if (s1 == s2)
    {
      return 0;
    }
  }
  else                        //numeric
  {
    double d1 = s1.toDouble();
    double d2 = s2.toDouble();
    if (d1 > d2)
    {
      if (ascending)
      {
        return 1;
      }
      else
      {
        return -1;
      }
    }
    else if (d1 < d2)
    {
      if (ascending)
      {
        return -1;
      }
      else
      {
        return 1;
      }
    }
    else if (d1 == d2)
    {
      return 0;
    }
  }

  return 0;                     // XXX has to return something; is this reasonable?
}

void QgsAttributeTable::qsort(int lower, int upper, int col, bool ascending, bool alphanumeric)
{
  int i, j;
  QString v;
  if (upper > lower)
  {
    //chose a random element (this avoids n^2 worst case)
    int element = int ( (double)rand() / (double)RAND_MAX * (upper - lower) + lower);
    swapRows(element, upper);
    v = item(upper, col)->text();
    i = lower - 1;
    j = upper;
    for (;;)
    {
      while (compareItems(item(++i, col)->text(), v, ascending, alphanumeric) == -1)
        ;
      while (compareItems(item(--j, col)->text(), v, ascending, alphanumeric) == 1 && j > 0); //make sure that j does not get negative
      if (i >= j)
      {
        break;
      }
      swapRows(i, j);
    }
    swapRows(i, upper);
    qsort(lower, i - 1, col, ascending, alphanumeric);
    qsort(i + 1, upper, col, ascending, alphanumeric);
  }
}

void QgsAttributeTable::swapRows(int row1, int row2)
{
  for (int col = 0; col < columnCount(); col++)
  {
    QTableWidgetItem *item = takeItem(row1, col);
    setItem(row1, col, takeItem(row2, col));
    setItem(row2, col, item);
  }
}

void QgsAttributeTable::contextMenuEvent(QContextMenuEvent *event)
{
  const QPoint& pos = event->globalPos();
  int row = rowAt(pos.x());
  int col = columnAt(pos.y());

  // Duplication of code in qgsidentufyresults.cpp. Consider placing
  // in a seperate class
  if (mActionPopup == 0)
  {
    mActionPopup = new QMenu();
    mActionPopup->addAction( tr("Run action") );
    mActionPopup->addSeparator();

    QgsAttributeAction::aIter	iter = mActions.begin();
    for (int j = 0; iter != mActions.end(); ++iter, ++j)
    {
      QAction* a = mActionPopup->addAction(iter->name());
      // The menu action stores an integer that is used later on to
      // associate an menu action with an actual qgis action.
      a->setData(QVariant::fromValue(j));
    }
    connect(mActionPopup, SIGNAL(triggered(QAction*)),
            this, SLOT(popupItemSelected(QAction*)));
  }

  // Get and store the attribute values and their column names are
  // these are needed for substituting into the actions if the user
  // chooses one.
  mActionValues.clear();

  for (int i = 0; i < columnCount(); ++i)
  {
    if (row >= 0) //prevent crash if row is negative, see ticket #1149
    {
      mActionValues.push_back(
        std::make_pair(
          horizontalHeaderItem( i )->text(),
          item( row, i )->text() ) );
    }
  }
  // The item that was clicked on, stored as an index into the
  // mActionValues vector.
  mClickedOnValue = col;

  if (mActions.size() > 0)
    mActionPopup->popup(pos);  
}

void QgsAttributeTable::popupItemSelected(QAction* menuAction)
{
  int id = menuAction->data().toInt();
  mActions.doAction(id, mActionValues, mClickedOnValue);
}

/* Deprecated: See QgisApp::editCopy() instead */
void QgsAttributeTable::copySelectedRows()
{
  // Copy selected rows to the clipboard

  QString toClipboard;
  const char fieldSep = '\t';

  // Pick up the headers first
  for (int i = 0; i < columnCount(); ++i)
    toClipboard += horizontalHeaderItem(i)->text() + fieldSep;
  toClipboard += '\n';

  QList<QTableWidgetSelectionRange> selection = selectedRanges();
  // Then populate with the cell contents
  for (int i = 0; i < selection.count(); ++i)
  {
    QTableWidgetSelectionRange sel = selection.at(i);
    for (int row = sel.topRow(); row < sel.topRow()+sel.rowCount(); ++row)
    {
      for (int column = 0; column < columnCount(); ++column)
        toClipboard += item(row, column)->text() + fieldSep;
      toClipboard += '\n';
    }
  }
#ifdef QGISDEBUG
  std::cerr << "Selected data in table is:\n" << toClipboard.data();
#endif
  // And then copy to the clipboard
  QClipboard* clipboard = QApplication::clipboard();

  // With qgis running under Linux, but with a Windows based X
  // server (Xwin32), ::Selection was necessary to get the data into
  // the Windows clipboard (which seems contrary to the Qt
  // docs). With a Linux X server, ::Clipboard was required.
  // The simple solution was to put the text into both clipboards.

  // The ::Selection setText() below one may need placing inside so
  // #ifdef so that it doesn't get compiled under Windows.
  clipboard->setText(toClipboard, QClipboard::Selection);
  clipboard->setText(toClipboard, QClipboard::Clipboard);
}

void QgsAttributeTable::fillTable(QgsVectorLayer *layer)
{
  int row = 0;

  const QgsFieldMap &fields = layer->pendingFields();

  // set up the column headers
  setColumnCount(fields.size()+1);

  setHorizontalHeaderItem(0, new QTableWidgetItem("id")); //label for the id-column

  int h = 1;
  for (QgsFieldMap::const_iterator fldIt = fields.begin(); fldIt!=fields.end(); fldIt++, h++)
  {
    QgsDebugMsg( QString("%1: field %2: %3 | %4")
                   .arg(h).arg(fldIt.key()).arg(fldIt->name()).arg( QVariant::typeToName(fldIt->type()) ) );

    QTableWidgetItem *twi = new QTableWidgetItem(fldIt->name());
    twi->setData( AttributeIndex, fldIt.key() );
    twi->setData( AttributeName, fldIt->name() );
    twi->setData( AttributeType, fldIt->type() );
    setHorizontalHeaderItem(h, twi);

    mAttrIdxMap.insert(fldIt.key(), h);
  }

  QgsFeatureList features;
  if( layer->selectedFeatureCount()==0 )
  {
    layer->select(layer->pendingAllAttributesList(), QgsRect(), false);

    QgsFeature f;
    while( layer->getNextFeature(f) )
      features << f;
  }
  else
  {
    features = layer->selectedFeatures();
  }

  setRowCount( features.size() );
  
  for(int i=0; i<features.size(); i++)
    putFeatureInTable(i, features[i]);

  // Default row height is too tall
  resizeRowsToContents();

  // Make each column wide enough to show all the contents
  for (int i=0; i<columnCount(); i++)
    resizeColumnToContents(i);
}

void QgsAttributeTable::putFeatureInTable(int row, const QgsFeature& fet)
{
  // Prevent a crash if a provider doesn't update the feature count properly
  if(row >= rowCount())
  {
    setRowCount(row+1);
  }

  //id-field
  int id = fet.featureId();
  QTableWidgetItem *twi = new QTableWidgetItem(QString::number(id));
  twi->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
  setItem(row, 0, twi);
  insertFeatureId(id, row);  //insert the id into the search tree of qgsattributetable

  const QgsAttributeMap& attr = fet.attributeMap();

  for (QgsAttributeMap::const_iterator it = attr.begin(); it != attr.end(); ++it)
  { 
    if( !mAttrIdxMap.contains( it.key() ) )
      continue;

    int h = mAttrIdxMap[ it.key() ];

    twi = horizontalHeaderItem(h);
    if(!twi)
    {
      QgsDebugMsg("header item not found.");
      continue;
    }

    int type = twi->data(AttributeType).toInt();
    bool isNum = (type == QVariant::Double || type == QVariant::Int);

    QString value;
    // get the field values
    if( it->isNull() )
    {
      if( isNum )
        value="";
      else
        value="NULL";
    } else {
      value = it->toString();
    }

    twi = new QTableWidgetItem(value);
    if (isNum)
      twi->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    setItem(row, h, twi);
  }
}

void QgsAttributeTable::bringSelectedToTop()
{
  blockSignals(true);
  horizontalHeader()->setSortIndicatorShown(false);
  mPreviousSortIndicatorColumn = -1;
  int swaptorow=0;
  QList<QTableWidgetSelectionRange> selections = selectedRanges();
  bool removeselection;

  for(QList<QTableWidgetSelectionRange>::iterator iter=selections.begin();iter!=selections.end();++iter)
  {
    removeselection=true;
    while(item(swaptorow, 0)->isSelected())//selections are not necessary stored in ascending order
    {
      ++swaptorow;
    }

    for(int j=iter->topRow();j<=iter->bottomRow();++j)
    {   
      if(j>swaptorow)//selections are not necessary stored in ascending order
      {
        swapRows(j,swaptorow);
        setRangeSelected(QTableWidgetSelectionRange(swaptorow, 0, swaptorow, columnCount()-1), true);
        ++swaptorow;	

      }
      else
      {
        removeselection=false;//keep selection
      }
    }
    if(removeselection)
    {	    
      setRangeSelected(*iter, false);
    }
  }

  //clear and rebuild rowIdMap.
  rowIdMap.clear();
  int id;
  for (int i = 0; i < rowCount(); i++)
  {
    id = item(i, 0)->text().toInt();
    rowIdMap.insert(id, i);
  }

  blockSignals(false);
}

void QgsAttributeTable::selectRowsWithId(const QgsFeatureIds& ids)
{
  /*
  // if selecting rows takes too much time we can use progress dialog
  QProgressDialog progress( tr("Updating selection..."), tr("Abort"), 0, mSelected.size(), tabledisplay);
  int i=0;
  for(std::set<int>::iterator iter=mSelected.begin();iter!=mSelected.end();++iter)
  {
    ++i;
    progress.setValue(i);
    qApp->processEvents();
    if(progress.wasCanceled())
    {
      //deselect the remaining features if action was canceled
      mSelected.erase(iter,--mSelected.end());
      break;
      }
    selectRowWithId(*iter);//todo: avoid that the table gets repainted during each selection
  }
  */


  // to select more rows at once effectively, we stop sending signals to handleChangedSelections()
  // otherwise it will repaint map everytime row is selected

  disconnect(this, SIGNAL(itemSelectionChanged()), this, SLOT(handleChangedSelections()));

  clearSelection();
  QgsFeatureIds::const_iterator it;
  for (it = ids.begin(); it != ids.end(); it++)
  {
    selectRowWithId(*it);
  }

  connect(this, SIGNAL(itemSelectionChanged()), this, SLOT(handleChangedSelections()));
  emit repaintRequested();
}

void QgsAttributeTable::showRowsWithId(const QgsFeatureIds& ids)
{
  setUpdatesEnabled(false);

  // hide all rows first
  for (int i = 0; i < rowCount(); i++)
    hideRow(i);

  // show only matching rows
  QgsFeatureIds::const_iterator it;
  for (it = ids.begin(); it != ids.end(); it++)
  {
    showRow(rowIdMap[*it]);
  }

  clearSelection(); // deselect all
  setUpdatesEnabled(true);
}

void QgsAttributeTable::showAllRows()
{
  for (int i = 0; i < rowCount(); i++)
    showRow(i);
}

void QgsAttributeTable::rowClicked(int row)
{
  if(checkSelectionChanges())//only repaint the canvas if the selection has changed
  {
    emit repaintRequested();
  }
}

void QgsAttributeTable::mouseReleaseEvent(QMouseEvent* e)
{
  QTableWidget::mouseReleaseEvent(e);
  if(checkSelectionChanges())//only repaint the canvas if the selection has changed
  {
    emit repaintRequested();
  }
}

bool QgsAttributeTable::checkSelectionChanges()
{
  std::set<int> theCurrentSelection;
  QList<QTableWidgetSelectionRange> selectedItemRanges = selectedRanges();
  QList<QTableWidgetSelectionRange>::const_iterator range_it = selectedItemRanges.constBegin();
  for (; range_it != selectedItemRanges.constEnd(); ++range_it)
  {
    for (int index = range_it->topRow(); index <= range_it->bottomRow(); index++)
    {
      theCurrentSelection.insert(index);
    }
  }

  if(theCurrentSelection == mLastSelectedRows)
  {
    return false;
  }
  else
  {
    mLastSelectedRows = theCurrentSelection;
    return true;
  }
}

void QgsAttributeTable::attributeValueChanged(int fid, int idx, const QVariant &value)
{
  if( !rowIdMap.contains(fid) )
    return;

  if( !mAttrIdxMap.contains(idx) )
    return;

  item( rowIdMap[fid], mAttrIdxMap[idx] )->setText( value.toString() );
}

void QgsAttributeTable::featureDeleted(int fid)
{
  if( !rowIdMap.contains(fid) )
    return;

  int row = rowIdMap[fid];

  removeRow(row);

  for(QMap<int,int>::iterator it=rowIdMap.begin(); it!=rowIdMap.end(); it++)
    if( it.value() > row )
      rowIdMap[ it.key() ]--;
}

void QgsAttributeTable::addAttribute(int attr, const QgsField &fld)
{
  QTableWidgetItem *twi = new QTableWidgetItem( fld.name() );
  twi->setData( AttributeIndex, attr );
  twi->setData( AttributeName, fld.name() );
  twi->setData( AttributeType, fld.type() );

  insertColumn( columnCount() );
  setHorizontalHeaderItem(columnCount()-1, twi);

  mAttrIdxMap.insert(attr, columnCount()-1);
}

void QgsAttributeTable::deleteAttribute(int attr)
{
  int column = mAttrIdxMap[attr];

  removeColumn( column );
  mAttrIdxMap.remove(attr);
  for(QMap<int, int>::iterator it=mAttrIdxMap.begin(); it!=mAttrIdxMap.end(); it++)
  {
    if( it.value()>column )
      mAttrIdxMap[ it.key() ]--;
  }
}
