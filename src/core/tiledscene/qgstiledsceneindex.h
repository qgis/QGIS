/***************************************************************************
                         qgstiledsceneindex.h
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

#ifndef QGSTILEDSCENEINDEX_H
#define QGSTILEDSCENEINDEX_H

#include "qgis_core.h"
#include "qgis.h"

#include <QCache>
#include <QReadWriteLock>

class QgsTiledSceneTile;
class QgsFeedback;
class QgsTiledSceneRequest;

#ifndef SIP_RUN

/**
 * \ingroup core
 * \brief An abstract base class for tiled scene data provider indices.
 *
 * \warning Subclasses must take care to ensure that their operation is thread safe, as the same
 * index object will potentially be used by multiple threads concurrently.
 *
 * \since QGIS 3.34
 */
class CORE_EXPORT QgsAbstractTiledSceneIndex
{
  public:

    QgsAbstractTiledSceneIndex();
    virtual ~QgsAbstractTiledSceneIndex();

    //! QgsAbstractTiledSceneIndex cannot be copied
    QgsAbstractTiledSceneIndex( const QgsAbstractTiledSceneIndex &other ) = delete;
    //! QgsAbstractTiledSceneIndex cannot be copied
    QgsAbstractTiledSceneIndex &operator=( const QgsAbstractTiledSceneIndex &other ) = delete;

    /**
     * Returns the root tile for the index.
     */
    virtual QgsTiledSceneTile rootTile() const = 0;

    /**
     * Returns the tile ID of the parent tile of the tile with matching \a id, or an empty string
     * if the tile has no parent.
     *
     * \see childTileIds()
     */
    virtual QString parentTileId( const QString &id ) const = 0;

    /**
     * Returns a list of the tile IDs of any children for the tile with matching \a id.
     *
     * \see parentTileId()
     */
    virtual QStringList childTileIds( const QString &id ) const = 0;

    /**
     * Returns the tile with matching \a id, or an invalid tile if the matching tile is not available.
     */
    virtual QgsTiledSceneTile getTile( const QString &id ) = 0;

    /**
     * Returns the tile IDs which match the given \a request.
     *
     * May return an empty list if no data satisfies the request.
     */
    virtual QStringList getTiles( const QgsTiledSceneRequest &request ) = 0;

    /**
     * Retrieves index content for the specified \a uri.
     *
     * The content is cached within the index, so multiple calls are potentially cost-free.
     *
     * The optional \a feedback argument can be used the cancel the request early.
     */
    QByteArray retrieveContent( const QString &uri, QgsFeedback *feedback = nullptr );

    /**
     * Returns the availability for a tile's children.
     *
     * \see fetchHierarchy()
     */
    virtual Qgis::TileChildrenAvailability childAvailability( const QString &id ) const = 0;

    /**
     * Populates the tile with the given \a id by fetching any sub datasets attached to the tile.
     *
     * Blocks while the child fetching is in progress.
     *
     * Returns TRUE if the population was successful.
     *
     * \see childAvailability()
     */
    virtual bool fetchHierarchy( const QString &id, QgsFeedback *feedback = nullptr ) = 0;

  protected:

    /**
     * Fetches index content for the specified \a uri.
     *
     * This must be implemented in subclasses to retrieve the corresponding binary content,
     * including performing any necessary network requests.
     *
     * The optional \a feedback argument can be used the cancel the request early.
     */
    virtual QByteArray fetchContent( const QString &uri, QgsFeedback *feedback = nullptr ) = 0;

  private:

    mutable QReadWriteLock mCacheLock;
    QCache< QString, QByteArray > mContentCache;

};
#endif

/**
 * \ingroup core
 * \brief An index for tiled scene data providers.
 *
 * This is a shallow copy, implicitly shared container for an underlying QgsAbstractTiledSceneIndex
 * implementation.
 *
 * The class is thread safe and can be used safely across multiple threads or transferred between
 * threads.
 *
 * \since QGIS 3.34
 */
class CORE_EXPORT QgsTiledSceneIndex
{
  public:

    /**
     * Constructor for QgsTiledSceneIndex.
     *
     * The specified \a index implementation will be transferred to the index.
     *
     * \note Not available in Python bindings.
     */
    explicit QgsTiledSceneIndex( QgsAbstractTiledSceneIndex *index SIP_TRANSFER = nullptr ) SIP_SKIP;

    ~QgsTiledSceneIndex();

    //! Copy constructor
    QgsTiledSceneIndex( const QgsTiledSceneIndex &other );
    QgsTiledSceneIndex &operator=( const QgsTiledSceneIndex &other );

    /**
     * Returns TRUE if the index is valid.
     */
    bool isValid() const;

    /**
     * Returns the root tile for the index.
     */
    QgsTiledSceneTile rootTile() const;

    /**
     * Returns the tile with matching \a id, or an invalid tile if the matching tile is not available.
     */
    QgsTiledSceneTile getTile( const QString &id );

    /**
     * Returns the tile ID of the parent tile of the tile with matching \a id, or an empty string
     * if the tile has no parent.
     *
     * \see childTileIds()
     */
    QString parentTileId( const QString &id ) const;

    /**
     * Returns a list of the tile IDs of any children for the tile with matching \a id.
     *
     * \see parentTileId()
     */
    QStringList childTileIds( const QString &id ) const;

    /**
     * Returns the list of tile IDs which match the given \a request.
     *
     * May return an empty list if no data satisfies the request.
     */
    QStringList getTiles( const QgsTiledSceneRequest &request );

    /**
     * Returns the availability for a tile's children.
     *
     * \see fetchHierarchy()
     */
    Qgis::TileChildrenAvailability childAvailability( const QString &id ) const;

    /**
     * Populates the tile with the given \a id by fetching any sub datasets attached to the tile.
     *
     * Blocks while the child fetching is in progress.
     *
     * Returns TRUE if the population was successful.
     *
     * \see childAvailability()
     */
    bool fetchHierarchy( const QString &id, QgsFeedback *feedback = nullptr );

    /**
     * Retrieves index content for the specified \a uri.
     *
     * The content is cached within the index, so multiple calls are potentially cost-free.
     *
     * The optional \a feedback argument can be used the cancel the request early.
     */
    QByteArray retrieveContent( const QString &uri, QgsFeedback *feedback = nullptr );

  private:

    std::shared_ptr<QgsAbstractTiledSceneIndex> mIndex;
};


#endif // QGSTILEDSCENEINDEX_H
