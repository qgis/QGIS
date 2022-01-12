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

#include "qgsprovidermetadata.h"

#include <set>

class QgsOapifSharedData;

class QgsOapifProvider final: public QgsVectorDataProvider
{
    Q_OBJECT
  public:

    static const QString OAPIF_PROVIDER_KEY;
    static const QString OAPIF_PROVIDER_DESCRIPTION;

    explicit QgsOapifProvider( const QString &uri, const QgsDataProvider::ProviderOptions &providerOptions, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() );
    ~QgsOapifProvider() override;

    /* Inherited from QgsVectorDataProvider */

    QgsAbstractFeatureSource *featureSource() const override;

    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request = QgsFeatureRequest() ) const override;

    QgsWkbTypes::Type wkbType() const override;
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

  private slots:

    void pushErrorSlot( const QString &errorMsg );

  private:
    std::shared_ptr<QgsOapifSharedData> mShared;

    //! Flag if provider is valid
    bool mValid = true;

    //! String used to define a subset of the layer
    QString mSubsetString;

    //! Layer metadata
    QgsLayerMetadata mLayerMetadata;

    //! Set to true by setSubsetString() if updateFeatureCount is true
    mutable bool mUpdateFeatureCountAtNextFeatureCountRequest = false;

    //! Initial requests
    bool init();

    /**
     * Invalidates cache of shared object
    */
    void reloadProviderData() override;
};

class QgsOapifProviderMetadata final: public QgsProviderMetadata
{
  public:
    QgsOapifProviderMetadata();
    QgsOapifProvider *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() ) override;
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

    bool hasGeometry() const override { return mWKBType != QgsWkbTypes::Unknown; }

    std::unique_ptr<QgsFeatureDownloaderImpl> newFeatureDownloaderImpl( QgsFeatureDownloader *, bool requestFromMainThread ) override;

    bool isRestrictedToRequestBBOX() const override;

  signals:

    //! Raise error
    void raiseError( const QString &errorMsg ) const;

    //! Extent has been updated
    void extentUpdated();

  protected:
    friend class QgsOapifProvider;
    friend class QgsOapifFeatureDownloaderImpl;

    //! Datasource URI
    QgsWFSDataSourceURI mURI;

    //! Geometry type of the features in this layer
    QgsWkbTypes::Type mWKBType = QgsWkbTypes::Unknown;

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

    //! Append extra query parameters if needed
    QString appendExtraQueryParameters( const QString &url ) const;

  private:

    // Translate part of an expression to a server-side filter
    QString translateNodeToServer( const QgsExpressionNode *node,
                                   QgsOapifProvider::FilterTranslationState &translationState,
                                   QString &untranslatedPart );

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

    DEFINE_FEATURE_DOWLOADER_IMPL_SLOTS

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
