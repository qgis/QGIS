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
/*  $Id$  */
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
      void insertFeatureId(int id);
      /**Selects the row which belongs to the feature with the specified id*/
      void selectRowWithId(int id);

      public slots:
      void columnClicked(int col);
      protected slots:
	  void handleChangedSelections();
      protected:
      /**Flag telling if the ctrl-button or the shift-button is pressed*/
      bool lockKeyPressed;
      /**Search tree to find a row corresponding to a feature id*/
      QMap<int,int> rowIdMap;
      void keyPressEvent(QKeyEvent* ev);
      void keyReleaseEvent(QKeyEvent* ev);
        signals:
      /**Is emitted when a row was selected*/
      void selected(int);
      /**Is emitted when all rows have been deselected*/
      void selectionRemoved();
      /**Is emmited when a set of related selection and deselection signals have been emitted*/
      void repaintRequested();
};

#endif
