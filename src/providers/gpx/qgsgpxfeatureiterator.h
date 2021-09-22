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
#include "qgscoordinatetransform.h"

class QgsGPXProvider;


class QgsGPXFeatureSource final: public QgsAbstractFeatureSource
{
  public:
    explicit QgsGPXFeatureSource( const QgsGPXProvider *p );
    ~QgsGPXFeatureSource() override;

    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request ) override;

  private:
    QString mFileName;
    QgsGPXProvider::DataType mFeatureType;
    QgsGpsData *mData = nullptr;
    QVector<int> mIndexToAttr;
    QgsFields mFields;
    QgsCoordinateReferenceSystem mCrs;

    friend class QgsGPXFeatureIterator;
};


class QgsGPXFeatureIterator final: public QgsAbstractFeatureIteratorFromSource<QgsGPXFeatureSource>
{
  public:
    QgsGPXFeatureIterator( QgsGPXFeatureSource *source, bool ownSource, const QgsFeatureRequest &request );

    ~QgsGPXFeatureIterator() override;

    bool rewind() override;
    bool close() override;

  protected:

    bool fetchFeature( QgsFeature &feature ) override;

  private:

    bool readFid( QgsFeature &feature );

    bool readWaypoint( const QgsWaypoint &wpt, QgsFeature &feature );
    bool readRoute( const QgsRoute &rte, QgsFeature &feature );
    bool readTrack( const QgsTrack &trk, QgsFeature &feature );

    QgsGeometry *readWaypointGeometry( const QgsWaypoint &wpt );
    QgsGeometry *readRouteGeometry( const QgsRoute &rte );
    QgsGeometry *readTrackGeometry( const QgsTrack &trk );

    void readAttributes( QgsFeature &feature, const QgsWaypoint &wpt );
    void readAttributes( QgsFeature &feature, const QgsRoute &rte );
    void readAttributes( QgsFeature &feature, const QgsTrack &trk );

    //! Current waypoint iterator
    QgsGpsData::WaypointIterator mWptIter;
    //! Current route iterator
    QgsGpsData::RouteIterator mRteIter;
    //! Current track iterator
    QgsGpsData::TrackIterator mTrkIter;

    bool mFetchedFid = false;

    QgsCoordinateTransform mTransform;
    QgsRectangle mFilterRect;
    QgsGeometry mDistanceWithinGeom;
    std::unique_ptr< QgsGeometryEngine > mDistanceWithinEngine;
};

#endif // QGSGPXFEATUREITERATOR_H
