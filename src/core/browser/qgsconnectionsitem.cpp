/***************************************************************************
                             qgsconnectionsitem.cpp
                             -------------------
    begin                : 2011-04-01
    copyright            : (C) 2011 Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsconnectionsitem.h"
#include "moc_qgsconnectionsitem.cpp"

QgsConnectionsRootItem::QgsConnectionsRootItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &providerKey )
  : QgsDataCollectionItem( parent, name, path, providerKey )
{
}

