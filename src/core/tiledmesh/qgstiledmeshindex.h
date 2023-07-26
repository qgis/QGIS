/***************************************************************************
                         qgstiledmeshindex.h
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

#ifndef QGSTILEDMESHINDEX_H
#define QGSTILEDMESHINDEX_H

#include "qgis_core.h"
#include "qgis.h"

#include <QCache>
#include <QReadWriteLock>

class QgsTiledMeshNode;
class QgsFeedback;
class QgsTiledMeshRequest;

#ifndef SIP_RUN

/**
 * \ingroup core
 * \brief An abstract base class for tiled mesh data provider indices.
 *
 * \since QGIS 3.34
 */
class CORE_EXPORT QgsAbstractTiledMeshIndex
{
  public:

    QgsAbstractTiledMeshIndex();
    virtual ~QgsAbstractTiledMeshIndex();

    QgsAbstractTiledMeshIndex( const QgsAbstractTiledMeshIndex &other ) = delete;
    QgsAbstractTiledMeshIndex &operator=( const QgsAbstractTiledMeshIndex &other ) = delete;

    /**
     * Returns the root node for the index.
     */
    virtual const QgsTiledMeshNode *rootNode() const = 0;

    /**
     * Returns a node tree containing all nodes matching the given \a request.
     *
     * Caller takes ownership of the returned node.
     *
     * May return NULLPTR if no data satisfies the request.
     */
    virtual QgsTiledMeshNode *getNodes( const QgsTiledMeshRequest &request ) = 0;

    /**
     * Retrieves index content for the specified \a uri.
     *
     * The content is cached within the index, so multiple calls are potentially cost-free.
     *
     * The optional \a feedback argument can be used the cancel the request early.
     */
    QByteArray retrieveContent( const QString &uri, QgsFeedback *feedback = nullptr );

    mutable QReadWriteLock mLock;

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

    QCache< QString, QByteArray > mContentCache;

};
#endif

/**
 * \ingroup core
 * \brief An index for tiled mesh data providers.
 *
 * This is a shallow copy, implicitly shared container for an underlying QgsAbstractTiledMeshIndex
 * implementation.
 *
 * The class is thread safe and can be used safely across multiple threads or transferred between
 * threads.
 *
 * \since QGIS 3.34
 */
class CORE_EXPORT QgsTiledMeshIndex
{
  public:

    /**
     * Constructor for QgsTiledMeshIndex.
     *
     * The specified \a index implementation will be transferred to the index.
     *
     * \note Not available in Python bindings.
     */
    explicit QgsTiledMeshIndex( QgsAbstractTiledMeshIndex *index SIP_TRANSFER = nullptr ) SIP_SKIP;

    ~QgsTiledMeshIndex();

    //! Copy constructor
    QgsTiledMeshIndex( const QgsTiledMeshIndex &other );
    QgsTiledMeshIndex &operator=( const QgsTiledMeshIndex &other );

    /**
     * Returns TRUE if the index is valid.
     */
    bool isValid() const;

    /**
     * Returns the root node for the index.
     */
    QgsTiledMeshNode rootNode() const;

    /**
     * Returns a node tree containing all nodes matching the given \a request.
     *
     * Caller takes ownership of the returned node.
     *
     * May return NULLPTR if no data satisfies the request.
     */
    QgsTiledMeshNode *getNodes( const QgsTiledMeshRequest &request ) SIP_FACTORY;

    /**
     * Retrieves index content for the specified \a uri.
     *
     * The content is cached within the index, so multiple calls are potentially cost-free.
     *
     * The optional \a feedback argument can be used the cancel the request early.
     */
    QByteArray retrieveContent( const QString &uri, QgsFeedback *feedback = nullptr );

  private:

    std::shared_ptr<QgsAbstractTiledMeshIndex> mIndex;
};


#endif // QGSTILEDMESHINDEX_H
