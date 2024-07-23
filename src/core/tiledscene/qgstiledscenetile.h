/***************************************************************************
                         qgstiledscenetile.h
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

#ifndef QGSTILEDSCENETILE_H
#define QGSTILEDSCENETILE_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgsmatrix4x4.h"
#include "qgstiledsceneboundingvolume.h"

#include <QUrl>

/**
 * \ingroup core
 * \brief Represents an individual tile from a tiled scene data source.
 *
 * \since QGIS 3.34
 */
class CORE_EXPORT QgsTiledSceneTile
{
  public:

    /**
     * Constructor for an invalid tile.
     *
     * \see isValid()
     */
    QgsTiledSceneTile();

    /**
     * Constructor for an valid tile.
     *
     * \see isValid()
     */
    explicit QgsTiledSceneTile( long long id );

    ~QgsTiledSceneTile();

    QgsTiledSceneTile( const QgsTiledSceneTile &other );
    QgsTiledSceneTile &operator=( const QgsTiledSceneTile &other );

    /**
     * Returns TRUE if the tile is a valid tile (i.e. not default constructed).
     */
    bool isValid() const { return mId >= 0; }

    /**
     * Returns the tile's unique ID.
     */
    long long id() const { return mId; }

    /**
     * Returns the tile's refinement process.
     *
     * Refinement determines the process by which a lower resolution parent tile's
     * content is handled when its higher resolution children are also included.
     *
     * \see setRefinementProcess()
     */
    Qgis::TileRefinementProcess refinementProcess() const { return mRefinementProcess; }

    /**
     * Sets the tile's refinement \a process.
     *
     * Refinement determines the process by which a lower resolution parent tile's
     * content is handled when its higher resolution children are also included.
     *
     * \see refinementProcess()
     */
    void setRefinementProcess( Qgis::TileRefinementProcess process );

    /**
     * Sets the bounding \a volume for the tile.
     *
     * \see boundingVolume()
     */
    void setBoundingVolume( const QgsTiledSceneBoundingVolume &volume );

    /**
     * Returns the bounding volume for the tile.
     *
     * \see setBoundingVolume()
     */
    const QgsTiledSceneBoundingVolume &boundingVolume() const;

    /**
     * Sets the tile's \a transform.
     *
     * \see transform()
     */
    void setTransform( const QgsMatrix4x4 &transform );

    /**
     * Returns the tile's transform. May be NULLPTR if no transform is associated with the tile.
     *
     * This represents the transformation which must be applied to all geometries from the tile
     * in order to transform them to the dataset's coordinate reference system.
     *
     * \see transform()
     */
    const QgsMatrix4x4 *transform() const { return mTransform.get(); }

    /**
     * Returns the resources attached to the tile.
     *
     * \see setResources()
     */
    QVariantMap resources() const;

    /**
     * Sets the \a resources \a attached to the tile.
     *
     * \see resources()
     */
    void setResources( const QVariantMap &resources );

    /**
     * Returns the tile's geometric error, which is the error, in meters, of
     * the tile's simplified representation of its source geometry.
     *
     * \see setGeometricError()
     */
    double geometricError() const { return mGeometricError; }

    /**
     * Sets the tile's geometric \a error, which is the error, in meters, of
     * the tile's simplified representation of its source geometry.
     *
     * \see geometricError()
     */
    void setGeometricError( double error );

    /**
     * Returns the tile's base URL. If this tile's resources are relative paths, they would
     * get resolved against this URL.
     *
     * \see setBaseUrl()
     */
    QUrl baseUrl() const;

    /**
     * Sets the tile's base URL. If this tile's resources are relative paths, they would
     * get resolved against this URL.
     *
     * \see baseUrl()
     */
    void setBaseUrl( const QUrl &baseUrl );

    /**
     * Returns additional metadata attached to the tile.
     *
     * \see setMetadata()
     */
    QVariantMap metadata() const;

    /**
     * Sets additional \a metadata attached to the tile.
     *
     * \see metadata()
     */
    void setMetadata( const QVariantMap &metadata );

  private:
    long long mId = -1;
    Qgis::TileRefinementProcess mRefinementProcess = Qgis::TileRefinementProcess::Replacement;
    QgsTiledSceneBoundingVolume mBoundingVolume;
    std::unique_ptr< QgsMatrix4x4 > mTransform;
    QVariantMap mResources;
    double mGeometricError = 0;
    QUrl mBaseUrl;
    QVariantMap mMetadata;

};

#endif // QGSTILEDSCENETILE_H
