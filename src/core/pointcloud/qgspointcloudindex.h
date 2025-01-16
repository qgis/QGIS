/***************************************************************************
                         qgspointcloudindex.h
                         --------------------
    begin                : October 2020
    copyright            : (C) 2020 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTCLOUDINDEX_H
#define QGSPOINTCLOUDINDEX_H

#include <QString>
#include <QHash>
#include <QStringList>
#include <QVector>
#include <QList>
#include <QMutex>
#include <QCache>
#include <QByteArray>

#include "qgis_core.h"
#include "qgspointcloudstatistics.h"
#include "qgsrectangle.h"
#include "qgsbox3d.h"
#include "qgis_sip.h"
#include "qgspointcloudblock.h"
#include "qgspointcloudattribute.h"
#include "qgspointcloudexpression.h"
#include "qgspointcloudrequest.h"


class QgsPointCloudAttributeCollection;
class QgsCoordinateReferenceSystem;
class QgsPointCloudBlockRequest;
class QgsPointCloudStatistics;
class QgsAbstractPointCloudIndex;

/**
 * \ingroup core
 *
 * \brief Represents a indexed point cloud node's position in octree
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsPointCloudNodeId
{
  public:
    //! Constructs invalid node
    QgsPointCloudNodeId();
    //! Constructs valid node
    QgsPointCloudNodeId( int _d, int _x, int _y, int _z );

    //! Returns whether node is valid
    bool isValid() const { return mD >= 0; }

    // TODO c++20 - replace with = default

    bool operator==( QgsPointCloudNodeId other ) const
    {
      return mD == other.d() && mX == other.x() && mY == other.y() && mZ == other.z();
    }

    /**
     * Returns the parent of the node
     * \since QGIS 3.20
     */
    QgsPointCloudNodeId parentNode() const;

    //! Creates node from string
    static QgsPointCloudNodeId fromString( const QString &str );

    //! Encode node to string
    QString toString() const;

    //! Returns d
    int d() const;

    //! Returns x
    int x() const;

    //! Returns y
    int y() const;

    //! Returns z
    int z() const;

  private:
    int mD = -1, mX = -1, mY = -1, mZ = -1;
};

Q_DECLARE_TYPEINFO( QgsPointCloudNodeId, Q_PRIMITIVE_TYPE );

//! Hash function for indexed nodes
CORE_EXPORT uint qHash( QgsPointCloudNodeId id );

#ifndef SIP_RUN

/**
 * \ingroup core
 *
 * \brief Container class for QgsPointCloudBlock cache keys
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.36
 */
class CORE_EXPORT QgsPointCloudCacheKey
{
  public:
    //! Ctor
    QgsPointCloudCacheKey( const QgsPointCloudNodeId &n, const QgsPointCloudRequest &request, const QgsPointCloudExpression &expression, const QString &uri );

    bool operator==( const QgsPointCloudCacheKey &other ) const;

    //! Returns the key's QgsPointCloudNodeId
    QgsPointCloudNodeId node() const { return mNode; }

    //! Returns the key's uri
    QString uri() const { return mUri; }

    //! Returns the key's QgsPointCloudRequest
    QgsPointCloudRequest request() const { return mRequest; }

    //! Returns the key's QgsPointCloudExpression
    QgsPointCloudExpression filterExpression() const { return mFilterExpression; }

  private:
    QgsPointCloudNodeId mNode;
    QString mUri;
    QgsPointCloudRequest mRequest;
    QgsPointCloudExpression mFilterExpression;
};

//! Hash function for QgsPointCloudCacheKey
uint qHash( const QgsPointCloudCacheKey &key );

#endif // !SIP_RUN

/**
 * \ingroup core
 *
 * \brief Keeps metadata for indexed point cloud node
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.42
 */
class CORE_EXPORT QgsPointCloudNode
{
  public:

    /**
     * Constructs new node object. Should only be called by QgsAbstractPointCloudIndex::getNode().
     * Bounds should always be computed by QgsPointCloudNode::bounds().
     */
    QgsPointCloudNode( const QgsPointCloudNodeId &id,
                       qint64 pointCount,
                       const QList<QgsPointCloudNodeId> &childIds,
                       float error,
                       const QgsBox3D &bounds )
      : mId( id )
      , mPointCount( pointCount )
      , mChildIds( childIds )
      , mError( error )
      , mBounds( bounds )
    {
    }
    //! Returns node's ID (unique in index)
    QgsPointCloudNodeId id() const { return mId; }
    //! Returns number of points contained in node data
    qint64 pointCount() const { return mPointCount; }
    //! Returns IDs of child nodes
    QList<QgsPointCloudNodeId> children() const { return mChildIds; }
    //! Returns node's error in map units (used to determine in whether the node has enough detail for the current view)
    float error() const;
    //! Returns node's bounding cube in CRS coords
    QgsBox3D bounds() const;

    //! Returns bounding box of specific node
    static QgsBox3D bounds( QgsBox3D rootBounds, QgsPointCloudNodeId id );

  private:
    // Specific node metadata:
    QgsPointCloudNodeId mId;
    qint64 mPointCount;
    QList<QgsPointCloudNodeId> mChildIds;
    float mError;
    QgsBox3D mBounds;
};


#ifndef SIP_RUN

/**
 * \ingroup core
 *
 * \brief Represents a indexed point clouds data in octree
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.42
 */
class CORE_EXPORT QgsAbstractPointCloudIndex
{
  public:
    //! Constructs index
    explicit QgsAbstractPointCloudIndex();
    virtual ~QgsAbstractPointCloudIndex();

    /**
     * Returns a clone of the current point cloud index object
     * \note It is the responsibility of the caller to handle the ownership and delete the object.
     * \since QGIS 3.26
     */
    virtual std::unique_ptr<QgsAbstractPointCloudIndex> clone() const = 0;

    //! Loads the index from the file
    virtual void load( const QString &fileName ) = 0;

    //! Returns whether index is loaded and valid
    virtual bool isValid() const = 0;

    /**
     * Returns the error that occurred during the loading of the index.
     * \since QGIS 3.26
     */
    QString error() const { return mError; }

    /**
     * Returns the access type of the data
     * If the access type is Remote, data will be fetched from an HTTP server either synchronously or asynchronously
     * If the access type is local, the data is stored locally as a file and will only be fetch synchronously ( blocking request with nodeData only )
     * \note Always make sure to check before trying to use asyncNodeData since it is not supported in the case of local access type
     */
    virtual Qgis::PointCloudAccessType accessType() const = 0;

    //! Returns the coordinate reference system of the point cloud index
    virtual QgsCoordinateReferenceSystem crs() const = 0;
    //! Returns the number of points in the point cloud
    virtual qint64 pointCount() const = 0;
    //! Returns the original metadata map
    virtual QVariantMap originalMetadata() const = 0;

    /**
     * Returns the object containing the statistics metadata extracted from the dataset
     * \since QGIS 3.42
     */
    virtual QgsPointCloudStatistics metadataStatistics() const;

    /**
     * Writes the statistics object \a stats into the backing file, if possible.
     * Returns true if the data was written successfully.
     * \since QGIS 3.26
     */
    virtual bool writeStatistics( QgsPointCloudStatistics &stats );

    //! Returns root node of the index
    QgsPointCloudNodeId root() const { return QgsPointCloudNodeId( 0, 0, 0, 0 ); }

    //! Returns whether the octree contain given node
    virtual bool hasNode( const QgsPointCloudNodeId &n ) const;

    //! Returns object for a given node
    virtual QgsPointCloudNode getNode( const QgsPointCloudNodeId &id ) const;

    //! Returns all attributes that are stored in the file
    QgsPointCloudAttributeCollection attributes() const;

    /**
     * Returns node data block.
     *
     * e.g. positions (needs to be scaled and offset applied to get coordinates) or
     * classification, intensity or custom attributes.
     *
     * May return nullptr in case the node is not present or any other problem with loading
     */
    virtual std::unique_ptr< QgsPointCloudBlock > nodeData( const QgsPointCloudNodeId &n, const QgsPointCloudRequest &request ) = 0;

    /**
     * Returns a handle responsible for loading a node data block
     *
     * e.g. positions (needs to be scaled and offset applied to get coordinates) or
     * classification, intensity or custom attributes
     *
     * It is caller responsibility to free the handle and the block issued by the handle if the loading succeeds.
     *
     * May return nullptr in case the node is not present or any other problem with loading
     */
    virtual QgsPointCloudBlockRequest *asyncNodeData( const QgsPointCloudNodeId &n, const QgsPointCloudRequest &request ) = 0;

    /**
     * Tries to update the data for the specified nodes.
     * Subclasses that support editing should override this to handle storing the data.
     * Default implementation does nothing, returns false.
     * \returns TRUE on success, otherwise FALSE
     * \since QGIS 3.42
     */
    virtual bool updateNodeData( const QHash<QgsPointCloudNodeId, QByteArray> &data );

    //! Returns extent of the data
    QgsRectangle extent() const { return mExtent; }

    //! Returns z min
    double zMin() const { return mZMin; }
    //! Returns z max
    double zMax() const { return mZMax; }

    //! Returns bounding box of root node in CRS coords
    QgsBox3D rootNodeBounds() const { return mRootBounds; }

    //! Returns scale of data relative to CRS
    QgsVector3D scale() const;

    //! Returns offset of data from CRS
    QgsVector3D offset() const;

    /**
     * Returns the number of points in one direction in a single node.
     */
    int span() const;

    /**
     * Sets the string used to define a subset of the point cloud.
     * \param subset The subset string to be used in a \a QgsPointCloudExpression
     * \returns true if the expression is parsed with no errors, false otherwise
     * \since QGIS 3.26
     */
    bool setSubsetString( const QString &subset );

    /**
     * Returns the string used to define a subset of the point cloud.
     * \returns The subset string or null QString if not implemented by the provider
     *
     * \since QGIS 3.26
     */
    QString subsetString() const;

    /**
     * Copies common properties to the \a destination index
     * \since QGIS 3.26
     */
    void copyCommonProperties( QgsAbstractPointCloudIndex *destination ) const;

    /**
     * Fetches the requested node data from the cache for the specified \a node and \a request.
     * If not found in the cache, nullptr is returned.
     * Caller takes ownership of the returned object.
     */
    QgsPointCloudBlock *getNodeDataFromCache( const QgsPointCloudNodeId &node, const QgsPointCloudRequest &request );

    /**
     * Stores existing \a data to the cache for the specified \a node and \a request. Ownership is not transferred, block gets cloned in the cache.
     */
    void storeNodeDataToCache( QgsPointCloudBlock *data, const QgsPointCloudNodeId &node, const QgsPointCloudRequest &request ) const;

    /**
     * Stores existing \a data to the cache for the specified \a node, \a request, \a expression and \a uri. Ownership is not transferred, block gets cloned in the cache.
     */
    static void storeNodeDataToCacheStatic( QgsPointCloudBlock *data, const QgsPointCloudNodeId &node, const QgsPointCloudRequest &request,
                                            const QgsPointCloudExpression &expression, const QString &uri );

    /**
     * Returns extra metadata that's not accessible through the other methods
     * in an implementation-specific dynamic structure.
     *
     * \since QGIS 3.42
     */
    virtual QVariantMap extraMetadata() const;

  protected: //TODO private
    //! Sets native attributes of the data
    void setAttributes( const QgsPointCloudAttributeCollection &attributes );

    QgsRectangle mExtent;  //!< 2D extent of data
    double mZMin = 0, mZMax = 0;   //!< Vertical extent of data

    mutable QMutex mHierarchyMutex;
    mutable QHash<QgsPointCloudNodeId, int> mHierarchy; //!< Data hierarchy
    QgsVector3D mScale; //!< Scale of our int32 coordinates compared to CRS coords
    QgsVector3D mOffset; //!< Offset of our int32 coordinates compared to CRS coords
    QgsBox3D mRootBounds;  //!< Bounds of the root node's cube (in int32 coordinates)
    QgsPointCloudAttributeCollection mAttributes; //! All native attributes stored in the file
    int mSpan = 0;  //!< Number of points in one direction in a single node
    QgsPointCloudExpression mFilterExpression;  //!< The filter expression to be evaluated when fetching node data

    QString mError;
    QString mUri;
    static QMutex sBlockCacheMutex;
    static QCache<QgsPointCloudCacheKey, QgsPointCloudBlock> sBlockCache;
};

#endif // !SIP_RUN


/**
 * \ingroup core
 * \brief Smart pointer for QgsAbstractPointCloudIndex
 *
 * This is a wrapper for QgsAbstractPointCloudIndex, an index for point cloud
 * layers. It contains a shared_pointer, ensuring that concurrent access to the
 * index is memory safe.
 *
 * \since QGIS 3.42
 */
class CORE_EXPORT QgsPointCloudIndex SIP_NODEFAULTCTORS
{
  public:
    //! Construct wrapper, takes ownership of index
    explicit QgsPointCloudIndex( QgsAbstractPointCloudIndex *index = nullptr ) SIP_SKIP;

    //! Checks if index is non-null
    operator bool() const;

    //! Returns pointer to the implementation class
    QgsAbstractPointCloudIndex *get() SIP_SKIP { return mIndex.get(); }

    /**
    * Loads the index from the file
    *
    * \see QgsAbstractPointCloudIndex::load
    */
    void load( const QString &fileName );

    /**
    * Returns whether index is loaded and valid
    *
    * \see QgsAbstractPointCloudIndex::isValid
    */
    bool isValid() const;

    /**
    * Returns the error that occurred during the loading of the index.
    *
    * \see QgsAbstractPointCloudIndex::error
    */
    QString error() const;

    /**
     * Returns the access type of the data
     * If the access type is Remote, data will be fetched from an HTTP server either synchronously or asynchronously
     * If the access type is local, the data is stored locally as a file and will only be fetch synchronously ( blocking request with nodeData only )
     * \note Always make sure to check before trying to use asyncNodeData since it is not supported in the case of local access type
    *
    * \see QgsAbstractPointCloudIndex::accessType
     */
    Qgis::PointCloudAccessType accessType() const;

    /**
    * Returns the coordinate reference system of the point cloud index
    *
    * \see QgsAbstractPointCloudIndex::crs
    */
    QgsCoordinateReferenceSystem crs() const;

    /**
    * Returns the number of points in the point cloud
    *
    * \see QgsAbstractPointCloudIndex::pointCount
    */
    qint64 pointCount() const;

    /**
    * Returns the original metadata map
    *
    * \see QgsAbstractPointCloudIndex::originalMetadata
    */
    QVariantMap originalMetadata() const;

    /**
     * Returns the object containing the statistics metadata extracted from the dataset
    *
    * \see QgsAbstractPointCloudIndex::metadataStatistics
     */
    QgsPointCloudStatistics metadataStatistics() const;

    /**
     * Writes the statistics object \a stats into the backing file, if possible.
     * Returns true if the data was written successfully.
    *
    * \see QgsAbstractPointCloudIndex::writeStatistics
     */
    bool writeStatistics( QgsPointCloudStatistics &stats );

    /**
    * Returns root node of the index
    *
    * \see QgsAbstractPointCloudIndex::root
    */
    QgsPointCloudNodeId root() const;

    /**
    * Returns whether the octree contain given node
    *
    * \see QgsAbstractPointCloudIndex::hasNode
    */
    bool hasNode( const QgsPointCloudNodeId &id ) const;

    /**
    * Returns object for a given node
    *
    * \see QgsAbstractPointCloudIndex::getNode
    */
    QgsPointCloudNode getNode( const QgsPointCloudNodeId &id ) const;

    /**
    * Returns all attributes that are stored in the file
    *
    * \see QgsAbstractPointCloudIndex::attributes
    */
    QgsPointCloudAttributeCollection attributes() const;

    /**
     * Returns node data block.
     *
     * e.g. positions (needs to be scaled and offset applied to get coordinates) or
     * classification, intensity or custom attributes.
     *
     * May return nullptr in case the node is not present or any other problem with loading
    *
    * \see QgsAbstractPointCloudIndex::nodeData
     */
    std::unique_ptr< QgsPointCloudBlock > nodeData( const QgsPointCloudNodeId &n, const QgsPointCloudRequest &request ) SIP_SKIP;

    /**
     * Returns a handle responsible for loading a node data block
     *
     * e.g. positions (needs to be scaled and offset applied to get coordinates) or
     * classification, intensity or custom attributes
     *
     * It is caller responsibility to free the handle and the block issued by the handle if the loading succeeds.
     *
     * May return nullptr in case the node is not present or any other problem with loading
    *
    * \see QgsAbstractPointCloudIndex::asyncNodeData
     */
    QgsPointCloudBlockRequest *asyncNodeData( const QgsPointCloudNodeId &n, const QgsPointCloudRequest &request ) SIP_SKIP;

    /**
     * Tries to update the data for the specified nodes.
     *
     * \returns TRUE on success, otherwise FALSE
     */
    bool updateNodeData( const QHash<QgsPointCloudNodeId, QByteArray> &data );

    /**
    * Returns extent of the data
    *
    * \see QgsAbstractPointCloudIndex::extent
    */
    QgsRectangle extent() const;

    /**
    * Returns z min
    *
    * \see QgsAbstractPointCloudIndex::zMin
    */
    double zMin() const;

    /**
    * Returns z max
    *
    * \see QgsAbstractPointCloudIndex::zMax
    */
    double zMax() const;

    /**
    * Returns bounding box of root node in CRS coords
    *
    * \see QgsAbstractPointCloudIndex::rootNodeBounds
    */
    QgsBox3D rootNodeBounds() const;

    /**
    * Returns scale of data relative to CRS
    *
    * \see QgsAbstractPointCloudIndex::scale
    */
    QgsVector3D scale() const;

    /**
    * Returns offset of data from CRS
    *
    * \see QgsAbstractPointCloudIndex::offset
    */
    QgsVector3D offset() const;

    /**
     * Returns the number of points in one direction in a single node.
    *
    * \see QgsAbstractPointCloudIndex::span
     */
    int span() const;

    /**
     * Sets the string used to define a subset of the point cloud.
     * \param subset The subset string to be used in a \a QgsPointCloudExpression
     * \returns true if the expression is parsed with no errors, false otherwise
    *
    * \see QgsAbstractPointCloudIndex::setSubsetString
     */
    bool setSubsetString( const QString &subset );

    /**
     * Returns the string used to define a subset of the point cloud.
     * \returns The subset string or null QString if not implemented by the provider
     *
    *
    * \see QgsAbstractPointCloudIndex::subsetString
     */
    QString subsetString() const;

    /**
     * Fetches the requested node data from the cache for the specified \a node and \a request.
     * If not found in the cache, nullptr is returned.
     * Caller takes ownership of the returned object.
    *
    * \see QgsAbstractPointCloudIndex::getNodeDataFromCache
     */
    QgsPointCloudBlock *getNodeDataFromCache( const QgsPointCloudNodeId &node, const QgsPointCloudRequest &request ) SIP_SKIP;

    /**
     * Stores existing \a data to the cache for the specified \a node and \a request. Ownership is not transferred, block gets cloned in the cache.
    *
    * \see QgsAbstractPointCloudIndex::storeNodeDataToCache
     */
    void storeNodeDataToCache( QgsPointCloudBlock *data, const QgsPointCloudNodeId &node, const QgsPointCloudRequest &request ) SIP_SKIP;

    /**
     * Returns extra metadata that's not accessible through the other methods
     * in an implementation-specific dynamic structure.
     *
     * \see QgsAbstractPointCloudIndex::extraMetadata
     */
    QVariantMap extraMetadata() const;

    /**
     * Tries to store pending changes to the data provider.
     * If errorMessage is not a null pointer, it will receive
     * an error message in case the call failed.
     * \return TRUE on success, otherwise FALSE
     */
    bool commitChanges( QString *errorMessage SIP_OUT = nullptr );

    //! Returns TRUE if there are uncommitted changes, FALSE otherwise
    bool isModified() const;

  private:
    std::shared_ptr<QgsAbstractPointCloudIndex> mIndex;

    friend class TestQgsPointCloudEditing;
};


#endif // QGSPOINTCLOUDINDEX_H
