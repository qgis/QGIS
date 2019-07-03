/***************************************************************************
   qgshanacolumntypethread.cpp
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
#include "qgshanacolumntypethread.h"
#include "qgshanaconnection.h"
#include "qgshanasettings.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"

QgsHanaColumnTypeThread::QgsHanaColumnTypeThread(const QgsHanaSettings& settings)
  : mSettings(settings)
{
  qRegisterMetaType<QgsHanaLayerProperty>("QgsHanaLayerProperty");
}

void QgsHanaColumnTypeThread::stop()
{
  mStopped = true;
}

void QgsHanaColumnTypeThread::run()
{
  mStopped = false;

  QgsDataSourceUri uri = mSettings.toDataSourceUri();
  QgsHanaConnectionRef conn(uri);
  if (conn.isNull())
  {
    QgsDebugMsg("Connection failed - " + uri.connectionInfo(false));
    mStopped = true;
    return;
  }

  emit progressMessage(tr("Retrieving tables of %1 .").arg(mSettings.getName()));
  QVector<QgsHanaLayerProperty> layerProperties = conn->getLayers(
    mSettings.getSchema(),
    mSettings.getAllowGeometrylessTables(),
    mSettings.getUserTablesOnly());

  if (layerProperties.isEmpty())
  {
    QgsMessageLog::logMessage(
      QObject::tr("Unable to get list of spatially enabled tables from the database"), tr("HANA"));
    return;
  }

  int i = 0;
  for (QgsHanaLayerProperty& layerProperty : layerProperties)
  {
    if (!mStopped)
    {
      emit progress(i++, layerProperties.size());
      emit progressMessage(tr("Scanning column %1.%2.%3…")
        .arg(layerProperty.schemaName, layerProperty.tableName, layerProperty.geometryColName));
      conn->readLayerInfo(layerProperty);
    }

    // Signal about new layer.
    emit setLayerType(layerProperty);
  }

  if (!mStopped)
    mLayerProperties = layerProperties;

  emit progress(0, 0);
  emit progressMessage(tr("Table retrieval finished."));
}
