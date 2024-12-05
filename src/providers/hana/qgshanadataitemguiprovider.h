/***************************************************************************
   qgshanadataitemguiprovider.h
   --------------------------------------
   Date      : 31-05-2019
   Copyright : (C) SAP SE
   Author    : Maxim Rylov
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#ifndef QGSHANADATAITEMGUIPROVIDER_H
#define QGSHANADATAITEMGUIPROVIDER_H

#include "qgsdataitemguiprovider.h"

class QgsHanaSchemaItem;
class QgsHanaLayerItem;

class QgsHanaDataItemGuiProvider : public QObject, public QgsDataItemGuiProvider
{
    Q_OBJECT
  public:
    QString name() override { return QStringLiteral( "SAP HANA" ); }

    void populateContextMenu( QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &selectedItems, QgsDataItemGuiContext context ) override;

    bool deleteLayer( QgsLayerItem *item, QgsDataItemGuiContext context ) override;

    bool acceptDrop( QgsDataItem *item, QgsDataItemGuiContext context ) override;
    bool handleDrop( QgsDataItem *item, QgsDataItemGuiContext context, const QMimeData *data, Qt::DropAction action ) override;

    QWidget *createParamWidget( QgsDataItem *root, QgsDataItemGuiContext ) override;

  private:
    static void newConnection( QgsDataItem *item );
    static void editConnection( QgsDataItem *item );
    static void duplicateConnection( QgsDataItem *item );
    static void refreshConnection( QgsDataItem *item );
    static void createSchema( QgsDataItem *item, QgsDataItemGuiContext context );
    static void deleteSchema( QgsHanaSchemaItem *schemaItem, QgsDataItemGuiContext context );
    static void renameSchema( QgsHanaSchemaItem *schemaItem, QgsDataItemGuiContext context );
    static void renameLayer( QgsHanaLayerItem *layerItem, QgsDataItemGuiContext context );
};

#endif // QGSHANADATAITEMGUIPROVIDER_H
