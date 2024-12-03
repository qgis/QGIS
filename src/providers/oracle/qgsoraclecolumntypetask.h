/***************************************************************************
 qgsoraclecolumntypetask.h - lookup oracle geometry type and srid in a thread
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
#ifndef QGSORACLECOLUMNTYPETASK_H
#define QGSORACLECOLUMNTYPETASK_H

#include "qgstaskmanager.h"
#include "qgsoracleconn.h"

// A class that determines the geometry type of a given database
// schema.table.column, with the option of doing so in a separate
// thread.

class QgsOracleColumnTypeTask : public QgsTask
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
    QgsOracleColumnTypeTask( const QString &connName, const QString &limitToSchema, bool useEstimatedMetaData, bool allowGeometrylessTables );

    // These functions get the layer types and pass that information out
    // by emitting the setLayerType() signal.
    bool run() override;

    QVector<QgsOracleLayerProperty> layerProperties() const { return mLayerProperties; }
    QString connectionName() const { return mName; }
    bool useEstimatedMetadata() const { return mUseEstimatedMetadata; }
    bool allowGeometrylessTables() const { return mAllowGeometrylessTables; }

  signals:
    void setLayerType( const QgsOracleLayerProperty &layerProperty );
    void progressMessage( const QString &message );

  private:
    QgsOracleColumnTypeTask() = default;

    QString mName;
    QString mSchema;
    bool mUseEstimatedMetadata = false;
    bool mAllowGeometrylessTables = false;
    QVector<QgsOracleLayerProperty> mLayerProperties;
};

#endif // QGSORACLECOLUMNTYPETASK_H
