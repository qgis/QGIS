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
#include "qgspointv2.h"

class QgsVectorLayer;

/**
 * \class QgsGeometrySnapper
 * \ingroup analysis
 * QgsGeometrySnapper allows a geometry to be snapped to the geometries within a
 * different reference layer. Vertices in the geometries will be modified to
 * match the reference layer features within a specified snap tolerance.
 * \note added in QGIS 3.0
 */
class ANALYSIS_EXPORT QgsGeometrySnapper : public QObject
{
    Q_OBJECT

  public:

    //! Snapping modes
    enum SnapMode
    {
      PreferNodes = 0, //!< Prefer to snap to nodes, even when a segment may be closer than a node
      PreferClosest, //!< Snap to closest point, regardless of it is a node or a segment
    };

    /**
     * Constructor for QgsGeometrySnapper. A reference layer which contains geometries to snap to must be
     * set. It is assumed that all geometries snapped using this object will have the
     * same CRS as the reference layer (ie, no reprojection is performed).
     */
    QgsGeometrySnapper( QgsVectorLayer* referenceLayer );

    /**
     * Snaps a geometry to the reference layer and returns the result. The geometry must be in the same
     * CRS as the reference layer, and must have the same type as the reference layer geometry. The snap tolerance
     * is specified in the layer units for the reference layer.
     */
    QgsGeometry snapGeometry( const QgsGeometry& geometry, double snapTolerance, SnapMode mode = PreferNodes ) const;

    /**
     * Snaps a set of features to the reference layer and returns the result. This operation is
     * multithreaded for performance. The featureSnapped() signal will be emitted each time a feature
     * is processed. The snap tolerance is specified in the layer units for the reference layer.
     */
    QgsFeatureList snapFeatures( const QgsFeatureList& features, double snapTolerance, SnapMode mode = PreferNodes );

  signals:

    //! Emitted each time a feature has been processed when calling snapFeatures()
    void featureSnapped();

  private:
    struct ProcessFeatureWrapper
    {
      QgsGeometrySnapper* instance;
      double snapTolerance;
      SnapMode mode;
      explicit ProcessFeatureWrapper( QgsGeometrySnapper* _instance, double snapTolerance, SnapMode mode )
          : instance( _instance )
          , snapTolerance( snapTolerance )
          , mode( mode )
      {}
      void operator()( QgsFeature& feature ) { return instance->processFeature( feature, snapTolerance, mode ); }
    };

    enum PointFlag { SnappedToRefNode, SnappedToRefSegment, Unsnapped };

    QgsVectorLayer* mReferenceLayer;
    QgsFeatureList mInputFeatures;

    QgsSpatialIndex mIndex;
    mutable QMutex mIndexMutex;
    mutable QMutex mReferenceLayerMutex;

    void processFeature( QgsFeature& feature, double snapTolerance, SnapMode mode );

    int polyLineSize( const QgsAbstractGeometry* geom, int iPart, int iRing ) const;
};

///@cond PRIVATE
class QgsSnapIndex
{
  public:
    struct CoordIdx
    {
      CoordIdx( const QgsAbstractGeometry* _geom, QgsVertexId _vidx )
          : geom( _geom )
          , vidx( _vidx )
      {}
      QgsPointV2 point() const { return geom->vertexAt( vidx ); }

      const QgsAbstractGeometry* geom;
      QgsVertexId vidx;
    };

    enum SnapType { SnapPoint, SnapSegment };

    class SnapItem
    {
      public:
        virtual ~SnapItem() {}
        SnapType type;
        virtual QgsPointV2 getSnapPoint( const QgsPointV2& p ) const = 0;

      protected:
        explicit SnapItem( SnapType _type ) : type( _type ) {}
    };

    class PointSnapItem : public QgsSnapIndex::SnapItem
    {
      public:
        explicit PointSnapItem( const CoordIdx* _idx );
        QgsPointV2 getSnapPoint( const QgsPointV2 &/*p*/ ) const override;
        const CoordIdx* idx;
    };

    class SegmentSnapItem : public QgsSnapIndex::SnapItem
    {
      public:
        SegmentSnapItem( const CoordIdx* _idxFrom, const CoordIdx* _idxTo );
        QgsPointV2 getSnapPoint( const QgsPointV2 &p ) const override;
        bool getIntersection( const QgsPointV2& p1, const QgsPointV2& p2, QgsPointV2& inter ) const;
        bool getProjection( const QgsPointV2 &p, QgsPointV2 &pProj );
        const CoordIdx* idxFrom;
        const CoordIdx* idxTo;
    };

    QgsSnapIndex( const QgsPointV2& origin, double cellSize );
    ~QgsSnapIndex();
    void addGeometry( const QgsAbstractGeometry *geom );
    QgsPointV2 getClosestSnapToPoint( const QgsPointV2& p, const QgsPointV2& q );
    SnapItem *getSnapItem( const QgsPointV2& pos, double tol, PointSnapItem **pSnapPoint = nullptr, SegmentSnapItem **pSnapSegment = nullptr ) const;

  private:
    typedef QList<SnapItem*> Cell;
    typedef QPair<QgsPointV2, QgsPointV2> Segment;

    class GridRow
    {
      public:
        GridRow() : mColStartIdx( 0 ) {}
        ~GridRow();
        const Cell *getCell( int col ) const;
        Cell& getCreateCell( int col );
        QList<SnapItem*> getSnapItems( int colStart, int colEnd ) const;

      private:
        QList<QgsSnapIndex::Cell> mCells;
        int mColStartIdx;
    };

    QgsPointV2 mOrigin;
    double mCellSize;

    QList<CoordIdx*> mCoordIdxs;
    QList<GridRow> mGridRows;
    int mRowsStartIdx;

    void addPoint( const CoordIdx* idx );
    void addSegment( const CoordIdx* idxFrom, const CoordIdx* idxTo );
    const Cell* getCell( int col, int row ) const;
    Cell &getCreateCell( int col, int row );

    QgsSnapIndex( const QgsSnapIndex& rh );
    QgsSnapIndex& operator=( const QgsSnapIndex& rh );
};

///@endcond

#endif // QGS_GEOMETRY_SNAPPER_H
