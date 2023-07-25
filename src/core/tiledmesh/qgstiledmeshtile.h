/***************************************************************************
                         qgstiledmeshtile.h
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

#ifndef QGSTILEDMESHTILE_H
#define QGSTILEDMESHTILE_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgsmatrix4x4.h"

class QgsAbstractTiledMeshNodeBoundingVolume;

/**
 * \ingroup core
 * \brief Represents an individual tile from a tiled mesh data source.
 *
 * \since QGIS 3.34
 */
class CORE_EXPORT QgsTiledMeshTile
{
  public:

    /**
     * Constructor for an invalid tile.
     *
     * \see isValid()
     */
    QgsTiledMeshTile();

    /**
     * Constructor for an valid tile.
     *
     * \see isValid()
     */
    explicit QgsTiledMeshTile( const QString &id );

    ~QgsTiledMeshTile();

    //! Copy constructor
    QgsTiledMeshTile( const QgsTiledMeshTile &other );
    //! Assignment operator
    QgsTiledMeshTile &operator=( const QgsTiledMeshTile &other );

    /**
     * Returns TRUE if the tile is a valid tile (i.e. not default constructed).
     */
    bool isValid() const { return !mId.isEmpty(); }

    /**
     * Returns the tile's unique ID.
     */
    QString id() const { return mId; }

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
     * Ownership of \a volume is transferred to the tile.
     *
     * \see boundingVolume()
     */
    void setBoundingVolume( QgsAbstractTiledMeshNodeBoundingVolume *volume SIP_TRANSFER );

    /**
     * Returns the bounding volume for the tile.
     *
     * \see setBoundingVolume()
     */
    const QgsAbstractTiledMeshNodeBoundingVolume *boundingVolume() const;

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
     * Returns the tile's geometric error, which is the error, in mesh CRS units, of the tile's
     * simplified representation of its source geometry.
     *
     * \see setGeometricError()
     */
    double geometricError() const { return mGeometricError; }

    /**
     * Sets the tile's geometric \a error, which is the error, in mesh CRS units, of the tile's
     * simplified representation of its source geometry.
     *
     * \see geometricError()
     */
    void setGeometricError( double error );

  private:
    QString mId;
    Qgis::TileRefinementProcess mRefinementProcess = Qgis::TileRefinementProcess::Replacement;
    std::unique_ptr< QgsAbstractTiledMeshNodeBoundingVolume > mBoundingVolume;
    std::unique_ptr< QgsMatrix4x4 > mTransform;
    QVariantMap mResources;
    double mGeometricError = 0;

};

#endif // QGSTILEDMESHTILE_H
