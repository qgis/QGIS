/***************************************************************************
    qgslegendpropertygroup.h
    ---------------------
    begin                : January 2007
    copyright            : (C) 2007 by Martin Dobias
    email                : wonder.sk at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
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

    /** Overloads cmpare function of QListViewItem
      * @note The property group must always be the first in the list
      */
    int compare( QTreeWidgetItem* i, int col, bool ascending )
    { Q_UNUSED( i ); Q_UNUSED( col ); Q_UNUSED( ascending ); return -1; }
};

#endif
