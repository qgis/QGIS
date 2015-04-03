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


#ifndef QGSNUMERICSORTLISTVIEWITEM_H
#define QGSNUMERICSORTLISTVIEWITEM_H

#include <QTreeWidgetItem>

/**

  \brief  QTreeWidgetItem that can sort numerically (as opposed to just lexigraphically)

  This class extends the Qt QTreeWidgetItem concept by
  reimplementing QTreeWidgetItem::operator< to allow numeric comparisons

  TODO: Make it work

*/

class GUI_EXPORT QgsNumericSortTreeWidgetItem : public QTreeWidgetItem
{
  public:
    /**
    * Constructor.
    */
    QgsNumericSortTreeWidgetItem( QTreeWidget * parent );
    QgsNumericSortTreeWidgetItem( QTreeWidgetItem * parent );

    //! Destructor
    virtual ~QgsNumericSortTreeWidgetItem();

    virtual bool operator<( const QTreeWidgetItem &other ) const override;

};

#endif
