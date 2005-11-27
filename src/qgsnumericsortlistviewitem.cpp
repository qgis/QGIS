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

/* $Id$ */

#include <fstream>
#include <iostream>

#include "qgsnumericsortlistviewitem.h"


QgsNumericSortListViewItem::QgsNumericSortListViewItem(Q3ListView * parent)
  : Q3ListViewItem(parent)
{
  // NOOP
}

QgsNumericSortListViewItem::QgsNumericSortListViewItem(Q3ListViewItem  * parent)
  : Q3ListViewItem(parent)
{
  // NOOP
}

QgsNumericSortListViewItem::~QgsNumericSortListViewItem()
{
  // NOOP
}

int QgsNumericSortListViewItem::compare(Q3ListViewItem * i, int col, bool ascending) const
{
  if (col == 0)  // The ID column
  {
    uint a = text(col).toUInt();
    uint b = i->text(col).toUInt();

    if (a < b)
    {
      return -1;
    }
    else if (a > b)
    {
      return +1;
    }
    else
    {
      return 0;
    }
  }
  else
  {
    // Pass-through to the QListViewItem implementation
    return Q3ListViewItem::compare(i, col, ascending);
  }
}




