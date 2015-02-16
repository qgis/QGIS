/***************************************************************************
    qgswcsdataitems.h
    ---------------------
    begin                : 2 July, 2012
    copyright            : (C) 2012 by Radim Blazek
    email                : radim dot blazek at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSWCSDATAITEMS_H
#define QGSWCSDATAITEMS_H

#include "qgsdataitem.h"
#include "qgsdatasourceuri.h"
#include "qgswcscapabilities.h"

class QgsWCSConnectionItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsWCSConnectionItem( QgsDataItem* parent, QString name, QString path, QString uri );
    ~QgsWCSConnectionItem();

    QVector<QgsDataItem*> createChildren() override;
    virtual bool equal( const QgsDataItem *other ) override;

    virtual QList<QAction*> actions() override;

    QgsWcsCapabilities mCapabilities;
    QVector<QgsWcsCoverageSummary> mLayerProperties;

  public slots:
    void editConnection();
    void deleteConnection();

  private:
    QString mUri;
};

// WCS Layers may be nested, so that they may be both QgsDataCollectionItem and QgsLayerItem
// We have to use QgsDataCollectionItem and support layer methods if necessary
class QgsWCSLayerItem : public QgsLayerItem
{
    Q_OBJECT
  public:
    QgsWCSLayerItem( QgsDataItem* parent, QString name, QString path,
                     QgsWcsCapabilitiesProperty capabilitiesProperty, QgsDataSourceURI dataSourceUri, const QgsWcsCoverageSummary& coverageSummary );
    ~QgsWCSLayerItem();

    QString createUri();

    QgsWcsCapabilitiesProperty mCapabilities;
    QgsDataSourceURI mDataSourceUri;
    QgsWcsCoverageSummary mCoverageSummary;
};

class QgsWCSRootItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsWCSRootItem( QgsDataItem* parent, QString name, QString path );
    ~QgsWCSRootItem();

    QVector<QgsDataItem*> createChildren() override;

    virtual QList<QAction*> actions() override;

    virtual QWidget * paramWidget() override;

  public slots:
    void connectionsChanged();

    void newConnection();
};

#endif // QGSWCSDATAITEMS_H
