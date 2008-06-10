/***************************************************************************
     qgsdetaileditemdata.h  -  A data represenation for a rich QItemData subclass
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
#ifndef QGSDETAILEDITEMDATA_H
#define QGSDETAILEDITEMDATA_H


#include <QMetaType>
#include <QString>
#include <QPixmap>

/** This class is the data only representation of a 
 * QgsDetailedItemWidget, designed to be used in custom views.
 */
class GUI_EXPORT QgsDetailedItemData 
{
  public:
    QgsDetailedItemData();
    ~QgsDetailedItemData();
    void setTitle(QString theTitle);
    void setDetail(QString theDetail);
    void setIcon(QPixmap theIcon);
    void setCheckable(bool theFlag);
    void setChecked(bool theFlag);
    /** This is a hint to the delegate to render using 
     * a widget rather than manually painting every 
     * part of the list item.
     * @note the delegate may completely ignore this 
     * depending on the delegate implementation.
     */
    void setRenderAsWidget(bool theFlag);

    QString title();
    QString detail();
    QPixmap icon();
    bool isCheckable();
    bool isChecked();
    bool isRenderedAsWidget();

  private:
    QString mTitle;
    QString mDetail;
    QString mLibraryName;
    QPixmap mPixmap;
    bool mCheckableFlag;
    bool mCheckedFlag;
    bool mRenderAsWidgetFlag;
};

// Make QVariant aware of this data type (see qtdocs star 
// rating delegate example for more details)
Q_DECLARE_METATYPE(QgsDetailedItemData)
#endif //QGSDETAILEDITEMDATA_H
