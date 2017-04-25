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

    QgsEllipse( const QgsPointV2 &center, const double semiMajorAxis, const double semiMinorAxis, const double azimuth = 90 );
%Docstring
 Constructs an ellipse by defining all the members.
 \param center The center of the ellipse.
 \param semiMajorAxis Semi-major axis of the ellipse.
 \param semiMinorAxis Semi-minor axis of the ellipse.
 \param azimuth Angle in degrees started from the North to the first quadrant.
%End

    static QgsEllipse fromFoci( const QgsPointV2 &pt1, const QgsPointV2 &pt2, const QgsPointV2 &pt3 );
%Docstring
 Constructs an ellipse by foci (pt1 and pt2) and a point pt3.
 The center point can have z and m values which are the result from the midpoint operation between pt1 and pt2.
 Axes are calculated from the 2D distance with the third point pt3.
 The azimuth is the angle between pt1 and pt2.
 \param pt1 First focus.
 \param pt2 Second focus.
 \param pt3 A point to calculate the axes.
 :rtype: QgsEllipse
%End

    static QgsEllipse fromExtent( const QgsPointV2 &pt1, const QgsPointV2 &pt2 );
%Docstring
 Constructs an ellipse by an extent (aka bounding box / QgsRectangle).
 The center point can have z and m values which are the result from the midpoint operation between pt1 and pt2.
 Axes are calculated from the 2D distance between pt1 and pt2.
 The azimuth always takes the default value.
 \param pt1 First corner.
 \param pt2 Second corner.
 :rtype: QgsEllipse
%End

    static QgsEllipse fromCenterPoint( const QgsPointV2 &ptc, const QgsPointV2 &pt1 );
%Docstring
 Constructs an ellipse by a center point and a another point.
 The center point keeps z and m values from ptc.
 Axes are calculated from the 2D distance between ptc and pt1.
 The azimuth always takes the default value.
 \param ptc Center point.
 \param pt1 First point.
 :rtype: QgsEllipse
%End

    static QgsEllipse fromCenter2Points( const QgsPointV2 &ptc, const QgsPointV2 &pt1, const QgsPointV2 &pt2 );
%Docstring
 Constructs an ellipse by a central point and two other points.
 The center point keeps z and m values from ptc.
 Axes are calculated from the 2D distance between ptc and pt1 and pt2.
 The azimuth is the angle between ptc and pt1.
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

    QgsPointV2 center() const;
%Docstring
 Returns the center point.
 \see setCenter()
 \see rcenter()
 :rtype: QgsPointV2
%End

    double semiMajorAxis() const;
%Docstring
 Returns the semi-major axis.
 \see setSemiMajorAxis()
 :rtype: float
%End

    double semiMinorAxis() const;
%Docstring
 Returns the semi-minor axis.
 \see setSemiMinorAxis()
 :rtype: float
%End

    double azimuth() const;
%Docstring
 Returns the azimuth.
 \see setAzimuth()
 :rtype: float
%End


    void setCenter( const QgsPointV2 &center );
%Docstring
 Sets the center point.
 \see center()
 \see rcenter()
%End

    virtual void setSemiMajorAxis( const double semiMajorAxis );
%Docstring
 Sets the semi-major axis.
 \see semiMajorAxis()
%End

    virtual void setSemiMinorAxis( const double semiMinorAxis );
%Docstring
 Sets the semi-minor axis.
 \see semiMinorAxis()
%End

    void setAzimuth( const double azimuth );
%Docstring
 Sets the azimuth (orientation).
 \see azimuth()
%End

    virtual double focusDistance() const;
%Docstring
 The distance between the center and each foci.
 \see fromFoci()
 \see foci()
 :return: The distance between the center and each foci.
 :rtype: float
%End

    virtual QVector<QgsPointV2> foci() const;
%Docstring
 Two foci of the ellipse. The axes are oriented by the azimuth and are on the semi-major axis.
 \see fromFoci()
 \see focusDistance()
 :return: the two foci.
 :rtype: list of QgsPointV2
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

    virtual QVector<QgsPointV2> quadrant() const;
%Docstring
 The four quadrants of the ellipse.
 They are oriented and started always from semi-major axis.
 :return: quadrants defined by four points.
 :rtype: list of QgsPointV2
%End

    virtual void points( QgsPointSequence &pts, unsigned int segments = 36 ) const;
%Docstring
 Returns a list of points into pts, with segmentation from segments.
 \param pts List of points returned.
 \param segments Number of segments used to segment geometry.
%End

    virtual QgsPolygonV2 *toPolygon( unsigned int segments = 36 ) const;
%Docstring
 Returns a segmented polygon.
 \param segments Number of segments used to segment geometry.
 :rtype: QgsPolygonV2
%End

    virtual QgsLineString *toLineString( unsigned int segments = 36 ) const;
%Docstring
 Returns a segmented linestring.
 \param segments Number of segments used to segment geometry.
 :rtype: QgsLineString
%End

    virtual QgsPolygonV2 *orientedBoundingBox() const;
%Docstring
 Returns the oriented minimal bounding box for the ellipse.
 :rtype: QgsPolygonV2
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
