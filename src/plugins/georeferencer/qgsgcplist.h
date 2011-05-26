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
#include <vector>

class QgsGeorefDataPoint;
class QgsPoint;

// what is better use inherid or agrigate QList?
class QgsGCPList : public QList<QgsGeorefDataPoint *>
{
  public:
    QgsGCPList();
    QgsGCPList( const QgsGCPList &list );

    void createGCPVectors( std::vector<QgsPoint> &mapCoords,
                           std::vector<QgsPoint> &pixelCoords );
    int size() const;
    int sizeAll() const;

    QgsGCPList &operator =( const QgsGCPList &list );
};

//typedef std::vector<QgsGeorefDataPoint *> QgsGCPList;

#endif
