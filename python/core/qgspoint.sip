/** \ingroup core
 * A class to represent a vector.
 * Currently no Z axis / 2.5D support is implemented.
 */

class QgsVector
{
%TypeHeaderCode
#include <qgspoint.h>
%End

  public:

    /** Default constructor for QgsVector. Creates a vector with length of 0.0.
     */
    QgsVector();

    /** Constructor for QgsVector taking x and y component values.
     * @param x x-component
     * @param y y-component
     */
    QgsVector( double x, double y );

    //! Swaps the sign of the x and y components of the vector.
    QgsVector operator-() const;

    /** Returns a vector where the components have been multiplied by a scalar value.
     * @param scalar factor to multiply by
     */
    QgsVector operator*( double scalar ) const;

    /** Returns a vector where the components have been divided by a scalar value.
     * @param scalar factor to divide by
     */
    QgsVector operator/( double scalar ) const;

    /** Returns the sum of the x component of this vector multiplied by the x component of another
     * vector plus the y component of this vector multipled by the y component of another vector.
     */
    double operator*( QgsVector v ) const;

    /** Returns the length of the vector.
     */
    double length() const;

    /** Returns the vector's x-component.
     * @see y()
     */
    double x() const;

    /** Returns the vector's y-component.
     * @see x()
     */
    double y() const;

    /** Returns the perpendicular vector to this vector (rotated 90 degrees counter-clockwise)
     */
    QgsVector perpVector() const;

    /** Returns the angle of the vector in radians.
     */
    double angle() const;

    /** Returns the angle between this vector and another vector in radians.
     */
    double angle( QgsVector v ) const;

    /** Rotates the vector by a specified angle.
     * @param rot angle in radians
     */
    QgsVector rotateBy( double rot ) const;

    /** Returns the vector's normalized (or "unit") vector (ie same angle but length of 1.0). Will throw an expection
     * if called on a vector with length of 0.
     * @deprecated use normalized() instead
     */
    QgsVector normal() const /Deprecated/;

    /** Returns the vector's normalized (or "unit") vector (ie same angle but length of 1.0). Will throw an expection
     * if called on a vector with length of 0.
     */
    QgsVector normalized() const;
};


/** \ingroup core
 * A class to represent a point.
 * Currently no Z axis / 2.5D support is implemented.
 */
class QgsPoint
{

%TypeHeaderCode
#include <qgspoint.h>
#include <QString>
%End

  public:
    /// Default constructor
    QgsPoint();

    /** Create a point from another point */
    QgsPoint( const QgsPoint& p );

    /** Create a point from x,y coordinates
     * @param x x coordinate
     * @param y y coordinate
     */
    QgsPoint( double x, double y );

    /** Create a point from a QPointF
     * @param point QPointF source
     * @note added in QGIS 2.7
     */
    QgsPoint( QPointF point );

    /** Create a point from a QPoint
     * @param point QPoint source
     * @note added in QGIS 2.7
     */
    QgsPoint( QPoint point );

    ~QgsPoint();

    /** Sets the x value of the point
     * @param x x coordinate
     */
    void setX( double x );

    /** Sets the y value of the point
     * @param y y coordinate
     */
    void setY( double y );

    /** Sets the x and y value of the point */
    void set( double x, double y );

    /** Get the x value of the point
     * @return x coordinate
     */
    double x() const;

    /** Get the y value of the point
     * @return y coordinate
     */
    double y() const;

    /** Converts a point to a QPointF
     * @returns QPointF with same x and y values
     * @note added in QGIS 2.7
     */
    QPointF toQPointF() const;

    //! String representation of the point (x,y)
    QString toString() const;

    //! As above but with precision for string representation of a point
    QString toString( int thePrecision ) const;

    /** Return a string representation as degrees minutes seconds.
     *  Its up to the calling function to ensure that this point can
     *  be meaningfully represented in this form.
     *  @param thePrecision number of decimal points to use for seconds
     *  @param useSuffix set to true to include a direction suffix (eg 'N'),
     *  set to false to use a "-" prefix for west and south coordinates
     *  @param padded set to true to force minutes and seconds to use two decimals,
     *  eg, '05' instead of '5'.
     */
    QString toDegreesMinutesSeconds( int thePrecision, const bool useSuffix = true, const bool padded = false ) const;

    /** Return a string representation as degrees minutes.
     *  Its up to the calling function to ensure that this point can
     *  be meaningfully represented in this form.
     *  @param thePrecision number of decimal points to use for minutes
     *  @param useSuffix set to true to include a direction suffix (eg 'N'),
     *  set to false to use a "-" prefix for west and south coordinates
     *  @param padded set to true to force minutes to use two decimals,
     *  eg, '05' instead of '5'.
     */
    QString toDegreesMinutes( int thePrecision, const bool useSuffix = true, const bool padded = false ) const;


    /** Return the well known text representation for the point.
     * The wkt is created without an SRID.
     * @return Well known text in the form POINT(x y)
     */
    QString wellKnownText() const;

    /** Returns the squared distance between this point a specified x, y coordinate.
     * @see distance()
    */
    double sqrDist( double x, double y ) const;

    /** Returns the squared distance between this point another point.
     * @see distance()
    */
    double sqrDist( const QgsPoint& other ) const;

    /** Returns the distance between this point and a specified x, y coordinate.
     * @param x x-coordniate
     * @param y y-coordinate
     * @see sqrDist()
     * @note added in QGIS 2.16
    */
    double distance( double x, double y ) const;

    /** Returns the distance between this point and another point.
     * @param other other point
     * @see sqrDist()
     * @note added in QGIS 2.16
    */
    double distance( const QgsPoint& other ) const;

    /** Returns the minimum distance between this point and a segment */
    double sqrDistToSegment( double x1, double y1, double x2, double y2, QgsPoint& minDistPoint /Out/, double epsilon = DEFAULT_SEGMENT_EPSILON ) const;

    /** Calculates azimuth between this point and other one (clockwise in degree, starting from north) */
    double azimuth( const QgsPoint& other ) const;

    /** Returns a new point which correponds to this point projected by a specified distance
     * in a specified bearing.
     * @param distance distance to project
     * @param bearing angle to project in, clockwise in degrees starting from north
     * @note added in QGIS 2.16
     */
    QgsPoint project( double distance, double bearing ) const;

    /** Compares this point with another point with a fuzzy tolerance
     * @param other point to compare with
     * @param epsilon maximum difference for coordinates between the points
     * @returns true if points are equal within specified tolerance
     * @note added in QGIS 2.9
     */
    bool compare( const QgsPoint &other, double epsilon = 4 * DBL_EPSILON ) const;

    //! equality operator
    bool operator==( const QgsPoint &other );

    //! Inequality operator
    bool operator!=( const QgsPoint &other ) const;

    //! Multiply x and y by the given value
    void multiply( double scalar );

    //! Test if this point is on the segment defined by points a, b
    //! @return 0 if this point is not on the open ray through a and b,
    //! 1 if point is on open ray a, 2 if point is within line segment,
    //! 3 if point is on open ray b.
    int onSegment( const QgsPoint& a, const QgsPoint& b ) const;

    //! Calculates the vector obtained by subtracting a point from this point
    QgsVector operator-( const QgsPoint& p ) const;

    //! Adds a vector to this point in place
    QgsPoint &operator+=( QgsVector v );

    //! Subtracts a vector from this point in place
    QgsPoint &operator-=( QgsVector v );

    //! Adds a vector to this point
    QgsPoint operator+( QgsVector v ) const;

    //! Subtracts a vector from this point
    QgsPoint operator-( QgsVector v ) const;

    //! Multiplies the coordinates in this point by a scalar quantity
    QgsPoint operator*( double scalar ) const;

    //! Divides the coordinates in this point by a scalar quantity
    QgsPoint operator/( double scalar ) const;

    //! Multiplies the coordinates in this point by a scalar quantity in place
    QgsPoint &operator*=( double scalar );

    //! Divides the coordinates in this point by a scalar quantity in place
    QgsPoint &operator/=( double scalar );

  SIP_PYOBJECT __repr__();
%MethodCode
    QString str = "(" + QString::number(sipCpp->x()) + "," + QString::number(sipCpp->y()) + ")";
    //QString str("(%f,%f)").arg(sipCpp->x()).arg(sipCpp->y());
    sipRes = PyUnicode_FromString( str.toUtf8().data() );
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

  long __hash__() const;
%MethodCode
  sipRes = qHash( *sipCpp );
%End
}; // class QgsPoint
