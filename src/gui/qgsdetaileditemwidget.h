/***************************************************************************
     qgsdetaileditemwidget.h  -  A rich QItemWidget subclass
                             -------------------
    begin                : Sat May 17 2008
    copyright            : (C) 2008 Tim Sutton
    email                : tim@linfiniti.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id:$ */
#ifndef QGSDETAILEDITEMWIDGET_H
#define QGSDETAILEDITEMWIDGET_H

#include <ui_qgsdetaileditemwidgetbase.h>
#include <qgsdetaileditemdata.h>

/** \ingroup gui
 * A widget renderer for detailed item views.
 * @see also QgsDetailedItem and QgsDetailedItemData.
 */
class QgsDetailedItemWidget :
    public QWidget, private Ui::QgsDetailedItemWidgetBase
{
    Q_OBJECT
  public:
    QgsDetailedItemWidget( QWidget * parent = 0 );
    ~QgsDetailedItemWidget();
    void setData( QgsDetailedItemData theData );
    void setChecked( bool theFlag );
  private:
    QgsDetailedItemData mData;
};

#endif //QGSDETAILEDITEMWIDGET_H
