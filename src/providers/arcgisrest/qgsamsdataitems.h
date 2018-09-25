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
#include "qgswkbtypes.h"


class QgsAmsRootItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsAmsRootItem( QgsDataItem *parent, QString name, QString path );

    QVector<QgsDataItem *> createChildren() override;

    QVariant sortKey() const override { return 12; }

#ifdef HAVE_GUI
    QList<QAction *> actions( QWidget *parent ) override;
    QWidget *paramWidget() override;
#endif

  public slots:
#ifdef HAVE_GUI
    void onConnectionsChanged();
    void newConnection();
#endif
};


class QgsAmsConnectionItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsAmsConnectionItem( QgsDataItem *parent, QString name, QString path, QString url );
    QVector<QgsDataItem *> createChildren() override;
    bool equal( const QgsDataItem *other ) override;
#ifdef HAVE_GUI
    QList<QAction *> actions( QWidget *parent ) override;
#endif

  public slots:
#ifdef HAVE_GUI
    void editConnection();
    void deleteConnection();
#endif

  private:
    QString mUrl;
};

class QgsAmsLayerItem : public QgsLayerItem
{
    Q_OBJECT

  public:
    QgsAmsLayerItem( QgsDataItem *parent, const QString &name, const QString &url, const QString &id, const QString &title, const QString &authid, const QString &format );
};

#endif // QGSAMSDATAITEMS_H
