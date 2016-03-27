/***************************************************************************
  qgstablewidgetitem.sip - QgsTableWidgetItem

 ---------------------
 begin                : 27.3.2016
 copyright            : (C) 2016 by Matthias Kuhn, OPENGIS.ch
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

class QgsTableWidgetItem : QTableWidgetItem
{
%TypeHeaderCode
#include "qgstablewidgetitem.h"
%End

  public:
    QgsTableWidgetItem();
    QgsTableWidgetItem( const QString& text );

    /**
     * Set the role by which the items should be sorted.
     * By default this will be set to Qt::DisplayRole
     */
    void setSortRole(  int role );
    /**
     * Get the role by which the items should be sorted.
     * By default this will be Qt::DisplayRole
     */
    int sortRole() const;
};
