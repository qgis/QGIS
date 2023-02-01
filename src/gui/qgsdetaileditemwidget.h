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
#ifndef QGSDETAILEDITEMWIDGET_H
#define QGSDETAILEDITEMWIDGET_H

#include "ui_qgsdetaileditemwidgetbase.h"
#include "qgis_sip.h"
#include "qgsdetaileditemdata.h"
#include "qgis_gui.h"

/**
 * \ingroup gui
 * \brief A widget renderer for detailed item views.
 * \see QgsDetailedItem
 * \see QgsDetailedItemData
 */
class GUI_EXPORT QgsDetailedItemWidget : public QWidget, private Ui::QgsDetailedItemWidgetBase
{
    Q_OBJECT
  public:

    //! Constructor for QgsDetailedItemWidget
    QgsDetailedItemWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );
    void setData( const QgsDetailedItemData &data );
    void setChecked( bool flag );
  private:
    QgsDetailedItemData mData;
};

#endif //QGSDETAILEDITEMWIDGET_H
