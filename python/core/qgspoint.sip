

class QgsPoint
{

%TypeHeaderCode
#include <qgspoint.h>
#include <QString>
%End

  public:
    /// Default constructor
    QgsPoint();

    /*! Create a point from another point */
    QgsPoint( const QgsPoint& p );

    /*! Create a point from x,y coordinates
     * @param x x coordinate
     * @param y y coordinate
     */
    QgsPoint( double x, double y );

    ~QgsPoint();

    /*! Sets the x value of the point
     * @param x x coordinate
     */
    void setX( double x );

    /*! Sets the y value of the point
     * @param y y coordinate
     */
    void setY( double y );

    /*! Sets the x and y value of the point */
    void set( double x, double y );

    /*! Get the x value of the point
     * @return x coordinate
     */
    double x() const;

    /*! Get the y value of the point
     * @return y coordinate
     */
    double y() const;

    //! String representation of the point (x,y)
    QString toString() const;

    //! As above but with precision for string representation of a point
    QString toString( int thePrecision ) const;

    /** Return a string representation as degrees minutes seconds.
     *  Its up to the calling function to ensure that this point can
     *  be meaningfully represented in this form.
     *  @note added in QGIS 1.4
     */
    QString toDegreesMinutesSeconds( int thePrecision ) const;

    /** Return a string representation as degrees minutes.
     *  Its up to the calling function to ensure that this point can
     *  be meaningfully represented in this form.
     *  @note added in QGIS 1.9
     */
    QString toDegreesMinutes( int thePrecision ) const;


    /*! Return the well known text representation for the point.
     * The wkt is created without an SRID.
     * @return Well known text in the form POINT(x y)
     */
    QString wellKnownText() const;

    /**Returns the squared distance between this point and x,y*/
    double sqrDist( double x, double y ) const;

    /**Returns the squared distance between this and other point*/
    double sqrDist( const QgsPoint& other ) const;

    /**Returns the minimum distance between this point and a segment
    @note added in QGIS 1.5*/
    double sqrDistToSegment( double x1, double y1, double x2, double y2, QgsPoint& minDistPoint /Out/, double epsilon = DEFAULT_SEGMENT_EPSILON ) const;

    /**Calculates azimut between this point and other one (clockwise in degree, starting from north)
      @note: this function has been added in version 1.7*/
    double azimuth( const QgsPoint& other );

    //! equality operator
    bool operator==( const QgsPoint &other );

    //! Inequality operator
    bool operator!=( const QgsPoint &other ) const;

    //! Multiply x and y by the given value
    void multiply( const double& scalar );

    //! Test if this point is on the segment defined by points a, b
    //! @return 0 if this point is not on the open ray through a and b,
    //! 1 if point is on open ray a, 2 if point is within line segment,
    //! 3 if point is on open ray b.
    int onSegment( const QgsPoint& a, const QgsPoint& b ) const;

  SIP_PYOBJECT __repr__();
%MethodCode
    QString str = "(" + QString::number(sipCpp->x()) + "," + QString::number(sipCpp->y()) + ")";
    //QString str("(%f,%f)").arg(sipCpp->x()).arg(sipCpp->y());
    sipRes = PyString_FromString(str.toLocal8Bit().data());
%End

  int __len__();
%MethodCode
    sipRes = 2;
%End


  SIP_PYOBJECT __getitem__(int);
%MethodCode
    if (a0 == 0) {
        sipRes = Py_BuildValue("d",sipCpp->x());
    } else if (a0 == 1) {
        sipRes = Py_BuildValue("d",sipCpp->y());
    } else {
        QString msg = QString("Bad index: %1").arg(a0);
        PyErr_SetString(PyExc_IndexError, msg.toAscii().constData());
    }
%End
}; // class QgsPoint
