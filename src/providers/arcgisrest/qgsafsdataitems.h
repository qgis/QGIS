/***************************************************************************
      qgsafsdataitems.h
      -----------------
    begin                : Jun 03, 2015
    copyright            : (C) 2015 by Sandro Mani
    email                : smani@sourcepole.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSAFSDATAITEMS_H
#define QGSAFSDATAITEMS_H

#include "qgsdataitem.h"
#include "qgsdatasourceuri.h"
#include "qgswkbtypes.h"
#include "qgsdataitemprovider.h"


class QgsAfsRootItem : public QgsConnectionsRootItem
{
    Q_OBJECT
  public:
    QgsAfsRootItem( QgsDataItem *parent, const QString &name, const QString &path );
    QVector<QgsDataItem *> createChildren() override;

    QVariant sortKey() const override { return 13; }

#ifdef HAVE_GUI
    QWidget *paramWidget() override;
#endif

  public slots:
#ifdef HAVE_GUI
    void onConnectionsChanged();
#endif
};

/**
 * Root item corresponding to a connection.
 *
 * Depending on whether the connection has the Portal endpoints set, child items will either be
 * QgsAfsServiceItems directly, or a QgsAfsPortalGroupsItem and QgsAfsServicesItem allowing the user
 * to explore content by services or by Portal content groups.
 */
class QgsAfsConnectionItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsAfsConnectionItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &connectionName );
    QVector<QgsDataItem *> createChildren() override;
    bool equal( const QgsDataItem *other ) override;
    QString url() const;

  private:
    QString mConnName;
    QString mPortalCommunityEndpoint;
    QString mPortalContentEndpoint;
};

/**
 * A container for all ArcGIS Portal user groups present on a server (which the user belongs to).
 *
 * Child items are QgsAfsPortalGroupItem items for each group.
 *
 * \note This is only used for AFS connections which have the Portal endpoints set.
 */
class QgsAfsPortalGroupsItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsAfsPortalGroupsItem( QgsDataItem *parent, const QString &path, const QString &authcfg, const QgsStringMap &headers,
                            const QString &communityEndpoint, const QString &contentEndpoint );
    QVector<QgsDataItem *> createChildren() override;
    bool equal( const QgsDataItem *other ) override;

  private:
    QString mAuthCfg;
    QgsStringMap mHeaders;
    QString mPortalCommunityEndpoint;
    QString mPortalContentEndpoint;
};

/**
 * Represents a single ArcGIS Portal user group. Child items are QgsAfsServiceItem representing each feature service
 * present within the group.
 *
 * \note This is only used for AFS connections which have the Portal endpoints set.
 */
class QgsAfsPortalGroupItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsAfsPortalGroupItem( QgsDataItem *parent, const QString &groupId, const QString &name, const QString &authcfg, const QgsStringMap &headers,
                           const QString &communityEndpoint, const QString &contentEndpoint );
    QVector<QgsDataItem *> createChildren() override;
    bool equal( const QgsDataItem *other ) override;

  private:
    QString mId;
    QString mAuthCfg;
    QgsStringMap mHeaders;
    QString mPortalCommunityEndpoint;
    QString mPortalContentEndpoint;
};


/**
 * A container for all ArcGIS Portal services present on a server.
 *
 * Child items are QgsAfsServiceItem items.
 *
 * \note This is only used for AFS connections which have the Portal endpoints set.
 */
class QgsAfsServicesItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsAfsServicesItem( QgsDataItem *parent, const QString &url, const QString &path, const QString &authcfg, const QgsStringMap &headers );
    QVector<QgsDataItem *> createChildren() override;
    bool equal( const QgsDataItem *other ) override;

  private:
    QString mUrl;
    QString mAuthCfg;
    QgsStringMap mHeaders;
    QString mPortalCommunityEndpoint;
    QString mPortalContentEndpoint;
};


class QgsAfsFolderItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsAfsFolderItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &baseUrl, const QString &authcfg, const QgsStringMap &headers );
    QVector<QgsDataItem *> createChildren() override;
    bool equal( const QgsDataItem *other ) override;

  private:
    QString mFolder;
    QString mBaseUrl;
    QString mAuthCfg;
    QgsStringMap mHeaders;
};

class QgsAfsServiceItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsAfsServiceItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &baseUrl, const QString &authcfg, const QgsStringMap &headers );
    QVector<QgsDataItem *> createChildren() override;
    bool equal( const QgsDataItem *other ) override;

  private:
    QString mFolder;
    QString mBaseUrl;
    QString mAuthCfg;
    QgsStringMap mHeaders;
};

class QgsAfsParentLayerItem : public QgsDataItem
{
    Q_OBJECT
  public:

    QgsAfsParentLayerItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &authcfg, const QgsStringMap &headers );
    bool equal( const QgsDataItem *other ) override;

  private:

    QString mAuthCfg;
    QgsStringMap mHeaders;

};

class QgsAfsLayerItem : public QgsLayerItem
{
    Q_OBJECT

  public:

    QgsAfsLayerItem( QgsDataItem *parent, const QString &name, const QString &url, const QString &title, const QString &authid, const QString &authcfg, const QgsStringMap &headers );

};

//! Provider for afs root data item
class QgsAfsDataItemProvider : public QgsDataItemProvider
{
  public:
    QString name() override;

    int capabilities() const override;

    QgsDataItem *createDataItem( const QString &path, QgsDataItem *parentItem ) override;
};

#endif // QGSAFSDATAITEMS_H
