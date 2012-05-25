/***************************************************************************
    qgslegendsymbologygroup.h
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
#ifndef QGSLEGENDSYMBOLOGYGROUP_H
#define QGSLEGENDSYMBOLOGYGROUP_H

#include <qgslegenditem.h>

class QgsMapLayer;

/**
@author Tim Sutton
*/
class QgsLegendSymbologyGroup : public QgsLegendItem
{
  public:
    QgsLegendSymbologyGroup( QTreeWidgetItem * theItem, QString theString );
    ~QgsLegendSymbologyGroup();

    /** Overloads cmpare function of QListViewItem
      * @note The symbology group must always be the second in the list
      */
    int compare( QTreeWidgetItem * i, int col, bool ascending );
};

#endif
