/***************************************************************************
    qgsgdalclouddataitems.h
    ---------------------
    begin                : June 2024
    copyright            : (C) 2024 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSGDALCLOUDDATAITEMS_H
#define QGSGDALCLOUDDATAITEMS_H

#include "qgsconnectionsitem.h"
#include "qgsdataitemprovider.h"
#include "qgsgdalutils.h"

///@cond PRIVATE
#define SIP_NO_FILE

//! Root item for GDAL cloud connections
class CORE_EXPORT QgsGdalCloudRootItem : public QgsConnectionsRootItem
{
    Q_OBJECT
  public:
    QgsGdalCloudRootItem( QgsDataItem *parent, QString name, QString path );

    QVector<QgsDataItem *> createChildren() override;

    QVariant sortKey() const override { return 8; }

};

//! Generic item for individual GDAL cloud providers, eg "Amazon S3", "Microsoft Azure"
class CORE_EXPORT QgsGdalCloudProviderItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsGdalCloudProviderItem( QgsDataItem *parent, const QgsGdalUtils::VsiNetworkFileSystemDetails &handler );
    QVector<QgsDataItem *> createChildren() override;
    const QgsGdalUtils::VsiNetworkFileSystemDetails &vsiHandler() const { return mVsiHandler; }

  private:
    QgsGdalUtils::VsiNetworkFileSystemDetails mVsiHandler;

};

//! Item for a specific connection to a cloud container
class CORE_EXPORT QgsGdalCloudConnectionItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsGdalCloudConnectionItem( QgsDataItem *parent, const QString &name, const QString &path );
    bool equal( const QgsDataItem *other ) override;
    QVector<QgsDataItem *> createChildren() override;
  private:
    QString mConnName;
};

//! Item for a directory within a cloud container
class CORE_EXPORT QgsGdalCloudDirectoryItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsGdalCloudDirectoryItem( QgsDataItem *parent, QString name, QString path, const QString &connectionName, const QString &directory );
    QVector<QgsDataItem *> createChildren() override;
  private:

    QString mConnName;
    QString mDirectory;
};

//! Provider for GDAL cloud root data item
class QgsGdalCloudDataItemProvider : public QgsDataItemProvider
{
  public:
    QString name() override;
    QString dataProviderKey() const override;
    Qgis::DataItemProviderCapabilities capabilities() const override;
    QgsDataItem *createDataItem( const QString &path, QgsDataItem *parentItem ) override;
};

///@endcond

#endif // QGSGDALCLOUDDATAITEMS_H
