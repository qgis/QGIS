/***************************************************************************
  qgstablewidgetitem.cpp - QgsTableWidgetItem

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
#include "qgstablewidgetitem.h"

QgsTableWidgetItem::QgsTableWidgetItem()
    : QTableWidgetItem()
    , mSortRole( Qt::DisplayRole )
{
}

QgsTableWidgetItem::QgsTableWidgetItem( const QString& text )
    : QTableWidgetItem( text )
    , mSortRole( Qt::DisplayRole )
{
}

void QgsTableWidgetItem::setSortRole( int role )
{
  mSortRole = role;
}

int QgsTableWidgetItem::sortRole() const
{
  return mSortRole;
}

bool QgsTableWidgetItem::operator<( const QTableWidgetItem& other ) const
{
#if QT_VERSION < 0x050000
  return data( mSortRole ).toString() < other.data( mSortRole ).toString();
#else
  return data( mSortRole ) < other.data( mSortRole );
#endif
}
