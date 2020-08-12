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

#include "qgswfsfeatureiterator.h"
#include "qgswfsrequest.h"
#include "qgswfscapabilities.h"
#include "qgsogcutils.h"

#include "qgsbackgroundcachedshareddata.h"
#include "qgsbackgroundcachedfeatureiterator.h"

// !Class shared between provider and feature source
class QgsWFSSharedData : public QObject, public QgsBackgroundCachedSharedData
{
    Q_OBJECT
  public:
    explicit QgsWFSSharedData( const QString &uri );
    ~QgsWFSSharedData() override;

    //! Compute WFS filter from the sql or filter in the URI
    bool computeFilter( QString &errorMsg );

    //! Returns srsName
    QString srsName() const;

    //! Return provider geometry attribute name
    const QString &geometryAttribute() const { return mGeometryAttribute; }

    std::unique_ptr<QgsFeatureDownloaderImpl> newFeatureDownloaderImpl( QgsFeatureDownloader * ) override;

    bool isRestrictedToRequestBBOX() const override;

    bool hasGeometry() const override { return !mGeometryAttribute.isEmpty(); }

  signals:

    //! Raise error
    void raiseError( const QString &errorMsg );

    //! Extent has been updated
    void extentUpdated();

  protected:
    friend class QgsWFSFeatureDownloaderImpl;
    friend class QgsWFSProvider;
    friend class QgsWFSSingleFeatureRequest;

    //! Datasource URI
    QgsWFSDataSourceURI mURI;

    //! WFS version to use. Comes from GetCapabilities response
    QString mWFSVersion;

    //! Name of geometry attribute
    QString mGeometryAttribute;

    //! Layer properties
    QList< QgsOgcUtils::LayerProperties > mLayerPropertiesList;

    //! Map a field name to the pair (typename, fieldname) that describes its source field
    QMap< QString, QPair<QString, QString> > mMapFieldNameToSrcLayerNameFieldName;

    //! Page size for WFS 2.0. 0 = disabled
    int mPageSize = 0;

    //! Server capabilities
    QgsWfsCapabilities::Capabilities mCaps;

    //! If we have already issued a warning about missing feature ids
    bool mHasWarnedAboutMissingFeatureId = false;

    /**
     * If the server (typically MapServer WFS 1.1) honours EPSG axis order, but returns
        EPSG:XXXX srsName and not EPSG urns */
    bool mGetFeatureEPSGDotHonoursEPSGOrder = false;

    //! Geometry type of the features in this layer
    QgsWkbTypes::Type mWKBType = QgsWkbTypes::Unknown;

    //! Create GML parser
    QgsGmlStreamingParser *createParser() const;

  private:

    //! WFS filter
    QString mWFSFilter;

    //! WFS SORTBY
    QString mSortBy;

    //! Log error to QgsMessageLog and raise it to the provider
    void pushError( const QString &errorMsg ) override;

    void emitExtentUpdated() override { emit extentUpdated(); }

    void invalidateCacheBaseUnderLock() override;

    bool supportsLimitedFeatureCountDownloads() const override { return  !( mWFSVersion.startsWith( QLatin1String( "1.0" ) ) ); }

    QString layerName() const override { return mURI.typeName(); }

    bool hasServerSideFilter() const override { return !mWFSFilter.isEmpty(); }

    bool supportsFastFeatureCount() const override { return mCaps.supportsHits; }

    QgsRectangle getExtentFromSingleFeatureRequest() const override;

    int getFeatureCountFromServer() const override;
};

//! Utility class to issue a GetFeature resultType=hits request
class QgsWFSFeatureHitsRequest: public QgsWfsRequest
{
    Q_OBJECT
  public:
    explicit QgsWFSFeatureHitsRequest( const QgsWFSDataSourceURI &uri );

    //! Returns the feature count, or -1 in case of error
    int getFeatureCount( const QString &WFSVersion, const QString &filter, const QgsWfsCapabilities::Capabilities &caps );

  protected:
    QString errorMessageWithReason( const QString &reason ) override;
};

/**
 * Utility class to issue a GetFeature requets with maxfeatures/count=1
 * Used by QgsWFSSharedData::endOfDownload() when capabilities extent are likely wrong */
class QgsWFSSingleFeatureRequest: public QgsWfsRequest
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
