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

#include <set>

#include "qgis.h"
#include "qgscoordinatereferencesystem.h"
#include "qgslayermetadata.h"
#include "qgsoapiffiltertranslationstate.h"
#include "qgsoapifitemsrequest.h"
#include "qgsprovidermetadata.h"
#include "qgsrectangle.h"
#include "qgsvectordataprovider.h"
#include "qgswfsdatasourceuri.h"

class QgsOapifSharedData;

class QgsOapifProvider final : public QgsVectorDataProvider
{
    Q_OBJECT
  public:
    static const QString OAPIF_PROVIDER_KEY;
    static const QString OAPIF_PROVIDER_DESCRIPTION;

    explicit QgsOapifProvider( const QString &uri, const QgsDataProvider::ProviderOptions &providerOptions, Qgis::DataProviderReadFlags flags = Qgis::DataProviderReadFlags() );
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
    QString subsetStringDialect() const override;
    QString subsetStringHelpUrl() const override;
    bool supportsSubsetString() const override;

    QString storageType() const override { return u"OGC API - Features"_s; }

    /* Inherited from QgsDataProvider */

    QgsRectangle extent() const override;
    bool isValid() const override;
    QString name() const override;
    QString description() const override;

    static QString providerKey();

    Qgis::VectorProviderCapabilities capabilities() const override;

    QgsLayerMetadata layerMetadata() const override { return mLayerMetadata; }

    bool empty() const override;

    QString geometryColumnName() const override;

    // For QgsWFSSourceSelect::buildQuery()
    QgsOapifFilterTranslationState filterTranslatedState() const;

    //! For QgsWFSSourceSelect::buildQuery()
    const QString &clientSideFilterExpression() const;

    void handlePostCloneOperations( QgsVectorDataProvider *source ) override;

    //Editing operations
    using QgsVectorDataProvider::addFeatures;
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
    Qgis::VectorProviderCapabilities mCapabilities;

    //! Whether server supports PATCH operation
    bool mSupportsPatch = false;

    //! String used to define a subset of the layer
    QString mSubsetString;

    //! Layer metadata
    QgsLayerMetadata mLayerMetadata;

    //! Feature count when known (currently only through ldproxy's itemCount)
    int64_t mFeatureCount = -1;

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

    //! Issue a GET /schema request and handle it
    void handleGetSchemaRequest( const QString &schemaUrl );
};

class QgsOapifProviderMetadata final : public QgsProviderMetadata
{
    Q_OBJECT
  public:
    QgsOapifProviderMetadata();
    QIcon icon() const override;
    QgsOapifProvider *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, Qgis::DataProviderReadFlags flags = Qgis::DataProviderReadFlags() ) override;
    QList<Qgis::LayerType> supportedLayerTypes() const override;
};

#endif /* QGSOAPIFPROVIDER_H */
