/***************************************************************************
                        qgsgeometrycollection.h
  -------------------------------------------------------------------
Date                 : 28 Oct 2014
Copyright            : (C) 2014 by Marco Hugentobler
email                : marco.hugentobler at sourcepole dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGEOMETRYCOLLECTIONV2_H
#define QGSGEOMETRYCOLLECTIONV2_H

#include <QVector>


#include "qgis_core.h"
#include "qgis.h"
#include "qgsabstractgeometry.h"

class QgsPoint;


/**
 * \ingroup core
 * \class QgsGeometryCollection
 * \brief Geometry collection
 * \since QGIS 2.10
 */
class CORE_EXPORT QgsGeometryCollection: public QgsAbstractGeometry
{
  public:
    QgsGeometryCollection();
    QgsGeometryCollection( const QgsGeometryCollection &c );
    QgsGeometryCollection &operator=( const QgsGeometryCollection &c );
    ~QgsGeometryCollection() override;

    bool operator==( const QgsAbstractGeometry &other ) const override;
    bool operator!=( const QgsAbstractGeometry &other ) const override;

    QgsGeometryCollection *clone() const override SIP_FACTORY;

    /**
     * Returns the number of geometries within the collection.
     */
    int numGeometries() const
    {
      return mGeometries.size();
    }

    /**
     * Returns a const reference to a geometry from within the collection.
     * \param n index of geometry to return
     * \note not available in Python bindings
     */
    const QgsAbstractGeometry *geometryN( int n ) const SIP_SKIP
    {
      return mGeometries.value( n );
    }

    /**
     * Returns a geometry from within the collection.
     * \param n index of geometry to return
     */
    QgsAbstractGeometry *geometryN( int n );

    //methods inherited from QgsAbstractGeometry
    bool isEmpty() const override;
    int dimension() const override;
    QString geometryType() const override;
    void clear() override;
    QgsGeometryCollection *snappedToGrid( double hSpacing, double vSpacing, double dSpacing = 0, double mSpacing = 0 ) const override SIP_FACTORY;
    bool removeDuplicateNodes( double epsilon = 4 * std::numeric_limits<double>::epsilon(), bool useZValues = false ) override;
    QgsAbstractGeometry *boundary() const override SIP_FACTORY;
    void adjacentVertices( QgsVertexId vertex, QgsVertexId &previousVertex SIP_OUT, QgsVertexId &nextVertex SIP_OUT ) const override;
    int vertexNumberFromVertexId( QgsVertexId id ) const override;

    //! Adds a geometry and takes ownership. Returns true in case of success.
    virtual bool addGeometry( QgsAbstractGeometry *g SIP_TRANSFER );

    /**
     * Inserts a geometry before a specified index and takes ownership. Returns true in case of success.
     * \param g geometry to insert. Ownership is transferred to the collection.
     * \param index position to insert geometry before
     */
    virtual bool insertGeometry( QgsAbstractGeometry *g SIP_TRANSFER, int index );

    /**
     * Removes a geometry from the collection.
     * \param nr index of geometry to remove
     * \returns true if removal was successful.
     */
    virtual bool removeGeometry( int nr );

    void transform( const QgsCoordinateTransform &ct, QgsCoordinateTransform::TransformDirection d = QgsCoordinateTransform::ForwardTransform, bool transformZ = false ) override SIP_THROW( QgsCsException );
    void transform( const QTransform &t, double zTranslate = 0.0, double zScale = 1.0, double mTranslate = 0.0, double mScale = 1.0 ) override;

    void draw( QPainter &p ) const override;

    bool fromWkb( QgsConstWkbPtr &wkb ) override;
    bool fromWkt( const QString &wkt ) override;
    QByteArray asWkb() const override;
    QString asWkt( int precision = 17 ) const override;
    QDomElement asGml2( QDomDocument &doc, int precision = 17, const QString &ns = "gml", QgsAbstractGeometry::AxisOrder axisOrder = QgsAbstractGeometry::AxisOrder::XY ) const override;
    QDomElement asGml3( QDomDocument &doc, int precision = 17, const QString &ns = "gml", QgsAbstractGeometry::AxisOrder axisOrder = QgsAbstractGeometry::AxisOrder::XY ) const override;
    QString asJson( int precision = 17 ) const override;

    QgsRectangle boundingBox() const override;

    QgsCoordinateSequence coordinateSequence() const override;
    int nCoordinates() const override;

    double closestSegment( const QgsPoint &pt, QgsPoint &segmentPt SIP_OUT, QgsVertexId &vertexAfter SIP_OUT, int *leftOf SIP_OUT = nullptr, double epsilon = 4 * std::numeric_limits<double>::epsilon() ) const override;
    bool nextVertex( QgsVertexId &id, QgsPoint &vertex SIP_OUT ) const override;

    //low-level editing
    bool insertVertex( QgsVertexId position, const QgsPoint &vertex ) override;
    bool moveVertex( QgsVertexId position, const QgsPoint &newPos ) override;
    bool deleteVertex( QgsVertexId position ) override;

    double length() const override;
    double area() const override;
    double perimeter() const override;

    bool hasCurvedSegments() const override;

    /**
     * Returns a geometry without curves. Caller takes ownership
     * \param tolerance segmentation tolerance
     * \param toleranceType maximum segmentation angle or maximum difference between approximation and curve*/
    QgsAbstractGeometry *segmentize( double tolerance = M_PI_2 / 90, SegmentationToleranceType toleranceType = MaximumAngle ) const override SIP_FACTORY;

    double vertexAngle( QgsVertexId vertex ) const override;
    double segmentLength( QgsVertexId startVertex ) const override;
    int vertexCount( int part = 0, int ring = 0 ) const override;
    int ringCount( int part = 0 ) const override;
    int partCount() const override;
    QgsPoint vertexAt( QgsVertexId id ) const override;

    bool addZValue( double zValue = 0 ) override;
    bool addMValue( double mValue = 0 ) override;
    bool dropZValue() override;
    bool dropMValue() override;
    void swapXy() override;
    QgsGeometryCollection *toCurveType() const override SIP_FACTORY;

#ifndef SIP_RUN
    void filterVertices( const std::function< bool( const QgsPoint & ) > &filter ) override;
    void transformVertices( const std::function< QgsPoint( const QgsPoint & ) > &transform ) override;

    /**
     * Cast the \a geom to a QgsGeometryCollection.
     * Should be used by qgsgeometry_cast<QgsGeometryCollection *>( geometry ).
     *
     * \note Not available in Python. Objects will be automatically be converted to the appropriate target type.
     * \since QGIS 3.0
     */
    inline const QgsGeometryCollection *cast( const QgsAbstractGeometry *geom ) const
    {
      if ( geom && QgsWkbTypes::isMultiType( geom->wkbType() ) )
        return static_cast<const QgsGeometryCollection *>( geom );
      return nullptr;
    }
#endif

    QgsGeometryCollection *createEmptyWithSameType() const override SIP_FACTORY;

  protected:
    int childCount() const override;
    QgsAbstractGeometry *childGeometry( int index ) const override;

  protected:
    QVector< QgsAbstractGeometry * > mGeometries;

    /**
     * Returns whether child type names are omitted from Wkt representations of the collection
     * \since QGIS 2.12
     */
    virtual bool wktOmitChildType() const;

    /**
     * Reads a collection from a WKT string.
     */
    bool fromCollectionWkt( const QString &wkt, const QVector<QgsAbstractGeometry *> &subtypes, const QString &defaultChildWkbType = QString() );

    QgsRectangle calculateBoundingBox() const override;
    void clearCache() const override;

  private:

    mutable QgsRectangle mBoundingBox;
};

// clazy:excludeall=qstring-allocations

#endif // QGSGEOMETRYCOLLECTIONV2_H
