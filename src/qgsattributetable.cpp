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
#include <qapplication.h>
#include <qcursor.h>
#include <qfont.h>
#include "qgsattributetable.h"
#include <iostream>
#include <stdlib.h>

QgsAttributeTable::QgsAttributeTable(QWidget * parent, const char *name):QTable(parent, name), lockKeyPressed(false),
sort_ascending(true)
{
  QFont f(font());
  f.setFamily("Helvetica");
  f.setPointSize(11);
  setFont(f);
  setSelectionMode(QTable::MultiRow);
  QObject::connect(this, SIGNAL(selectionChanged()), this, SLOT(handleChangedSelections()));
  setFocus();
}

QgsAttributeTable::~QgsAttributeTable()
{

}
void QgsAttributeTable::columnClicked(int col)
{
  QApplication::setOverrideCursor(Qt::waitCursor);

  //store the ids of the selected rows in a list
  QValueList < int >idsOfSelected;
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

  QValueList < int >::iterator it;
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
  QTableSelection cselection;
  if (lockKeyPressed == false)
    {
      //clear the list and evaluate the last selection
      emit selectionRemoved();
    }
  //if there is no current selection, there is nothing to do
  if (currentSelection() == -1)
    {
      emit repaintRequested();
      return;
    }

  cselection = selection(currentSelection());

  for (int index = cselection.topRow(); index <= cselection.bottomRow(); index++)
    {
      emit selected(text(index, 0).toInt());
    }

  //emit repaintRequested();

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
  for (uint i = 0; i < firstentry.length(); i++)
    {
      if (firstentry.ref(i).isLetter())
        {
          containsletter = true;
        }
    }

  if (containsletter)
    {
      qsort(0, numRows() - 1, col, ascending, true);
  } else
    {
      qsort(0, numRows() - 1, col, ascending, false);
    }

  repaintContents();
}

int QgsAttributeTable::compareItems(QString s1, QString s2, bool ascending, bool alphanumeric)
{
  if (alphanumeric)
    {
      if (s1 > s2)
        {
          if (ascending)
            {
              return 1;
          } else
            {
              return -1;
            }
      } else if (s1 < s2)
        {
          if (ascending)
            {
              return -1;
          } else
            {
              return 1;
            }
      } else if (s1 == s2)
        {
          return 0;
        }
  } else                        //numeric
    {
      double d1 = s1.toDouble();
      double d2 = s2.toDouble();
      if (d1 > d2)
        {
          if (ascending)
            {
              return 1;
          } else
            {
              return -1;
            }
      } else if (d1 < d2)
        {
          if (ascending)
            {
              return -1;
          } else
            {
              return 1;
            }
      } else if (d1 == d2)
        {
          return 0;
        }
    }
}

void QgsAttributeTable::qsort(int lower, int upper, int col, bool ascending, bool alphanumeric)
{
  int i, j;
  QString v;
  if (upper > lower)
    {
      //chose a random element (this avoids n^2 worst case)
      int element = int (rand() / RAND_MAX * (upper - lower) + lower);

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

void QgsAttributeTable::contentsMouseReleaseEvent(QMouseEvent * e)
{
  contentsMouseMoveEvent(e);    //send out a move event to keep the selections updated 
  emit repaintRequested();
}
