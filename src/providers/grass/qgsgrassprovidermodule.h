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

class QgsGrassLocationItem : public QgsDirectoryItem
{
  public:
    QgsGrassLocationItem( QgsDataItem* parent, QString dirPath, QString path );
    ~QgsGrassLocationItem();

    QIcon icon() override { return QgsDataItem::icon(); }

    static bool isLocation( QString path );
    QVector<QgsDataItem*> createChildren() override;
};

class QgsGrassMapsetItem : public QgsDirectoryItem
{
  public:
    QgsGrassMapsetItem( QgsDataItem* parent, QString dirPath, QString path );
    ~QgsGrassMapsetItem();

    QIcon icon() override { return QgsDataItem::icon(); }

    static bool isMapset( QString path );
    QVector<QgsDataItem*> createChildren() override;

    QString mLocation;
    QString mGisdbase;
};

class QgsGrassVectorLayerItem : public QgsLayerItem
{
  public:
    QgsGrassVectorLayerItem( QgsDataItem* parent, QString mapName, QString layerName, QString path, QString uri, LayerType layerType, QString providerKey );
    ~QgsGrassVectorLayerItem() {};

    QString layerName() const override;

  private:
    QString mMapName;
};

#endif // QGSGRASSPROVIDERMODULE_H
