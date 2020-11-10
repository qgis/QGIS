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
#include "qgspointcloudattribute.h"
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

    QgsPointCloudBlock *nodeData( const IndexedPointCloudNode &n, const QgsPointCloudRequest &request ) override;

    QgsCoordinateReferenceSystem crs() const;
  private:
    bool loadHierarchy();

    QString mDataType;
    QString mDirectory;
    QString mWkt;
};

///@endcond
#endif // QGSEPTPOINTCLOUDINDEX_H
