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
#include "qgsvectordataprovider.h"
#include "qgsbackgroundcachedshareddata.h"
#include "qgswfsdatasourceuri.h"

#include "qgsprovidermetadata.h"

class QgsOapifSharedData;

class QgsOapifProvider : public QgsVectorDataProvider
{
    Q_OBJECT
  public:

    static const QString OAPIF_PROVIDER_KEY;
    static const QString OAPIF_PROVIDER_DESCRIPTION;

    explicit QgsOapifProvider( const QString &uri, const QgsDataProvider::ProviderOptions &providerOptions );
    ~QgsOapifProvider() override;

    /* Inherited from QgsVectorDataProvider */

    QgsAbstractFeatureSource *featureSource() const override;

    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request = QgsFeatureRequest() ) const override;

    QgsWkbTypes::Type wkbType() const override;
    long featureCount() const override;

    QgsFields fields() const override;

    QgsCoordinateReferenceSystem crs() const override;

    //QString subsetString() const override;
    //bool setSubsetString( const QString &theSQL, bool updateFeatureCount = true ) override;

    //bool supportsSubsetString() const override { return true; }
    bool supportsSubsetString() const override { return false; }

    /* Inherited from QgsDataProvider */

    QgsRectangle extent() const override;
    bool isValid() const override;
    QString name() const override;
    QString description() const override;

    QgsVectorDataProvider::Capabilities capabilities() const override;

    bool empty() const override;

  public slots:

    void reloadData() override;

  private slots:

    void pushErrorSlot( const QString &errorMsg );

  private:
    std::shared_ptr<QgsOapifSharedData> mShared;

    //! Flag if provider is valid
    bool mValid = true;

    //! Initial requests
    bool init();
};

class QgsOapifProviderMetadata: public QgsProviderMetadata
{
  public:
    QgsOapifProviderMetadata();
    QgsOapifProvider *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options ) override;
};

// !Class shared between provider and feature source
class QgsOapifSharedData : public QObject, public QgsBackgroundCachedSharedData
{
    Q_OBJECT
  public:
    explicit QgsOapifSharedData( const QString &uri );
    ~QgsOapifSharedData() override;

    bool hasGeometry() const override { return mWKBType != QgsWkbTypes::Unknown; }

    std::unique_ptr<QgsFeatureDownloaderImpl> newFeatureDownloaderImpl( QgsFeatureDownloader * ) override;

    bool isRestrictedToRequestBBOX() const override;

  signals:

    //! Raise error
    void raiseError( const QString &errorMsg );

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
    int mPageSize = 0;

    //! Whether progress dialog should be hidden
    bool mHideProgressDialog = false;

    //! Url to /collections/{collectionId}
    QString mCollectionUrl;

    //! Url to /collections/{collectionId}/items
    QString mItemsUrl;

  private:

    //! Log error to QgsMessageLog and raise it to the provider
    void pushError( const QString &errorMsg ) override;

    void emitExtentUpdated() override { emit extentUpdated(); }

    void invalidateCacheBaseUnderLock() override;

    bool supportsLimitedFeatureCountDownloads() const override { return true; }

    QString layerName() const override { return mURI.typeName(); }

    bool hasServerSideFilter() const override { return false; }

    bool supportsFastFeatureCount() const override { return false; }

    QgsRectangle getExtentFromSingleFeatureRequest() const override { return QgsRectangle(); }

    int getFeatureCountFromServer() const override { return -1; }
};


class QgsOapifFeatureDownloaderImpl: public QObject, public QgsFeatureDownloaderImpl
{
    Q_OBJECT
  public:
    QgsOapifFeatureDownloaderImpl( QgsOapifSharedData *shared, QgsFeatureDownloader *downloader );
    ~QgsOapifFeatureDownloaderImpl() override;

    void run( bool serializeFeatures, int maxFeatures ) override;

    void stop() override;

  signals:

    //! Used internally by the stop() method
    void doStop();

    //! Emitted with the total accumulated number of features downloaded.
    void updateProgress( int totalFeatureCount );

  private:

    //! Mutable data shared between provider, feature sources and downloader.
    QgsOapifSharedData *mShared = nullptr;

    //! Whether the download should stop
    bool mStop = false;
};


#endif /* QGSOAPIFPROVIDER_H */
