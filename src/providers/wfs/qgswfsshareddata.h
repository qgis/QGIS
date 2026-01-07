/***************************************************************************
    qgswfsshareddata.h
    ---------------------
    begin                : March 2016
    copyright            : (C) 2016 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSWFSSHAREDDATA_H
#define QGSWFSSHAREDDATA_H

#include "qgsbackgroundcachedfeatureiterator.h"
#include "qgsbackgroundcachedshareddata.h"
#include "qgsogcutils.h"
#include "qgswfscapabilities.h"
#include "qgswfsrequest.h"

class QgsFeatureDownloaderImpl;
class QgsGmlStreamingParser;

//! Class shared between provider and feature source
class QgsWFSSharedData : public QObject, public QgsBackgroundCachedSharedData
{
    Q_OBJECT
  public:
    explicit QgsWFSSharedData( const QString &uri );
    ~QgsWFSSharedData() override;

    //! Compute WFS filter from the sql or filter in the URI
    bool computeFilter( QString &errorMsg );

    //! Returns computed WFS server expression
    QString computedExpression( const QgsExpression &expression ) const override;

    //! Returns srsName
    QString srsName() const;

    //! Return list of layer properties.
    const QList<QgsOgcUtils::LayerProperties> &layerProperties() const { return mLayerPropertiesList; }

    std::unique_ptr<QgsFeatureDownloaderImpl> newFeatureDownloaderImpl( QgsFeatureDownloader *, bool requestFromMainThread ) override;

    bool isRestrictedToRequestBBOX() const override;

    bool hasGeometry() const override { return !mGeometryAttribute.isEmpty(); }

    const QgsWfsCapabilities &capabilities() const { return mCaps; }

    //! Set a new filter and return the previous one. Only used to temporarily disable filtering when trying to get layer geometry type.
    QString setWFSFilter( const QString &newFilter )
    {
      QString oldFilter = mWFSFilter;
      mWFSFilter = newFilter;
      return oldFilter;
    }

    //! Returns the WFS filter computed by computeFilter()
    const QString &WFSFilter() const { return mWFSFilter; }

    //! Compute mWFSGeometryTypeFilter
    void computeGeometryTypeFilter();

    //! Combine several WFS filters together with a And condition
    QString combineWFSFilters( const std::vector<QString> &filters ) const;

    //! Creates a deep copy of this shared data
    QgsWFSSharedData *clone() const;

    /**
     * Returns TRUE if the initial GetFeature request was issued
     * \note This does not mean that the request actually returned any feature, only that it was completed successfully.
     */
    bool initialGetFeatureIssued() const;

    /**
     * Sets whether the initial GetFeature request was \a issued
     * \note This does not mean that the request actually returned any feature, only that it was completed successfully.
     */
    void setInitialGetFeatureIssued( bool issued );

  signals:

    //! Raise error
    void raiseError( const QString &errorMsg ) const;

    //! Extent has been updated
    void extentUpdated();

  protected:
    friend class QgsWFSFeatureDownloaderImpl;
    friend class QgsWFSProvider;
    friend class QgsWFSSingleFeatureRequest;

    //! WFS version to use. Comes from GetCapabilities response
    QString mWFSVersion;

    //! Layer properties
    QList<QgsOgcUtils::LayerProperties> mLayerPropertiesList;

    //! Map a field name to the pair (typename, fieldname) that describes its source field
    QMap<QString, QPair<QString, QString>> mMapFieldNameToSrcLayerNameFieldName;

    //! Preferred HTTP method
    Qgis::HttpMethod mHttpMethod = Qgis::HttpMethod::Get;

    //! Page size for WFS 2.0. 0 = disabled
    long long mPageSize = 0;

    //! Server capabilities
    QgsWfsCapabilities mCaps;

    /**
     * If the server (typically MapServer WFS 1.1) honours EPSG axis order, but returns
     * EPSG:XXXX srsName and not EPSG urns
    */
    bool mGetFeatureEPSGDotHonoursEPSGOrder = false;

    /**
     * If the server (typically ESRI with WFS-T 1.1 in 2020) does not like "pos" and "posList", and requires "coordinates" for WFS 1.1 transactions
     */
    bool mServerPrefersCoordinatesForTransactions_1_1 = false;

    //! Geometry type filter to ensure geometries returned by the layer are of type mWKBType.
    QString mWFSGeometryTypeFilter;

    //! Create GML parser
    QgsGmlStreamingParser *createParser() const;

    //! Returns true if it is likely that the server doesn't properly honor axis order.
    bool detectPotentialServerAxisOrderIssueFromSingleFeatureExtent() const override;

  private:
    //! WFS filter
    QString mWFSFilter;

    //! WFS SORTBY
    QString mSortBy;

    //! Log error to QgsMessageLog and raise it to the provider
    void pushError( const QString &errorMsg ) const override;

    void emitExtentUpdated() override { emit extentUpdated(); }

    void invalidateCacheBaseUnderLock() override;

    bool supportsLimitedFeatureCountDownloads() const override { return !( mWFSVersion.startsWith( "1.0"_L1 ) ); }

    bool hasServerSideFilter() const override { return !mWFSFilter.isEmpty(); }

    bool supportsFastFeatureCount() const override { return mCaps.supportsHits; }

    QgsRectangle getExtentFromSingleFeatureRequest() const override;

    long long getFeatureCountFromServer() const override;

    void getVersionValues( QgsOgcUtils::GMLVersion &gmlVersion, QgsOgcUtils::FilterVersion &filterVersion, bool &honourAxisOrientation ) const;

    bool mInitialGetFeatureIssued = false;
};

//! Utility class to issue a GetFeature resultType=hits request
class QgsWFSFeatureHitsRequest : public QgsWfsRequest
{
    Q_OBJECT
  public:
    explicit QgsWFSFeatureHitsRequest( const QgsWFSDataSourceURI &uri );

    //! Returns the feature count, or -1 in case of error
    long long getFeatureCount( const QString &WFSVersion, const QString &filter, const QgsWfsCapabilities &caps );

  protected:
    QString errorMessageWithReason( const QString &reason ) override;
};

/**
 * Utility class to issue a GetFeature requets with maxfeatures/count=1
 * Used by QgsWFSSharedData::endOfDownload() when capabilities extent are likely wrong
*/
class QgsWFSSingleFeatureRequest : public QgsWfsRequest
{
    Q_OBJECT
  public:
    explicit QgsWFSSingleFeatureRequest( const QgsWFSSharedData *shared );

    //! Returns the feature  extent of the single feature requested
    QgsRectangle getExtent();

  protected:
    QString errorMessageWithReason( const QString &reason ) override;

  private:
    const QgsWFSSharedData *mShared = nullptr;
};

#endif // QGSWFSSHAREDDATA_H
