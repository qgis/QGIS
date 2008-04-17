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
#include <QApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QFont>
#include <QClipboard>
#include <QAction>
#include <QMenu>
#include <QLineEdit>
#include <QValidator>

#include "qgsattributetable.h"
#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgslogger.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"

#include <stdlib.h>

QgsAttributeTable::QgsAttributeTable(QWidget * parent, const char *name):
        Q3Table(parent, name),
        lockKeyPressed(false),
        sort_ascending(true),
        mEditable(false),
        mEdited(false),
        mActionPopup(0)
{
  QFont f(font());
  f.setFamily("Helvetica");
  f.setPointSize(11);
  setFont(f);
  setSelectionMode(Q3Table::MultiRow);
  QObject::connect(this, SIGNAL(selectionChanged()), this, SLOT(handleChangedSelections()));
  connect(this, SIGNAL(contextMenuRequested(int, int, const QPoint&)), this, SLOT(popupMenu(int, int, const QPoint&)));
  connect(this, SIGNAL(valueChanged(int, int)), this, SLOT(storeChangedValue(int,int)));
  connect(verticalHeader(), SIGNAL(released(int)), this, SLOT(rowClicked(int))); 
  setReadOnly(true);
  setFocus();
}

QgsAttributeTable::~QgsAttributeTable()
{
}

QWidget *QgsAttributeTable::createEditor(int row, int col, bool initFromCell ) const
{
  QLineEdit *le = static_cast<QLineEdit*>(Q3Table::createEditor(row, col, initFromCell));

  if( mFields[col-1].type()==QVariant::Int )
  {
    le->setValidator( new QIntValidator(le) );
  }
  else if( mFields[col-1].type()==QVariant::Double )
  {
    le->setValidator( new QDoubleValidator(le) );
  }

  return le;
}

void QgsAttributeTable::columnClicked(int col)
{
  QApplication::setOverrideCursor(Qt::waitCursor);

  //store the ids of the selected rows in a list
  QList < int >idsOfSelected;
  for (int i = 0; i < numSelections(); i++)
  {
    for (int j = selection(i).topRow(); j <= selection(i).bottomRow(); j++)
    {
      idsOfSelected.append(text(j, 0).toInt());
    }
  }

  sortColumn(col, sort_ascending, true);

  //clear and rebuild rowIdMap. Overwrite sortColumn later and sort rowIdMap there
  rowIdMap.clear();
  int id;
  for (int i = 0; i < numRows(); i++)
  {
    id = text(i, 0).toInt();
    rowIdMap.insert(id, i);
  }

  QObject::disconnect(this, SIGNAL(selectionChanged()), this, SLOT(handleChangedSelections()));
  clearSelection(true);

  //select the rows again after sorting

  QList < int >::iterator it;
  for (it = idsOfSelected.begin(); it != idsOfSelected.end(); ++it)
  {
    selectRowWithId((*it));
  }
  QObject::connect(this, SIGNAL(selectionChanged()), this, SLOT(handleChangedSelections()));

  //change the sorting order after each sort
  (sort_ascending == true) ? sort_ascending = false : sort_ascending = true;

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
  Q3TableSelection cselection;
  if (lockKeyPressed == false)
  {
    //clear the list and evaluate the last selection
    emit selectionRemoved(false);
  }
  //if there is no current selection, there is nothing to do
  if (currentSelection() == -1)
  {
    return;
  }

  cselection = selection(currentSelection());

  for (int index = cselection.topRow(); index <= cselection.bottomRow(); index++)
  {
    emit selected(text(index, 0).toInt(), false);
  }

  //don't send the signal repaintRequested() from here
  //but in contentsMouseReleaseEvent() and rowClicked(int)
  //todo: don't repaint in case of double clicks
}

void QgsAttributeTable::insertFeatureId(int id, int row)
{
  rowIdMap.insert(id, row);
}

void QgsAttributeTable::selectRowWithId(int id)
{
  QMap < int, int >::iterator it = rowIdMap.find(id);
  selectRow(it.data());
}

void QgsAttributeTable::sortColumn(int col, bool ascending, bool wholeRows)
{
  //if the first entry contains a letter, sort alphanumerically, otherwise numerically
  QString firstentry = text(0, col);
  bool containsletter = false;
  for (int i = 0; i < firstentry.length(); i++)
  {
    if (firstentry.ref(i).isLetter())
    {
      containsletter = true;
    }
  }

  if (containsletter)
  {
    qsort(0, numRows() - 1, col, ascending, true);
  }
  else
  {
    qsort(0, numRows() - 1, col, ascending, false);
  }

  repaintContents();
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
    v = text(upper, col);
    i = lower - 1;
    j = upper;
    for (;;)
    {
      while (compareItems(text(++i, col), v, ascending, alphanumeric) == -1);
      while (compareItems(text(--j, col), v, ascending, alphanumeric) == 1 && j > 0); //make sure that j does not get negative
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

void QgsAttributeTable::popupMenu(int row, int col, const QPoint& pos)
{
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
  Q3Header* header = horizontalHeader();

#ifdef QGISDEBUG
  if (header->count() != numCols())
    std::cerr << "Something wrong with the table (file " 
	      << __FILE__<< ", line " << __LINE__ 
	      << ")." << std::endl;
#endif

  for (int i = 0; i < numCols(); ++i)
    mActionValues.push_back(std::make_pair(header->label(i), text(row, i)));

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

bool QgsAttributeTable::addAttribute(const QString& name, const QString& type)
{
  //first test if an attribute with the same name is already in the table
  for(int i=0;i<horizontalHeader()->count();++i)
  {
    if(horizontalHeader()->label(i)==name)
    {
      //name conflict
      return false;
    }
  }
  mAddedAttributes.insert(name,type);

  QgsDebugMsg("inserting attribute " + name + " of type " + type + ", numCols: " + QString::number(numCols()) );

  //add a new column at the end of the table

  insertColumns(numCols());
  horizontalHeader()->setLabel(numCols()-1,name);
  mEdited=true;
  return true;
}

void QgsAttributeTable::deleteAttribute(const QString& name)
{
  //check, if there is already an attribute with this name in mAddedAttributes
  QgsNewAttributesMap::iterator iter = mAddedAttributes.find(name);
  if(iter!=mAddedAttributes.end())
  {
    mAddedAttributes.remove(iter);
    removeAttrColumn(name);
  }
  else
  {
    mDeletedAttributes.insert(name); 
    removeAttrColumn(name);
  }
  mEdited=true;
}


/* Deprecated: See QgisApp::editCopy() instead */
void QgsAttributeTable::copySelectedRows()
{
  // Copy selected rows to the clipboard

  QString toClipboard;
  const char fieldSep = '\t';

  // Pick up the headers first
  Q3Header* header = horizontalHeader();
  for (int i = 0; i < header->count(); ++i)
    toClipboard += header->label(i) + fieldSep;
  toClipboard += '\n';

  // Then populate with the cell contents
  for (int i = 0; i < numSelections(); ++i)
  {
    Q3TableSelection sel = selection(i);
    for (int row = sel.topRow(); row < sel.topRow()+sel.numRows(); ++row)
    {
      for (int column = 0; column < numCols(); ++column)
        toClipboard += text(row, column) + fieldSep;
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

bool QgsAttributeTable::commitChanges(QgsVectorLayer* layer)
{
  bool isSuccessful = false;

  if(layer)
  {
    //convert strings of deleted attributes to ids

    QgsVectorDataProvider* provider = layer->getDataProvider();

    if(provider)
    {

      QgsAttributeIds deletedIds;
      QSet<QString>::const_iterator it = mDeletedAttributes.constBegin();

      for(; it != mDeletedAttributes.constEnd(); ++it)
      {
        deletedIds.insert(provider->indexFromFieldName(*it));
      }

      isSuccessful = true;
      if( !mAddedAttributes.empty() )
      {
        // add new attributes beforehand, so attribute changes can be applied
        isSuccessful = layer->commitAttributeChanges(QgsAttributeIds(), mAddedAttributes, QgsChangedAttributesMap());

        if(isSuccessful)
          // forget added attributes on successful addition
          mAddedAttributes.clear();
      }

      if(isSuccessful)
      {
        QgsChangedAttributesMap attributeChanges; //convert mChangedValues to QgsChangedAttributesMap
        int fieldIndex;

        QMap<int, QMap<QString, QString> >::const_iterator att_it = mChangedValues.constBegin();
        for(; att_it != mChangedValues.constEnd(); ++att_it)
        {
          QgsAttributeMap newAttMap;
          QMap<QString, QString>::const_iterator record_it = att_it->constBegin();
          for(; record_it != att_it->constEnd(); ++record_it)
          {
            fieldIndex = provider->indexFromFieldName(record_it.key());
            if(fieldIndex != -1)
            {
              if( record_it.value()=="NULL" ||
                  ( record_it.value().isEmpty() &&
                    (provider->fields()[fieldIndex].type()==QVariant::Int ||
                     provider->fields()[fieldIndex].type()==QVariant::Double) ) )
                newAttMap.insert(fieldIndex, QVariant(QString::null) );
              else
                newAttMap.insert(fieldIndex, record_it.value());
            }
            else
            {
              QgsDebugMsg("Changed attribute " + record_it.key() + " not found");
            }
          }
          attributeChanges.insert(att_it.key(), newAttMap);
        } 

        isSuccessful = layer->commitAttributeChanges(deletedIds,
          QgsNewAttributesMap(),
          attributeChanges);
      }
    }
  }

  if (isSuccessful)
  {
    mEdited=false;
    clearEditingStructures();
  }

  return isSuccessful;
}

bool QgsAttributeTable::rollBack(QgsVectorLayer* layer)
{
  if(layer)
  {
    fillTable(layer);
  }
  mEdited=false;
  clearEditingStructures();
  return true;
}


void QgsAttributeTable::fillTable(QgsVectorLayer* layer)
{
  QgsVectorDataProvider* provider=layer->getDataProvider();
  if(provider)
  {
    QgsFeature fet;
    int row = 0;
    
    QgsFeatureList& addedFeatures = layer->addedFeatures();
    QgsFeatureIds& deletedFeatures = layer->deletedFeatureIds();

    // set up the column headers
    Q3Header *colHeader = horizontalHeader();
    mFields = provider->fields();
    int fieldcount=provider->fieldCount();
  
    setNumRows(provider->featureCount() + addedFeatures.size() - deletedFeatures.size());
    setNumCols(fieldcount+1);
    colHeader->setLabel(0, "id"); //label for the id-column

    int h = 1;
    QgsFieldMap::const_iterator fldIt;
    for (fldIt = mFields.begin(); fldIt != mFields.end(); ++fldIt)
    {
      QgsDebugMsg("field " + QString::number(fldIt.key()) + ": " + fldIt->name() +
                  " | " + QString(QVariant::typeToName(fldIt->type())) );
        
      colHeader->setLabel(h++, fldIt->name());
    }

    //go through the features and fill the values into the table
    QgsAttributeList all = provider->allAttributesList();
    provider->select(all, QgsRect(), false);
    
    while (provider->getNextFeature(fet))
    {
      if (!deletedFeatures.contains(fet.featureId()))
      {
        putFeatureInTable(row, fet);
        row++;
      }
    }

    //also consider the not commited features
    for(QgsFeatureList::iterator it = addedFeatures.begin(); it != addedFeatures.end(); it++)
    {
      putFeatureInTable(row, *it);
      row++;
    }

    // Make each column wide enough to show all the contents
    for (int i = 0; i < numCols(); ++i)
      adjustColumn(i);
  }
}

void QgsAttributeTable::putFeatureInTable(int row, QgsFeature& fet)
{
  if(row >= numRows())//prevent a crash if a provider doesn't update the feature count properly
  {
    setNumRows(row+1);
  }

  //id-field
  int id = fet.featureId();
  setText(row, 0, QString::number(id));
  insertFeatureId(id, row);  //insert the id into the search tree of qgsattributetable
  const QgsAttributeMap& attr = fet.attributeMap();
  QgsAttributeMap::const_iterator it;
  int h = 1;
  for (it = attr.begin(); it != attr.end(); ++it)
  { 
    QString value;

    // get the field values
    if( it->isNull() )
    {
      if( mFields[h-1].type()==QVariant::Int || mFields[h-1].type()==QVariant::Double )
        value="";
      else
        value="NULL";
    } else {
      value = it->toString();
    }

    clearCellWidget(row, h);
    setText(row, h++, value);
  }
}

void QgsAttributeTable::storeChangedValue(int row, int column)
{
  //id column is not editable
  if(column>0)
  {
    //find feature id
    int id=text(row,0).toInt();
    QString field = horizontalHeader()->label(column);

    if(id>=0)
    {
      // add empty map for feature if doesn't exist
      if (!mChangedValues.contains(id))
      {
        mChangedValues.insert(id, QMap<QString, QString>());
      }

      mChangedValues[id].insert(field, text(row,column));
      mEdited=true;
    }
    else
    {
      // added feature attribute changed
      emit featureAttributeChanged(row,column);
    }
  }
}

void QgsAttributeTable::clearEditingStructures()
{
  mDeletedAttributes.clear();
  mAddedAttributes.clear();
  mChangedValues.clear();
}

void QgsAttributeTable::removeAttrColumn(const QString& name)
{
  Q3Header* header=horizontalHeader();
  for(int i=0;i<header->count();++i)
  {
    if(header->label(i)==name)
    {
      removeColumn(i);
      break;
    }
  }
}

void QgsAttributeTable::bringSelectedToTop()
{
  blockSignals(true);
  int swaptorow=0;
  std::list<Q3TableSelection> selections;
  bool removeselection;

  for(int i=0;i<numSelections();++i)
  {
    selections.push_back(selection(i));
  }

  Q3TableSelection sel;  

  for(std::list<Q3TableSelection>::iterator iter=selections.begin();iter!=selections.end();++iter)
  {
    removeselection=true;
    while(isRowSelected(swaptorow, true))//selections are not necessary stored in ascending order
    {
      ++swaptorow;
    }

    for(int j=iter->topRow();j<=iter->bottomRow();++j)
    {   
      if(j>swaptorow)//selections are not necessary stored in ascending order
      {
        swapRows(j,swaptorow);
        selectRow(swaptorow);
        ++swaptorow;	

      }
      else
      {
        removeselection=false;//keep selection
      }
    }
    if(removeselection)
    {	    
      removeSelection(*iter);
    }
  }

  //clear and rebuild rowIdMap.
  rowIdMap.clear();
  int id;
  for (int i = 0; i < numRows(); i++)
  {
    id = text(i, 0).toInt();
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

  QObject::disconnect(this, SIGNAL(selectionChanged()), this, SLOT(handleChangedSelections()));

  clearSelection(true); 
  QgsFeatureIds::const_iterator it;
  for (it = ids.begin(); it != ids.end(); it++)
  {
    selectRowWithId(*it);
  }

  QObject::connect(this, SIGNAL(selectionChanged()), this, SLOT(handleChangedSelections()));
  emit repaintRequested();
}

void QgsAttributeTable::showRowsWithId(const QgsFeatureIds& ids)
{
  setUpdatesEnabled(false);

  // hide all rows first
  for (int i = 0; i < numRows(); i++)
    hideRow(i);

  // show only matching rows
  QgsFeatureIds::const_iterator it;
  for (it = ids.begin(); it != ids.end(); it++)
  {
    showRow(rowIdMap[*it]);
  }

  clearSelection(); // deselect all
  setUpdatesEnabled(true);
  repaintContents();
}

void QgsAttributeTable::showAllRows()
{
  for (int i = 0; i < numRows(); i++)
    showRow(i);
}

void QgsAttributeTable::rowClicked(int row)
{
  if(checkSelectionChanges())//only repaint the canvas if the selection has changed
  {
    emit repaintRequested();
  }
}

void QgsAttributeTable::contentsMouseReleaseEvent(QMouseEvent* e)
{
  Q3Table::contentsMouseReleaseEvent(e);
  if(checkSelectionChanges())//only repaint the canvas if the selection has changed
  {
    emit repaintRequested();
  }
}

bool QgsAttributeTable::checkSelectionChanges()
{
  std::set<int> theCurrentSelection;
  Q3TableSelection cselection;
  cselection = selection(currentSelection());
  for (int index = cselection.topRow(); index <= cselection.bottomRow(); index++)
  {
    theCurrentSelection.insert(index);
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
