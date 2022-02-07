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

class QgsGeorefDataPoint;
class QgsPointXY;
class QgsCoordinateReferenceSystem;

/**
 * A container for GCP data points.
 *
 * The container does NOT own the points!
 */
class QgsGCPList : public QList<QgsGeorefDataPoint *>
{
  public:
    QgsGCPList() = default;
    QgsGCPList( const QgsGCPList &list );

    void createGCPVectors( QVector<QgsPointXY> &sourceCoordinates, QVector<QgsPointXY> &destinationCoordinates, const QgsCoordinateReferenceSystem &targetCrs );

    /**
     * Returns the count of currently enabled data points.
     */
    int countEnabledPoints() const;

    QgsGCPList &operator =( const QgsGCPList &list );
};

#endif
