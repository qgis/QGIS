/***************************************************************************
                          qgsattributetable.h  -  description
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
#ifndef QGSATTRIBUTETABLE_H
#define QGSATTRIBUTETABLE_H

#include <qtable.h>
#include <qmap.h>

/**
  *@author Gary E.Sherman
  */

class QgsAttributeTable:public QTable
{
  Q_OBJECT 

      public:
      QgsAttributeTable(QWidget * parent = 0, const char *name = 0);
      ~QgsAttributeTable();

      /**Inserts the feature with the specified id into rowIdMap. This function has to be called (e.g. from QgsShapeFileLayer) when a row is inserted into the table*/
      void insertFeatureId(int id, int row);
      /**Selects the row which belongs to the feature with the specified id*/
      void selectRowWithId(int id);
      /**Sorts a column. This method replaces the one from QTable to allow numeric sorting*/
      virtual void sortColumn(int col, bool ascending=true, bool wholeRows=false);
      public slots:
      void columnClicked(int col);
      protected slots:
	  void handleChangedSelections();
      protected:
      /**Flag telling if the ctrl-button or the shift-button is pressed*/
      bool lockKeyPressed;
      /**Search tree to find a row corresponding to a feature id*/
      QMap<int,int> rowIdMap;
      /**Flag indicating, which sorting order should be used*/
      bool sort_ascending;
      /**Compares the content of two cells either alphanumeric or numeric. If 'ascending' is true, -1 means s1 is less, 0 equal, 1 greater. If 'ascending' is false, -1 means s1 is more, 0 equal, 1 greater. This method is used mainly to sort a column*/
      int compareItems(QString s1, QString s2, bool ascending, bool alphanumeric);
      void keyPressEvent(QKeyEvent* ev);
      void keyReleaseEvent(QKeyEvent* ev);
      /**Method used by sortColumn (implementation of a quicksort)*/
      void qsort(int lower, int upper, int col, bool ascending, bool alphanumeric);
      void contentsMouseReleaseEvent(QMouseEvent* e);
        signals:

      /**Is emitted when a row was selected*/
      void selected(int);
      /**Is emitted when all rows have been deselected*/
      void selectionRemoved();
      /**Is emmited when a set of related selection and deselection signals have been emitted*/
      void repaintRequested();
};

#endif
