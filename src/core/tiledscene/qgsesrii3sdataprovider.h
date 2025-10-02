/***************************************************************************
  qgsesrii3sdataprovider.h
  --------------------------------------
  Date                 : July 2025
  Copyright            : (C) 2025 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSESRII3SDATAPROVIDER_H
#define QGSESRII3SDATAPROVIDER_H

#include "qgis_core.h"
#include "qgstiledscenedataprovider.h"
#include "qgis.h"
#include "qgsprovidermetadata.h"

#define SIP_NO_FILE

class QgsEsriI3SDataProviderSharedData;

///@cond PRIVATE

/**
 * \ingroup core
 * Data provider implementation for Esri I3S
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsEsriI3SDataProvider final: public QgsTiledSceneDataProvider
{
    Q_OBJECT
  public:

    //! Constructor for QgsEsriI3SDataProvider
    QgsEsriI3SDataProvider( const QString &uri,
                            const QgsDataProvider::ProviderOptions &providerOptions,
                            Qgis::DataProviderReadFlags flags = Qgis::DataProviderReadFlags() );
    QgsEsriI3SDataProvider( const QgsEsriI3SDataProvider &other );
    QgsEsriI3SDataProvider &operator=( const QgsEsriI3SDataProvider &other ) = delete;

    ~QgsEsriI3SDataProvider() final;
    Qgis::DataProviderFlags flags() const override;
    Qgis::TiledSceneProviderCapabilities capabilities() const final;
    QgsEsriI3SDataProvider *clone() const final;
    QgsCoordinateReferenceSystem crs() const final;
    QgsRectangle extent() const final;
    bool isValid() const final;
    QString name() const final;
    QString description() const final;
    QString htmlMetadata() const final;
    const QgsCoordinateReferenceSystem sceneCrs() const final;
    const QgsTiledSceneBoundingVolume &boundingVolume() const final;
    QgsTiledSceneIndex index() const final;
    QgsDoubleRange zRange() const final;

  private:

    bool loadFromRestService( const QString &uri, json &layerJson, QString &i3sVersion );
    bool loadFromSlpk( const QString &uri, json &layerJson, QString &i3sVersion );
    bool checkI3SVersion( const QString &i3sVersion );

    bool mIsValid = false;

    std::shared_ptr<QgsEsriI3SDataProviderSharedData> mShared;  //!< Mutable data shared between provider instances

};

/**
 * \ingroup core
 * Data provider metadata implementation for Esri I3S
 * \since QGIS 4.0
 */
class QgsEsriI3SProviderMetadata : public QgsProviderMetadata
{
    Q_OBJECT

  public:
    QgsEsriI3SProviderMetadata();
    QIcon icon() const override;
    QgsProviderMetadata::ProviderMetadataCapabilities capabilities() const override;
    QgsEsriI3SDataProvider *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, Qgis::DataProviderReadFlags flags = Qgis::DataProviderReadFlags() ) override;
    QString filters( Qgis::FileFilterType type ) override;
    ProviderCapabilities providerCapabilities() const override;
    QList< Qgis::LayerType > supportedLayerTypes() const override;

};

///@endcond

#endif // QGSESRII3SDATAPROVIDER_H
