/***************************************************************************
    qgsspatialitedataitems.h
    ---------------------
    begin                : October 2011
    copyright            : (C) 2011 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSSPATIALITEDATAITEMS_H
#define QGSSPATIALITEDATAITEMS_H

#include "qgslayeritem.h"
#include "qgsdatacollectionitem.h"
#include "qgsdataitemprovider.h"
#include "qgsconnectionsitem.h"
#include "qgsconfig.h"

class QgsSLLayerItem : public QgsLayerItem
{
    Q_OBJECT
  public:
    QgsSLLayerItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &uri, Qgis::BrowserLayerType layerType );

    // QgsDataItem interface
    QVector<QgsDataItem *> createChildren() override;
};


class QgsSLConnectionItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsSLConnectionItem( QgsDataItem *parent, const QString &name, const QString &path );

    QVector<QgsDataItem *> createChildren() override;
    bool equal( const QgsDataItem *other ) override;

    QString databasePath() const { return mDbPath; }

  protected:
    QString mDbPath;

    // QgsDataItem interface
  public:
    bool layerCollection() const override;
};

class QgsSLRootItem : public QgsConnectionsRootItem
{
    Q_OBJECT
  public:
    QgsSLRootItem( QgsDataItem *parent, const QString &name, const QString &path );

    QVector<QgsDataItem *> createChildren() override;

    QVariant sortKey() const override { return 2; }

#ifdef HAVE_GUI
    QWidget *paramWidget() override;
#endif

  public slots:
#ifdef HAVE_GUI
    void onConnectionsChanged();
#endif
};

namespace SpatiaLiteUtils
{
  bool createDb( const QString &dbPath, QString &errCause );
  bool deleteLayer( const QString &dbPath, const QString &tableName, QString &errCause );
} // namespace SpatiaLiteUtils

//! Provider for SpatiaLite root data item
class QgsSpatiaLiteDataItemProvider : public QgsDataItemProvider
{
  public:
    QString name() override;
    QString dataProviderKey() const override;
    Qgis::DataItemProviderCapabilities capabilities() const override;
    QgsDataItem *createDataItem( const QString &pathIn, QgsDataItem *parentItem ) override;
};

#endif // QGSSPATIALITEDATAITEMS_H
