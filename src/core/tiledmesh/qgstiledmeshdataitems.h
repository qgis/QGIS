/***************************************************************************
    qgstiledmeshdataitems.h
    ---------------------
    begin                : June 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSTILEDMESHDATAITEMS_H
#define QGSTILEDMESHDATAITEMS_H

#include "qgsconnectionsitem.h"
#include "qgslayeritem.h"
#include "qgsdataitemprovider.h"

///@cond PRIVATE
#define SIP_NO_FILE

//! Root item for tiled mesh connections
class CORE_EXPORT QgsTiledMeshRootItem : public QgsConnectionsRootItem
{
    Q_OBJECT
  public:
    QgsTiledMeshRootItem( QgsDataItem *parent, QString name, QString path );

    QVector<QgsDataItem *> createChildren() override;

    QVariant sortKey() const override { return 8; }

};

//! Item implementation for tiled mesh layers
class CORE_EXPORT QgsTiledMeshLayerItem : public QgsLayerItem
{
    Q_OBJECT
  public:
    QgsTiledMeshLayerItem( QgsDataItem *parent, QString name, QString path, const QString &encodedUri, const QString &provider );

};


//! Provider for tiled mesh root data item
class QgsTiledMeshDataItemProvider : public QgsDataItemProvider
{
  public:
    QString name() override;
    QString dataProviderKey() const override;
    int capabilities() const override;

    QgsDataItem *createDataItem( const QString &path, QgsDataItem *parentItem ) override;
};

///@endcond

#endif // QGSTILEDMESHDATAITEMS_H
