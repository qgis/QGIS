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

class QgsSLLayerItem : public QgsLayerItem
{
    Q_OBJECT
  public:
    QgsSLLayerItem( QgsDataItem* parent, QString name, QString path, QString uri, LayerType layerType );

    QList<QAction*> actions() override;

  public slots:
    void deleteLayer();
};

class QgsSLConnectionItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsSLConnectionItem( QgsDataItem* parent, QString name, QString path );
    ~QgsSLConnectionItem();

    QVector<QgsDataItem*> createChildren() override;
    virtual bool equal( const QgsDataItem *other ) override;

    virtual QList<QAction*> actions() override;

    virtual bool acceptDrop() override { return true; }
    virtual bool handleDrop( const QMimeData * data, Qt::DropAction action ) override;

  public slots:
    void editConnection();
    void deleteConnection();

  protected:
    QString mDbPath;
};

class QgsSLRootItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsSLRootItem( QgsDataItem* parent, QString name, QString path );
    ~QgsSLRootItem();

    QVector<QgsDataItem*> createChildren() override;

    virtual QWidget * paramWidget() override;

    virtual QList<QAction*> actions() override;

  public slots:
    void connectionsChanged();
    void newConnection();
    void createDatabase();
};


#endif // QGSSPATIALITEDATAITEMS_H
