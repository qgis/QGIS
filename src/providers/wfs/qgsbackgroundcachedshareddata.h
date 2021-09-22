/***************************************************************************
    qgsbacckgroundcachedshareddata.h
    ---------------------
    begin                : October 2019
    copyright            : (C) 2016-2019 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSBACKGROUNDCACHEDSHAREDDATA_H
#define QGSBACKGROUNDCACHEDSHAREDDATA_H

#include "qgsbackgroundcachedfeatureiterator.h"
#include "qgsrectangle.h"
#include "qgsspatialiteutils.h"
#include "qgscachedirectorymanager.h"

#include <QSet>

#include <map>

class QgsBackgroundCachedFeatureIterator;
class QgsFeatureDownloader;
class QgsFeatureDownloaderImpl;
class QgsThreadedFeatureDownloader;

/**
 * This class holds data, and logic, shared between the provider, QgsBackgroundCachedFeatureIterator
 *  and QgsFeatureDownloader. It manages the on-disk cache, as a SpatiaLite
 *  database.
 *
 *  The structure of the table in the database is the following one :
 *
 * - attribute fields of the DescribeFeatureType response
 * - __qgis_gen_counter: generation counter
 * - __qgis_unique_id: typically feature 'fid' or 'gml:id'
 * - __qgis_hexwkb_geom: feature geometry as a hexadecimal encoded WKB string.
 * - geometry: polygon with the bounding box of the geometry.
 *
 *  The generation counter is a synchronization mechanism between the iterator
 *  that will try to return cached features first and then downloaded features.
 *  It avoids the iterator to return features in duplicates, by returning features
 *  that have just been serialized by the live downloader and notified to the
 *  iterator.
 *
 *  The reason for not storing directly the geometry is that we may potentially
 *  store in the future non-linear geometries that aren't handled by SpatiaLite.
 *
 *  It contains also methods used in WFS-T context to update the cache content,
 *  from the changes initiated by the user.
 */
class QgsBackgroundCachedSharedData
{
  public:
    QgsBackgroundCachedSharedData( const QString &providerName, const QString &componentTranslated );
    virtual ~QgsBackgroundCachedSharedData();

    //////////// Methods to be used by provider

    //! Returns extent computed from currently downloaded features
    const QgsRectangle &computedExtent() const;

    //! Returns whether the feature download is finished
    bool downloadFinished() const { return mDownloadFinished; }

    //! Returns layer feature count. Might issue a network request if issueRequestIfNeeded == true
    long long getFeatureCount( bool issueRequestIfNeeded = true );

    //! Return a "consolidated" extent mixing the one from the capabilities from the one of the features already downloaded.
    QgsRectangle consolidatedExtent() const;

    /**
     * Used by provider's reloadData(). The effect is to invalid
     * all the caching state, so that a new request results in fresh download.
    */
    void invalidateCache();

    //! Give a feature id, find the correspond fid/gml.id. Used for edition.
    QString findUniqueId( QgsFeatureId fid ) const;

    //! Delete from the on-disk cache the features of given fid. Used byedition.
    bool deleteFeatures( const QgsFeatureIds &fidlist );

    //! Change into the on-disk cache the passed geometries. Used by edition.
    bool changeGeometryValues( const QgsGeometryMap &geometry_map );

    //! Change into the on-disk cache the passed attributes. Used by edition.
    bool changeAttributeValues( const QgsChangedAttributesMap &attr_map );

    //////////// Methods to be used by feature iterator & downloader

    //// Getters

    //! Retrieve the dbId from the qgisId
    QgsFeatureIds dbIdsFromQgisIds( const QgsFeatureIds &qgisIds ) const;

    //! Returns maximum number of features to download, or 0 if unlimited
    int requestLimit() const { return mRequestLimit; }

    //! Returns whether the feature count is exact, or approximate/transient
    bool isFeatureCountExact() const { return mFeatureCountExact; }

    //! Return source/provider CRS
    const QgsCoordinateReferenceSystem &sourceCrs() const { return mSourceCrs; }

    //! Return the provider for the Spatialite cache.
    QgsVectorDataProvider *cacheDataProvider() { return mCacheDataProvider.get(); }

    //! Return provider fields.
    const QgsFields &fields() const { return mFields; }

    //! Return the spatialite field name from the user visible column name
    QString getSpatialiteFieldNameFromUserVisibleName( const QString &columnName ) const;

    //! Return the user visible FeatureID from the spatialite FeatureID
    bool getUserVisibleIdFromSpatialiteId( QgsFeatureId dbId, QgsFeatureId &outId ) const;

    //! Return current BBOX used by the downloader
    const QgsRectangle &currentRect() const { return mRect; }

    //! Returns a unique identifier made from feature content
    static QString getMD5( const QgsFeature &f );

    //! Filter expression to apply on client side
    const QString &clientSideFilterExpression() const { return mClientSideFilterExpression; }

    //// Actions

    /**
     * Used by a QgsBackgroundCachedFeatureIterator to start a downloader and get the
     * generation counter.
    */
    int registerToCache( QgsBackgroundCachedFeatureIterator *iterator, int limit, const QgsRectangle &rect = QgsRectangle() );

    /**
     * Used by the rewind() method of an iterator so as to get the up-to-date
     * generation counter.
    */
    int getUpdatedCounter();

    /**
     * Used by the background downloader to serialize downloaded features into
     * the cache. Also used by a insert operation.
    */
    void serializeFeatures( QVector<QgsFeatureUniqueIdPair> &featureList );

    //! Called by QgsFeatureDownloader::run() at the end of the download process.
    void endOfDownload( bool success, long long featureCount, bool truncatedResponse, bool interrupted, const QString &errorMsg );

    //! Force an update of the feature count
    void setFeatureCount( long long featureCount, bool featureCountExact );

    //! Returns the name of temporary directory. To be paired with releaseCacheDirectory()
    QString acquireCacheDirectory();

    //! To be called when a temporary file is removed from the directory
    void releaseCacheDirectory();

    //! Set whether the progress dialog should be hidden
    void setHideProgressDialog( bool b ) { mHideProgressDialog = b; }

    //////// Pure virtual methods

    //! Instantiate a new feature downloader implementation.
    virtual std::unique_ptr<QgsFeatureDownloaderImpl> newFeatureDownloaderImpl( QgsFeatureDownloader *, bool requestMadeFromMainThread ) = 0;

    //! Return whether the GetFeature request should include the request bounding box.
    virtual bool isRestrictedToRequestBBOX() const = 0;

    //! Return whether the layer has a geometry field
    virtual bool hasGeometry() const = 0;

    //! Return layer name
    virtual QString layerName() const = 0;

    //! Called when an error must be raised to the provider
    virtual void pushError( const QString &errorMsg ) const = 0;

  protected:

    //////////// Input members. Implementations should define them to meaningful values

    //! Attribute fields of the layer
    QgsFields mFields;

    //! Source CRS
    QgsCoordinateReferenceSystem mSourceCrs;

    //! SELECT DISTINCT
    bool mDistinctSelect = false;

    //! Filter expression to apply on client side
    QString mClientSideFilterExpression;

    //! Server-side or user-side limit of downloaded features (including with paging). Valid if > 0
    long long mMaxFeatures = 0;

    //! Server-side limit of downloaded features (including with paging). Valid if > 0
    long long mServerMaxFeatures = 0;

    //! Bounding box for the layer as returned by GetCapabilities
    QgsRectangle mCapabilityExtent;

    //! Extent computed from downloaded features
    QgsRectangle mComputedExtent;

    //! Flag is a /items request returns a numberMatched property
    bool mHasNumberMatched = false;

    //! Whether progress dialog should be hidden
    bool mHideProgressDialog = false;

    //////////// Methods

    //! Should be called in the destructor of the implementation of this class !
    void cleanup();

    //! Returns true if it is likely that the server doesn't properly honor axis order.
    virtual bool detectPotentialServerAxisOrderIssueFromSingleFeatureExtent() const { return false; }

  private:

    //! Cache directory manager
    QgsCacheDirectoryManager &mCacheDirectoryManager;

    //! Main mutex to protect most data members that can be modified concurrently
    mutable QMutex mMutex;

    //! Mutex used specifically by registerToCache()
    QMutex mMutexRegisterToCache;

    //! Mutex used only by serializeFeatures()
    QMutex mCacheWriteMutex;

    //! For error messages, name of the translated component. For example tr("WFS")
    QString mComponentTranslated;

    //! Whether the downloader has finished (or been canceled)
    bool mDownloadFinished = false;

    /**
     * The generation counter. When a iterator is built or rewind, it gets the
     * current value of the generation counter to query the features in the cache
     * whose generation counter is <= the current value. That way the iterator
     * can consume first cached features, and then deal with the features that are
     * notified in live by the downloader.
    */
    int mGenCounter = 0;

    //! Spatial index of requested cached regions
    QgsSpatialIndex mCachedRegions;

    //! Requested cached regions
    QVector< QgsFeature > mRegions;

    //! Limit of retrieved number of features for the current request
    int mRequestLimit = 0;

    //! Current BBOX used by the downloader
    QgsRectangle mRect;

    //! The background feature downloader
    std::unique_ptr<QgsThreadedFeatureDownloader> mDownloader;

    //! Filename of the on-disk cache
    QString mCacheDbname;

    //! Tablename of the on-disk cache
    QString mCacheTablename;

    //! The data provider of the on-disk cache
    std::unique_ptr<QgsVectorDataProvider> mCacheDataProvider;

    //! Name of the gmlid, spatialite_id, qgis_id cache. This cache persists even after a layer reload so as to ensure feature id stability.
    QString mCacheIdDbname;

    //! Connection to mCacheIdDbname
    sqlite3_database_unique_ptr mCacheIdDb;

    //! Map each user visible field name to the column name in the spatialite DB cache
    // This is useful when there are user visible fields with same name, but different case
    std::map<QString, QString> mMapUserVisibleFieldNameToSpatialiteColumnName;

    //! Next value for qgisId column
    QgsFeatureId mNextCachedIdQgisId = 1;

    //! Number of features that have been cached, or attempted to be cached
    long long mTotalFeaturesAttemptedToBeCached = 0;

    //! Whether we have already tried fetching one feature after realizing that the capabilities extent is wrong
    bool mTryFetchingOneFeature = false;

    //! Number of features of the layer
    long long mFeatureCount = 0;

    //! Whether mFeatureCount value is exact or approximate / in construction
    bool mFeatureCountExact = false;

    //! Whether a request has been issued to retrieve the number of features
    bool mFeatureCountRequestIssued = false;

    ///////////////// METHODS ////////////////////////

    //! Create the on-disk cache and connect to it
    bool createCache();

    /**
     * Returns the set of unique ids that have already been downloaded and
     * cached, so as to avoid to cache duplicates.
    */
    QSet<QString> getExistingCachedUniqueIds( const QVector<QgsFeatureUniqueIdPair> &featureList );

    /**
     * Returns the set of md5 of features that have already been downloaded and
     * cached, so as to avoid to cache duplicates.
    */
    QSet<QString> getExistingCachedMD5( const QVector<QgsFeatureUniqueIdPair> &featureList );

    ///////////////// PURE VIRTUAL METHODS ////////////////////////

    //! Called when the extent is updated
    virtual void emitExtentUpdated() = 0;

    //! called by invalidateCache()
    virtual void invalidateCacheBaseUnderLock() = 0;

    //! Return whether the server limit downloading a limiter number of features
    virtual bool supportsLimitedFeatureCountDownloads() const = 0;

    //! Return whether a server-side (non-spatial) filter is applied
    virtual bool hasServerSideFilter() const = 0;

    //! Return whether the server supports a (relatively) fast way of reporting the feature count
    virtual bool supportsFastFeatureCount() const = 0;

    //! Launch a synchronous request for a single feature and return its extent.
    virtual QgsRectangle getExtentFromSingleFeatureRequest() const = 0;

    //! Launch a synchronous request to count the number of features (return -1 in case of error)
    virtual long long getFeatureCountFromServer() const = 0;
};

#endif
