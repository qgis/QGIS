/***************************************************************************
                         qgstiledmeshrequest.h
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

#ifndef QGSTILEDMESHREQUEST_H
#define QGSTILEDMESHREQUEST_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgsorientedbox3d.h"

class QgsFeedback;

/**
 * \ingroup core
 *
 * \brief Tiled mesh data request.
 *
 * \since QGIS 3.34
 */
class CORE_EXPORT QgsTiledMeshRequest
{
  public:

    QgsTiledMeshRequest();

    /**
     * Sets \a flags that affect how tiles will be fetched.
     *
     * \see flags()
     */
    void setFlags( Qgis::TiledMeshRequestFlags flags ) { mFlags = flags; }

    /**
     * Returns the flags which affect how tiles are fetched.
     *
     * \see setFlags()
     */
    Qgis::TiledMeshRequestFlags flags() const { return mFlags; }

    /**
    * Returns the box from which data will be taken, in the layer's CRS.
    *
    * If the returned box is null, then no filter box is set.
    *
    * \see setFilterBox()
    */
    QgsOrientedBox3D filterBox() const { return mFilterBox; }

    /**
     * Sets the \a box from which data will be taken, in the layer's CRS.
     *
     * An null \a box removes the filter.
     *
     * \see filterBox()
     */
    void setFilterBox( const QgsOrientedBox3D &box ) { mFilterBox = box; }

    /**
     * Returns the required geometric error threshold for the returned tiles, in
     * mesh CRS units.
     *
     * If the error is 0 then no geometric error filtering will be applied.
     *
     * \see setRequiredGeometricError()
     */
    double requiredGeometricError() const { return mRequiredGeometricError; }

    /**
     * Sets the required geometric \a error threshold for the returned tiles, in
     * mesh CRS units.
     *
     * If the \a error is 0 then no geometric error filtering will be applied.
     *
     * \see requiredGeometricError()
     */
    void setRequiredGeometricError( double error ) { mRequiredGeometricError = error; }

    /**
     * Attach a \a feedback object that can be queried regularly by the request to check
     * if it should be canceled.
     *
     * Ownership of \a feedback is NOT transferred, and the caller must take care that it exists
     * for the lifetime of the request.
     *
     * \see feedback()
     */
    void setFeedback( QgsFeedback *feedback );

    /**
     * Returns the feedback object that can be queried regularly by the request to check
     * if it should be canceled, if set.
     *
     * \see setFeedback()
     */
    QgsFeedback *feedback() const;

    /**
     * Returns the parent tile ID, if filtering is limited to children of a specific tile.
     *
     * \see setParentTileId()
     */
    QString parentTileId() const { return mParentTileId; }

    /**
     * Sets the parent tile \a id, if filtering is to be limited to children of a specific tile.
     *
     * \see parentTileId()
     */
    void setParentTileId( const QString &id ) { mParentTileId = id; }

  private:

    Qgis::TiledMeshRequestFlags mFlags;
    QgsOrientedBox3D mFilterBox;
    QgsFeedback *mFeedback = nullptr;
    double mRequiredGeometricError = 0;
    QString mParentTileId;
};


#endif // QGSTILEDMESHREQUEST_H
