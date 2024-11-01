/***************************************************************************
   qgshanacolumntypethread.h
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
#ifndef QGSHANACOLUMNTYPETHREAD_H
#define QGSHANACOLUMNTYPETHREAD_H

#include "qgsdatasourceuri.h"
#include "qgshanatablemodel.h"
#include <QThread>

// A class that determines the geometry type of a given database
// schema.table.column, with the option of doing so in a separate
// thread.
class QgsHanaColumnTypeThread : public QThread
{
    Q_OBJECT
  public:
    QgsHanaColumnTypeThread( const QString &connName, const QgsDataSourceUri &uri, bool allowGeometrylessTables, bool userTablesOnly );

    // These functions get the layer types and pass that information out
    // by emitting the setLayerType() signal.
    void run() override;

    const QString &errorMessage() const { return mErrorMessage; }

  signals:
    void setLayerType( QgsHanaLayerProperty layerProperty );
    void progress( int layerIndex, int layersCount );
    void progressMessage( const QString & );

  private:
    const QString mConnectionName;
    const QgsDataSourceUri mUri;
    const bool mAllowGeometrylessTables;
    const bool mUserTablesOnly;
    QString mErrorMessage;
};

#endif // QGSHANACOLUMNTYPETHREAD_H
