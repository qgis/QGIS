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

#ifdef HAVE_GUI
#include "qgsdataitemguiprovider.h"
#endif

class QgsAfsRootItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsAfsRootItem( QgsDataItem *parent, const QString &name, const QString &path );
    QVector<QgsDataItem *> createChildren() override;

    QVariant sortKey() const override { return 13; }

#ifdef HAVE_GUI
    QList<QAction *> actions( QWidget *parent ) override;
    QWidget *paramWidget() override;
#endif

  public slots:
#ifdef HAVE_GUI
    void onConnectionsChanged();
    void newConnection();
#endif
};


class QgsAfsConnectionItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsAfsConnectionItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &connectionName );
    QVector<QgsDataItem *> createChildren() override;
    bool equal( const QgsDataItem *other ) override;
    QString url() const;
#ifdef HAVE_GUI
    QList<QAction *> actions( QWidget *parent ) override;
#endif

  public slots:
#ifdef HAVE_GUI
    void editConnection();
    void deleteConnection();
    void refreshConnection();
#endif

  private:
    QString mConnName;
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
    QString name() override { return QStringLiteral( "AFS" ); }

    int capabilities() override { return QgsDataProvider::Net; }

    QgsDataItem *createDataItem( const QString &path, QgsDataItem *parentItem ) override;
};

#ifdef HAVE_GUI

class QgsAfsItemGuiProvider : public QObject, public QgsDataItemGuiProvider
{
    Q_OBJECT

  public:

    QgsAfsItemGuiProvider() = default;

    QString name() override;

    void populateContextMenu( QgsDataItem *item, QMenu *menu,
                              const QList<QgsDataItem *> &selectedItems, QgsDataItemGuiContext context ) override;


};

#endif

#endif // QGSAFSDATAITEMS_H
