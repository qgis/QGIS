/***************************************************************************
   qgshanadataitemguiprovider.cpp
   --------------------------------------
   Date      : 31-05-2019
   Copyright : (C) SAP SE
   Author    : Maksim Rylov
/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#include "qgshanaconnection.h"
#include "qgshanadataitems.h"
#include "qgshanadataitemguiprovider.h"
#include "qgshananewconnection.h"
#include "qgshanaprovider.h"
#include "qgsnewnamedialog.h"

#include <QInputDialog>
#include <QMessageBox>

void QgsHanaDataItemGuiProvider::populateContextMenu(
  QgsDataItem *item, QMenu *menu, const QList<QgsDataItem *> &, QgsDataItemGuiContext)
{
  if (QgsHanaRootItem *rootItem = qobject_cast<QgsHanaRootItem *>(item))
  {
    QAction *actionNew = new QAction(tr("New Connection…"), this);
    connect(actionNew, &QAction::triggered, this, [rootItem] { newConnection(rootItem); });
    menu->addAction(actionNew);
  }

  if (QgsHanaConnectionItem *connItem = qobject_cast<QgsHanaConnectionItem *>(item))
  {
    QAction *actionRefresh = new QAction(tr("Refresh"), this);
    connect(actionRefresh, &QAction::triggered, this, [connItem] { refreshConnection(connItem); });
    menu->addAction(actionRefresh);

    menu->addSeparator();

    QAction *actionEdit = new QAction(tr("Edit Connection…"), this);
    connect(actionEdit, &QAction::triggered, this, [connItem] { editConnection(connItem); });
    menu->addAction(actionEdit);

    QAction *actionDelete = new QAction(tr("Delete Connection"), this);
    connect(actionDelete, &QAction::triggered, this, [connItem] { deleteConnection(connItem); });
    menu->addAction(actionDelete);
  }

  if (QgsHanaSchemaItem *schemaItem = qobject_cast<QgsHanaSchemaItem *>(item))
  {
    QAction *actionRefresh = new QAction(tr("Refresh"), this);
    connect(actionRefresh, &QAction::triggered, this, [schemaItem] { schemaItem->refresh(); });
    menu->addAction(actionRefresh);
  }
}

bool QgsHanaDataItemGuiProvider::deleteLayer(QgsLayerItem *item, QgsDataItemGuiContext context)
{
  if (QgsHanaLayerItem *layerItem = qobject_cast<QgsHanaLayerItem *>(item))
  {
    const QgsHanaLayerProperty &layerInfo = layerItem->layerInfo();
    QString typeName = layerInfo.isView ? tr("View") : tr("Table");

    if (QMessageBox::question(nullptr, tr("Delete %1").arg(typeName),
      tr("Are you sure you want to delete %1.%2?").arg(layerInfo.schemaName, layerInfo.tableName),
      QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
      return true;

    QgsHanaConnectionItem* connItem = qobject_cast<QgsHanaConnectionItem*>(layerItem->parent()->parent());

    QgsHanaConnectionRef conn(connItem->name());
    if (conn.isNull())
      return false;

    QString errMessage;
    bool res = conn->dropTable(layerInfo.schemaName, layerInfo.tableName, &errMessage);
    if (!res)
    {
      QMessageBox::warning(nullptr, tr("Delete %1").arg(typeName), errMessage);
    }
    else
    {
      QMessageBox::information(nullptr,
        tr("Delete %1").arg(typeName),
        tr("%1 deleted successfully.").arg(typeName));
      if ( layerItem->parent() )
        layerItem->parent()->refresh();
    }
    return res;
  }
  return false;
}

bool QgsHanaDataItemGuiProvider::acceptDrop(QgsDataItem *item, QgsDataItemGuiContext)
{
  if (qobject_cast<QgsHanaConnectionItem *>(item))
    return true;
  if (qobject_cast<QgsHanaSchemaItem *>(item))
    return true;

  return false;
}

bool QgsHanaDataItemGuiProvider::handleDrop(
  QgsDataItem *item, QgsDataItemGuiContext, const QMimeData *data, Qt::DropAction)
{
  if (QgsHanaConnectionItem *connItem = qobject_cast<QgsHanaConnectionItem *>(item))
  {
    return connItem->handleDrop(data, QString());
  }
  else if (QgsHanaSchemaItem *schemaItem = qobject_cast<QgsHanaSchemaItem *>(item))
  {
    QgsHanaConnectionItem *connItem = qobject_cast<QgsHanaConnectionItem *>(schemaItem->parent());
    if (!connItem)
      return false;

    return connItem->handleDrop(data, schemaItem->name());
  }
  return false;
}

void QgsHanaDataItemGuiProvider::newConnection(QgsDataItem *item)
{
  QgsHanaNewConnection nc(nullptr);
  if (nc.exec())
  {
    item->refresh();
  }
}

void QgsHanaDataItemGuiProvider::editConnection(QgsDataItem *item)
{
  QgsHanaNewConnection nc(nullptr, item->name());
  if ( nc.exec() )
  {
    // the parent should be updated
    if (item->parent())
      item->parent()->refreshConnections();
  }
}

void QgsHanaDataItemGuiProvider::deleteConnection(QgsDataItem *item)
{
  if (QMessageBox::question(nullptr, QObject::tr("Delete Connection"),
    tr("Are you sure you want to delete the connection to %1?").arg(item->name()),
    QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
    return;

  QgsHanaSettings::removeConnection(item->name());
  // the parent should be updated
  if (item->parent())
    item->parent()->refreshConnections();
}

void QgsHanaDataItemGuiProvider::refreshConnection(QgsDataItem *item)
{
  item->refresh();
  // the parent should be updated
  if (item->parent())
    item->parent()->refreshConnections();
}
