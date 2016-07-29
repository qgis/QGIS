class QgsWKBTypes
{
%TypeHeaderCode
#include <qgswkbtypes.h>
%End

  public:

    enum Type
    {
      Unknown,
      Point,
      LineString,
      Polygon,
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

    /** Returns the single type for a WKB type. Eg, for MultiPolygon WKB types the single type would be Polygon.
     * @see isSingleType()
     * @see multiType()
     * @see flatType()
     */
    static Type singleType( Type type );

    /** Returns the multi type for a WKB type. Eg, for Polygon WKB types the multi type would be MultiPolygon.
     * @see isMultiType()
     * @see singleType()
     * @see flatType()
     */
    static Type multiType( Type type );

    /** Returns the flat type for a WKB type. This is the WKB type minus any Z or M dimensions.
     * Eg, for PolygonZM WKB types the single type would be Polygon.
     * @see singleType()
     * @see multiType()
     */
    static Type flatType( Type type );

    /** Returns the modified input geometry type according to hasZ / hasM */
    static Type zmType( Type type, bool hasZ, bool hasM );

    /** Attempts to extract the WKB type from a WKT string.
     * @param wktStr a valid WKT string
     */
    static Type parseType( const QString& wktStr );

    /** Returns true if the WKB type is a single type.
     * @see isMultiType()
     * @see singleType()
     */
    static bool isSingleType( Type type );

    /** Returns true if the WKB type is a multi type.
     * @see isSingleType()
     * @see multiType()
     */
    static bool isMultiType( Type type );

    /** Returns true if the WKB type is a curved type or can contain curved geometries.
     * @note added in QGIS 2.14
     */
    static bool isCurvedType( Type type );

    /** Returns the inherent dimension of the geometry type as an integer. Returned value will
     * always be less than or equal to the coordinate dimension.
     * @returns 0 for point geometries, 1 for line geometries, 2 for polygon geometries
     * Invalid geometry types will return a dimension of 0.
     * @see coordDimensions()
     */
    static int wkbDimensions( Type type );

    /** Returns the coordinate dimension of the geometry type as an integer. Returned value will
     * be between 2-4, depending on whether the geometry type contains the Z or M dimensions.
     * Invalid geometry types will return a dimension of 0.
     * @note added in QGIS 2.14
     * @see wkbDimensions()
     */
    static int coordDimensions( Type type );

    /** Returns the geometry type for a WKB type, eg both MultiPolygon and CurvePolygon would have a
     * PolygonGeometry geometry type.
     */
    static GeometryType geometryType( Type type );

    /** Returns a display string type for a WKB type, eg the geometry name used in WKT geometry representations.
     */
    static QString displayString( Type type );

    /** Tests whether a WKB type contains the z-dimension.
     * @returns true if type has z values
     * @see addZ()
     * @see hasM()
     */
    static bool hasZ( Type type );

    /** Tests whether a WKB type contains m values.
     * @returns true if type has m values
     * @see addM()
     * @see hasZ()
     */
    static bool hasM( Type type );

    /** Adds the z dimension to a WKB type and returns the new type
     * @param type original type
     * @note added in QGIS 2.12
     * @see addM()
     * @see dropZ()
     * @see hasZ()
     */
    static Type addZ( Type type );

    /** Adds the m dimension to a WKB type and returns the new type
     * @param type original type
     * @note added in QGIS 2.12
     * @see addZ()
     * @see dropM()
     * @see hasM()
     */
    static Type addM( Type type );

    /** Drops the z dimension (if present) for a WKB type and returns the new type.
     * @param type original type
     * @note added in QGIS 2.14
     * @see dropM()
     * @see addZ()
     */
    static Type dropZ( Type type );

    /** Drops the m dimension (if present) for a WKB type and returns the new type.
     * @param type original type
     * @note added in QGIS 2.14
     * @see dropZ()
     * @see addM()
     */
    static Type dropM( Type type );

    /**
     * Will convert the 25D version of the flat type if supported or Unknown if not supported.
     * @param type The type to convert
     * @return the 25D version of the type or Unknown
     */
    static Type to25D( Type type );
};
