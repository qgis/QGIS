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

#include "qgsdataitem.h"
#include "qgsdataitemprovider.h"

class QgsSLLayerItem : public QgsLayerItem
{
    Q_OBJECT
  public:
    QgsSLLayerItem( QgsDataItem *parent, const QString &name, const QString &path, const QString &uri, LayerType layerType );

#ifdef HAVE_GUI
    bool deleteLayer() override;
#endif
};

class QgsSLConnectionItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsSLConnectionItem( QgsDataItem *parent, const QString &name, const QString &path );

    QVector<QgsDataItem *> createChildren() override;
    bool equal( const QgsDataItem *other ) override;

#ifdef HAVE_GUI
    QList<QAction *> actions( QWidget *parent ) override;
#endif

    bool acceptDrop() override { return true; }
    bool handleDrop( const QMimeData *data, Qt::DropAction action ) override;

  public slots:
#ifdef HAVE_GUI
    void editConnection();
    void deleteConnection();
#endif

  protected:
    QString mDbPath;
};

class QgsSLRootItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsSLRootItem( QgsDataItem *parent, const QString &name, const QString &path );

    QVector<QgsDataItem *> createChildren() override;

    QVariant sortKey() const override { return 2; }

#ifdef HAVE_GUI
    QWidget *paramWidget() override;
    QList<QAction *> actions( QWidget *parent ) override;
#endif

  public slots:
#ifdef HAVE_GUI
    void onConnectionsChanged();
    void newConnection();
#endif
    void createDatabase();
};

namespace SpatiaLiteUtils
{
  bool createDb( const QString &dbPath, QString &errCause );
}

//! Provider for SpatiaLite root data item
class QgsSpatiaLiteDataItemProvider : public QgsDataItemProvider
{
  public:
    QString name() override { return QStringLiteral( "SPATIALITE" ); }

    int capabilities() const override { return QgsDataProvider::Database; }

    QgsDataItem *createDataItem( const QString &pathIn, QgsDataItem *parentItem ) override;
};

#endif // QGSSPATIALITEDATAITEMS_H
