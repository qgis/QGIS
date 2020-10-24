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

#include "qgis_core.h"
#include "qgsrectangle.h"
#include "qgsvector3d.h"
#include "qgis_sip.h"

#define SIP_NO_FILE


/**
 * \ingroup core
 *
 * Represents a indexed point cloud node in octree
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

    //! Compares nodes
    bool operator==( const IndexedPointCloudNode &other ) const;

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

//! Hash function for indexed nodes
uint qHash( const IndexedPointCloudNode &id );

/**
 * \ingroup core
 *
 * Represents packaged data bounds
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
    QgsPointCloudDataBounds( qint32 xmin, qint32 ymin, qint32 zmin, qint32 xmax, qint32 ymax, qint32 zmax );

    //! Returns x min
    qint32 xMin() const;

    //! Returns y min
    qint32 yMin() const;

    //! Returns z min
    qint32 zMin() const;

    //! Returns x max
    qint32 xMax() const;

    //! Returns y max
    qint32 yMax() const;

    //! Returns z max
    qint32 zMax() const;

    //! Returns 2D rectangle in map coordinates
    QgsRectangle mapExtent( const QgsVector3D &offset, const QgsVector3D &scale ) const;

  private:
    qint32 mXMin, mYMin, mZMin, mXMax, mYMax, mZMax;
};


/**
 * \ingroup core
 *
 * Represents a indexed point clouds data in octree
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsPointCloudIndex: public QObject
{
    Q_OBJECT
  public:
    //! Constructs index
    explicit QgsPointCloudIndex();
    ~QgsPointCloudIndex();

    //! Loads the index from the file
    virtual bool load( const QString &fileName ) = 0;

    //! Returns root node of the index
    IndexedPointCloudNode root() { return IndexedPointCloudNode( 0, 0, 0, 0 ); }

    //! Traverses tree and returns all nodes in specified depth
    QList<IndexedPointCloudNode> traverseTree( const QgsRectangle &extent, IndexedPointCloudNode n, int maxDepth = 3 );

    //! Returns all children of node
    QList<IndexedPointCloudNode> children( const IndexedPointCloudNode &n );

    //! Returns node positions, needs to be scaled and offset applied to get coordinates
    virtual QVector<qint32> nodePositionDataAsInt32( const IndexedPointCloudNode &n ) = 0;

    /**
     Loads classes data

    Standard LIDAR classes:
      0 Created, never classified
      1 Unclassified
      2 Ground
      3 Low Vegetation
      4 Medium Vegetation
      5 High Vegetation
      6 Building
      7 Low Point (noise)
      8 Model Key-point (mass point)
      9 Water
      10 Reserved for ASPRS Definition
      11 Reserved for ASPRS Definition
      12 Overlap Points
      13-31 Reserved for ASPRS Definition
    */
    virtual QVector<char> nodeClassesDataAsChar( const IndexedPointCloudNode &n ) = 0;

    //! Returns extent of the data
    QgsRectangle extent() const { return mExtent; }

    //! Returns z min
    double zMin() const { return mZMin; }
    //! Returns z max
    double zMax() const { return mZMax; }

    //! Returns bounds of particular node
    QgsPointCloudDataBounds nodeBounds( const IndexedPointCloudNode &n );

    //! Returns node extent in map coordinates
    QgsRectangle nodeMapExtent( const IndexedPointCloudNode &n );

    //! Returns scale
    QgsVector3D scale() const;

    //! Returns offset
    QgsVector3D offset() const;

  protected: //TODO private
    QgsRectangle mExtent;  //!< 2D extent of data
    double mZMin = 0, mZMax = 0;   //!< Vertical extent of data

    QHash<IndexedPointCloudNode, int> mHierarchy; //!< Data hierarchy
    QgsVector3D mScale; //!< Scale of our int32 coordinates compared to CRS coords
    QgsVector3D mOffset; //!< Offset of our int32 coordinates compared to CRS coords
    QgsPointCloudDataBounds mRootBounds;  //!< Bounds of the root node's cube (in int32 coordinates)

    int mSpan;  //!< Number of points in one direction in a single node
};

#endif // QGSPOINTCLOUDINDEX_H
