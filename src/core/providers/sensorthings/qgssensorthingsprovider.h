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

#include "qgsvectordataprovider.h"
#include "qgssensorthingsshareddata.h"
#include "qgsprovidermetadata.h"

#define SIP_NO_FILE
///@cond PRIVATE

/**
 * \brief A vector data provider reading features from an OGC SensorThings data source.
 *
 * \since QGIS 3.36
 */
class QgsSensorThingsProvider : public QgsVectorDataProvider
{
    Q_OBJECT

  public:

    static const inline QString SENSORTHINGS_PROVIDER_KEY = QStringLiteral( "sensorthings" );
    static const inline QString SENSORTHINGS_PROVIDER_DESCRIPTION = QStringLiteral( "OGC SensorThings API data provider" );

    QgsSensorThingsProvider( const QString &uri, const QgsDataProvider::ProviderOptions &providerOptions, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() );

    QgsAbstractFeatureSource *featureSource() const override;
    QString storageType() const override;
    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request = QgsFeatureRequest() ) const override;
    Qgis::WkbType wkbType() const override;
    long long featureCount() const override;
    QgsFields fields() const override;
    QgsLayerMetadata layerMetadata() const override;
    QString htmlMetadata() const override;

    QgsVectorDataProvider::Capabilities capabilities() const override;

    QgsCoordinateReferenceSystem crs() const override;
    void setDataSourceUri( const QString &uri ) override;
    QgsRectangle extent() const override;
    bool isValid() const override { return mValid; }

    QString name() const override;
    QString description() const override;

    static QString providerKey();

    void handlePostCloneOperations( QgsVectorDataProvider *source ) override;

  private:
    bool mValid = false;
    std::shared_ptr<QgsSensorThingsSharedData> mSharedData;

    QgsLayerMetadata mLayerMetadata;

    void reloadProviderData() override;
};

class QgsSensorThingsProviderMetadata: public QgsProviderMetadata
{
    Q_OBJECT

  public:
    QgsSensorThingsProviderMetadata();
    QIcon icon() const override;
    QList<QgsDataItemProvider *> dataItemProviders() const override;
    QVariantMap decodeUri( const QString &uri ) const override;
    QString encodeUri( const QVariantMap &parts ) const override;
    QgsSensorThingsProvider *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() ) override;
    QList< Qgis::LayerType > supportedLayerTypes() const override;

    // handling of stored connections

    QMap<QString, QgsAbstractProviderConnection *> connections( bool cached ) override;
    QgsAbstractProviderConnection *createConnection( const QString &name ) override;
    void deleteConnection( const QString &name ) override;
    void saveConnection( const QgsAbstractProviderConnection *connection, const QString &name ) override;

};

///@endcond PRIVATE
#endif // QGSSENSORTHINGSPROVIDER_H
