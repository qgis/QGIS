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
#include "qgspoint.h"


/** \ingroup core
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
    virtual ~QgsGeometryCollection();

    virtual QgsGeometryCollection *clone() const override SIP_FACTORY;

    /** Returns the number of geometries within the collection.
     */
    int numGeometries() const;

    /** Returns a const reference to a geometry from within the collection.
     * \param n index of geometry to return
     * \note not available in Python bindings
     */
    const QgsAbstractGeometry *geometryN( int n ) const SIP_SKIP;

    /** Returns a geometry from within the collection.
     * \param n index of geometry to return
     */
    QgsAbstractGeometry *geometryN( int n );

    //methods inherited from QgsAbstractGeometry
    bool isEmpty() const override;
    virtual int dimension() const override;
    virtual QString geometryType() const override;
    virtual void clear() override;
    virtual QgsAbstractGeometry *boundary() const override SIP_FACTORY;

    //! Adds a geometry and takes ownership. Returns true in case of success.
    virtual bool addGeometry( QgsAbstractGeometry *g SIP_TRANSFER );

    /** Inserts a geometry before a specified index and takes ownership. Returns true in case of success.
     * \param g geometry to insert. Ownership is transferred to the collection.
     * \param index position to insert geometry before
     */
    virtual bool insertGeometry( QgsAbstractGeometry *g, int index SIP_TRANSFER );

    /** Removes a geometry from the collection.
     * \param nr index of geometry to remove
     * \returns true if removal was successful.
     */
    virtual bool removeGeometry( int nr );

    virtual void transform( const QgsCoordinateTransform &ct, QgsCoordinateTransform::TransformDirection d = QgsCoordinateTransform::ForwardTransform,
                            bool transformZ = false ) override;
    void transform( const QTransform &t ) override;
#if 0
    virtual void clip( const QgsRectangle &rect ) override;
#endif
    virtual void draw( QPainter &p ) const override;

    bool fromWkb( QgsConstWkbPtr &wkb ) override;
    virtual bool fromWkt( const QString &wkt ) override;
    QByteArray asWkb() const override;
    QString asWkt( int precision = 17 ) const override;
    QDomElement asGML2( QDomDocument &doc, int precision = 17, const QString &ns = "gml" ) const override;
    QDomElement asGML3( QDomDocument &doc, int precision = 17, const QString &ns = "gml" ) const override;
    QString asJSON( int precision = 17 ) const override;

    virtual QgsRectangle boundingBox() const override;

    virtual QgsCoordinateSequence coordinateSequence() const override;
    virtual int nCoordinates() const override;

    virtual double closestSegment( const QgsPoint &pt, QgsPoint &segmentPt SIP_OUT,
                                   QgsVertexId &vertexAfter SIP_OUT, bool *leftOf SIP_OUT,
                                   double epsilon ) const override;
    bool nextVertex( QgsVertexId &id, QgsPoint &vertex SIP_OUT ) const override;

    //low-level editing
    virtual bool insertVertex( QgsVertexId position, const QgsPoint &vertex ) override;
    virtual bool moveVertex( QgsVertexId position, const QgsPoint &newPos ) override;
    virtual bool deleteVertex( QgsVertexId position ) override;

    virtual double length() const override;
    virtual double area() const override;
    virtual double perimeter() const override;

    bool hasCurvedSegments() const override;

    /** Returns a geometry without curves. Caller takes ownership
     * \param tolerance segmentation tolerance
     * \param toleranceType maximum segmentation angle or maximum difference between approximation and curve*/
    QgsAbstractGeometry *segmentize( double tolerance = M_PI_2 / 90, SegmentationToleranceType toleranceType = MaximumAngle ) const override SIP_FACTORY;

    /** Returns approximate rotation angle for a vertex. Usually average angle between adjacent segments.
     * \param vertex the vertex id
     * \returns rotation in radians, clockwise from north
     */
    double vertexAngle( QgsVertexId vertex ) const override;

    virtual int vertexCount( int part = 0, int ring = 0 ) const override;
    virtual int ringCount( int part = 0 ) const override;
    virtual int partCount() const override { return mGeometries.size(); }
    virtual QgsPoint vertexAt( QgsVertexId id ) const override { return mGeometries[id.part]->vertexAt( id ); }

    virtual bool addZValue( double zValue = 0 ) override;
    virtual bool addMValue( double mValue = 0 ) override;
    virtual bool dropZValue() override;
    virtual bool dropMValue() override;

#ifndef SIP_RUN

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

  protected:
    QVector< QgsAbstractGeometry * > mGeometries;

    /** Returns whether child type names are omitted from Wkt representations of the collection
     * \since QGIS 2.12
     */
    virtual bool wktOmitChildType() const;

    /** Reads a collection from a WKT string.
     */
    bool fromCollectionWkt( const QString &wkt, const QList<QgsAbstractGeometry *> &subtypes, const QString &defaultChildWkbType = QString() );

    virtual QgsRectangle calculateBoundingBox() const override;
    virtual void clearCache() const override;

  private:

    mutable QgsRectangle mBoundingBox;
    mutable QgsCoordinateSequence mCoordinateSequence;
};

#endif // QGSGEOMETRYCOLLECTIONV2_H
