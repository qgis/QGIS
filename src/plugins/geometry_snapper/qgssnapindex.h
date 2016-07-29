/***************************************************************************
 *  qgssnapindex.h                                                         *
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

#ifndef QGSSNAPINDEX_H
#define QGSSNAPINDEX_H

#include "qgspointv2.h"
#include "qgsabstractgeometryv2.h"

class QgsSnapIndex
{
  public:
    struct CoordIdx
    {
      CoordIdx( const QgsAbstractGeometryV2* _geom, QgsVertexId _vidx )
          : geom( _geom )
          , vidx( _vidx )
      {}
      QgsPointV2 point() const { return geom->vertexAt( vidx ); }

      const QgsAbstractGeometryV2* geom;
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
    void addGeometry( const QgsAbstractGeometryV2 *geom );
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

#endif // QGSSNAPINDEX_H
