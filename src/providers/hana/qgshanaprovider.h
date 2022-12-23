/***************************************************************************
   qgshanarovider.h  -  Data provider for SAP HANA
   --------------------------------------
   Date      : 31-05-2019
   Copyright : (C) SAP SE
   Author    : Maxim Rylov
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#ifndef QGSHANAPROVIDER_H
#define QGSHANAPROVIDER_H

#include "qgsfields.h"
#include "qgsprovidermetadata.h"
#include "qgshanaconnection.h"
#include "qgshanaprimarykeys.h"
#include "qgsvectordataprovider.h"

#include <QVersionNumber>

#include "odbc/Forwards.h"

class QgsFeature;
class QgsField;
class QDomDocument;

class QgsHanaConnectionRef;
class QgsHanaFeatureIterator;

/**
 * \class QgsHanaProvider
 * \brief Data provider for SAP HANA database.
*/
class QgsHanaProvider final : public QgsVectorDataProvider
{
    Q_OBJECT

  public:
    static const QString HANA_KEY;
    static const QString HANA_DESCRIPTION;

    QgsHanaProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options,
                     QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() );

    /* Functions inherited from QgsVectorDataProvider */

    QgsAbstractFeatureSource *featureSource() const override;
    QString storageType() const override;
    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request ) const override;
    QgsWkbTypes::Type wkbType() const override;
    QgsLayerMetadata layerMetadata() const override;
    QString dataComment() const override;
    long long featureCount() const override;
    QgsAttributeList pkAttributeIndexes() const override { return mPrimaryKeyAttrs; }
    QgsFields fields() const override;
    QVariant minimumValue( int index ) const override;
    QVariant maximumValue( int index ) const override;
    QSet< QVariant > uniqueValues( int index, int limit = -1 ) const override;
    QString subsetString() const override;
    bool setSubsetString( const QString &subset, bool updateFeatureCount = true ) override;
    bool supportsSubsetString() const override { return true; }
    bool addFeatures( QgsFeatureList &flist, QgsFeatureSink::Flags flags = QgsFeatureSink::Flags() ) override;
    bool deleteFeatures( const QgsFeatureIds &id ) override;
    bool truncate() override;
    bool addAttributes( const QList<QgsField> &attributes ) override;
    bool deleteAttributes( const QgsAttributeIds &attributes ) override;
    bool renameAttributes( const QgsFieldNameMap &fieldMap ) override;
    bool changeGeometryValues( const QgsGeometryMap &geometry_map ) override;
    bool changeFeatures(
      const QgsChangedAttributesMap &attrMap,
      const QgsGeometryMap &geometryMap ) override;
    bool changeAttributeValues( const QgsChangedAttributesMap &attrMap ) override;

    QgsVectorDataProvider::Capabilities capabilities() const override;
    QVariant defaultValue( int fieldId ) const override;

    /* Functions inherited from QgsDataProvider */

    QgsRectangle extent() const override;
    void updateExtents() override;
    bool isValid() const override;
    QString name() const override;
    QString description() const override;
    QgsCoordinateReferenceSystem crs() const override;

    //! Import a vector layer into the database
    static Qgis::VectorExportResult createEmptyLayer(
      const QString &uri,
      const QgsFields &fields,
      QgsWkbTypes::Type wkbType,
      const QgsCoordinateReferenceSystem &srs,
      bool overwrite,
      QMap<int, int> *oldToNewAttrIdxMap,
      QString *errorMessage = nullptr,
      const QMap<QString, QVariant> *options = nullptr
    );

    Qgis::VectorLayerTypeFlags vectorLayerTypeFlags() const override;

  private:
    QgsHanaConnectionRef createConnection() const;
    QString buildQuery( const QString &columns, const QString &where, const QString &orderBy, int limit ) const;
    QString buildQuery( const QString &columns, const QString &where ) const;
    QString buildQuery( const QString &columns ) const;
    bool checkPermissionsAndSetCapabilities( QgsHanaConnection &conn );
    QgsRectangle estimateExtent() const;
    void readAttributeFields( QgsHanaConnection &conn );
    void readGeometryType( QgsHanaConnection &conn );
    void readMetadata( QgsHanaConnection &conn );
    void readSrsInformation( QgsHanaConnection &conn );
    void determinePrimaryKey( QgsHanaConnection &conn );
    long long getFeatureCount( const QString &whereClause ) const;
    void updateFeatureIdMap( QgsFeatureId fid, const QgsAttributeMap &attributes );

  private:
    // Flag indicating whether the layer is a valid or not
    bool mValid = false;
    // Database version
    QVersionNumber mDatabaseVersion;
    // Data source URI
    QgsDataSourceUri mUri;
    // Srid of the geometry column
    int mSrid = -1;
    // Srs extent
    QgsRectangle mSrsExtent;
    // Flag that shows the presence of a planar equivalent in a database
    bool mHasSrsPlanarEquivalent = false;
    // Name of the table with no schema
    QString mTableName;
    // Name of the schema
    QString mSchemaName;
    // Data type for the primary key
    QgsHanaPrimaryKeyType mPrimaryKeyType = QgsHanaPrimaryKeyType::PktUnknown;
    // List of primary key attributes for fetching features
    QList<int> mPrimaryKeyAttrs;
    // Name of the geometry column
    QString mGeometryColumn;
    // Spatial type
    QgsWkbTypes::Type mRequestedGeometryType = QgsWkbTypes::Unknown;
    QgsWkbTypes::Type mDetectedGeometryType = QgsWkbTypes::Unknown;
    // Layer extent
    mutable QgsRectangle mLayerExtent;
    // Source for sql query
    QString mQuerySource;
    // Provider references query (instead of a table)
    bool mIsQuery = false;
    // Where clause of the SQL statement
    QString mQueryWhereClause;
    // Disable support for SelectAtId
    bool mSelectAtIdDisabled = false;
    // Attributes of nongeometry fields
    QgsFields mFields;
    AttributeFields mAttributeFields;
    //Capabilities of the layer
    QgsVectorDataProvider::Capabilities mCapabilities;
    // Default values of the result set
    QMap<int, QVariant> mDefaultValues;
    // Number of features in the layer
    mutable long long mFeaturesCount = 0;
    QgsLayerMetadata mLayerMetadata;
    std::shared_ptr<QgsHanaPrimaryKeyContext> mPrimaryKeyCntx;

    friend class QgsHanaFeatureSource;
};

class QgsHanaProviderMetadata : public QgsProviderMetadata
{
    Q_OBJECT

  public:
    QgsHanaProviderMetadata();
    QIcon icon() const override;

    void cleanupProvider() override;

    QgsHanaProvider *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() ) override;

    Qgis::VectorExportResult createEmptyLayer(
      const QString &uri,
      const QgsFields &fields,
      QgsWkbTypes::Type wkbType,
      const QgsCoordinateReferenceSystem &srs,
      bool overwrite,
      QMap<int, int> &oldToNewAttrIdxMap,
      QString &errorMessage,
      const QMap<QString, QVariant> *options ) override;

    QList<QgsDataItemProvider *> dataItemProviders() const override;

    // Connections API
    QMap<QString, QgsAbstractProviderConnection *> connections( bool cached = true ) override;
    QgsAbstractProviderConnection *createConnection( const QString &name ) override;
    QgsAbstractProviderConnection *createConnection( const QString &uri, const QVariantMap &configuration ) override;
    void deleteConnection( const QString &name ) override;
    void saveConnection( const QgsAbstractProviderConnection *createConnection, const QString &name ) override;

    // Data source URI API
    QVariantMap decodeUri( const QString &uri ) const override;
    QString encodeUri( const QVariantMap &parts ) const override;
    QList< QgsMapLayerType > supportedLayerTypes() const override;
};

#endif // QGSHANAPROVIDER_H
