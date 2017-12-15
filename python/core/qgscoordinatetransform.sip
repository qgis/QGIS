/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgscoordinatetransform.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsCoordinateTransform
{
%Docstring
Class for doing transforms between two map coordinate systems.

This class can convert map coordinates to a different coordinate reference system.
It is normally associated with a map layer and is used to transform between the
layer's coordinate system and the coordinate system of the map canvas, although
it can be used in a more general sense to transform coordinates.

When used to transform between a layer and the map canvas, all references to source
and destination coordinate systems refer to layer and map canvas respectively. All
operations are from the perspective of the layer. For example, a forward transformation
transforms coordinates from the layer's coordinate system to the map canvas.

.. note::

   Since QGIS 3.0 QgsCoordinateReferenceSystem objects are implicitly shared.
%End

%TypeHeaderCode
#include "qgscoordinatetransform.h"
%End
  public:

    enum TransformDirection
    {
      ForwardTransform,
      ReverseTransform
    };

    QgsCoordinateTransform();
%Docstring
Default constructor, creates an invalid QgsCoordinateTransform.
%End


    explicit QgsCoordinateTransform( const QgsCoordinateReferenceSystem &source,
                                     const QgsCoordinateReferenceSystem &destination,
                                     const QgsCoordinateTransformContext &context );
%Docstring
Constructs a QgsCoordinateTransform to transform from the ``source``
to ``destination`` coordinate reference system.

The ``context`` argument specifies the context under which the transform
will be applied, and is used for calculating necessary datum transforms
to utilize.

.. versionadded:: 3.0
%End

    explicit QgsCoordinateTransform( const QgsCoordinateReferenceSystem &source,
                                     const QgsCoordinateReferenceSystem &destination,
                                     const QgsProject *project );
%Docstring
Constructs a QgsCoordinateTransform to transform from the ``source``
to ``destination`` coordinate reference system, when used with the
given ``project``.

No reference to ``project`` is stored or utilized outside of the constructor,
and it is used to retrieve the project's transform context only.

.. versionadded:: 3.0
%End

    explicit QgsCoordinateTransform( const QgsCoordinateReferenceSystem &source,
                                     const QgsCoordinateReferenceSystem &destination,
                                     int sourceDatumTransformId,
                                     int destinationDatumTransformId );
%Docstring
Constructs a QgsCoordinateTransform to transform from the ``source``
to ``destination`` coordinate reference system, with the specified
datum transforms.

.. versionadded:: 3.0
%End

    QgsCoordinateTransform( const QgsCoordinateTransform &o );
%Docstring
Copy constructor
%End


    ~QgsCoordinateTransform();
%Docstring
Assignment operator
%End

    bool isValid() const;
%Docstring
Returns true if the coordinate transform is valid, ie both the source and destination
CRS have been set and are valid.

:rtype: bool

.. versionadded:: 3.0
%End

    void setSourceCrs( const QgsCoordinateReferenceSystem &crs );
%Docstring
Sets the source coordinate reference system.
:param crs: CRS to transform coordinates from

.. seealso:: :py:func:`sourceCrs()`

.. seealso:: :py:func:`setDestinationCrs()`
%End

    void setDestinationCrs( const QgsCoordinateReferenceSystem &crs );
%Docstring
Sets the destination coordinate reference system.
:param crs: CRS to transform coordinates to

.. seealso:: :py:func:`destinationCrs()`

.. seealso:: :py:func:`setSourceCrs()`
%End

    void setContext( const QgsCoordinateTransformContext &context );
%Docstring
Sets the ``context`` in which the coordinate transform should be
calculated.

.. versionadded:: 3.0
%End

    QgsCoordinateReferenceSystem sourceCrs() const;
%Docstring
Returns the source coordinate reference system, which the transform will
transform coordinates from.

:rtype: QgsCoordinateReferenceSystem

.. seealso:: :py:func:`setSourceCrs()`

.. seealso:: :py:func:`destinationCrs()`
%End

    QgsCoordinateReferenceSystem destinationCrs() const;
%Docstring
Returns the destination coordinate reference system, which the transform will
transform coordinates to.

:rtype: QgsCoordinateReferenceSystem

.. seealso:: :py:func:`setDestinationCrs()`

.. seealso:: :py:func:`sourceCrs()`
%End

    QgsPointXY transform( const QgsPointXY &point, TransformDirection direction = ForwardTransform ) const;
%Docstring
Transform the point from the source CRS to the destination CRS.
If the direction is ForwardTransform then coordinates are transformed from source to destination,
otherwise points are transformed from destination to source CRS.
:param point: point to transform
:param direction: transform direction (defaults to ForwardTransform)

:return: transformed point
:rtype: QgsPointXY

%End

    QgsPointXY transform( const double x, const double y, TransformDirection direction = ForwardTransform ) const;
%Docstring
Transform the point specified by x,y from the source CRS to the destination CRS.
If the direction is ForwardTransform then coordinates are transformed from source to destination,
otherwise points are transformed from destination to source CRS.
:param x: x coordinate of point to transform
:param y: y coordinate of point to transform
:param direction: transform direction (defaults to ForwardTransform)

:return: transformed point
:rtype: QgsPointXY

%End

    QgsRectangle transformBoundingBox( const QgsRectangle &rectangle, TransformDirection direction = ForwardTransform, const bool handle180Crossover = false ) const;
%Docstring
Transforms a rectangle from the source CRS to the destination CRS.
If the direction is ForwardTransform then coordinates are transformed from source to destination,
otherwise points are transformed from destination to source CRS.
This method assumes that the rectangle is a bounding box, and creates a bounding box
in the projected CRS, such that all points from the source rectangle are within
the returned rectangle.
:param rectangle: rectangle to transform
:param direction: transform direction (defaults to ForwardTransform)
:param handle180Crossover: set to true if destination CRS is geographic and handling of extents
crossing the 180 degree longitude line is required

:return: rectangle in destination CRS
:rtype: QgsRectangle

%End

    void transformInPlace( double &x, double &y, double &z, TransformDirection direction = ForwardTransform ) const;
%Docstring
Transforms an array of x, y and z double coordinates in place, from the source CRS to the destination CRS.
If the direction is ForwardTransform then coordinates are transformed from source to destination,
otherwise points are transformed from destination to source CRS.
:param x: array of x coordinates of points to transform
:param y: array of y coordinates of points to transform
:param z: array of z coordinates of points to transform. The z coordinates of the points
must represent height relative to the vertical datum of the source CRS (generally ellipsoidal
heights) and must be expressed in its vertical units (generally meters)
:param direction: transform direction (defaults to ForwardTransform)
%End





    void transformPolygon( QPolygonF &polygon, TransformDirection direction = ForwardTransform ) const;
%Docstring
Transforms a polygon to the destination coordinate system.
:param polygon: polygon to transform (occurs in place)
:param direction: transform direction (defaults to forward transformation)
%End

    QgsRectangle transform( const QgsRectangle &rectangle, TransformDirection direction = ForwardTransform ) const;
%Docstring
Transforms a rectangle to the destination CRS.
If the direction is ForwardTransform then coordinates are transformed from source to destination,
otherwise points are transformed from destination to source CRS.
:param rectangle: rectangle to transform
:param direction: transform direction (defaults to ForwardTransform)

:return: transformed rectangle
:rtype: QgsRectangle

%End

    void transformCoords( int numPoint, double *x, double *y, double *z, TransformDirection direction = ForwardTransform ) const;
%Docstring
Transform an array of coordinates to the destination CRS.
If the direction is ForwardTransform then coordinates are transformed from source to destination,
otherwise points are transformed from destination to source CRS.
:param numPoint: number of coordinates in arrays
:param x: array of x coordinates to transform
:param y: array of y coordinates to transform
:param z: array of z coordinates to transform
:param direction: transform direction (defaults to ForwardTransform)
%End

    bool isShortCircuited() const;
%Docstring
Returns true if the transform short circuits because the source and destination are equivalent.

:rtype: bool
%End

    struct TransformPair
    {

      TransformPair( int sourceTransformId = -1, int destinationTransformId = -1 );
%Docstring
Constructor for a TransformPair with the specified ``sourceTransformId``
and ``destinationTransformId`` transforms.
%End

      int sourceTransformId;
%Docstring
ID for the datum transform to use when projecting from the source CRS.

.. seealso:: :py:func:`QgsCoordinateTransform.datumTransformCrsInfo()`
%End

      int destinationTransformId;
%Docstring
ID for the datum transform to use when projecting to the destination CRS.

.. seealso:: :py:func:`QgsCoordinateTransform.datumTransformCrsInfo()`
%End

      bool operator==( const QgsCoordinateTransform::TransformPair &other ) const;

      bool operator!=( const QgsCoordinateTransform::TransformPair &other ) const;
%Docstring

:rtype: bool
%End

    };

    static QList< QgsCoordinateTransform::TransformPair > datumTransformations( const QgsCoordinateReferenceSystem &source, const QgsCoordinateReferenceSystem &destination );
%Docstring
Returns a list of datum transformations which are available for the given ``source`` and ``destination`` CRS.

:rtype: list of QgsCoordinateTransform.TransformPair

.. seealso:: :py:func:`datumTransformToProj()`

.. seealso:: :py:func:`datumTransformInfo()`
%End

    static QString datumTransformToProj( int datumTransformId );
%Docstring
Returns a proj string representing the specified ``datumTransformId`` datum transform ID.

:rtype: str

.. seealso:: :py:func:`datumTransformations()`

.. seealso:: :py:func:`datumTransformInfo()`

.. seealso:: :py:func:`projStringToDatumTransformId()`
%End

    static int projStringToDatumTransformId( const QString &string );
%Docstring
Returns the datum transform ID corresponding to a specified proj ``string``.
Returns -1 if matching datum ID was not found.

:rtype: int

.. seealso:: :py:func:`datumTransformToProj()`

.. versionadded:: 3.0
%End

    struct TransformInfo
    {
      int datumTransformId;
%Docstring
Datum transform ID
%End

      int epsgCode;
%Docstring
EPSG code for the transform, or 0 if not found in EPSG database
%End

      QString sourceCrsAuthId;
%Docstring
Source CRS auth ID
%End

      QString destinationCrsAuthId;
%Docstring
Destination CRS auth ID
%End

      QString sourceCrsDescription;
%Docstring
Source CRS description
%End

      QString destinationCrsDescription;
%Docstring
Destination CRS description
%End

      QString remarks;
%Docstring
Transform remarks
%End

      QString scope;
%Docstring
Scope of transform
%End

      bool preferred;
%Docstring
True if transform is the preferred transform to use for the source/destination CRS combination
%End

      bool deprecated;
%Docstring
True if transform is deprecated
%End

    };

    static QgsCoordinateTransform::TransformInfo datumTransformInfo( int datumTransformId );
%Docstring
Returns detailed information about the specified ``datumTransformId``.
If ``datumTransformId`` was not a valid transform ID, a TransformInfo with TransformInfo.datumTransformId of
-1 will be returned.

:rtype: QgsCoordinateTransform.TransformInfo

.. seealso:: :py:func:`datumTransformations()`

.. seealso:: :py:func:`datumTransformToProj()`
%End

    int sourceDatumTransformId() const;
%Docstring
Returns the ID of the datum transform to use when projecting from the source
CRS.

This is usually calculated automatically from the transform's QgsCoordinateTransformContext,
but can be manually overwritten by a call to setSourceDatumTransformId().

:rtype: int

.. seealso:: :py:func:`setSourceDatumTransformId()`

.. seealso:: :py:func:`destinationDatumTransformId()`

.. seealso:: :py:func:`datumTransformInfo()`
%End

    void setSourceDatumTransformId( int datumId );
%Docstring
Sets the ``datumId`` ID of the datum transform to use when projecting from the source
CRS.

This is usually calculated automatically from the transform's QgsCoordinateTransformContext.
Calling this method will overwrite any automatically calculated datum transform.

.. seealso:: :py:func:`sourceDatumTransformId()`

.. seealso:: :py:func:`setDestinationDatumTransformId()`

.. seealso:: :py:func:`datumTransformInfo()`
%End

    int destinationDatumTransformId() const;
%Docstring
Returns the ID of the datum transform to use when projecting to the destination
CRS.

This is usually calculated automatically from the transform's QgsCoordinateTransformContext,
but can be manually overwritten by a call to setDestinationDatumTransformId().

:rtype: int

.. seealso:: :py:func:`setDestinationDatumTransformId()`

.. seealso:: :py:func:`sourceDatumTransformId()`

.. seealso:: :py:func:`datumTransformInfo()`
%End

    void setDestinationDatumTransformId( int datumId );
%Docstring
Sets the ``datumId`` ID of the datum transform to use when projecting to the destination
CRS.

This is usually calculated automatically from the transform's QgsCoordinateTransformContext.
Calling this method will overwrite any automatically calculated datum transform.

.. seealso:: :py:func:`destinationDatumTransformId()`

.. seealso:: :py:func:`setSourceDatumTransformId()`

.. seealso:: :py:func:`datumTransformInfo()`
%End

    static void invalidateCache();
%Docstring
Clears the internal cache used to initialize QgsCoordinateTransform objects.
This should be called whenever the srs database has
been modified in order to ensure that outdated CRS transforms are not created.

.. versionadded:: 3.0
%End

};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgscoordinatetransform.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
