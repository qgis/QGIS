/***************************************************************************
   qgsredshiftdataitemguiprovider.h
   --------------------------------------
   Date      : 16.02.2021
   Copyright : (C) 2021 Amazon Inc. or its affiliates
   Author    : Marcel Bezdrighin
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#ifndef QGSREDSHIFTDATAITEMGUIPROVIDER_H
#define QGSREDSHIFTDATAITEMGUIPROVIDER_H

#include "qgsdataitemguiprovider.h"

class QgsRSSchemaItem;
class QgsRSLayerItem;

class QgsRedshiftDataItemGuiProvider : public QObject, public QgsDataItemGuiProvider
{
    Q_OBJECT
  public:
    QString name() override
    {
      return QStringLiteral( "Redshift Spatial" );
    }

    void populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &selectedItems,
                              QgsDataItemGuiContext context ) override;

    bool deleteLayer( QgsLayerItem *item, QgsDataItemGuiContext context ) override;

    bool acceptDrop( QgsDataItem *item, QgsDataItemGuiContext context ) override;
    bool handleDrop( QgsDataItem *item, QgsDataItemGuiContext context, const QMimeData *data,
                     Qt::DropAction action ) override;

    QWidget *createParamWidget( QgsDataItem *root, QgsDataItemGuiContext ) override;

  private:
    static void newConnection( QgsDataItem *item );
    static void editConnection( QgsDataItem *item );
    static void deleteConnection( QgsDataItem *item );
    static void refreshConnection( QgsDataItem *item );
    static void createSchema( QgsDataItem *item );
    static void deleteSchema( QgsRSSchemaItem *schemaItem );
    static void renameSchema( QgsRSSchemaItem *schemaItem );
    static void renameLayer( QgsRSLayerItem *layerItem );
    static void truncateTable( QgsRSLayerItem *layerItem );
    static void refreshMaterializedView( QgsRSLayerItem *layerItem );
    static void saveConnections();
    static void loadConnections( QgsDataItem *item );
};

#endif // QGSREDSHIFTDATAITEMGUIPROVIDER_H
