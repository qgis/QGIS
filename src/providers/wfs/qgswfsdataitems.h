/***************************************************************************
    qgswfsdataitems.h
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
#ifndef QGSWFSDATAITEMS_H
#define QGSWFSDATAITEMS_H

#include "qgsdataitem.h"
#include "qgsdataitemprovider.h"
#include "qgsdataprovider.h"
#include "qgsdatasourceuri.h"
#include "qgswfscapabilities.h"

class QgsWfsRootItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsWfsRootItem( QgsDataItem *parent, QString name, QString path );
    ~QgsWfsRootItem();

    QVector<QgsDataItem *> createChildren() override;

#ifdef HAVE_GUI
    virtual QList<QAction *> actions() override;
    virtual QWidget *paramWidget() override;
#endif

  public slots:
#ifdef HAVE_GUI
    void connectionsChanged();
    void newConnection();
#endif
};

class QgsWfsConnection;

class QgsWfsConnectionItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsWfsConnectionItem( QgsDataItem *parent, QString name, QString path, QString uri );
    ~QgsWfsConnectionItem();

    QVector<QgsDataItem *> createChildren() override;
    //virtual bool equal( const QgsDataItem *other );

#ifdef HAVE_GUI
    virtual QList<QAction *> actions() override;
#endif

  private slots:
#ifdef HAVE_GUI
    void editConnection();
    void deleteConnection();
#endif

  private:
    QString mUri;

    QgsWfsCapabilities *mWfsCapabilities = nullptr;
};


class QgsWfsLayerItem : public QgsLayerItem
{
    Q_OBJECT

  public:
    QgsWfsLayerItem( QgsDataItem *parent, QString name, const QgsDataSourceUri &uri, QString featureType, QString title, QString crsString );
    ~QgsWfsLayerItem();

};


//! Provider for WFS root data item
class QgsWfsDataItemProvider : public QgsDataItemProvider
{
  public:
    virtual QString name() override { return QStringLiteral( "WFS" ); }

    virtual int capabilities() override { return QgsDataProvider::Net; }

    virtual QgsDataItem *createDataItem( const QString &path, QgsDataItem *parentItem ) override;

    virtual QVector<QgsDataItem *> createDataItems( const QString &path, QgsDataItem *parentItem ) override;
};


#endif // QGSWFSDATAITEMS_H
