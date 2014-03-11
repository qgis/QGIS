/***************************************************************************
    qgsgpxfeatureiterator.h
    ---------------------
    begin                : Dezember 2012
    copyright            : (C) 2012 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSGPXFEATUREITERATOR_H
#define QGSGPXFEATUREITERATOR_H

#include "qgsfeatureiterator.h"

#include "gpsdata.h"
#include "qgsgpxprovider.h"

class QgsGPXProvider;


class QgsGPXFeatureSource : public QgsAbstractFeatureSource
{
  public:
    QgsGPXFeatureSource( const QgsGPXProvider* p );
    ~QgsGPXFeatureSource();

    virtual QgsFeatureIterator getFeatures( const QgsFeatureRequest& request );

  protected:
    QString mFileName;
    QgsGPXProvider::DataType mFeatureType;
    QgsGPSData* data;
    QVector<int> indexToAttr;
    QgsFields mFields;

    friend class QgsGPXFeatureIterator;
};


class QgsGPXFeatureIterator : public QgsAbstractFeatureIteratorFromSource<QgsGPXFeatureSource>
{
  public:
    QgsGPXFeatureIterator( QgsGPXFeatureSource* source, bool ownSource, const QgsFeatureRequest& request );

    ~QgsGPXFeatureIterator();

    //! reset the iterator to the starting position
    virtual bool rewind();

    //! end of iterating: free the resources / lock
    virtual bool close();

  protected:

    //! fetch next feature, return true on success
    virtual bool fetchFeature( QgsFeature& feature );

    bool readFid( QgsFeature& feature );

    bool readWaypoint( const QgsWaypoint& wpt, QgsFeature& feature );
    bool readRoute( const QgsRoute& rte, QgsFeature& feature );
    bool readTrack( const QgsTrack& trk, QgsFeature& feature );

    QgsGeometry* readWaypointGeometry( const QgsWaypoint& wpt );
    QgsGeometry* readRouteGeometry( const QgsRoute& rte );
    QgsGeometry* readTrackGeometry( const QgsTrack& trk );

    void readAttributes( QgsFeature& feature, const QgsWaypoint& wpt );
    void readAttributes( QgsFeature& feature, const QgsRoute& rte );
    void readAttributes( QgsFeature& feature, const QgsTrack& trk );

  protected:

    //! Current waypoint iterator
    QgsGPSData::WaypointIterator mWptIter;
    //! Current route iterator
    QgsGPSData::RouteIterator mRteIter;
    //! Current track iterator
    QgsGPSData::TrackIterator mTrkIter;

    bool mFetchedFid;
};

#endif // QGSGPXFEATUREITERATOR_H
