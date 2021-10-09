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

#ifndef QGSGEOMETRYCOLLECTION_H
#define QGSGEOMETRYCOLLECTION_H

#include <QVector>


#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsabstractgeometry.h"
#include "qgsrectangle.h"

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


    /**
     * Constructor for an empty geometry collection.
     */
    QgsGeometryCollection() SIP_HOLDGIL;

    QgsGeometryCollection( const QgsGeometryCollection &c );
    QgsGeometryCollection &operator=( const QgsGeometryCollection &c );
    ~QgsGeometryCollection() override;

    bool operator==( const QgsAbstractGeometry &other ) const override;
    bool operator!=( const QgsAbstractGeometry &other ) const override;

    QgsGeometryCollection *clone() const override SIP_FACTORY;

    /**
     * Returns the number of geometries within the collection.
     */
    int numGeometries() const SIP_HOLDGIL
    {
      return mGeometries.size();
    }

#ifdef SIP_RUN

    /**
     * Returns the number of geometries within the collection.
     */
    int __len__() const;
    % MethodCode
    sipRes = sipCpp->numGeometries();
    % End

    //! Ensures that bool(obj) returns TRUE (otherwise __len__() would be used)
    int __bool__() const;
    % MethodCode
    sipRes = true;
    % End
#endif


    /**
     * Returns a const reference to a geometry from within the collection.
     * \param n index of geometry to return
     * \note not available in Python bindings
     */
    const QgsAbstractGeometry *geometryN( int n ) const SIP_SKIP
    {
      return mGeometries.value( n );
    }

#ifndef SIP_RUN

    /**
     * Returns a geometry from within the collection.
     * \param n index of geometry to return
     */
    QgsAbstractGeometry *geometryN( int n ) SIP_HOLDGIL;
#else

    /**
     * Returns a geometry from within the collection.
     * \param n index of geometry to return.
     * \throws IndexError if no geometry with the specified index exists.
     */
    SIP_PYOBJECT geometryN( int n ) SIP_TYPEHINT( QgsAbstractGeometry );
    % MethodCode
    if ( a0 < 0 || a0 >= sipCpp->numGeometries() )
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      return sipConvertFromType( sipCpp->geometryN( a0 ), sipType_QgsAbstractGeometry, NULL );
    }
    % End
#endif


    //methods inherited from QgsAbstractGeometry
    bool isEmpty() const override SIP_HOLDGIL;
    int dimension() const override SIP_HOLDGIL;
    QString geometryType() const override SIP_HOLDGIL;
    void clear() override;
    QgsGeometryCollection *snappedToGrid( double hSpacing, double vSpacing, double dSpacing = 0, double mSpacing = 0 ) const override SIP_FACTORY;
    bool removeDuplicateNodes( double epsilon = 4 * std::numeric_limits<double>::epsilon(), bool useZValues = false ) override;
    QgsAbstractGeometry *boundary() const override SIP_FACTORY;
    void adjacentVertices( QgsVertexId vertex, QgsVertexId &previousVertex SIP_OUT, QgsVertexId &nextVertex SIP_OUT ) const override;
    int vertexNumberFromVertexId( QgsVertexId id ) const override;
    bool boundingBoxIntersects( const QgsRectangle &rectangle ) const override SIP_HOLDGIL;

    /**
     * Attempts to allocate memory for at least \a size geometries.
     *
     * If the number of geometries is known in advance, calling this function prior to adding geometries will prevent
     * reallocations and memory fragmentation.
     *
     * \since QGIS 3.10
     */
    void reserve( int size ) SIP_HOLDGIL;

    //! Adds a geometry and takes ownership. Returns TRUE in case of success.
    virtual bool addGeometry( QgsAbstractGeometry *g SIP_TRANSFER );

    /**
     * Inserts a geometry before a specified index and takes ownership. Returns TRUE in case of success.
     * \param g geometry to insert. Ownership is transferred to the collection.
     * \param index position to insert geometry before
     */
    virtual bool insertGeometry( QgsAbstractGeometry *g SIP_TRANSFER, int index );

#ifndef SIP_RUN

    /**
     * Removes a geometry from the collection.
     * \param nr index of geometry to remove
     * \returns TRUE if removal was successful.
     */
    virtual bool removeGeometry( int nr );
#else

    /**
     * Removes a geometry from the collection by index.
     *
     * \returns TRUE if removal was successful.
     * \throws IndexError if no geometry with the specified index exists.
     */
    virtual bool removeGeometry( int nr );
    % MethodCode
    const int count = sipCpp->numGeometries();
    if ( a0 < 0 || a0 >= count )
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      return PyBool_FromLong( sipCpp->removeGeometry( a0 ) );
    }
    % End
#endif

    void normalize() final SIP_HOLDGIL;
    void transform( const QgsCoordinateTransform &ct, Qgis::TransformDirection d = Qgis::TransformDirection::Forward, bool transformZ = false ) override SIP_THROW( QgsCsException );
    void transform( const QTransform &t, double zTranslate = 0.0, double zScale = 1.0, double mTranslate = 0.0, double mScale = 1.0 ) override;

    void draw( QPainter &p ) const override;
    QPainterPath asQPainterPath() const override;

    bool fromWkb( QgsConstWkbPtr &wkb ) override;
    bool fromWkt( const QString &wkt ) override;

    int wkbSize( QgsAbstractGeometry::WkbFlags flags = QgsAbstractGeometry::WkbFlags() ) const override;
    QByteArray asWkb( QgsAbstractGeometry::WkbFlags flags = QgsAbstractGeometry::WkbFlags() ) const override;
    QString asWkt( int precision = 17 ) const override;
    QDomElement asGml2( QDomDocument &doc, int precision = 17, const QString &ns = "gml", QgsAbstractGeometry::AxisOrder axisOrder = QgsAbstractGeometry::AxisOrder::XY ) const override;
    QDomElement asGml3( QDomDocument &doc, int precision = 17, const QString &ns = "gml", QgsAbstractGeometry::AxisOrder axisOrder = QgsAbstractGeometry::AxisOrder::XY ) const override;
    json asJsonObject( int precision = 17 ) const override SIP_SKIP;
    QString asKml( int precision = 17 ) const override;

    QgsRectangle boundingBox() const override;

    QgsCoordinateSequence coordinateSequence() const override;
    int nCoordinates() const override;

    double closestSegment( const QgsPoint &pt, QgsPoint &segmentPt SIP_OUT, QgsVertexId &vertexAfter SIP_OUT, int *leftOf SIP_OUT = nullptr, double epsilon = 4 * std::numeric_limits<double>::epsilon() ) const override;
    bool nextVertex( QgsVertexId &id, QgsPoint &vertex SIP_OUT ) const override;

    //low-level editing
    bool insertVertex( QgsVertexId position, const QgsPoint &vertex ) override;
    bool moveVertex( QgsVertexId position, const QgsPoint &newPos ) override;
    bool deleteVertex( QgsVertexId position ) override;

    double length() const override SIP_HOLDGIL;
    double area() const override SIP_HOLDGIL;
    double perimeter() const override SIP_HOLDGIL;

    bool hasCurvedSegments() const override SIP_HOLDGIL;

    /**
     * Returns a geometry without curves. Caller takes ownership
     * \param tolerance segmentation tolerance
     * \param toleranceType maximum segmentation angle or maximum difference between approximation and curve
    */
    QgsAbstractGeometry *segmentize( double tolerance = M_PI_2 / 90, SegmentationToleranceType toleranceType = MaximumAngle ) const override SIP_FACTORY;

    double vertexAngle( QgsVertexId vertex ) const override;
    double segmentLength( QgsVertexId startVertex ) const override;
    int vertexCount( int part = 0, int ring = 0 ) const override;
    int ringCount( int part = 0 ) const override;
    int partCount() const override;
    QgsPoint vertexAt( QgsVertexId id ) const override;
    bool isValid( QString &error SIP_OUT, Qgis::GeometryValidityFlags flags = Qgis::GeometryValidityFlags() ) const override;

    bool addZValue( double zValue = 0 ) override;
    bool addMValue( double mValue = 0 ) override;
    bool dropZValue() override;
    bool dropMValue() override;
    void swapXy() override;
    QgsGeometryCollection *toCurveType() const override SIP_FACTORY;
    const QgsAbstractGeometry *simplifiedTypeRef() const override SIP_HOLDGIL;

    bool transform( QgsAbstractGeometryTransformer *transformer, QgsFeedback *feedback = nullptr ) override;

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
    inline static const QgsGeometryCollection *cast( const QgsAbstractGeometry *geom )
    {
      if ( geom && QgsWkbTypes::isMultiType( geom->wkbType() ) )
        return static_cast<const QgsGeometryCollection *>( geom );
      return nullptr;
    }
#endif


#ifdef SIP_RUN

    /**
    * Returns the geometry at the specified ``index``.
    *
    * Indexes can be less than 0, in which case they correspond to geometries from the end of the collect. E.g. an index of -1
    * corresponds to the last geometry in the collection.
    *
    * \throws IndexError if no geometry with the specified ``index`` exists.
    *
    * \since QGIS 3.6
    */
    SIP_PYOBJECT __getitem__( int index ) SIP_TYPEHINT( QgsAbstractGeometry );
    % MethodCode
    const int count = sipCpp->numGeometries();
    if ( a0 < -count || a0 >= count )
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else if ( a0 >= 0 )
    {
      return sipConvertFromType( sipCpp->geometryN( a0 ), sipType_QgsAbstractGeometry, NULL );
    }
    else
    {
      return sipConvertFromType( sipCpp->geometryN( count + a0 ), sipType_QgsAbstractGeometry, NULL );
    }
    % End

    /**
     * Deletes the geometry at the specified ``index``.
     *
     * Indexes can be less than 0, in which case they correspond to geometries from the end of the collection. E.g. an index of -1
     * corresponds to the last geometry in the collection.
     *
     * \throws IndexError if no geometry at the ``index`` exists
     *
     * \since QGIS 3.6
     */
    void __delitem__( int index );
    % MethodCode
    const int count = sipCpp->numGeometries();
    if ( a0 >= 0 && a0 < count )
      sipCpp->removeGeometry( a0 );
    else if ( a0 < 0 && a0 >= -count )
      sipCpp->removeGeometry( count + a0 );
    else
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    % End

    /**
     * Iterates through all geometries in the collection.
     *
     * \since QGIS 3.6
     */
    SIP_PYOBJECT __iter__() SIP_TYPEHINT( QgsGeometryPartIterator );
    % MethodCode
    sipRes = sipConvertFromNewType( new QgsGeometryPartIterator( sipCpp ), sipType_QgsGeometryPartIterator, Py_None );
    % End
#endif

    QgsGeometryCollection *createEmptyWithSameType() const override SIP_FACTORY;

  protected:
    int childCount() const override;
    QgsAbstractGeometry *childGeometry( int index ) const override;
    int compareToSameClass( const QgsAbstractGeometry *other ) const final;

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
    mutable bool mHasCachedValidity = false;
    mutable QString mValidityFailureReason;
};

// clazy:excludeall=qstring-allocations

#endif // QGSGEOMETRYCOLLECTION_H
