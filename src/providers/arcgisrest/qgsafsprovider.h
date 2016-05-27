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

#include "qgsvectordataprovider.h"
#include "qgsdatasourceuri.h"
#include "qgscoordinatereferencesystem.h"
#include "geometry/qgswkbtypes.h"

/**
 * @brief A provider reading features from a ArcGIS Feature Service
 */
class QgsAfsProvider : public QgsVectorDataProvider
{
    Q_OBJECT

  public:

    QgsAfsProvider( const QString& uri );
    ~QgsAfsProvider() {}

    bool getFeature( const QgsFeatureId& id, QgsFeature& f, bool fetchGeometry, const QList<int> &fetchAttributes, const QgsRectangle filterRect = QgsRectangle() );

    /* Inherited from QgsVectorDataProvider */
    QgsAbstractFeatureSource* featureSource() const override;
    QString storageType() const override { return "ESRI ArcGIS Feature Server"; }
    QgsFeatureIterator getFeatures( const QgsFeatureRequest& request = QgsFeatureRequest() ) override;
    QGis::WkbType geometryType() const override { return static_cast<QGis::WkbType>( mGeometryType ); }
    long featureCount() const override { return mObjectIds.size(); }
    const QgsFields &fields() const override { return mFields; }
    /* Read only for the moment
    bool addFeatures( QgsFeatureList &flist ) override{ return false; }
    bool deleteFeatures( const QgsFeatureIds &id ) override{ return false; }
    bool addAttributes( const QList<QgsField> &attributes ) override{ return false; }
    bool deleteAttributes( const QgsAttributeIds &attributes ) override{ return false; }
    bool changeAttributeValues( const QgsChangedAttributesMap &attr_map ) override{ return false; }
    bool changeGeometryValues( QgsGeometryMap & geometry_map ) override{ return false; }
    */
    int capabilities() const override { return QgsVectorDataProvider::NoCapabilities; }
    QgsAttributeList pkAttributeIndexes() override { return QgsAttributeList() << mObjectIdFieldIdx; }
    QgsAttrPalIndexNameHash palAttributeIndexNames() const override { return QgsAttrPalIndexNameHash(); }

    /* Inherited from QgsDataProvider */
    QgsCoordinateReferenceSystem crs() override { return mSourceCRS; }
    void setDataSourceUri( const QString & uri ) override;
    QgsRectangle extent() override { return mExtent; }
    bool isValid() override { return mValid; }
    /* Read only for the moment
    void updateExtents() override{}
    */
    QString name() const override { return mLayerName; }
    QString description() const override { return mLayerDescription; }
    void reloadData() override { mCache.clear(); }

  private:
    bool mValid;
    QgsDataSourceURI mDataSource;
    QgsRectangle mExtent;
    QgsWKBTypes::Type mGeometryType;
    QgsFields mFields;
    int mObjectIdFieldIdx;
    QString mObjectIdFieldName;
    QString mLayerName;
    QString mLayerDescription;
    QList<quint32> mObjectIds;
    QgsCoordinateReferenceSystem mSourceCRS;
    QMap<QgsFeatureId, QgsFeature> mCache;
};

#endif // QGSAFSPROVIDER_H
