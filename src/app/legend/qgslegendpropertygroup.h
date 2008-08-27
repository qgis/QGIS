/***************************************************************************
 *   Copyright (C) 2005 by Tim Sutton   *
 *   aps02ts@macbuntu   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef QGSLEGENDPROPERTYGROUP_H
#define QGSLEGENDPROPERTYGROUP_H

#include <qgslegenditem.h>

/**
container for layer properties  (e.g. projection, scale dependent view)

@author Tim Sutton
*/
class QgsLegendPropertyGroup : public QgsLegendItem
{
  public:
    QgsLegendPropertyGroup( QTreeWidgetItem *, QString );

    ~QgsLegendPropertyGroup();

    bool isLeafNode() {return mLeafNodeFlag;}
    DRAG_ACTION accept( LEGEND_ITEM_TYPE type );
    QgsLegendItem::DRAG_ACTION accept( const QgsLegendItem* li ) const;
    /** Overloads cmpare function of QListViewItem
      * @note The property group must always be the first in the list
      */
    int compare( QTreeWidgetItem* i, int col, bool ascending ) {return -1;}
};

#endif
