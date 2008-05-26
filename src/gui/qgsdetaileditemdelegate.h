/***************************************************************************
     qgsdetaileditemdelegate.h  -  A rich QItemDelegate subclass
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
#ifndef QGSDETAILEDITEMDELEGATE_H
#define QGSDETAILEDITEMDELEGATE_H

#include <QAbstractItemDelegate>
#include <QString>

class QCheckBox;
class QgsDetailedItemWidget;
class QFontMetrics;

class GUI_EXPORT QgsDetailedItemDelegate : 
     public QAbstractItemDelegate 
{
  Q_OBJECT;
  public:
    QgsDetailedItemDelegate(QObject * parent = 0);
    ~QgsDetailedItemDelegate();
    void paint(QPainter * thePainter, 
               const QStyleOptionViewItem & theOption,
               const QModelIndex & theIndex) const;
    QSize sizeHint( const QStyleOptionViewItem & theOption, 
               const QModelIndex & theIndex ) const;
  private:
    QStringList wordWrap(QString theString, 
                         QFontMetrics theMetrics, 
                         int theWidth) const;
    QgsDetailedItemWidget * mpWidget;
    QCheckBox * mpCheckBox;
};

#endif //QGSDETAILEDITEMDELEGATE_H
