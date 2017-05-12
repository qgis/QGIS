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
.. seealso:: isSingleType()
.. seealso:: multiType()
.. seealso:: flatType()
 :rtype: Type
%End

    static Type multiType( Type type );
%Docstring
 Returns the multi type for a WKB type. For example, for Polygon WKB types the multi type would be MultiPolygon.
.. seealso:: isMultiType()
.. seealso:: singleType()
.. seealso:: flatType()
 :rtype: Type
%End

    static Type flatType( Type type );
%Docstring
 Returns the flat type for a WKB type. This is the WKB type minus any Z or M dimensions.
 For example, for PolygonZM WKB types the single type would be Polygon.
.. seealso:: singleType()
.. seealso:: multiType()
 :rtype: Type
%End

    static Type zmType( Type type, bool hasZ, bool hasM );
%Docstring
Returns the modified input geometry type according to hasZ / hasM
 :rtype: Type
%End

    static Type parseType( const QString &wktStr );
%Docstring
 Attempts to extract the WKB type from a WKT string.
 \param wktStr a valid WKT string
 :rtype: Type
%End

    static bool isSingleType( Type type );
%Docstring
 Returns true if the WKB type is a single type.
.. seealso:: isMultiType()
.. seealso:: singleType()
 :rtype: bool
%End

    static bool isMultiType( Type type );
%Docstring
 Returns true if the WKB type is a multi type.
.. seealso:: isSingleType()
.. seealso:: multiType()
 :rtype: bool
%End

    static bool isCurvedType( Type type );
%Docstring
 Returns true if the WKB type is a curved type or can contain curved geometries.
.. versionadded:: 2.14
 :rtype: bool
%End

    static int wkbDimensions( Type type );
%Docstring
 Returns the inherent dimension of the geometry type as an integer. Returned value will
 always be less than or equal to the coordinate dimension.
 :return: 0 for point geometries, 1 for line geometries, 2 for polygon geometries
 Invalid geometry types will return a dimension of 0.
.. seealso:: coordDimensions()
 :rtype: int
%End

    static int coordDimensions( Type type );
%Docstring
 Returns the coordinate dimension of the geometry type as an integer. Returned value will
 be between 2-4, depending on whether the geometry type contains the Z or M dimensions.
 Invalid geometry types will return a dimension of 0.
.. versionadded:: 2.14
.. seealso:: wkbDimensions()
 :rtype: int
%End

    static GeometryType geometryType( Type type );
%Docstring
 Returns the geometry type for a WKB type, e.g., both MultiPolygon and CurvePolygon would have a
 PolygonGeometry geometry type.
 GeometryCollections are reported as QgsWkbTypes.UnknownGeometry.
 :rtype: GeometryType
%End

    static QString displayString( Type type );
%Docstring
 Returns a display string type for a WKB type, e.g., the geometry name used in WKT geometry representations.
 :rtype: str
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
 :rtype: str
%End

    static bool hasZ( Type type );
%Docstring
 Tests whether a WKB type contains the z-dimension.
 :return: true if type has z values
.. seealso:: addZ()
.. seealso:: hasM()
 :rtype: bool
%End

    static bool hasM( Type type );
%Docstring
 Tests whether a WKB type contains m values.
 :return: true if type has m values
.. seealso:: addM()
.. seealso:: hasZ()
 :rtype: bool
%End

    static Type addZ( Type type );
%Docstring
 Adds the z dimension to a WKB type and returns the new type
 \param type original type
.. versionadded:: 2.12
.. seealso:: addM()
.. seealso:: dropZ()
.. seealso:: hasZ()
 :rtype: Type
%End

    static Type addM( Type type );
%Docstring
 Adds the m dimension to a WKB type and returns the new type
 \param type original type
.. versionadded:: 2.12
.. seealso:: addZ()
.. seealso:: dropM()
.. seealso:: hasM()
 :rtype: Type
%End

    static Type dropZ( Type type );
%Docstring
 Drops the z dimension (if present) for a WKB type and returns the new type.
 \param type original type
.. versionadded:: 2.14
.. seealso:: dropM()
.. seealso:: addZ()
 :rtype: Type
%End

    static Type dropM( Type type );
%Docstring
 Drops the m dimension (if present) for a WKB type and returns the new type.
 \param type original type
.. versionadded:: 2.14
.. seealso:: dropZ()
.. seealso:: addM()
 :rtype: Type
%End

    static Type to25D( Type type );
%Docstring
 Will convert the 25D version of the flat type if supported or Unknown if not supported.
 \param type The type to convert
 :return: the 25D version of the type or Unknown
 :rtype: Type
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geometry/qgswkbtypes.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
