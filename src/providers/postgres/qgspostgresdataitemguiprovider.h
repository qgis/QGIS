/***************************************************************************
  qgspostgresdataitemguiprovider.h
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

#ifndef QGSPOSTGRESDATAITEMGUIPROVIDER_H
#define QGSPOSTGRESDATAITEMGUIPROVIDER_H

#include "qgsdataitemguiprovider.h"

class QgsPGSchemaItem;
class QgsPGLayerItem;

class QgsPostgresDataItemGuiProvider : public QObject, public QgsDataItemGuiProvider
{
    Q_OBJECT
  public:

    QString name() override { return QStringLiteral( "PostGIS" ); }

    void populateContextMenu( QgsDataItem *item, QMenu *menu,
                              const QList<QgsDataItem *> &selectedItems, QgsDataItemGuiContext context ) override;

    bool deleteLayer( QgsLayerItem *item, QgsDataItemGuiContext context ) override;

    bool acceptDrop( QgsDataItem *item, QgsDataItemGuiContext context ) override;
    bool handleDrop( QgsDataItem *item, QgsDataItemGuiContext context, const QMimeData *data, Qt::DropAction action ) override;

    QWidget *createParamWidget( QgsDataItem *root, QgsDataItemGuiContext ) override;

  private:
    static void newConnection( QgsDataItem *item );
    static void editConnection( QgsDataItem *item );
    static void deleteConnection( QgsDataItem *item );
    static void refreshConnection( QgsDataItem *item );
    static void createSchema( QgsDataItem *item );
    static void deleteSchema( QgsPGSchemaItem *schemaItem );
    static void renameSchema( QgsPGSchemaItem *schemaItem );
    static void renameLayer( QgsPGLayerItem *layerItem );
    static void truncateTable( QgsPGLayerItem *layerItem );
    static void refreshMaterializedView( QgsPGLayerItem *layerItem );

};

#endif // QGSPOSTGRESDATAITEMGUIPROVIDER_H
