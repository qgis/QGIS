/***************************************************************************
    qgssensorthingsdataitems.h
    ---------------------
    begin                : December 2023
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
#ifndef QGSSENSORTHINGSDATAITEMS_H
#define QGSSENSORTHINGSDATAITEMS_H

#include "qgsconnectionsitem.h"
#include "qgslayeritem.h"
#include "qgsdataitemprovider.h"

///@cond PRIVATE
#define SIP_NO_FILE

//! Root item for sensorthings connections
class CORE_EXPORT QgsSensorThingsRootItem : public QgsConnectionsRootItem
{
    Q_OBJECT
  public:
    QgsSensorThingsRootItem( QgsDataItem *parent, QString name, QString path );

    QVector<QgsDataItem *> createChildren() override;

    QVariant sortKey() const override { return 8; }

};

//! Item implementation for sensorthings connections
class CORE_EXPORT QgsSensorThingsConnectionItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsSensorThingsConnectionItem( QgsDataItem *parent, const QString &name, const QString &path );
    bool equal( const QgsDataItem *other ) override;
    QVector<QgsDataItem *> createChildren() override;
  private:
    QString mConnName;
};

//! Item implementation for sensorthings entities which contain children
class CORE_EXPORT QgsSensorThingsEntityContainerItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsSensorThingsEntityContainerItem( QgsDataItem *parent, const QString &name, const QString &path, const QVariantMap &entityUriParts,
                                        Qgis::SensorThingsEntity entityType, const QString &connectionName );
    bool equal( const QgsDataItem *other ) override;
    QVector<QgsDataItem *> createChildren() override;
  private:
    QVariantMap mEntityUriParts;
    Qgis::SensorThingsEntity mEntityType = Qgis::SensorThingsEntity::Invalid;
    QString mConnectionName;
};


class CORE_EXPORT QgsSensorThingsLayerEntityItem : public QgsLayerItem
{
    Q_OBJECT
  public:
    QgsSensorThingsLayerEntityItem( QgsDataItem *parent, const QString &name, const QString &path,
                                    const QVariantMap &uriParts, const QString &provider, Qgis::BrowserLayerType type,
                                    Qgis::SensorThingsEntity entityType, const QString &connectionName );
    QString layerName() const final;
  private:
    QVariantMap mUriParts;
    Qgis::SensorThingsEntity mEntityType = Qgis::SensorThingsEntity::Invalid;
    QString mConnectionName;
};

//! Provider for sensor things root data item
class QgsSensorThingsDataItemProvider : public QgsDataItemProvider
{
  public:
    QString name() override;
    QString dataProviderKey() const override;
    Qgis::DataItemProviderCapabilities capabilities() const override;

    QgsDataItem *createDataItem( const QString &path, QgsDataItem *parentItem ) override;
};

///@endcond

#endif // QGSSENSORTHINGSDATAITEMS_H
