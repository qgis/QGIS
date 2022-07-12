/***************************************************************************
    memoryprovider.h - provider with storage in memory
    ------------------
    begin                : June 2008
    copyright            : (C) 2008 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#define SIP_NO_FILE

#include "qgsvectordataprovider.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsfields.h"
#include "qgsprovidermetadata.h"

///@cond PRIVATE
typedef QMap<QgsFeatureId, QgsFeature> QgsFeatureMap;

class QgsSpatialIndex;

class QgsMemoryFeatureIterator;

class QgsMemoryProvider final: public QgsVectorDataProvider
{
    Q_OBJECT

  public:
    explicit QgsMemoryProvider( const QString &uri, const QgsVectorDataProvider::ProviderOptions &coordinateTransformContext,
                                QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() );

    ~QgsMemoryProvider() override;

    //! Returns the memory provider key
    static QString providerKey();
    //! Returns the memory provider description
    static QString providerDescription();

    /* Implementation of functions from QgsVectorDataProvider */

    QgsAbstractFeatureSource *featureSource() const override;

    QString dataSourceUri( bool expandAuthConfig = true ) const override;
    QString storageType() const override;
    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request ) const override;
    QgsWkbTypes::Type wkbType() const override;
    long long featureCount() const override;
    QgsFields fields() const override;
    bool addFeatures( QgsFeatureList &flist, QgsFeatureSink::Flags flags = QgsFeatureSink::Flags() ) override;
    bool deleteFeatures( const QgsFeatureIds &id ) override;
    bool addAttributes( const QList<QgsField> &attributes ) override;
    bool renameAttributes( const QgsFieldNameMap &renamedAttributes ) override;
    bool deleteAttributes( const QgsAttributeIds &attributes ) override;
    bool changeAttributeValues( const QgsChangedAttributesMap &attr_map ) override;
    bool changeGeometryValues( const QgsGeometryMap &geometry_map ) override;
    QString subsetString() const override;
    bool setSubsetString( const QString &theSQL, bool updateFeatureCount = true ) override;
    bool supportsSubsetString() const override { return true; }
    bool createSpatialIndex() override;
    QgsFeatureSource::SpatialIndexPresence hasSpatialIndex() const override;
    QgsVectorDataProvider::Capabilities capabilities() const override;
    bool truncate() override;

    /* Implementation of functions from QgsDataProvider */

    QString name() const override;
    QString description() const override;
    QgsRectangle extent() const override;
    void updateExtents() override;
    bool isValid() const override;
    QgsCoordinateReferenceSystem crs() const override;
    void handlePostCloneOperations( QgsVectorDataProvider *source ) override;

  private:
    // Coordinate reference system
    QgsCoordinateReferenceSystem mCrs;

    // fields
    QgsFields mFields;
    QgsWkbTypes::Type mWkbType;
    mutable QgsRectangle mExtent;

    // features
    QgsFeatureMap mFeatures;
    QgsFeatureId mNextFeatureId;

    // indexing
    QgsSpatialIndex *mSpatialIndex = nullptr;

    QString mSubsetString;

    friend class QgsMemoryFeatureSource;
};


class QgsMemoryProviderMetadata final: public QgsProviderMetadata
{
    Q_OBJECT

  public:
    QgsMemoryProviderMetadata();
    QIcon icon() const override;
    QgsDataProvider *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() ) override;
    QList< QgsMapLayerType > supportedLayerTypes() const override;
};

///@endcond
