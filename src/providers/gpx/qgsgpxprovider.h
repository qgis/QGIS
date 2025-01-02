/***************************************************************************
      qgsgpxprovider.h  -  Data provider for GPS eXchange files
                             -------------------
    begin                : 2004-04-14
    copyright            : (C) 2004 by Lars Luthman
    email                : larsl@users.sourceforge.net

    Partly based on qgsdelimitedtextprovider.h, (C) 2004 Gary E. Sherman
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGPXPROVIDER_H
#define QGSGPXPROVIDER_H

#include "qgsvectordataprovider.h"
#include "gpsdata.h"
#include "qgsfields.h"
#include "qgsprovidermetadata.h"

class QgsFeature;
class QgsField;
class QFile;
class QDomDocument;
class QgsGpsData;

class QgsGPXFeatureIterator;

/**
 * \class QgsGPXProvider
 * \brief Data provider for GPX (GPS eXchange) files
 * This provider adds the ability to load GPX files as vector layers.
*/
class QgsGPXProvider final : public QgsVectorDataProvider
{
    Q_OBJECT

  public:
    explicit QgsGPXProvider( const QString &uri, const QgsDataProvider::ProviderOptions &providerOptions, Qgis::DataProviderReadFlags flags = Qgis::DataProviderReadFlags() );
    ~QgsGPXProvider() override;

    /* Functions inherited from QgsVectorDataProvider */

    QgsAbstractFeatureSource *featureSource() const override;
    QString storageType() const override;
    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request ) const override;
    Qgis::WkbType wkbType() const override;
    long long featureCount() const override;
    QgsFields fields() const override;
    bool addFeatures( QgsFeatureList &flist, QgsFeatureSink::Flags flags = QgsFeatureSink::Flags() ) override;
    bool deleteFeatures( const QgsFeatureIds &id ) override;
    bool changeAttributeValues( const QgsChangedAttributesMap &attr_map ) override;
    Qgis::VectorProviderCapabilities capabilities() const override;
    QVariant defaultValue( int fieldId ) const override;


    /* Functions inherited from QgsDataProvider */

    QgsRectangle extent() const override;
    bool isValid() const override;
    QString name() const override;
    QString description() const override;
    QgsCoordinateReferenceSystem crs() const override;


    /* new functions */

    void changeAttributeValues( QgsGpsObject &obj, const QgsAttributeMap &attrs );

    bool addFeature( QgsFeature &f, QgsFeatureSink::Flags flags = QgsFeatureSink::Flags() ) override;

    static QString encodeUri( const QVariantMap &parts );
    static QVariantMap decodeUri( const QString &uri );

    enum DataType
    {
      WaypointType = 1,
      RouteType = 2,
      TrackType = 4,

      TrkRteType = RouteType | TrackType,
      AllType = WaypointType | RouteType | TrackType

    };

    enum Attribute
    {
      NameAttr = 0,
      EleAttr,
      SymAttr,
      NumAttr,
      CmtAttr,
      DscAttr,
      SrcAttr,
      URLAttr,
      URLNameAttr,
      TimeAttr
    };

  private:
    QgsGpsData *mData = nullptr;

    //! Fields
    QgsFields mAttributeFields;
    //! map from field index to attribute
    QVector<int> mIndexToAttr;

    QString mFileName;

    DataType mFeatureType = WaypointType;

    static const QStringList sAttributeNames;
    static const QList<QMetaType::Type> sAttributeTypes;
    static const QList<DataType> sAttributedUsedForLayerType;

    bool mValid = false;

    friend class QgsGPXFeatureSource;
};

class QgsGpxProviderMetadata final : public QgsProviderMetadata
{
    Q_OBJECT
  public:
    QgsGpxProviderMetadata();
    QIcon icon() const override;
    QgsDataProvider *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, Qgis::DataProviderReadFlags flags = Qgis::DataProviderReadFlags() ) override;
    QgsProviderMetadata::ProviderCapabilities providerCapabilities() const override;
    QString encodeUri( const QVariantMap &parts ) const override;
    QVariantMap decodeUri( const QString &uri ) const override;
    QString absoluteToRelativeUri( const QString &uri, const QgsReadWriteContext &context ) const override;
    QString relativeToAbsoluteUri( const QString &uri, const QgsReadWriteContext &context ) const override;
    QList<Qgis::LayerType> supportedLayerTypes() const override;
};

#endif
