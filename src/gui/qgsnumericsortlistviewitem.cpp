/***************************************************************************
  qgsnumericsortlistviewitem.cpp  -  A QListViewItem that can sort numerically
                                     (as opposed to just lexigraphically)
                             -------------------
    begin                : 06 Nov, 2005
    copyright            : (C) 2005 by Brendan Morley
    email                : morb at ozemail dot com dot au
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsnumericsortlistviewitem.h"


QgsNumericSortTreeWidgetItem::QgsNumericSortTreeWidgetItem( QTreeWidget * parent )
    : QTreeWidgetItem( parent, UserType )
{
  // NOOP
}

QgsNumericSortTreeWidgetItem::QgsNumericSortTreeWidgetItem( QTreeWidgetItem * parent )
    : QTreeWidgetItem( parent, UserType )
{
  // NOOP
}

QgsNumericSortTreeWidgetItem::~QgsNumericSortTreeWidgetItem()
{
  // NOOP
}

bool QgsNumericSortTreeWidgetItem::operator<( const QTreeWidgetItem &other ) const
{
  int column = treeWidget() ? treeWidget()->sortColumn() : 0;
  if ( column == 0 )  // The ID column
  {
    return text( column ).toUInt() < other.text( column ).toUInt();
  }
  else
    return text( column ) < other.text( column );
}
