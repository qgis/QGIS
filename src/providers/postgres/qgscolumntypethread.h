/***************************************************************************
 qgscolumntypethread.cpp - lookup postgres geometry type and srid in a thread
                             -------------------
    begin                : 3.1.2012
    copyright            : (C) 2012 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCOLUMNTYPETHREAD_H
#define QGSCOLUMNTYPETHREAD_H

#include <QThread>
#include "qgspostgresconn.h"

// A class that determines the geometry type of a given database
// schema.table.column, with the option of doing so in a separate
// thread.

class QgsGeomColumnTypeThread : public QThread
{
    Q_OBJECT
  public:
    QgsGeomColumnTypeThread( const QString &connName, bool useEstimatedMetaData, bool allowGeometrylessTables );

    // These functions get the layer types and pass that information out
    // by emitting the setLayerType() signal.
    void run() override;

  signals:
    void setLayerType( const QgsPostgresLayerProperty &layerProperty );
    void progress( int, int );
    void progressMessage( const QString & );

  public slots:
    void stop();

  private:
    QgsGeomColumnTypeThread() = default;

    QgsPostgresConn *mConn = nullptr;
    QString mName;
    bool mUseEstimatedMetadata = false;
    bool mAllowGeometrylessTables = false;
    bool mStopped = false;
    QList<QgsPostgresLayerProperty> layerProperties;
};

#endif // QGSCOLUMNTYPETHREAD_H
