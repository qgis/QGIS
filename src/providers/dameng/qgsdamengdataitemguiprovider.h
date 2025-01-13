/***************************************************************************
    qgsdamengdataitemguiprovider.h
    --------------------------------------
    begin                : 2025/01/14
    copyright            : ( C ) 2025 by Haiyang Zhao
    email                : zhaohaiyang@dameng.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   ( at your option ) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSDAMENGDATAITEMGUIPROVIDER_H
#define QGSDAMENGDATAITEMGUIPROVIDER_H

#include "qgsdataitemguiprovider.h"

class QgsDamengSchemaItem;
class QgsDamengLayerItem;

class QgsDamengDataItemGuiProvider : public QObject, public QgsDataItemGuiProvider
{
    Q_OBJECT
  public:

    QString name() override;

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
    static void createSchema( QgsDataItem *item, QgsDataItemGuiContext context );
    static void deleteSchema( QgsDamengSchemaItem *schemaItem, QgsDataItemGuiContext context );
    static void renameLayer( QgsDamengLayerItem *layerItem, QgsDataItemGuiContext context );
    static void truncateTable( QgsDamengLayerItem *layerItem, QgsDataItemGuiContext context );
    static void refreshMaterializedView( QgsDamengLayerItem *layerItem, QgsDataItemGuiContext context );
    static void saveConnections();
    static void loadConnections( QgsDataItem *item );

};

#endif // QGSDAMENGDATAITEMGUIPROVIDER_H
