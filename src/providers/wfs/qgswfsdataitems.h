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

class QgsWfsRootItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsWfsRootItem( QgsDataItem* parent, QString name, QString path );
    ~QgsWfsRootItem();

    QVector<QgsDataItem*> createChildren() override;

    virtual QList<QAction*> actions() override;

    virtual QWidget * paramWidget() override;

  public slots:
    void connectionsChanged();
    void newConnection();
};

class QgsWfsConnection;

class QgsWfsConnectionItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsWfsConnectionItem( QgsDataItem* parent, QString name, QString path, QString uri );
    ~QgsWfsConnectionItem();

    QVector<QgsDataItem*> createChildren() override;
    //virtual bool equal( const QgsDataItem *other );

    virtual QList<QAction*> actions() override;

  private slots:
    void editConnection();
    void deleteConnection();

  private:
    QString mUri;

    QgsWfsCapabilities* mCapabilities;
};


class QgsWfsLayerItem : public QgsLayerItem
{
    Q_OBJECT

  public:
    QgsWfsLayerItem( QgsDataItem* parent, QString name, const QgsDataSourceUri &uri, QString featureType, QString title, QString crsString );
    ~QgsWfsLayerItem();

};


#endif // QGSWFSDATAITEMS_H
