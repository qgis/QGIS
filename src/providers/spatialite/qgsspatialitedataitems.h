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
class SpatialiteDbInfo;

class QgsSLLayerItem : public QgsLayerItem
{
    Q_OBJECT
  public:
    QgsSLLayerItem( QgsDataItem *parent, QString name, QString path, QString uri, LayerType layerType );
    QgsSLLayerItem( QgsDataItem *parent, QString path, QString sLayerName, QString sLayerInfo, SpatialiteDbInfo *spatialiteDbInfo );

#ifdef HAVE_GUI
    QList<QAction *> actions() override;
#endif

  public slots:
#ifdef HAVE_GUI
    void deleteLayer();
#endif
  private:
    SpatialiteDbInfo *mSpatialiteDbInfo = nullptr;
};

class QgsSLConnectionItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsSLConnectionItem( QgsDataItem *parent, QString name, QString path );

    QVector<QgsDataItem *> createChildren() override;
    virtual bool equal( const QgsDataItem *other ) override;

#ifdef HAVE_GUI
    virtual QList<QAction *> actions() override;
#endif

    virtual bool acceptDrop() override { return true; }
    virtual bool handleDrop( const QMimeData *data, Qt::DropAction action ) override;

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
    QgsSLRootItem( QgsDataItem *parent, QString name, QString path );

    QVector<QgsDataItem *> createChildren() override;

#ifdef HAVE_GUI
    virtual QWidget *paramWidget() override;
    virtual QList<QAction *> actions() override;
#endif

  public slots:
#ifdef HAVE_GUI
    void connectionsChanged();
    void newConnection();
#endif
    void createDatabase();
};


#endif // QGSSPATIALITEDATAITEMS_H
