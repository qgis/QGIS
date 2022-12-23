/***************************************************************************
      qgsafsprovider.h
      ----------------
    begin                : May 27, 2015
    copyright            : (C) 2015 Sandro Mani
    email                : smani@sourcepole.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAFSPROVIDER_H
#define QGSAFSPROVIDER_H

#include <memory>
#include "qgsvectordataprovider.h"
#include "qgsdatasourceuri.h"
#include "qgsafsshareddata.h"
#include "qgscoordinatereferencesystem.h"
#include "geometry/qgswkbtypes.h"
#include "qgsfields.h"
#include "qgslayermetadata.h"

#include "qgsprovidermetadata.h"
#include "qgshttpheaders.h"

/**
 * \brief A provider reading features from a ArcGIS Feature Service
 */
class QgsAfsProvider : public QgsVectorDataProvider
{
    Q_OBJECT

  public:

    static const QString AFS_PROVIDER_KEY;
    static const QString AFS_PROVIDER_DESCRIPTION;

    QgsAfsProvider( const QString &uri, const QgsDataProvider::ProviderOptions &providerOptions, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() );

    /* Inherited from QgsVectorDataProvider */
    QgsAbstractFeatureSource *featureSource() const override;
    QString storageType() const override { return QStringLiteral( "ArcGIS Feature Service" ); }
    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request = QgsFeatureRequest() ) const override;
    QgsWkbTypes::Type wkbType() const override;
    long long featureCount() const override;
    QgsFields fields() const override;
    QgsLayerMetadata layerMetadata() const override;
    bool deleteFeatures( const QgsFeatureIds &id ) override;
    bool addFeatures( QgsFeatureList &flist, QgsFeatureSink::Flags flags = QgsFeatureSink::Flags() ) override;
    bool changeAttributeValues( const QgsChangedAttributesMap &attrMap ) override;
    bool changeGeometryValues( const QgsGeometryMap &geometryMap ) override;
    bool changeFeatures( const QgsChangedAttributesMap &attrMap, const QgsGeometryMap &geometryMap ) override;
    bool addAttributes( const QList<QgsField> &attributes ) override;
    bool deleteAttributes( const QgsAttributeIds &attributes ) override;
    bool createAttributeIndex( int field ) override;

    QgsVectorDataProvider::Capabilities capabilities() const override;
    QgsAttributeList pkAttributeIndexes() const override;
    QString defaultValueClause( int fieldId ) const override;
    QgsAttrPalIndexNameHash palAttributeIndexNames() const override { return QgsAttrPalIndexNameHash(); }
    bool skipConstraintCheck( int fieldIndex, QgsFieldConstraints::Constraint constraint, const QVariant &value = QVariant() ) const override;

    QString subsetString() const override;
    bool setSubsetString( const QString &subset, bool updateFeatureCount = true ) override;
    bool supportsSubsetString() const override { return true; }

    /* Inherited from QgsDataProvider */
    QgsCoordinateReferenceSystem crs() const override;
    void setDataSourceUri( const QString &uri ) override;
    QgsRectangle extent() const override;
    bool isValid() const override { return mValid; }
    /* Read only for the moment
    void updateExtents() override{}
    */
    QString name() const override;
    QString description() const override;
    QString dataComment() const override;
    QgsFeatureRenderer *createRenderer( const QVariantMap &configuration = QVariantMap() ) const override;
    QgsAbstractVectorLayerLabeling *createLabeling( const QVariantMap &configuration = QVariantMap() ) const override;
    bool renderInPreview( const QgsDataProvider::PreviewContext &context ) override;

    static QString providerKey();

    void handlePostCloneOperations( QgsVectorDataProvider *source ) override;

  private:
    bool mValid = false;
    std::shared_ptr<QgsAfsSharedData> mSharedData;
    QString mLayerName;
    QString mLayerDescription;
    QStringList mCapabilityStrings;
    QgsLayerMetadata mLayerMetadata;
    QVariantMap mRendererDataMap;
    QVariantList mLabelingDataList;
    QgsHttpHeaders mRequestHeaders;
    bool mServerSupportsCurves = false;
    QString mAdminUrl;
    QVariantMap mAdminData;
    QStringList mAdminCapabilityStrings;

    /**
     * Clears cache
    */
    void reloadProviderData() override;
};

class QgsAfsProviderMetadata: public QgsProviderMetadata
{
    Q_OBJECT
  public:
    QgsAfsProviderMetadata();
    QIcon icon() const override;
    QList<QgsDataItemProvider *> dataItemProviders() const override;
    QVariantMap decodeUri( const QString &uri ) const override;
    QString encodeUri( const QVariantMap &parts ) const override;
    QgsAfsProvider *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() ) override;
    QList< QgsMapLayerType > supportedLayerTypes() const override;
};

#endif // QGSAFSPROVIDER_H
