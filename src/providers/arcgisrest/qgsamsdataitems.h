/***************************************************************************
      qgsamsdataitems.h
      -----------------
    begin                : Nov 26, 2015
    copyright            : (C) 2015 by Sandro Mani
    email                : smani@sourcepole.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSAMSDATAITEMS_H
#define QGSAMSDATAITEMS_H

#include "qgsdataitem.h"
#include "qgsdatasourceuri.h"


class QgsAmsRootItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsAmsRootItem( QgsDataItem* parent, QString name, QString path );

    QVector<QgsDataItem*> createChildren() override;
    virtual QList<QAction*> actions() override;
    virtual QWidget * paramWidget() override;

  public slots:
    void connectionsChanged();
    void newConnection();
};


class QgsAmsConnectionItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsAmsConnectionItem( QgsDataItem* parent, QString name, QString path, QString url );
    QVector<QgsDataItem*> createChildren() override;
    bool equal( const QgsDataItem *other ) override;
    QList<QAction*> actions() override;

  public slots:
    void editConnection();
    void deleteConnection();

  private:
    QString mUrl;
};

class QgsAmsLayerItem : public QgsLayerItem
{
  public:
    QgsAmsLayerItem( QgsDataItem* parent, const QString& name, const QString &url, const QString& id, const QString& title, const QString& authid, const QString& format );
};

#endif // QGSAMSDATAITEMS_H
