/***************************************************************************
    qgswfsdataitems.h
    ---------------------
    begin                : October 2011
    copyright            : (C) 2011 by Martin Dobias
    email                : wonder.sk at gmail.com
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

    QVector<QgsDataItem*> createChildren();

    virtual QList<QAction*> actions();

    virtual QWidget * paramWidget();

  public slots:
    void connectionsChanged();

    void newConnection();
};

class QgsWFSConnection;

class QgsWFSConnectionItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsWFSConnectionItem( QgsDataItem* parent, QString name, QString path );
    ~QgsWFSConnectionItem();

    QVector<QgsDataItem*> createChildren();
    //virtual bool equal( const QgsDataItem *other );

    virtual QList<QAction*> actions();

  private slots:
    void gotCapabilities();

    void editConnection();
    void deleteConnection();

  private:
    QString mName;

    QgsWFSCapabilities* mCapabilities;
    bool mGotCapabilities;
};


class QgsWFSLayerItem : public QgsLayerItem
{
  public:
    QgsWFSLayerItem( QgsDataItem* parent, QString name, QgsDataSourceURI uri, QString featureType, QString title );
    ~QgsWFSLayerItem();

};


#endif // QGSWFSDATAITEMS_H
