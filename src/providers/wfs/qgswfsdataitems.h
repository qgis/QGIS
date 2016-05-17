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
#include "qgsdatasourceuri.h"
#include "qgswfscapabilities.h"

class QgsWFSRootItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsWFSRootItem( QgsDataItem* parent, QString name, QString path );
    ~QgsWFSRootItem();

    QVector<QgsDataItem*> createChildren() override;

    virtual QList<QAction*> actions() override;

    virtual QWidget * paramWidget() override;

  public slots:
    void connectionsChanged();
    void newConnection();
};

class QgsWFSConnection;

class QgsWFSConnectionItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsWFSConnectionItem( QgsDataItem* parent, QString name, QString path, QString uri );
    ~QgsWFSConnectionItem();

    QVector<QgsDataItem*> createChildren() override;
    //virtual bool equal( const QgsDataItem *other );

    virtual QList<QAction*> actions() override;

  private slots:
    void editConnection();
    void deleteConnection();

  private:
    QString mUri;

    QgsWFSCapabilities* mCapabilities;
};


class QgsWFSLayerItem : public QgsLayerItem
{
    Q_OBJECT

  public:
    QgsWFSLayerItem( QgsDataItem* parent, QString name, const QgsDataSourceURI &uri, QString featureType, QString title, QString crsString );
    ~QgsWFSLayerItem();

};


#endif // QGSWFSDATAITEMS_H
