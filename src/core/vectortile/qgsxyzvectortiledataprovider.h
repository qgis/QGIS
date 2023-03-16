/***************************************************************************
  qgsxyzvectortiledataprovider.h
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

#ifndef QGSXYZVECTORTILEDATAPROVIDER_H
#define QGSXYZVECTORTILEDATAPROVIDER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsvectortiledataprovider.h"

#define SIP_NO_FILE

///@cond PRIVATE
class CORE_EXPORT QgsXyzVectorTileDataProvider : public QgsVectorTileDataProvider
{
    Q_OBJECT

  public:
    QgsXyzVectorTileDataProvider( const QString &uri,
                                  const QgsDataProvider::ProviderOptions &providerOptions,
                                  QgsDataProvider::ReadFlags flags );

    QgsVectorTileDataProvider *clone() const override;
    QString sourcePath() const override;
    bool isValid() const override;
    QgsCoordinateReferenceSystem crs() const override;
    bool supportsAsync() const override;
    QByteArray readTile( const QgsTileMatrix &tileMatrix, const QgsTileXYZ &id, QgsFeedback *feedback = nullptr ) const override;
    QList<QgsVectorTileRawData> readTiles( const QgsTileMatrix &, const QVector<QgsTileXYZ> &tiles, QgsFeedback *feedback = nullptr ) const override;
    QNetworkRequest tileRequest( const QgsTileMatrix &tileMatrix, const QgsTileXYZ &id, Qgis::RendererUsage usage ) const override;

  protected:

    QString mAuthCfg;
    QgsHttpHeaders mHeaders;

  private:

    //! Returns raw tile data for a single tile, doing a HTTP request. Block the caller until tile data are downloaded.
    static QByteArray loadFromNetwork( const QgsTileXYZ &id,
                                       const QgsTileMatrix &tileMatrix,
                                       const QString &requestUrl,
                                       const QString &authid,
                                       const QgsHttpHeaders &headers,
                                       QgsFeedback *feedback = nullptr );

};

///@endcond

#endif // QGSXYZVECTORTILEDATAPROVIDER_H
