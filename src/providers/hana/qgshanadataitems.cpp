/***************************************************************************
   qgshanadataitems.cpp
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
#include "qgsapplication.h"
#include "qgsdatasourceuri.h"
#include "qgshanaconnection.h"
#include "qgshanaexception.h"
#include "qgshananewconnection.h"
#include "qgshanadataitems.h"
#include "qgshanasettings.h"
#include "qgshanasourceselect.h"
#include "qgshanautils.h"
#include "qgslogger.h"
#include "qgsmessageoutput.h"
#include "qgsnewnamedialog.h"
#include "qgsvectorlayer.h"

#include <QMessageBox>
#include <climits>

using namespace std;

// ---------------------------------------------------------------------------
QgsHanaConnectionItem::QgsHanaConnectionItem(
  QgsDataItem* parent,
  const QString &name,
  const QString &path)
  : QgsDataCollectionItem(parent, name, path)
{
  mIconName = QStringLiteral("mIconConnect.svg");
  mCapabilities |= Collapse;

  updateToolTip(QString(""), QString(""));
}

QVector<QgsDataItem*> QgsHanaConnectionItem::createChildren()
{
  QVector<QgsDataItem*> items;

  QgsHanaConnectionRef conn(mName);

  if (conn.isNull())
  {
    items.append(new QgsErrorItem(this, tr("Connection failed"), mPath + "/error"));
    QgsDebugMsg("Connection failed - " + mName);
    return items;
  }

  updateToolTip(conn->getUserName(), conn->getDatabaseVersion());

  try
  {
    QgsHanaSettings settings(mName, true);
    QVector<QgsHanaSchemaProperty> schemas =
      conn->getSchemas(settings.getUserTablesOnly() ? settings.getUserName() : "");
    conn.release();

    if (schemas.isEmpty())
    {
      items.append(new QgsErrorItem(this, tr("No schemas found"), mPath + "/error"));
    }
    else
    {
      Q_FOREACH(const QgsHanaSchemaProperty &schema, schemas)
      {
        QgsHanaSchemaItem* schemaItem = new QgsHanaSchemaItem(this, mName, schema.name,
          mPath + '/' + schema.name);
        items.append(schemaItem);
      }
    }
  }
  catch(const QException& ex)
  {
    QgsErrorItem* itemError = new QgsErrorItem(this, tr("Server error occured"), mPath + "/error");
    itemError->setToolTip(ex.what());
    items.append(itemError);
  }

  return items;
}

bool QgsHanaConnectionItem::equal(const QgsDataItem* other)
{
  if (type() != other->type())
    return false;

  const QgsHanaConnectionItem* o = qobject_cast<const QgsHanaConnectionItem*>(other);
  return (mPath == o->mPath && mName == o->mName);
}

QList<QAction*> QgsHanaConnectionItem::actions(QWidget* parent)
{
  QList<QAction*> lst;

  QAction* actionRefresh = new QAction(tr("Refresh"), parent);
  connect(actionRefresh, &QAction::triggered, this, &QgsHanaConnectionItem::refreshConnection);
  lst.append(actionRefresh);

  QAction* separator = new QAction(parent);
  separator->setSeparator(true);
  lst.append(separator);

  QAction* actionEdit = new QAction(tr("Edit Connection…"), parent);
  connect(actionEdit, &QAction::triggered, this, &QgsHanaConnectionItem::editConnection);
  lst.append(actionEdit);

  QAction* actionDelete = new QAction(tr("Delete Connection"), this);
  connect(actionDelete, &QAction::triggered, this, &QgsHanaConnectionItem::deleteConnection);
  lst.append(actionDelete);

  return lst;
}

void QgsHanaConnectionItem::editConnection()
{
  QgsHanaNewConnection nc(nullptr, mName);
  if (nc.exec())
  {
    // the parent should be updated
    if (mParent)
      mParent->refreshConnections();
  }
}

void QgsHanaConnectionItem::deleteConnection()
{
  if (QMessageBox::question(nullptr, QObject::tr("Delete Connection"),
    tr("Are you sure you want to delete the connection to %1?").arg(mName),
    QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
    return;

  QgsHanaSettings::removeConnection(mName);
  // the parent should be updated
  if (mParent)
    mParent->refreshConnections();
}

void QgsHanaConnectionItem::refreshConnection()
{
  refresh();
}

void QgsHanaConnectionItem::refreshSchema(const QString &schema)
{
  Q_FOREACH(QgsDataItem* child, mChildren)
  {
    if (child->name() == schema || schema.isEmpty())
      child->refresh();
  }
}

void QgsHanaConnectionItem::updateToolTip(const QString& userName, const QString& dbmsVersion)
{
  QgsHanaSettings settings(mName, true);
  QString tip;
  if (!settings.getDatabase().isEmpty())
    tip = QStringLiteral("Database: ") + settings.getDatabase();
  if (!tip.isEmpty())
    tip += '\n';
  tip += QStringLiteral("Host: ") + settings.getHost() + QStringLiteral(" ");
  if (QgsHanaIdentifierType::fromInt(settings.getIdentifierType()) == QgsHanaIdentifierType::INSTANCE_NUMBER)
    tip += settings.getIdentifier();
  else
    tip += settings.getPort();
  if (!tip.isEmpty())
    tip += '\n';
  if (!dbmsVersion.isEmpty())
  {
    tip += QStringLiteral("DB Version: ") + dbmsVersion;
    if (!tip.isEmpty())
      tip += '\n';
  }
  tip += QStringLiteral("User: ") + userName;
  if (!tip.isEmpty())
    tip += '\n';
  tip += QStringLiteral("Encrypted: ") + QString(settings.getEnableSsl() ? QStringLiteral("yes") : QStringLiteral("no"));
  setToolTip(tip);
}

bool QgsHanaConnectionItem::handleDrop(const QMimeData* data, Qt::DropAction)
{
  return handleDrop(data, QString());
}

bool QgsHanaConnectionItem::handleDrop(const QMimeData* data, const QString &toSchema)
{
  if (!QgsMimeDataUtils::isUriList(data))
    return false;

  QStringList importResults;
  bool hasError = false;

  QgsDataSourceUri uri = QgsHanaSettings(mName, true).toDataSourceUri();
  QgsHanaConnectionRef conn(uri);

  if (!conn.isNull())
  {
    QgsMimeDataUtils::UriList lst = QgsMimeDataUtils::decodeUriList(data);
    Q_FOREACH(const QgsMimeDataUtils::Uri &u, lst)
    {
      if (u.layerType != QLatin1String("vector"))
      {
        importResults.append(tr("%1: Not a vector layer!").arg(u.name));
        hasError = true; // only vectors can be imported
        continue;
      }

      // open the source layer
      QgsVectorLayer* srcLayer = new QgsVectorLayer(u.uri, u.name, u.providerKey);

      if (srcLayer->isValid())
      {
        bool fieldsInUpperCase = QgsHanaUtils::countFieldsInUppercase(srcLayer->fields()) > srcLayer->fields().size() / 2;

        uri.setWkbType(srcLayer->wkbType());
        uri.setDataSource(!toSchema.isNull() ? toSchema : nullptr,
          u.name,
          (srcLayer->geometryType() != QgsWkbTypes::NullGeometry) ? (fieldsInUpperCase ? QStringLiteral("GEOM") : QStringLiteral("geom")) : nullptr);

        QgsDebugMsg("URI " + uri.uri(false));

        std::unique_ptr< QgsVectorLayerExporterTask > exportTask(
          QgsVectorLayerExporterTask::withLayerOwnership(srcLayer, uri.uri(),
            QStringLiteral("hana"), srcLayer->crs()));
        // when export is successful:
        connect(exportTask.get(), &QgsVectorLayerExporterTask::exportComplete, this,
          [&]()
          {
            QMessageBox::information(nullptr, tr("Import to HANA database"), tr("Import was successful."));
            refreshSchema(toSchema);
          });

        // when an error occurs:
        connect(exportTask.get(), &QgsVectorLayerExporterTask::errorOccurred, this,
          [&](int error, const QString& errorMessage)
          {
            if (error != QgsVectorLayerExporter::ErrUserCanceled)
            {
              QgsMessageOutput* output = QgsMessageOutput::createMessageOutput();
              output->setTitle(tr("Import to HANA database"));
              output->setMessage(tr("Failed to import some layers!\n\n") +
                errorMessage, QgsMessageOutput::MessageText);
              output->showMessage();
            }
            refreshSchema(toSchema);
          });

        QgsApplication::taskManager()->addTask(exportTask.release());
      }
      else
      {
        importResults.append(tr("%1: Not a valid layer!").arg(u.name));
        hasError = true;
      }
    }
  }
  else
  {
    importResults.append(tr("Connection failed"));
    hasError = true;
  }

  if (hasError)
  {
    QgsMessageOutput* output = QgsMessageOutput::createMessageOutput();
    output->setTitle(tr("Import to HANA database"));
    output->setMessage(tr("Failed to import some layers!\n\n") +
      importResults.join(QStringLiteral("\n")), QgsMessageOutput::MessageText);
    output->showMessage();
  }

  return true;
}

// ---------------------------------------------------------------------------
QgsHanaLayerItem::QgsHanaLayerItem(
  QgsDataItem* parent,
  const QString &name,
  const QString &path,
  QgsLayerItem::LayerType layerType,
  const QgsHanaLayerProperty &layerProperty)
  : QgsLayerItem(parent, name, path, QString(), layerType, QStringLiteral("hana"))
  , mLayerProperty(layerProperty)
{
  mUri = createUri();
  setState(Populated);
}

QString QgsHanaLayerItem::comments() const
{
  return mLayerProperty.tableComment;
}

QString QgsHanaLayerItem::createUri() const
{
  QString pkColName = !mLayerProperty.pkCols.isEmpty() ? mLayerProperty.pkCols.at(0) : QString();
  QgsHanaConnectionItem* connItem = qobject_cast<QgsHanaConnectionItem*>(parent() ?
    parent()->parent() : nullptr);

  if (!connItem)
  {
    QgsDebugMsg("Connection item not found.");
    return QString();
  }

  QgsHanaSettings settings(connItem->name(), true);
  QgsDataSourceUri uri = settings.toDataSourceUri();
  uri.setDataSource(mLayerProperty.schemaName, mLayerProperty.tableName,
    mLayerProperty.geometryColName, mLayerProperty.sql, pkColName);
  uri.setWkbType(mLayerProperty.type);
  if (uri.wkbType() != QgsWkbTypes::NoGeometry)
    uri.setSrid(QString::number(mLayerProperty.srid));
  QgsDebugMsg(QStringLiteral("layer uri: %1").arg(uri.uri(false)));
  return uri.uri(false);
}

QList<QAction *> QgsHanaLayerItem::actions(QWidget *parent)
{
  QList<QAction *> lst;

  QAction *actionDeleteLayer = new QAction(tr("Delete Table"), parent);
  connect(actionDeleteLayer, &QAction::triggered, this, &QgsHanaLayerItem::deleteLayer);
  lst.append(actionDeleteLayer);

  return lst;
}

bool QgsHanaLayerItem::deleteLayer()
{
  QString typeName = mLayerProperty.isView ? tr("View") : tr("Table");

  if (QMessageBox::question(nullptr, tr("Delete %1").arg(typeName),
    tr("Are you sure you want to delete %1.%2?").arg(mLayerProperty.schemaName, mLayerProperty.tableName),
    QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
    return true;

  QgsHanaConnectionItem* connItem = qobject_cast<QgsHanaConnectionItem*>(parent() ?
    parent()->parent() : nullptr);

  QgsHanaConnectionRef conn(connItem->name());
  if (conn.isNull())
    return false;

  QString errMessage;
  bool res = conn->dropTable(mLayerProperty.schemaName, mLayerProperty.tableName, &errMessage);
  if (!res)
  {
    QMessageBox::warning(nullptr, tr("Delete %1").arg(typeName), errMessage);
  }
  else
  {
    QMessageBox::information(nullptr, tr("Delete %1")
      .arg(typeName), tr("%1 deleted successfully.").arg(typeName));
    if (mParent)
      mParent->refresh();
  }
  return res;
}

// ---------------------------------------------------------------------------
QgsHanaSchemaItem::QgsHanaSchemaItem(
  QgsDataItem* parent,
  const QString &connectionName,
  const QString &name,
  const QString &path)
  : QgsDataCollectionItem(parent, name, path)
  , mConnectionName(connectionName)
{
  mIconName = QStringLiteral("mIconDbSchema.svg");
  mSchemaName = name;
}

QVector<QgsDataItem*> QgsHanaSchemaItem::createChildren()
{
  QVector<QgsDataItem*> items;

  QgsHanaConnectionRef conn(mConnectionName);
  if (conn.isNull())
  {
    items.append(new QgsErrorItem(this, tr("Connection failed"), mPath + "/error"));
    return items;
  }

  QgsHanaSettings settings(mConnectionName, true);
  QVector<QgsHanaLayerProperty> layerProperties = conn->getLayers(mSchemaName,
    settings.getAllowGeometrylessTables(), settings.getUserTablesOnly());

  if (!layerProperties.isEmpty())
  {
    size_t numLayersWithGeom = 0;
    Q_FOREACH(QgsHanaLayerProperty layerProperty, layerProperties)
    {
      if (layerProperty.schemaName != mSchemaName)
        continue;

      conn->readLayerInfo(layerProperty);

      QgsHanaLayerItem* layerItem = createLayer(layerProperty);
      if (layerItem)
        items.append(layerItem);

      if (!layerProperty.geometryColName.isEmpty())
        ++numLayersWithGeom;
    }
  }

  this->setName(mSchemaName);

  return items;
}

QList<QAction*> QgsHanaSchemaItem::actions(QWidget* parent)
{
  QList<QAction*> lst;

  QAction* actionRefresh = new QAction(tr("Refresh"), parent);
  connect(actionRefresh, &QAction::triggered, this,
    static_cast<void (QgsDataItem::*)()>(&QgsDataItem::refresh));
  lst.append(actionRefresh);

  return lst;
}

QgsHanaLayerItem* QgsHanaSchemaItem::createLayer(const QgsHanaLayerProperty &layerProperty)
{
  QString tip = layerProperty.isView ? QStringLiteral("View"): QStringLiteral("Table");

  QgsLayerItem::LayerType layerType = QgsLayerItem::TableLayer;
  if (!layerProperty.geometryColName.isEmpty() && layerProperty.isValid())
  {
    if (layerProperty.srid < 0)
    {
      tip += QStringLiteral("\n%1 as %2").arg(layerProperty.geometryColName,
        QgsWkbTypes::displayString(layerProperty.type));
    }
    else
    {
      tip += QStringLiteral("\n%1 as %2 in SRID %3")
        .arg(layerProperty.geometryColName, QgsWkbTypes::displayString(layerProperty.type))
        .arg(layerProperty.srid);
    }

    if (!layerProperty.tableComment.isEmpty())
      tip = layerProperty.tableComment + '\n' + tip;

    QgsWkbTypes::GeometryType geomType = QgsWkbTypes::geometryType(layerProperty.type);
    switch (geomType)
    {
    case QgsWkbTypes::PointGeometry:
      layerType = QgsLayerItem::Point;
      break;
    case QgsWkbTypes::LineGeometry:
      layerType = QgsLayerItem::Line;
      break;
    case QgsWkbTypes::PolygonGeometry:
      layerType = QgsLayerItem::Polygon;
      break;
    default:
      break;
    }
  }
  else
  {
    tip += QStringLiteral("\nno geometry column");
  }

  QgsHanaLayerItem* layerItem = new QgsHanaLayerItem(this, layerProperty.defaultName(),
    mPath + '/' + layerProperty.tableName, layerType, layerProperty);
  layerItem->setToolTip(tip);
  return layerItem;
}

bool QgsHanaSchemaItem::handleDrop(const QMimeData* data, Qt::DropAction)
{
  QgsHanaConnectionItem* conn = qobject_cast<QgsHanaConnectionItem*>(parent());
  if (!conn)
    return false;

  bool result = conn->handleDrop(data, mSchemaName);
  if (result)
    refresh();

  return result;
}

QgsHanaRootItem::QgsHanaRootItem(QgsDataItem* parent, const QString &name, const QString &path)
  : QgsDataCollectionItem(parent, name, path)
{
  mCapabilities |= Fast;
  mIconName = QStringLiteral("mIconHana.svg");
  populate();
}

QVector<QgsDataItem*> QgsHanaRootItem::createChildren()
{
  QVector<QgsDataItem*> connections;
  Q_FOREACH(const QString &connName, QgsHanaSettings::getConnectionNames())
  {
    connections << new QgsHanaConnectionItem(this, connName, mPath + '/' + connName);
  }
  return connections;
}

QList<QAction*> QgsHanaRootItem::actions(QWidget* parent)
{
  QList<QAction*> lst;

  QAction* actionNew = new QAction(tr("New Connection…"), parent);
  connect(actionNew, &QAction::triggered, this, &QgsHanaRootItem::newConnection);
  lst.append(actionNew);

  return lst;
}

QMainWindow *QgsHanaRootItem::sMainWindow = nullptr;

QWidget* QgsHanaRootItem::paramWidget()
{
  QgsHanaSourceSelect* select = new QgsHanaSourceSelect(nullptr, nullptr,
    QgsProviderRegistry::WidgetMode::Manager);
  connect(select, &QgsHanaSourceSelect::connectionsChanged, this,
    &QgsHanaRootItem::onConnectionsChanged);
  return select;
}

void QgsHanaRootItem::onConnectionsChanged()
{
  refresh();
}

void QgsHanaRootItem::newConnection()
{
  QgsHanaNewConnection nc(nullptr);
  if (nc.exec())
    refresh();
}

QgsDataItem *QgsHanaDataItemProvider::createDataItem(
  const QString &pathIn, QgsDataItem *parentItem)
{
  Q_UNUSED(pathIn);
  QgsDebugMsg(QStringLiteral("HANA: Browser Panel; data item detected."));
  return new QgsHanaRootItem(parentItem, QStringLiteral("HANA"), QStringLiteral("hana:"));
}
