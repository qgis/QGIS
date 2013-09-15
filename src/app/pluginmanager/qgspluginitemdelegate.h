/***************************************************************************
    qgspluginitemdelegate.h  -  a QItemDelegate subclass for plugin manager
                             -------------------
    begin                : Fri Sep 13 2013, Brighton HF
    copyright            : (C) 2013 Borys Jurgiel
    email                : info@borysjurgiel.pl
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPLUGINITEMDELEGATE_H
#define QGSPLUGINITEMDELEGATE_H

#include <QStyledItemDelegate>

/**
 * A custom model/view delegate that can eithe display checkbox or empty space for proprer text alignment
 */
class QgsPluginItemDelegate : public QStyledItemDelegate
{
  Q_OBJECT
  public:
    QgsPluginItemDelegate( QObject * parent = 0 );
    QSize sizeHint( const QStyleOptionViewItem & theOption, const QModelIndex & theIndex ) const;
    void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
};

#endif //QGSPLUGINITEMDELEGATE_H
