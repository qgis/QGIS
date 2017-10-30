/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgsellipse.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsEllipse
{
%Docstring
 Ellipse geometry type.

 An ellipse is defined by a center point with a semi-major axis, a semi-minor axis and an azimuth.
 The azimuth is the north angle to the first quadrant (always oriented on the semi-major axis), in degrees. By default, the semi-major axis is oriented to the east (90 degrees).
 The semi-minor axis is always smaller than the semi-major axis. If it is set larger, it will be swapped and the azimuth will increase by 90 degrees.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsellipse.h"
%End
  public:

    QgsEllipse();
%Docstring
 Constructor for QgsEllipse.
%End

    virtual ~QgsEllipse();

    QgsEllipse( const QgsPoint &center, const double semiMajorAxis, const double semiMinorAxis, const double azimuth = 90 );
%Docstring
 Constructs an ellipse by defining all the members.
 \param center The center of the ellipse.
 \param semiMajorAxis Semi-major axis of the ellipse.
 \param semiMinorAxis Semi-minor axis of the ellipse.
 \param azimuth Angle in degrees started from the North to the first quadrant.
%End

    static QgsEllipse fromFoci( const QgsPoint &pt1, const QgsPoint &pt2, const QgsPoint &pt3 );
%Docstring
 Constructs an ellipse by foci (``pt1`` and ``pt2``) and a point ``pt3``.
 The center point can have z and m values which are the result from the midpoint operation between ``pt1`` and ``pt2``.
 Axes are calculated from the 2D distance with the third point ``pt3``.
 The azimuth is the angle between ``pt1`` and ``pt2``.
 \param pt1 First focus.
 \param pt2 Second focus.
 \param pt3 A point to calculate the axes.
 :rtype: QgsEllipse
%End

    static QgsEllipse fromExtent( const QgsPoint &pt1, const QgsPoint &pt2 );
%Docstring
 Constructs an ellipse by an extent (aka bounding box / QgsRectangle).
 The center point can have z and m values which are the result from the midpoint operation between ``pt1`` and ``pt2``.
 Axes are calculated from the 2D distance between ``pt1`` and ``pt2``.
 The azimuth always takes the default value.
 \param pt1 First corner.
 \param pt2 Second corner.
 :rtype: QgsEllipse
%End

    static QgsEllipse fromCenterPoint( const QgsPoint &ptc, const QgsPoint &pt1 );
%Docstring
 Constructs an ellipse by a center point and a another point.
 The center point keeps z and m values from ``ptc``.
 Axes are calculated from the 2D distance between ``ptc`` and ``pt1``.
 The azimuth always takes the default value.
 \param ptc Center point.
 \param pt1 First point.
 :rtype: QgsEllipse
%End

    static QgsEllipse fromCenter2Points( const QgsPoint &ptc, const QgsPoint &pt1, const QgsPoint &pt2 );
%Docstring
 Constructs an ellipse by a central point and two other points.
 The center point keeps z and m values from ``ptc``.
 Axes are calculated from the 2D distance between ``ptc`` and ``pt1`` and ``pt2``.
 The azimuth is the angle between ``ptc`` and ``pt1``.
 \param ptc Center point.
 \param pt1 First point.
 \param pt2 Second point.
 :rtype: QgsEllipse
%End

    virtual bool operator ==( const QgsEllipse &elp ) const;
    virtual bool operator !=( const QgsEllipse &elp ) const;

    virtual bool isEmpty() const;
%Docstring
An ellipse is empty if axes are equal to 0
 :rtype: bool
%End

    QgsPoint center() const;
%Docstring
 Returns the center point.
.. seealso:: setCenter()
.. seealso:: rcenter()
 :rtype: QgsPoint
%End

    double semiMajorAxis() const;
%Docstring
 Returns the semi-major axis.
.. seealso:: setSemiMajorAxis()
 :rtype: float
%End

    double semiMinorAxis() const;
%Docstring
 Returns the semi-minor axis.
.. seealso:: setSemiMinorAxis()
 :rtype: float
%End

    double azimuth() const;
%Docstring
 Returns the azimuth.
.. seealso:: setAzimuth()
 :rtype: float
%End


    void setCenter( const QgsPoint &center );
%Docstring
 Sets the center point.
.. seealso:: center()
.. seealso:: rcenter()
%End

    virtual void setSemiMajorAxis( const double semiMajorAxis );
%Docstring
 Sets the semi-major axis.
.. seealso:: semiMajorAxis()
%End

    virtual void setSemiMinorAxis( const double semiMinorAxis );
%Docstring
 Sets the semi-minor axis.
.. seealso:: semiMinorAxis()
%End

    void setAzimuth( const double azimuth );
%Docstring
 Sets the azimuth (orientation).
.. seealso:: azimuth()
%End

    virtual double focusDistance() const;
%Docstring
 The distance between the center and each foci.
.. seealso:: fromFoci()
.. seealso:: foci()
 :return: The distance between the center and each foci.
 :rtype: float
%End

    virtual QVector<QgsPoint> foci() const;
%Docstring
 Two foci of the ellipse. The axes are oriented by the azimuth and are on the semi-major axis.
.. seealso:: fromFoci()
.. seealso:: focusDistance()
 :return: the two foci.
 :rtype: list of QgsPoint
%End

    virtual double eccentricity() const;
%Docstring
 The eccentricity of the ellipse.
 nan is returned if the ellipse is empty.
 :rtype: float
%End
    virtual double area() const;
%Docstring
The area of the ellipse.
 :rtype: float
%End
    virtual double perimeter() const;
%Docstring
The circumference of the ellipse using first approximation of Ramanujan.
 :rtype: float
%End

    virtual QVector<QgsPoint> quadrant() const;
%Docstring
 The four quadrants of the ellipse.
 They are oriented and started always from semi-major axis.
 :return: quadrants defined by four points.
 :rtype: list of QgsPoint
%End

    virtual QgsPointSequence points( unsigned int segments = 36 ) const;
%Docstring
 Returns a list of points with segmentation from ``segments``.
 \param segments Number of segments used to segment geometry.
 :rtype: QgsPointSequence
%End

    virtual QgsPolygon *toPolygon( unsigned int segments = 36 ) const /Factory/;
%Docstring
 Returns a segmented polygon.
 \param segments Number of segments used to segment geometry.
 :rtype: QgsPolygon
%End

    virtual QgsLineString *toLineString( unsigned int segments = 36 ) const /Factory/;
%Docstring
 Returns a segmented linestring.
 \param segments Number of segments used to segment geometry.
 :rtype: QgsLineString
%End

    virtual QgsPolygon *orientedBoundingBox() const /Factory/;
%Docstring
 Returns the oriented minimal bounding box for the ellipse.
 :rtype: QgsPolygon
%End

    virtual QgsRectangle boundingBox() const;
%Docstring
 Returns the minimal bounding box for the ellipse.
 :rtype: QgsRectangle
%End

    virtual QString toString( int pointPrecision = 17, int axisPrecision = 17, int azimuthPrecision = 2 ) const;
%Docstring
 returns a string representation of the ellipse.
 Members will be truncated to the specified precision.
 :rtype: str
%End

  protected:

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgsellipse.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
