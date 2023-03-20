/***************************************************************************
  qgsvtpkvectortiledataprovider.h
  --------------------------------------
  Date                 : March 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVTPKVECTORTILEDATAPROVIDER_H
#define QGSVTPKVECTORTILEDATAPROVIDER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsvectortiledataprovider.h"
#include "qgsprovidermetadata.h"

#define SIP_NO_FILE

///@cond PRIVATE

class QgsVtpkTiles;

class CORE_EXPORT QgsVtpkVectorTileDataProvider : public QgsVectorTileDataProvider
{
    Q_OBJECT

  public:
    QgsVtpkVectorTileDataProvider( const QString &uri,
                                   const QgsDataProvider::ProviderOptions &providerOptions,
                                   QgsDataProvider::ReadFlags flags );

    QString name() const override;
    QString description() const override;
    QgsVectorTileDataProvider *clone() const override;
    QString sourcePath() const override;
    bool isValid() const override;
    QgsCoordinateReferenceSystem crs() const override;
    QByteArray readTile( const QgsTileMatrix &tileMatrix, const QgsTileXYZ &id, QgsFeedback *feedback = nullptr ) const override;
    QList<QgsVectorTileRawData> readTiles( const QgsTileMatrix &, const QVector<QgsTileXYZ> &tiles, QgsFeedback *feedback = nullptr ) const override;

    static QString DATA_PROVIDER_KEY;
    static QString DATA_PROVIDER_DESCRIPTION;

  private:

    //! Returns raw tile data for a single tile loaded from VTPK file
    static QByteArray loadFromVtpk( QgsVtpkTiles &vtpkTileReader, const QgsTileXYZ &id, QgsFeedback *feedback = nullptr );

};


class QgsVtpkVectorTileDataProviderMetadata : public QgsProviderMetadata
{
    Q_OBJECT
  public:
    QgsVtpkVectorTileDataProviderMetadata();
    QgsVtpkVectorTileDataProvider *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() ) override;
    QIcon icon() const override;
    ProviderCapabilities providerCapabilities() const override;
    QVariantMap decodeUri( const QString &uri ) const override;
    QString encodeUri( const QVariantMap &parts ) const override;
    QString absoluteToRelativeUri( const QString &uri, const QgsReadWriteContext &context ) const override;
    QString relativeToAbsoluteUri( const QString &uri, const QgsReadWriteContext &context ) const override;
    QList< Qgis::LayerType > supportedLayerTypes() const override;
};


///@endcond

#endif // QGSVTPKVECTORTILEDATAPROVIDER_H
