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

///@cond PRIVATE
typedef QMap<QgsFeatureId, QgsFeature> QgsFeatureMap;

class QgsSpatialIndex;

class QgsMemoryFeatureIterator;

class QgsMemoryProvider : public QgsVectorDataProvider
{
    Q_OBJECT

  public:
    explicit QgsMemoryProvider( const QString &uri = QString() );

    virtual ~QgsMemoryProvider();

    //! Returns the memory provider key
    static QString providerKey();
    //! Returns the memory provider description
    static QString providerDescription();

    static QgsMemoryProvider *createProvider( const QString &uri );

    /* Implementation of functions from QgsVectorDataProvider */

    virtual QgsAbstractFeatureSource *featureSource() const override;

    virtual QString dataSourceUri( bool expandAuthConfig = true ) const override;
    virtual QString storageType() const override;
    virtual QgsFeatureIterator getFeatures( const QgsFeatureRequest &request ) const override;
    virtual QgsWkbTypes::Type wkbType() const override;
    virtual long featureCount() const override;
    virtual QgsFields fields() const override;
    virtual bool addFeatures( QgsFeatureList &flist, QgsFeatureSink::Flags flags = nullptr ) override;
    virtual bool deleteFeatures( const QgsFeatureIds &id ) override;
    virtual bool addAttributes( const QList<QgsField> &attributes ) override;
    virtual bool renameAttributes( const QgsFieldNameMap &renamedAttributes ) override;
    virtual bool deleteAttributes( const QgsAttributeIds &attributes ) override;
    virtual bool changeAttributeValues( const QgsChangedAttributesMap &attr_map ) override;
    virtual bool changeGeometryValues( const QgsGeometryMap &geometry_map ) override;
    QString subsetString() const override;
    bool setSubsetString( const QString &theSQL, bool updateFeatureCount = true ) override;
    virtual bool supportsSubsetString() const override { return true; }
    virtual bool createSpatialIndex() override;
    virtual QgsVectorDataProvider::Capabilities capabilities() const override;

    /* Implementation of functions from QgsDataProvider */

    QString name() const override;
    QString description() const override;
    virtual QgsRectangle extent() const override;
    void updateExtents() override;
    bool isValid() const override;
    virtual QgsCoordinateReferenceSystem crs() const override;

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

///@endcond
