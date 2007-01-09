
/** polyline is just a list of points */
typedef QVector<QgsPoint> QgsPolyline;

/** polygon: first item of the list is outer ring, inner rings (if any) start from second item */
typedef QVector< QVector<QgsPoint> > QgsPolygon;
    
/** a collection of QgsPoints that share a common collection of attributes */
typedef QVector<QgsPoint> QgsMultiPoint;

/** a collection of QgsPolylines that share a common collection of attributes */
typedef QVector< QVector<QgsPoint> > QgsMultiPolyline;

/** a collection of QgsPolygons that share a common collection of attributes */
typedef QVector< QVector< QVector<QgsPoint> > > QgsMultiPolygon;

    
template <TYPE>
%MappedType QVector< QVector<TYPE> >
{
%TypeHeaderCode
#include <QVector>
%End

%ConvertFromTypeCode
  // Create the list.
  PyObject *l;

  if ((l = PyList_New(sipCpp->size())) == NULL)
    return NULL;
      
  const sipMappedType* qvector_qgspoint = sipFindMappedType("QVector<QgsPoint>");
      
  // Set the list elements.
  for (int i = 0; i < sipCpp->size(); ++i)
  {
    QVector<TYPE>* t = new QVector<TYPE>(sipCpp->at(i));
    PyObject *tobj;

    if ((tobj = sipConvertFromMappedType(t, qvector_qgspoint, sipTransferObj)) == NULL)
    {
      Py_DECREF(l);
      delete t;
      return NULL;
    }
    PyList_SET_ITEM(l, i, tobj);
  }
 
  return l;
%End

%ConvertToTypeCode
  // Check the type if that is all that is required.
  if (sipIsErr == NULL)
  {
    if (!PyList_Check(sipPy))
      return 0;

    for (int i = 0; i < PyList_GET_SIZE(sipPy); ++i)
      if (!sipCanConvertToInstance(PyList_GET_ITEM(sipPy, i), sipClass_TYPE, SIP_NOT_NONE))
        return 0;

    return 1;
  }

  const sipMappedType* qvector_qgspoint = sipFindMappedType("QVector<QgsPoint>");
  
  QVector< QVector<TYPE> > *ql = new QVector< QVector<TYPE> >;

  for (int i = 0; i < PyList_GET_SIZE(sipPy); ++i)
  {
    int state;
    //TYPE *t = reinterpret_cast<TYPE *>(sipConvertToInstance(PyList_GET_ITEM(sipPy, i), sipClass_TYPE, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));
    QVector<TYPE> * t = reinterpret_cast< QVector<TYPE> * >(sipConvertToMappedType(PyList_GET_ITEM(sipPy, i), qvector_qgspoint, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));

    if (*sipIsErr)
    {
      sipReleaseInstance(t, sipClass_TYPE, state);
      delete ql;
      return 0;
    }
    ql->append(*t);
    sipReleaseInstance(t, sipClass_TYPE, state);
  }

  *sipCppPtr = ql;
  return sipGetState(sipTransferObj);
%End

};


template <TYPE>
%MappedType QVector< QVector< QVector<TYPE> > >
{
%TypeHeaderCode
#include <QVector>
%End

%ConvertFromTypeCode
  // Create the list.
  PyObject *l;

  if ((l = PyList_New(sipCpp->size())) == NULL)
    return NULL;
      
  const sipMappedType* qvector_qgspoint = sipFindMappedType("QVector<QVector<QgsPoint> >");
      
  // Set the list elements.
  for (int i = 0; i < sipCpp->size(); ++i)
  {
    QVector<QVector<TYPE> >* t = new QVector<QVector<TYPE> >(sipCpp->at(i));
    PyObject *tobj;

    if ((tobj = sipConvertFromMappedType(t, qvector_qgspoint, sipTransferObj)) == NULL)
    {
      Py_DECREF(l);
      delete t;
      return NULL;
    }
    PyList_SET_ITEM(l, i, tobj);
  }
  return l;
%End

%ConvertToTypeCode
  // Check the type if that is all that is required.
  if (sipIsErr == NULL)
  {
    if (!PyList_Check(sipPy))
      return 0;

    // TODO: this is not correct!
    for (int i = 0; i < PyList_GET_SIZE(sipPy); ++i)
      if (!sipCanConvertToInstance(PyList_GET_ITEM(sipPy, i), sipClass_TYPE, SIP_NOT_NONE))
        return 0;

    return 1;
  }

  const sipMappedType* qvector_qgspoint = sipFindMappedType("QVector<QVector<QgsPoint> >");
  
  QVector< QVector< QVector<TYPE> > > *ql = new QVector< QVector< QVector<TYPE> > >;

  for (int i = 0; i < PyList_GET_SIZE(sipPy); ++i)
  {
    int state;
    //TYPE *t = reinterpret_cast<TYPE *>(sipConvertToInstance(PyList_GET_ITEM(sipPy, i), sipClass_TYPE, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));
    QVector<QVector<TYPE> > * t = reinterpret_cast< QVector< QVector<TYPE> > * >(sipConvertToMappedType(PyList_GET_ITEM(sipPy, i), qvector_qgspoint, sipTransferObj, SIP_NOT_NONE, &state, sipIsErr));

    if (*sipIsErr)
    {
      sipReleaseInstance(t, sipClass_TYPE, state);
      delete ql;
      return 0;
    }
    ql->append(*t);
    sipReleaseInstance(t, sipClass_TYPE, state);
  }

  *sipCppPtr = ql;
  return sipGetState(sipTransferObj);
%End

};



class QgsGeometry
{
%TypeHeaderCode
#include <qgsgeometry.h>
%End

  public:

    //! Constructor
    QgsGeometry();
    
    /** copy constructor will prompt a deep copy of the object */
    QgsGeometry( const QgsGeometry & );
    

    //! Destructor
    ~QgsGeometry();


    /** static method that creates geometry from WKT */
    static QgsGeometry* fromWkt(QString wkt) /Factory/;
    
    typedef unsigned int size_t;

    
    /** 
       Set the geometry, feeding in the buffer containing OGC Well-Known Binary and the buffer's length.
       This class will take ownership of the buffer.
    */
    // SIP: buffer will be transferred from python to C++
    // TODO: create pythonic interface that will receive wkb as a string
    void setWkbAndOwnership(unsigned char * wkb /Transfer/, size_t length);
    
    /** 
       Returns the buffer containing this geometry in WKB format.
       You may wish to use in conjunction with wkbSize().
    */
    unsigned char * wkbBuffer() const;
    
    /** 
       Returns the size of the WKB in wkbBuffer().
    */
    size_t wkbSize() const;
    
    /** 
       Returns the QString containing this geometry in WKT format.
    */
    const QString & wkt() const; 

    /** Returns type of wkb (point / linestring / polygon etc.) */
    QGis::WKBTYPE wkbType() const;

    /** Returns type of the vector */
    QGis::VectorType vectorType() const;

    /** Returns true if wkb of the geometry is of WKBMulti* type */
    bool isMultipart() const;

    /**
       Set the geometry, feeding in a geometry in GEOS format.
    */
    // TODO: unsupported class... would be possible to use PyGEOS?
    //void setGeos(geos::Geometry* geos);

    double distance(QgsGeometry& geom);

    /**
       Returns the vertex closest to the given point 
       (and also vertex index, squared distance and indexes of the vertices before/after)
    */
    QgsPoint closestVertex(const QgsPoint& point, QgsGeometryVertexIndex& atVertex, int& beforeVertex, int& afterVertex, double& sqrDist) const;


    /**
       Returns the indexes of the vertices before and after the given vertex index.

       This function takes into account the following factors:

       1. If the given vertex index is at the end of a linestring,
          the adjacent index will be -1 (for "no adjacent vertex")
       2. If the given vertex index is at the end of a linear ring
          (such as in a polygon), the adjacent index will take into
          account the first vertex is equal to the last vertex (and will
          skip equal vertex positions).
    */
    void adjacentVerticies(const QgsGeometryVertexIndex& atVertex, int& beforeVertex, int& afterVertex) const;


    /** Insert a new vertex before the given vertex index,
     *  ring and item (first number is index 0)
     *  If the requested vertex number (beforeVertex.back()) is greater
     *  than the last actual vertex on the requested ring and item,
     *  it is assumed that the vertex is to be appended instead of inserted.
     *  Returns FALSE if atVertex does not correspond to a valid vertex
     *  on this geometry (including if this geometry is a Point).
     *  It is up to the caller to distinguish between
     *  these error conditions.  (Or maybe we add another method to this
     *  object to help make the distinction?)
     */
    bool insertVertexBefore(double x, double y, QgsGeometryVertexIndex beforeVertex);

    /** Moves the vertex at the given position number,
     *  ring and item (first number is index 0)
     *  to the given coordinates.
     *  Returns FALSE if atVertex does not correspond to a valid vertex
     *  on this geometry 
     */
    bool moveVertexAt(double x, double y, QgsGeometryVertexIndex atVertex);

    /** Deletes the vertex at the given position number,
     *  ring and item (first number is index 0)
     *  Returns FALSE if atVertex does not correspond to a valid vertex
     *  on this geometry (including if this geometry is a Point),
     *  or if the number of remaining verticies in the linestring
     *  would be less than two.
     *  It is up to the caller to distinguish between
     *  these error conditions.  (Or maybe we add another method to this
     *  object to help make the distinction?)
     */
    bool deleteVertexAt(QgsGeometryVertexIndex atVertex);

    /**
        Returns the squared cartesian distance between the given point
        to the given vertex index (vertex at the given position number,
        ring and item (first number is index 0))

     */
    double sqrDistToVertexAt(QgsPoint& point,
                             QgsGeometryVertexIndex& atVertex) const;

    /**
     *  Modifies x and y to indicate the location of
     *  the vertex at the given position number,
     *  ring and item (first number is index 0)
     *  to the given coordinates
     *
     *  Returns TRUE if atVertex is a valid position on
     *  the geometry, otherwise FALSE.
     *
     *  If FALSE, x and y are not modified.
     */
    bool vertexAt(double &x, double &y, QgsGeometryVertexIndex atVertex) const;

    /**
        Returns, in atVertex, the closest vertex in this geometry to the given point.
        The squared cartesian distance is also returned in sqrDist.
     */
    QgsPoint closestVertexWithContext(QgsPoint& point,
                                      QgsGeometryVertexIndex& atVertex,
                                      double& sqrDist);

    /**
        Returns, in beforeVertex, the closest segment in this geometry to the given point.
        The squared cartesian distance is also returned in sqrDist.
     */
    QgsPoint closestSegmentWithContext(QgsPoint& point,
                                       QgsGeometryVertexIndex& beforeVertex,
                                       double& sqrDist);

    /**Returns the bounding box of this feature*/
    QgsRect boundingBox() const;

    /**Test for intersection with a rectangle (uses GEOS)*/
    bool intersects(const QgsRect& r) const;

    /**Also tests for intersection, but uses direct geos export of QgsGeometry instead wkb export and geos wkb import. Therefore this method is faster and could replace QgsGeometry::intersects in the future*/
    bool fast_intersects(const QgsRect& r) const;

    /** Test for containment of a point (uses GEOS) */
    bool contains(QgsPoint* p) const;

    /**Creates a geos geometry from this features geometry. Note, that the returned object needs to be deleted*/
    // TODO: unsupported class... would be possible to use PyGEOS?
    //geos::Geometry* geosGeometry() const;

    
    /* Accessor functions for getting geometry data */
    
    /** return contents of the geometry as a point
        if wkbType is WKBPoint, otherwise returns [0,0] */
    QgsPoint asPoint();
    
    /** return contents of the geometry as a polyline
        if wkbType is WKBLineString, otherwise an empty list */
    QgsPolyline asPolyline();
    
    /** return contents of the geometry as a polygon
        if wkbType is WKBPolygon, otherwise an empty list */
    QgsPolygon asPolygon();
    
    /** return contents of the geometry as a polygon
        if wkbType is WKBPolygon, otherwise an empty list */
    QgsMultiPoint asMultiPoint();
    
    /** return contents of the geometry as a polygon
        if wkbType is WKBPolygon, otherwise an empty list */
    QgsMultiPolyline asMultiPolyline();
    
    /** return contents of the geometry as a polygon
        if wkbType is WKBPolygon, otherwise an empty list */
    QgsMultiPolygon asMultiPolygon();

}; // class QgsGeometry

