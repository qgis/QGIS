/***************************************************************************
  qgsmbtilesvectortiledataprovider.h
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

#ifndef QGSMBTILESVECTORTILEDATAPROVIDER_H
#define QGSMBTILESVECTORTILEDATAPROVIDER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsvectortiledataprovider.h"

class QgsMbTiles;

#define SIP_NO_FILE

///@cond PRIVATE

class CORE_EXPORT QgsMbTilesVectorTileDataProvider : public QgsVectorTileDataProvider
{
    Q_OBJECT

  public:
    QgsMbTilesVectorTileDataProvider( const QString &uri,
                                      const QgsDataProvider::ProviderOptions &providerOptions,
                                      QgsDataProvider::ReadFlags flags );

    QgsVectorTileDataProvider *clone() const override;
    QString sourcePath() const override;
    bool isValid() const override;
    QgsCoordinateReferenceSystem crs() const override;
    QByteArray readTile( const QgsTileMatrix &tileMatrix, const QgsTileXYZ &id, QgsFeedback *feedback = nullptr ) const override;
    QList<QgsVectorTileRawData> readTiles( const QgsTileMatrix &, const QVector<QgsTileXYZ> &tiles, QgsFeedback *feedback = nullptr ) const override;

  private:

    //! Returns raw tile data for a single tile loaded from MBTiles file
    static QByteArray loadFromMBTiles( QgsMbTiles &mbTileReader, const QgsTileXYZ &id, QgsFeedback *feedback = nullptr );

};

///@endcond

#endif // QGSMBTILESVECTORTILEDATAPROVIDER_H
