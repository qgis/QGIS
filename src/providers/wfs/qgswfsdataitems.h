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

    QVector<QgsDataItem *> createChildren() override;

    QVariant sortKey() const override { return 9; }

#ifdef HAVE_GUI
    QWidget *paramWidget() override;
#endif

  public slots:
#ifdef HAVE_GUI
    void onConnectionsChanged();
#endif
};

class QgsWfsConnection;

class QgsWfsConnectionItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsWfsConnectionItem( QgsDataItem *parent, QString name, QString path, QString uri );

    QVector<QgsDataItem *> createChildren() override;
    //virtual bool equal( const QgsDataItem *other );

  private:
    QString mUri;

    QVector<QgsDataItem *> createChildrenOapif();
};


class QgsWfsLayerItem : public QgsLayerItem
{
    Q_OBJECT

  public:
    QgsWfsLayerItem( QgsDataItem *parent, QString name, const QgsDataSourceUri &uri, QString featureType, QString title, QString crsString, const QString &providerKey );

    QList<QMenu *> menus( QWidget *parent ) override;

  protected:
    QString mBaseUri;

  private slots:

    /**
     * Gets style of the active data item (geonode layer item) and copy it to the clipboard.
     */
    void copyStyle();

    /**
     * Paste style on the clipboard to the active data item (geonode layer item) and push it to the source.
     */
    //    void pasteStyle();
};


//! Provider for WFS root data item
class QgsWfsDataItemProvider : public QgsDataItemProvider
{
  public:
    QString name() override;

    int capabilities() const override;

    QgsDataItem *createDataItem( const QString &path, QgsDataItem *parentItem ) override;

    QVector<QgsDataItem *> createDataItems( const QString &path, QgsDataItem *parentItem ) override;
};


#endif // QGSWFSDATAITEMS_H
