/***************************************************************************
      qgssensorthingsprovider.h
      ----------------
    begin                : November 2023
    copyright            : (C) 2013 Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSENSORTHINGSPROVIDER_H
#define QGSSENSORTHINGSPROVIDER_H

#include "qgsprovidermetadata.h"
#include "qgssensorthingsshareddata.h"
#include "qgsvectordataprovider.h"

#define SIP_NO_FILE
///@cond PRIVATE

/**
 * \brief A vector data provider reading features from an OGC SensorThings data source.
 *
 * \since QGIS 3.36
 */
class CORE_EXPORT QgsSensorThingsProvider final : public QgsVectorDataProvider
{
    Q_OBJECT

  public:

    static const inline QString SENSORTHINGS_PROVIDER_KEY = u"sensorthings"_s;
    static const inline QString SENSORTHINGS_PROVIDER_DESCRIPTION = u"OGC SensorThings API data provider"_s;

    QgsSensorThingsProvider( const QString &uri, const QgsDataProvider::ProviderOptions &providerOptions, Qgis::DataProviderReadFlags flags = Qgis::DataProviderReadFlags() );

    QgsAbstractFeatureSource *featureSource() const final;
    QString storageType() const final;
    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request = QgsFeatureRequest() ) const final;
    Qgis::WkbType wkbType() const final;
    long long featureCount() const final;
    QgsFields fields() const final;
    QgsLayerMetadata layerMetadata() const final;
    QString htmlMetadata() const final;

    Qgis::DataProviderFlags flags() const final;
    Qgis::VectorProviderCapabilities capabilities() const final;
    bool supportsSubsetString() const final;
    QString subsetStringDialect() const override;
    QString subsetStringHelpUrl() const override;
    QString subsetString() const final;
    bool setSubsetString( const QString &subset, bool updateFeatureCount = true ) final;
    QgsCoordinateReferenceSystem crs() const final;
    void setDataSourceUri( const QString &uri ) final;
    QgsRectangle extent() const final;
    bool isValid() const final { return mValid; }

    QString name() const final;
    QString description() const final;
    bool renderInPreview( const QgsDataProvider::PreviewContext &context ) final;

    static QString providerKey();

    void handlePostCloneOperations( QgsVectorDataProvider *source ) final;

  private:
    bool mValid = false;
    std::shared_ptr<QgsSensorThingsSharedData> mSharedData;

    QgsLayerMetadata mLayerMetadata;

    void reloadProviderData() final;
};

class QgsSensorThingsProviderMetadata final: public QgsProviderMetadata
{
    Q_OBJECT

  public:
    QgsSensorThingsProviderMetadata();
    QIcon icon() const final;
    QList<QgsDataItemProvider *> dataItemProviders() const final;
    QVariantMap decodeUri( const QString &uri ) const final;
    QString encodeUri( const QVariantMap &parts ) const final;
    QgsSensorThingsProvider *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, Qgis::DataProviderReadFlags flags = Qgis::DataProviderReadFlags() ) final;
    QList< Qgis::LayerType > supportedLayerTypes() const final;

    // handling of stored connections

    QMap<QString, QgsAbstractProviderConnection *> connections( bool cached ) final;

    using QgsProviderMetadata::createConnection;
    QgsAbstractProviderConnection *createConnection( const QString &name ) final;
    void deleteConnection( const QString &name ) final;
    void saveConnection( const QgsAbstractProviderConnection *connection, const QString &name ) final;

};

///@endcond PRIVATE
#endif // QGSSENSORTHINGSPROVIDER_H
