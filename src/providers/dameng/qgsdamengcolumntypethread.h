/***************************************************************************
 qgsdamengcolumntypethread.cpp - lookup dameng geometry type and srid in a thread
                             -------------------
    begin                : 2025/01/14
    copyright            : ( C ) 2025 by Haiyang Zhao
    email                : zhaohaiyang@dameng.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   ( at your option ) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSDAMENGCOLUMNTYPETHREAD_H
#define QGSDAMENGCOLUMNTYPETHREAD_H

#include <QThread>
#include "qgsdamengconn.h"

// A class that determines the geometry type of a given database
// schema.table.column, with the option of doing so in a separate
// thread.

class QgsDamengGeomColumnTypeThread : public QThread
{
    Q_OBJECT
  public:
    QgsDamengGeomColumnTypeThread( const QString &connName, bool useEstimatedMetaData, bool allowGeometrylessTables );

    // These functions get the layer types and pass that information out
    // by emitting the setLayerType() signal.
    void run() override;

  signals:
    void setLayerType( const QgsDamengLayerProperty &layerProperty );
    void progress( int, int );
    void progressMessage( const QString & );

  public slots:
    void stop();

  private:
    QgsDamengGeomColumnTypeThread() = default;

    QgsDamengConn *mConn = nullptr;
    QString mName;
    bool mUseEstimatedMetadata;
    bool mAllowGeometrylessTables;
    bool mStopped;
    QList<QgsDamengLayerProperty> layerProperties;
};

#endif // QGSDAMENGCOLUMNTYPETHREAD_H
