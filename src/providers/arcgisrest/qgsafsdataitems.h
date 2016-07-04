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


class QgsAfsRootItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsAfsRootItem( QgsDataItem* parent, const QString& name, const QString& path );
    QVector<QgsDataItem*> createChildren() override;
    QList<QAction*> actions() override;
    QWidget * paramWidget() override;

  public slots:
    void connectionsChanged();
    void newConnection();
};


class QgsAfsConnectionItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsAfsConnectionItem( QgsDataItem* parent, const QString& name, const QString& path, const QString& url );
    QVector<QgsDataItem*> createChildren() override;
    bool equal( const QgsDataItem *other ) override;
    QList<QAction*> actions() override;

  public slots:
    void editConnection();
    void deleteConnection();

  private:
    QString mUrl;
};


class QgsAfsLayerItem : public QgsLayerItem
{
  public:
    QgsAfsLayerItem( QgsDataItem* parent, const QString& name, const QString &url, const QString& title , const QString &authid );
};

#endif // QGSAFSDATAITEMS_H
