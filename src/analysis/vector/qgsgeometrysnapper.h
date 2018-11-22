/***************************************************************************
 *  qgsgeometrysnapper.h                                                   *
 *  -------------------                                                    *
 *  copyright            : (C) 2014 by Sandro Mani / Sourcepole AG         *
 *  email                : smani@sourcepole.ch                             *
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS_GEOMETRY_SNAPPER_H
#define QGS_GEOMETRY_SNAPPER_H

#include <QMutex>
#include <QFuture>
#include <QStringList>
#include "qgsspatialindex.h"
#include "qgsabstractgeometry.h"
#include "qgspoint.h"
#include "qgsgeometry.h"
#include "qgis_analysis.h"

class QgsVectorLayer;

/**
 * \class QgsGeometrySnapper
 * \ingroup analysis
 * QgsGeometrySnapper allows a geometry to be snapped to the geometries within a
 * different reference layer. Vertices in the geometries will be modified to
 * match the reference layer features within a specified snap tolerance.
 * \since QGIS 3.0
 */
class ANALYSIS_EXPORT QgsGeometrySnapper : public QObject
{
    Q_OBJECT

  public:

    //! Snapping modes
    enum SnapMode
    {
      PreferNodes = 0, //!< Prefer to snap to nodes, even when a segment may be closer than a node. New nodes will be inserted to make geometries follow each other exactly when inside allowable tolerance.
      PreferClosest, //!< Snap to closest point, regardless of it is a node or a segment. New nodes will be inserted to make geometries follow each other exactly when inside allowable tolerance.
      PreferNodesNoExtraVertices, //!< Prefer to snap to nodes, even when a segment may be closer than a node. No new nodes will be inserted.
      PreferClosestNoExtraVertices, //!< Snap to closest point, regardless of it is a node or a segment. No new nodes will be inserted.
      EndPointPreferNodes, //!< Only snap start/end points of lines (point features will also be snapped, polygon features will not be modified), prefer to snap to nodes
      EndPointPreferClosest, //!< Only snap start/end points of lines (point features will also be snapped, polygon features will not be modified), snap to closest point
      EndPointToEndPoint, //!< Only snap the start/end points of lines to other start/end points of lines
    };

    /**
     * Constructor for QgsGeometrySnapper. A reference feature source which contains geometries to snap to must be
     * set. It is assumed that all geometries snapped using this object will have the
     * same CRS as the reference source (ie, no reprojection is performed).
     */
    QgsGeometrySnapper( QgsFeatureSource *referenceSource );

    /**
     * Snaps a geometry to the reference layer and returns the result. The geometry must be in the same
     * CRS as the reference layer, and must have the same type as the reference layer geometry. The snap tolerance
     * is specified in the layer units for the reference layer.
     */
    QgsGeometry snapGeometry( const QgsGeometry &geometry, double snapTolerance, SnapMode mode = PreferNodes ) const;

    /**
     * Snaps a set of features to the reference layer and returns the result. This operation is
     * multithreaded for performance. The featureSnapped() signal will be emitted each time a feature
     * is processed. The snap tolerance is specified in the layer units for the reference layer.
     */
    QgsFeatureList snapFeatures( const QgsFeatureList &features, double snapTolerance, SnapMode mode = PreferNodes );

    /**
     * Snaps a single geometry against a list of reference geometries.
     */
    static QgsGeometry snapGeometry( const QgsGeometry &geometry, double snapTolerance, const QList<QgsGeometry> &referenceGeometries, SnapMode mode = PreferNodes );

  signals:

    //! Emitted each time a feature has been processed when calling snapFeatures()
    void featureSnapped();

  private:
    struct ProcessFeatureWrapper
    {
      QgsGeometrySnapper *instance = nullptr;
      double snapTolerance;
      SnapMode mode;
      explicit ProcessFeatureWrapper( QgsGeometrySnapper *_instance, double snapTolerance, SnapMode mode )
        : instance( _instance )
        , snapTolerance( snapTolerance )
        , mode( mode )
      {}
      void operator()( QgsFeature &feature ) { instance->processFeature( feature, snapTolerance, mode ); }
    };

    enum PointFlag { SnappedToRefNode, SnappedToRefSegment, Unsnapped };

    QgsFeatureSource *mReferenceSource = nullptr;
    QgsFeatureList mInputFeatures;

    QgsSpatialIndex mIndex;
    mutable QMutex mIndexMutex;
    mutable QMutex mReferenceLayerMutex;

    void processFeature( QgsFeature &feature, double snapTolerance, SnapMode mode );

    static int polyLineSize( const QgsAbstractGeometry *geom, int iPart, int iRing );

};


/**
 * \class QgsInternalGeometrySnapper
 * \ingroup analysis
 * QgsInternalGeometrySnapper allows a set of geometries to be snapped to each other. It can be used to close gaps in layers.
 *
 * To use QgsInternalGeometrySnapper, first construct the snapper using the desired snap parameters. Then,
 * features are fed to to the snapper one-by-one by calling snapFeature(). Each feature passed by calling
 * snapFeature() will be snapped to any features which have already been processed by the snapper.
 *
 * After processing all desired features, the results can be fetched by calling snappedGeometries().
 * The returned QgsGeometryMap can be passed to QgsVectorDataProvider::changeGeometryValues() to save
 * the snapped geometries back to the source layer.
 *
 * \since QGIS 3.0
 */
class ANALYSIS_EXPORT QgsInternalGeometrySnapper
{

  public:

    /**
     * Constructor for QgsInternalGeometrySnapper. The \a snapTolerance and \a mode parameters dictate
     * how geometries will be snapped by the snapper.
     */
    QgsInternalGeometrySnapper( double snapTolerance, QgsGeometrySnapper::SnapMode mode = QgsGeometrySnapper::PreferNodes );

    /**
     * Snaps a single feature's geometry against all feature geometries already processed by
     * calls to snapFeature() in this object, and returns the snapped geometry.
     */
    QgsGeometry snapFeature( const QgsFeature &feature );

    /**
     * Returns a QgsGeometryMap of all feature geometries snapped by this object.
     */
    QgsGeometryMap snappedGeometries() const { return mProcessedGeometries; }

  private:

    bool mFirstFeature = true;
    double mSnapTolerance = 0;
    QgsGeometrySnapper::SnapMode mMode = QgsGeometrySnapper::PreferNodes;
    QgsSpatialIndex mProcessedIndex;
    QgsGeometryMap mProcessedGeometries;

};

#ifndef SIP_RUN

///@cond PRIVATE
class QgsSnapIndex
{
  public:
    struct CoordIdx
    {
      CoordIdx( const QgsAbstractGeometry *_geom, QgsVertexId _vidx )
        : geom( _geom )
        , vidx( _vidx )
      {}
      QgsPoint point() const { return geom->vertexAt( vidx ); }

      const QgsAbstractGeometry *geom = nullptr;
      QgsVertexId vidx;
    };

    enum SnapType { SnapPoint, SnapEndPoint, SnapSegment };

    class SnapItem
    {
      public:
        virtual ~SnapItem() = default;
        SnapType type;
        virtual QgsPoint getSnapPoint( const QgsPoint &p ) const = 0;

      protected:
        explicit SnapItem( SnapType _type ) : type( _type ) {}
    };

    class PointSnapItem : public QgsSnapIndex::SnapItem
    {
      public:
        explicit PointSnapItem( const CoordIdx *_idx, bool isEndPoint );
        QgsPoint getSnapPoint( const QgsPoint &/*p*/ ) const override;
        const CoordIdx *idx = nullptr;
    };

    class SegmentSnapItem : public QgsSnapIndex::SnapItem
    {
      public:
        SegmentSnapItem( const CoordIdx *_idxFrom, const CoordIdx *_idxTo );
        QgsPoint getSnapPoint( const QgsPoint &p ) const override;
        bool getIntersection( const QgsPoint &p1, const QgsPoint &p2, QgsPoint &inter ) const;
        bool getProjection( const QgsPoint &p, QgsPoint &pProj );
        const CoordIdx *idxFrom = nullptr;
        const CoordIdx *idxTo = nullptr;
    };

    QgsSnapIndex( const QgsPoint &origin, double cellSize );
    ~QgsSnapIndex();

    QgsSnapIndex( const QgsSnapIndex &rh ) = delete;
    QgsSnapIndex &operator=( const QgsSnapIndex &rh ) = delete;

    void addGeometry( const QgsAbstractGeometry *geom );
    QgsPoint getClosestSnapToPoint( const QgsPoint &p, const QgsPoint &q );
    SnapItem *getSnapItem( const QgsPoint &pos, double tol, PointSnapItem **pSnapPoint = nullptr, SegmentSnapItem **pSnapSegment = nullptr, bool endPointOnly = false ) const;

  private:
    typedef QList<SnapItem *> Cell;
    typedef QPair<QgsPoint, QgsPoint> Segment;

    class GridRow
    {
      public:
        GridRow() = default;
        ~GridRow();
        const Cell *getCell( int col ) const;
        Cell &getCreateCell( int col );
        QList<SnapItem *> getSnapItems( int colStart, int colEnd ) const;

      private:
        QList<QgsSnapIndex::Cell> mCells;
        int mColStartIdx = 0;
    };

    QgsPoint mOrigin;
    double mCellSize;

    QList<CoordIdx *> mCoordIdxs;
    QList<GridRow> mGridRows;
    int mRowsStartIdx;

    void addPoint( const CoordIdx *idx, bool isEndPoint );
    void addSegment( const CoordIdx *idxFrom, const CoordIdx *idxTo );
    const Cell *getCell( int col, int row ) const;
    Cell &getCreateCell( int col, int row );

};

///@endcond

#endif

#endif // QGS_GEOMETRY_SNAPPER_H
