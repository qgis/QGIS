/***************************************************************************
                          qgis.h - QGIS namespace
                             -------------------
    begin                : Sat Jun 30 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGIS_H
#define QGIS_H


#include <QMetaEnum>
#include <cfloat>
#include <memory>
#include <cmath>

#include "qgis_core.h"
#include "qgis_sip.h"

#ifdef SIP_RUN
% ModuleHeaderCode
#include <qgis.h>
% End

% ModuleCode
int QgisEvent = QEvent::User + 1;
% End
#endif

#ifndef SIP_RUN
// qHash implementation for scoped enum type
// https://gitlab.com/frostasm/programming-knowledge-base/-/snippets/20120
#define QHASH_FOR_CLASS_ENUM(T) \
  inline uint qHash(const T &t, uint seed) { \
    return ::qHash(static_cast<typename std::underlying_type<T>::type>(t), seed); \
  }
#endif

/**
 * \ingroup core
 * \brief The Qgis class provides global constants for use throughout the application.
 */
class CORE_EXPORT Qgis
{
    Q_GADGET
    Q_CLASSINFO( "RegisterEnumClassesUnscoped", "false" )

  public:

    /**
     * Version string.
     *
     * \since QGIS 3.12
     */
    static QString version();

    /**
     * Version number used for comparing versions using the "Check QGIS Version" function
     *
     * \since QGIS 3.12
     */
    static int versionInt();

    /**
     * Release name
     *
     * \since QGIS 3.12
     */
    static QString releaseName();

    //! The development version
    static const char *QGIS_DEV_VERSION;

    /**
     * The development version
     *
     * \since QGIS 3.12
     */
    static QString devVersion();

    // Enumerations
    //

    /**
     * \brief Level for messages
     * This will be used both for message log and message bar in application.
     */
    enum MessageLevel
    {
      Info = 0, //!< Information message
      Warning = 1, //!< Warning message
      Critical = 2, //!< Critical/error message
      Success = 3, //!< Used for reporting a successful operation
      NoLevel = 4, //!< No level
    };
    Q_ENUM( MessageLevel )

    /**
     * Types of layers that can be added to a map
     *
     * \since QGIS 3.30. Prior to 3.30 this was available as QgsMapLayerType.
     */
    enum class LayerType SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsMapLayer, LayerType ) : int
      {
      Vector SIP_MONKEYPATCH_COMPAT_NAME( VectorLayer ), //!< Vector layer
      Raster SIP_MONKEYPATCH_COMPAT_NAME( RasterLayer ), //!< Raster layer
      Plugin SIP_MONKEYPATCH_COMPAT_NAME( PluginLayer ), //!< Plugin based layer
      Mesh SIP_MONKEYPATCH_COMPAT_NAME( MeshLayer ),    //!< Mesh layer. Added in QGIS 3.2
      VectorTile SIP_MONKEYPATCH_COMPAT_NAME( VectorTileLayer ), //!< Vector tile layer. Added in QGIS 3.14
      Annotation SIP_MONKEYPATCH_COMPAT_NAME( AnnotationLayer ), //!< Contains freeform, georeferenced annotations. Added in QGIS 3.16
      PointCloud SIP_MONKEYPATCH_COMPAT_NAME( PointCloudLayer ), //!< Point cloud layer. Added in QGIS 3.18
      Group SIP_MONKEYPATCH_COMPAT_NAME( GroupLayer ), //!< Composite group layer. Added in QGIS 3.24
      TiledScene, //!< Tiled scene layer. Added in QGIS 3.34
    };
    Q_ENUM( LayerType )

    /**
     * Filter for layers
     *
     * \since QGIS 3.34. Prior to 3.34 this was available as QgsMapLayerProxyModel::Filter.
     */
    enum class LayerFilter SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsMapLayerProxyModel, Filter ) : int SIP_ENUM_BASETYPE( IntFlag )
    {
      RasterLayer = 1,
      NoGeometry = 2,
      PointLayer = 4,
      LineLayer = 8,
      PolygonLayer = 16,
      HasGeometry = PointLayer | LineLayer | PolygonLayer,
      VectorLayer = NoGeometry | HasGeometry,
      PluginLayer = 32,
      WritableLayer = 64,
      MeshLayer = 128, //!< QgsMeshLayer \since QGIS 3.6
      VectorTileLayer = 256, //!< QgsVectorTileLayer \since QGIS 3.14
      PointCloudLayer = 512, //!< QgsPointCloudLayer \since QGIS 3.18
      AnnotationLayer = 1024, //!< QgsAnnotationLayer \since QGIS 3.22
      TiledSceneLayer = 2048, //!< QgsTiledSceneLayer \since QGIS 3.34
      All = RasterLayer | VectorLayer | PluginLayer | MeshLayer | VectorTileLayer | PointCloudLayer | AnnotationLayer | TiledSceneLayer,
      SpatialLayer = RasterLayer | HasGeometry | PluginLayer | MeshLayer | VectorTileLayer | PointCloudLayer | AnnotationLayer | TiledSceneLayer //!< \since QGIS 3.24
    };
    Q_DECLARE_FLAGS( LayerFilters, LayerFilter )
    Q_FLAG( LayerFilters )

    /**
     * The WKB type describes the number of dimensions a geometry has
     *
     * - Point
     * - LineString
     * - Polygon
     *
     * as well as the number of dimensions for each individual vertex
     *
     * - X (always)
     * - Y (always)
     * - Z (optional)
     * - M (measurement value, optional)
     *
     * it also has values for multi types, collections, unknown geometry,
     * null geometry, no geometry and curve support.
     *
     * These classes of geometry are often used for data sources to
     * communicate what kind of geometry should be expected for a given
     * geometry field. It is also used for tools or algorithms to decide
     * if they should be available for a given geometry type or act in
     * a different mode.
     *
     * \note Prior to 3.30 this was available as QgsWkbTypes.Type.
     *
     * \since QGIS 3.30
     */
    enum class WkbType SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsWkbTypes, Type ) : quint32
      {
      Unknown =  0, //!< Unknown
      Point = 1, //!< Point
      LineString = 2, //!< LineString
      Polygon = 3, //!< Polygon
      Triangle = 17, //!< Triangle
      MultiPoint = 4, //!< MultiPoint
      MultiLineString = 5, //!< MultiLineString
      MultiPolygon = 6, //!< MultiPolygon
      GeometryCollection = 7, //!< GeometryCollection
      CircularString = 8, //!< CircularString
      CompoundCurve = 9, //!< CompoundCurve
      CurvePolygon = 10, //!< CurvePolygon
      MultiCurve = 11, //!< MultiCurve
      MultiSurface = 12, //!< MultiSurface
      NoGeometry = 100, //!< No geometry
      PointZ = 1001, //!< PointZ
      LineStringZ = 1002, //!< LineStringZ
      PolygonZ = 1003, //!< PolygonZ
      TriangleZ = 1017, //!< TriangleZ
      MultiPointZ = 1004, //!< MultiPointZ
      MultiLineStringZ = 1005, //!< MultiLineStringZ
      MultiPolygonZ = 1006, //!< MultiPolygonZ
      GeometryCollectionZ = 1007, //!< GeometryCollectionZ
      CircularStringZ = 1008, //!< CircularStringZ
      CompoundCurveZ = 1009, //!< CompoundCurveZ
      CurvePolygonZ = 1010, //!< CurvePolygonZ
      MultiCurveZ = 1011, //!< MultiCurveZ
      MultiSurfaceZ = 1012, //!< MultiSurfaceZ
      PointM = 2001, //!< PointM
      LineStringM = 2002, //!< LineStringM
      PolygonM = 2003, //!< PolygonM
      TriangleM = 2017, //!< TriangleM
      MultiPointM = 2004, //!< MultiPointM
      MultiLineStringM = 2005, //!< MultiLineStringM
      MultiPolygonM = 2006, //!< MultiPolygonM
      GeometryCollectionM = 2007, //!< GeometryCollectionM
      CircularStringM = 2008, //!< CircularStringM
      CompoundCurveM = 2009, //!< CompoundCurveM
      CurvePolygonM = 2010, //!< CurvePolygonM
      MultiCurveM = 2011, //!< MultiCurveM
      MultiSurfaceM = 2012, //!< MultiSurfaceM
      PointZM = 3001, //!< PointZM
      LineStringZM = 3002, //!< LineStringZM
      PolygonZM = 3003, //!< PolygonZM
      MultiPointZM = 3004, //!< MultiPointZM
      MultiLineStringZM = 3005, //!< MultiLineStringZM
      MultiPolygonZM = 3006, //!< MultiPolygonZM
      GeometryCollectionZM = 3007, //!< GeometryCollectionZM
      CircularStringZM = 3008, //!< CircularStringZM
      CompoundCurveZM = 3009, //!< CompoundCurveZM
      CurvePolygonZM = 3010, //!< CurvePolygonZM
      MultiCurveZM = 3011, //!< MultiCurveZM
      MultiSurfaceZM = 3012, //!< MultiSurfaceZM
      TriangleZM = 3017, //!< TriangleZM
      Point25D = 0x80000001, //!< Point25D
      LineString25D, //!< LineString25D
      Polygon25D, //!< Polygon25D
      MultiPoint25D, //!< MultiPoint25D
      MultiLineString25D, //!< MultiLineString25D
      MultiPolygon25D //!< MultiPolygon25D
    };
    Q_ENUM( WkbType )

    /**
     * The geometry types are used to group Qgis::WkbType in a
     * coarse way.
     *
     * \note Prior to 3.30 this was available as QgsWkbTypes.GeometryType.
     *
     * \since QGIS 3.30
     */
    enum class GeometryType SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsWkbTypes, GeometryType ) : int
      {
      Point SIP_MONKEYPATCH_COMPAT_NAME( PointGeometry ), //!< Points
      Line SIP_MONKEYPATCH_COMPAT_NAME( LineGeometry ), //!< Lines
      Polygon SIP_MONKEYPATCH_COMPAT_NAME( PolygonGeometry ), //!< Polygons
      Unknown SIP_MONKEYPATCH_COMPAT_NAME( UnknownGeometry ), //!< Unknown types
      Null SIP_MONKEYPATCH_COMPAT_NAME( NullGeometry ), //!< No geometry
    };
    Q_ENUM( GeometryType )

    /**
     * Raster data types.
     * This is modified and extended copy of GDALDataType.
     */
    enum class DataType SIP_MONKEYPATCH_SCOPEENUM_UNNEST( Qgis, DataType ) : int
      {
      UnknownDataType = 0, //!< Unknown or unspecified type
      Byte = 1, //!< Eight bit unsigned integer (quint8)
      Int8 = 14, //!< Eight bit signed integer (qint8) (added in QGIS 3.30)
      UInt16 = 2, //!< Sixteen bit unsigned integer (quint16)
      Int16 = 3, //!< Sixteen bit signed integer (qint16)
      UInt32 = 4, //!< Thirty two bit unsigned integer (quint32)
      Int32 = 5, //!< Thirty two bit signed integer (qint32)
      Float32 = 6, //!< Thirty two bit floating point (float)
      Float64 = 7, //!< Sixty four bit floating point (double)
      CInt16 = 8, //!< Complex Int16
      CInt32 = 9, //!< Complex Int32
      CFloat32 = 10, //!< Complex Float32
      CFloat64 = 11, //!< Complex Float64
      ARGB32 = 12, //!< Color, alpha, red, green, blue, 4 bytes the same as QImage::Format_ARGB32
      ARGB32_Premultiplied = 13 //!< Color, alpha, red, green, blue, 4 bytes  the same as QImage::Format_ARGB32_Premultiplied
    };
    Q_ENUM( DataType )

    /**
     * Capture technique.
     *
     * \since QGIS 3.26
     */
    enum class CaptureTechnique : int
    {
      StraightSegments, //!< Default capture mode - capture occurs with straight line segments
      CircularString, //!< Capture in circular strings
      Streaming, //!< Streaming points digitizing mode (points are automatically added as the mouse cursor moves).
      Shape, //!< Digitize shapes.
    };
    Q_ENUM( CaptureTechnique )

    /**
     * Vector layer type flags.
     *
     * \since QGIS 3.24
     */
    enum class VectorLayerTypeFlag : int SIP_ENUM_BASETYPE( IntFlag )
    {
      SqlQuery = 1 << 0 //!< SQL query layer
    };
    Q_ENUM( VectorLayerTypeFlag )
    //! Vector layer type flags
    Q_DECLARE_FLAGS( VectorLayerTypeFlags, VectorLayerTypeFlag )
    Q_FLAG( VectorLayerTypeFlags )

    /**
     * Authorisation to run Python Macros
     * \since QGIS 3.10
     */
    enum class PythonMacroMode SIP_MONKEYPATCH_SCOPEENUM_UNNEST( Qgis, PythonMacroMode ) : int
      {
      Never = 0, //!< Macros are never run
      Ask = 1, //!< User is prompt before running
      SessionOnly = 2, //!< Only during this session
      Always = 3, //!< Macros are always run
      NotForThisSession, //!< Macros will not be run for this session
    };
    Q_ENUM( PythonMacroMode )

    /**
     * \ingroup core
     * \brief Enumeration of feature count states
     * \since QGIS 3.20
     */
    enum class FeatureCountState SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsVectorDataProvider, FeatureCountState ) : int
      {
      Uncounted = -2, //!< Feature count not yet computed
      UnknownCount = -1, //!< Provider returned an unknown feature count
    };
    Q_ENUM( FeatureCountState )

    /**
     * Enumeration of spatial index presence states.
     *
     * \note Prior to QGIS 3.36 this was available as QgsFeatureSource::SpatialIndexPresence
     * \since QGIS 3.36
     */
    enum class SpatialIndexPresence SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsFeatureSource, SpatialIndexPresence ) : int
      {
      Unknown SIP_MONKEYPATCH_COMPAT_NAME( SpatialIndexUnknown ) = 0, //!< Spatial index presence cannot be determined, index may or may not exist
      NotPresent SIP_MONKEYPATCH_COMPAT_NAME( SpatialIndexNotPresent ) = 1, //!< No spatial index exists for the source
      Present SIP_MONKEYPATCH_COMPAT_NAME( SpatialIndexPresent ) = 2, //!< A valid spatial index exists for the source
    };
    Q_ENUM( SpatialIndexPresence )

    /**
     * Possible return value for QgsFeatureSource::hasFeatures() to determine if a source is empty.
     *
     * It is implemented as a three-value logic, so it can return if
     * there are features available for sure, if there are no features
     * available for sure or if there might be features available but
     * there is no guarantee for this.
     *
     * \note Prior to QGIS 3.36 this was available as QgsFeatureSource::FeatureAvailability
     * \since QGIS 3.36
     */
    enum class FeatureAvailability SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsFeatureSource, FeatureAvailability ) : int
      {
      NoFeaturesAvailable = 0, //!< There are certainly no features available in this source
      FeaturesAvailable, //!< There is at least one feature available in this source
      FeaturesMaybeAvailable //!< There may be features available in this source
    };
    Q_ENUM( FeatureAvailability )

    /**
     * Attribute editing capabilities which may be supported by vector data providers.
     *
     * \since QGIS 3.32
     */
    enum class VectorDataProviderAttributeEditCapability : int SIP_ENUM_BASETYPE( IntFlag )
    {
      EditAlias = 1 << 0, //!< Allows editing aliases
      EditComment = 1 << 1, //!< Allows editing comments
    };

    Q_ENUM( VectorDataProviderAttributeEditCapability )

    /**
     * Attribute editing capabilities which may be supported by vector data providers.
     *
     * \since QGIS 3.32
     */
    Q_DECLARE_FLAGS( VectorDataProviderAttributeEditCapabilities, VectorDataProviderAttributeEditCapability )
    Q_FLAG( VectorDataProviderAttributeEditCapabilities )

    /**
     * \brief Symbol types
     * \since QGIS 3.20
     */
    enum class SymbolType SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsSymbol, SymbolType ) : int
      {
      Marker, //!< Marker symbol
      Line, //!< Line symbol
      Fill, //!< Fill symbol
      Hybrid //!< Hybrid symbol
    };
    Q_ENUM( SymbolType )

    /**
     * \brief Scale methods
     *
     * \since QGIS 3.20
     */
    enum class ScaleMethod SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsSymbol, ScaleMethod ) : int
      {
      ScaleArea,     //!< Calculate scale by the area
      ScaleDiameter  //!< Calculate scale by the diameter
    };
    Q_ENUM( ScaleMethod )

    /**
     * Types of settings entries
     * \since QGIS 3.26
     */
    enum class SettingsType SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsSettingsEntryBase, SettingsType ) : int
      {
      Custom, //!< Custom implementation
      Variant, //!< Generic variant
      String, //!< String
      StringList, //!< List of strings
      VariantMap, //!< Map of strings
      Bool, //!< Boolean
      Integer, //!< Integer
      Double, //!< Double precision number
      EnumFlag, //!< Enum or Flag
      Color //!< Color
    };
    Q_ENUM( SettingsType )

    /**
     * Type of tree node
     * \since QGIS 3.30
     */
    enum class SettingsTreeNodeType
    {
      Root, //!< Root Node
      Standard, //!< Normal Node
      NamedList, //! Named List Node
    };
    Q_ENUM( SettingsTreeNodeType )

    /**
     * Options for named list nodes
     * \since QGIS 3.30
     */
    enum class SettingsTreeNodeOption : int SIP_ENUM_BASETYPE( IntFlag )
    {
      NamedListSelectedItemSetting = 1 << 0, //!< Creates a setting to store which is the current item
    };

    Q_ENUM( SettingsTreeNodeOption )
    Q_DECLARE_FLAGS( SettingsTreeNodeOptions, SettingsTreeNodeOption )
    Q_FLAG( SettingsTreeNodeOptions )

    /**
     * Property types
     *
     * \note Prior to QGIS 3.36 this was available as QgsProperty::Type
     *
     * \since QGIS 3.36
     */
    enum class PropertyType SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsProperty, Type ): int
      {
      Invalid SIP_MONKEYPATCH_COMPAT_NAME( InvalidProperty ), //!< Invalid (not set) property
      Static SIP_MONKEYPATCH_COMPAT_NAME( StaticProperty ), //!< Static property
      Field SIP_MONKEYPATCH_COMPAT_NAME( FieldBasedProperty ), //!< Field based property
      Expression SIP_MONKEYPATCH_COMPAT_NAME( ExpressionBasedProperty ), //!< Expression based property
    };
    Q_ENUM( PropertyType )

    /**
     * \brief SLD export options
     *
     * \since QGIS 3.30
     */
    enum class SldExportOption : int SIP_ENUM_BASETYPE( IntFlag )
    {
      NoOptions = 0,                      //!< Default SLD export
      Svg = 1 << 0,                       //!< Export complex styles to separate SVG files for better compatibility with OGC servers
      Png = 1 << 1,                       //!< Export complex styles to separate PNG files for better compatibility with OGC servers
    };
    Q_ENUM( SldExportOption )
    Q_DECLARE_FLAGS( SldExportOptions, SldExportOption )
    Q_FLAG( SldExportOptions )

    /**
     * \brief SLD export vendor extensions, allow the use of vendor extensions when exporting to SLD.
     *
     * \since QGIS 3.30
     */
    enum class SldExportVendorExtension : int SIP_ENUM_BASETYPE( IntFlag )
    {
      NoVendorExtension = 0,             //!< No vendor extensions
      GeoServerVendorExtension = 1 << 1, //!< Use GeoServer vendor extensions when required
      DeegreeVendorExtension = 1 << 2,   //!< Use Deegree vendor extensions when required
    };
    Q_ENUM( SldExportVendorExtension )


    /**
     * Settings options
     * \since QGIS 3.26
     */
    enum class SettingsOption : int SIP_ENUM_BASETYPE( IntFlag )
    {
      SaveFormerValue = 1 << 1, //<! Save the former value of the settings
      SaveEnumFlagAsInt = 1 << 2, //! The enum/flag will be saved as an integer value instead of text
    };
    Q_ENUM( SettingsOption )
    Q_DECLARE_FLAGS( SettingsOptions, SettingsOption )
    Q_FLAG( SettingsOptions )

    /**
     * SnappingMode defines on which layer the snapping is performed
     * \since QGIS 3.26
     */
    enum class SnappingMode SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsSnappingConfig, SnappingMode ) : int
      {
      ActiveLayer = 1, //!< On the active layer
      AllLayers = 2, //!< On all vector layers
      AdvancedConfiguration = 3, //!< On a per layer configuration basis
    };
    Q_ENUM( SnappingMode )

    /**
     * SnappingTypeFlag defines on what object the snapping is performed
     * \since QGIS 3.26
     */
    enum class SnappingType SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsSnappingConfig, SnappingTypes ) : int SIP_ENUM_BASETYPE( IntFlag )
    {
      NoSnap SIP_MONKEYPATCH_COMPAT_NAME( NoSnapFlag ) = 0, //!< No snapping
      Vertex SIP_MONKEYPATCH_COMPAT_NAME( VertexFlag ) = 1 << 0, //!< On vertices
      Segment SIP_MONKEYPATCH_COMPAT_NAME( SegmentFlag ) = 1 << 1, //!< On segments
      Area SIP_MONKEYPATCH_COMPAT_NAME( AreaFlag ) = 1 << 2, //!< On Area
      Centroid SIP_MONKEYPATCH_COMPAT_NAME( CentroidFlag ) = 1 << 3, //!< On centroid
      MiddleOfSegment SIP_MONKEYPATCH_COMPAT_NAME( MiddleOfSegmentFlag ) = 1 << 4, //!< On Middle segment
      LineEndpoint SIP_MONKEYPATCH_COMPAT_NAME( LineEndpointFlag ) = 1 << 5, //!< Start or end points of lines, or first vertex in polygon rings only (since QGIS 3.20)
    };
    Q_ENUM( SnappingType )
    //! Snapping types
    Q_DECLARE_FLAGS( SnappingTypes, SnappingType ) SIP_MONKEYPATCH_FLAGS_UNNEST( QgsSnappingConfig, SnappingTypeFlag )
    Q_FLAG( SnappingTypes )

    /**
     * \brief Flags controlling behavior of symbols during rendering
     *
     * \since QGIS 3.20
     */
    enum class SymbolRenderHint SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsSymbol, RenderHint ) : int SIP_ENUM_BASETYPE( IntFlag )
    {
      DynamicRotation = 2, //!< Rotation of symbol may be changed during rendering and symbol should not be cached
    };
    Q_ENUM( SymbolRenderHint )
    //! Symbol render hints
    Q_DECLARE_FLAGS( SymbolRenderHints, SymbolRenderHint ) SIP_MONKEYPATCH_FLAGS_UNNEST( QgsSymbol, RenderHints )
    Q_FLAG( SymbolRenderHints )

    /**
     * \brief Modes for handling how symbol and text entity rotation is handled when maps are rotated.
     *
     * \since QGIS 3.32
     */
    enum class SymbolRotationMode : int
    {
      RespectMapRotation, //!< Entity is rotated along with the map
      IgnoreMapRotation, //!< Entity ignores map rotation
    };
    Q_ENUM( SymbolRotationMode )

    /**
     * \brief Flags controlling behavior of symbols
     *
     * \since QGIS 3.20
     */
    enum class SymbolFlag : int SIP_ENUM_BASETYPE( IntFlag )
    {
      RendererShouldUseSymbolLevels = 1 << 0, //!< If present, indicates that a QgsFeatureRenderer using the symbol should use symbol levels for best results
    };
    Q_ENUM( SymbolFlag )
    //! Symbol flags
    Q_DECLARE_FLAGS( SymbolFlags, SymbolFlag )
    Q_FLAG( SymbolFlags )

    /**
     * Flags for controlling how symbol preview images are generated.
     *
     * \since QGIS 3.20
     */
    enum class SymbolPreviewFlag SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsSymbol, PreviewFlag ) : int SIP_ENUM_BASETYPE( IntFlag )
    {
      FlagIncludeCrosshairsForMarkerSymbols = 1 << 0, //!< Include a crosshairs reference image in the background of marker symbol previews
    };
    Q_ENUM( SymbolPreviewFlag )
    //! Symbol preview flags
    Q_DECLARE_FLAGS( SymbolPreviewFlags, SymbolPreviewFlag ) SIP_MONKEYPATCH_FLAGS_UNNEST( QgsSymbol, SymbolPreviewFlags )
    Q_FLAG( SymbolPreviewFlags )

    /**
     * \brief Flags controlling behavior of symbol layers
     *
     * \note These differ from Qgis::SymbolLayerUserFlag in that Qgis::SymbolLayerFlag flags are used to reflect the inbuilt properties
     * of a symbol layer type, whereas Qgis::SymbolLayerUserFlag are optional, user controlled flags which can be toggled
     * for a symbol layer.
     *
     * \since QGIS 3.22
     */
    enum class SymbolLayerFlag : int SIP_ENUM_BASETYPE( IntFlag )
    {
      DisableFeatureClipping = 1 << 0, //!< If present, indicates that features should never be clipped to the map extent during rendering
    };
    Q_ENUM( SymbolLayerFlag )
    //! Symbol layer flags
    Q_DECLARE_FLAGS( SymbolLayerFlags, SymbolLayerFlag )
    Q_FLAG( SymbolLayerFlags )

    /**
     * \brief User-specified flags controlling behavior of symbol layers.
     *
     * \note These differ from Qgis::SymbolLayerFlag in that Qgis::SymbolLayerFlag flags are used to reflect the inbuilt properties
     * of a symbol layer type, whereas Qgis::SymbolLayerUserFlag are optional, user controlled flags which can be toggled
     * for a symbol layer.
     *
     * \since QGIS 3.34
     */
    enum class SymbolLayerUserFlag : int SIP_ENUM_BASETYPE( IntFlag )
    {
      DisableSelectionRecoloring = 1 << 0, //!< If present, indicates that the symbol layer should not be recolored when rendering selected features
    };
    Q_ENUM( SymbolLayerUserFlag )

    /**
     * Symbol layer user flags.
     *
     * \since QGIS 3.34
     */
    Q_DECLARE_FLAGS( SymbolLayerUserFlags, SymbolLayerUserFlag )
    Q_FLAG( SymbolLayerUserFlags )

    /**
     * Browser item types.
     *
     * \since QGIS 3.20
     */
    enum class BrowserItemType SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsDataItem, Type ) : int
      {
      Collection, //!< A collection of items
      Directory, //!< Represents a file directory
      Layer, //!< Represents a map layer
      Error, //!< Contains an error message
      Favorites, //!< Represents a favorite item
      Project, //!< Represents a QGIS project
      Custom, //!< Custom item type
      Fields, //!< Collection of fields
      Field, //!< Vector layer field
    };
    Q_ENUM( BrowserItemType )

    /**
     * Browser item states.
     *
     * \since QGIS 3.20
     */
    enum class BrowserItemState SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsDataItem, State ) : int
      {
      NotPopulated, //!< Children not yet created
      Populating, //!< Creating children in separate thread (populating or refreshing)
      Populated, //!< Children created
    };
    Q_ENUM( BrowserItemState )

    /**
     * Browser item capabilities.
     *
     * \since QGIS 3.20
     */
    enum class BrowserItemCapability SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsDataItem, Capability ) : int SIP_ENUM_BASETYPE( IntFlag )
    {
      NoCapabilities = 0, //!< Item has no capabilities
      SetCrs = 1 << 0, //!< Can set CRS on layer or group of layers. deprecated since QGIS 3.6 -- no longer used by QGIS and will be removed in QGIS 4.0
      Fertile = 1 << 1, //!< Can create children. Even items without this capability may have children, but cannot create them, it means that children are created by item ancestors.
      Fast = 1 << 2, //!< CreateChildren() is fast enough to be run in main thread when refreshing items, most root items (wms,wfs,wcs,postgres...) are considered fast because they are reading data only from QgsSettings
      Collapse = 1 << 3, //!< The collapse/expand status for this items children should be ignored in order to avoid undesired network connections (wms etc.)
      Rename = 1 << 4, //!< Item can be renamed
      Delete = 1 << 5, //!< Item can be deleted
      ItemRepresentsFile = 1 << 6, //!< Item's path() directly represents a file on disk (since QGIS 3.22)
      RefreshChildrenWhenItemIsRefreshed = 1 << 7, //!< When the item is refreshed, all its populated children will also be refreshed in turn (since QGIS 3.26)
    };
    Q_ENUM( BrowserItemCapability )
    //! Browser item capabilities
    Q_DECLARE_FLAGS( BrowserItemCapabilities, BrowserItemCapability ) SIP_MONKEYPATCH_FLAGS_UNNEST( QgsDataItem, Capabilities )
    Q_FLAG( BrowserItemCapabilities )

    /**
     * Capabilities for data item providers.
     *
     * \note Prior to QGIS 3.36 this was available as QgsDataProvider::DataCapability
     *
     * \since QGIS 3.36
     */
    enum class DataItemProviderCapability SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsDataProvider, DataCapability ) : int SIP_ENUM_BASETYPE( IntFlag )
    {
      NoCapabilities SIP_MONKEYPATCH_COMPAT_NAME( NoDataCapabilities ) = 0, //!< No capabilities
      Files SIP_MONKEYPATCH_COMPAT_NAME( File ) = 1, //!< Can provides items which corresponds to files
      Directories SIP_MONKEYPATCH_COMPAT_NAME( Dir ) = 1 << 1, //!< Can provides items which corresponds to directories
      Databases SIP_MONKEYPATCH_COMPAT_NAME( Database ) = 1 << 2, //!< Can provides items which corresponds to databases
      NetworkSources SIP_MONKEYPATCH_COMPAT_NAME( Net ) = 1 << 3, //!< Network/internet source
    };
    Q_ENUM( DataItemProviderCapability )

    /**
     * Capabilities for data item providers.
     *
     * \note Prior to QGIS 3.36 this was available as QgsDataProvider::DataCapabilities
     *
     * \since QGIS 3.36
     */
    Q_DECLARE_FLAGS( DataItemProviderCapabilities, DataItemProviderCapability ) SIP_MONKEYPATCH_FLAGS_UNNEST( QgsDataProvider, DataCapabilities )
    Q_FLAG( DataItemProviderCapabilities )

    /**
     * Browser item layer types
     *
     * \since QGIS 3.20
     */
    enum class BrowserLayerType SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsLayerItem, LayerType ) : int
      {
      NoType, //!< No type
      Vector, //!< Generic vector layer
      Raster, //!< Raster layer
      Point, //!< Vector point layer
      Line, //!< Vector line layer
      Polygon, //!< Vector polygon layer
      TableLayer, //!< Vector non-spatial layer
      Database, //!< Database layer
      Table, //!< Database table
      Plugin, //!< Plugin based layer
      Mesh, //!< Mesh layer
      VectorTile, //!< Vector tile layer
      PointCloud, //!< Point cloud layer
      TiledScene, //!< Tiled scene layer (since QGIS 3.34)
    };
    Q_ENUM( BrowserLayerType )

    /**
     * Browser directory item monitoring switches.
     *
     * \since QGIS 3.20
     */
    enum class BrowserDirectoryMonitoring : int
    {
      Default, //!< Use default logic to determine whether directory should be monitored
      NeverMonitor, //!< Never monitor the directory, regardless of the default logic
      AlwaysMonitor, //!< Always monitor the directory, regardless of the default logic
    };
    Q_ENUM( BrowserDirectoryMonitoring )

    /**
     * Different methods of HTTP requests
     * \since QGIS 3.22
     */
    enum class HttpMethod : int
    {
      Get = 0, //!< GET method
      Post = 1 //!< POST method
    };
    Q_ENUM( HttpMethod )

    /**
     * Vector layer export result codes.
     *
     * \since QGIS 3.20
     */
    enum class VectorExportResult SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsVectorLayerExporter, ExportError ) : int
      {
      Success SIP_MONKEYPATCH_COMPAT_NAME( NoError ) = 0, //!< No errors were encountered
      ErrorCreatingDataSource SIP_MONKEYPATCH_COMPAT_NAME( ErrCreateDataSource ), //!< Could not create the destination data source
      ErrorCreatingLayer SIP_MONKEYPATCH_COMPAT_NAME( ErrCreateLayer ), //!< Could not create destination layer
      ErrorAttributeTypeUnsupported SIP_MONKEYPATCH_COMPAT_NAME( ErrAttributeTypeUnsupported ), //!< Source layer has an attribute type which could not be handled by destination
      ErrorAttributeCreationFailed SIP_MONKEYPATCH_COMPAT_NAME( ErrAttributeCreationFailed ), //!< Destination provider was unable to create an attribute
      ErrorProjectingFeatures SIP_MONKEYPATCH_COMPAT_NAME( ErrProjection ), //!< An error occurred while reprojecting features to destination CRS
      ErrorFeatureWriteFailed SIP_MONKEYPATCH_COMPAT_NAME( ErrFeatureWriteFailed ), //!< An error occurred while writing a feature to the destination
      ErrorInvalidLayer SIP_MONKEYPATCH_COMPAT_NAME( ErrInvalidLayer ), //!< Could not access newly created destination layer
      ErrorInvalidProvider SIP_MONKEYPATCH_COMPAT_NAME( ErrInvalidProvider ), //!< Could not find a matching provider key
      ErrorProviderUnsupportedFeature SIP_MONKEYPATCH_COMPAT_NAME( ErrProviderUnsupportedFeature ), //!< Provider does not support creation of empty layers
      ErrorConnectionFailed SIP_MONKEYPATCH_COMPAT_NAME( ErrConnectionFailed ), //!< Could not connect to destination
      UserCanceled SIP_MONKEYPATCH_COMPAT_NAME( ErrUserCanceled ), //!< User canceled the export
    };
    Q_ENUM( VectorExportResult )

    /**
     * Capabilities supported by a QgsVectorFileWriter object.
     * \since QGIS 3.32
     */
    enum class VectorFileWriterCapability : int SIP_ENUM_BASETYPE( IntFlag )
    {
      FieldAliases = 1 << 0, //!< Writer can support field aliases
      FieldComments = 1 << 2, //!< Writer can support field comments
    };
    Q_ENUM( VectorFileWriterCapability )

    /**
     * Capabilities supported by a QgsVectorFileWriter object.
     * \since QGIS 3.32
     */
    Q_DECLARE_FLAGS( VectorFileWriterCapabilities, VectorFileWriterCapability )
    Q_FLAG( VectorFileWriterCapabilities )

    /**
     * SqlLayerDefinitionCapability enum lists the arguments supported by the provider when creating SQL query layers.
     * \since QGIS 3.22
     */
    enum class SqlLayerDefinitionCapability : int SIP_ENUM_BASETYPE( IntFlag )
    {
      SubsetStringFilter = 1 << 1,  //!< SQL layer definition supports subset string filter
      GeometryColumn = 1 << 2,      //!< SQL layer definition supports geometry column
      PrimaryKeys = 1 << 3,         //!< SQL layer definition supports primary keys
      UnstableFeatureIds = 1 << 4   //!< SQL layer definition supports disabling select at id
    };
    Q_ENUM( SqlLayerDefinitionCapability )
    //! SQL layer definition capabilities
    Q_DECLARE_FLAGS( SqlLayerDefinitionCapabilities, SqlLayerDefinitionCapability )
    Q_FLAG( SqlLayerDefinitionCapabilities )

    /**
     * SqlKeywordCategory enum represents the categories of the SQL keywords used by the SQL query editor.
     * \note The category has currently no usage, but it was planned for future uses.
     * \since QGIS 3.22
     */
    enum class SqlKeywordCategory : int
    {
      Keyword,      //!< SQL keyword
      Constant,     //!< SQL constant
      Function,     //!< SQL generic function
      Geospatial,   //!< SQL spatial function
      Operator,     //!< SQL operator
      Math,         //!< SQL math function
      Aggregate,    //!< SQL aggregate function
      String,       //!< SQL string function
      Identifier    //!< SQL identifier
    };
    Q_ENUM( SqlKeywordCategory )

    /**
     * Drive types
     * \since QGIS 3.20
     */
    enum class DriveType : int
    {
      Unknown, //!< Unknown type
      Invalid, //!< Invalid path
      Removable, //!< Removable drive
      Fixed, //!< Fixed drive
      Remote, //!< Remote drive
      CdRom, //!< CD-ROM
      RamDisk, //!< RAM disk
      Cloud, //!< Cloud storage -- files may be remote or locally stored, depending on user configuration
    };
    Q_ENUM( DriveType )

    /**
     * Enum to determine when an operation would begin
     * \since QGIS 3.22
     */
    enum class ActionStart SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsNetworkContentFetcherRegistry, FetchingMode ) : int
      {
      Deferred SIP_MONKEYPATCH_COMPAT_NAME( DownloadLater ), //!< Do not start immediately the action
      Immediate SIP_MONKEYPATCH_COMPAT_NAME( DownloadImmediately ), //!< Action will start immediately
    };
    Q_ENUM( ActionStart )

    /**
     * Unplaced label visibility.
     *
     * \since QGIS 3.20
     */
    enum class UnplacedLabelVisibility : int
    {
      FollowEngineSetting, //!< Respect the label engine setting
      NeverShow, //!< Never show unplaced labels, regardless of the engine setting
    };
    Q_ENUM( UnplacedLabelVisibility )

    /**
     * Label overlap handling.
     *
     * \since QGIS 3.26
     */
    enum class LabelOverlapHandling : int
    {
      PreventOverlap, //!< Do not allow labels to overlap other labels
      AllowOverlapIfRequired, //!< Avoids overlapping labels when possible, but permit overlaps if labels for features cannot otherwise be placed
      AllowOverlapAtNoCost, //!< Labels may freely overlap other labels, at no cost
    };
    Q_ENUM( LabelOverlapHandling )

    /**
     * Placement modes which determine how label candidates are generated for a feature.
     *
     * \note Prior to QGIS 3.26 this was available as QgsPalLayerSettings::Placement
     *
     * \since QGIS 3.26
     */
    enum class LabelPlacement SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsPalLayerSettings, Placement ) : int
      {
      AroundPoint, //!< Arranges candidates in a circle around a point (or centroid of a polygon). Applies to point or polygon layers only.
      OverPoint, //!< Arranges candidates over a point (or centroid of a polygon), or at a preset offset from the point. Applies to point or polygon layers only.
      Line, //!< Arranges candidates parallel to a generalised line representing the feature or parallel to a polygon's perimeter. Applies to line or polygon layers only.
      Curved, //!< Arranges candidates following the curvature of a line feature. Applies to line layers only.
      Horizontal, //!< Arranges horizontal candidates scattered throughout a polygon feature. Applies to polygon layers only.
      Free, //!< Arranges candidates scattered throughout a polygon feature. Candidates are rotated to respect the polygon's orientation. Applies to polygon layers only.
      OrderedPositionsAroundPoint, //!< Candidates are placed in predefined positions around a point. Preference is given to positions with greatest cartographic appeal, e.g., top right, bottom right, etc. Applies to point layers only.
      PerimeterCurved, //!< Arranges candidates following the curvature of a polygon's boundary. Applies to polygon layers only.
      OutsidePolygons, //!< Candidates are placed outside of polygon boundaries. Applies to polygon layers only. Since QGIS 3.14
    };
    Q_ENUM( LabelPlacement )

    /**
     * Positions for labels when using the Qgis::LabelPlacement::OrderedPositionsAroundPoint placement mode.
     *
     * \note Prior to QGIS 3.26 this was available as QgsPalLayerSettings::PredefinedPointPosition
     *
     * \since QGIS 3.26
     */
    enum class LabelPredefinedPointPosition SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsPalLayerSettings, PredefinedPointPosition ) : int
      {
      TopLeft, //!< Label on top-left of point
      TopSlightlyLeft, //!< Label on top of point, slightly left of center
      TopMiddle, //!< Label directly above point
      TopSlightlyRight, //!< Label on top of point, slightly right of center
      TopRight, //!< Label on top-right of point
      MiddleLeft, //!< Label on left of point
      MiddleRight, //!< Label on right of point
      BottomLeft, //!< Label on bottom-left of point
      BottomSlightlyLeft, //!< Label below point, slightly left of center
      BottomMiddle, //!< Label directly below point
      BottomSlightlyRight, //!< Label below point, slightly right of center
      BottomRight, //!< Label on bottom right of point
    };
    Q_ENUM( LabelPredefinedPointPosition )

    /**
     * Behavior modifier for label offset and distance, only applies in some
     * label placement modes.
     *
     * \note Prior to QGIS 3.26 this was available as QgsPalLayerSettings::OffsetType
     *
     * \since QGIS 3.26
     */
    enum class LabelOffsetType SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsPalLayerSettings, OffsetType ) : int
      {
      FromPoint, //!< Offset distance applies from point geometry
      FromSymbolBounds, //!< Offset distance applies from rendered symbol bounds
    };
    Q_ENUM( LabelOffsetType )

    /**
     * Label quadrant positions
     *
     * \note Prior to QGIS 3.26 this was available as QgsPalLayerSettings::QuadrantPosition
     *
     * \since QGIS 3.26
     */
    enum class LabelQuadrantPosition SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsPalLayerSettings, QuadrantPosition ) : int
      {
      AboveLeft SIP_MONKEYPATCH_COMPAT_NAME( QuadrantAboveLeft ), //!< Above left
      Above SIP_MONKEYPATCH_COMPAT_NAME( QuadrantAbove ), //!< Above center
      AboveRight SIP_MONKEYPATCH_COMPAT_NAME( QuadrantAboveRight ), //!< Above right
      Left SIP_MONKEYPATCH_COMPAT_NAME( QuadrantLeft ), //!< Left middle
      Over SIP_MONKEYPATCH_COMPAT_NAME( QuadrantOver ), //!< Center middle
      Right SIP_MONKEYPATCH_COMPAT_NAME( QuadrantRight ), //!< Right middle
      BelowLeft SIP_MONKEYPATCH_COMPAT_NAME( QuadrantBelowLeft ), //!< Below left
      Below SIP_MONKEYPATCH_COMPAT_NAME( QuadrantBelow ), //!< Below center
      BelowRight SIP_MONKEYPATCH_COMPAT_NAME( QuadrantBelowRight ), //!< BelowRight
    };
    Q_ENUM( LabelQuadrantPosition )

    /**
     * Line placement flags, which control how candidates are generated for a linear feature.
     *
     * \note Prior to QGIS 3.32 this was available as QgsLabeling::LinePlacementFlag
     * \since QGIS 3.32
     */
    enum class LabelLinePlacementFlag SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsLabeling, LinePlacementFlag ) : int SIP_ENUM_BASETYPE( IntFlag )
    {
      OnLine = 1, //!< Labels can be placed directly over a line feature.
      AboveLine = 2, //!< Labels can be placed above a line feature. Unless MapOrientation is also specified this mode respects the direction of the line feature, so a line from right to left labels will have labels placed placed below the line feature.
      BelowLine = 4, //!< Labels can be placed below a line feature. Unless MapOrientation is also specified this mode respects the direction of the line feature, so a line from right to left labels will have labels placed placed above the line feature.
      MapOrientation = 8, //!< Signifies that the AboveLine and BelowLine flags should respect the map's orientation rather than the feature's orientation. For example, AboveLine will always result in label's being placed above a line, regardless of the line's direction.
    };
    Q_ENUM( LabelLinePlacementFlag )

    /**
     * Line placement flags, which control how candidates are generated for a linear feature.
     *
     * \note Prior to QGIS 3.32 this was available as QgsLabeling::LinePlacementFlags
     *
     * \since QGIS 3.32
     */
    Q_DECLARE_FLAGS( LabelLinePlacementFlags, LabelLinePlacementFlag ) SIP_MONKEYPATCH_FLAGS_UNNEST( QgsLabeling, LinePlacementFlags )
    Q_FLAG( LabelLinePlacementFlags )

    /**
     * Polygon placement flags, which control how candidates are generated for a polygon feature.
     *
     * \note Prior to QGIS 3.32 this was available as QgsLabeling::PolygonPlacementFlag
     * \since QGIS 3.32
     */
    enum class LabelPolygonPlacementFlag SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsLabeling, PolygonPlacementFlag ) : int SIP_ENUM_BASETYPE( IntFlag )
    {
      AllowPlacementOutsideOfPolygon = 1 << 0, //!< Labels can be placed outside of a polygon feature
      AllowPlacementInsideOfPolygon = 1 << 1, //!< Labels can be placed inside a polygon feature
    };
    Q_ENUM( LabelPolygonPlacementFlag )

    /**
     * Polygon placement flags, which control how candidates are generated for a polygon feature.
     *
     * \note Prior to QGIS 3.32 this was available as QgsLabeling::PolygonPlacementFlags
     * \since QGIS 3.32
     */
    Q_DECLARE_FLAGS( LabelPolygonPlacementFlags, LabelPolygonPlacementFlag )
    Q_FLAG( LabelPolygonPlacementFlags )

    /**
     * Handling techniques for upside down labels.
     *
     * \note Prior to QGIS 3.26 this was available as QgsPalLayerSettings::UpsideDownLabels
     *
     * \since QGIS 3.26
     */
    enum class UpsideDownLabelHandling SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsPalLayerSettings, UpsideDownLabels ) : int
      {
      FlipUpsideDownLabels SIP_MONKEYPATCH_COMPAT_NAME( Upright ), //!< Upside-down labels (90 <= angle < 270) are shown upright
      AllowUpsideDownWhenRotationIsDefined SIP_MONKEYPATCH_COMPAT_NAME( ShowDefined ), //!< Show upside down when rotation is layer- or data-defined
      AlwaysAllowUpsideDown SIP_MONKEYPATCH_COMPAT_NAME( ShowAll ) //!< Show upside down for all labels, including dynamic ones
    };
    Q_ENUM( UpsideDownLabelHandling )

    /**
     * Text alignment for multi-line labels.
     *
     * \note Prior to QGIS 3.26 this was available as QgsPalLayerSettings::MultiLineAlign
     *
     * \since QGIS 3.26
     */
    enum class LabelMultiLineAlignment SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsPalLayerSettings, MultiLineAlign ) : int
      {
      Left SIP_MONKEYPATCH_COMPAT_NAME( MultiLeft ) = 0, //!< Left align
      Center SIP_MONKEYPATCH_COMPAT_NAME( MultiCenter ), //!< Center align
      Right SIP_MONKEYPATCH_COMPAT_NAME( MultiRight ), //!< Right align
      FollowPlacement SIP_MONKEYPATCH_COMPAT_NAME( MultiFollowPlacement ), //!< Alignment follows placement of label, e.g., labels to the left of a feature will be drawn with right alignment
      Justify SIP_MONKEYPATCH_COMPAT_NAME( MultiJustify ), //!< Justified
    };
    Q_ENUM( LabelMultiLineAlignment )

    /**
     * Type of file filters
     *
     * Prior to QGIS 3.32 this was available as QgsProviderMetadata::FilterType
     * \since QGIS 3.32
     */
    enum class FileFilterType SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsProviderMetadata, FilterType ) : int
      {
      Vector SIP_MONKEYPATCH_COMPAT_NAME( FilterVector ) = 1, //!< Vector layers
      Raster SIP_MONKEYPATCH_COMPAT_NAME( FilterRaster ), //!< Raster layers
      Mesh SIP_MONKEYPATCH_COMPAT_NAME( FilterMesh ), //!< Mesh layers
      MeshDataset SIP_MONKEYPATCH_COMPAT_NAME( FilterMeshDataset ), //!< Mesh datasets
      PointCloud SIP_MONKEYPATCH_COMPAT_NAME( FilterPointCloud ), //!< Point clouds (since QGIS 3.18)
      VectorTile, //!< Vector tile layers (since QGIS 3.32)
      TiledScene, //!< Tiled scene layers (since QGIS 3.34)
    };
    Q_ENUM( FileFilterType )

    /**
     * Flags which control how data providers will scan for sublayers in a dataset.
     *
     * \since QGIS 3.22
     */
    enum class SublayerQueryFlag : int SIP_ENUM_BASETYPE( IntFlag )
    {
      FastScan = 1 << 0, //!< Indicates that the provider must scan for sublayers using the fastest possible approach -- e.g. by first checking that a uri has an extension which is known to be readable by the provider
      ResolveGeometryType = 1 << 1, //!< Attempt to resolve the geometry type for vector sublayers
      CountFeatures = 1 << 2, //!< Count features in vector sublayers
      IncludeSystemTables = 1 << 3, //!< Include system or internal tables (these are not included by default)
    };
    //! Sublayer query flags
    Q_DECLARE_FLAGS( SublayerQueryFlags, SublayerQueryFlag )
    Q_ENUM( SublayerQueryFlag )
    Q_FLAG( SublayerQueryFlags )

    /**
     * Flags which reflect the properties of sublayers in a dataset.
     *
     * \since QGIS 3.22
     */
    enum class SublayerFlag : int SIP_ENUM_BASETYPE( IntFlag )
    {
      SystemTable = 1 << 0, //!< Sublayer is a system or internal table, which should be hidden by default
    };
    //! Sublayer flags
    Q_DECLARE_FLAGS( SublayerFlags, SublayerFlag )
    Q_ENUM( SublayerFlag )
    Q_FLAG( SublayerFlags )

    /**
     * Raster pipe interface roles.
     *
     * \since QGIS 3.22
     */
    enum class RasterPipeInterfaceRole SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsRasterPipe, Role ) : int
      {
      Unknown SIP_MONKEYPATCH_COMPAT_NAME( UnknownRole ) = 0, //!< Unknown role
      Provider SIP_MONKEYPATCH_COMPAT_NAME( ProviderRole ) = 1, //!< Data provider role
      Renderer SIP_MONKEYPATCH_COMPAT_NAME( RendererRole ) = 2, //!< Raster renderer role
      Brightness SIP_MONKEYPATCH_COMPAT_NAME( BrightnessRole ) = 3, //!< Brightness filter role
      Resampler SIP_MONKEYPATCH_COMPAT_NAME( ResamplerRole ) = 4, //!< Resampler role
      Projector SIP_MONKEYPATCH_COMPAT_NAME( ProjectorRole ) = 5, //!< Projector role
      Nuller SIP_MONKEYPATCH_COMPAT_NAME( NullerRole ) = 6, //!< Raster nuller role
      HueSaturation SIP_MONKEYPATCH_COMPAT_NAME( HueSaturationRole ) = 7, //!< Hue/saturation filter role (also applies grayscale/color inversion)
    };
    Q_ENUM( RasterPipeInterfaceRole )

    /**
     * Stage at which raster resampling occurs.
     * \since QGIS 3.22
     */
    enum class RasterResamplingStage SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsRasterPipe, ResamplingStage ) : int
      {
      //! Resampling occurs in ResamplingFilter
      ResampleFilter,
      //! Resampling occurs in Provider
      Provider
    };
    Q_ENUM( RasterResamplingStage )

    /**
     * Flags which control behavior of raster renderers.
     *
     * \since QGIS 3.28
     */
    enum class RasterRendererFlag : int SIP_ENUM_BASETYPE( IntFlag )
    {
      InternalLayerOpacityHandling = 1 << 0, //!< The renderer internally handles the raster layer's opacity, so the default layer level opacity handling should not be applied.
    };

    /**
     * Flags which control behavior of raster renderers.
     *
     * \since QGIS 3.28
     */
    Q_DECLARE_FLAGS( RasterRendererFlags, RasterRendererFlag )

    Q_ENUM( RasterRendererFlag )
    Q_FLAG( RasterRendererFlags )

    /**
     * Raster renderer capabilities.
     *
     * \since QGIS 3.48
     */
    enum class RasterRendererCapability : int SIP_ENUM_BASETYPE( IntFlag )
    {
      UsesMultipleBands = 1 << 0, //!< The renderer utilizes multiple raster bands for color data (note that alpha bands are not considered for this capability)
    };
    Q_ENUM( RasterRendererCapability )

    /**
     * Raster renderer capabilities.
     *
     * \since QGIS 3.38
     */
    Q_DECLARE_FLAGS( RasterRendererCapabilities, RasterRendererCapability )
    Q_FLAG( RasterRendererCapabilities )

    /**
     * \brief The RasterAttributeTableFieldUsage enum represents the usage of a Raster Attribute Table field.
     * \note Directly mapped from GDALRATFieldUsage enum values.
     * \since QGIS 3.30
     */
    enum class RasterAttributeTableFieldUsage : int
    {
      Generic = 0, //!< Field usage Generic
      PixelCount = 1, //!< Field usage PixelCount
      Name = 2, //!< Field usage Name
      Min = 3, //!< Field usage Min
      Max = 4, //!< Field usage Max
      MinMax = 5, //!< Field usage MinMax
      Red = 6, //!< Field usage Red
      Green = 7, //!< Field usage Green
      Blue = 8, //!< Field usage Blue
      Alpha = 9, //!< Field usage Alpha
      RedMin = 10, //!< Field usage RedMin
      GreenMin = 11, //!< Field usage GreenMin
      BlueMin = 12, //!< Field usage BlueMin
      AlphaMin = 13, //!< Field usage AlphaMin
      RedMax = 14, //!< Field usage RedMax
      GreenMax = 15, //!< Field usage GreenMax
      BlueMax = 16, //!< Field usage BlueMax
      AlphaMax = 17, //!< Field usage AlphaMax
      MaxCount   //!< Not used by QGIS: GDAL Maximum GFU value (equals to GFU_AlphaMax+1 currently)
    };
    Q_ENUM( RasterAttributeTableFieldUsage )

    /**
     * \brief The RasterAttributeTableType enum represents the type of RAT.
     *  note Directly mapped from GDALRATTableType enum values.
     * \since QGIS 3.30
     */
    enum class RasterAttributeTableType : int
    {
      Thematic = 0,
      Athematic = 1
    };
    Q_ENUM( RasterAttributeTableType )

    /**
     * Raster file export types.
     *
     * Prior to QGIS 3.32 this was available as QgsRasterFileWriter::Mode
     * \since QGIS 3.32
     */
    enum class RasterExportType SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsRasterFileWriter, Mode ) : int
      {
      Raw = 0, //!< Raw data
      RenderedImage SIP_MONKEYPATCH_COMPAT_NAME( Image ) = 1 //!< Rendered image
    };
    Q_ENUM( RasterExportType )

    /**
     * Raster file export results.
     *
     * Prior to QGIS 3.32 this was available as QgsRasterFileWriter::WriterError
     * \since QGIS 3.32
     */
    enum class RasterFileWriterResult SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsRasterFileWriter, WriterError ) : int
      {
      Success SIP_MONKEYPATCH_COMPAT_NAME( NoError ) = 0, //!< Successful export
      SourceProviderError = 1, //!< Source data provider error
      DestinationProviderError SIP_MONKEYPATCH_COMPAT_NAME( DestProviderError ) = 2, //!< Destination data provider error
      CreateDatasourceError = 3, //!< Data source creation error
      WriteError = 4, //!< Write error
      NoDataConflict = 5, //!< Internal error if a value used for 'no data' was found in input
      Canceled SIP_MONKEYPATCH_COMPAT_NAME( WriteCanceled ) = 6, //!< Writing was manually canceled
    };
    Q_ENUM( RasterFileWriterResult )

    /**
     * Type of error that can occur during mesh frame editing.
     *
     * \since QGIS 3.22
     */
    enum class MeshEditingErrorType : int
    {
      NoError, //!< No type
      InvalidFace, //!< An error occurs due to an invalid face (for example, vertex indexes are unordered)
      TooManyVerticesInFace, //!< A face has more vertices than the maximum number supported per face
      FlatFace, //!< A flat face is present
      UniqueSharedVertex, //!< A least two faces share only one vertices
      InvalidVertex, //!< An error occurs due to an invalid vertex (for example, vertex index is out of range the available vertex)
      ManifoldFace, //!< ManifoldFace
    };
    Q_ENUM( MeshEditingErrorType )

    /**
     * File path types.
     *
     * \since QGIS 3.22
     */
    enum class FilePathType : int
    {
      Absolute, //!< Absolute path
      Relative, //!< Relative path
    };
    Q_ENUM( FilePathType )

    /**
     * Specifies how to handle layer sources with multiple sublayers.
     *
     * \since QGIS 3.22
     */
    enum class SublayerPromptMode : int
    {
      AlwaysAsk, //!< Always ask users to select from available sublayers, if sublayers are present
      AskExcludingRasterBands, //!< Ask users to select from available sublayers, unless only raster bands are present
      NeverAskSkip, //!< Never ask users to select sublayers, instead don't load anything
      NeverAskLoadAll, //!< Never ask users to select sublayers, instead automatically load all available sublayers
    };
    Q_ENUM( SublayerPromptMode )

    /**
     * Configuration flags for fields
     * These flags are meant to be user-configurable
     * and are not describing any information from the data provider.
     * \note FieldConfigurationFlag are expressed in the negative forms so that default flags is NoFlag.
     * \since QGIS 3.34
     */
    enum class FieldConfigurationFlag : int SIP_ENUM_BASETYPE( IntFlag )
    {
      NoFlag = 0, //!< No flag is defined
      NotSearchable = 1 << 1, //!< Defines if the field is searchable (used in the locator search for instance)
      HideFromWms = 1 << 2, //!< Field is not available if layer is served as WMS from QGIS server
      HideFromWfs = 1 << 3, //!< Field is not available if layer is served as WFS from QGIS server
    };
    Q_ENUM( FieldConfigurationFlag )

    /**
     * Configuration flags for fields
     * These flags are meant to be user-configurable
     * and are not describing any information from the data provider.
     * \note FieldConfigurationFlag are expressed in the negative forms so that default flags is NoFlag.
     * \since QGIS 3.34
     */
    Q_DECLARE_FLAGS( FieldConfigurationFlags, FieldConfigurationFlag )
    Q_FLAG( FieldConfigurationFlags )

    /**
     * Standard field metadata values.
     *
     * \since QGIS 3.30
     */
    enum class FieldMetadataProperty : int
    {
      GeometryCrs = 0x1000, //!< Available for geometry field types with a specific associated coordinate reference system (as a QgsCoordinateReferenceSystem value)
      GeometryWkbType = 0x1001, //!< Available for geometry field types which accept geometries of a specific WKB type only (as a QgsWkbTypes::Type value)
      CustomProperty = 0x100000, //!< Starting point for custom user set properties
    };
    Q_ENUM( FieldMetadataProperty )

    /**
     * Specifies how a selection should be rendered.
     *
     * \since QGIS 3.34
     */
    enum class SelectionRenderingMode : int
    {
      Default, //!< Use default symbol and selection colors
      CustomColor, //!< Use default symbol with a custom selection color
      CustomSymbol, //!< Use a custom symbol
    };
    Q_ENUM( SelectionRenderingMode )

    /**
     * Specifies how a selection should be applied.
     *
     * \since QGIS 3.22
     */
    enum class SelectBehavior SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsVectorLayer, SelectBehavior ) : int
      {
      SetSelection, //!< Set selection, removing any existing selection
      AddToSelection, //!< Add selection to current selection
      IntersectSelection, //!< Modify current selection to include only select features which match
      RemoveFromSelection, //!< Remove from current selection
    };
    Q_ENUM( SelectBehavior )

    /**
     * Geometry relationship test to apply for selecting features.
     *
     * \since QGIS 3.28
     */
    enum class SelectGeometryRelationship : int
    {
      Intersect, //!< Select where features intersect the reference geometry
      Within, //!< Select where features are within the reference geometry
    };
    Q_ENUM( SelectGeometryRelationship )

    /**
     * Flags which control feature selection behavior.
     *
     * \since QGIS 3.28
     */
    enum class SelectionFlag : int SIP_ENUM_BASETYPE( IntFlag )
    {
      SingleFeatureSelection = 1 << 0, //!< Select only a single feature, picking the "best" match for the selection geometry
      ToggleSelection = 1 << 1, //!< Enables a "toggle" selection mode, where previously selected matching features will be deselected and previously deselected features will be selected. This flag works only when the SingleFeatureSelection flag is also set.
    };

    /**
     * Flags which control feature selection behavior.
     *
     * \since QGIS 3.28
     */
    Q_DECLARE_FLAGS( SelectionFlags, SelectionFlag )

    Q_ENUM( SelectionFlag )
    Q_FLAG( SelectionFlags )

    /**
     * Specifies the result of a vector layer edit operation
     *
     * \since QGIS 3.22
     */
    enum class VectorEditResult SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsVectorLayer, EditResult ) : int
      {
      Success = 0, //!< Edit operation was successful
      EmptyGeometry = 1, //!< Edit operation resulted in an empty geometry
      EditFailed = 2, //!< Edit operation failed
      FetchFeatureFailed = 3, //!< Unable to fetch requested feature
      InvalidLayer = 4, //!< Edit failed due to invalid layer
    };
    Q_ENUM( VectorEditResult )

    /**
     * Editing vertex markers, used for showing vertices during a edit operation.
     *
     * \since QGIS 3.22
     */
    enum class VertexMarkerType SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsSymbolLayerUtils, VertexMarkerType ) : int
      {
      SemiTransparentCircle, //!< Semi-transparent circle marker
      Cross, //!< Cross marker
      NoMarker, //!< No marker
    };
    Q_ENUM( VertexMarkerType )

    /**
     * Status for fetched or stored content
     * \since QGIS 3.22
     */
    enum class ContentStatus : int
    {
      NotStarted, //!< Content fetching/storing has not started yet
      Running, //!< Content fetching/storing is in progress
      Finished, //!< Content fetching/storing is finished and successful
      Failed, //!< Content fetching/storing has failed
      Canceled, //!< Content fetching/storing has been canceled
    };
    Q_ENUM( ContentStatus )

    /**
     * GPS connection types.
     *
     * \since QGIS 3.30
     */
    enum class GpsConnectionType : int
    {
      Automatic, //!< Automatically detected GPS device connection
      Internal, //!< Internal GPS device
      Serial, //!< Serial port GPS device
      Gpsd, //!< GPSD device
    };
    Q_ENUM( GpsConnectionType )

    /**
     * GPS connection status.
     *
     * \since QGIS 3.30
     */
    enum class DeviceConnectionStatus SIP_MONKEYPATCH_SCOPEENUM_UNNEST( Qgis, GpsConnectionStatus ) : int
      {
      Disconnected, //!< Device is disconnected
      Connecting, //!< Device is connecting
      Connected, //!< Device is successfully connected
    };
    Q_ENUM( DeviceConnectionStatus )

    /**
     * GPS fix status.
     *
     * \note Prior to QGIS 3.30 this was available as QgsGpsInformation::FixStatus
     *
     * \since QGIS 3.30
     */
    enum class GpsFixStatus SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsGpsInformation, FixStatus ) : int
      {
      NoData, //!< No fix data available
      NoFix, //!< GPS is not fixed
      Fix2D, //!< 2D fix
      Fix3D //!< 3D fix
    };
    Q_ENUM( GpsFixStatus );


    /**
     * GNSS constellation
     *
     * \since QGIS 3.30
     */
    enum class GnssConstellation
    {
      Unknown, //!< Unknown/other system
      Gps, //!< Global Positioning System (GPS)
      Glonass, //!< Global Navigation Satellite System (GLONASS)
      Galileo, //!< Galileo
      BeiDou, //!< BeiDou
      Qzss, //!< Quasi Zenith Satellite System (QZSS)
      Navic, //!< Indian Regional Navigation Satellite System (IRNSS) / NAVIC
      Sbas, //!< SBAS
    };
    Q_ENUM( GnssConstellation );

    /**
     * GPS signal quality indicator
     *
     * \since QGIS 3.22.6
     */
    enum class GpsQualityIndicator
    {
      Unknown = -1, //!< Unknown
      Invalid, //!< Invalid
      GPS, //!< Standalone
      DGPS, //!< Differential GPS
      PPS, //!< PPS
      RTK, //!< Real-time-kynematic
      FloatRTK, //!< Float real-time-kynematic
      Estimated, //!< Estimated
      Manual, //!< Manual input mode
      Simulation, //!< Simulation mode
    };
    Q_ENUM( GpsQualityIndicator )

    /**
     * GPS information component.
     *
     * \since QGIS 3.30
     */
    enum class GpsInformationComponent : int SIP_ENUM_BASETYPE( IntFlag )
    {
      Location = 1 << 0, //!< 2D location (latitude/longitude), as a QgsPointXY value
      Altitude = 1 << 1, //!< Altitude/elevation above or below the mean sea level
      GroundSpeed = 1 << 2, //!< Ground speed
      Bearing = 1 << 3, //!< Bearing measured in degrees clockwise from true north to the direction of travel
      TotalTrackLength = 1 << 4, //!< Total distance of current GPS track (available from QgsGpsLogger class only)
      TrackDistanceFromStart = 1 << 5, //!< Direct distance from first vertex in current GPS track to last vertex (available from QgsGpsLogger class only)
      Pdop = 1 << 6, //!< Dilution of precision
      Hdop = 1 << 7, //!< Horizontal dilution of precision
      Vdop = 1 << 8, //!< Vertical dilution of precision
      HorizontalAccuracy = 1 << 9, //!< Horizontal accuracy in meters
      VerticalAccuracy = 1 << 10, //!< Vertical accuracy in meters
      HvAccuracy = 1 << 11, //!< 3D RMS
      SatellitesUsed = 1 << 12, //!< Count of satellites used in obtaining the fix
      Timestamp = 1 << 13, //!< Timestamp
      TrackStartTime = 1 << 14, //!< Timestamp at start of current track (available from QgsGpsLogger class only)
      TrackEndTime = 1 << 15, //!< Timestamp at end (current point) of current track (available from QgsGpsLogger class only)
      TrackDistanceSinceLastPoint = 1 << 16, //!< Distance since last recorded location (available from QgsGpsLogger class only)
      TrackTimeSinceLastPoint = 1 << 17, //!< Time since last recorded location (available from QgsGpsLogger class only)
      GeoidalSeparation = 1 << 18, //!< Geoidal separation, the difference between the WGS-84 Earth ellipsoid and mean-sea-level (geoid), "-" means mean-sea-level below ellipsoid
      EllipsoidAltitude = 1 << 19, //!< Altitude/elevation above or below the WGS-84 Earth ellipsoid
    };

    /**
     * GPS information component.
     *
     * \since QGIS 3.30
     */
    Q_DECLARE_FLAGS( GpsInformationComponents, GpsInformationComponent )
    Q_ENUM( GpsInformationComponent )
    Q_FLAG( GpsInformationComponents )

    /**
     * Babel GPS format capabilities.
     *
     * \since QGIS 3.22
     */
    enum class BabelFormatCapability : int SIP_ENUM_BASETYPE( IntFlag )
    {
      Import = 1 << 0, //!< Format supports importing
      Export = 1 << 1, //!< Format supports exporting
      Waypoints = 1 << 2, //!< Format supports waypoints
      Routes = 1 << 3, //!< Format supports routes
      Tracks = 1 << 4, //!< Format supports tracks
    };
    //! Babel GPS format capabilities
    Q_DECLARE_FLAGS( BabelFormatCapabilities, BabelFormatCapability )
    Q_ENUM( BabelFormatCapability )
    Q_FLAG( BabelFormatCapabilities )

    /**
     * Babel command flags, which control how commands and arguments
     * are generated for executing GPSBabel processes.
     *
     * \since QGIS 3.22
     */
    enum class BabelCommandFlag : int SIP_ENUM_BASETYPE( IntFlag )
    {
      QuoteFilePaths = 1 << 0, //!< File paths should be enclosed in quotations and escaped
    };
    //! Babel command flags
    Q_DECLARE_FLAGS( BabelCommandFlags, BabelCommandFlag )
    Q_ENUM( BabelCommandFlag )
    Q_FLAG( BabelCommandFlags )

    /**
     * GPS feature types.
     *
     * \since QGIS 3.22
     */
    enum class GpsFeatureType : int
    {
      Waypoint, //!< Waypoint
      Route, //!< Route
      Track, //!< Track
    };
    Q_ENUM( GpsFeatureType )

    /**
     * Success or failure of a geometry operation.
     *
     * This enum gives details about cause of failure.
     *
     * \since QGIS 3.22
     */
    enum class GeometryOperationResult SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsGeometry, OperationResult ) : int
      {
      Success = 0, //!< Operation succeeded
      NothingHappened = 1000, //!< Nothing happened, without any error
      InvalidBaseGeometry, //!< The base geometry on which the operation is done is invalid or empty
      InvalidInputGeometryType, //!< The input geometry (ring, part, split line, etc.) has not the correct geometry type
      SelectionIsEmpty, //!< No features were selected
      SelectionIsGreaterThanOne, //!< More than one features were selected
      GeometryEngineError, //!< Geometry engine misses a method implemented or an error occurred in the geometry engine
      LayerNotEditable, //!< Cannot edit layer
      // Add part issues
      AddPartSelectedGeometryNotFound, //!< The selected geometry cannot be found
      AddPartNotMultiGeometry, //!< The source geometry is not multi
      // Add ring issues
      AddRingNotClosed, //!< The input ring is not closed
      AddRingNotValid, //!< The input ring is not valid
      AddRingCrossesExistingRings, //!< The input ring crosses existing rings (it is not disjoint)
      AddRingNotInExistingFeature, //!< The input ring doesn't have any existing ring to fit into
      // Split features
      SplitCannotSplitPoint, //!< Cannot split points
      GeometryTypeHasChanged, //!< Operation has changed geometry type
    };
    Q_ENUM( GeometryOperationResult )

    /**
     * Geometry validity check flags.
     *
     * \since QGIS 3.22
     */
    enum class GeometryValidityFlag SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsGeometry, ValidityFlag ) : int SIP_ENUM_BASETYPE( IntFlag )
    {
      AllowSelfTouchingHoles SIP_MONKEYPATCH_COMPAT_NAME( FlagAllowSelfTouchingHoles ) = 1 << 0, //!< Indicates that self-touching holes are permitted. OGC validity states that self-touching holes are NOT permitted, whilst other vendor validity checks (e.g. ESRI) permit self-touching holes.
    };
    //! Geometry validity flags
    Q_DECLARE_FLAGS( GeometryValidityFlags, GeometryValidityFlag ) SIP_MONKEYPATCH_FLAGS_UNNEST( QgsGeometry, ValidityFlags )
    Q_ENUM( GeometryValidityFlag )
    Q_FLAG( GeometryValidityFlags )

    /**
     * Available engines for validating geometries.
     * \since QGIS 3.22
     */
    enum class GeometryValidationEngine SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsGeometry, ValidationMethod ) : int
      {
      QgisInternal SIP_MONKEYPATCH_COMPAT_NAME( ValidatorQgisInternal ), //!< Use internal QgsGeometryValidator method
      Geos SIP_MONKEYPATCH_COMPAT_NAME( ValidatorGeos ), //!< Use GEOS validation methods
    };
    Q_ENUM( GeometryValidationEngine )

    /**
     * Side of line to buffer.
     *
     * \since QGIS 3.22
     */
    enum class BufferSide SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsGeometry, BufferSide ) : int
      {
      Left SIP_MONKEYPATCH_COMPAT_NAME( SideLeft ) = 0, //!< Buffer to left of line
      Right SIP_MONKEYPATCH_COMPAT_NAME( SideRight ), //!< Buffer to right of line
    };
    Q_ENUM( BufferSide )

    /**
     * End cap styles for buffers.
     *
     * \since QGIS 3.22
     */
    enum class EndCapStyle SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsGeometry, EndCapStyle ) : int
      {
      Round SIP_MONKEYPATCH_COMPAT_NAME( CapRound ) = 1, //!< Round cap
      Flat SIP_MONKEYPATCH_COMPAT_NAME( CapFlat ), //!< Flat cap (in line with start/end of line)
      Square SIP_MONKEYPATCH_COMPAT_NAME( CapSquare ), //!< Square cap (extends past start/end of line by buffer distance)
    };
    Q_ENUM( EndCapStyle )

    /**
     * Join styles for buffers.
     *
     * \since QGIS 3.22
     */
    enum class JoinStyle SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsGeometry, JoinStyle ) : int
      {
      Round SIP_MONKEYPATCH_COMPAT_NAME( JoinStyleRound ) = 1, //!< Use rounded joins
      Miter SIP_MONKEYPATCH_COMPAT_NAME( JoinStyleMiter ), //!< Use mitered joins
      Bevel SIP_MONKEYPATCH_COMPAT_NAME( JoinStyleBevel ), //!< Use beveled joins
    };
    Q_ENUM( JoinStyle )

    /**
     * Coverage validity results.
     *
     * \since QGIS 3.36
     */
    enum class CoverageValidityResult : int
    {
      Invalid = 0, //!< Coverage is invalid. Invalidity includes polygons that overlap, that have gaps smaller than the gap width, or non-polygonal entries in the input collection.
      Valid = 1, //!< Coverage is valid
      Error = 2, //!< An exception occurred while determining validity
    };
    Q_ENUM( CoverageValidityResult )

    /**
     * Algorithms to use when repairing invalid geometries.
     *
     * \since QGIS 3.28
     */
    enum class MakeValidMethod : int
    {
      Linework = 0, //!< Combines all rings into a set of noded lines and then extracts valid polygons from that linework.
      Structure = 1, //!< Structured method, first makes all rings valid and then merges shells and subtracts holes from shells to generate valid result. Assumes that holes and shells are correctly categorized. Requires GEOS 3.10+.
    };
    Q_ENUM( MakeValidMethod )

    /**
     * Flags for controlling feature requests.
     *
     * \note Prior to QGIS 3.36 this was available as QgsFeatureRequest::Flag
     *
     * \since QGIS 3.36
     */
    enum class FeatureRequestFlag SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsFeatureRequest, Flag ) : int SIP_ENUM_BASETYPE( IntFlag )
    {
      NoFlags            = 0, //!< No flags are set
      NoGeometry         = 1,  //!< Geometry is not required. It may still be returned if e.g. required for a filter condition.
      SubsetOfAttributes = 2,  //!< Fetch only a subset of attributes (setSubsetOfAttributes sets this flag)
      ExactIntersect     = 4,   //!< Use exact geometry intersection (slower) instead of bounding boxes
      IgnoreStaticNodesDuringExpressionCompilation = 8, //!< If a feature request uses a filter expression which can be partially precalculated due to static nodes in the expression, setting this flag will prevent these precalculated values from being utilized during compilation of the filter for the backend provider. This flag significantly slows down feature requests and should be used for debugging purposes only. (Since QGIS 3.18)
      EmbeddedSymbols    = 16,  //!< Retrieve any embedded feature symbology (since QGIS 3.20)
    };
    Q_ENUM( FeatureRequestFlag )

    /**
     * Flags for controlling feature requests.
     *
     * \note Prior to QGIS 3.36 this was available as QgsFeatureRequest::Flags
     *
     * \since QGIS 3.36
     */
    Q_DECLARE_FLAGS( FeatureRequestFlags, FeatureRequestFlag ) SIP_MONKEYPATCH_FLAGS_UNNEST( QgsFeatureRequest, Flags )
    Q_FLAG( FeatureRequestFlags )

    /**
     * Types of feature request filters.
     *
     * \note Prior to QGIS 3.36 this was available as QgsFeatureRequest::FilterType
     *
     * \since QGIS 3.36
     */
    enum class FeatureRequestFilterType SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsFeatureRequest, FilterType ) : int
      {
      NoFilter SIP_MONKEYPATCH_COMPAT_NAME( FilterNone ), //!< No filter is applied
      Fid SIP_MONKEYPATCH_COMPAT_NAME( FilterFid ), //!< Filter using feature ID
      Expression SIP_MONKEYPATCH_COMPAT_NAME( FilterExpression ), //!< Filter using expression
      Fids SIP_MONKEYPATCH_COMPAT_NAME( FilterFids ) //!< Filter using feature IDs
    };
    Q_ENUM( FeatureRequestFilterType )

    /**
     * Methods for handling of features with invalid geometries
     *
     * \note Prior to QGIS 3.36 this was available as QgsFeatureRequest::InvalidGeometryCheck
     *
     * \since QGIS 3.36
     */
    enum class InvalidGeometryCheck SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsFeatureRequest, InvalidGeometryCheck ) : int
      {
      NoCheck SIP_MONKEYPATCH_COMPAT_NAME( GeometryNoCheck ) = 0, //!< No invalid geometry checking
      SkipInvalid SIP_MONKEYPATCH_COMPAT_NAME( GeometrySkipInvalid ) = 1, //!< Skip any features with invalid geometry. This requires a slow geometry validity check for every feature.
      AbortOnInvalid SIP_MONKEYPATCH_COMPAT_NAME( GeometryAbortOnInvalid ) = 2, //!< Close iterator on encountering any features with invalid geometry. This requires a slow geometry validity check for every feature.
    };
    Q_ENUM( InvalidGeometryCheck )

    /**
     * Feature request spatial filter types.
     *
     * \since QGIS 3.22
     */
    enum class SpatialFilterType : int
    {
      NoFilter, //!< No spatial filtering of features
      BoundingBox, //!< Filter using a bounding box
      DistanceWithin, //!< Filter by distance to reference geometry
    };
    Q_ENUM( SpatialFilterType )

    /**
     * File operation flags.
     *
     * \since QGIS 3.22
     */
    enum class FileOperationFlag : int SIP_ENUM_BASETYPE( IntFlag )
    {
      IncludeMetadataFile = 1 << 0, //!< Indicates that any associated .qmd metadata file should be included with the operation
      IncludeStyleFile = 1 << 1, //!< Indicates that any associated .qml styling file should be included with the operation
    };
    //! File operation flags
    Q_DECLARE_FLAGS( FileOperationFlags, FileOperationFlag )
    Q_ENUM( FileOperationFlag )
    Q_FLAG( FileOperationFlags )

    /**
     * Generic map layer properties.
     *
     * \since QGIS 3.22
     */
    enum class MapLayerProperty : int SIP_ENUM_BASETYPE( IntFlag )
    {
      UsersCannotToggleEditing = 1 << 0, //!< Indicates that users are not allowed to toggle editing for this layer. Note that this does not imply that the layer is non-editable (see isEditable(), supportsEditing() ), rather that the editable status of the layer cannot be changed by users manually. Since QGIS 3.22.
      IsBasemapLayer = 1 << 1, //!< Layer is considered a 'basemap' layer, and certain properties of the layer should be ignored when calculating project-level properties. For instance, the extent of basemap layers is ignored when calculating the extent of a project, as these layers are typically global and extend outside of a project's area of interest. Since QGIS 3.26.
    };
    //! Map layer properties
    Q_DECLARE_FLAGS( MapLayerProperties, MapLayerProperty )
    Q_ENUM( MapLayerProperty )
    Q_FLAG( MapLayerProperties )

    /**
     * Map layer automatic refresh modes.
     *
     * \since QGIS 3.34
     */
    enum class AutoRefreshMode : int
    {
      Disabled = 0, //!< Automatic refreshing is disabled
      ReloadData = 1, //!< Reload data (and draw the new data)
      RedrawOnly = 2, //!< Redraw current data only
    };
    Q_ENUM( AutoRefreshMode )

    /**
     * Generic data provider flags.
     *
     * \since QGIS 3.26
     */
    enum class DataProviderFlag : int SIP_ENUM_BASETYPE( IntFlag )
    {
      IsBasemapSource = 1 << 1, //!< Associated source should be considered a 'basemap' layer. See Qgis::MapLayerProperty::IsBasemapLayer.
      FastExtent2D = 1 << 2, //!< Provider's 2D extent retrieval via QgsDataProvider::extent() is always guaranteed to be trivial/fast to calculate. Since QGIS 3.38.
      FastExtent3D = 1 << 3, //!< Provider's 3D extent retrieval via QgsDataProvider::extent3D() is always guaranteed to be trivial/fast to calculate. Since QGIS 3.38.
    };
    //! Data provider flags
    Q_DECLARE_FLAGS( DataProviderFlags, DataProviderFlag )
    Q_ENUM( DataProviderFlag )
    Q_FLAG( DataProviderFlags )

    /**
     * Coordinate reference system types.
     *
     * Contains a subset of Proj's PJ_TYPE enum, specifically the types which relate to CRS types.
     *
     * \since QGIS 3.34
     */
    enum class CrsType : int
    {
      Unknown, //!< Unknown type
      Geodetic, //!< Geodetic CRS
      Geocentric, //!< Geocentric CRS
      Geographic2d, //!< 2D geographic CRS
      Geographic3d, //!< 3D geopraphic CRS
      Vertical, //!< Vertical CRS
      Projected, //!< Projected CRS
      Compound, //!< Compound (horizontal + vertical) CRS
      Temporal, //!< Temporal CRS
      Engineering, //!< Engineering CRS
      Bound, //!< Bound CRS
      Other, //!< Other type
      DerivedProjected, //!< Derived projected CRS
    };
    Q_ENUM( CrsType )

    /**
     * Coordinate reference system axis directions.
     *
     * From "Geographic information — Well-known text representation of coordinate reference systems", section 7.5.1.
     *
     * \since QGIS 3.26
     */
    enum class CrsAxisDirection : int
    {
      North, //!< North
      NorthNorthEast, //!< North North East
      NorthEast, //!< North East
      EastNorthEast, //!< East North East
      East, //!< East
      EastSouthEast, //!< East South East
      SouthEast, //!< South East
      SouthSouthEast, //!< South South East
      South, //!< South
      SouthSouthWest, //!< South South West
      SouthWest, //!< South West
      WestSouthWest, //!< West South West
      West, //!< West
      WestNorthWest, //!< West North West
      NorthWest, //!< North West
      NorthNorthWest, //!< North North West
      GeocentricX, //!< Geocentric (X)
      GeocentricY, //!< Geocentric (Y)
      GeocentricZ, //!< Geocentric (Z)
      Up, //!< Up
      Down, //!< Down
      Forward, //!< Forward
      Aft, //!< Aft
      Port, //!< Port
      Starboard, //!< Starboard
      Clockwise, //!< Clockwise
      CounterClockwise, //!< Counter clockwise
      ColumnPositive, //!< Column positive
      ColumnNegative, //!< Column negative
      RowPositive, //!< Row positive
      RowNegative, //!< Row negative
      DisplayRight, //!< Display right
      DisplayLeft, //!< Display left
      DisplayUp, //!< Display up
      DisplayDown, //!< Display down
      Future, //!< Future
      Past, //!< Past
      Towards, //!< Towards
      AwayFrom, //!< Away from
      Unspecified, //!< Unspecified
    };
    Q_ENUM( CrsAxisDirection )

    /**
     * Order of coordinates.
     *
     * \since QGIS 3.26
     */
    enum class CoordinateOrder : int
    {
      Default, //!< Respect the default axis ordering for the CRS, as defined in the CRS's parameters
      XY, //!< Easting/Northing (or Longitude/Latitude for geographic CRS)
      YX, //!< Northing/Easting (or Latitude/Longitude for geographic CRS)
    };
    Q_ENUM( CoordinateOrder )

    /**
     * Available identifier string types for representing coordinate reference systems
     *
     * \note Prior to QGIS 3.36 this was available as QgsCoordinateReferenceSystem::IdentifierType
     *
     * \since QGIS 3.36
     */
    enum class CrsIdentifierType SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsCoordinateReferenceSystem, IdentifierType ) : int
      {
      ShortString, //!< A heavily abbreviated string, for use when a compact representation is required
      MediumString, //!< A medium-length string, recommended for general purpose use
      FullString, //!< Full definition -- possibly a very lengthy string, e.g. with no truncation of custom WKT definitions
    };
    Q_ENUM( CrsIdentifierType )

    /**
     * Coordinate reference system WKT formatting variants.
     *
     * \note Prior to QGIS 3.36 this was available as QgsCoordinateReferenceSystem::WktVariant
     *
     * \since QGIS 3.36
     */
    enum class CrsWktVariant SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsCoordinateReferenceSystem, WktVariant ) : int
      {
      Wkt1Gdal SIP_MONKEYPATCH_COMPAT_NAME( WKT1_GDAL ), //!< WKT1 as traditionally output by GDAL, deriving from OGC 01-009. A notable departure from WKT1_GDAL with respect to OGC 01-009 is that in WKT1_GDAL, the unit of the PRIMEM value is always degrees.
      Wkt1Esri SIP_MONKEYPATCH_COMPAT_NAME( WKT1_ESRI ), //!< WKT1 as traditionally output by ESRI software, deriving from OGC 99-049.
      Wkt2_2015 SIP_MONKEYPATCH_COMPAT_NAME( WKT2_2015 ), //!< Full WKT2 string, conforming to ISO 19162:2015(E) / OGC 12-063r5 with all possible nodes and new keyword names.
      Wkt2_2015Simplified SIP_MONKEYPATCH_COMPAT_NAME( WKT2_2015_SIMPLIFIED ), //!< Same as WKT2_2015 with the following exceptions: UNIT keyword used. ID node only on top element. No ORDER element in AXIS element. PRIMEM node omitted if it is Greenwich.  ELLIPSOID.UNIT node omitted if it is UnitOfMeasure::METRE. PARAMETER.UNIT / PRIMEM.UNIT omitted if same as AXIS. AXIS.UNIT omitted and replaced by a common GEODCRS.UNIT if they are all the same on all axis.
      Wkt2_2019  SIP_MONKEYPATCH_COMPAT_NAME( WKT2_2019 ), //!< Full WKT2 string, conforming to ISO 19162:2019 / OGC 18-010, with all possible nodes and new keyword names. Non-normative list of differences: WKT2_2019 uses GEOGCRS / BASEGEOGCRS keywords for GeographicCRS.
      Wkt2_2019Simplified  SIP_MONKEYPATCH_COMPAT_NAME( WKT2_2019_SIMPLIFIED ), //!< WKT2_2019 with the simplification rule of WKT2_SIMPLIFIED
      Preferred SIP_MONKEYPATCH_COMPAT_NAME( WKT_PREFERRED ) = Wkt2_2019, //!< Preferred format, matching the most recent WKT ISO standard. Currently an alias to WKT2_2019, but may change in future versions.
      PreferredSimplified  SIP_MONKEYPATCH_COMPAT_NAME( WKT_PREFERRED_SIMPLIFIED ) = Wkt2_2019Simplified, //!< Preferred simplified format, matching the most recent WKT ISO standard. Currently an alias to WKT2_2019_SIMPLIFIED, but may change in future versions.
      PreferredGdal SIP_MONKEYPATCH_COMPAT_NAME( WKT_PREFERRED_GDAL ) = Wkt2_2019, //!< Preferred format for conversion of CRS to WKT for use with the GDAL library.
    };
    Q_ENUM( CrsWktVariant )

    /**
     * Cartesian axes.
     *
     * \since QGIS 3.34
     */
    enum class Axis : int
    {
      X, //!< X-axis
      Y, //!< Y-axis
      Z //!< Z-axis
    };
    Q_ENUM( Axis )

    /**
     * Flags for annotation items.
     *
     * \since QGIS 3.22
     */
    enum class AnnotationItemFlag : int SIP_ENUM_BASETYPE( IntFlag )
    {
      ScaleDependentBoundingBox = 1 << 0, //!< Item's bounding box will vary depending on map scale
    };
    //! Annotation item flags
    Q_DECLARE_FLAGS( AnnotationItemFlags, AnnotationItemFlag )
    Q_ENUM( AnnotationItemFlag )
    Q_FLAG( AnnotationItemFlags )

    /**
     * Flags for controlling how an annotation item behaves in the GUI.
     *
     * \since QGIS 3.22
     */
    enum class AnnotationItemGuiFlag : int SIP_ENUM_BASETYPE( IntFlag )
    {
      FlagNoCreationTools = 1 << 0,  //!< Do not show item creation tools for the item type
    };
    //! Annotation item GUI flags
    Q_DECLARE_FLAGS( AnnotationItemGuiFlags, AnnotationItemGuiFlag )
    Q_ENUM( AnnotationItemGuiFlag )
    Q_FLAG( AnnotationItemGuiFlags )

    /**
     * Annotation item node types.
     *
     * \since QGIS 3.22
     */
    enum class AnnotationItemNodeType : int
    {
      VertexHandle, //!< Node is a handle for manipulating vertices
    };
    Q_ENUM( AnnotationItemNodeType )

    /**
     * Results from an edit operation on an annotation item.
     *
     * \since QGIS 3.22
     */
    enum class AnnotationItemEditOperationResult : int
    {
      Success, //!< Item was modified successfully
      Invalid, //!< Operation has invalid parameters for the item, no change occurred
      ItemCleared, //!< The operation results in the item being cleared, and the item should be removed from the layer as a result
    };
    Q_ENUM( AnnotationItemEditOperationResult )

    /**
     * Temporal navigation modes.
     *
     * \note Prior to QGIS 3.36 this was available as QgsTemporalNavigationObject::NavigationMode
     *
     * \since QGIS 3.36
     */
    enum class TemporalNavigationMode SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsTemporalNavigationObject, NavigationMode ) : int
      {
      Disabled SIP_MONKEYPATCH_COMPAT_NAME( NavigationOff ), //!< Temporal navigation is disabled
      Animated, //!< Temporal navigation relies on frames within a datetime range
      FixedRange, //!< Temporal navigation relies on a fixed datetime range
      Movie, //!< Movie mode -- behaves like a video player, with a fixed frame duration and no temporal range (since QGIS 3.36)
    };
    Q_ENUM( TemporalNavigationMode )

    /**
     * Animation states.
     *
     * \note Prior to QGIS 3.36 this was available as QgsTemporalNavigationObject::AnimationState
     *
     * \since QGIS 3.36
     */
    enum class AnimationState SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsTemporalNavigationObject, AnimationState ) : int
      {
      Forward, //!< Animation is playing forward.
      Reverse, //!< Animation is playing in reverse.
      Idle, //!< Animation is paused.
    };
    Q_ENUM( AnimationState )

    /**
     * Media playback operations.
     *
     * \since QGIS 3.36
     */
    enum class PlaybackOperation : int
    {
      SkipToStart, //!< Jump to start of playback
      PreviousFrame, //!< Step to previous frame
      PlayReverse, //!< Play in reverse
      Pause, //!< Pause playback
      PlayForward, //!< Play forward
      NextFrame, //!< Step to next frame
      SkipToEnd, //!< Jump to end of playback
    };
    Q_ENUM( PlaybackOperation )

    /**
     * Vector layer temporal feature modes
     *
     * \since QGIS 3.22
     */
    enum class VectorTemporalMode SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsVectorLayerTemporalProperties, TemporalMode ) : int
      {
      FixedTemporalRange SIP_MONKEYPATCH_COMPAT_NAME( ModeFixedTemporalRange ) = 0, //!< Mode when temporal properties have fixed start and end datetimes.
      FeatureDateTimeInstantFromField SIP_MONKEYPATCH_COMPAT_NAME( ModeFeatureDateTimeInstantFromField ), //!< Mode when features have a datetime instant taken from a single field
      FeatureDateTimeStartAndEndFromFields SIP_MONKEYPATCH_COMPAT_NAME( ModeFeatureDateTimeStartAndEndFromFields ), //!< Mode when features have separate fields for start and end times
      FeatureDateTimeStartAndDurationFromFields SIP_MONKEYPATCH_COMPAT_NAME( ModeFeatureDateTimeStartAndDurationFromFields ), //!< Mode when features have a field for start time and a field for event duration
      FeatureDateTimeStartAndEndFromExpressions SIP_MONKEYPATCH_COMPAT_NAME( ModeFeatureDateTimeStartAndEndFromExpressions ), //!< Mode when features use expressions for start and end times
      RedrawLayerOnly SIP_MONKEYPATCH_COMPAT_NAME( ModeRedrawLayerOnly ), //!< Redraw the layer when temporal range changes, but don't apply any filtering. Useful when symbology or rule based renderer expressions depend on the time range.
    };
    Q_ENUM( VectorTemporalMode )

    /**
     * Mode for the handling of the limits of the filtering timeframe for vector features
     *
     * \since QGIS 3.22
     */
    enum class VectorTemporalLimitMode : int
    {
      IncludeBeginExcludeEnd = 0, //!< Default mode: include the Begin limit, but exclude the End limit
      IncludeBeginIncludeEnd, //!< Mode to include both limits of the filtering timeframe
    };
    Q_ENUM( VectorTemporalLimitMode )

    /**
     * Vector data provider temporal handling modes.
     *
     * \since QGIS 3.22
     */
    enum class VectorDataProviderTemporalMode SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsVectorDataProviderTemporalCapabilities, TemporalMode ) : int
      {
      HasFixedTemporalRange SIP_MONKEYPATCH_COMPAT_NAME( ProviderHasFixedTemporalRange ) = 0, //!< Entire dataset from provider has a fixed start and end datetime.
      StoresFeatureDateTimeInstantInField SIP_MONKEYPATCH_COMPAT_NAME( ProviderStoresFeatureDateTimeInstantInField ), //!< Dataset has feature datetime instants stored in a single field
      StoresFeatureDateTimeStartAndEndInSeparateFields SIP_MONKEYPATCH_COMPAT_NAME( ProviderStoresFeatureDateTimeStartAndEndInSeparateFields ), //!< Dataset stores feature start and end datetimes in separate fields
    };
    Q_ENUM( VectorDataProviderTemporalMode )

    /**
     * Raster layer temporal modes
     *
     * \since QGIS 3.22
     */
    enum class RasterTemporalMode SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsRasterLayerTemporalProperties, TemporalMode ) : int
      {
      FixedTemporalRange SIP_MONKEYPATCH_COMPAT_NAME( ModeFixedTemporalRange ) = 0, //!< Mode when temporal properties have fixed start and end datetimes.
      TemporalRangeFromDataProvider SIP_MONKEYPATCH_COMPAT_NAME( ModeTemporalRangeFromDataProvider ) = 1, //!< Mode when raster layer delegates temporal range handling to the dataprovider.
      RedrawLayerOnly SIP_MONKEYPATCH_COMPAT_NAME( ModeRedrawLayerOnly ) = 2, //!< Redraw the layer when temporal range changes, but don't apply any filtering. Useful when raster symbology expressions depend on the time range. (since QGIS 3.22)
      FixedRangePerBand = 3, //!< Layer has a fixed temporal range per band (since QGIS 3.38)
    };
    Q_ENUM( RasterTemporalMode )

    /**
     * Method to use when resolving a temporal range to a data provider layer or band.
     *
     * \since QGIS 3.22
     */
    enum class TemporalIntervalMatchMethod SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsRasterDataProviderTemporalCapabilities, IntervalHandlingMethod ) : int
      {
      MatchUsingWholeRange, //!< Use an exact match to the whole temporal range
      MatchExactUsingStartOfRange, //!< Match the start of the temporal range to a corresponding layer or band, and only use exact matching results
      MatchExactUsingEndOfRange, //!< Match the end of the temporal range to a corresponding layer or band, and only use exact matching results
      FindClosestMatchToStartOfRange, //!< Match the start of the temporal range to the least previous closest datetime.
      FindClosestMatchToEndOfRange //!< Match the end of the temporal range to the least previous closest datetime.
    };
    Q_ENUM( TemporalIntervalMatchMethod )

    /**
     * Flags for raster layer temporal capabilities.
     *
     * \since QGIS 3.28
     */
    enum class RasterTemporalCapabilityFlag : int SIP_ENUM_BASETYPE( IntFlag )
    {
      RequestedTimesMustExactlyMatchAllAvailableTemporalRanges = 1 << 0, //!< If present, indicates that the provider must only request temporal values which are exact matches for the values present in QgsRasterDataProviderTemporalCapabilities::allAvailableTemporalRanges().
    };
    Q_ENUM( RasterTemporalCapabilityFlag )

    /**
     * Flags for raster layer temporal capabilities.
     *
     * \since QGIS 3.28
     */
    Q_DECLARE_FLAGS( RasterTemporalCapabilityFlags, RasterTemporalCapabilityFlag )
    Q_FLAG( RasterTemporalCapabilityFlags )

    /**
     * Indicates the direction (forward or inverse) of a transform.
     *
     * \since QGIS 3.22
     */
    enum class TransformDirection SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsCoordinateTransform, TransformDirection ) : int
      {
      Forward SIP_MONKEYPATCH_COMPAT_NAME( ForwardTransform ), //!< Forward transform (from source to destination)
      Reverse SIP_MONKEYPATCH_COMPAT_NAME( ReverseTransform ) //!< Reverse/inverse transform (from destination to source)
    };
    Q_ENUM( TransformDirection )

    /**
     * Flags which adjust the coordinate transformations behave.
     *
     * \since QGIS 3.26
     */
    enum class CoordinateTransformationFlag  : int SIP_ENUM_BASETYPE( IntFlag )
    {
      BallparkTransformsAreAppropriate = 1 << 0, //!< Indicates that approximate "ballpark" results are appropriate for this coordinate transform. See QgsCoordinateTransform::setBallparkTransformsAreAppropriate() for further details.
      IgnoreImpossibleTransformations = 1 << 1, //!< Indicates that impossible transformations (such as those which attempt to transform between two different celestial bodies) should be silently handled and marked as invalid. See QgsCoordinateTransform::isTransformationPossible() and QgsCoordinateTransform::isValid().
    };
    Q_ENUM( CoordinateTransformationFlag )

    /**
     * Coordinate transformation flags.
     *
     * \since QGIS 3.26
     */
    Q_DECLARE_FLAGS( CoordinateTransformationFlags, CoordinateTransformationFlag )
    Q_FLAG( CoordinateTransformationFlags )

    /**
     * Flags which adjust the way maps are rendered.
     *
     * \since QGIS 3.22
     */
    enum class MapSettingsFlag SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsMapSettings, Flag ) : int SIP_ENUM_BASETYPE( IntFlag )
    {
      Antialiasing             = 0x01,  //!< Enable anti-aliasing for map rendering
      DrawEditingInfo          = 0x02,  //!< Enable drawing of vertex markers for layers in editing mode
      ForceVectorOutput        = 0x04,  //!< Vector graphics should not be cached and drawn as raster images
      UseAdvancedEffects       = 0x08,  //!< Enable layer opacity and blending effects
      DrawLabeling             = 0x10,  //!< Enable drawing of labels on top of the map
      UseRenderingOptimization = 0x20,  //!< Enable vector simplification and other rendering optimizations
      DrawSelection            = 0x40,  //!< Whether vector selections should be shown in the rendered map
      DrawSymbolBounds         = 0x80,  //!< Draw bounds of symbols (for debugging/testing)
      RenderMapTile            = 0x100, //!< Draw map such that there are no problems between adjacent tiles
      RenderPartialOutput      = 0x200, //!< Whether to make extra effort to update map image with partially rendered layers (better for interactive map canvas). Added in QGIS 3.0
      RenderPreviewJob         = 0x400, //!< Render is a 'canvas preview' render, and shortcuts should be taken to ensure fast rendering
      RenderBlocking           = 0x800, //!< Render and load remote sources in the same thread to ensure rendering remote sources (svg and images). WARNING: this flag must NEVER be used from GUI based applications (like the main QGIS application) or crashes will result. Only for use in external scripts or QGIS server.
      LosslessImageRendering   = 0x1000, //!< Render images losslessly whenever possible, instead of the default lossy jpeg rendering used for some destination devices (e.g. PDF). This flag only works with builds based on Qt 5.13 or later.
      Render3DMap              = 0x2000, //!< Render is for a 3D map
      HighQualityImageTransforms = 0x4000, //!< Enable high quality image transformations, which results in better appearance of scaled or rotated raster components of a map (since QGIS 3.24)
      SkipSymbolRendering      = 0x8000, //!< Disable symbol rendering while still drawing labels if enabled (since QGIS 3.24)
      ForceRasterMasks         = 0x10000,  //!< Force symbol masking to be applied using a raster method. This is considerably faster when compared to the vector method, but results in a inferior quality output. (since QGIS 3.26.1)
      RecordProfile            = 0x20000, //!< Enable run-time profiling while rendering (since QGIS 3.34)
    };
    //! Map settings flags
    Q_DECLARE_FLAGS( MapSettingsFlags, MapSettingsFlag ) SIP_MONKEYPATCH_FLAGS_UNNEST( QgsMapSettings, Flags )
    Q_ENUM( MapSettingsFlag )
    Q_FLAG( MapSettingsFlags )

    /**
     * Flags which affect rendering operations.
     *
     * \since QGIS 3.22
     */
    enum class RenderContextFlag SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsRenderContext, Flag ) : int SIP_ENUM_BASETYPE( IntFlag )
    {
      DrawEditingInfo          = 0x01,  //!< Enable drawing of vertex markers for layers in editing mode
      ForceVectorOutput        = 0x02,  //!< Vector graphics should not be cached and drawn as raster images
      UseAdvancedEffects       = 0x04,  //!< Enable layer opacity and blending effects
      UseRenderingOptimization = 0x08,  //!< Enable vector simplification and other rendering optimizations
      DrawSelection            = 0x10,  //!< Whether vector selections should be shown in the rendered map
      DrawSymbolBounds         = 0x20,  //!< Draw bounds of symbols (for debugging/testing)
      RenderMapTile            = 0x40,  //!< Draw map such that there are no problems between adjacent tiles
      Antialiasing             = 0x80,  //!< Use antialiasing while drawing
      RenderPartialOutput      = 0x100, //!< Whether to make extra effort to update map image with partially rendered layers (better for interactive map canvas). Added in QGIS 3.0
      RenderPreviewJob         = 0x200, //!< Render is a 'canvas preview' render, and shortcuts should be taken to ensure fast rendering
      RenderBlocking           = 0x400, //!< Render and load remote sources in the same thread to ensure rendering remote sources (svg and images). WARNING: this flag must NEVER be used from GUI based applications (like the main QGIS application) or crashes will result. Only for use in external scripts or QGIS server.
      RenderSymbolPreview      = 0x800, //!< The render is for a symbol preview only and map based properties may not be available, so care should be taken to handle map unit based sizes in an appropriate way.
      LosslessImageRendering   = 0x1000, //!< Render images losslessly whenever possible, instead of the default lossy jpeg rendering used for some destination devices (e.g. PDF). This flag only works with builds based on Qt 5.13 or later.
      ApplyScalingWorkaroundForTextRendering = 0x2000, //!< Whether a scaling workaround designed to stablise the rendering of small font sizes (or for painters scaled out by a large amount) when rendering text. Generally this is recommended, but it may incur some performance cost.
      Render3DMap              = 0x4000, //!< Render is for a 3D map
      ApplyClipAfterReprojection = 0x8000, //!< Feature geometry clipping to mapExtent() must be performed after the geometries are transformed using coordinateTransform(). Usually feature geometry clipping occurs using the extent() in the layer's CRS prior to geometry transformation, but in some cases when extent() could not be accurately calculated it is necessary to clip geometries to mapExtent() AFTER transforming them using coordinateTransform().
      RenderingSubSymbol       = 0x10000, //!< Set whenever a sub-symbol of a parent symbol is currently being rendered. Can be used during symbol and symbol layer rendering to determine whether the symbol being rendered is a subsymbol. (Since QGIS 3.24)
      HighQualityImageTransforms = 0x20000, //!< Enable high quality image transformations, which results in better appearance of scaled or rotated raster components of a map (since QGIS 3.24)
      SkipSymbolRendering      = 0x40000, //!< Disable symbol rendering while still drawing labels if enabled (since QGIS 3.24)
      RecordProfile            = 0x80000, //!< Enable run-time profiling while rendering (since QGIS 3.34)
    };
    //! Render context flags
    Q_DECLARE_FLAGS( RenderContextFlags, RenderContextFlag ) SIP_MONKEYPATCH_FLAGS_UNNEST( QgsRenderContext, Flags )
    Q_ENUM( RenderContextFlag )
    Q_FLAG( RenderContextFlags )

    /**
     * Flags which control how map layer renderers behave.
     *
     * \since QGIS 3.34
     */
    enum class MapLayerRendererFlag : int SIP_ENUM_BASETYPE( IntFlag )
    {
      RenderPartialOutputs = 1 << 0,  //!< The renderer benefits from rendering temporary in-progress preview renders. These are temporary results which will be used for the layer during rendering in-progress compositions, which will differ from the final layer render. They can be used for showing overlays or other information to users which help inform them about what is actually occurring during a slow layer render, but where these overlays and additional content is not wanted in the final layer renders. Another use case is rendering unsorted results as soon as they are available, before doing a final sorted render of the entire layer contents.
      RenderPartialOutputOverPreviousCachedImage = 1 << 1,//!< When rendering temporary in-progress preview renders, these preview renders can be drawn over any previously cached layer render we have for the same region. This can allow eg a low-resolution zoomed in version of the last map render to be used as a base painting surface to overdraw with incremental preview render outputs. If not set, an empty image will be used as the starting point for the render preview image.
    };
    Q_ENUM( MapLayerRendererFlag )

    /**
     * Flags which control how map layer renderers behave.
     *
     * \since QGIS 3.34
     */
    Q_DECLARE_FLAGS( MapLayerRendererFlags, MapLayerRendererFlag )
    Q_FLAG( MapLayerRendererFlags )

    // refs for below dox: https://github.com/qgis/QGIS/pull/1286#issuecomment-39806854
    // https://github.com/qgis/QGIS/pull/8573#issuecomment-445585826

    /**
     * Options for rendering text.
     * \since QGIS 3.22
     */
    enum class TextRenderFormat SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsRenderContext, TextRenderFormat ) : int
      {
      AlwaysOutlines SIP_MONKEYPATCH_COMPAT_NAME( TextFormatAlwaysOutlines ), //!< Always render text using path objects (AKA outlines/curves). This setting guarantees the best quality rendering, even when using a raster paint surface (where sub-pixel path based text rendering is superior to sub-pixel text-based rendering). The downside is that text is converted to paths only, so users cannot open created vector outputs for post-processing in other applications and retain text editability.  This setting also guarantees complete compatibility with the full range of formatting options available through QgsTextRenderer and QgsTextFormat, some of which may not be possible to reproduce when using a vector-based paint surface and TextFormatAlwaysText mode. A final benefit to this setting is that vector exports created using text as outlines do not require all users to have the original fonts installed in order to display the text in its original style.
      AlwaysText SIP_MONKEYPATCH_COMPAT_NAME( TextFormatAlwaysText ), //!< Always render text as text objects. While this mode preserves text objects as text for post-processing in external vector editing applications, it can result in rendering artifacts or poor quality rendering, depending on the text format settings. Even with raster based paint devices, TextFormatAlwaysText can result in inferior rendering quality to TextFormatAlwaysOutlines. When rendering using TextFormatAlwaysText to a vector based device (e.g. PDF or SVG), care must be taken to ensure that the required fonts are available to users when opening the created files, or default fallback fonts will be used to display the output instead. (Although PDF exports MAY automatically embed some fonts when possible, depending on the user's platform).
    };
    Q_ENUM( TextRenderFormat )

    /**
     * Various flags that affect drawing and placement of labels.
     *
     * Prior to QGIS 3.30 this was available as QgsLabelingEngineSettings::Flag
     *
     * \since QGIS 3.30
     */
    enum class LabelingFlag SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsLabelingEngineSettings, Flag ) : int SIP_ENUM_BASETYPE( IntFlag )
    {
      UseAllLabels          = 1 << 1, //!< Whether to draw all labels even if there would be collisions
      UsePartialCandidates  = 1 << 2, //!< Whether to use also label candidates that are partially outside of the map view
      // TODO QGIS 4.0: remove
      RenderOutlineLabels   = 1 << 3, //!< Whether to render labels as text or outlines. Deprecated and of QGIS 3.4.3 - use defaultTextRenderFormat() instead.
      DrawLabelRectOnly     = 1 << 4, //!< Whether to only draw the label rect and not the actual label text (used for unit tests)
      DrawCandidates        = 1 << 5, //!< Whether to draw rectangles of generated candidates (good for debugging)
      DrawUnplacedLabels    = 1 << 6, //!< Whether to render unplaced labels as an indicator/warning for users
      CollectUnplacedLabels = 1 << 7, //!< Whether unplaced labels should be collected in the labeling results (regardless of whether they are being rendered). Since QGIS 3.20
      DrawLabelMetrics      = 1 << 8, //!< Whether to render label metric guides (for debugging). Since QGIS 3.30
    };
    Q_ENUM( LabelingFlag )

    /**
     * Flags that affect drawing and placement of labels.
     *
     * Prior to QGIS 3.30 this was available as QgsLabelingEngineSettings::Flags
     *
     * \since QGIS 3.30
     */
    Q_DECLARE_FLAGS( LabelingFlags, LabelingFlag ) SIP_MONKEYPATCH_FLAGS_UNNEST( QgsLabelingEngineSettings, Flags )
    Q_FLAG( LabelingFlags )

    /**
     * Labeling placement engine version.
     *
     * Prior to QGIS 3.30 this was available as QgsLabelingEngineSettings::PlacementEngineVersion
     *
     * \since QGIS 3.30
     */
    enum class LabelPlacementEngineVersion SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsLabelingEngineSettings, PlacementEngineVersion ) : int
      {
      Version1 SIP_MONKEYPATCH_COMPAT_NAME( PlacementEngineVersion1 ), //!< Version 1, matches placement from QGIS <= 3.10.1
      Version2 SIP_MONKEYPATCH_COMPAT_NAME( PlacementEngineVersion2 ), //!< Version 2 (default for new projects since QGIS 3.12)
    };
    Q_ENUM( LabelPlacementEngineVersion )

    /**
     * Text orientations.
     *
     * \note Prior to QGIS 3.28 this was available as QgsTextFormat::TextOrientation
     *
     * \since QGIS 3.28
     */
    enum class TextOrientation SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsTextFormat, TextOrientation ) : int
      {
      Horizontal SIP_MONKEYPATCH_COMPAT_NAME( HorizontalOrientation ), //!< Horizontally oriented text
      Vertical SIP_MONKEYPATCH_COMPAT_NAME( VerticalOrientation ), //!< Vertically oriented text
      RotationBased SIP_MONKEYPATCH_COMPAT_NAME( RotationBasedOrientation ), //!< Horizontally or vertically oriented text based on rotation (only available for map labeling)
    };
    Q_ENUM( TextOrientation )

    /**
     * Text layout modes.
     *
     * \note Prior to QGIS 3.28 this was available as QgsTextRenderer::DrawMode
     *
     * \since QGIS 3.28
     */
    enum class TextLayoutMode SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsTextRenderer, DrawMode ) : int
      {
      Rectangle SIP_MONKEYPATCH_COMPAT_NAME( Rect ), //!< Text within rectangle layout mode
      Point, //!< Text at point of origin layout mode
      Labeling SIP_MONKEYPATCH_COMPAT_NAME( Label ), //!< Labeling-specific layout mode
      RectangleCapHeightBased, //!< Similar to Rectangle mode, but uses cap height only when calculating font heights for the first line of text, and cap height + descent for subsequent lines of text (since QGIS 3.30)
      RectangleAscentBased, //!< Similar to Rectangle mode, but uses ascents only when calculating font and line heights. (since QGIS 3.30)
    };
    Q_ENUM( TextLayoutMode )

    /**
     * Text components.
     *
     * \note Prior to QGIS 3.28 this was available as QgsTextRenderer::TextPart
     *
     * \since QGIS 3.28
     */
    enum class TextComponent SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsTextRenderer, TextPart ) : int
      {
      Text, //!< Text component
      Buffer, //!< Buffer component
      Background, //!< Background shape
      Shadow, //!< Drop shadow
    };
    Q_ENUM( TextComponent )

    /**
     * Text horizontal alignment.
     *
     * \note Prior to QGIS 3.28 this was available as QgsTextRenderer::HAlignment
     *
     * \since QGIS 3.28
     */
    enum class TextHorizontalAlignment SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsTextRenderer, HAlignment ) : int
      {
      Left SIP_MONKEYPATCH_COMPAT_NAME( AlignLeft ), //!< Left align
      Center SIP_MONKEYPATCH_COMPAT_NAME( AlignCenter ), //!< Center align
      Right SIP_MONKEYPATCH_COMPAT_NAME( AlignRight ), //!< Right align
      Justify SIP_MONKEYPATCH_COMPAT_NAME( AlignJustify ), //!< Justify align
    };
    Q_ENUM( TextHorizontalAlignment )

    /**
     * Text vertical alignment.
     *
     * This enum controls vertical alignment of text in a predefined rectangular
     * bounding box. See also Qgis::TextCharacterVerticalAlignment.
     *
     * \note Prior to QGIS 3.28 this was available as QgsTextRenderer::VAlignment
     *
     * \since QGIS 3.28
     */
    enum class TextVerticalAlignment SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsTextRenderer, VAlignment ) : int
      {
      Top SIP_MONKEYPATCH_COMPAT_NAME( AlignTop ), //!< Align to top
      VerticalCenter SIP_MONKEYPATCH_COMPAT_NAME( AlignVCenter ), //!< Center align
      Bottom SIP_MONKEYPATCH_COMPAT_NAME( AlignBottom ), //!< Align to bottom
    };
    Q_ENUM( TextVerticalAlignment )

    /**
     * Text vertical alignment for characters.
     *
     * This enum controls vertical alignment of individual characters within a block
     * of text.
     *
     * \since QGIS 3.30
     */
    enum class TextCharacterVerticalAlignment : int
    {
      Normal, //!< Adjacent characters are positioned in the standard way for text in the writing system in use
      SuperScript, //!< Characters are placed above the base line for normal text.
      SubScript, //!< Characters are placed below the base line for normal text.
    };
    Q_ENUM( TextCharacterVerticalAlignment )

    /**
     * Rendering subcomponent properties.
     *
     * \since QGIS 3.22
     */
    enum class RenderSubcomponentProperty : int
    {
      Generic, //!< Generic subcomponent property
      ShadowOffset, //!< Shadow offset
      BlurSize, //!< Blur size
      GlowSpread, //!< Glow spread size
    };
    Q_ENUM( RenderSubcomponentProperty )

    /**
     * Types of vertex.
     * \since QGIS 3.22
     */
    enum class VertexType SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsVertexId, VertexType ) : int
      {
      Segment SIP_MONKEYPATCH_COMPAT_NAME( SegmentVertex ) = 1, //!< The actual start or end point of a segment
      Curve SIP_MONKEYPATCH_COMPAT_NAME( CurveVertex ) = 2, //!< An intermediate point on a segment defining the curvature of the segment
    };
    Q_ENUM( VertexType )

    /**
     * Marker shapes.
     *
     * \note Prior to QGIS 3.24 this was available as QgsSimpleMarkerSymbolLayerBase::Shape
     *
     * \since QGIS 3.24
     */
    enum class MarkerShape SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsSimpleMarkerSymbolLayerBase, Shape ) : int
      {
      Square, //!< Square
      Diamond, //!< Diamond
      Pentagon, //!< Pentagon
      Hexagon, //!< Hexagon
      Triangle, //!< Triangle
      EquilateralTriangle, //!< Equilateral triangle
      Star, //!< Star
      Arrow, //!< Arrow
      Circle, //!< Circle
      Cross, //!< Cross (lines only)
      CrossFill, //!< Solid filled cross
      Cross2, //!< Rotated cross (lines only), 'x' shape
      Line, //!< Vertical line
      ArrowHead, //!< Right facing arrow head (unfilled, lines only)
      ArrowHeadFilled, //!< Right facing filled arrow head
      SemiCircle, //!< Semi circle (top half)
      ThirdCircle, //!< One third circle (top left third)
      QuarterCircle, //!< Quarter circle (top left quarter)
      QuarterSquare, //!< Quarter square (top left quarter)
      HalfSquare, //!< Half square (left half)
      DiagonalHalfSquare, //!< Diagonal half square (bottom left half)
      RightHalfTriangle, //!< Right half of triangle
      LeftHalfTriangle, //!< Left half of triangle
      Octagon, //!< Octagon (since QGIS 3.18)
      SquareWithCorners, //!< A square with diagonal corners (since QGIS 3.18)
      AsteriskFill, //!< A filled asterisk shape (since QGIS 3.18)
      HalfArc, //!< A line-only half arc (since QGIS 3.20)
      ThirdArc, //!< A line-only one third arc (since QGIS 3.20)
      QuarterArc, //!< A line-only one quarter arc (since QGIS 3.20)
      ParallelogramRight, //!< Parallelogram that slants right (since QGIS 3.28)
      ParallelogramLeft, //!< Parallelogram that slants left (since QGIS 3.28)
      Trapezoid, //!< Trapezoid (since QGIS 3.28)
      Shield, //!< A shape consisting of a triangle attached to a rectangle (since QGIS 3.28)
      DiamondStar, //!< A 4-sided star (since QGIS 3.28)
      Heart, //!< Heart (since QGIS 3.28)
      Decagon, //!< Decagon (since QGIS 3.28)
      RoundedSquare, //!< A square with rounded corners (since QGIS 3.28)
    };
    Q_ENUM( MarkerShape )

    /**
     * Defines how/where the symbols should be placed on a line.
     *
     * \note Prior to QGIS 3.24 this was available as QgsTemplatedLineSymbolLayerBase::Placement
     *
     * \since QGIS 3.24
     */
    enum class MarkerLinePlacement SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsTemplatedLineSymbolLayerBase, Placement ) : int SIP_ENUM_BASETYPE( IntFlag )
    {
      Interval = 1 << 0, //!< Place symbols at regular intervals
      Vertex = 1 << 1, //!< Place symbols on every vertex in the line
      LastVertex = 1 << 2, //!< Place symbols on the last vertex in the line
      FirstVertex = 1 << 3, //!< Place symbols on the first vertex in the line
      CentralPoint = 1 << 4, //!< Place symbols at the mid point of the line
      CurvePoint = 1 << 5, //!< Place symbols at every virtual curve point in the line (used when rendering curved geometry types only)
      SegmentCenter = 1 << 6, //!< Place symbols at the center of every line segment
      InnerVertices = 1 << 7, //!< Inner vertices (i.e. all vertices except the first and last vertex) (since QGIS 3.24)
    };
    Q_ENUM( MarkerLinePlacement )
    Q_DECLARE_FLAGS( MarkerLinePlacements, MarkerLinePlacement )
    Q_FLAG( MarkerLinePlacements )

    /**
     * Gradient color sources.
     *
     * \note Prior to QGIS 3.24 this was available as QgsGradientFillSymbolLayer::GradientColorType
     *
     * \since QGIS 3.24
     */
    enum class GradientColorSource SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsGradientFillSymbolLayer, GradientColorType ) : int
      {
      SimpleTwoColor, //!< Simple two color gradient
      ColorRamp, //!< Gradient color ramp
    };
    Q_ENUM( GradientColorSource )

    /**
     * Gradient types.
     *
     * \note Prior to QGIS 3.24 this was available as QgsGradientFillSymbolLayer::GradientType
     *
     * \since QGIS 3.24
     */
    enum class GradientType SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsGradientFillSymbolLayer, GradientType ) : int
      {
      Linear, //!< Linear gradient
      Radial, //!< Radial (circular) gradient
      Conical, //!< Conical (polar) gradient
    };
    Q_ENUM( GradientType )

    /**
     * Symbol coordinate reference modes.
     *
     * \note Prior to QGIS 3.24 this was available as QgsGradientFillSymbolLayer::GradientCoordinateMode
     *
     * \since QGIS 3.24
     */
    enum class SymbolCoordinateReference SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsGradientFillSymbolLayer, GradientCoordinateMode ) : int
      {
      Feature, //!< Relative to feature/shape being rendered
      Viewport, //!< Relative to the whole viewport/output device
    };
    Q_ENUM( SymbolCoordinateReference )

    /**
     * Gradient spread options, which control how gradients are rendered outside of their
     * start and end points.
     *
     * \note Prior to QGIS 3.24 this was available as QgsGradientFillSymbolLayer::GradientSpread
     *
     * \since QGIS 3.24
     */
    enum class GradientSpread SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsGradientFillSymbolLayer, GradientSpread ) : int
      {
      Pad, //!< Pad out gradient using colors at endpoint of gradient
      Reflect, //!< Reflect gradient
      Repeat, //!< Repeat gradient
    };
    Q_ENUM( GradientSpread )

    /**
     * Methods which define the number of points randomly filling a polygon.
     *
     * \note Prior to QGIS 3.24 this was available as QgsRandomMarkerFillSymbolLayer::CountMethod
     *
     * \since QGIS 3.24
     */
    enum class PointCountMethod SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsRandomMarkerFillSymbolLayer, CountMethod ) : int
      {
      Absolute SIP_MONKEYPATCH_COMPAT_NAME( AbsoluteCount ), //!< The point count is used as an absolute count of markers
      DensityBased SIP_MONKEYPATCH_COMPAT_NAME( DensityBasedCount ), //!< The point count is part of a marker density count
    };
    Q_ENUM( PointCountMethod )

    /**
     * Marker clipping modes.
     *
     * \since QGIS 3.24
     */
    enum class MarkerClipMode : int
    {
      NoClipping, //!< No clipping, render complete markers
      Shape, //!< Clip to polygon shape
      CentroidWithin, //!< Render complete markers wherever their centroid falls within the polygon shape
      CompletelyWithin, //!< Render complete markers wherever the completely fall within the polygon shape
    };
    Q_ENUM( MarkerClipMode )

    /**
     * Line clipping modes.
     *
     * \since QGIS 3.24
     */
    enum class LineClipMode : int
    {
      ClipPainterOnly, //!< Applying clipping on the painter only (i.e. line endpoints will coincide with polygon bounding box, but will not be part of the visible portion of the line)
      ClipToIntersection, //!< Clip lines to intersection with polygon shape (slower) (i.e. line endpoints will coincide with polygon exterior)
      NoClipping, //!< Lines are not clipped, will extend to shape's bounding box.
    };
    Q_ENUM( LineClipMode )

    /**
     * Dash pattern line ending rules.
     *
     * \since QGIS 3.24
     */
    enum class DashPatternLineEndingRule : int
    {
      NoRule, //!< No special rule
      FullDash, //!< Start or finish the pattern with a full dash
      HalfDash, //!< Start or finish the pattern with a half length dash
      FullGap, //!< Start or finish the pattern with a full gap
      HalfGap, //!< Start or finish the pattern with a half length gap
    };
    Q_ENUM( DashPatternLineEndingRule )

    /**
     * Dash pattern size adjustment options.
     *
     * \since QGIS 3.24
     */
    enum class DashPatternSizeAdjustment : int
    {
      ScaleBothDashAndGap, //!< Both the dash and gap lengths are adjusted equally
      ScaleDashOnly, //!< Only dash lengths are adjusted
      ScaleGapOnly, //!< Only gap lengths are adjusted
    };
    Q_ENUM( DashPatternSizeAdjustment )

    /**
     * Methods for modifying symbols by range in a graduated symbol renderer.
     *
     * \note Prior to QGIS 3.26 this was available as QgsGraduatedSymbolRenderer::GraduatedMethod
     *
     * \since QGIS 3.26
     */
    enum class GraduatedMethod SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsGraduatedSymbolRenderer, GraduatedMethod ) : int
      {
      Color SIP_MONKEYPATCH_COMPAT_NAME( GraduatedColor ), //!< Alter color of symbols
      Size SIP_MONKEYPATCH_COMPAT_NAME( GraduatedSize ), //!< Alter size of symbols
    };
    Q_ENUM( GraduatedMethod )

    /**
     * Placement options for suffixes in the labels for axis of plots.
     *
     * \since QGIS 3.32
     */
    enum class PlotAxisSuffixPlacement
    {
      NoLabels, //!< Do not place suffixes
      EveryLabel, //!< Place suffix after every value label
      FirstLabel, //!< Place suffix after the first label value only
      LastLabel, //!< Place suffix after the last label value only
      FirstAndLastLabels, //!< Place suffix after the first and last label values only
    };
    Q_ENUM( PlotAxisSuffixPlacement )

    /**
     * DpiMode enum
     * \since QGIS 3.26
     */
    enum class DpiMode
    {
      All = 7, //!< All
      Off = 0, //!< Off
      QGIS = 1, //!< QGIS
      UMN = 2, //!< UMN
      GeoServer = 4, //!< GeoServer
    };
    Q_ENUM( DpiMode )

    /**
     * DpiMode enum
     * \since QGIS 3.30
     */
    enum class TilePixelRatio
    {
      Undefined = 0, //!< Undefined (not scale)
      StandardDpi = 1, //!< Standard (96 DPI)
      HighDpi = 2, //!< High (192 DPI)
    };
    Q_ENUM( TilePixelRatio )

    // NOTE -- the hardcoded numbers here must match QFont::Capitalization!

    /**
     * String capitalization options.
     *
     * \note Prior to QGIS 3.24 this was available as QgsStringUtils::Capitalization
     *
     * \since QGIS 3.24
     */
    enum class Capitalization SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsStringUtils, Capitalization ) : int
      {
      MixedCase = 0, //!< Mixed case, ie no change
      AllUppercase = 1, //!< Convert all characters to uppercase
      AllLowercase = 2,  //!< Convert all characters to lowercase
      ForceFirstLetterToCapital = 4, //!< Convert just the first letter of each word to uppercase, leave the rest untouched
      SmallCaps = 5, //!< Mixed case small caps (since QGIS 3.24)
      TitleCase = 1004, //!< Simple title case conversion - does not fully grammatically parse the text and uses simple rules only. Note that this method does not convert any characters to lowercase, it only uppercases required letters. Callers must ensure that input strings are already lowercased.
      UpperCamelCase = 1005, //!< Convert the string to upper camel case. Note that this method does not unaccent characters.
      AllSmallCaps = 1006, //!< Force all characters to small caps (since QGIS 3.24)
    };
    Q_ENUM( Capitalization )

    /**
     * Flags which control the behavior of rendering text.
     *
     * \since QGIS 3.24
     */
    enum class TextRendererFlag : int SIP_ENUM_BASETYPE( IntFlag )
    {
      WrapLines = 1 << 0, //!< Automatically wrap long lines of text
    };
    Q_ENUM( TextRendererFlag )
    Q_DECLARE_FLAGS( TextRendererFlags, TextRendererFlag )
    Q_FLAG( TextRendererFlags )

    /**
     * Available methods for converting map scales to tile zoom levels.
     *
     * \since QGIS 3.26
     */
    enum class ScaleToTileZoomLevelMethod : int
    {
      MapBox, //!< Uses a scale doubling approach to account for hi-DPI tiles, and rounds to the nearest tile level for the map scale
      Esri, //!< No scale doubling, always rounds down when matching to available tile levels
    };
    Q_ENUM( ScaleToTileZoomLevelMethod );

    /**
     * Angular directions.
     *
     * \since QGIS 3.24
     */
    enum class AngularDirection SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsCurve, Orientation ) : int
      {
      Clockwise, //!< Clockwise direction
      CounterClockwise, //!< Counter-clockwise direction
      NoOrientation, //!< Unknown orientation or sentinel value
    };
    Q_ENUM( AngularDirection )

    /**
     * Usage of the renderer.
     *
     * \since QGIS 3.24
     */
    enum class RendererUsage : int
    {
      View, //!< Renderer used for displaying on screen
      Export, //!< Renderer used for printing or exporting to a file
      Unknown, //!< Renderer used for unknown usage
    };
    Q_ENUM( RendererUsage )

    /**
     * Synchronization of 2D map canvas and 3D view
     *
     * \since QGIS 3.26
     */
    enum class ViewSyncModeFlag : int SIP_ENUM_BASETYPE( IntFlag )
    {
      Sync3DTo2D = 1 << 0, //!< Synchronize 3D view camera to the main map canvas extent
      Sync2DTo3D = 1 << 1, //!< Update the 2D main canvas extent to include the viewed area from the 3D view
    };
    Q_ENUM( ViewSyncModeFlag )
    Q_DECLARE_FLAGS( ViewSyncModeFlags, ViewSyncModeFlag )

    /**
     * Modes for recentering map canvases.
     *
     * \since QGIS 3.30
     */
    enum class MapRecenteringMode
    {
      Always, //!< Always recenter map
      WhenOutsideVisibleExtent, //!< Only recenter map when new center would be outside of current visible extent
      Never, //!< Never recenter map
    };
    Q_ENUM( MapRecenteringMode )

    /**
     * History provider backends.
     *
     * \since QGIS 3.24
     */
    enum class HistoryProviderBackend : int SIP_ENUM_BASETYPE( IntFlag )
    {
      LocalProfile = 1 << 0, //!< Local profile
//      Project = 1 << 1, //!< QGIS Project  (not yet implemented)
    };
    Q_ENUM( HistoryProviderBackend )
    Q_DECLARE_FLAGS( HistoryProviderBackends, HistoryProviderBackend )
    Q_FLAG( HistoryProviderBackends )

    /**
     * Processing data source types.
     *
     * \note Prior to QGIS 3.36 this was available as QgsProcessing::SourceType
     *
     * \since QGIS 3.36
     */
    enum class ProcessingSourceType SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsProcessing, SourceType ) : int
      {
      MapLayer SIP_MONKEYPATCH_COMPAT_NAME( TypeMapLayer ) = -2, //!< Any map layer type (raster, vector, mesh, point cloud, annotation or plugin layer)
      VectorAnyGeometry SIP_MONKEYPATCH_COMPAT_NAME( TypeVectorAnyGeometry ) = -1, //!< Any vector layer with geometry
      VectorPoint SIP_MONKEYPATCH_COMPAT_NAME( TypeVectorPoint ) = 0, //!< Vector point layers
      VectorLine SIP_MONKEYPATCH_COMPAT_NAME( TypeVectorLine ) = 1, //!< Vector line layers
      VectorPolygon SIP_MONKEYPATCH_COMPAT_NAME( TypeVectorPolygon ) = 2, //!< Vector polygon layers
      Raster SIP_MONKEYPATCH_COMPAT_NAME( TypeRaster ) = 3, //!< Raster layers
      File SIP_MONKEYPATCH_COMPAT_NAME( TypeFile ) = 4, //!< Files (i.e. non map layer sources, such as text files)
      Vector SIP_MONKEYPATCH_COMPAT_NAME( TypeVector ) = 5, //!< Tables (i.e. vector layers with or without geometry). When used for a sink this indicates the sink has no geometry.
      Mesh SIP_MONKEYPATCH_COMPAT_NAME( TypeMesh ) = 6, //!< Mesh layers \since QGIS 3.6
      Plugin SIP_MONKEYPATCH_COMPAT_NAME( TypePlugin ) = 7, //!< Plugin layers \since QGIS 3.22
      PointCloud SIP_MONKEYPATCH_COMPAT_NAME( TypePointCloud ) = 8, //!< Point cloud layers \since QGIS 3.22
      Annotation SIP_MONKEYPATCH_COMPAT_NAME( TypeAnnotation ) = 9, //!< Annotation layers \since QGIS 3.22
      VectorTile SIP_MONKEYPATCH_COMPAT_NAME( TypeVectorTile ) = 10 //!< Vector tile layers \since QGIS 3.32
    };
    Q_ENUM( ProcessingSourceType )


    /**
     * Flags indicating how and when an processing provider operates and should be exposed to users.
     *
     * \note Prior to QGIS 3.36 this was available as QgsProcessingProvider::Flag
     *
     * \since QGIS 3.36
     */
    enum class ProcessingProviderFlag SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsProcessingProvider, Flag ) : int SIP_ENUM_BASETYPE( IntFlag )
    {
      DeemphasiseSearchResults SIP_MONKEYPATCH_COMPAT_NAME( FlagDeemphasiseSearchResults ) = 1 << 1, //!< Algorithms should be de-emphasised in the search results when searching for algorithms. Use for low-priority providers or those with substantial known issues.
      CompatibleWithVirtualRaster SIP_MONKEYPATCH_COMPAT_NAME( FlagCompatibleWithVirtualRaster ) = 1 << 2, //!< The processing provider's algorithms can work with QGIS virtualraster data provider. Since QGIS 3.36
    };
    Q_ENUM( ProcessingProviderFlag );

    /**
     * Flags indicating how and when an processing provider operates and should be exposed to users.
     *
     * \note Prior to QGIS 3.36 this was available as QgsProcessingProvider::Flags
     *
     * \since QGIS 3.36
     */
    Q_DECLARE_FLAGS( ProcessingProviderFlags, ProcessingProviderFlag ) SIP_MONKEYPATCH_FLAGS_UNNEST( QgsProcessingProvider, Flags )
    Q_FLAG( ProcessingProviderFlags )

    /**
     * Flags indicating how and when an algorithm operates and should be exposed to users.
     *
     * \note Prior to QGIS 3.36 this was available as QgsProcessingAlgorithm::Flag
     *
     * \since QGIS 3.36
     */
    enum class ProcessingAlgorithmFlag SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsProcessingAlgorithm, Flag ) : int SIP_ENUM_BASETYPE( IntFlag )
    {
      HideFromToolbox SIP_MONKEYPATCH_COMPAT_NAME( FlagHideFromToolbox ) = 1 << 1, //!< Algorithm should be hidden from the toolbox
      HideFromModeler SIP_MONKEYPATCH_COMPAT_NAME( FlagHideFromModeler ) = 1 << 2, //!< Algorithm should be hidden from the modeler
      SupportsBatch SIP_MONKEYPATCH_COMPAT_NAME( FlagSupportsBatch ) = 1 << 3,  //!< Algorithm supports batch mode
      CanCancel SIP_MONKEYPATCH_COMPAT_NAME( FlagCanCancel ) = 1 << 4, //!< Algorithm can be canceled
      RequiresMatchingCrs SIP_MONKEYPATCH_COMPAT_NAME( FlagRequiresMatchingCrs ) = 1 << 5, //!< Algorithm requires that all input layers have matching coordinate reference systems
      NoThreading SIP_MONKEYPATCH_COMPAT_NAME( FlagNoThreading ) = 1 << 6, //!< Algorithm is not thread safe and cannot be run in a background thread, e.g. for algorithms which manipulate the current project, layer selections, or with external dependencies which are not thread-safe.
      DisplayNameIsLiteral SIP_MONKEYPATCH_COMPAT_NAME( FlagDisplayNameIsLiteral ) = 1 << 7, //!< Algorithm's display name is a static literal string, and should not be translated or automatically formatted. For use with algorithms named after commands, e.g. GRASS 'v.in.ogr'.
      SupportsInPlaceEdits SIP_MONKEYPATCH_COMPAT_NAME( FlagSupportsInPlaceEdits ) = 1 << 8, //!< Algorithm supports in-place editing
      KnownIssues SIP_MONKEYPATCH_COMPAT_NAME( FlagKnownIssues ) = 1 << 9, //!< Algorithm has known issues
      CustomException SIP_MONKEYPATCH_COMPAT_NAME( FlagCustomException ) = 1 << 10, //!< Algorithm raises custom exception notices, don't use the standard ones
      PruneModelBranchesBasedOnAlgorithmResults SIP_MONKEYPATCH_COMPAT_NAME( FlagPruneModelBranchesBasedOnAlgorithmResults ) = 1 << 11, //!< Algorithm results will cause remaining model branches to be pruned based on the results of running the algorithm
      SkipGenericModelLogging SIP_MONKEYPATCH_COMPAT_NAME( FlagSkipGenericModelLogging ) = 1 << 12, //!< When running as part of a model, the generic algorithm setup and results logging should be skipped
      NotAvailableInStandaloneTool SIP_MONKEYPATCH_COMPAT_NAME( FlagNotAvailableInStandaloneTool ) = 1 << 13, //!< Algorithm should not be available from the standalone "qgis_process" tool. Used to flag algorithms which make no sense outside of the QGIS application, such as "select by..." style algorithms.
      RequiresProject SIP_MONKEYPATCH_COMPAT_NAME( FlagRequiresProject ) = 1 << 14, //!< The algorithm requires that a valid QgsProject is available from the processing context in order to execute
      Deprecated SIP_MONKEYPATCH_COMPAT_NAME( FlagDeprecated ) = HideFromToolbox | HideFromModeler, //!< Algorithm is deprecated
    };
    Q_ENUM( ProcessingAlgorithmFlag );

    /**
     * Flags indicating how and when an algorithm operates and should be exposed to users.
     *
     * \note Prior to QGIS 3.36 this was available as QgsProcessingAlgorithm::Flags
     *
     * \since QGIS 3.36
     */
    Q_DECLARE_FLAGS( ProcessingAlgorithmFlags, ProcessingAlgorithmFlag ) SIP_MONKEYPATCH_FLAGS_UNNEST( QgsProcessingAlgorithm, Flags )
    Q_FLAG( ProcessingAlgorithmFlags )

    /**
     * Property availability, used for QgsProcessingAlgorithm::VectorProperties
     * in order to determine if properties are available or not.
     *
     * \note Prior to QGIS 3.36 this was available as QgsProcessingAlgorithm::PropertyAvailability
     *
     * \since QGIS 3.36
     */
    enum class ProcessingPropertyAvailability SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsProcessingAlgorithm, PropertyAvailability ) : int
      {
      NotAvailable, //!< Properties are not available
      Available, //!< Properties are available
    };
    Q_ENUM( ProcessingPropertyAvailability )

    /**
     * Logging level for algorithms to use when pushing feedback messages.
     *
     * \note Prior to QGIS 3.36 this was available as QgsProcessingContext::LogLevel
     *
     * \since QGIS 3.36
     */
    enum class ProcessingLogLevel SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsProcessingContext, LogLevel ) : int
      {
      DefaultLevel = 0, //!< Default logging level
      Verbose, //!< Verbose logging
      ModelDebug, //!< Model debug level logging. Includes verbose logging and other outputs useful for debugging models (since QGIS 3.34).
    };
    Q_ENUM( ProcessingLogLevel )

    /**
     * Flags which control behavior for a Processing feature source.
     *
     * \note Prior to QGIS 3.36 this was available as QgsProcessingFeatureSourceDefinition::Flag
     *
     * \since QGIS 3.36
     */
    enum class ProcessingFeatureSourceDefinitionFlag SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsProcessingFeatureSourceDefinition, Flag ) : int SIP_ENUM_BASETYPE( IntFlag )
    {
      OverrideDefaultGeometryCheck SIP_MONKEYPATCH_COMPAT_NAME( FlagOverrideDefaultGeometryCheck ) = 1 << 0, //!< If set, the default geometry check method (as dictated by QgsProcessingContext) will be overridden for this source
      CreateIndividualOutputPerInputFeature SIP_MONKEYPATCH_COMPAT_NAME( FlagCreateIndividualOutputPerInputFeature ) = 1 << 1, //!< If set, every feature processed from this source will be placed into its own individually created output destination. Support for this flag depends on how an algorithm is executed.
    };
    Q_ENUM( ProcessingFeatureSourceDefinitionFlag )

    /**
     * Flags which control behavior for a Processing feature source.
     *
     * \note Prior to QGIS 3.36 this was available as QgsProcessingFeatureSourceDefinition::Flags
     *
     * \since QGIS 3.36
     */
    Q_DECLARE_FLAGS( ProcessingFeatureSourceDefinitionFlags, ProcessingFeatureSourceDefinitionFlag ) SIP_MONKEYPATCH_FLAGS_UNNEST( QgsProcessingFeatureSourceDefinition, Flags )
    Q_FLAG( ProcessingFeatureSourceDefinitionFlags )

    /**
     * Flags which control how QgsProcessingFeatureSource fetches features.
     *
     * \note Prior to QGIS 3.36 this was available as QgsProcessingFeatureSource::Flag
     *
     * \since QGIS 3.36
     */
    enum class ProcessingFeatureSourceFlag SIP_ENUM_BASETYPE( IntFlag ) SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsProcessingFeatureSource, Flag ) : int SIP_ENUM_BASETYPE( IntFlag )
    {
      SkipGeometryValidityChecks SIP_MONKEYPATCH_COMPAT_NAME( FlagSkipGeometryValidityChecks ) = 1 << 1, //!< Invalid geometry checks should always be skipped. This flag can be useful for algorithms which always require invalid geometries, regardless of any user settings (e.g. "repair geometry" type algorithms).
    };
    Q_ENUM( ProcessingFeatureSourceFlag )

    /**
     * Flags which control how QgsProcessingFeatureSource fetches features.
     *
     * \note Prior to QGIS 3.36 this was available as QgsProcessingFeatureSource::Flags
     *
     * \since QGIS 3.36
     */
    Q_DECLARE_FLAGS( ProcessingFeatureSourceFlags, ProcessingFeatureSourceFlag ) SIP_MONKEYPATCH_FLAGS_UNNEST( QgsProcessingFeatureSource, Flags )
    Q_FLAG( ProcessingFeatureSourceFlags )

    /**
     * Flags which dictate the behavior of Processing parameter types.
     *
     * Each parameter type can offer a number of additional flags to fine tune its behavior
     * and capabilities.
     *
     * \note Prior to QGIS 3.36 this was available as QgsProcessingParameterType::ParameterFlag
     *
     * \since QGIS 3.36
     */
    enum class ProcessingParameterTypeFlag SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsProcessingParameterType, ParameterFlag ) : int SIP_ENUM_BASETYPE( IntFlag )
    {
      ExposeToModeler = 1 //!< Is this parameter available in the modeler. Is set to on by default.
    };
    Q_ENUM( ProcessingParameterTypeFlag )

    /**
     * Flags which dictate the behavior of Processing parameter types.
     *
     * Each parameter type can offer a number of additional flags to fine tune its behavior
     * and capabilities.
     *
     * \note Prior to QGIS 3.36 this was available as QgsProcessingParameterType::ParameterFlags
     *
     * \since QGIS 3.36
     */
    Q_DECLARE_FLAGS( ProcessingParameterTypeFlags, ProcessingParameterTypeFlag ) SIP_MONKEYPATCH_FLAGS_UNNEST( QgsProcessingParameterType, ParameterFlags )
    Q_FLAG( ProcessingParameterTypeFlags )

    /**
     * Flags which dictate the behavior of Processing parameters.
     *
     * \note Prior to QGIS 3.36 this was available as QgsProcessingParameterDefinition::Flag
     *
     * \since QGIS 3.36
     */
    enum class ProcessingParameterFlag SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsProcessingParameterDefinition, Flag ) : int SIP_ENUM_BASETYPE( IntFlag )
    {
      Advanced SIP_MONKEYPATCH_COMPAT_NAME( FlagAdvanced ) = 1 << 1, //!< Parameter is an advanced parameter which should be hidden from users by default
      Hidden SIP_MONKEYPATCH_COMPAT_NAME( FlagHidden ) = 1 << 2, //!< Parameter is hidden and should not be shown to users
      Optional SIP_MONKEYPATCH_COMPAT_NAME( FlagOptional ) = 1 << 3, //!< Parameter is optional
      IsModelOutput SIP_MONKEYPATCH_COMPAT_NAME( FlagIsModelOutput ) = 1 << 4, //!< Destination parameter is final output. The parameter name will be used.
    };
    Q_ENUM( ProcessingParameterFlag )

    /**
     * Flags which dictate the behavior of Processing parameters.
     *
     * \note Prior to QGIS 3.36 this was available as QgsProcessingParameterDefinition::Flags
     *
     * \since QGIS 3.36
     */
    Q_DECLARE_FLAGS( ProcessingParameterFlags, ProcessingParameterFlag ) SIP_MONKEYPATCH_FLAGS_UNNEST( QgsProcessingParameterDefinition, Flags )
    Q_FLAG( ProcessingParameterFlags )

    /**
     * Flags which dictate the behavior of QgsProcessingParameterFile.
     *
     * \note Prior to QGIS 3.36 this was available as QgsProcessingParameterFile::Behavior
     *
     * \since QGIS 3.36
     */
    enum class ProcessingFileParameterBehavior SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsProcessingParameterFile, Behavior ) : int
      {
      File = 0, //!< Parameter is a single file
      Folder, //!< Parameter is a folder
    };
    Q_ENUM( ProcessingFileParameterBehavior )

    /**
     * Processing numeric parameter data types.
     *
     * \note Prior to QGIS 3.36 this was available as QgsProcessingParameterNumber::Type
     *
     * \since QGIS 3.36
     */
    enum class ProcessingNumberParameterType SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsProcessingParameterNumber, Type ) : int
      {
      Integer, //!< Integer values
      Double, //!< Double/float values
    };
    Q_ENUM( ProcessingNumberParameterType )

    /**
     * Processing field parameter data types.
     *
     * \note Prior to QGIS 3.36 this was available as QgsProcessingParameterField::DataType
     *
     * \since QGIS 3.36
     */
    enum class ProcessingFieldParameterDataType SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsProcessingParameterField, DataType ) : int
      {
      Any = -1, //!< Accepts any field
      Numeric = 0, //!< Accepts numeric fields
      String = 1, //!< Accepts string fields
      DateTime = 2, //!< Accepts datetime fields
      Binary = 3, //!< Accepts binary fields, since QGIS 3.34
      Boolean = 4, //!< Accepts boolean fields, since QGIS 3.34
    };
    Q_ENUM( ProcessingFieldParameterDataType )

    /**
     * Processing date time parameter data types.
     *
     * \note Prior to QGIS 3.36 this was available as QgsProcessingParameterDateTime::Type
     *
     * \since QGIS 3.36
     */
    enum class ProcessingDateTimeParameterDataType SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsProcessingParameterDateTime, Type ) : int
      {
      DateTime, //!< Datetime values
      Date, //!< Date values
      Time, //!< Time values
    };
    Q_ENUM( ProcessingDateTimeParameterDataType )

    /**
     * Processing model child parameter sources.
     *
     * \since QGIS 3.34
     */
    enum class ProcessingModelChildParameterSource : int
    {
      ModelParameter, //!< Parameter value is taken from a parent model parameter
      ChildOutput, //!< Parameter value is taken from an output generated by a child algorithm
      StaticValue, //!< Parameter value is a static value
      Expression, //!< Parameter value is taken from an expression, evaluated just before the algorithm runs
      ExpressionText, //!< Parameter value is taken from a text with expressions, evaluated just before the algorithm runs
      ModelOutput, //!< Parameter value is linked to an output parameter for the model
    };
    Q_ENUM( ProcessingModelChildParameterSource )

    /**
     * Defines the type of input layer for a Processing TIN input.
     *
     * \note Prior to QGIS 3.36 this was available as QgsProcessingParameterTinInputLayers::Type
     *
     * \since QGIS 3.36
     */
    enum class ProcessingTinInputLayerType SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsProcessingParameterTinInputLayers, Type ) : int
      {
      Vertices, //!< Input that adds only vertices
      StructureLines, //!< Input that adds add structure lines
      BreakLines //!< Input that adds vertices and break lines
    };
    Q_ENUM( ProcessingTinInputLayerType )

    /**
     * CRS definition formats.
     *
     * \since QGIS 3.24
     */
    enum class CrsDefinitionFormat SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsCoordinateReferenceSystem, Format ) : int
      {
      Wkt SIP_MONKEYPATCH_COMPAT_NAME( FormatWkt ), //!< WKT format (always recommended over proj string format)
      Proj SIP_MONKEYPATCH_COMPAT_NAME( FormatProj ), //!< Proj string format
    };
    Q_ENUM( CrsDefinitionFormat )

    /**
     * Split policy for field domains.
     *
     * When a feature is split into multiple parts, defines how the value of attributes
     * following the domain are computed.
     *
     * \since QGIS 3.26
     */
    enum class FieldDomainSplitPolicy : int
    {
      DefaultValue, //!< Use default field value
      Duplicate, //!< Duplicate original value
      GeometryRatio, //!< New values are computed by the ratio of their area/length compared to the area/length of the original feature
      UnsetField, //!< Clears the field value so that the data provider backend will populate using any backend triggers or similar logic (since QGIS 3.30)
    };
    Q_ENUM( FieldDomainSplitPolicy )

    /**
     * Merge policy for field domains.
     *
     * When a feature is built by merging multiple features, defines how the value of
     * attributes following the domain are computed.
     *
     * \since QGIS 3.26
     */
    enum class FieldDomainMergePolicy : int
    {
      DefaultValue, //!< Use default field value
      Sum, //!< Sum of values
      GeometryWeighted, //!< New values are computed as the weighted average of the source values
    };
    Q_ENUM( FieldDomainMergePolicy )

    /**
     * Types of field domain
     *
     * \since QGIS 3.26
     */
    enum class FieldDomainType : int
    {
      Coded, //!< Coded field domain
      Range, //!< Numeric range field domain (min/max)
      Glob, //!< Glob string pattern field domain
    };
    Q_ENUM( FieldDomainType )

    /**
     * Transaction mode.
     *
     * \since QGIS 3.26
     */
    enum class TransactionMode : int
    {
      Disabled = 0, //!< Edits are buffered locally and sent to the provider when toggling layer editing mode.
      AutomaticGroups = 1, //!< Automatic transactional editing means that on supported datasources (postgres and geopackage databases) the edit state of all tables that originate from the same database are synchronized and executed in a server side transaction.
      BufferedGroups = 2, //!< Buffered transactional editing means that all editable layers in the buffered transaction group are toggled synchronously and all edits are saved in a local edit buffer. Saving changes is executed within a single transaction on all layers (per provider).
    };
    Q_ENUM( TransactionMode )

    /**
     * Altitude clamping.
     *
     * \since QGIS 3.26
     */
    enum class AltitudeClamping : int
    {
      Absolute, //!< Elevation is taken directly from feature and is independent of terrain height (final elevation = feature elevation)
      Relative, //!< Elevation is relative to terrain height (final elevation = terrain elevation + feature elevation)
      Terrain, //!< Elevation is clamped to terrain (final elevation = terrain elevation)
    };
    Q_ENUM( AltitudeClamping )

    /**
     * Altitude binding.
     *
     * \since QGIS 3.26
     */
    enum class AltitudeBinding : int
    {
      Vertex, //!< Clamp every vertex of feature
      Centroid, //!< Clamp just centroid of feature
    };
    Q_ENUM( AltitudeBinding )

    /**
     * Describes how the limits of a range are handled.
     *
     * \since QGIS 3.38
     */
    enum class RangeLimits : int
    {
      IncludeBoth = 0, //!< Both lower and upper values are included in the range
      IncludeLowerExcludeUpper, //!< Lower value is included in the range, upper value is excluded
      ExcludeLowerIncludeUpper, //!< Lower value is excluded from the range, upper value in inccluded
      ExcludeBoth, //!< Both lower and upper values are excluded from the range
    };
    Q_ENUM( RangeLimits )

    /**
     * Raster layer elevation modes.
     *
     * \since QGIS 3.38
     */
    enum class RasterElevationMode : int
    {
      FixedElevationRange = 0, //!< Layer has a fixed elevation range
      RepresentsElevationSurface = 1, //!< Pixel values represent an elevation surface
      FixedRangePerBand = 2, //!< Layer has a fixed (manually specified) elevation range per band
      DynamicRangePerBand = 3, //!< Layer has a elevation range per band, calculated dynamically from an expression
    };
    Q_ENUM( RasterElevationMode )

    /**
     * Mesh layer elevation modes.
     *
     * \since QGIS 3.38
     */
    enum class MeshElevationMode : int
    {
      FixedElevationRange = 0, //!< Layer has a fixed elevation range
      FromVertices = 1 //!< Elevation should be taken from mesh vertices
    };
    Q_ENUM( MeshElevationMode )

    /**
     * Between line constraints which can be enabled
     *
     * \since QGIS 3.26
     */
    enum class BetweenLineConstraint SIP_MONKEYPATCH_SCOPEENUM : int
    {
      NoConstraint,  //!< No additional constraint
      Perpendicular, //!< Perpendicular
      Parallel       //!< Parallel
    };
    Q_ENUM( BetweenLineConstraint )

    /**
     * Designates whether the line extension constraint is currently soft locked
     * with the previous or next vertex of the locked one.
     * \since QGIS 3.26
     */
    enum class LineExtensionSide : int
    {
      BeforeVertex, //!< Lock to previous vertex
      AfterVertex, //!< Lock to next vertex
      NoVertex, //!< Don't lock to vertex
    };
    Q_ENUM( LineExtensionSide )


    /**
     * Advanced digitizing constraint type.
     * \since QGIS 3.32
     */
    enum class CadConstraintType : int
    {
      Generic,      //!< Generic value
      Angle,        //!< Angle value
      Distance,     //!< Distance value
      XCoordinate,  //!< X Coordinate value
      YCoordinate,  //!< Y Coordinate value
      ZValue,       //!< Z value
      MValue,       //!< M value
    };
    Q_ENUM( CadConstraintType )


    /**
     * Flags which control the behavior of QgsProjects.
     *
     * \since QGIS 3.26
     */
    enum class ProjectFlag : int SIP_ENUM_BASETYPE( IntFlag )
    {
      EvaluateDefaultValuesOnProviderSide = 1 << 0, //!< If set, default values for fields will be evaluated on the provider side when features from the project are created instead of when they are committed.
      TrustStoredLayerStatistics = 1 << 1, //!< If set, then layer statistics (such as the layer extent) will be read from values stored in the project instead of requesting updated values from the data provider. Additionally, when this flag is set, primary key unicity is not checked for views and materialized views with Postgres provider.
      RememberLayerEditStatusBetweenSessions = 1 << 2, //!< If set, then any layers set to be editable will be stored in the project and immediately made editable whenever that project is restored
      RememberAttributeTableWindowsBetweenSessions = 1 << 3, //!< If set, then any open attribute tables will be stored in the project and immediately reopened when the project is restored
    };
    Q_ENUM( ProjectFlag )
    Q_DECLARE_FLAGS( ProjectFlags, ProjectFlag )
    Q_FLAG( ProjectFlags )

    /**
     * Flags that control the way the QgsPlotTools operate.
     *
     * \since QGIS 3.26
     */
    enum class PlotToolFlag : int SIP_ENUM_BASETYPE( IntFlag )
    {
      ShowContextMenu = 1 << 0, //!< Show a context menu when right-clicking with the tool.
    };
    Q_ENUM( PlotToolFlag )
    Q_DECLARE_FLAGS( PlotToolFlags, PlotToolFlag )
    Q_FLAG( PlotToolFlags )


    /**
     * 3D point shape types.
     *
     * \note Prior to QGIS 3.36 this was available as QgsPoint3DSymbol::Shape
     *
     * \since QGIS 3.36
     */
    enum class Point3DShape : int
    {
      Cylinder, //!< Cylinder
      Sphere, //!< Sphere
      Cone, //!< Cone
      Cube, //!< Cube
      Torus, //!< Torus
      Plane, //!< Flat plane
      ExtrudedText, //!< Extruded text
      Model, //!< Model
      Billboard, //!< Billboard
    };
    Q_ENUM( Point3DShape )

    /**
     * Light source types for 3D scenes.
     *
     * \since QGIS 3.26
     */
    enum class LightSourceType : int
    {
      Point, //!< Point light source
      Directional, //!< Directional light source
    };
    Q_ENUM( LightSourceType )

    /**
     * The navigation mode used by 3D cameras.
     *
     * \since QGIS 3.30
     */
    enum class NavigationMode : int
    {
      TerrainBased, //!< The default navigation based on the terrain
      Walk //!< Uses WASD keys or arrows to navigate in walking (first person) manner
    };
    Q_ENUM( NavigationMode )

    /**
     * Vertical axis inversion options for 3D views.
     *
     * \since QGIS 3.30
     */
    enum class VerticalAxisInversion : int
    {
      Never, //!< Never invert vertical axis movements
      WhenDragging, //!< Invert vertical axis movements when dragging in first person modes
      Always, //!< Always invert vertical axis movements
    };
    Q_ENUM( VerticalAxisInversion )

    /**
     * Surface symbology type for elevation profile plots.
     *
     * \since QGIS 3.26
     */
    enum class ProfileSurfaceSymbology : int
    {
      Line, //!< The elevation surface will be rendered using a line symbol
      FillBelow, //!< The elevation surface will be rendered using a fill symbol below the surface level
      FillAbove, //!< The elevation surface will be rendered using a fill symbol above the surface level (since QGIS 3.32)
    };
    Q_ENUM( ProfileSurfaceSymbology );

    /**
     * Types of elevation profiles to generate for vector sources.
     *
     * \since QGIS 3.26
     */
    enum class VectorProfileType : int
    {
      IndividualFeatures, //!< Treat each feature as an individual object (eg buildings)
      ContinuousSurface, //!< The features should be treated as representing values on a continuous surface (eg contour lines)
    };
    Q_ENUM( VectorProfileType );

    /**
     * Flags that control the way the QgsAbstractProfileGenerator operate.
     *
     * \since QGIS 3.26
     */
    enum class ProfileGeneratorFlag : int SIP_ENUM_BASETYPE( IntFlag )
    {
      RespectsMaximumErrorMapUnit = 1 << 0, //!< Generated profile respects the QgsProfileGenerationContext::maximumErrorMapUnits() property.
      RespectsDistanceRange = 1 << 1, //!< Generated profile respects the QgsProfileGenerationContext::distanceRange() property.
      RespectsElevationRange = 1 << 2, //!< Generated profile respects the QgsProfileGenerationContext::elevationRange() property.
    };
    Q_ENUM( ProfileGeneratorFlag )
    Q_DECLARE_FLAGS( ProfileGeneratorFlags, ProfileGeneratorFlag )
    Q_FLAG( ProfileGeneratorFlags )

    /**
     * Types of export for elevation profiles.
     *
     * \since QGIS 3.32
     */
    enum class ProfileExportType : int
    {
      Features3D, //!< Export profiles as 3D features, with elevation values stored in exported geometry Z values
      Profile2D, //!< Export profiles as 2D profile lines, with elevation stored in exported geometry Y dimension and distance in X dimension
      DistanceVsElevationTable, //!< Export profiles as a table of sampled distance vs elevation values
    };
    Q_ENUM( ProfileExportType );

    /**
     * Rendering symbols for point cloud points.
     *
     * \since QGIS 3.26
     */
    enum class PointCloudSymbol SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsPointCloudRenderer, PointSymbol ) : int
      {
      Square, //!< Renders points as squares
      Circle, //!< Renders points as circles
    };
    Q_ENUM( PointCloudSymbol )

    /**
     * Pointcloud rendering order for 2d views
     *
     * /since QGIS 3.26
     */
    enum class PointCloudDrawOrder SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsPointCloudRenderer, DrawOrder ) : int
      {
      Default, //!< Draw points in the order they are stored
      BottomToTop, //!< Draw points with larger Z values last
      TopToBottom, //!< Draw points with larger Z values first
    };
    Q_ENUM( PointCloudDrawOrder )

    /**
     * Flags which control how intersections of pre-existing feature are handled when digitizing new features.
     *
     * \note Prior to QGIS 3.26 this was available as QgsProject::AvoidIntersectionsMode
     *
     * \since QGIS 3.26
     */
    enum class AvoidIntersectionsMode SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsProject, AvoidIntersectionsMode ) : int
      {
      AllowIntersections, //!< Overlap with any feature allowed when digitizing new features
      AvoidIntersectionsCurrentLayer, //!< Overlap with features from the active layer when digitizing new features not allowed
      AvoidIntersectionsLayers, //!< Overlap with features from a specified list of layers when digitizing new features not allowed
    };
    Q_ENUM( AvoidIntersectionsMode )

    /**
     * Flags which control project read behavior.
     *
     * \note Prior to QGIS 3.26 this was available as QgsProject::FileFormat
     *
     * \since QGIS 3.26
     */
    enum class ProjectFileFormat SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsProject, FileFormat ) : int
      {
      Qgz, //!< Archive file format, supports auxiliary data
      Qgs, //!< Project saved in a clear text, does not support auxiliary data
    };
    Q_ENUM( ProjectFileFormat )

    /**
     * Flags which control project read behavior.
     *
     * \note Prior to QGIS 3.26 this was available as QgsProject::ReadFlag
     *
     * \since QGIS 3.26
     */
    enum class ProjectReadFlag SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsProject, ReadFlag ) : int SIP_ENUM_BASETYPE( IntFlag )
    {
      DontResolveLayers SIP_MONKEYPATCH_COMPAT_NAME( FlagDontResolveLayers ) = 1 << 0, //!< Don't resolve layer paths (i.e. don't load any layer content). Dramatically improves project read time if the actual data from the layers is not required.
      DontLoadLayouts SIP_MONKEYPATCH_COMPAT_NAME( FlagDontLoadLayouts ) = 1 << 1, //!< Don't load print layouts. Improves project read time if layouts are not required, and allows projects to be safely read in background threads (since print layouts are not thread safe).
      TrustLayerMetadata SIP_MONKEYPATCH_COMPAT_NAME( FlagTrustLayerMetadata ) = 1 << 2, //!< Trust layer metadata. Improves project read time. Do not use it if layers' extent is not fixed during the project's use by QGIS and QGIS Server.
      DontStoreOriginalStyles SIP_MONKEYPATCH_COMPAT_NAME( FlagDontStoreOriginalStyles ) = 1 << 3, //!< Skip the initial XML style storage for layers. Useful for minimising project load times in non-interactive contexts.
      DontLoad3DViews SIP_MONKEYPATCH_COMPAT_NAME( FlagDontLoad3DViews ) = 1 << 4, //!< Skip loading 3D views (since QGIS 3.26)
      DontLoadProjectStyles = 1 << 5, //!< Skip loading project style databases (deprecated -- use ProjectCapability::ProjectStyles flag instead)
      ForceReadOnlyLayers = 1 << 6, //!< Open layers in a read-only mode. (since QGIS 3.28)
    };
    Q_ENUM( ProjectReadFlag )

    /**
     * Project load flags.
     *
     * \note Prior to QGIS 3.26 this was available as QgsProject::ReadFlags.
     *
     * \since QGIS 3.26
     */
    Q_DECLARE_FLAGS( ProjectReadFlags, ProjectReadFlag ) SIP_MONKEYPATCH_FLAGS_UNNEST( QgsProject, ReadFlags )
    Q_FLAG( ProjectReadFlags )

    /**
     * Flags which control project capabilities.
     *
     * These flags are specific upfront on creation of a QgsProject object, and can
     * be used to selectively enable potentially costly functionality for the project.
     *
     * \since QGIS 3.26.1
     */
    enum class ProjectCapability : int SIP_ENUM_BASETYPE( IntFlag )
    {
      ProjectStyles = 1 << 0, //!< Enable the project embedded style library. Enabling this flag can increase the time required to clear and load projects.
    };
    Q_ENUM( ProjectCapability )

    /**
     * Flags which control project capabilities.
     *
     * \since QGIS 3.26.1
     */
    Q_DECLARE_FLAGS( ProjectCapabilities, ProjectCapability )
    Q_FLAG( ProjectCapabilities )

    /**
     * Available MapBox GL style source types.
     *
     * \since QGIS 3.28
     */
    enum class MapBoxGlStyleSourceType : int
    {
      Vector, //!< Vector source
      Raster, //!< Raster source
      RasterDem, //!< Raster DEM source
      GeoJson, //!< GeoJSON source
      Image, //!< Image source
      Video, //!< Video source
      Unknown, //!< Other/unknown source type
    };
    Q_ENUM( MapBoxGlStyleSourceType )

    /**
     * Available ArcGIS REST service types.
     *
     * \note Prior to QGIS 3.26 this was available as QgsArcGisPortalUtils::ItemType.
     *
     * \since QGIS 3.28
     */
    enum class ArcGisRestServiceType SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsArcGisPortalUtils, ItemType ) : int
      {
      FeatureServer SIP_MONKEYPATCH_COMPAT_NAME( FeatureService ), //!< FeatureServer
      MapServer SIP_MONKEYPATCH_COMPAT_NAME( MapService ), //!< MapServer
      ImageServer SIP_MONKEYPATCH_COMPAT_NAME( ImageService ), //!< ImageServer
      GlobeServer, //!< GlobeServer
      GPServer, //!< GPServer
      GeocodeServer, //!< GeocodeServer
      Unknown, //!< Other unknown/unsupported type
    };
    Q_ENUM( ArcGisRestServiceType )

    /**
     * Relationship types.
     *
     * \note Prior to QGIS 3.28 this was available as QgsRelation::RelationType.
     *
     * \since QGIS 3.28
     */
    enum class RelationshipType SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsRelation, RelationType ) : int
      {
      Normal, //!< A normal relation
      Generated, //!< A generated relation is a child of a polymorphic relation
    };
    Q_ENUM( RelationshipType )

    /**
     * Relationship strength.
     *
     * \note Prior to QGIS 3.28 this was available as QgsRelation::RelationStrength.
     *
     * \since QGIS 3.28
     */
    enum class RelationshipStrength SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsRelation, RelationStrength ) : int
      {
      Association, //!< Loose relation, related elements are not part of the parent and a parent copy will not copy any children.
      Composition, //!< Fix relation, related elements are part of the parent and a parent copy will copy any children or delete of parent will delete children
    };
    Q_ENUM( RelationshipStrength )

    /**
     * Relationship cardinality.
     *
     * \since QGIS 3.28
     */
    enum class RelationshipCardinality : int
    {
      OneToOne, //!< One to one relationship
      OneToMany, //!< One to many relationship
      ManyToOne, //!< Many to one relationship
      ManyToMany, //!< Many to many relationship
    };
    Q_ENUM( RelationshipCardinality )

    /**
     * Relationship capabilities.
     *
     * \since QGIS 3.30
     */
    enum class RelationshipCapability : int SIP_ENUM_BASETYPE( IntFlag )
    {
      MultipleFieldKeys = 1 << 0, //!< Supports multiple field keys (as opposed to a singular field)
      ForwardPathLabel = 1 << 1, //!< Supports forward path labels
      BackwardPathLabel = 1 << 2, //!< Supports backward path labels
    };
    Q_ENUM( RelationshipCapability )

    /**
     * Relationship capabilities.
     *
     * \since QGIS 3.30
     */
    Q_DECLARE_FLAGS( RelationshipCapabilities, RelationshipCapability )
    Q_FLAG( RelationshipCapabilities )

    /**
     * Formats for displaying coordinates
     *
     * \since QGIS 3.28
     */
    enum class CoordinateDisplayType : int
    {
      MapCrs, //!< Map CRS
      MapGeographic, //!< Map Geographic CRS equivalent (stays unchanged if the map CRS is geographic)
      CustomCrs, //!< Custom CRS
    };
    Q_ENUM( CoordinateDisplayType )

    /**
     * The setting origin describes where a setting is stored.
     *
     * \since QGIS 3.30
     */
    enum class SettingsOrigin : int
    {
      Any, //!< From any origin
      Global, //!< Global settings are stored in `qgis_global_settings.ini`
      Local, //!< Local settings are stored in the user profile
    };
    Q_ENUM( SettingsOrigin )

    /**
     * Scripting languages.
     *
     * \since QGIS 3.30
     */
    enum class ScriptLanguage : int
    {
      Css, //!< CSS
      QgisExpression, //!< QGIS expressions
      Html, //!< HTML
      JavaScript, //!< JavaScript
      Json, //!< JSON
      Python, //!< Python
      R, //!< R Stats
      Sql, //!< SQL
      Batch, //!< Windows batch files
      Bash, //!< Bash scripts
      Unknown, //!< Unknown/other language
    };
    Q_ENUM( ScriptLanguage )

    /**
     * Script language capabilities.
     *
     * The flags reflect the support capabilities of a scripting language.
     *
     * \since QGIS 3.32
     */
    enum class ScriptLanguageCapability : int SIP_ENUM_BASETYPE( IntFlag )
    {
      Reformat = 1 << 0, //!< Language supports automatic code reformatting
      CheckSyntax = 1 << 1, //!< Language supports syntax checking
      ToggleComment = 1 << 2, //!< Language supports comment toggling
    };
    Q_ENUM( ScriptLanguageCapability )

    /**
     * Script language capabilities.
     *
     * \since QGIS 3.32
     */
    Q_DECLARE_FLAGS( ScriptLanguageCapabilities, ScriptLanguageCapability )
    Q_FLAG( ScriptLanguageCapabilities )

    /**
     * Layer tree insertion methods
     *
     * \since QGIS 3.30
     */
    enum class LayerTreeInsertionMethod : int
    {
      AboveInsertionPoint, //!< Layers are added in the tree above the insertion point
      TopOfTree, //!< Layers are added at the top of the layer tree
      OptimalInInsertionGroup, //!< Layers are added at optimal locations across the insertion point's group
    };
    Q_ENUM( LayerTreeInsertionMethod )

    /**
     * Layer tree filter flags.
     *
     * \since QGIS 3.32
     */
    enum class LayerTreeFilterFlag : int SIP_ENUM_BASETYPE( IntFlag )
    {
      SkipVisibilityCheck = 1 << 0, //!< If set, the standard visibility check should be skipped
    };
    Q_ENUM( LayerTreeFilterFlag )

    /**
     * Layer tree filter flags.
     *
     * \since QGIS 3.32
     */
    Q_DECLARE_FLAGS( LayerTreeFilterFlags, LayerTreeFilterFlag )
    Q_FLAG( LayerTreeFilterFlags )


    /**
     * Legend JSON export flags.
     *
     * Flags to control JSON attributes when exporting a legend in JSON format.
     *
     * \since QGIS 3.36
     */
    enum class LegendJsonRenderFlag : int SIP_ENUM_BASETYPE( IntFlag )
    {
      ShowRuleDetails = 1 << 0, //!< If set, the rule expression of a rule based renderer legend item will be added to the JSON
    };
    Q_ENUM( LegendJsonRenderFlag )
    Q_DECLARE_FLAGS( LegendJsonRenderFlags, LegendJsonRenderFlag )
    Q_FLAG( LegendJsonRenderFlags )

    /**
     * Action types.
     *
     * Prior to QGIS 3.30 this was available as QgsActionMenu::ActionType
     *
     * \since QGIS 3.30
     */
    enum class ActionType : int
    {
      Invalid, //!< Invalid
      MapLayerAction, //!< Standard actions (defined by core or plugins), corresponds to QgsMapLayerAction class.
      AttributeAction //!< Custom actions (manually defined in layer properties), corresponds to QgsAction class.
    };
    Q_ENUM( ActionType )

    /**
     * Map layer action targets.
     *
     * Prior to QGIS 3.30 this was available as QgsMapLayerAction::Target
     *
     * \since QGIS 3.30
     */
    enum class MapLayerActionTarget : int SIP_ENUM_BASETYPE( IntFlag )
    {
      Layer = 1 << 0, //!< Action targets a complete layer
      SingleFeature = 1 << 1, //!< Action targets a single feature from a layer
      MultipleFeatures = 1 << 2, //!< Action targets multiple features from a layer
      AllActions = Layer | SingleFeature | MultipleFeatures
    };
    Q_ENUM( MapLayerActionTarget )

    /**
     * Map layer action targets.
     *
     * Prior to QGIS 3.30 this was available as QgsMapLayerAction::Targets
     *
     * \since QGIS 3.30
     */
    Q_DECLARE_FLAGS( MapLayerActionTargets, MapLayerActionTarget )
    Q_FLAG( MapLayerActionTargets )

    /**
     * Map layer action flags.
     *
     * Prior to QGIS 3.30 this was available as QgsMapLayerAction::Flag
     *
     * \since QGIS 3.30
     */
    enum class MapLayerActionFlag : int SIP_ENUM_BASETYPE( IntFlag )
    {
      EnabledOnlyWhenEditable = 1 << 1, //!< Action should be shown only for editable layers
    };
    Q_ENUM( MapLayerActionFlag )

    /**
     * Map layer action flags.
     *
     * Prior to QGIS 3.30 this was available as QgsMapLayerAction::Flags
     *
     * \since QGIS 3.30
     */
    Q_DECLARE_FLAGS( MapLayerActionFlags, MapLayerActionFlag )
    Q_FLAG( MapLayerActionFlags )

    /**
     * Attribute action types.
     *
     * Prior to QGIS 3.30 this was available as QgsAction::ActionType
     *
     * \since QGIS 3.30
     */
    enum class AttributeActionType SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsAction, ActionType ) : int
      {
      Generic, //!< Generic
      GenericPython, //!< Python
      Mac, //!< MacOS specific
      Windows, //!< Windows specific
      Unix, //!< Unix specific
      OpenUrl, //!< Open URL action
      SubmitUrlEncoded, //!< POST data to an URL, using "application/x-www-form-urlencoded" or "application/json" if the body is valid JSON \since QGIS 3.24
      SubmitUrlMultipart, //!< POST data to an URL using "multipart/form-data"  \since QGIS 3.24
    };
    Q_ENUM( AttributeActionType )

    /**
     * Date types for metadata.
     *
     * \since QGIS 3.30
     */
    enum class MetadataDateType
    {
      Created, //!< Date created
      Published, //!< Date published
      Revised, //!< Date revised
      Superseded, //!< Date superseded
    };
    Q_ENUM( MetadataDateType )

    /**
     * Raster color interpretation.
     *
     * This is a modified copy of the GDAL GDALColorInterp enum.
     *
     * \note Prior to QGIS 3.30 this was available as QgsRaster::ColorInterpretation
     *
     * \since QGIS 3.30
     */
    enum class RasterColorInterpretation SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsRaster, ColorInterpretation ) : int
      {
      Undefined SIP_MONKEYPATCH_COMPAT_NAME( UndefinedColorInterpretation ) = 0, //!< Undefined
      GrayIndex = 1,          //!< Grayscale
      PaletteIndex = 2,       //!< Paletted (see associated color table)
      RedBand = 3,            //!< Red band of RGBA image
      GreenBand = 4,          //!< Green band of RGBA image
      BlueBand = 5,           //!< Blue band of RGBA image
      AlphaBand = 6,          //!< Alpha (0=transparent, 255=opaque)
      HueBand = 7,            //!< Hue band of HLS image
      SaturationBand = 8,     //!< Saturation band of HLS image
      LightnessBand = 9,      //!< Lightness band of HLS image
      CyanBand = 10,          //!< Cyan band of CMYK image
      MagentaBand = 11,       //!< Magenta band of CMYK image
      YellowBand = 12,        //!< Yellow band of CMYK image
      BlackBand = 13,         //!< Black band of CMLY image
      YCbCr_YBand = 14,       //!< Y Luminance
      YCbCr_CbBand = 15,      //!< Cb Chroma
      YCbCr_CrBand = 16,      //!< Cr Chroma
      ContinuousPalette = 17  //!< Continuous palette, QGIS addition, GRASS
    };
    Q_ENUM( RasterColorInterpretation )

    /**
     * Raster layer types.
     *
     * \note Prior to QGIS 3.30 this was available as QgsRasterLayer::LayerType
     *
     * \since QGIS 3.30
    */
    enum class RasterLayerType SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsRasterLayer, LayerType ) : int
      {
      GrayOrUndefined, //!< Gray or undefined
      Palette, //!< Palette
      MultiBand SIP_MONKEYPATCH_COMPAT_NAME( Multiband ), //!< Multi band
      SingleBandColorData SIP_MONKEYPATCH_COMPAT_NAME( ColorLayer ), //!< Single band containing color data
    };
    Q_ENUM( RasterLayerType )

    /**
     * Raster drawing styles.
     *
     * \note Prior to QGIS 3.30 this was available as QgsRaster::DrawingStyle
     *
     * \since QGIS 3.30
    */
    enum class RasterDrawingStyle SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsRaster, DrawingStyle ) : int
      {
      Undefined SIP_MONKEYPATCH_COMPAT_NAME( UndefinedDrawingStyle ),//!< Undefined
      SingleBandGray, //!< A single band image drawn as a range of gray colors
      SingleBandPseudoColor, //!< A single band image drawn using a pseudocolor algorithm
      PalettedColor, //!< A "Palette" image drawn using color table
      PalettedSingleBandGray, //!< A "Palette" layer drawn in gray scale
      PalettedSingleBandPseudoColor, //!< A "Palette" layerdrawn using a pseudocolor algorithm
      PalettedMultiBandColor, //!< Currently not supported
      MultiBandSingleBandGray, //!< A layer containing 2 or more bands, but a single band drawn as a range of gray colors
      MultiBandSingleBandPseudoColor, //!< A layer containing 2 or more bands, but a single band drawn using a pseudocolor algorithm
      MultiBandColor, //!< A layer containing 2 or more bands, mapped to RGB color space. In the case of a multiband with only two bands, one band will be mapped to more than one color.
      SingleBandColorData SIP_MONKEYPATCH_COMPAT_NAME( SingleBandColorDataStyle ), //!< ARGB values rendered directly
    };
    Q_ENUM( RasterDrawingStyle )

    /**
     * Raster pyramid formats.
     *
     * \note Prior to QGIS 3.30 this was available as QgsRaster::RasterPyramidsFormat
     *
     * \since QGIS 3.30
     */
    enum class RasterPyramidFormat SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsRaster, RasterPyramidsFormat ) : int
      {
      GeoTiff SIP_MONKEYPATCH_COMPAT_NAME( PyramidsGTiff ) = 0, //!< Geotiff .ovr (external)
      Internal SIP_MONKEYPATCH_COMPAT_NAME( PyramidsInternal ) = 1, //!< Internal
      Erdas SIP_MONKEYPATCH_COMPAT_NAME( PyramidsErdas ) = 2 //!< Erdas Image .aux (external)
    };
    Q_ENUM( RasterPyramidFormat )

    /**
     * Raster pyramid building options.
     *
     * \note Prior to QGIS 3.30 this was available as QgsRaster::RasterBuildPyramids
     *
     * \since QGIS 3.30
     */
    enum class RasterBuildPyramidOption SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsRaster, RasterBuildPyramids ) : int
      {
      No SIP_MONKEYPATCH_COMPAT_NAME( PyramidsFlagNo ) = 0, //!< Never
      Yes SIP_MONKEYPATCH_COMPAT_NAME( PyramidsFlagYes ) = 1, //!< Yes
      CopyExisting SIP_MONKEYPATCH_COMPAT_NAME( PyramidsCopyExisting ) = 2 //!< Copy existing
    };
    Q_ENUM( RasterBuildPyramidOption )

    /**
     * Raster identify formats.
     *
     * \note Prior to QGIS 3.30 this was available as QgsRaster::IdentifyFormat
     *
     * \since QGIS 3.30
     */
    enum class RasterIdentifyFormat SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsRaster, IdentifyFormat ) : int SIP_ENUM_BASETYPE( IntFlag )
    {
      Undefined SIP_MONKEYPATCH_COMPAT_NAME( IdentifyFormatUndefined ) = 0, //!< Undefined
      Value SIP_MONKEYPATCH_COMPAT_NAME( IdentifyFormatValue ) = 1, //!< Numerical pixel value
      Text SIP_MONKEYPATCH_COMPAT_NAME( IdentifyFormatText ) = 1 << 1, //!< WMS text
      Html SIP_MONKEYPATCH_COMPAT_NAME( IdentifyFormatHtml ) = 1 << 2, //!< WMS HTML
      Feature SIP_MONKEYPATCH_COMPAT_NAME( IdentifyFormatFeature ) = 1 << 3, //!< WMS GML/JSON -> feature
    };
    Q_ENUM( RasterIdentifyFormat )

    /**
     * Methods used to select the elevation when two elevation maps are combined
     *
     * \since QGIS 3.30
     */
    enum class ElevationMapCombineMethod : int
    {
      HighestElevation, //!< Keep the highest elevation if it is not null
      NewerElevation, //!< Keep the new elevation regardless of its value if it is not null
    };
    Q_ENUM( ElevationMapCombineMethod )

    /**
     * Blending modes defining the available composition modes that can
     * be used when painting.
     *
     * \note Prior to QGIS 3.30 this was available as QgsPainting::BlendMode.
     *
     * \since QGIS 3.30
     */
    enum class BlendMode SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsPainting, BlendMode ) : int
      {
      Normal SIP_MONKEYPATCH_COMPAT_NAME( BlendNormal ), //!< Normal
      Lighten SIP_MONKEYPATCH_COMPAT_NAME( BlendLighten ), //!< Lighten
      Screen SIP_MONKEYPATCH_COMPAT_NAME( BlendScreen ), //!< Screen
      Dodge SIP_MONKEYPATCH_COMPAT_NAME( BlendDodge ), //!< Dodge
      Addition SIP_MONKEYPATCH_COMPAT_NAME( BlendAddition ), //!< Addition
      Darken SIP_MONKEYPATCH_COMPAT_NAME( BlendDarken ), //!< Darken
      Multiply SIP_MONKEYPATCH_COMPAT_NAME( BlendMultiply ), //!< Multiple
      Burn SIP_MONKEYPATCH_COMPAT_NAME( BlendBurn ), //!< Burn
      Overlay SIP_MONKEYPATCH_COMPAT_NAME( BlendOverlay ), //!< Overlay
      SoftLight SIP_MONKEYPATCH_COMPAT_NAME( BlendSoftLight ), //!< Soft light
      HardLight SIP_MONKEYPATCH_COMPAT_NAME( BlendHardLight ), //!< Hard light
      Difference SIP_MONKEYPATCH_COMPAT_NAME( BlendDifference ), //!< Difference
      Subtract SIP_MONKEYPATCH_COMPAT_NAME( BlendSubtract ), //!< Subtract
      Source SIP_MONKEYPATCH_COMPAT_NAME( BlendSource ), //!< Source
      DestinationOver SIP_MONKEYPATCH_COMPAT_NAME( BlendDestinationOver ), //!< Destination over
      Clear SIP_MONKEYPATCH_COMPAT_NAME( BlendClear ), //!< Clear
      Destination SIP_MONKEYPATCH_COMPAT_NAME( BlendDestination ), //!< Destination
      SourceIn SIP_MONKEYPATCH_COMPAT_NAME( BlendSourceIn ), //!< Source in
      DestinationIn SIP_MONKEYPATCH_COMPAT_NAME( BlendDestinationIn ), //!< Destination in
      SourceOut SIP_MONKEYPATCH_COMPAT_NAME( BlendSourceOut ), //!< Source out
      DestinationOut SIP_MONKEYPATCH_COMPAT_NAME( BlendDestinationOut ), //!< Destination out
      SourceAtop SIP_MONKEYPATCH_COMPAT_NAME( BlendSourceAtop ), //!< Source atop
      DestinationAtop SIP_MONKEYPATCH_COMPAT_NAME( BlendDestinationAtop ), //!< Destination atop
      Xor SIP_MONKEYPATCH_COMPAT_NAME( BlendXor ), //!< XOR
    };
    Q_ENUM( BlendMode )

    /**
     * Systems of unit measurement.
     *
     * \note Prior to QGIS 3.30 this was available as QgsUnitTypes::SystemOfMeasurement.
     *
     * \since QGIS 3.30
     */
    enum class SystemOfMeasurement SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsUnitTypes, SystemOfMeasurement ) : int
      {
      Unknown SIP_MONKEYPATCH_COMPAT_NAME( UnknownSystem ) = 0, //!< Unknown system of measurement
      Metric SIP_MONKEYPATCH_COMPAT_NAME( MetricSystem ), //!< International System of Units (SI)
      Imperial SIP_MONKEYPATCH_COMPAT_NAME( ImperialSystem ), //!< British Imperial
      USCS SIP_MONKEYPATCH_COMPAT_NAME( USCSSystem ), //!< United States customary system
    };
    Q_ENUM( SystemOfMeasurement )

    /**
    * Type of unit of tolerance value from settings.
    * For map (project) units, use MapToolUnit::Project.
    *
    * \since QGIS 3.32
    */
    enum class MapToolUnit SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsTolerance, UnitType ) : int
      {
      Layer SIP_MONKEYPATCH_COMPAT_NAME( LayerUnits ), //!< Layer unit value
      Pixels, //!< Pixels unit of tolerance
      Project  SIP_MONKEYPATCH_COMPAT_NAME( ProjectUnits ) //!< Map (project) units
    };
    Q_ENUM( MapToolUnit )

    /**
     * Unit types.
     *
     * \note Prior to QGIS 3.30 this was available as QgsUnitTypes::UnitType.
     *
     * \since QGIS 3.30
     */
    enum class UnitType SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsUnitTypes, UnitType ) : int
      {
      Distance SIP_MONKEYPATCH_COMPAT_NAME( TypeDistance ) = 0, //!< Distance unit
      Area SIP_MONKEYPATCH_COMPAT_NAME( TypeArea ), //!< Area unit
      Volume SIP_MONKEYPATCH_COMPAT_NAME( TypeVolume ), //!< Volume unit
      Unknown SIP_MONKEYPATCH_COMPAT_NAME( TypeUnknown ), //!< Unknown unit type
      Temporal SIP_MONKEYPATCH_COMPAT_NAME( TypeTemporal ), //!< Temporal unit
    };
    Q_ENUM( UnitType )

    /**
     * Units of distance
     *
     * \note Prior to QGIS 3.30 this was available as QgsUnitTypes::DistanceUnit.
     *
     * \since QGIS 3.30
     */
    enum class DistanceUnit SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsUnitTypes, DistanceUnit ) : int
      {
      Meters SIP_MONKEYPATCH_COMPAT_NAME( DistanceMeters ), //!< Meters
      Kilometers SIP_MONKEYPATCH_COMPAT_NAME( DistanceKilometers ), //!< Kilometers
      Feet SIP_MONKEYPATCH_COMPAT_NAME( DistanceFeet ), //!< Imperial feet
      NauticalMiles SIP_MONKEYPATCH_COMPAT_NAME( DistanceNauticalMiles ), //!< Nautical miles
      Yards SIP_MONKEYPATCH_COMPAT_NAME( DistanceYards ), //!< Imperial yards
      Miles SIP_MONKEYPATCH_COMPAT_NAME( DistanceMiles ), //!< Terrestrial miles
      Degrees SIP_MONKEYPATCH_COMPAT_NAME( DistanceDegrees ), //!< Degrees, for planar geographic CRS distance measurements
      Centimeters SIP_MONKEYPATCH_COMPAT_NAME( DistanceCentimeters ), //!< Centimeters
      Millimeters SIP_MONKEYPATCH_COMPAT_NAME( DistanceMillimeters ), //!< Millimeters
      Inches, //!< Inches (since QGIS 3.32)
      Unknown SIP_MONKEYPATCH_COMPAT_NAME( DistanceUnknownUnit ), //!< Unknown distance unit
    };
    Q_ENUM( DistanceUnit )

    /**
     * Types of distance units
     *
     * \note Prior to QGIS 3.30 this was available as QgsUnitTypes::DistanceUnitType.
     *
     * \since QGIS 3.30
     */
    enum class DistanceUnitType SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsUnitTypes, DistanceUnitType ) : int
      {
      Standard, //!< Unit is a standard measurement unit
      Geographic, //!< Unit is a geographic (e.g., degree based) unit
      Unknown SIP_MONKEYPATCH_COMPAT_NAME( UnknownType ),  //!< Unknown unit type
    };
    Q_ENUM( DistanceUnitType )

    /**
     * Units of area
     *
     * \note Prior to QGIS 3.30 this was available as QgsUnitTypes::AreaUnit.
     *
     * \since QGIS 3.30
     */
    enum class AreaUnit SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsUnitTypes, AreaUnit ) : int
      {
      SquareMeters SIP_MONKEYPATCH_COMPAT_NAME( AreaSquareMeters ), //!< Square meters
      SquareKilometers SIP_MONKEYPATCH_COMPAT_NAME( AreaSquareKilometers ), //!< Square kilometers
      SquareFeet SIP_MONKEYPATCH_COMPAT_NAME( AreaSquareFeet ), //!< Square feet
      SquareYards SIP_MONKEYPATCH_COMPAT_NAME( AreaSquareYards ), //!< Square yards
      SquareMiles SIP_MONKEYPATCH_COMPAT_NAME( AreaSquareMiles ), //!< Square miles
      Hectares SIP_MONKEYPATCH_COMPAT_NAME( AreaHectares ), //!< Hectares
      Acres SIP_MONKEYPATCH_COMPAT_NAME( AreaAcres ), //!< Acres
      SquareNauticalMiles SIP_MONKEYPATCH_COMPAT_NAME( AreaSquareNauticalMiles ), //!< Square nautical miles
      SquareDegrees SIP_MONKEYPATCH_COMPAT_NAME( AreaSquareDegrees ), //!< Square degrees, for planar geographic CRS area measurements
      SquareCentimeters SIP_MONKEYPATCH_COMPAT_NAME( AreaSquareCentimeters ), //!< Square centimeters
      SquareMillimeters SIP_MONKEYPATCH_COMPAT_NAME( AreaSquareMillimeters ), //!< Square millimeters
      SquareInches, //!< Square inches (since QGIS 3.32)
      Unknown SIP_MONKEYPATCH_COMPAT_NAME( AreaUnknownUnit ), //!< Unknown areal unit
    };
    Q_ENUM( AreaUnit )

    /**
     * Units of volume.
     *
     * \note Prior to QGIS 3.30 this was available as QgsUnitTypes::VolumeUnit.
     *
     * \since QGIS 3.30
     */
    enum class VolumeUnit SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsUnitTypes, VolumeUnit ) : int
      {
      CubicMeters SIP_MONKEYPATCH_COMPAT_NAME( VolumeCubicMeters ), //!< Cubic meters
      CubicFeet SIP_MONKEYPATCH_COMPAT_NAME( VolumeCubicFeet ), //!< Cubic feet
      CubicYards SIP_MONKEYPATCH_COMPAT_NAME( VolumeCubicYards ), //!< Cubic yards
      Barrel SIP_MONKEYPATCH_COMPAT_NAME( VolumeBarrel ), //!< Barrels
      CubicDecimeter SIP_MONKEYPATCH_COMPAT_NAME( VolumeCubicDecimeter ), //!< Cubic decimeters
      Liters SIP_MONKEYPATCH_COMPAT_NAME( VolumeLiters ), //!< Litres
      GallonUS SIP_MONKEYPATCH_COMPAT_NAME( VolumeGallonUS ), //!< US Gallons
      CubicInch SIP_MONKEYPATCH_COMPAT_NAME( VolumeCubicInch ), //!< Cubic inches
      CubicCentimeter SIP_MONKEYPATCH_COMPAT_NAME( VolumeCubicCentimeter ), //!< Cubic Centimeters
      CubicDegrees SIP_MONKEYPATCH_COMPAT_NAME( VolumeCubicDegrees ), //!< Cubic degrees, for planar geographic CRS volume measurements
      Unknown SIP_MONKEYPATCH_COMPAT_NAME( VolumeUnknownUnit ), //!< Unknown volume unit
    };
    Q_ENUM( VolumeUnit )

    /**
     * Units of angles.
     *
     * \note Prior to QGIS 3.30 this was available as QgsUnitTypes::AngleUnit.
     *
     * \since QGIS 3.30
     */
    enum class AngleUnit SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsUnitTypes, AngleUnit ) : int
      {
      Degrees SIP_MONKEYPATCH_COMPAT_NAME( AngleDegrees ), //!< Degrees
      Radians SIP_MONKEYPATCH_COMPAT_NAME( AngleRadians ), //!< Square kilometers
      Gon SIP_MONKEYPATCH_COMPAT_NAME( AngleGon ), //!< Gon/gradian
      MinutesOfArc SIP_MONKEYPATCH_COMPAT_NAME( AngleMinutesOfArc ), //!< Minutes of arc
      SecondsOfArc SIP_MONKEYPATCH_COMPAT_NAME( AngleSecondsOfArc ), //!< Seconds of arc
      Turn SIP_MONKEYPATCH_COMPAT_NAME( AngleTurn ), //!< Turn/revolutions
      MilliradiansSI SIP_MONKEYPATCH_COMPAT_NAME( AngleMilliradiansSI ), //!< Angular milliradians (SI definition, 1/1000 of radian)
      MilNATO SIP_MONKEYPATCH_COMPAT_NAME( AngleMilNATO ), //!< Angular mil (NATO definition, 6400 mil = 2PI radians)
      Unknown SIP_MONKEYPATCH_COMPAT_NAME( AngleUnknownUnit ), //!< Unknown angle unit
    };
    Q_ENUM( AngleUnit )

    /**
     * Temporal units.
     *
     * \note Prior to QGIS 3.30 this was available as QgsUnitTypes::TemporalUnit.
     *
     * \since QGIS 3.30
     */
    enum class TemporalUnit SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsUnitTypes, TemporalUnit ) : int
      {
      Milliseconds SIP_MONKEYPATCH_COMPAT_NAME( TemporalMilliseconds ), //!< Milliseconds
      Seconds SIP_MONKEYPATCH_COMPAT_NAME( TemporalSeconds ), //!< Seconds
      Minutes SIP_MONKEYPATCH_COMPAT_NAME( TemporalMinutes ), //!< Minutes
      Hours SIP_MONKEYPATCH_COMPAT_NAME( TemporalHours ), //!< Hours
      Days SIP_MONKEYPATCH_COMPAT_NAME( TemporalDays ), //!< Days
      Weeks SIP_MONKEYPATCH_COMPAT_NAME( TemporalWeeks ), //!< Weeks
      Months SIP_MONKEYPATCH_COMPAT_NAME( TemporalMonths ),  //!< Months
      Years SIP_MONKEYPATCH_COMPAT_NAME( TemporalYears ), //!< Years
      Decades SIP_MONKEYPATCH_COMPAT_NAME( TemporalDecades ), //!< Decades
      Centuries SIP_MONKEYPATCH_COMPAT_NAME( TemporalCenturies ), //!< Centuries
      IrregularStep SIP_MONKEYPATCH_COMPAT_NAME( TemporalIrregularStep ), //!< Special 'irregular step' time unit, used for temporal data which uses irregular, non-real-world unit steps (since QGIS 3.20)
      Unknown SIP_MONKEYPATCH_COMPAT_NAME( TemporalUnknownUnit ) //!< Unknown time unit
    };
    Q_ENUM( TemporalUnit )

    /**
     * Rendering size units
     *
     * \note Prior to QGIS 3.30 this was available as QgsUnitTypes::RenderUnit.
     *
     * \since QGIS 3.30
     */
    enum class RenderUnit SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsUnitTypes, RenderUnit ) : int
      {
      Millimeters SIP_MONKEYPATCH_COMPAT_NAME( RenderMillimeters ), //!< Millimeters
      MapUnits SIP_MONKEYPATCH_COMPAT_NAME( RenderMapUnits ), //!< Map units
      Pixels SIP_MONKEYPATCH_COMPAT_NAME( RenderPixels ), //!< Pixels
      Percentage SIP_MONKEYPATCH_COMPAT_NAME( RenderPercentage ), //!< Percentage of another measurement (e.g., canvas size, feature size)
      Points SIP_MONKEYPATCH_COMPAT_NAME( RenderPoints ), //!< Points (e.g., for font sizes)
      Inches SIP_MONKEYPATCH_COMPAT_NAME( RenderInches ), //!< Inches
      Unknown SIP_MONKEYPATCH_COMPAT_NAME( RenderUnknownUnit ), //!< Mixed or unknown units
      MetersInMapUnits SIP_MONKEYPATCH_COMPAT_NAME( RenderMetersInMapUnits ), //!< Meters value as Map units
    };
    Q_ENUM( RenderUnit )

    /**
     * Layout measurement units
     *
     * \note Prior to QGIS 3.30 this was available as QgsUnitTypes::LayoutUnit.
     *
     * \since QGIS 3.30
     */
    enum class LayoutUnit SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsUnitTypes, LayoutUnit ) : int
      {
      Millimeters SIP_MONKEYPATCH_COMPAT_NAME( LayoutMillimeters ), //!< Millimeters
      Centimeters SIP_MONKEYPATCH_COMPAT_NAME( LayoutCentimeters ), //!< Centimeters
      Meters SIP_MONKEYPATCH_COMPAT_NAME( LayoutMeters ), //!< Meters
      Inches SIP_MONKEYPATCH_COMPAT_NAME( LayoutInches ), //!< Inches
      Feet SIP_MONKEYPATCH_COMPAT_NAME( LayoutFeet ), //!< Feet
      Points SIP_MONKEYPATCH_COMPAT_NAME( LayoutPoints ), //!< Typographic points
      Picas SIP_MONKEYPATCH_COMPAT_NAME( LayoutPicas ), //!< Typographic picas
      Pixels SIP_MONKEYPATCH_COMPAT_NAME( LayoutPixels ) //!< Pixels
    };
    Q_ENUM( LayoutUnit )

    /**
     * Types of layout units
     *
     * \note Prior to QGIS 3.30 this was available as QgsUnitTypes::LayoutUnitType.
     *
     * \since QGIS 3.30
     */
    enum class LayoutUnitType SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsUnitTypes, LayoutUnitType ) : int
      {
      PaperUnits SIP_MONKEYPATCH_COMPAT_NAME( LayoutPaperUnits ), //!< Unit is a paper based measurement unit
      ScreenUnits SIP_MONKEYPATCH_COMPAT_NAME( LayoutScreenUnits ) //!< Unit is a screen based measurement unit
    };
    Q_ENUM( LayoutUnitType )

    /**
     * Input controller types.
     *
     * \since QGIS 3.34
     */
    enum class InputControllerType : int
    {
      Map2D, //!< 2D map controller
      Map3D //!< 3D map controller
    };
    Q_ENUM( InputControllerType );

    /**
     * Postgres database relkind options.
     *
     * \since QGIS 3.32
     */
    enum class PostgresRelKind
    {
      NotSet, //!< Not set
      Unknown, //!< Unknown
      OrdinaryTable, //!< Ordinary table
      Index, //!< Index
      Sequence, //!< Sequence
      View, //!< View
      MaterializedView, //!< Materialized view
      CompositeType, //!< Composition type
      ToastTable, //!< TOAST table
      ForeignTable, //!< Foreign table
      PartitionedTable, //!< Partitioned table
    };
    Q_ENUM( PostgresRelKind )

    /**
     * The Capability enum represents the extended operations supported by the connection.
     *
     * \since QGIS 3.32
     */
    enum class DatabaseProviderConnectionCapability2 : int SIP_ENUM_BASETYPE( IntFlag )
    {
      SetFieldComment = 1 << 0, //!< Can set comments for fields via setFieldComment()
      SetFieldAlias = 1 << 1, //!< Can set aliases for fields via setFieldAlias()
    };
    Q_ENUM( DatabaseProviderConnectionCapability2 )
    Q_DECLARE_FLAGS( DatabaseProviderConnectionCapabilities2, DatabaseProviderConnectionCapability2 )
    Q_FLAG( DatabaseProviderConnectionCapabilities2 )

    /**
     * The StorageCapability enum represents the style storage operations supported by the provider.
     *
     * \since QGIS 3.34
     */
    enum class ProviderStyleStorageCapability : int SIP_ENUM_BASETYPE( IntFlag )
    {
      SaveToDatabase = 1 << 1,
      LoadFromDatabase = 1 << 2,
      DeleteFromDatabase = 1 << 3
    };
    Q_ENUM( ProviderStyleStorageCapability );
    Q_DECLARE_FLAGS( ProviderStyleStorageCapabilities, ProviderStyleStorageCapability )
    Q_FLAG( ProviderStyleStorageCapabilities )

    /**
     * User profile selection policy.
     *
     * \since QGIS 3.32
     */
    enum class UserProfileSelectionPolicy : int
    {
      LastProfile, //!< Open the last closed profile (only mode supported prior to QGIS 3.32)
      DefaultProfile, //!< Open a specific profile
      AskUser, //!< Let the user choose which profile to open
    };
    Q_ENUM( UserProfileSelectionPolicy )

    /**
     * Attribute editor types.
     *
     * \note Prior to QGIS 3.32 this was available as QgsAttributeEditorElement::AttributeEditorType.
     *
     * \since QGIS 3.32
     */
    enum class AttributeEditorType SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsAttributeEditorElement, AttributeEditorType ) : int
      {
      Container SIP_MONKEYPATCH_COMPAT_NAME( AeTypeContainer ), //!< A container
      Field SIP_MONKEYPATCH_COMPAT_NAME( AeTypeField ), //!< A field
      Relation SIP_MONKEYPATCH_COMPAT_NAME( AeTypeRelation ), //!< A relation
      QmlElement SIP_MONKEYPATCH_COMPAT_NAME( AeTypeQmlElement ), //!< A QML element
      HtmlElement SIP_MONKEYPATCH_COMPAT_NAME( AeTypeHtmlElement ), //!< A HTML element
      Action SIP_MONKEYPATCH_COMPAT_NAME( AeTypeAction ), //!< A layer action element (since QGIS 3.22)
      TextElement SIP_MONKEYPATCH_COMPAT_NAME( AeTypeTextElement ), //!< A text element (since QGIS 3.30)
      SpacerElement SIP_MONKEYPATCH_COMPAT_NAME( AeTypeSpacerElement ), //!< A spacer element (since QGIS 3.30)
      Invalid SIP_MONKEYPATCH_COMPAT_NAME( AeTypeInvalid ), //!< Invalid
    };
    Q_ENUM( AttributeEditorType )

    /**
     * Attribute editor container types.
     *
     * \since QGIS 3.32
     */
    enum class AttributeEditorContainerType : int
    {
      GroupBox, //!< A group box
      Tab, //!< A tab widget
      Row, //!< A row of editors (horizontal layout)
    };
    Q_ENUM( AttributeEditorContainerType )

    /**
     * Available form types for layout of the attribute form editor.
     *
     * \note Prior to QGIS 3.32 this was available as QgsEditFormConfig::EditorLayout.
     *
     * \since QGIS 3.32
     */
    enum class AttributeFormLayout SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsEditFormConfig, EditorLayout ) : int
      {
      AutoGenerated SIP_MONKEYPATCH_COMPAT_NAME( GeneratedLayout ) = 0, //!< Autogenerate a simple tabular layout for the form
      DragAndDrop SIP_MONKEYPATCH_COMPAT_NAME( TabLayout ) = 1, //!< "Drag and drop" layout. Needs to be configured.
      UiFile SIP_MONKEYPATCH_COMPAT_NAME( UiFileLayout ) = 2 //!< Load a .ui file for the layout. Needs to be configured.
    };
    Q_ENUM( AttributeFormLayout )

    /**
     * Available form types for layout of the attribute form editor.
     *
     * \note Prior to QGIS 3.32 this was available as QgsEditFormConfig::FeatureFormSuppress.
     *
     * \since QGIS 3.32
     */
    enum class AttributeFormSuppression SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsEditFormConfig, FeatureFormSuppress ) : int
      {
      Default SIP_MONKEYPATCH_COMPAT_NAME( SuppressDefault ) = 0, //!< Use the application-wide setting.
      On SIP_MONKEYPATCH_COMPAT_NAME( SuppressOn ) = 1, //!< Always suppress feature form.
      Off SIP_MONKEYPATCH_COMPAT_NAME( SuppressOff ) = 2 //!< Never suppress feature form.
    };
    Q_ENUM( AttributeFormSuppression )

    /**
     * The Python init code source for attribute forms.
     *
     * \note Prior to QGIS 3.32 this was available as QgsEditFormConfig::PythonInitCodeSource.
     *
     * \since QGIS 3.32
     */
    enum class AttributeFormPythonInitCodeSource SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsEditFormConfig, PythonInitCodeSource ) : int
      {
      NoSource SIP_MONKEYPATCH_COMPAT_NAME( CodeSourceNone ) = 0, //!< Do not use Python code at all
      File SIP_MONKEYPATCH_COMPAT_NAME( CodeSourceFile ) = 1, //!< Load the Python code from an external file
      Dialog SIP_MONKEYPATCH_COMPAT_NAME( CodeSourceDialog ) = 2, //!< Use the Python code provided in the dialog
      Environment SIP_MONKEYPATCH_COMPAT_NAME( CodeSourceEnvironment ) = 3 //!< Use the Python code available in the Python environment
    };
    Q_ENUM( AttributeFormPythonInitCodeSource )

    /**
     * Expression types
     *
     * \since QGIS 3.32
     */
    enum class ExpressionType
    {
      Qgis, //!< Native QGIS expression
      PointCloud, //!< Point cloud expression
      RasterCalculator, //!< Raster calculator expression (since QGIS 3.34)
    };
    Q_ENUM( ExpressionType )

    /**
     * Options for exporting features considering their symbology.
     *
     * \note Prior to QGIS 3.32 this was available as QgsVectorFileWriter::SymbologyExport.
     *
     * \since QGIS 3.32
     */
    enum class FeatureSymbologyExport SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsVectorFileWriter, SymbologyExport ) : int
      {
      NoSymbology = 0, //!< Export only data
      PerFeature SIP_MONKEYPATCH_COMPAT_NAME( FeatureSymbology ), //!< Keeps the number of features and export symbology per feature
      PerSymbolLayer SIP_MONKEYPATCH_COMPAT_NAME( SymbolLayerSymbology ) //!< Exports one feature per symbol layer (considering symbol levels)
    };
    Q_ENUM( FeatureSymbologyExport )

    /**
     * Flags for vector tile data providers.
     *
     * \since QGIS 3.32
     */
    enum class VectorTileProviderFlag : int SIP_ENUM_BASETYPE( IntFlag )
    {
      AlwaysUseTileMatrixSetFromProvider = 1 << 1, //!< Vector tile layer must always use the tile matrix set from the data provider, and should never store, restore or override the definition of this matrix set.
    };
    Q_ENUM( VectorTileProviderFlag )

    /**
     * Vector tile data provider flags.
     *
     * \since QGIS 3.32
     */
    Q_DECLARE_FLAGS( VectorTileProviderFlags, VectorTileProviderFlag )
    Q_FLAG( VectorTileProviderFlags )

    /**
     * Enumeration with capabilities that vector tile data providers might implement.
     * \since QGIS 3.32
     */
    enum class VectorTileProviderCapability : int SIP_ENUM_BASETYPE( IntFlag )
    {
      ReadLayerMetadata = 1 << 1, //!< Provider can read layer metadata from data store. See QgsDataProvider::layerMetadata()
    };
    Q_ENUM( VectorTileProviderCapability )

    /**
     * Vector tile data provider capabilities.
     *
     * \since QGIS 3.32
     */
    Q_DECLARE_FLAGS( VectorTileProviderCapabilities, VectorTileProviderCapability )
    Q_FLAG( VectorTileProviderCapabilities )

    /**
     * Possible availability states for a tile within a tile matrix.
     *
     * \since QGIS 3.32
     */
    enum class TileAvailability
    {
      Available, //!< Tile is available within the matrix
      NotAvailable, //!< Tile is not available within the matrix, e.g. there is no content for the tile
      AvailableNoChildren, //!< Tile is available within the matrix, and is known to have no children (ie no higher zoom level tiles exist covering this tile's region)
      UseLowerZoomLevelTile, //!< Tile is not available at the requested zoom level, it should be replaced by a tile from a lower zoom level instead182
    };
    Q_ENUM( TileAvailability )

    /**
     * Tiled scene data provider capabilities.
     *
     * \since QGIS 3.34
     */
    enum class TiledSceneProviderCapability : int SIP_ENUM_BASETYPE( IntFlag )
    {
      ReadLayerMetadata = 1 << 1, //!< Provider can read layer metadata from data store. See QgsDataProvider::layerMetadata()
    };
    Q_ENUM( TiledSceneProviderCapability )

    /**
     * Tiled scene data provider capabilities.
     *
     * \since QGIS 3.34
     */
    Q_DECLARE_FLAGS( TiledSceneProviderCapabilities, TiledSceneProviderCapability )
    Q_FLAG( TiledSceneProviderCapabilities )

    /**
     * Tiled scene bounding volume types.
     *
     * \since QGIS 3.34
     */
    enum class TiledSceneBoundingVolumeType
    {
      Region, //!< Region type
      OrientedBox, //!< Oriented bounding box (rotated box)
      Sphere, //!< Sphere
    };
    Q_ENUM( TiledSceneBoundingVolumeType )

    /**
     * Tiled scene tile refinement processes.
     *
     * Refinement determines the process by which a lower resolution parent tile
     * renders when its higher resolution children are selected to be rendered.
     *
     * \since QGIS 3.34
     */
    enum class TileRefinementProcess
    {
      Replacement, //!< When tile is refined then its children should be used in place of itself.
      Additive, //!< When tile is refined its content should be used alongside its children simultaneously.
    };
    Q_ENUM( TileRefinementProcess )

    /**
     * Possible availability states for a tile's children.
     *
     * \since QGIS 3.34
     */
    enum class TileChildrenAvailability
    {
      NoChildren, //!< Tile is known to have no children
      Available, //!< Tile children are already available
      NeedFetching, //!< Tile has children, but they are not yet available and must be fetched
    };
    Q_ENUM( TileChildrenAvailability )

    /**
     * Flags which control how tiled scene requests behave.
     *
     * \since QGIS 3.34
     */
    enum class TiledSceneRequestFlag : int SIP_ENUM_BASETYPE( IntFlag )
    {
      NoHierarchyFetch = 1 << 0,  //!< Do not allow hierarchy fetching when hierarchy is not currently available. Avoids network requests, but may result in an incomplete tile set. If set, then callers will need to manually perform hierarchy fetches as required.
    };
    Q_ENUM( TiledSceneRequestFlag )

    /**
     * Flags which control how tiled scene requests behave.
     *
     * \since QGIS 3.34
     */
    Q_DECLARE_FLAGS( TiledSceneRequestFlags, TiledSceneRequestFlag )
    Q_FLAG( TiledSceneRequestFlags )

    /**
     * Flags which control how tiled scene 2D renderers behave.
     *
     * \since QGIS 3.34
     */
    enum class TiledSceneRendererFlag : int SIP_ENUM_BASETYPE( IntFlag )
    {
      RequiresTextures = 1 << 0,  //!< Renderer requires textures
      ForceRasterRender = 1 << 1, //!< Layer should always be rendered as a raster image
      RendersTriangles = 1 << 2, //!< Renderer can render triangle primitives
      RendersLines = 1 << 3, //!< Renderer can render line primitives
    };
    Q_ENUM( TiledSceneRendererFlag )

    /**
     * Flags which control how tiled scene 2D renderers behave.
     *
     * \since QGIS 3.34
     */
    Q_DECLARE_FLAGS( TiledSceneRendererFlags, TiledSceneRendererFlag )
    Q_FLAG( TiledSceneRendererFlags )

    /**
     * Resampling algorithm to be used (equivalent to GDAL's enum GDALResampleAlg)
     * \note RA_Max, RA_Min, RA_Median, RA_Q1 and RA_Q3 are available on GDAL >= 2.0 builds only
     * \since QGIS 3.34
     */
    enum class GdalResampleAlgorithm : int
    {
      RA_NearestNeighbour = 0, //!< Nearest neighbour (select on one input pixel)
      RA_Bilinear = 1,       //!< Bilinear (2x2 kernel)
      RA_Cubic = 2,          //!< Cubic Convolution Approximation (4x4 kernel)
      RA_CubicSpline = 3,    //!< Cubic B-Spline Approximation (4x4 kernel)
      RA_Lanczos = 4,        //!< Lanczos windowed sinc interpolation (6x6 kernel)
      RA_Average = 5,        //!< Average (computes the average of all non-NODATA contributing pixels)
      RA_Mode = 6,            //!< Mode (selects the value which appears most often of all the sampled points)
      RA_Max = 8, //!< Maximum (selects the maximum of all non-NODATA contributing pixels)
      RA_Min = 9, //!< Minimum (selects the minimum of all non-NODATA contributing pixels)
      RA_Median = 10, //!< Median (selects the median of all non-NODATA contributing pixels)
      RA_Q1 = 11, //!< First quartile (selects the first quartile of all non-NODATA contributing pixels)
      RA_Q3 = 12, //!< Third quartile (selects the third quartile of all non-NODATA contributing pixels)
    };
    Q_ENUM( GdalResampleAlgorithm )


    /**
     * Statistics to be calculated during a zonal statistics operation.
     *
     * \since QGIS 3.36. Prior to 3.36 this was available as QgsZonalStatistics::Statistic.
     */
    enum class ZonalStatistic : int SIP_ENUM_BASETYPE( IntFlag )
    {
      Count = 1,  //!< Pixel count
      Sum = 2,  //!< Sum of pixel values
      Mean = 4,  //!< Mean of pixel values
      Median = 8, //!< Median of pixel values
      StDev = 16, //!< Standard deviation of pixel values
      Min = 32,  //!< Min of pixel values
      Max = 64,  //!< Max of pixel values
      Range = 128, //!< Range of pixel values (max - min)
      Minority = 256, //!< Minority of pixel values
      Majority = 512, //!< Majority of pixel values
      Variety = 1024, //!< Variety (count of distinct) pixel values
      Variance = 2048, //!< Variance of pixel values
      All = Count | Sum | Mean | Median | StDev | Max | Min | Range | Minority | Majority | Variety | Variance, //!< All statistics
      Default = Count | Sum | Mean, //!< Default statistics
    };
    Q_ENUM( ZonalStatistic )

    /**
     * Statistics to be calculated during a zonal statistics operation.
     *
     * \since QGIS 3.36. Prior to 3.36 this was available as QgsZonalStatistics::Statistic.
     */
    Q_DECLARE_FLAGS( ZonalStatistics, ZonalStatistic ) SIP_MONKEYPATCH_FLAGS_UNNEST( Qgis, ZonalStatistics )
    Q_FLAG( ZonalStatistics )

    /**
     * Zonal statistics result codes.
     *
     * \since QGIS 3.36. Prior to 3.36 this was available as QgsZonalStatistics::Result.
     */
    enum class ZonalStatisticResult : int
    {
      Success = 0, //!< Success
      LayerTypeWrong = 1, //!< Layer is not a polygon layer
      LayerInvalid, //!< Layer is invalid
      RasterInvalid, //!< Raster layer is invalid
      RasterBandInvalid, //!< The raster band does not exist on the raster layer
      FailedToCreateField = 8, //!< Output fields could not be created
      Canceled = 9 //!< Algorithm was canceled
    };
    Q_ENUM( ZonalStatisticResult )

    /**
     * Available aggregates to calculate. Not all aggregates are available for all field
     * types.
     *
     * \since QGIS 3.36. Prior to 3.36 this was available as QgsAggregateCalculator::Aggregate.
     */
    enum class Aggregate SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsAggregateCalculator, Aggregate ) : int
      {
      Count, //!< Count
      CountDistinct, //!< Number of distinct values
      CountMissing, //!< Number of missing (null) values
      Min, //!< Min of values
      Max, //!< Max of values
      Sum, //!< Sum of values
      Mean, //!< Mean of values (numeric fields only)
      Median, //!< Median of values (numeric fields only)
      StDev, //!< Standard deviation of values (numeric fields only)
      StDevSample,//!< Sample standard deviation of values (numeric fields only)
      Range, //!< Range of values (max - min) (numeric and datetime fields only)
      Minority, //!< Minority of values
      Majority, //!< Majority of values
      FirstQuartile, //!< First quartile (numeric fields only)
      ThirdQuartile, //!< Third quartile (numeric fields only)
      InterQuartileRange, //!< Inter quartile range (IQR) (numeric fields only)
      StringMinimumLength, //!< Minimum length of string (string fields only)
      StringMaximumLength, //!< Maximum length of string (string fields only)
      StringConcatenate, //!< Concatenate values with a joining string (string fields only). Specify the delimiter using setDelimiter().
      GeometryCollect, //!< Create a multipart geometry from aggregated geometries
      ArrayAggregate, //!< Create an array of values
      StringConcatenateUnique //!< Concatenate unique values with a joining string (string fields only). Specify the delimiter using setDelimiter().
    };
    Q_ENUM( Aggregate )

    /**
     * Available generic statistics.
     *
     * \since QGIS 3.36. Prior to 3.36 this was available as QgsStatisticalSummary::Statistic.
     */
    enum class Statistic SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsStatisticalSummary, Statistic ) : int SIP_ENUM_BASETYPE( IntFlag )
    {
      Count = 1 << 0,  //!< Count
      CountMissing = 1 << 15, //!< Number of missing (null) values
      Sum = 1 << 1,  //!< Sum of values
      Mean = 1 << 2,  //!< Mean of values
      Median = 1 << 3, //!< Median of values
      StDev = 1 << 4, //!< Standard deviation of values
      StDevSample = 1 << 5, //!< Sample standard deviation of values
      Min = 1 << 6,  //!< Min of values
      Max = 1 << 7,  //!< Max of values
      Range = 1 << 8, //!< Range of values (max - min)
      Minority = 1 << 9, //!< Minority of values
      Majority = 1 << 10, //!< Majority of values
      Variety = 1 << 11, //!< Variety (count of distinct) values
      FirstQuartile = 1 << 12, //!< First quartile
      ThirdQuartile = 1 << 13, //!< Third quartile
      InterQuartileRange = 1 << 14, //!< Inter quartile range (IQR)
      First = 1 << 16, //!< First value (since QGIS 3.6)
      Last = 1 << 17, //!< Last value (since QGIS 3.6)
      All = Count | CountMissing | Sum | Mean | Median | StDev | Max | Min | Range | Minority | Majority | Variety | FirstQuartile | ThirdQuartile | InterQuartileRange | First | Last //!< All statistics
    };
    Q_ENUM( Statistic )

    /**
     * Statistics to be calculated for generic values.
     *
     * \since QGIS 3.36. Prior to 3.36 this was available as QgsStatisticalSummary::Statistics.
     */
    Q_DECLARE_FLAGS( Statistics, Statistic ) SIP_MONKEYPATCH_FLAGS_UNNEST( QgsStatisticalSummary, Statistics )
    Q_FLAG( Statistics )

    /**
     * Available date/time statistics.
     *
     * \since QGIS 3.36. Prior to 3.36 this was available as QgsDateTimeStatisticalSummary::Statistic.
     */
    enum class DateTimeStatistic SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsDateTimeStatisticalSummary, Statistic ) : int SIP_ENUM_BASETYPE( IntFlag )
    {
      Count = 1,  //!< Count
      CountDistinct = 2,  //!< Number of distinct datetime values
      CountMissing = 4,  //!< Number of missing (null) values
      Min = 8, //!< Minimum (earliest) datetime value
      Max = 16, //!< Maximum (latest) datetime value
      Range = 32, //!< Interval between earliest and latest datetime value
      All = Count | CountDistinct | CountMissing | Min | Max | Range, //!< All statistics
    };
    Q_ENUM( DateTimeStatistic )

    /**
     * Statistics to be calculated for date/time values.
     *
     * \since QGIS 3.36. Prior to 3.36 this was available as QgsDateTimeStatisticalSummary::Statistic.
     */
    Q_DECLARE_FLAGS( DateTimeStatistics, DateTimeStatistic ) SIP_MONKEYPATCH_FLAGS_UNNEST( QgsDateTimeStatisticalSummary, Statistics )
    Q_FLAG( DateTimeStatistics )

    /**
     * Available string statistics.
     *
     * \since QGIS 3.36. Prior to 3.36 this was available as QgsStringStatisticalSummary::Statistic.
     */
    enum class StringStatistic SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsStringStatisticalSummary, Statistic ) : int SIP_ENUM_BASETYPE( IntFlag )
    {
      Count = 1,  //!< Count
      CountDistinct = 2,  //!< Number of distinct string values
      CountMissing = 4,  //!< Number of missing (null) values
      Min = 8, //!< Minimum string value
      Max = 16, //!< Maximum string value
      MinimumLength = 32, //!< Minimum length of string
      MaximumLength = 64, //!< Maximum length of string
      MeanLength = 128, //!< Mean length of strings
      Minority = 256, //!< Minority of strings
      Majority = 512, //!< Majority of strings
      All = Count | CountDistinct | CountMissing | Min | Max | MinimumLength | MaximumLength | MeanLength | Minority | Majority, //!< All statistics
    };
    Q_ENUM( StringStatistic )

    /**
     * Statistics to be calculated for string values.
     *
     * \since QGIS 3.36. Prior to 3.36 this was available as QgsStringStatisticalSummary::Statistic.
     */
    Q_DECLARE_FLAGS( StringStatistics, StringStatistic ) SIP_MONKEYPATCH_FLAGS_UNNEST( QgsStringStatisticalSummary, Statistics )
    Q_FLAG( StringStatistics )

    /**
     * Available raster band statistics.
     *
     * \since QGIS 3.36. Prior to 3.36 this was available as QgsRasterBandStats::Stats.
     */
    enum class RasterBandStatistic SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsRasterBandStats, Stats ) : int SIP_ENUM_BASETYPE( IntFlag )
    {
      NoStatistic = 0, //!< No statistic
      Min = 1, //!< Minimum
      Max = 1 << 1, //!< Maximum
      Range = 1 << 2, //!< Range
      Sum = 1 << 3, //!< Sum
      Mean = 1 << 4, //!< Mean
      StdDev = 1 << 5, //!< Standard deviation
      SumOfSquares = 1 << 6, //!< Sum of squares
      All = Min | Max | Range | Sum | Mean | StdDev | SumOfSquares //!< All available statistics
    };
    Q_ENUM( RasterBandStatistic )

    /**
     * Statistics to be calculated for raster bands.
     *
     * \since QGIS 3.36. Prior to 3.36 this was available as QgsRasterBandStats::Stats.
     */
    Q_DECLARE_FLAGS( RasterBandStatistics, RasterBandStatistic ) SIP_MONKEYPATCH_FLAGS_UNNEST( Qgis, RasterBandStatistics )
    Q_FLAG( RasterBandStatistics )

    /**
     * OGC SensorThings API entity types.
     *
     * \since QGIS 3.36
     */
    enum class SensorThingsEntity : int
    {
      Invalid, //!< An invalid/unknown entity
      Thing, //!< A Thing is an object of the physical world (physical things) or the information world (virtual things) that is capable of being identified and integrated into communication networks
      Location, //!< A Location entity locates the Thing or the Things it associated with. A Thing’s Location entity is defined as the last known location of the Thing
      HistoricalLocation, //!< A Thing’s HistoricalLocation entity set provides the times of the current (i.e., last known) and previous locations of the Thing
      Datastream, //!< A Datastream groups a collection of Observations measuring the same ObservedProperty and produced by the same Sensor
      Sensor, //!< A Sensor is an instrument that observes a property or phenomenon with the goal of producing an estimate of the value of the property
      ObservedProperty, //!< An ObservedProperty specifies the phenomenon of an Observation
      Observation, //!< An Observation is the act of measuring or otherwise determining the value of a property
      FeatureOfInterest, //!< In the context of the Internet of Things, many Observations’ FeatureOfInterest can be the Location of the Thing. For example, the FeatureOfInterest of a wifi-connect thermostat can be the Location of the thermostat (i.e., the living room where the thermostat is located in). In the case of remote sensing, the FeatureOfInterest can be the geographical area or volume that is being sensed
      MultiDatastream, //!< A MultiDatastream groups a collection of Observations and the Observations in a MultiDatastream have a complex result type. Implemented in the SensorThings version 1.1 "MultiDatastream extension". (Since QGIS 3.38)
    };
    Q_ENUM( SensorThingsEntity )

    /**
     * Identify search radius in mm
     */
    static const double DEFAULT_SEARCH_RADIUS_MM;

    //! Default threshold between map coordinates and device coordinates for map2pixel simplification
    static const float DEFAULT_MAPTOPIXEL_THRESHOLD;

    /**
     * Default highlight color.  The transparency is expected to only be applied to polygon
     * fill. Lines and outlines are rendered opaque.
     *
     */
    static const QColor DEFAULT_HIGHLIGHT_COLOR;

    /**
     * Default highlight buffer in mm.
     */
    static const double DEFAULT_HIGHLIGHT_BUFFER_MM;

    /**
     * Default highlight line/stroke minimum width in mm.
     */
    static const double DEFAULT_HIGHLIGHT_MIN_WIDTH_MM;

    /**
     * Fudge factor used to compare two scales. The code is often going from scale to scale
     *  denominator. So it looses precision and, when a limit is inclusive, can lead to errors.
     *  To avoid that, use this factor instead of using <= or >=.
     */
    static const double SCALE_PRECISION;

    /**
     * Default Z coordinate value.
     * This value have to be assigned to the Z coordinate for the vertex.
     */
    static const double DEFAULT_Z_COORDINATE;

    /**
     * Default M coordinate value.
     * This value have to be assigned to the M coordinate for the vertex.
     * \since QGIS 3.20
     */
    static const double DEFAULT_M_COORDINATE;

    /**
     * UI scaling factor. This should be applied to all widget sizes obtained from font metrics,
     * to account for differences in the default font sizes across different platforms.
     */
    static const double UI_SCALE_FACTOR;

    /**
     * Default snapping distance tolerance.
     */
    static const double DEFAULT_SNAP_TOLERANCE;

    /**
     * Default snapping distance units.
     */
    static const Qgis::MapToolUnit DEFAULT_SNAP_UNITS;

    /**
     * A string with default project scales.
     *
     * \since QGIS 3.12
     */
    static QString defaultProjectScales();

    /**
     * GEOS version number linked
     *
     * \since QGIS 3.20
     */
    static int geosVersionInt();

    /**
     * GEOS Major version number linked
     *
     * \since QGIS 3.20
     */
    static int geosVersionMajor();

    /**
     * GEOS Minor version number linked
     *
     * \since QGIS 3.20
     */
    static int geosVersionMinor();

    /**
     * GEOS Patch version number linked
     *
     * \since QGIS 3.20
     */
    static int geosVersionPatch();

    /**
     * GEOS string version linked
     *
     * \since QGIS 3.20
     */
    static QString geosVersion();
};

QHASH_FOR_CLASS_ENUM( Qgis::CaptureTechnique )
QHASH_FOR_CLASS_ENUM( Qgis::RasterAttributeTableFieldUsage )

Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::AnnotationItemFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::AnnotationItemGuiFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::BabelCommandFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::BabelFormatCapabilities )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::BrowserItemCapabilities )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::CoordinateTransformationFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::DatabaseProviderConnectionCapabilities2 )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::DataProviderFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::FileOperationFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::GeometryValidityFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::GpsInformationComponents )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::HistoryProviderBackends )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::LabelingFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::LabelLinePlacementFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::LabelPolygonPlacementFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::LayerTreeFilterFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::LegendJsonRenderFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::MapLayerActionFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::MapLayerActionTargets )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::MapLayerProperties )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::MapLayerRendererFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::MapSettingsFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::MarkerLinePlacements )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::PlotToolFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::ProfileGeneratorFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::ProjectCapabilities )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::ProjectReadFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::RasterRendererFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::RasterRendererCapabilities )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::RasterTemporalCapabilityFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::RelationshipCapabilities )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::RenderContextFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::ScriptLanguageCapabilities )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::SelectionFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::SettingsTreeNodeOptions )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::SnappingTypes )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::SqlLayerDefinitionCapabilities )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::SublayerFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::SublayerQueryFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::SymbolFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::SymbolLayerFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::SymbolLayerUserFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::SymbolPreviewFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::SymbolRenderHints )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::TextRendererFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::TiledSceneProviderCapabilities )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::TiledSceneRendererFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::FieldConfigurationFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::LayerFilters )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::TiledSceneRequestFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::VectorDataProviderAttributeEditCapabilities )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::VectorFileWriterCapabilities )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::VectorLayerTypeFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::VectorTileProviderCapabilities )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::VectorTileProviderFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::FeatureRequestFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::ProcessingFeatureSourceDefinitionFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::ZonalStatistics )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::Statistics )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::DateTimeStatistics )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::StringStatistics )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::RasterBandStatistics )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::ProcessingProviderFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::ProcessingAlgorithmFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::ProcessingFeatureSourceFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::ProcessingParameterTypeFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::ProcessingParameterFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::DataItemProviderCapabilities )


// hack to workaround warnings when casting void pointers
// retrieved from QLibrary::resolve to function pointers.
// It's assumed that this works on all systems supporting
// QLibrary
#define cast_to_fptr(f) f


/**
 * \ingroup core
 * \brief RAII signal blocking class. Used for temporarily blocking signals from a QObject
 * for the lifetime of QgsSignalBlocker object.
 * \see whileBlocking()
 * \note not available in Python bindings
 */
// based on Boojum's code from http://stackoverflow.com/questions/3556687/prevent-firing-signals-in-qt
template<class Object> class QgsSignalBlocker SIP_SKIP SIP_SKIP // clazy:exclude=rule-of-three
{
  public:

    /**
     * Constructor for QgsSignalBlocker
     * \param object QObject to block signals from
     */
    explicit QgsSignalBlocker( Object *object )
      : mObject( object )
      , mPreviousState( object->blockSignals( true ) )
    {}

    ~QgsSignalBlocker()
    {
      mObject->blockSignals( mPreviousState );
    }

    //! Returns pointer to blocked QObject
    Object *operator->() { return mObject; }

  private:

    Object *mObject = nullptr;
    bool mPreviousState;

};

/**
 * Temporarily blocks signals from a QObject while calling a single method from the object.
 *
 * Usage:
 *   whileBlocking( checkBox )->setChecked( true );
 *   whileBlocking( spinBox )->setValue( 50 );
 *
 * No signals will be emitted when calling these methods.
 *
 * \see QgsSignalBlocker
 * \note not available in Python bindings
 */
// based on Boojum's code from http://stackoverflow.com/questions/3556687/prevent-firing-signals-in-qt
template<class Object> inline QgsSignalBlocker<Object> whileBlocking( Object *object ) SIP_SKIP SIP_SKIP
{
  return QgsSignalBlocker<Object>( object );
}

//! Hash for QVariant
CORE_EXPORT uint qHash( const QVariant &variant );

/**
 * Returns a string representation of a double
 * \param a double value
 * \param precision number of decimal places to retain
 */
inline QString qgsDoubleToString( double a, int precision = 17 )
{
  QString str;
  if ( precision )
  {
    if ( precision < 0 )
    {
      const double roundFactor = std::pow( 10, -precision );
      str = QString::number( static_cast< long long >( std::round( a / roundFactor ) * roundFactor ) );
    }
    else
    {
      str = QString::number( a, 'f', precision );
      if ( str.contains( QLatin1Char( '.' ) ) )
      {
        // remove ending 0s
        int idx = str.length() - 1;
        while ( str.at( idx ) == '0' && idx > 1 )
        {
          idx--;
        }
        if ( idx < str.length() - 1 )
          str.truncate( str.at( idx ) == '.' ? idx : idx + 1 );
      }
    }
  }
  else
  {
    str = QString::number( a, 'f', precision );
  }
  // avoid printing -0
  // see https://bugreports.qt.io/browse/QTBUG-71439
  if ( str == QLatin1String( "-0" ) )
  {
    return QLatin1String( "0" );
  }
  return str;
}

/**
 * Compare two doubles, treating nan values as equal
 * \param a first double
 * \param b second double
 * \since QGIS 3.20
 */
inline bool qgsNanCompatibleEquals( double a, double b )
{
  const bool aIsNan = std::isnan( a );
  const bool bIsNan = std::isnan( b );
  if ( aIsNan || bIsNan )
    return aIsNan && bIsNan;

  return a == b;
}

#ifndef SIP_RUN

/**
 * Compare two numbers of type T (but allow some difference)
 * \param a first number
 * \param b second number
 * \param epsilon maximum difference allowable between numbers
 * \since QGIS 3.36
 */
template<typename T>
inline bool qgsNumberNear( T a, T b, T epsilon = std::numeric_limits<T>::epsilon() * 4 )
{
  const bool aIsNan = std::isnan( a );
  const bool bIsNan = std::isnan( b );
  if ( aIsNan || bIsNan )
    return aIsNan && bIsNan;

  const T diff = a - b;
  return diff >= -epsilon && diff <= epsilon;
}
#endif

/**
 * Compare two doubles (but allow some difference)
 * \param a first double
 * \param b second double
 * \param epsilon maximum difference allowable between doubles
 */
inline bool qgsDoubleNear( double a, double b, double epsilon = 4 * std::numeric_limits<double>::epsilon() )
{
  return qgsNumberNear<double>( a, b, epsilon );
}

/**
 * Compare two floats (but allow some difference)
 * \param a first float
 * \param b second float
 * \param epsilon maximum difference allowable between floats
 */
inline bool qgsFloatNear( float a, float b, float epsilon = 4 * FLT_EPSILON )
{
  return qgsNumberNear<float>( a, b, epsilon );
}

//! Compare two doubles using specified number of significant digits
inline bool qgsDoubleNearSig( double a, double b, int significantDigits = 10 )
{
  const bool aIsNan = std::isnan( a );
  const bool bIsNan = std::isnan( b );
  if ( aIsNan || bIsNan )
    return aIsNan && bIsNan;

  // The most simple would be to print numbers as %.xe and compare as strings
  // but that is probably too costly
  // Then the fastest would be to set some bits directly, but little/big endian
  // has to be considered (maybe TODO)
  // Is there a better way?
  int aexp, bexp;
  const double ar = std::frexp( a, &aexp );
  const double br = std::frexp( b, &bexp );

  return aexp == bexp &&
         std::round( ar * std::pow( 10.0, significantDigits ) ) == std::round( br * std::pow( 10.0, significantDigits ) );
}

/**
 * Returns a double \a number, rounded (as close as possible) to the specified number of \a places.
 *
 */
inline double qgsRound( double number, int places )
{
  const double m = ( number < 0.0 ) ? -1.0 : 1.0;
  const double scaleFactor = std::pow( 10.0, places );
  return ( std::round( number * m * scaleFactor ) / scaleFactor ) * m;
}

#ifndef SIP_RUN

/**
 * Joins all the \a map keys into a single string with each element separated by the given
 * \a separator.
 * This method avoid calling keys() before joining because it creates an unneeded temporary list
 * see clazy container-anti-pattern
 */
template<class Key, class Value>
QString qgsMapJoinKeys( const QMap<Key, Value> &map, const QString &separator )
{
  QString result;
  for ( auto it = map.constBegin(); it != map.constEnd(); it++ )
    result += QString( "%1%2" ).arg( it.key() ).arg( separator );

  result.chop( separator.size() );
  return result;
}

/**
 * Joins all the \a map values into a single string with each element separated by the given
 * \a separator.
 * This method avoid calling values() before joining because it creates an unneeded temporary list
 * see clazy container-anti-pattern
 */
template<class Key, class Value>
QString qgsMapJoinValues( const QMap<Key, Value> &map, const QString &separator )
{
  QString result;
  for ( auto it = map.constBegin(); it != map.constEnd(); it++ )
    result += QString( "%1%2" ).arg( it.value() ).arg( separator );

  result.chop( separator.size() );
  return result;
}

/**
 * Joins all the \a set values into a single string with each element separated by the given
 * \a separator.
 * This method avoid calling values() before joining because it creates an unneeded temporary list
 * see clazy container-anti-pattern
 */
template<class T>
QString qgsSetJoin( const QSet<T> &set, const QString &separator )
{
  QString result;
  for ( auto it = set.constBegin(); it != set.constEnd(); it++ )
    result += QString( "%1%2" ).arg( *it ).arg( separator );

  result.chop( separator.size() );
  return result;
}

///@cond PRIVATE

/**
 * Contains "polyfills" for backporting c++ features from standards > c++11 and Qt global methods
 * added later than our minimum version.
 *
 * To be removed when minimum c++ or Qt build requirement includes the std implementation
 * for these features.
 *
 * \note not available in Python bindings.
 */
namespace qgis
{

  /**
   * Use qgis::down_cast<Derived*>(pointer_to_base) as equivalent of
   * static_cast<Derived*>(pointer_to_base) with safe checking in debug
   * mode.
   *
   * Only works if no virtual inheritance is involved.
   *
   * Ported from GDAL's cpl::down_cast method.
   *
   * \param f pointer to a base class
   * \return pointer to a derived class
   */
  template<typename To, typename From> inline To down_cast( From *f )
  {
    static_assert(
      ( std::is_base_of<From,
        typename std::remove_pointer<To>::type>::value ),
      "target type not derived from source type" );
    Q_ASSERT( f == nullptr || dynamic_cast<To>( f ) != nullptr );
    return static_cast<To>( f );
  }

  template<class T>
  QSet<T> listToSet( const QList<T> &list )
  {
    return QSet<T>( list.begin(), list.end() );
  }

  template<class T>
  QList<T> setToList( const QSet<T> &set )
  {
    return QList<T>( set.begin(), set.end() );
  }
}

///@endcond
#endif

/**
 * Returns a list all enum entries.
 * The enum must have been declared using Q_ENUM or Q_FLAG.
 */
template<class T> const QList<T> qgsEnumList() SIP_SKIP
{
  const QMetaEnum metaEnum = QMetaEnum::fromType<T>();
  Q_ASSERT( metaEnum.isValid() );
  QList<T> enumList;
  for ( int idx = 0; idx < metaEnum.keyCount(); ++idx )
  {
    enumList.append( static_cast<T>( metaEnum.value( idx ) ) );
  }
  return enumList;
}

/**
 * Returns a map of all enum entries.
 * The map has the enum values (int) as keys and the enum keys (QString) as values.
 * The enum must have been declared using Q_ENUM or Q_FLAG.
 */
template<class T> const QMap<T, QString> qgsEnumMap() SIP_SKIP
{
  const QMetaEnum metaEnum = QMetaEnum::fromType<T>();
  Q_ASSERT( metaEnum.isValid() );
  QMap<T, QString> enumMap;
  for ( int idx = 0; idx < metaEnum.keyCount(); ++idx )
  {
    enumMap.insert( static_cast<T>( metaEnum.value( idx ) ), QString( metaEnum.key( idx ) ) );
  }
  return enumMap;
}

/**
 * Returns the value for the given key of an enum.
 * If \a returnOk is given, it defines if the value could be converted to the key
 * \since QGIS 3.6
 */
template<class T> QString qgsEnumValueToKey( const T &value, bool *returnOk = nullptr ) SIP_SKIP
{
  const QMetaEnum metaEnum = QMetaEnum::fromType<T>();
  Q_ASSERT( metaEnum.isValid() );
  const char *key = metaEnum.valueToKey( static_cast<int>( value ) );
  if ( returnOk )
  {
    *returnOk = key ? true : false;
  }
  return QString::fromUtf8( key );
}

/**
 * Returns the value corresponding to the given \a key of an enum.
 * If the key is invalid, it will return the \a defaultValue.
 * If \a tryValueAsKey is TRUE, it will try to convert the string key to an enum value
 * If \a returnOk is given, it defines if the key could be converted to the value or if it had returned the default
 * \since QGIS 3.6
 */
template<class T> T qgsEnumKeyToValue( const QString &key, const T &defaultValue, bool tryValueAsKey = true,  bool *returnOk = nullptr ) SIP_SKIP
{
  const QMetaEnum metaEnum = QMetaEnum::fromType<T>();
  Q_ASSERT( metaEnum.isValid() );
  bool ok = false;
  T v = static_cast<T>( metaEnum.keyToValue( key.toUtf8().data(), &ok ) );
  if ( returnOk )
  {
    *returnOk = ok;
  }
  if ( ok )
  {
    return v;
  }
  else
  {
    // if conversion has failed, try with conversion from int value
    if ( tryValueAsKey )
    {
      bool canConvert = false;
      const int intValue = key.toInt( &canConvert );
      if ( canConvert && metaEnum.valueToKey( intValue ) )
      {
        if ( returnOk )
        {
          *returnOk = true;
        }
        return static_cast<T>( intValue );
      }
    }
  }
  return defaultValue;
}

/**
 * Returns the value for the given keys of a flag.
 * If \a returnOk is given, it defines if the value could be converted to the keys
 * \since QGIS 3.16
 */
template<class T> QString qgsFlagValueToKeys( const T &value, bool *returnOk = nullptr ) SIP_SKIP
{
  const QMetaEnum metaEnum = QMetaEnum::fromType<T>();
  Q_ASSERT( metaEnum.isValid() );
  int intValue = static_cast<int>( value );
  const QByteArray ba = metaEnum.valueToKeys( intValue );
  // check that the int value does correspond to a flag
  // see https://stackoverflow.com/a/68495949/1548052
  const int intValueCheck = metaEnum.keysToValue( ba );
  bool ok = intValue == intValueCheck;
  if ( returnOk )
    *returnOk = ok;
  return ok ? QString::fromUtf8( ba ) : QString();
}

/**
 * Returns the value corresponding to the given \a keys of a flag.
 * If the keys are invalid, it will return the \a defaultValue.
 * If \a tryValueAsKey is TRUE, it will try to convert the string key to an enum value.
 * If \a returnOk is given, it defines if the key could be converted to the value or if it had returned the default
 * \since QGIS 3.16
 */
template<class T> T qgsFlagKeysToValue( const QString &keys, const T &defaultValue, bool tryValueAsKey = true,  bool *returnOk = nullptr ) SIP_SKIP
{
  const QMetaEnum metaEnum = QMetaEnum::fromType<T>();
  Q_ASSERT( metaEnum.isValid() );
  bool ok = false;
  T v = static_cast<T>( metaEnum.keysToValue( keys.toUtf8().constData(), &ok ) );
  if ( returnOk )
  {
    *returnOk = ok;
  }
  if ( ok )
  {
    return v;
  }
  else
  {
    // if conversion has failed, try with conversion from int value
    if ( tryValueAsKey )
    {
      bool canConvert = false;
      const int intValue = keys.toInt( &canConvert );
      if ( canConvert )
      {
        const QByteArray keys = metaEnum.valueToKeys( intValue );
        const int intValueCheck = metaEnum.keysToValue( keys );
        if ( intValue == intValueCheck )
        {
          if ( returnOk )
          {
            *returnOk = true;
          }
          return T( intValue );
        }
      }
    }
  }
  return defaultValue;
}


/**
 * Converts a string to a double in a permissive way, e.g., allowing for incorrect
 * numbers of digits between thousand separators
 * \param string string to convert
 * \param ok will be set to TRUE if conversion was successful
 * \returns string converted to double if possible
 * \see permissiveToInt
 */
CORE_EXPORT double qgsPermissiveToDouble( QString string, bool &ok );

/**
 * Converts a string to an integer in a permissive way, e.g., allowing for incorrect
 * numbers of digits between thousand separators
 * \param string string to convert
 * \param ok will be set to TRUE if conversion was successful
 * \returns string converted to int if possible
 * \see permissiveToDouble
 */
CORE_EXPORT int qgsPermissiveToInt( QString string, bool &ok );

/**
 * Converts a string to an qlonglong in a permissive way, e.g., allowing for incorrect
 * numbers of digits between thousand separators
 * \param string string to convert
 * \param ok will be set to TRUE if conversion was successful
 * \returns string converted to int if possible
 * \see permissiveToInt
 * \since QGIS 3.4
 */
CORE_EXPORT qlonglong qgsPermissiveToLongLong( QString string, bool &ok );

/**
 * Compares two QVariant values and returns whether the first is less than the second.
 * Useful for sorting lists of variants, correctly handling sorting of the various
 * QVariant data types (such as strings, numeric values, dates and times)
 *
 * Invalid < NULL < Values
 *
 * \see qgsVariantGreaterThan()
 */
CORE_EXPORT bool qgsVariantLessThan( const QVariant &lhs, const QVariant &rhs );

/**
 * Compares two QVariant values and returns whether they are equal, two NULL values are
 * always treated as equal and 0 is not treated as equal with NULL
 *
 * \param lhs first value
 * \param rhs second value
 * \return TRUE if values are equal
 */
CORE_EXPORT bool qgsVariantEqual( const QVariant &lhs, const QVariant &rhs );

/**
 * Compares two QVariant values and returns whether the first is greater than the second.
 * Useful for sorting lists of variants, correctly handling sorting of the various
 * QVariant data types (such as strings, numeric values, dates and times)
 * \see qgsVariantLessThan()
 */
CORE_EXPORT bool qgsVariantGreaterThan( const QVariant &lhs, const QVariant &rhs );

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)

/**
 * Compares two QVariant values and returns whether the first is greater than the second.
 * Useful for sorting lists of variants, correctly handling sorting of the various
 * QVariant data types (such as strings, numeric values, dates and times)
 * \see qgsVariantLessThan()
 */
inline bool operator> ( const QVariant &v1, const QVariant &v2 )
{
  return qgsVariantGreaterThan( v1, v2 );
}

/**
 * Compares two QVariant values and returns whether the first is less than the second.
 * Useful for sorting lists of variants, correctly handling sorting of the various
 * QVariant data types (such as strings, numeric values, dates and times)
 *
 * Invalid < NULL < Values
 *
 * \see qgsVariantGreaterThan()
 */
inline bool operator< ( const QVariant &v1, const QVariant &v2 )
{
  return qgsVariantLessThan( v1, v2 );
}
#endif


#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)

/**
 * Compares two QVariantList values and returns whether the first is less than the second.
 */
template<> CORE_EXPORT bool qMapLessThanKey<QVariantList>( const QVariantList &key1, const QVariantList &key2 ) SIP_SKIP;
#endif

CORE_EXPORT QString qgsVsiPrefix( const QString &path );

/**
 * Allocates size bytes and returns a pointer to the allocated  memory.
 * Works like C malloc() but prints debug message by QgsLogger if allocation fails.
 * \param size size in bytes
 */
void CORE_EXPORT *qgsMalloc( size_t size ) SIP_SKIP;

/**
 * Frees the memory space  pointed  to  by  ptr. Works like C free().
 * \param ptr pointer to memory space
 */
void CORE_EXPORT qgsFree( void *ptr ) SIP_SKIP;

#ifndef SIP_RUN

#ifdef _MSC_VER
#define CONSTLATIN1STRING inline const QLatin1String
#else
#define CONSTLATIN1STRING constexpr QLatin1String
#endif

///@cond PRIVATE
class ScopedIntIncrementor
{
  public:

    ScopedIntIncrementor( int *variable )
      : mVariable( variable )
    {
      ( *mVariable )++;
    }

    ScopedIntIncrementor( const ScopedIntIncrementor &other ) = delete;
    ScopedIntIncrementor &operator=( const ScopedIntIncrementor &other ) = delete;

    void release()
    {
      if ( mVariable )
        ( *mVariable )--;

      mVariable = nullptr;
    }

    ~ScopedIntIncrementor()
    {
      release();
    }

  private:
    int *mVariable = nullptr;
};
///@endcond

/**
* Wkt string that represents a geographic coord sys
* \since QGIS GEOWkt
*/
CONSTLATIN1STRING geoWkt()
{
  return QLatin1String(
           R"""(GEOGCRS["WGS 84",DATUM["World Geodetic System 1984",ELLIPSOID["WGS 84",6378137,298.257223563,LENGTHUNIT["metre",1]]],PRIMEM["Greenwich",0,ANGLEUNIT["degree",0.0174532925199433]],CS[ellipsoidal,2],AXIS["geodetic latitude (Lat)",north,ORDER[1],ANGLEUNIT["degree",0.0174532925199433]],AXIS["geodetic longitude (Lon)",east,ORDER[2],ANGLEUNIT["degree",0.0174532925199433]],USAGE[SCOPE["unknown"],AREA["World"],BBOX[-90,-180,90,180]],ID["EPSG",4326]] )"""
         );
}

//! PROJ4 string that represents a geographic coord sys
CONSTLATIN1STRING geoProj4()
{
  return QLatin1String( "+proj=longlat +datum=WGS84 +no_defs" );
}

//! Geographic coord sys from EPSG authority
CONSTLATIN1STRING geoEpsgCrsAuthId()
{
  return QLatin1String( "EPSG:4326" );
}

//! Constant that holds the string representation for "No ellips/No CRS"
CONSTLATIN1STRING geoNone()
{
  return QLatin1String( "NONE" );
}

///@cond PRIVATE

//! Delay between the scheduling of 2 preview jobs
const int PREVIEW_JOB_DELAY_MS = 250;

//! Maximum rendering time for a layer of a preview job
const int MAXIMUM_LAYER_PREVIEW_TIME_MS = 250;

///@endcond

#endif

//! Magic number for a geographic coord sys in POSTGIS SRID
const long GEOSRID = 4326;

//! Magic number for a geographic coord sys in QGIS srs.db tbl_srs.srs_id
const long GEOCRS_ID = 3452;

//! Magic number for a geographic coord sys in EpsgCrsId ID format
const long GEO_EPSG_CRS_ID = 4326;

/**
 * Magick number that determines whether a projection crsid is a system (srs.db)
 * or user (~/.qgis.qgis.db) defined projection.
*/
const int USER_CRS_START_ID = 100000;

//
// Constants for point symbols
//

//! Magic number that determines the default point size for point symbols
const double DEFAULT_POINT_SIZE = 2.0;
const double DEFAULT_LINE_WIDTH = 0.26;

//! Default snapping tolerance for segments
const double DEFAULT_SEGMENT_EPSILON = 1e-8;

typedef QMap<QString, QString> QgsStringMap SIP_SKIP;

/**
 * Qgssize is used instead of size_t, because size_t is stdlib type, unknown
 *  by SIP, and it would be hard to define size_t correctly in SIP.
 *  Currently used "unsigned long long" was introduced in C++11 (2011)
 *  but it was supported already before C++11 on common platforms.
 *  "unsigned long long int" gives syntax error in SIP.
 *  KEEP IN SYNC WITH qgssize defined in SIP!
*/
typedef unsigned long long qgssize;

#ifndef SIP_RUN
#if (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)) || defined(__clang__)

#define Q_NOWARN_DEPRECATED_PUSH \
  _Pragma("GCC diagnostic push") \
  _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"");
#define Q_NOWARN_DEPRECATED_POP \
  _Pragma("GCC diagnostic pop");
#define Q_NOWARN_UNREACHABLE_PUSH
#define Q_NOWARN_UNREACHABLE_POP

#elif defined(_MSC_VER)

#define Q_NOWARN_DEPRECATED_PUSH \
  __pragma(warning(push)) \
  __pragma(warning(disable:4996))
#define Q_NOWARN_DEPRECATED_POP \
  __pragma(warning(pop))
#define Q_NOWARN_UNREACHABLE_PUSH \
  __pragma(warning(push)) \
  __pragma(warning(disable:4702))
#define Q_NOWARN_UNREACHABLE_POP \
  __pragma(warning(pop))

#else

#define Q_NOWARN_DEPRECATED_PUSH
#define Q_NOWARN_DEPRECATED_POP
#define Q_NOWARN_UNREACHABLE_PUSH
#define Q_NOWARN_UNREACHABLE_POP

#endif
#endif

#ifndef QGISEXTERN
#ifdef Q_OS_WIN
#  define QGISEXTERN extern "C" __declspec( dllexport )
#else
#  if defined(__GNUC__) || defined(__clang__)
#    define QGISEXTERN extern "C" __attribute__ ((visibility ("default")))
#  else
#    define QGISEXTERN extern "C"
#  endif
#endif
#endif
#endif

// see https://infektor.net/posts/2017-01-19-using-cpp17-attributes-today.html#using-the-nodiscard-attribute
#if __cplusplus >= 201703L
#define NODISCARD [[nodiscard]]
#elif defined(__clang__)
#define NODISCARD [[nodiscard]]
#elif defined(_MSC_VER)
#define NODISCARD // no support
#elif defined(__has_cpp_attribute)
#if __has_cpp_attribute(nodiscard)
#define NODISCARD [[nodiscard]]
#elif __has_cpp_attribute(gnu::warn_unused_result)
#define NODISCARD [[gnu::warn_unused_result]]
#else
#define NODISCARD Q_REQUIRED_RESULT
#endif
#else
#define NODISCARD Q_REQUIRED_RESULT
#endif

#if __cplusplus >= 201703L
#define MAYBE_UNUSED [[maybe_unused]]
#elif defined(__clang__)
#define MAYBE_UNUSED [[maybe_unused]]
#elif defined(_MSC_VER)
#define MAYBE_UNUSED // no support
#elif defined(__has_cpp_attribute)
#if __has_cpp_attribute(gnu::unused)
#define MAYBE_UNUSED [[gnu::unused]]
#else
#define MAYBE_UNUSED
#endif
#else
#define MAYBE_UNUSED
#endif

#ifndef FINAL
#define FINAL final
#endif

#ifndef SIP_RUN
#ifdef _MSC_VER
#define BUILTIN_UNREACHABLE \
  __assume(false);
#elif defined(__GNUC__) && !defined(__clang__)
// Workaround a GCC bug where a -Wreturn-type warning is emitted in constructs
// like:
// switch( mVariableThatCanOnlyBeXorY )
// {
//    case X:
//        return "foo";
//    case Y:
//        return "foo";
// }
// See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=87951
#define BUILTIN_UNREACHABLE \
  __builtin_unreachable();
#else
#define BUILTIN_UNREACHABLE
#endif
#endif // SIP_RUN

#ifdef SIP_RUN

/**
 * Wkt string that represents a geographic coord sys
 * \since QGIS GEOWkt
 */
QString CORE_EXPORT geoWkt();

//! PROJ4 string that represents a geographic coord sys
QString CORE_EXPORT geoProj4();

//! Geographic coord sys from EPSG authority
QString CORE_EXPORT geoEpsgCrsAuthId();

//! Constant that holds the string representation for "No ellips/No CRS"
QString CORE_EXPORT geoNone();

#endif
