/***************************************************************************
    qgsgrassprovidermodule.h  -  Data provider for GRASS format
                             -------------------
    begin                : March, 2004
    copyright            : (C) 2004 by Gary E.Sherman, Radim Blazek
    email                : sherman@mrcc.com, blazek@itc.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSGRASSPROVIDERMODULE_H
#define QGSGRASSPROVIDERMODULE_H

#include "qgsdataitem.h"

class QgsGrassLocationItem : public QgsDataCollectionItem
{
  public:
    QgsGrassLocationItem( QgsDataItem* parent, QString path );
    ~QgsGrassLocationItem();

    static bool isLocation( QString path );
    QVector<QgsDataItem*> createChildren();
};

class QgsGrassMapsetItem : public QgsDataCollectionItem
{
  public:
    QgsGrassMapsetItem( QgsDataItem* parent, QString path );
    ~QgsGrassMapsetItem();

    static bool isMapset( QString path );
    QVector<QgsDataItem*> createChildren();

    QString mLocation;
    QString mGisdbase;
};

#endif // QGSGRASSPROVIDERMODULE_H
