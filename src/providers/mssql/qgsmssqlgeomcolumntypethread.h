/***************************************************************************
    qgsmssqlgeomcolumntypethread.h
    ---------------------
    begin                : July 2017
    copyright            : (C) 2017 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSMSSQLGEOMCOLUMNTYPETHREAD_H
#define QGSMSSQLGEOMCOLUMNTYPETHREAD_H

#include "qgsmssqltablemodel.h"

#include <QThread>

// A class that determines the geometry type of a given database
// schema.table.column, with the option of doing so in a separate
// thread.

class QgsMssqlGeomColumnTypeThread : public QThread
{
    Q_OBJECT
  public:
    QgsMssqlGeomColumnTypeThread( const QString &service, const QString &host, const QString &database, const QString &username, const QString &password, bool useEstimatedMetadata );

    // These functions get the layer types and pass that information out
    // by emitting the setLayerType() signal.
    void run() override;

  signals:
    void setLayerType( const QgsMssqlLayerProperty &layerProperty );

  public slots:
    void addGeometryColumn( const QgsMssqlLayerProperty &layerProperty );
    void stop();

  private:
    QgsMssqlGeomColumnTypeThread() = delete;

    QString mService;
    QString mHost;
    QString mDatabase;
    QString mUsername;
    QString mPassword;
    bool mUseEstimatedMetadata;
    bool mStopped;
    QList<QgsMssqlLayerProperty> layerProperties;
};

#endif // QGSMSSQLGEOMCOLUMNTYPETHREAD_H
