/***************************************************************************
  qgsmssqldataitemguiprovider.h
  --------------------------------------
  Date                 : June 2019
  Copyright            : (C) 2019 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMSSQLDATAITEMGUIPROVIDER_H
#define QGSMSSQLDATAITEMGUIPROVIDER_H

#include "qgsdataitemguiprovider.h"

class QgsMssqlConnectionItem;
class QgsMssqlLayerItem;

class QgsMssqlDataItemGuiProvider : public QObject, public QgsDataItemGuiProvider
{
    Q_OBJECT
  public:
    QString name() override { return QStringLiteral( "MSSQL" ); }

    void populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &selectedItems, QgsDataItemGuiContext context ) override;

    bool deleteLayer( QgsLayerItem *item, QgsDataItemGuiContext context ) override;

    bool acceptDrop( QgsDataItem *item, QgsDataItemGuiContext context ) override;
    bool handleDrop( QgsDataItem *item, QgsDataItemGuiContext context, const QMimeData *data, Qt::DropAction action ) override;

  private:
    static void newConnection( QgsDataItem *item );
    static void editConnection( QgsDataItem *item );
    static void duplicateConnection( QgsDataItem *item );
    static void createSchema( QgsMssqlConnectionItem *connItem );
    static void truncateTable( QgsMssqlLayerItem *layerItem );
    static void saveConnections();
    static void loadConnections( QgsDataItem *item );
};

#endif // QGSMSSQLDATAITEMGUIPROVIDER_H
