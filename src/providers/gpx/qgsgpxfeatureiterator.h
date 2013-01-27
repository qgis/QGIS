#ifndef QGSGPXFEATUREITERATOR_H
#define QGSGPXFEATUREITERATOR_H

#include "qgsfeatureiterator.h"

#include "gpsdata.h"

class QgsGPXProvider;

class QgsGPXFeatureIterator : public QgsAbstractFeatureIterator
{
  public:
    QgsGPXFeatureIterator( QgsGPXProvider* p, const QgsFeatureRequest& request );

    ~QgsGPXFeatureIterator();

    //! fetch next feature, return true on success
    virtual bool nextFeature( QgsFeature& feature );

    //! reset the iterator to the starting position
    virtual bool rewind();

    //! end of iterating: free the resources / lock
    virtual bool close();

  protected:

    bool readFid( QgsFeature& feature );

    bool readWaypoint(const QgsWaypoint& wpt, QgsFeature& feature);
    bool readRoute(const QgsRoute& rte, QgsFeature& feature);
    bool readTrack(const QgsTrack& trk, QgsFeature& feature);

    QgsGeometry* readWaypointGeometry(const QgsWaypoint& wpt);
    QgsGeometry* readRouteGeometry(const QgsRoute& rte);
    QgsGeometry* readTrackGeometry(const QgsTrack& trk);

    void readAttributes( QgsFeature& feature, const QgsWaypoint& wpt );
    void readAttributes( QgsFeature& feature, const QgsRoute& rte );
    void readAttributes( QgsFeature& feature, const QgsTrack& trk );

  protected:
    QgsGPXProvider* P;

    //! Current waypoint iterator
    QgsGPSData::WaypointIterator mWptIter;
    //! Current route iterator
    QgsGPSData::RouteIterator mRteIter;
    //! Current track iterator
    QgsGPSData::TrackIterator mTrkIter;


    bool mFetchedFid;
};

#endif // QGSGPXFEATUREITERATOR_H
