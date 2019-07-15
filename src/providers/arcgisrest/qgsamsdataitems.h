/***************************************************************************
      qgsamsdataitems.h
      -----------------
    begin                : Nov 26, 2015
    copyright            : (C) 2015 by Sandro Mani
    email                : smani@sourcepole.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSAMSDATAITEMS_H
#define QGSAMSDATAITEMS_H

#include "qgsdataitem.h"
#include "qgsdatasourceuri.h"
#include "qgswkbtypes.h"
#include "qgsdataitemprovider.h"


class QgsAmsRootItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsAmsRootItem( QgsDataItem *parent, const QString &name, const QString &path );

    QVector<QgsDataItem *> createChildren() override;

    QVariant sortKey() const override { return 12; }

#ifdef HAVE_GUI
    QWidget *paramWidget() override;
#endif

  public slots:
#ifdef HAVE_GUI
    void onConnectionsChanged();
#endif
};


class QgsAmsConnectionItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsAmsConnectionItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &url );
    QVector<QgsDataItem *> createChildren() override;
    bool equal( const QgsDataItem *other ) override;
    QString url() const;

  private:
    QString mConnName;
};


class QgsAmsFolderItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsAmsFolderItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &baseUrl, const QString &authcfg, const QgsStringMap &headers );
    QVector<QgsDataItem *> createChildren() override;
    bool equal( const QgsDataItem *other ) override;

  private:
    QString mFolder;
    QString mBaseUrl;
    QString mAuthCfg;
    QgsStringMap mHeaders;
};


class QgsAmsServiceItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsAmsServiceItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &baseUrl, const QString &authcfg, const QgsStringMap &headers );
    QVector<QgsDataItem *> createChildren() override;
    bool equal( const QgsDataItem *other ) override;

  private:
    QString mFolder;
    QString mBaseUrl;
    QString mAuthCfg;
    QgsStringMap mHeaders;
};

class QgsAmsLayerItem : public QgsLayerItem
{
    Q_OBJECT

  public:
    QgsAmsLayerItem( QgsDataItem *parent, const QString &name, const QString &url, const QString &id, const QString &title, const QString &authid, const QString &format, const QString &authcfg, const QgsStringMap &headers );
};


//! Provider for ams root data item
class QgsAmsDataItemProvider : public QgsDataItemProvider
{
  public:
    QString name() override;

    int capabilities() const override;

    QgsDataItem *createDataItem( const QString &path, QgsDataItem *parentItem ) override;
};

#endif // QGSAMSDATAITEMS_H
