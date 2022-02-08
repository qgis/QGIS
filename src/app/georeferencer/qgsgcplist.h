/***************************************************************************
    qgsgcplist.h - GCP list class
     --------------------------------------
    Date                 : 27-Feb-2009
    Copyright            : (c) 2009 by Manuel Massing
    Email                : m.massing at warped-space.de
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS_GCP_LIST_H
#define QGS_GCP_LIST_H

#include <QList>
#include <QVector>
#include "qgis_app.h"

class QgsGeorefDataPoint;
class QgsGcpPoint;
class QgsPointXY;
class QgsCoordinateReferenceSystem;

/**
 * A container for GCP data points.
 *
 * The container does NOT own the points -- they have to be manually deleted elsewhere!!
 */
class APP_EXPORT QgsGCPList : public QList<QgsGeorefDataPoint * >
{
  public:
    QgsGCPList() = default;
    QgsGCPList( const QgsGCPList &list ) = delete;
    QgsGCPList &operator =( const QgsGCPList &list ) = delete;

    /**
     * Creates vectors of source and destination points, where the destination points are all transformed to the
     * specified \a targetCrs.
     */
    void createGCPVectors( QVector<QgsPointXY> &sourcePoints, QVector<QgsPointXY> &destinationPoints, const QgsCoordinateReferenceSystem &targetCrs ) const;

    /**
     * Returns the count of currently enabled data points.
     */
    int countEnabledPoints() const;

    /**
     * Returns the container as a list of GCP points.
     */
    QList< QgsGcpPoint > asPoints() const;

};

#endif
