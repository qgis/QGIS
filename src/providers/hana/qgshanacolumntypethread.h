/***************************************************************************
   qgshanacolumntypethread.h
   --------------------------------------
   Date      : 31-05-2019
   Copyright : (C) SAP SE
   Author    : Maksim Rylov
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

#include "qgshanatablemodel.h"
#include "qgshanasettings.h"
#include <QThread>

// A class that determines the geometry type of a given database
// schema.table.column, with the option of doing so in a separate
// thread.
class QgsHanaColumnTypeThread : public QThread
{
    Q_OBJECT
  public:
    QgsHanaColumnTypeThread( const QgsHanaSettings &settings );

    // These functions get the layer types and pass that information out
    // by emitting the setLayerType() signal.
    void run() override;

    bool isStopped() const { return mStopped; }
    QVector<QgsHanaLayerProperty> layerProperties() const { return mLayerProperties; }

  signals:
    void setLayerType( QgsHanaLayerProperty layerProperty );
    void progress( int, int );
    void progressMessage( const QString & );

  public slots:
    void stop();

  private:
    QgsHanaColumnTypeThread() = default;

    QgsHanaSettings mSettings;
    bool mStopped = false;
    QVector<QgsHanaLayerProperty> mLayerProperties;
};

#endif  // QGSHANACOLUMNTYPETHREAD_H
