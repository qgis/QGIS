/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgsgeometrycollection.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/







class QgsGeometryCollection: QgsAbstractGeometry
{
%Docstring
Geometry collection

.. versionadded:: 2.10
%End

%TypeHeaderCode
#include "qgsgeometrycollection.h"
%End
  public:
    QgsGeometryCollection();
    QgsGeometryCollection( const QgsGeometryCollection &c );
    ~QgsGeometryCollection();

    virtual bool operator==( const QgsAbstractGeometry &other ) const;

    virtual bool operator!=( const QgsAbstractGeometry &other ) const;


    virtual QgsGeometryCollection *clone() const /Factory/;


    int numGeometries() const;
%Docstring
Returns the number of geometries within the collection.
%End


    int __len__() const;
%Docstring
Returns the number of geometries within the collection.
%End
%MethodCode
    sipRes = sipCpp->numGeometries();
%End

    //! Ensures that bool(obj) returns ``True`` (otherwise __len__() would be used)
    int __bool__() const;
%MethodCode
    sipRes = true;
%End




    SIP_PYOBJECT geometryN( int n ) /TypeHint="QgsAbstractGeometry"/;
%Docstring
Returns a geometry from within the collection.

:param n: index of geometry to return. An IndexError will be raised if no geometry with the specified index exists.
%End
%MethodCode
    if ( a0 < 0 || a0 >= sipCpp->numGeometries() )
    {
      PyErr_SetString( PyExc_IndexError, QByteArray::number( a0 ) );
      sipIsErr = 1;
    }
    else
    {
      return sipConvertFromType( sipCpp->geometryN( a0 ), sipType_QgsAbstractGeometry, NULL );
    }
%End


    virtual bool isEmpty() const;

    virtual int dimension() const;

    virtual QString geometryType() const;

    virtual void clear();

    virtual QgsGeometryCollection *snappedToGrid( double hSpacing, double vSpacing, double dSpacing = 0, double mSpacing = 0 ) const /Factory/;

    virtual bool removeDuplicateNodes( double epsilon = 4 * DBL_EPSILON, bool useZValues = false );

    virtual QgsAbstractGeometry *boundary() const /Factory/;

    virtual void adjacentVertices( QgsVertexId vertex, QgsVertexId &previousVertex /Out/, QgsVertexId &nextVertex /Out/ ) const;

    virtual int vertexNumberFromVertexId( QgsVertexId id ) const;


    virtual bool addGeometry( QgsAbstractGeometry *g /Transfer/ );
%Docstring
Adds a geometry and takes ownership. Returns ``True`` in case of success.
%End

    virtual bool insertGeometry( QgsAbstractGeometry *g /Transfer/, int index );
%Docstring
Inserts a geometry before a specified index and takes ownership. Returns ``True`` in case of success.

:param g: geometry to insert. Ownership is transferred to the collection.
:param index: position to insert geometry before
%End


    virtual bool removeGeometry( int nr );
%Docstring
Removes a geometry from the collection by index.

An IndexError will be raised if no geometry with the specified index exists.

:return: ``True`` if removal was successful.
%End
%MethodCode
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
%End

    virtual void transform( const QgsCoordinateTransform &ct, QgsCoordinateTransform::TransformDirection d = QgsCoordinateTransform::ForwardTransform, bool transformZ = false ) throw( QgsCsException );

    virtual void transform( const QTransform &t, double zTranslate = 0.0, double zScale = 1.0, double mTranslate = 0.0, double mScale = 1.0 );


    virtual void draw( QPainter &p ) const;


    virtual bool fromWkb( QgsConstWkbPtr &wkb );

    virtual bool fromWkt( const QString &wkt );

    virtual QByteArray asWkb() const;

    virtual QString asWkt( int precision = 17 ) const;

    virtual QDomElement asGml2( QDomDocument &doc, int precision = 17, const QString &ns = "gml", QgsAbstractGeometry::AxisOrder axisOrder = QgsAbstractGeometry::AxisOrder::XY ) const;

    virtual QDomElement asGml3( QDomDocument &doc, int precision = 17, const QString &ns = "gml", QgsAbstractGeometry::AxisOrder axisOrder = QgsAbstractGeometry::AxisOrder::XY ) const;

    virtual QString asJson( int precision = 17 ) const;


    virtual QgsRectangle boundingBox() const;


    virtual QgsCoordinateSequence coordinateSequence() const;

    virtual int nCoordinates() const;


    virtual double closestSegment( const QgsPoint &pt, QgsPoint &segmentPt /Out/, QgsVertexId &vertexAfter /Out/, int *leftOf /Out/ = 0, double epsilon = 4 * DBL_EPSILON ) const;

    virtual bool nextVertex( QgsVertexId &id, QgsPoint &vertex /Out/ ) const;


    virtual bool insertVertex( QgsVertexId position, const QgsPoint &vertex );

    virtual bool moveVertex( QgsVertexId position, const QgsPoint &newPos );

    virtual bool deleteVertex( QgsVertexId position );


    virtual double length() const;

    virtual double area() const;

    virtual double perimeter() const;


    virtual bool hasCurvedSegments() const;


    virtual QgsAbstractGeometry *segmentize( double tolerance = M_PI_2 / 90, SegmentationToleranceType toleranceType = MaximumAngle ) const /Factory/;

%Docstring
Returns a geometry without curves. Caller takes ownership

:param tolerance: segmentation tolerance
:param toleranceType: maximum segmentation angle or maximum difference between approximation and curve*
%End

    virtual double vertexAngle( QgsVertexId vertex ) const;

    virtual double segmentLength( QgsVertexId startVertex ) const;

    virtual int vertexCount( int part = 0, int ring = 0 ) const;

    virtual int ringCount( int part = 0 ) const;

    virtual int partCount() const;

    virtual QgsPoint vertexAt( QgsVertexId id ) const;

    virtual bool isValid( QString &error /Out/, int flags = 0 ) const;


    virtual bool addZValue( double zValue = 0 );

    virtual bool addMValue( double mValue = 0 );

    virtual bool dropZValue();

    virtual bool dropMValue();

    virtual void swapXy();

    virtual QgsGeometryCollection *toCurveType() const /Factory/;





    SIP_PYOBJECT __getitem__( int index ) /TypeHint="QgsAbstractGeometry"/;
%Docstring
Returns the geometry at the specified ``index``. An IndexError will be raised if no geometry with the specified ``index`` exists.

Indexes can be less than 0, in which case they correspond to geometries from the end of the collect. E.g. an index of -1
corresponds to the last geometry in the collection.

.. versionadded:: 3.6
%End
%MethodCode
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
%End

    void __delitem__( int index );
%Docstring
Deletes the geometry at the specified ``index``. A geometry at the ``index`` must already exist or an IndexError will be raised.

Indexes can be less than 0, in which case they correspond to geometries from the end of the collection. E.g. an index of -1
corresponds to the last geometry in the collection.

.. versionadded:: 3.6
%End
%MethodCode
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
%End

    SIP_PYOBJECT __iter__() /TypeHint="QgsGeometryPartIterator"/;
%Docstring
Iterates through all geometries in the collection.

.. versionadded:: 3.6
%End
%MethodCode
    sipRes = sipConvertFromNewType( new QgsGeometryPartIterator( sipCpp ), sipType_QgsGeometryPartIterator, Py_None );
%End

    virtual QgsGeometryCollection *createEmptyWithSameType() const /Factory/;


  protected:
    virtual int childCount() const;

    virtual QgsAbstractGeometry *childGeometry( int index ) const;


  protected:

    virtual bool wktOmitChildType() const;
%Docstring
Returns whether child type names are omitted from Wkt representations of the collection

.. versionadded:: 2.12
%End

    bool fromCollectionWkt( const QString &wkt, const QVector<QgsAbstractGeometry *> &subtypes, const QString &defaultChildWkbType = QString() );
%Docstring
Reads a collection from a WKT string.
%End

    virtual QgsRectangle calculateBoundingBox() const;

    virtual void clearCache() const;


};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgsgeometrycollection.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
