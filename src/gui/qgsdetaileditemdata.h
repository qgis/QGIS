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

/** This class is the data only representation of a 
 * QgsDetailedItemWidget, designed to be used in custom views.
 */
class QgsDetailedItemData 
{
  public:
    QgsDetailedItemData();
    ~QgsDetailedItemData();
    void setTitle(QString theTitle);
    void setDetail(QString theDetail);
    QString title();
    QString detail();
  private:
    QString mTitle;
    QString mDetail;
    QString mLibraryName;
    bool mCheckBoxEnabled;
};

// Make QVariant aware of this data type (see qtdocs star 
// rating delegate example for more details)
Q_DECLARE_METATYPE(QgsDetailedItemData)
#endif //QGSDETAILEDITEMDATA_H
