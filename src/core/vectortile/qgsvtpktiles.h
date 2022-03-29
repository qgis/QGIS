/***************************************************************************
  qgsvtpktiles.h
  --------------------------------------
  Date                 : March 2022
  Copyright            : (C) 2022 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVTPKTILES_H
#define QGSVTPKTILES_H

#include "qgis_core.h"

#include "sqlite3.h"
#include "qgsvectortilematrixset.h"

#include <QVariantMap>

class QImage;
class QgsRectangle;
class QgsVectorTileMatrixSet;
class QgsLayerMetadata;

/**
 * \ingroup core
 * \brief Utility class for reading and writing ESRI VTPK files.
 *
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsVtpkTiles
{
  public:
    //! Constructs VTPK reader (but it does not open the file yet)
    explicit QgsVtpkTiles( const QString &filename );

#ifndef SIP_RUN
    //! QgsVtpkTiles cannot be copied
    QgsVtpkTiles( const QgsVtpkTiles &other ) = delete;
    //! QgsVtpkTiles cannot be copied
    QgsVtpkTiles &operator=( const QgsVtpkTiles &other ) = delete;
#endif
    ~QgsVtpkTiles();

    //! Tries to open the file, returns true on success
    bool open();

    //! Returns whether the VTPK file is currently opened
    bool isOpen() const;

    /**
     * Returns the VTPK metadata.
     *
     * This method returns the contents of the "root.json" file.
     */
    QVariantMap metadata() const;

    /**
     * Returns the VTPK style definition.
     */
    QVariantMap styleDefinition() const;

    /**
     * Returns the VTPK sprites definitions.
     */
    QVariantMap spriteDefinition() const;

    /**
     * Returns the VTPK sprite image, if it exists.
     */
    QImage spriteImage() const;

    /**
     * Reads layer metadata from the VTPK file.
     */
    QgsLayerMetadata layerMetadata() const;

    /**
     * Returns the vector tile matrix set representing the tiles.
     */
    QgsVectorTileMatrixSet matrixSet() const;

    /**
     * Returns the coordinate reference system of the tiles.
     */
    QgsCoordinateReferenceSystem crs() const;

    /**
     * Returns bounding box from metadata, given in the tiles crs().
     */
    QgsRectangle extent( const QgsCoordinateTransformContext &context ) const;

    /**
     * Returns the raw tile data for the matching tile.
     */
    QByteArray tileData( int z, int x, int y );

  private:

#ifdef SIP_RUN
    //! QgsVtpkTiles cannot be copied
    QgsVtpkTiles( const QgsVtpkTiles &other );
#endif

    QString mFilename;
    struct zip *mZip = nullptr;
    mutable QVariantMap mMetadata;
    mutable QgsVectorTileMatrixSet mMatrixSet;
    mutable int mPacketSize = -1;
};


#endif // QGSVTPKTILES_H
