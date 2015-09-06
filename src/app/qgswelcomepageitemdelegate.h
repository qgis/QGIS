/***************************************************************************

               ----------------------------------------------------
              date                 : 17.8.2015
              copyright            : (C) 2015 by Matthias Kuhn
              email                : matthias (at) opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSWELCOMEPAGEITEMDELEGATE_H
#define QGSWELCOMEPAGEITEMDELEGATE_H

#include <QStyledItemDelegate>

class QgsWelcomePageItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

  public:
    QgsWelcomePageItemDelegate( QObject * parent = 0 );
    void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const override;
    QSize sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const override;
};

#endif // QGSWELCOMEPAGEITEMDELEGATE_H
