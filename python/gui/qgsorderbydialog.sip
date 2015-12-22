/***************************************************************************
 qgsorderbydialog.h

 ---------------------
 begin                : 20.12.2015
 copyright            : (C) 2015 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

class QgsOrderByDialog : QDialog
{
%TypeHeaderCode
#include "qgsorderbydialog.h"
%End
  public:
    QgsOrderByDialog( QgsVectorLayer* layer, QWidget* parent = nullptr );

    /**
     * Set the order by to manage
     */
    void setOrderBys( const QList<QgsFeatureRequest::OrderByClause>& orderBys );

    /**
     * Get the order by defined in the dialog
     */
    QgsFeatureRequest::OrderBy orderBys();
};
