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
#include <qapplication.h>
#include <qcursor.h>
#include <qfont.h>
#include "qgsattributetable.h"
#include <iostream>

QgsAttributeTable::QgsAttributeTable(QWidget * parent, const char *name):QTable(parent, name), lockKeyPressed(false)
{
	QFont f( font() );
	f.setFamily( "Helvetica" );
	f.setPointSize(11);
	setFont(f);
	setSelectionMode(QTable::MultiRow);
	QObject::connect(this,SIGNAL(selectionChanged()),this,SLOT(handleChangedSelections()));
	setFocus();
}

QgsAttributeTable::~QgsAttributeTable()
{
   
}
void QgsAttributeTable::columnClicked(int col)
{
	QApplication::setOverrideCursor(Qt::waitCursor);
	sortColumn(col, true, true);
	//clear and rebuild rowIdMap. Overwrite sortColumn later and sort rowIdMap there
	rowIdMap.clear();
	int id;
	for(int i=0;i<numRows();i++)
	{
	    id=text(i,0).toInt();
	    rowIdMap.insert(id,i);
	}
	clearSelection(true);
	emit selectionRemoved();
	emit repaintRequested();
	QApplication::restoreOverrideCursor();
}

void QgsAttributeTable::keyPressEvent(QKeyEvent* ev)
{
    if(ev->key()==Qt::Key_Control||ev->key()==Qt::Key_Shift)
    {
	lockKeyPressed=true;
    }
}

void QgsAttributeTable::keyReleaseEvent(QKeyEvent* ev)
{
    if(ev->key()==Qt::Key_Control||ev->key()==Qt::Key_Shift)
    {
	lockKeyPressed=false;
    }
}

void QgsAttributeTable::handleChangedSelections()
{   
    QTableSelection cselection;
    if(lockKeyPressed==false)
    {
	//clear the list and evaluate the last selection
	emit selectionRemoved();
    }
    
    //if there is no current selection, there is nothing to do
    if(currentSelection()==-1)
    {
	emit repaintRequested();
	return;
    }

    cselection=selection(currentSelection());
    
    for(int index=cselection.topRow();index<=cselection.bottomRow();index++)
    {
	emit selected(text(index,0).toInt());
    }

    emit repaintRequested();
   
}

void QgsAttributeTable::insertFeatureId(int id)
{
    rowIdMap.insert(id,id+1);
}

void QgsAttributeTable::selectRowWithId(int id)
{
    //brute force approach, add a solution with a hash table or a search tree later (and rebuild this structure every time the table is sorted)
    /*for(int i=0;i<numRows();i++)
    {
	if(text(i,0).toInt()==id)
	{
	    selectRow(i);
	    return;
	}
	}*/
    QMap<int,int>::iterator it=rowIdMap.find(id);
    selectRow(it.data());
}

	  
