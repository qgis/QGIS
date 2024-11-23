/***************************************************************************
                         qgsquantizedmeshdataprovider.h
                         --------------------
    begin                : June 2024
    copyright            : (C) 2024 by David Koňařík
    email                : dvdkon at konarici dot cz
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

#pragma once

#include "qgis.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransformcontext.h"
#include "qgshttpheaders.h"
#include "qgsprovidermetadata.h"
#include "qgstiledsceneboundingvolume.h"
#include "qgstiledscenedataprovider.h"
#include "qgstiledsceneindex.h"
#include "qgstiles.h"
#include <optional>
#include <qmap.h>
#include <qvector.h>

#define SIP_NO_FILE

///@cond PRIVATE

class CORE_EXPORT QgsQuantizedMeshMetadata
{
  public:

    /**
     * \warning Check \p error, object is incomplete if non-empty!
     */
    QgsQuantizedMeshMetadata( const QString &uri,
                              const QgsCoordinateTransformContext &transformContext,
                              QgsError &error );

    bool containsTile( QgsTileXYZ tile ) const;
    double geometricErrorAtZoom( int zoom ) const;

    QString mAuthCfg;
    QgsHttpHeaders mHeaders;

    QgsRectangle mExtent;
    QgsTiledSceneBoundingVolume mBoundingVolume;
    // Map of zoom level -> list of AABBs of available tiles (tile index ranges)
    QVector<QVector<QgsTileRange>> mAvailableTiles;
    QgsCoordinateReferenceSystem mCrs;
    QString mTileScheme;
    uint8_t mMinZoom;
    uint8_t mMaxZoom;
    std::vector<QString> mTileUrls;
    QgsTileMatrix mTileMatrix;

    // The Quantized Mesh TileJSON(-ish) metadata doesn't tell us, so choose something big enough for Earth
    static const QgsDoubleRange dummyZRange;
};

class CORE_EXPORT QgsQuantizedMeshIndex : public QgsAbstractTiledSceneIndex
{
  public:
    QgsQuantizedMeshIndex( const QgsQuantizedMeshMetadata &metadata,
                           const QgsCoordinateTransform &wgs84ToCrs )
      : mMetadata( metadata ), mWgs84ToCrs( wgs84ToCrs ) {}
    QgsTiledSceneTile rootTile() const override;
    long long parentTileId( long long id ) const override;
    QVector< long long > childTileIds( long long id ) const override;
    QgsTiledSceneTile getTile( long long id ) override;
    QVector< long long > getTiles( const QgsTiledSceneRequest &request ) override;
    Qgis::TileChildrenAvailability childAvailability( long long id ) const override;
    bool fetchHierarchy( long long id, QgsFeedback *feedback = nullptr ) override;

    // Tile ID coding scheme:
    // From MSb, 2 bits zero, 1 bit one, 5 bits zoom, 28 bits X, 28 bits Y
    static long long encodeTileId( QgsTileXYZ tile );
    static QgsTileXYZ decodeTileId( long long id );

    static constexpr long long ROOT_TILE_ID = std::numeric_limits<long long>::max();

  protected:
    QByteArray fetchContent( const QString &uri, QgsFeedback *feedback = nullptr ) override;

    QgsQuantizedMeshMetadata mMetadata;
    QgsCoordinateTransform mWgs84ToCrs;
};

class CORE_EXPORT QgsQuantizedMeshDataProvider: public QgsTiledSceneDataProvider
{
    Q_OBJECT
  public:
    QgsQuantizedMeshDataProvider( const QString &uri,
                                  const QgsDataProvider::ProviderOptions &providerOptions,
                                  Qgis::DataProviderReadFlags flags = Qgis::DataProviderReadFlags() );
    Qgis::TiledSceneProviderCapabilities capabilities() const override;
    QgsTiledSceneDataProvider *clone() const override;
    const QgsCoordinateReferenceSystem sceneCrs() const override;
    const QgsTiledSceneBoundingVolume &boundingVolume() const override;
    QgsTiledSceneIndex index() const override;
    QgsDoubleRange zRange() const override;
    QgsCoordinateReferenceSystem crs() const override;
    QgsRectangle extent() const override;
    bool isValid() const override;
    QString name() const override;
    QString description() const override;

    const QgsQuantizedMeshMetadata &quantizedMeshMetadata() const;

    static constexpr const char *providerName = "quantizedmesh";
    static constexpr const char *providerDescription = "Cesium Quantized Mesh tiles";

  private:
    QString uriFromIon( const QString &uri );

  private:
    QString mUri; // For clone()
    QgsDataProvider::ProviderOptions mProviderOptions; // For clone()
    bool mIsValid = false;
    std::optional<QgsQuantizedMeshMetadata> mMetadata; // Initialized in constructor
    std::optional<QgsTiledSceneIndex> mIndex;
};

class QgsQuantizedMeshProviderMetadata : public QgsProviderMetadata
{
    Q_OBJECT

  public:
    QgsQuantizedMeshProviderMetadata();
    QgsDataProvider *createProvider( const QString &uri,
                                     const QgsDataProvider::ProviderOptions &providerOptions,
                                     Qgis::DataProviderReadFlags flags = Qgis::DataProviderReadFlags() );
};

///@endcond
