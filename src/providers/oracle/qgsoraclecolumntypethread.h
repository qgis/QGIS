/***************************************************************************
 qgsoraclecolumntypethread.cpp - lookup oracle geometry type and srid in a thread
                             -------------------
    begin                : 12.12.2012
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
#ifndef QGSORACLECOLUMNTYPETHREAD_H
#define QGSORACLECOLUMNTYPETHREAD_H

#include <QThread>
#include "qgsoracleconn.h"

// A class that determines the geometry type of a given database
// schema.table.column, with the option of doing so in a separate
// thread.

class QgsOracleColumnTypeThread : public QThread
{
    Q_OBJECT
  public:

    /**
     *
     * \param connName
     * \param limitToSchema If specified, only tables from this schema will be scanned
     * \param useEstimatedMetaData
     * \param allowGeometrylessTables
     */
    QgsOracleColumnTypeThread( const QString &connName,
                               const QString &limitToSchema,
                               bool useEstimatedMetaData,
                               bool allowGeometrylessTables );

    // These functions get the layer types and pass that information out
    // by emitting the setLayerType() signal.
    void run() override;

    bool isStopped() const { return mStopped; }
    QVector<QgsOracleLayerProperty> layerProperties() const { return mLayerProperties; }
    QString connectionName() const { return mName; }
    bool useEstimatedMetadata() const { return mUseEstimatedMetadata; }
    bool allowGeometrylessTables() const { return mAllowGeometrylessTables; }

  signals:
    void setLayerType( QgsOracleLayerProperty layerProperty );
    void progress( int, int );
    void progressMessage( QString );

  public slots:
    void stop();

  private:
    QgsOracleColumnTypeThread() = default;

    QString mName;
    QString mSchema;
    bool mUseEstimatedMetadata = false;
    bool mAllowGeometrylessTables = false;
    bool mStopped = false;
    QVector<QgsOracleLayerProperty> mLayerProperties;
};

#endif // QGSORACLECOLUMNTYPETHREAD_H
