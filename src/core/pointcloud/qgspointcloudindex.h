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

#include <QObject>
#include <QString>
#include <QHash>
#include <QStringList>
#include <QVector>
#include <QList>
#include <QMutex>
#include <QCache>

#include "qgis_core.h"
#include "qgsrectangle.h"
#include "qgsvector3d.h"
#include "qgis_sip.h"
#include "qgspointcloudblock.h"
#include "qgsrange.h"
#include "qgspointcloudattribute.h"
#include "qgspointcloudexpression.h"
#include "qgspointcloudrequest.h"

#define SIP_NO_FILE

class QgsPointCloudRequest;
class QgsPointCloudAttributeCollection;
class QgsCoordinateReferenceSystem;
class QgsPointCloudBlockRequest;
class QgsPointCloudStatistics;
class QgsPointCloudIndex;

/**
 * \ingroup core
 *
 * \brief Represents a indexed point cloud node in octree
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT IndexedPointCloudNode
{
  public:
    //! Constructs invalid node
    IndexedPointCloudNode();
    //! Constructs valid node
    IndexedPointCloudNode( int _d, int _x, int _y, int _z );

    //! Returns whether node is valid
    bool isValid() const { return mD >= 0; }

    // TODO c++20 - replace with = default

    bool operator==( IndexedPointCloudNode other ) const
    {
      return mD == other.d() && mX == other.x() && mY == other.y() && mZ == other.z();
    }

    /**
     * Returns the parent of the node
     * \since QGIS 3.20
     */
    IndexedPointCloudNode parentNode() const;

    //! Creates node from string
    static IndexedPointCloudNode fromString( const QString &str );

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

Q_DECLARE_TYPEINFO( IndexedPointCloudNode, Q_PRIMITIVE_TYPE );

//! Hash function for indexed nodes
CORE_EXPORT uint qHash( IndexedPointCloudNode id );

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
    QgsPointCloudCacheKey( const IndexedPointCloudNode &n, const QgsPointCloudRequest &request, const QgsPointCloudExpression &expression, const QString &uri );

    bool operator==( const QgsPointCloudCacheKey &other ) const;

    //! Returns the key's IndexedPointCloudNode
    IndexedPointCloudNode node() const { return mNode; }

    //! Returns the key's uri
    QString uri() const { return mUri; }

    //! Returns the key's QgsPointCloudRequest
    QgsPointCloudRequest request() const { return mRequest; }

    //! Returns the key's QgsPointCloudExpression
    QgsPointCloudExpression filterExpression() const { return mFilterExpression; }

  private:
    IndexedPointCloudNode mNode;
    QString mUri;
    QgsPointCloudRequest mRequest;
    QgsPointCloudExpression mFilterExpression;
};

//! Hash function for QgsPointCloudCacheKey
uint qHash( const QgsPointCloudCacheKey &key );

/**
 * \ingroup core
 *
 * \brief Represents packaged data bounds
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsPointCloudDataBounds
{
  public:
    //! Constructs invalid bounds
    QgsPointCloudDataBounds();
    //! Constructs bounds
    QgsPointCloudDataBounds( qint64 xmin, qint64 ymin, qint64 zmin, qint64 xmax, qint64 ymax, qint64 zmax );

    //! Returns x min
    qint64 xMin() const;

    //! Returns y min
    qint64 yMin() const;

    //! Returns z min
    qint64 zMin() const;

    //! Returns x max
    qint64 xMax() const;

    //! Returns y max
    qint64 yMax() const;

    //! Returns z max
    qint64 zMax() const;

    //! Returns 2D rectangle in map coordinates
    QgsRectangle mapExtent( const QgsVector3D &offset, const QgsVector3D &scale ) const;

    //! Returns the z range, applying the specified \a offset and \a scale.
    QgsDoubleRange zRange( const QgsVector3D &offset, const QgsVector3D &scale ) const;

  private:
    qint64 mXMin = 0;
    qint64 mYMin = 0;
    qint64 mZMin = 0;
    qint64 mXMax = 0;
    qint64 mYMax = 0;
    qint64 mZMax = 0;
};

/**
 * \ingroup core
 *
 * \brief Represents a indexed point clouds data in octree
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsPointCloudIndex: public QObject
{
    Q_OBJECT
  public:
    //! The access type of the data, local is for local files and remote for remote files (over HTTP)
    enum AccessType
    {
      Local, //!< Local means the source is a local file on the machine
      Remote //!< Remote means it's loaded through a protocol like HTTP
    };

    //! Constructs index
    explicit QgsPointCloudIndex();
    ~QgsPointCloudIndex();

    /**
     * Returns a clone of the current point cloud index object
     * \note It is the responsibility of the caller to handle the ownership and delete the object.
     * \since QGIS 3.26
     */
    virtual std::unique_ptr<QgsPointCloudIndex> clone() const = 0;

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
    virtual AccessType accessType() const = 0;

    //! Returns the coordinate reference system of the point cloud index
    virtual QgsCoordinateReferenceSystem crs() const = 0;
    //! Returns the number of points in the point cloud
    virtual qint64 pointCount() const = 0;
    //! Returns whether the dataset contains metadata of statistics
    virtual bool hasStatisticsMetadata() const = 0;
    //! Returns the statistic \a statistic of \a attribute
    virtual QVariant metadataStatistic( const QString &attribute, Qgis::Statistic statistic ) const;
    //! Returns the classes of \a attribute
    virtual QVariantList metadataClasses( const QString &attribute ) const;
    //! Returns the statistic \a statistic of the class \a value of the attribute \a attribute
    virtual QVariant metadataClassStatistic( const QString &attribute, const QVariant &value, Qgis::Statistic statistic ) const;
    //! Returns the original metadata map
    virtual QVariantMap originalMetadata() const = 0;

    /**
     * Returns the object containing the statistics metadata extracted from the dataset
     * \since QGIS 3.26
     */
    virtual QgsPointCloudStatistics metadataStatistics() const;

    //! Returns root node of the index
    IndexedPointCloudNode root() { return IndexedPointCloudNode( 0, 0, 0, 0 ); }

    //! Returns whether the octree contain given node
    virtual bool hasNode( const IndexedPointCloudNode &n ) const;

    //! Returns the number of points of a given node \a n
    virtual qint64 nodePointCount( const IndexedPointCloudNode &n ) const;

    //! Returns all children of node
    virtual QList<IndexedPointCloudNode> nodeChildren( const IndexedPointCloudNode &n ) const;

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
    virtual std::unique_ptr< QgsPointCloudBlock > nodeData( const IndexedPointCloudNode &n, const QgsPointCloudRequest &request ) = 0;

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
    virtual QgsPointCloudBlockRequest *asyncNodeData( const IndexedPointCloudNode &n, const QgsPointCloudRequest &request ) = 0;

    //! Returns extent of the data
    QgsRectangle extent() const { return mExtent; }

    //! Returns z min
    double zMin() const { return mZMin; }
    //! Returns z max
    double zMax() const { return mZMax; }

    //! Returns bounds of particular \a node
    QgsPointCloudDataBounds nodeBounds( const IndexedPointCloudNode &node ) const;

    /**
     * Returns the extent of a \a node in map coordinates.
     *
     * \see nodeZRange()
     */
    QgsRectangle nodeMapExtent( const IndexedPointCloudNode &node ) const;

    /**
     * Returns the z range of a \a node.
     *
     * \see nodeMapExtent()
     */
    QgsDoubleRange nodeZRange( const IndexedPointCloudNode &node ) const;

    //! Returns node's error in map units (used to determine in whether the node has enough detail for the current view)
    float nodeError( const IndexedPointCloudNode &n ) const;

    //! Returns scale
    QgsVector3D scale() const;

    //! Returns offset
    QgsVector3D offset() const;

    /**
     * Returns the number of points in one direction in a single node.
     */
    int span() const;

    /**
     * Returns the number of points of indexed point cloud node \a n
     */
    int nodePointCount( const IndexedPointCloudNode &n );

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
    void copyCommonProperties( QgsPointCloudIndex *destination ) const;

    /**
     * Fetches the requested node data from the cache for the specified \a node and \a request.
     * If not found in the cache, nullptr is returned.
     * Caller takes ownership of the returned object.
     */
    QgsPointCloudBlock *getNodeDataFromCache( const IndexedPointCloudNode &node, const QgsPointCloudRequest &request );

    /**
     * Stores existing \a data to the cache for the specified \a node and \a request. Ownership is not transferred, block gets cloned in the cache.
     */
    void storeNodeDataToCache( QgsPointCloudBlock *data, const IndexedPointCloudNode &node, const QgsPointCloudRequest &request );

    /**
     * Stores existing \a data to the cache for the specified \a node, \a request, \a expression and \a uri. Ownership is not transferred, block gets cloned in the cache.
     */
    static void storeNodeDataToCacheStatic( QgsPointCloudBlock *data, const IndexedPointCloudNode &node, const QgsPointCloudRequest &request,
                                            const QgsPointCloudExpression &expression, const QString &uri );

  protected: //TODO private
    //! Sets native attributes of the data
    void setAttributes( const QgsPointCloudAttributeCollection &attributes );

    QgsRectangle mExtent;  //!< 2D extent of data
    double mZMin = 0, mZMax = 0;   //!< Vertical extent of data

    mutable QMutex mHierarchyMutex;
    mutable QHash<IndexedPointCloudNode, int> mHierarchy; //!< Data hierarchy
    QgsVector3D mScale; //!< Scale of our int32 coordinates compared to CRS coords
    QgsVector3D mOffset; //!< Offset of our int32 coordinates compared to CRS coords
    QgsPointCloudDataBounds mRootBounds;  //!< Bounds of the root node's cube (in int32 coordinates)
    QgsPointCloudAttributeCollection mAttributes; //! All native attributes stored in the file
    int mSpan = 0;  //!< Number of points in one direction in a single node
    QgsPointCloudExpression mFilterExpression;  //!< The filter expression to be evaluated when fetching node data

    QString mError;
    QString mUri;
    static QMutex sBlockCacheMutex;
    static QCache<QgsPointCloudCacheKey, QgsPointCloudBlock> sBlockCache;
};

#endif // QGSPOINTCLOUDINDEX_H
