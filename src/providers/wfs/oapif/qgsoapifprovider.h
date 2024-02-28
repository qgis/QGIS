/***************************************************************************
    qgsoapifprovider.h
    ---------------------
    begin                : October 2019
    copyright            : (C) 2019 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSOAPIFPROVIDER_H
#define QGSOAPIFPROVIDER_H

#include "qgis.h"
#include "qgsrectangle.h"
#include "qgscoordinatereferencesystem.h"
#include "qgslayermetadata.h"
#include "qgsvectordataprovider.h"
#include "qgsbackgroundcachedshareddata.h"
#include "qgswfsdatasourceuri.h"
#include "qgsoapifapirequest.h"
#include "qgsoapifitemsrequest.h"
#include "qgsoapifqueryablesrequest.h"

#include "qgsprovidermetadata.h"

#include <set>

class QgsOapifSharedData;

class QgsOapifProvider final: public QgsVectorDataProvider
{
    Q_OBJECT
  public:

    static const QString OAPIF_PROVIDER_KEY;
    static const QString OAPIF_PROVIDER_DESCRIPTION;

    static const QString OAPIF_PROVIDER_DEFAULT_CRS;

    explicit QgsOapifProvider( const QString &uri, const QgsDataProvider::ProviderOptions &providerOptions, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() );
    ~QgsOapifProvider() override;

    /* Inherited from QgsVectorDataProvider */

    QgsAbstractFeatureSource *featureSource() const override;

    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request = QgsFeatureRequest() ) const override;

    Qgis::WkbType wkbType() const override;
    long long featureCount() const override;

    QgsFields fields() const override;

    QgsCoordinateReferenceSystem crs() const override;

    QString subsetString() const override { return mSubsetString; }
    bool setSubsetString( const QString &theSQL, bool updateFeatureCount = true ) override;

    bool supportsSubsetString() const override { return true; }

    QString storageType() const override { return QStringLiteral( "OGC API - Features" ); }

    /* Inherited from QgsDataProvider */

    QgsRectangle extent() const override;
    bool isValid() const override;
    QString name() const override;
    QString description() const override;

    static QString providerKey();

    QgsVectorDataProvider::Capabilities capabilities() const override;

    QgsLayerMetadata layerMetadata() const override { return mLayerMetadata; }

    bool empty() const override;

    enum class FilterTranslationState
    {
      FULLY_CLIENT,
      PARTIAL,
      FULLY_SERVER
    };

    // For QgsWFSSourceSelect::buildQuery()
    FilterTranslationState filterTranslatedState() const;

    //! For QgsWFSSourceSelect::buildQuery()
    const QString &clientSideFilterExpression() const;

    void handlePostCloneOperations( QgsVectorDataProvider *source ) override;

    //Editing operations

    bool addFeatures( QgsFeatureList &flist, QgsFeatureSink::Flags flags = QgsFeatureSink::Flags() ) override;
    bool deleteFeatures( const QgsFeatureIds &ids ) override;
    bool changeGeometryValues( const QgsGeometryMap &geometry_map ) override;
    bool changeAttributeValues( const QgsChangedAttributesMap &attr_map ) override;

  private slots:

    void pushErrorSlot( const QString &errorMsg );

  private:
    std::shared_ptr<QgsOapifSharedData> mShared;

    //! Flag if provider is valid
    bool mValid = true;

    //! Server capabilities for this layer (generated from capabilities document)
    QgsVectorDataProvider::Capabilities mCapabilities = QgsVectorDataProvider::Capabilities();

    //! Whether server supports PATCH operation
    bool mSupportsPatch = false;

    //! String used to define a subset of the layer
    QString mSubsetString;

    //! Layer metadata
    QgsLayerMetadata mLayerMetadata;

    //! Set to true by reloadProviderData()
    mutable bool mUpdateFeatureCountAtNextFeatureCountRequest = true;

    //! Initial requests
    bool init();

    /**
     * Invalidates cache of shared object
    */
    void reloadProviderData() override;

    //! Compute capabilities
    void computeCapabilities( const QgsOapifItemsRequest &itemsRequest );
};

class QgsOapifProviderMetadata final: public QgsProviderMetadata
{
    Q_OBJECT
  public:
    QgsOapifProviderMetadata();
    QIcon icon() const override;
    QgsOapifProvider *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() ) override;
    QList< Qgis::LayerType > supportedLayerTypes() const override;
};

//! Class shared between provider and feature source
class QgsOapifSharedData final: public QObject, public QgsBackgroundCachedSharedData
{
    Q_OBJECT
  public:
    explicit QgsOapifSharedData( const QString &uri );
    ~QgsOapifSharedData() override;

    //! Compute OAPIF filter from the filter in the URI
    bool computeServerFilter( QString &errorMsg );

    QString computedExpression( const QgsExpression &expression ) const override;

    bool hasGeometry() const override { return mWKBType != Qgis::WkbType::Unknown; }

    std::unique_ptr<QgsFeatureDownloaderImpl> newFeatureDownloaderImpl( QgsFeatureDownloader *, bool requestFromMainThread ) override;

    bool isRestrictedToRequestBBOX() const override;

    //! Creates a deep copy of this shared data
    QgsOapifSharedData *clone() const;

  signals:

    //! Raise error
    void raiseError( const QString &errorMsg ) const;

    //! Extent has been updated
    void extentUpdated();

  protected:
    friend class QgsOapifProvider;
    friend class QgsOapifFeatureDownloaderImpl;
    friend class QgsOapifCreateFeatureRequest;
    friend class QgsOapifPutFeatureRequest;
    friend class QgsOapifPatchFeatureRequest;

    //! Datasource URI
    QgsWFSDataSourceURI mURI;

    //! Geometry type of the features in this layer
    Qgis::WkbType mWKBType = Qgis::WkbType::Unknown;

    //! Page size. 0 = disabled
    long long mPageSize = 0;

    //! Extra query parameters from the URI, to append to other requests
    QString mExtraQueryParameters;

    //! Url to /collections/{collectionId}
    QString mCollectionUrl;

    //! Url to /collections/{collectionId}/items
    QString mItemsUrl;

    //! Server filter
    QString mServerFilter;

    //! Translation state of filter to server-side filter.
    QgsOapifProvider::FilterTranslationState mFilterTranslationState = QgsOapifProvider::FilterTranslationState::FULLY_CLIENT;

    //! Set if an "id" is present at top level of features
    bool mFoundIdTopLevel = false;

    //! Set if an "id" is present in the "properties" object of features
    bool mFoundIdInProperties = false;

    // Map of simple queryables items (that is as query parameters). The key of the map is a queryable name.
    QMap<QString, QgsOapifApiRequest::SimpleQueryable> mSimpleQueryables;

    //! Whether server supports OGC API Features Part3 with CQL2-Text
    bool mServerSupportsFilterCql2Text = false;

    //! Whether server supports CQL2 advanced-comparison-operators conformance class (LIKE, BETWEEN, IN)
    bool mServerSupportsLikeBetweenIn = false;

    //! Whether server supports CQL2 case-insensitive-comparison conformance class (CASEI function)
    bool mServerSupportsCaseI = false;

    //! Whether server supports CQL2 basic-spatial-operators conformance class (S_INTERSECTS(,BBOX() or POINT()))
    bool mServerSupportsBasicSpatialOperators = false;

    // Map of queryables items for CQL2 request. The key of the map is a queryable name.
    QMap<QString, QgsOapifQueryablesRequest::Queryable> mQueryables;

    //! Append extra query parameters if needed
    QString appendExtraQueryParameters( const QString &url ) const;

  private:

    // Translate part of an expression to a server-side filter using Part1 features only
    QString compileExpressionNodeUsingPart1( const QgsExpressionNode *node,
        QgsOapifProvider::FilterTranslationState &translationState,
        QString &untranslatedPart ) const;

    // Translate part of an expression to a server-side filter using Part1 or Part3
    bool computeFilter( const QgsExpression &expr,
                        QgsOapifProvider::FilterTranslationState &translationState,
                        QString &serverSideParameters,
                        QString &clientSideFilterExpression ) const;

    //! Log error to QgsMessageLog and raise it to the provider
    void pushError( const QString &errorMsg ) const override;

    void emitExtentUpdated() override { emit extentUpdated(); }

    void invalidateCacheBaseUnderLock() override;

    bool supportsLimitedFeatureCountDownloads() const override { return true; }

    QString layerName() const override { return mURI.typeName(); }

    bool hasServerSideFilter() const override { return false; }

    bool supportsFastFeatureCount() const override { return false; }

    QgsRectangle getExtentFromSingleFeatureRequest() const override { return QgsRectangle(); }

    long long getFeatureCountFromServer() const override { return -1; }
};


class QgsOapifFeatureDownloaderImpl final: public QObject, public QgsFeatureDownloaderImpl
{
    Q_OBJECT

    DEFINE_FEATURE_DOWNLOADER_IMPL_SLOTS

  signals:
    /* Used internally by the stop() method */
    void doStop();

    /* Emitted with the total accumulated number of features downloaded. */
    void updateProgress( long long totalFeatureCount );

  public:
    QgsOapifFeatureDownloaderImpl( QgsOapifSharedData *shared, QgsFeatureDownloader *downloader, bool requestMadeFromMainThread );
    ~QgsOapifFeatureDownloaderImpl() override;

    void run( bool serializeFeatures, long long maxFeatures ) override;

  private slots:
    void createProgressDialog();

  private:

    //! Mutable data shared between provider, feature sources and downloader.
    QgsOapifSharedData *mShared = nullptr;

    int mNumberMatched = -1;
};


#endif /* QGSOAPIFPROVIDER_H */
