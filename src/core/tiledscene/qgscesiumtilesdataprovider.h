/***************************************************************************
                         qgscesiumtilesdataprovider.h
                         --------------------
    begin                : June 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ******************************************************************
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCESIUMTILESDATAPROVIDER_H
#define QGSCESIUMTILESDATAPROVIDER_H

#include "qgis_core.h"
#include "qgstiledscenedataprovider.h"
#include "qgis.h"
#include "qgsprovidermetadata.h"

#define SIP_NO_FILE

class QgsAbstractTiledSceneBoundingVolume;
class QgsCoordinateTransformContext;
class QgsCesiumTilesDataProviderSharedData;

///@cond PRIVATE

class CORE_EXPORT QgsCesiumTilesDataProvider final: public QgsTiledSceneDataProvider
{
    Q_OBJECT
  public:


    //! Constructor for QgsCesiumTilesDataProvider
    QgsCesiumTilesDataProvider( const QString &uri,
                                const QgsDataProvider::ProviderOptions &providerOptions,
                                QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() );
    QgsCesiumTilesDataProvider( const QgsCesiumTilesDataProvider &other );
    QgsCesiumTilesDataProvider &operator=( const QgsCesiumTilesDataProvider &other ) = delete;

    ~QgsCesiumTilesDataProvider() final;
    Qgis::DataProviderFlags flags() const override;
    Qgis::TiledSceneProviderCapabilities capabilities() const final;
    QgsCesiumTilesDataProvider *clone() const final;
    QgsCoordinateReferenceSystem crs() const final;
    QgsRectangle extent() const final;
    bool isValid() const final;
    QString name() const final;
    QString description() const final;
    QString htmlMetadata() const final;
    QgsLayerMetadata layerMetadata() const override;
    const QgsCoordinateReferenceSystem sceneCrs() const final;
    const QgsTiledSceneBoundingVolume &boundingVolume() const final;
    QgsTiledSceneIndex index() const final;
    QgsDoubleRange zRange() const final;

  private:

    bool init();

    bool mIsValid = false;

    QString mAuthCfg;
    QgsHttpHeaders mHeaders;

    std::shared_ptr<QgsCesiumTilesDataProviderSharedData> mShared;  //!< Mutable data shared between provider instances

};


class QgsCesiumTilesProviderMetadata : public QgsProviderMetadata
{
    Q_OBJECT

  public:
    QgsCesiumTilesProviderMetadata();
    QIcon icon() const override;
    QgsProviderMetadata::ProviderMetadataCapabilities capabilities() const override;
    QgsCesiumTilesDataProvider *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() ) override;
    QList< QgsProviderSublayerDetails > querySublayers( const QString &uri, Qgis::SublayerQueryFlags flags = Qgis::SublayerQueryFlags(), QgsFeedback *feedback = nullptr ) const override;
    int priorityForUri( const QString &uri ) const override;
    QList< Qgis::LayerType > validLayerTypesForUri( const QString &uri ) const override;
    QString encodeUri( const QVariantMap &parts ) const override;
    QVariantMap decodeUri( const QString &uri ) const override;
    QString filters( Qgis::FileFilterType type ) override;
    ProviderCapabilities providerCapabilities() const override;
    QList< Qgis::LayerType > supportedLayerTypes() const override;

};

///@endcond

#endif // QGSCESIUMTILESDATAPROVIDER_H

