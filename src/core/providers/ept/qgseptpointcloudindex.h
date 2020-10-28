/***************************************************************************
                         qgspointcloudindex.h
                         --------------------
    begin                : October 2020
    copyright            : (C) 2020 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSEPTPOINTCLOUDINDEX_H
#define QGSEPTPOINTCLOUDINDEX_H

#include <QObject>
#include <QString>
#include <QHash>
#include <QStringList>
#include <QVector>
#include <QList>

#include "qgspointcloudindex.h"
#include "qgis_sip.h"

///@cond PRIVATE
#define SIP_NO_FILE

class QgsCoordinateReferenceSystem;

class QgsEptPointCloudIndex: public QgsPointCloudIndex
{
    Q_OBJECT
  public:

    explicit QgsEptPointCloudIndex();
    ~QgsEptPointCloudIndex();

    bool load( const QString &fileName ) override;

    QVector<qint32> nodePositionDataAsInt32( const IndexedPointCloudNode &n ) override;
    QVector<char> nodeClassesDataAsChar( const IndexedPointCloudNode &n ) override;

    QgsCoordinateReferenceSystem crs() const;
  private:
    QString mDataType;
    QString mDirectory;
    QString mWkt;
    int mPointRecordSize = 0;  //!< Size of one point record in bytes (only relevant for "binary" and "zstandard" data type)
};

///@endcond
#endif // QGSEPTPOINTCLOUDINDEX_H
