/***************************************************************************
   qgshanarovider.h  -  Data provider for SAP HANA
   --------------------------------------
   Date      : 31-05-2019
   Copyright : (C) SAP SE
   Author    : Maksim Rylov
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
#ifdef HAVE_GUI
#include "qgsproviderguimetadata.h"
#endif
#include "qgsvectordataprovider.h"

#include <QVersionNumber>

#include "odbc/Forwards.h"

class QgsFeature;
class QgsField;
class QDomDocument;

class QgsHanaFeatureIterator;

struct FieldInfo
{
  short type;
  bool isAutoIncrement;
  bool isNullable;
  bool isSigned;
};

/**
\class QgsHanaProvider
\brief Data provider for SAP HANA database.
*
*/
class QgsHanaProvider : public QgsVectorDataProvider
{
  Q_OBJECT

public:
  static const QString HANA_KEY;
  static const QString HANA_DESCRIPTION;

  explicit QgsHanaProvider(const QString &uri, const QgsDataProvider::ProviderOptions &options);
  ~QgsHanaProvider() override;

  /* Functions inherited from QgsVectorDataProvider */

  QgsAbstractFeatureSource *featureSource() const override;
  QString storageType() const override;
  QgsFeatureIterator getFeatures(const QgsFeatureRequest &request) const override;
  QgsWkbTypes::Type wkbType() const override;
  long featureCount() const override;
  QgsFields fields() const override;
  QString subsetString() const override;
  bool setSubsetString(const QString &subset, bool updateFeatureCount = true) override;
  bool supportsSubsetString() const override { return true; }
  bool addFeatures(QgsFeatureList &flist, QgsFeatureSink::Flags flags = nullptr) override;
  bool deleteFeatures(const QgsFeatureIds &id) override;
  bool truncate() override;
  bool addAttributes(const QList<QgsField> &attributes) override;
  bool deleteAttributes(const QgsAttributeIds &attributes) override;
  bool renameAttributes(const QgsFieldNameMap &fieldMap) override;
  bool changeGeometryValues(const QgsGeometryMap &geometry_map) override;
  bool changeFeatures(
    const QgsChangedAttributesMap &attrMap,
    const QgsGeometryMap &geometryMap) override;
  bool changeAttributeValues(const QgsChangedAttributesMap &attrMap) override;

  QgsVectorDataProvider::Capabilities capabilities() const override;
  QVariant defaultValue(int fieldId) const override;

  /* Functions inherited from QgsDataProvider */

  QgsRectangle extent() const override;
  bool isValid() const override;
  QString name() const override;
  QString description() const override;
  QgsCoordinateReferenceSystem crs() const override;

  //! Import a vector layer into the database
  static QgsVectorLayerExporter::ExportError createEmptyLayer(
    const QString &uri,
    const QgsFields &fields,
    QgsWkbTypes::Type wkbType,
    const QgsCoordinateReferenceSystem &srs,
    bool overwrite,
    QMap<int, int> *oldToNewAttrIdxMap,
    QString *errorMessage = nullptr,
    const QMap<QString, QVariant> *options = nullptr
  );

private:
  bool checkPermissionsAndSetCapabilities();
  QgsRectangle estimateExtent() const;
  bool isSrsRoundEarth(int srsID) const;
  int readSrid();
  void readSrsInformation();
  void readAttributeFields();
  long getFeatureCount(const QString& whereClause) const;

private:
  // Database version
  QVersionNumber mDatabaseVersion;
  // Data source URI
  QgsDataSourceUri mUri;
  // Srid of the geometry column
  int mSrid;
  // Srs extent
  QgsRectangle mSrsExtent;
  // Flag that shows the presence of a planar equivalent in a database
  bool mHasSrsPlanarEquivalent;
  // Name of the table with no schema
  QString mTableName;
  // Name of the schema
  QString mSchemaName;
  // Name of the feature id column
  QString mFidColumn;
  // Name of the geometry column
  QString mGeometryColumn;
  // Spatial type
  QgsWkbTypes::Type mGeometryType = QgsWkbTypes::Unknown;
  // Layer extent
  mutable QgsRectangle mLayerExtent;
  // Full sql query
  QString mQuery;
  // Provider references query (instead of a table)
  bool mIsQuery;
  // Where clause of the SQL statement
  QString mQueryWhereClause;
  // Disable support for SelectAtId
  bool mSelectAtIdDisabled;
  // Flag indicating whether the layer is a valid HANA layer
  bool mValid = false;
  // Attributes of nongeometry fields
  QgsFields mAttributeFields;
  // Additional information about HANA fields
  QVector<FieldInfo> mFieldInfos;
  //Capabilities of the layer
  QgsVectorDataProvider::Capabilities mCapabilities;
  // Default values of the result set
  QMap<int, QVariant> mDefaultValues;
  // Number of features in the layer
  mutable long mFeaturesCount = 0;

  friend class QgsHanaFeatureSource;
};

class QgsHanaProviderMetadata : public QgsProviderMetadata
{
public:
  QgsHanaProviderMetadata();

  void initProvider() override;

  void cleanupProvider() override;

  QgsHanaProvider *createProvider(const QString &uri, const QgsDataProvider::ProviderOptions &options) override;

  QgsVectorLayerExporter::ExportError createEmptyLayer(
    const QString &uri,
    const QgsFields &fields,
    QgsWkbTypes::Type wkbType,
    const QgsCoordinateReferenceSystem &srs,
    bool overwrite,
    QMap<int, int> &oldToNewAttrIdxMap,
    QString &errorMessage,
    const QMap<QString, QVariant> *options) override;

  QList<QgsDataItemProvider *> dataItemProviders() const override;
};

#endif // QGSHANAPROVIDER_H
