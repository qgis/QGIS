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
                                     const QgsCoordinateReferenceSystem &destination );
%Docstring
 Constructs a QgsCoordinateTransform using QgsCoordinateReferenceSystem objects.
 \param source source CRS, typically of the layer's coordinate system
 \param destination CRS, typically of the map canvas coordinate system
%End

    explicit QgsCoordinateTransform( const QgsCoordinateReferenceSystem &source,
                                     const QgsCoordinateReferenceSystem &destination,
                                     const QgsCoordinateTransformContext &context );
%Docstring
 Constructs a QgsCoordinateTransform to transform from the ``source``
 to ``destination`` coordinate reference system.

 The ``context`` argument specifies the context under which the transform
 will be applied, and is used for calculating necessary datum transforms
 to utilise.

.. versionadded:: 3.0
%End

    explicit QgsCoordinateTransform( const QgsCoordinateReferenceSystem &source,
                                     const QgsCoordinateReferenceSystem &destination,
                                     const QgsProject *project );
%Docstring
 Constructs a QgsCoordinateTransform to transform from the ``source``
 to ``destination`` coordinate reference system, when used with the
 given ``project``.

 No reference to ``project`` is stored or utilised outside of the constructor,
 and it is used to retrieve the project's transform context only.

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
.. versionadded:: 3.0
 :rtype: bool
%End

    void setSourceCrs( const QgsCoordinateReferenceSystem &crs );
%Docstring
 Sets the source coordinate reference system.
 \param crs CRS to transform coordinates from
.. seealso:: :py:func:`sourceCrs()`
.. seealso:: :py:func:`setDestinationCrs()`
%End

    void setDestinationCrs( const QgsCoordinateReferenceSystem &crs );
%Docstring
 Sets the destination coordinate reference system.
 \param crs CRS to transform coordinates to
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
.. seealso:: :py:func:`setSourceCrs()`
.. seealso:: :py:func:`destinationCrs()`
 :rtype: QgsCoordinateReferenceSystem
%End

    QgsCoordinateReferenceSystem destinationCrs() const;
%Docstring
 Returns the destination coordinate reference system, which the transform will
 transform coordinates to.
.. seealso:: :py:func:`setDestinationCrs()`
.. seealso:: :py:func:`sourceCrs()`
 :rtype: QgsCoordinateReferenceSystem
%End

    QgsPointXY transform( const QgsPointXY &point, TransformDirection direction = ForwardTransform ) const;
%Docstring
 Transform the point from the source CRS to the destination CRS.
 If the direction is ForwardTransform then coordinates are transformed from source to destination,
 otherwise points are transformed from destination to source CRS.
 \param point point to transform
 \param direction transform direction (defaults to ForwardTransform)
 :return: transformed point
 :rtype: QgsPointXY
%End

    QgsPointXY transform( const double x, const double y, TransformDirection direction = ForwardTransform ) const;
%Docstring
 Transform the point specified by x,y from the source CRS to the destination CRS.
 If the direction is ForwardTransform then coordinates are transformed from source to destination,
 otherwise points are transformed from destination to source CRS.
 \param x x coordinate of point to transform
 \param y y coordinate of point to transform
 \param direction transform direction (defaults to ForwardTransform)
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
 \param rectangle rectangle to transform
 \param direction transform direction (defaults to ForwardTransform)
 \param handle180Crossover set to true if destination CRS is geographic and handling of extents
 crossing the 180 degree longitude line is required
 :return: rectangle in destination CRS
 :rtype: QgsRectangle
%End

    void transformInPlace( double &x, double &y, double &z, TransformDirection direction = ForwardTransform ) const;
%Docstring
 Transforms an array of x, y and z double coordinates in place, from the source CRS to the destination CRS.
 If the direction is ForwardTransform then coordinates are transformed from source to destination,
 otherwise points are transformed from destination to source CRS.
 \param x array of x coordinates of points to transform
 \param y array of y coordinates of points to transform
 \param z array of z coordinates of points to transform. The z coordinates of the points
 must represent height relative to the vertical datum of the source CRS (generally ellipsoidal
 heights) and must be expressed in its vertical units (generally meters)
 \param direction transform direction (defaults to ForwardTransform)
%End





    void transformPolygon( QPolygonF &polygon, TransformDirection direction = ForwardTransform ) const;
%Docstring
 Transforms a polygon to the destination coordinate system.
 \param polygon polygon to transform (occurs in place)
 \param direction transform direction (defaults to forward transformation)
%End

    QgsRectangle transform( const QgsRectangle &rectangle, TransformDirection direction = ForwardTransform ) const;
%Docstring
 Transforms a rectangle to the destination CRS.
 If the direction is ForwardTransform then coordinates are transformed from source to destination,
 otherwise points are transformed from destination to source CRS.
 \param rectangle rectangle to transform
 \param direction transform direction (defaults to ForwardTransform)
 :return: transformed rectangle
 :rtype: QgsRectangle
%End

    void transformCoords( int numPoint, double *x, double *y, double *z, TransformDirection direction = ForwardTransform ) const;
%Docstring
 Transform an array of coordinates to the destination CRS.
 If the direction is ForwardTransform then coordinates are transformed from source to destination,
 otherwise points are transformed from destination to source CRS.
 \param numPoint number of coordinates in arrays
 \param x array of x coordinates to transform
 \param y array of y coordinates to transform
 \param z array of z coordinates to transform
 \param direction transform direction (defaults to ForwardTransform)
%End

    bool isShortCircuited() const;
%Docstring
 Returns true if the transform short circuits because the source and destination are equivalent.
 :rtype: bool
%End


    static QString datumTransformString( int datumTransform );
%Docstring
 :rtype: str
%End

    static bool datumTransformCrsInfo( int datumTransform, int &epsgNr, QString &srcProjection, QString &dstProjection, QString &remarks, QString &scope, bool &preferred, bool &deprecated );
%Docstring
 Gets name of source and dest geographical CRS (to show in a tooltip)
:return: epsgNr epsg code of the transformation (or 0 if not in epsg db)*
 :rtype: bool
%End

    int sourceDatumTransform() const;
%Docstring
 Returns the index of the datum transform to use when projecting from the source
 CRS.

 This is usually calculated automatically from the transform's QgsCoordinateTransformContext,
 but can be manually overwritten by a call to setSourceDatumTransform().

.. seealso:: setSourceDatumTransform()
.. seealso:: destinationDatumTransform()
 :rtype: int
%End

    void setSourceDatumTransform( int datum );
%Docstring
 Sets the index of the ``datum`` transform to use when projecting from the source
 CRS.

 This is usually calculated automatically from the transform's QgsCoordinateTransformContext.
 Calling this method will overwrite any automatically calculated datum transform.

.. seealso:: sourceDatumTransform()
.. seealso:: setDestinationDatumTransform()
%End

    int destinationDatumTransform() const;
%Docstring
 Returns the index of the datum transform to use when projecting to the destination
 CRS.

 This is usually calculated automatically from the transform's QgsCoordinateTransformContext,
 but can be manually overwritten by a call to setDestinationDatumTransform().

.. seealso:: setDestinationDatumTransform()
.. seealso:: sourceDatumTransform()
 :rtype: int
%End

    void setDestinationDatumTransform( int datum );
%Docstring
 Sets the index of the ``datum`` transform to use when projecting to the destination
 CRS.

 This is usually calculated automatically from the transform's QgsCoordinateTransformContext.
 Calling this method will overwrite any automatically calculated datum transform.

.. seealso:: destinationDatumTransform()
.. seealso:: setSourceDatumTransform()
%End

    void initialize();
%Docstring
initialize is used to actually create the Transformer instance
%End

    bool readXml( const QDomNode &node );
%Docstring
 Restores state from the given Dom node.
 \param node The node from which state will be restored
 :return: bool True on success, False on failure
.. seealso:: :py:func:`writeXml()`
 :rtype: bool
%End

    bool writeXml( QDomNode &node, QDomDocument &document ) const;
%Docstring
 Stores state to the given Dom node in the given document
 \param node The node in which state will be restored
 \param document The document in which state will be stored
 :return: bool True on success, False on failure
.. seealso:: :py:func:`readXml()`
 :rtype: bool
%End

};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgscoordinatetransform.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
