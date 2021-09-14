/***************************************************************************
      qgsarcgisrestdataitems.h
      -----------------
    begin                : December 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSARCGISRESTDATAITEMS_H
#define QGSARCGISRESTDATAITEMS_H

#include "qgsconnectionsitem.h"
#include "qgsdatacollectionitem.h"
#include "qgsdatasourceuri.h"
#include "qgswkbtypes.h"
#include "qgsdataitemprovider.h"
#include "qgslayeritem.h"
#include "qgsconfig.h"
#include "qgshttpheaders.h"


class QgsArcGisRestRootItem : public QgsConnectionsRootItem
{
    Q_OBJECT
  public:
    QgsArcGisRestRootItem( QgsDataItem *parent, const QString &name, const QString &path );
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
 * QgsArcGisFeatureServiceItems directly, or a QgsArcGisPortalGroupsItem and QgsArcGisRestServicesItem allowing the user
 * to explore content by services or by Portal content groups.
 */
class QgsArcGisRestConnectionItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsArcGisRestConnectionItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &connectionName );
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
 * Child items are QgsArcGisPortalGroupItem items for each group.
 *
 * \note This is only used for ArcGIS REST connections which have the Portal endpoints set.
 */
class QgsArcGisPortalGroupsItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsArcGisPortalGroupsItem( QgsDataItem *parent, const QString &path, const QString &authcfg, const QgsHttpHeaders &headers,
                               const QString &communityEndpoint, const QString &contentEndpoint );
    QVector<QgsDataItem *> createChildren() override;
    bool equal( const QgsDataItem *other ) override;

  private:
    QString mAuthCfg;
    QgsHttpHeaders mHeaders;
    QString mPortalCommunityEndpoint;
    QString mPortalContentEndpoint;
};

/**
 * Represents a single ArcGIS Portal user group. Child items are QgsArcGisFeatureServiceItems or QgsArcGisMapServiceItem representing each feature service
 * present within the group.
 *
 * \note This is only used for ArcGIS REST connections which have the Portal endpoints set.
 */
class QgsArcGisPortalGroupItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsArcGisPortalGroupItem( QgsDataItem *parent, const QString &groupId, const QString &name, const QString &authcfg, const QgsHttpHeaders &headers,
                              const QString &communityEndpoint, const QString &contentEndpoint );
    QVector<QgsDataItem *> createChildren() override;
    bool equal( const QgsDataItem *other ) override;

  private:
    QString mId;
    QString mAuthCfg;
    QgsHttpHeaders mHeaders;
    QString mPortalCommunityEndpoint;
    QString mPortalContentEndpoint;
};


/**
 * A container for all ArcGIS Portal services present on a server.
 *
 * Child items are QgsArcGisFeatureServiceItem or QgsArcGisMapServiceItem items.
 *
 * \note This is only used for ArcGIS REST connections which have the Portal endpoints set.
 */
class QgsArcGisRestServicesItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsArcGisRestServicesItem( QgsDataItem *parent, const QString &url, const QString &path, const QString &authcfg, const QgsHttpHeaders &headers );
    QVector<QgsDataItem *> createChildren() override;
    bool equal( const QgsDataItem *other ) override;

  private:
    QString mUrl;
    QString mAuthCfg;
    QgsHttpHeaders mHeaders;
    QString mPortalCommunityEndpoint;
    QString mPortalContentEndpoint;
};

/**
 * A "folder" container for ArcGIS REST services and layers present on a server.
 *
 * Child items are usually QgsArcGisFeatureServiceItem or QgsArcGisMapServiceItem items, but sometimes servers
 * have nested structures and QgsArcGisRestFolderItems may also be present as children.
 */
class QgsArcGisRestFolderItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsArcGisRestFolderItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &baseUrl, const QString &authcfg, const QgsHttpHeaders &headers );
    void setSupportedFormats( const QString &formats );

    QVector<QgsDataItem *> createChildren() override;
    bool equal( const QgsDataItem *other ) override;

  private:
    QString mFolder;
    QString mBaseUrl;
    QString mAuthCfg;
    QgsHttpHeaders mHeaders;
    QString mSupportedFormats;
};


/**
 * Represents a ArcGIS REST "Feature Service" item.
 *
 * Usually has no child items, but sometimes services are nested and will contain other QgsArcGisFeatureServiceItem children
 * or QgsArcGisRestFolderItem children.
 */
class QgsArcGisFeatureServiceItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsArcGisFeatureServiceItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &baseUrl, const QString &authcfg, const QgsHttpHeaders &headers );
    void setSupportedFormats( const QString &formats );
    QVector<QgsDataItem *> createChildren() override;
    bool equal( const QgsDataItem *other ) override;

  private:
    QString mFolder;
    QString mBaseUrl;
    QString mAuthCfg;
    QgsHttpHeaders mHeaders;
    QString mSupportedFormats;
};

/**
 * Represents a ArcGIS REST "Map Service" (or "Image Service") item.
 *
 * Usually has no child items, but sometimes services are nested and will contain other QgsArcGisMapServiceItem children
 * or QgsArcGisRestFolderItem children.
 */
class QgsArcGisMapServiceItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsArcGisMapServiceItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &baseUrl, const QString &authcfg, const QgsHttpHeaders &headers, const QString &serviceType );
    QVector<QgsDataItem *> createChildren() override;
    bool equal( const QgsDataItem *other ) override;

  private:
    QString mFolder;
    QString mBaseUrl;
    QString mAuthCfg;
    QgsHttpHeaders mHeaders;
    QString mServiceType;
};

/**
 * Represents a "parent layer" containing one or more QgsArcGisFeatureServiceItem or QgsArcGisMapServiceItem children.
 */
class QgsArcGisRestParentLayerItem : public QgsDataItem
{
    Q_OBJECT
  public:

    QgsArcGisRestParentLayerItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &authcfg, const QgsHttpHeaders &headers );
    bool equal( const QgsDataItem *other ) override;

  private:

    QString mAuthCfg;
    QgsHttpHeaders mHeaders;

};

/**
 * Represents a ArcGIS REST "Feature Service" layer item.
 */
class QgsArcGisFeatureServiceLayerItem : public QgsLayerItem
{
    Q_OBJECT

  public:

    QgsArcGisFeatureServiceLayerItem( QgsDataItem *parent, const QString &name, const QString &url, const QString &title, const QString &authid, const QString &authcfg, const QgsHttpHeaders &headers,
                                      Qgis::BrowserLayerType geometryType );

};

/**
 * Represents a ArcGIS REST "Map Service" (or "Image Service") layer item.
 */

class QgsArcGisMapServiceLayerItem : public QgsLayerItem
{
    Q_OBJECT

  public:
    QgsArcGisMapServiceLayerItem( QgsDataItem *parent, const QString &name, const QString &url, const QString &id, const QString &title, const QString &authid, const QString &format, const QString &authcfg, const QgsHttpHeaders &headers );
    void setSupportedFormats( const QString &formats ) { mSupportedFormats = formats; }
    QString supportedFormats() const { return mSupportedFormats; }

  private:

    QString mSupportedFormats;
};


//! Provider for ArcGIS REST root data item
class QgsArcGisRestDataItemProvider : public QgsDataItemProvider
{
  public:

    QgsArcGisRestDataItemProvider();

    QString name() override;

    int capabilities() const override;

    QgsDataItem *createDataItem( const QString &path, QgsDataItem *parentItem ) override;
};

#endif // QGSARCGISRESTDATAITEMS_H
