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
    IndexedPointCloudNode();   // invalid node ID
    IndexedPointCloudNode( int _d, int _x, int _y, int _z );

    bool isValid() const { return d >= 0; }

    bool operator==( const IndexedPointCloudNode &other ) const;

    static IndexedPointCloudNode fromString( const QString &str );

    QString toString() const;

    // TODO make private
    int d = -1, x = -1, y = -1, z = -1;
};

uint qHash( const IndexedPointCloudNode &id );

///@cond PRIVATE
// what are the min/max to expect in the piece of data
class CORE_EXPORT QgsPointCloudDataBounds
{
  public:
    QgsPointCloudDataBounds(); // invalid
    QgsPointCloudDataBounds( qint32 xmin, qint32 ymin, qint32 zmin, qint32 xmax, qint32 ymax, qint32 zmax );

    qint32 xMin() const;

    qint32 yMin() const;

    qint32 zMin() const;

    qint32 xMax() const;

    qint32 yMax() const;

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

    explicit QgsPointCloudIndex();
    ~QgsPointCloudIndex();

    virtual bool load( const QString &fileName ) = 0;


    IndexedPointCloudNode root() { return IndexedPointCloudNode( 0, 0, 0, 0 ); }


    QList<IndexedPointCloudNode> traverseTree( const QgsRectangle &extent, IndexedPointCloudNode n, int maxDepth = 3 );
    QList<IndexedPointCloudNode> children( const IndexedPointCloudNode &n );

    virtual QVector<qint32> nodePositionDataAsInt32( const IndexedPointCloudNode &n ) = 0;

    /*
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

    QgsRectangle extent() const { return mExtent; }
    double zMin() const { return mZMin; }
    double zMax() const { return mZMax; }
    QgsPointCloudDataBounds nodeBounds( const IndexedPointCloudNode &n );
    QgsRectangle nodeMapExtent( const IndexedPointCloudNode &n );
    QgsVector3D scale() const;
    QgsVector3D offset() const;

  protected: //TODO private
    QgsRectangle mExtent;  //!< 2D extent of data
    double mZMin = 0, mZMax = 0;   //!< Vertical extent of data

    QHash<IndexedPointCloudNode, int> mHierarchy;
    QgsVector3D mScale; //!< Scale of our int32 coordinates compared to CRS coords
    QgsVector3D mOffset; //!< Offset of our int32 coordinates compared to CRS coords
    QgsPointCloudDataBounds mRootBounds;  //!< Bounds of the root node's cube (in int32 coordinates)

    int mSpan;  //!< Number of points in one direction in a single node
};


#endif // QGSPOINTCLOUDINDEX_H
