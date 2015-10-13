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

    /*struct wkbEntry
    {
      wkbEntry( const QString& name, bool isMultiType, Type multiType, Type singleType, Type flatType, GeometryType geometryType );
      QString mName;
      bool mIsMultiType;
      Type mMultiType;
      Type mSingleType;
      Type mFlatType;
      GeometryType mGeometryType;
    };*/

    static Type singleType( Type type );
    static Type multiType( Type type );
    static Type flatType( Type type );
    static Type parseType( const QString& wktStr );
    static bool isSingleType( Type type );
    static bool isMultiType( Type type );
    static int wkbDimensions( Type type );
    static GeometryType geometryType( Type type );
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
     * @see hasZ()
     */
    static Type addZ( Type type );

    /** Adds the m dimension to a WKB type and returns the new type
     * @param type original type
     * @note added in QGIS 2.12
     * @see addZ()
     * @see hasM()
     */
    static Type addM( Type type );
};
