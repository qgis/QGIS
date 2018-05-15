/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgswkbtypes.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/







class QgsWkbTypes
{
%Docstring
 Handles storage of information regarding WKB types and their properties.

.. versionadded:: 2.10
%End

%TypeHeaderCode
#include "qgswkbtypes.h"
%End
  public:

    enum Type
    {
      Unknown,
      Point,
      LineString,
      Polygon,
      Triangle,
      MultiPoint,
      MultiLineString,
      MultiPolygon,
      GeometryCollection,
      CircularString,
      CompoundCurve,
      CurvePolygon,
      MultiCurve,
      MultiSurface,
      NoGeometry,
      PointZ,
      LineStringZ,
      PolygonZ,
      TriangleZ,
      MultiPointZ,
      MultiLineStringZ,
      MultiPolygonZ,
      GeometryCollectionZ,
      CircularStringZ,
      CompoundCurveZ,
      CurvePolygonZ,
      MultiCurveZ,
      MultiSurfaceZ,
      PointM,
      LineStringM,
      PolygonM,
      TriangleM,
      MultiPointM,
      MultiLineStringM,
      MultiPolygonM,
      GeometryCollectionM,
      CircularStringM,
      CompoundCurveM,
      CurvePolygonM,
      MultiCurveM,
      MultiSurfaceM,
      PointZM,
      LineStringZM,
      PolygonZM,
      MultiPointZM,
      MultiLineStringZM,
      MultiPolygonZM,
      GeometryCollectionZM,
      CircularStringZM,
      CompoundCurveZM,
      CurvePolygonZM,
      MultiCurveZM,
      MultiSurfaceZM,
      TriangleZM,
      Point25D,
      LineString25D,
      Polygon25D,
      MultiPoint25D,
      MultiLineString25D,
      MultiPolygon25D
    };

    enum GeometryType
    {
      PointGeometry,
      LineGeometry,
      PolygonGeometry,
      UnknownGeometry,
      NullGeometry
    };

    static Type singleType( Type type );
%Docstring
Returns the single type for a WKB type. For example, for MultiPolygon WKB types the single type would be Polygon.

.. seealso:: :py:func:`isSingleType`

.. seealso:: :py:func:`multiType`

.. seealso:: :py:func:`flatType`
%End

    static Type multiType( Type type );
%Docstring
Returns the multi type for a WKB type. For example, for Polygon WKB types the multi type would be MultiPolygon.

.. seealso:: :py:func:`isMultiType`

.. seealso:: :py:func:`singleType`

.. seealso:: :py:func:`flatType`
%End

    static Type flatType( Type type );
%Docstring
Returns the flat type for a WKB type. This is the WKB type minus any Z or M dimensions.
For example, for PolygonZM WKB types the single type would be Polygon.

.. seealso:: :py:func:`singleType`

.. seealso:: :py:func:`multiType`
%End

    static Type zmType( Type type, bool hasZ, bool hasM );
%Docstring
Returns the modified input geometry type according to hasZ / hasM
%End

    static Type parseType( const QString &wktStr );
%Docstring
Attempts to extract the WKB type from a WKT string.

:param wktStr: a valid WKT string
%End

    static bool isSingleType( Type type );
%Docstring
Returns true if the WKB type is a single type.

.. seealso:: :py:func:`isMultiType`

.. seealso:: :py:func:`singleType`
%End

    static bool isMultiType( Type type );
%Docstring
Returns true if the WKB type is a multi type.

.. seealso:: :py:func:`isSingleType`

.. seealso:: :py:func:`multiType`
%End

    static bool isCurvedType( Type type );
%Docstring
Returns true if the WKB type is a curved type or can contain curved geometries.

.. versionadded:: 2.14
%End

    static int wkbDimensions( Type type );
%Docstring
Returns the inherent dimension of the geometry type as an integer. Returned value will
always be less than or equal to the coordinate dimension.

:return: 0 for point geometries, 1 for line geometries, 2 for polygon geometries
Invalid geometry types will return a dimension of 0.

.. seealso:: :py:func:`coordDimensions`
%End

    static int coordDimensions( Type type );
%Docstring
Returns the coordinate dimension of the geometry type as an integer. Returned value will
be between 2-4, depending on whether the geometry type contains the Z or M dimensions.
Invalid geometry types will return a dimension of 0.

.. versionadded:: 2.14

.. seealso:: :py:func:`wkbDimensions`
%End

    static GeometryType geometryType( Type type );
%Docstring
Returns the geometry type for a WKB type, e.g., both MultiPolygon and CurvePolygon would have a
PolygonGeometry geometry type.
GeometryCollections are reported as QgsWkbTypes.UnknownGeometry.
%End

    static QString displayString( Type type );
%Docstring
Returns a display string type for a WKB type, e.g., the geometry name used in WKT geometry representations.
%End

    static QString geometryDisplayString( GeometryType type );
%Docstring
Return a display string for a geometry type.

This will return one of the following strings:

- Point
- Line
- Polygon
- Unknown Geometry
- No Geometry
- Invalid Geometry

.. versionadded:: 3.0
%End

    static bool hasZ( Type type );
%Docstring
Tests whether a WKB type contains the z-dimension.

:return: true if type has z values

.. seealso:: :py:func:`addZ`

.. seealso:: :py:func:`hasM`
%End

    static bool hasM( Type type );
%Docstring
Tests whether a WKB type contains m values.

:return: true if type has m values

.. seealso:: :py:func:`addM`

.. seealso:: :py:func:`hasZ`
%End

    static Type addZ( Type type );
%Docstring
Adds the z dimension to a WKB type and returns the new type

:param type: original type

.. versionadded:: 2.12

.. seealso:: :py:func:`addM`

.. seealso:: :py:func:`dropZ`

.. seealso:: :py:func:`hasZ`
%End

    static Type addM( Type type );
%Docstring
Adds the m dimension to a WKB type and returns the new type

:param type: original type

.. versionadded:: 2.12

.. seealso:: :py:func:`addZ`

.. seealso:: :py:func:`dropM`

.. seealso:: :py:func:`hasM`
%End

    static Type dropZ( Type type );
%Docstring
Drops the z dimension (if present) for a WKB type and returns the new type.

:param type: original type

.. versionadded:: 2.14

.. seealso:: :py:func:`dropM`

.. seealso:: :py:func:`addZ`
%End

    static Type dropM( Type type );
%Docstring
Drops the m dimension (if present) for a WKB type and returns the new type.

:param type: original type

.. versionadded:: 2.14

.. seealso:: :py:func:`dropZ`

.. seealso:: :py:func:`addM`
%End

    static Type to25D( Type type );
%Docstring
Will convert the 25D version of the flat type if supported or Unknown if not supported.

:param type: The type to convert

:return: the 25D version of the type or Unknown
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgswkbtypes.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
