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

/**
 * \brief A provider reading features from a ArcGIS Feature Service
 */
class QgsAfsProvider : public QgsVectorDataProvider
{
    Q_OBJECT

  public:

    QgsAfsProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options );

    /* Inherited from QgsVectorDataProvider */
    QgsAbstractFeatureSource *featureSource() const override;
    QString storageType() const override { return QStringLiteral( "ESRI ArcGIS Feature Server" ); }
    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request = QgsFeatureRequest() ) const override;
    QgsWkbTypes::Type wkbType() const override;
    long featureCount() const override;
    QgsFields fields() const override;
    QgsLayerMetadata layerMetadata() const override;
    /* Read only for the moment
    bool addFeatures( QgsFeatureList &flist ) override{ return false; }
    bool deleteFeatures( const QgsFeatureIds &id ) override{ return false; }
    bool addAttributes( const QList<QgsField> &attributes ) override{ return false; }
    bool deleteAttributes( const QgsAttributeIds &attributes ) override{ return false; }
    bool changeAttributeValues( const QgsChangedAttributesMap &attr_map ) override{ return false; }
    bool changeGeometryValues( QgsGeometryMap & geometry_map ) override{ return false; }
    */
    QgsVectorDataProvider::Capabilities capabilities() const override;
    QgsAttributeList pkAttributeIndexes() const override { return QgsAttributeList() << mObjectIdFieldIdx; }
    QgsAttrPalIndexNameHash palAttributeIndexNames() const override { return QgsAttrPalIndexNameHash(); }

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
    void reloadData() override;
    QgsFeatureRenderer *createRenderer( const QVariantMap &configuration = QVariantMap() ) const override;
    QgsAbstractVectorLayerLabeling *createLabeling( const QVariantMap &configuration = QVariantMap() ) const override;

  private:
    bool mValid = false;
    std::shared_ptr<QgsAfsSharedData> mSharedData;
    int mObjectIdFieldIdx = -1;
    QString mLayerName;
    QString mLayerDescription;
    QgsLayerMetadata mLayerMetadata;
    QVariantMap mRendererDataMap;
    QVariantList mLabelingDataList;
    QgsStringMap mRequestHeaders;
};

#endif // QGSAFSPROVIDER_H
