/***************************************************************************
                         qgstiledmeshnode.h
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

#ifndef QGSTILEDMESHNODE_H
#define QGSTILEDMESHNODE_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgsmatrix4x4.h"

class QgsAbstractTiledMeshNodeBoundingVolume;

/**
 * \ingroup core
 * \brief Represents an individual tile node from a tiled mesh data source.
 *
 * \since QGIS 3.34
 */
class CORE_EXPORT QgsTiledMeshNode
{
  public:

    /**
     * Constructor for an invalid node.
     *
     * \see isValid()
     */
    QgsTiledMeshNode();

    ~QgsTiledMeshNode();

    //! Copy constructor. Does not copy children or parent!
    QgsTiledMeshNode( const QgsTiledMeshNode &other );
    //! Assignment operator. Does not copy children or parent!
    QgsTiledMeshNode &operator=( const QgsTiledMeshNode &other );

    /**
     * Returns TRUE if the node is a valid node (i.e. not default constructed).
     */
    bool isValid() const { return mIsValid; }

    /**
     * Adds a \a child to this node.
     *
     * Ownership of \a child is transferred to this node, and the parent for \a child will
     * automatically be set to this node.
     */
    void addChild( QgsTiledMeshNode *child SIP_TRANSFER )
    {
      child->mParent = this;
      mChildren.append( child );
      mIsValid = true;
    }

    /**
     * Returns the parent of the node.
     */
    QgsTiledMeshNode *parentNode() const { return mParent; }

    /**
     * Returns the child nodes.
     */
    QList< QgsTiledMeshNode * > children() const { return mChildren; }

    /**
     * Returns the node's refinement process.
     *
     * Refinement determines the process by which a lower resolution parent tile's
     * content is handled when its higher resolution children are also included.
     *
     * \see setRefinementProcess()
     */
    Qgis::TileRefinementProcess refinementProcess() const { return mRefinementProcess; }

    /**
     * Sets the node's refinement \a process.
     *
     * Refinement determines the process by which a lower resolution parent tile's
     * content is handled when its higher resolution children are also included.
     *
     * \see refinementProcess()
     */
    void setRefinementProcess( Qgis::TileRefinementProcess process );

    /**
     * Sets the bounding \a volume for the node.
     *
     * Ownership of \a volume is transferred to the node.
     *
     * \see boundingVolume()
     */
    void setBoundingVolume( QgsAbstractTiledMeshNodeBoundingVolume *volume SIP_TRANSFER );

    /**
     * Returns the bounding volume for the node.
     *
     * \see setBoundingVolume()
     */
    const QgsAbstractTiledMeshNodeBoundingVolume *boundingVolume() const;

    /**
     * Sets the node's \a transform.
     *
     * \note if the node has a boundingVolume(), the bounding volume's transform will also
     * be set to \a transform.
     *
     * \see transform()
     */
    void setTransform( const QgsMatrix4x4 &transform );

    /**
     * Returns the node's transform.
     *
     * This represents the transformation which must be applied to all geometries from the tile
     * in order to transform them to the dataset's coordinate reference system.
     *
     * \see transform()
     */
    const QgsMatrix4x4 &transform() const { return mTransform; }

    /**
     * Returns the content URI for the node.
     *
     * \see setContentUri()
     */
    QString contentUri() const;

    /**
     * Sets the content \a uri for the node.
     *
     * \see contentUri()
     */
    void setContentUri( const QString &uri );

    /**
     * Returns the node's geometric error, which is the error, in mesh CRS units, of the tile's
     * simplified representation of its source geometry.
     *
     * \see setGeometricError()
     */
    double geometricError() const { return mGeometricError; }

    /**
     * Sets the node's geometric \a error, which is the error, in mesh CRS units, of the tile's
     * simplified representation of its source geometry.
     *
     * \see geometricError()
     */
    void setGeometricError( double error );

  private:
    bool mIsValid = false;
    QgsTiledMeshNode *mParent = nullptr;
    QList< QgsTiledMeshNode * > mChildren;
    Qgis::TileRefinementProcess mRefinementProcess = Qgis::TileRefinementProcess::Replacement;
    std::unique_ptr< QgsAbstractTiledMeshNodeBoundingVolume > mBoundingVolume;
    QgsMatrix4x4 mTransform;
    QString mContentUri;
    double mGeometricError = 0;

};

#endif // QGSTILEDMESHNODE_H
