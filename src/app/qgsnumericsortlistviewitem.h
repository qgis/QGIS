/***************************************************************************
    qgsnumericsortlistviewitem.h  -  A QListViewItem that can sort numerically
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

#ifndef QGSNUMERICSORTLISTVIEWITEM_H
#define QGSNUMERICSORTLISTVIEWITEM_H

#include <Q3ListView>
#include <Q3ListViewItem>

/**

  \brief  QListViewItem that can sort numerically (as opposed to just lexigraphically)

  This class extends the Qt QListViewItem concept by 
  reimplementing QListViewItem::compare to allow numeric comparisons

  TODO: Make it work

*/

class QgsNumericSortListViewItem : public Q3ListViewItem
{

//  Q_OBJECT

public:
  /**
  * Constructor.
  */
  QgsNumericSortListViewItem ( Q3ListView * parent );
  QgsNumericSortListViewItem ( Q3ListViewItem * parent );

  //! Destructor
  virtual ~QgsNumericSortListViewItem ();

  virtual int compare ( Q3ListViewItem * i, int col, bool ascending ) const;

};

#endif
