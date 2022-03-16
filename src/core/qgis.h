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

#include "qgstolerance.h"
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

/**
 * \ingroup core
 * \brief Types of layers that can be added to a map
 * \since QGIS 3.8
 */
enum class QgsMapLayerType SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsMapLayer, LayerType ) : int
  {
  VectorLayer, //!< Vector layer
  RasterLayer, //!< Raster layer
  PluginLayer, //!< Plugin based layer
  MeshLayer,      //!< Mesh layer. Added in QGIS 3.2
  VectorTileLayer, //!< Vector tile layer. Added in QGIS 3.14
  AnnotationLayer, //!< Contains freeform, georeferenced annotations. Added in QGIS 3.16
  PointCloudLayer, //!< Point cloud layer. Added in QGIS 3.18
  GroupLayer, //!< Composite group layer. Added in QGIS 3.24
};

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
     * Raster data types.
     * This is modified and extended copy of GDALDataType.
     */
    enum class DataType SIP_MONKEYPATCH_SCOPEENUM_UNNEST( Qgis, DataType ) : int
      {
      UnknownDataType = 0, //!< Unknown or unspecified type
      Byte = 1, //!< Eight bit unsigned integer (quint8)
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
    enum class VectorLayerTypeFlag : int
    {
      SqlQuery = 1 << 0 //!< SQL query layer
    };
    Q_ENUM( VectorLayerTypeFlag )
    //! Vector layer type flags
    Q_DECLARE_FLAGS( VectorLayerTypeFlags, VectorLayerTypeFlag )

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
      Variant, //!< Generic variant
      String, //!< String
      StringList, //!< List of strings
      Bool, //!< Boolean
      Integer, //!< Integer
      Double, //!< Double precision number
      EnumFlag, //!< Enum or Flag
      Color //!< Color
    };
    Q_ENUM( SettingsType )

    /**
     * Settings options
     * \since QGIS 3.26
     */
    enum class SettingsOption : int
    {
      SaveFormerValue = 1 << 1, //<! Save the former value of the settings
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
    enum class SnappingType SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsSnappingConfig, SnappingTypes ) : int
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
    enum class SymbolRenderHint SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsSymbol, RenderHint ) : int
      {
      DynamicRotation = 2, //!< Rotation of symbol may be changed during rendering and symbol should not be cached
    };
    Q_ENUM( SymbolRenderHint )
    //! Symbol render hints
    Q_DECLARE_FLAGS( SymbolRenderHints, SymbolRenderHint ) SIP_MONKEYPATCH_FLAGS_UNNEST( QgsSymbol, RenderHints )


    /**
     * \brief Flags controlling behavior of symbols
     *
     * \since QGIS 3.20
     */
    enum class SymbolFlag : int
    {
      RendererShouldUseSymbolLevels = 1 << 0, //!< If present, indicates that a QgsFeatureRenderer using the symbol should use symbol levels for best results
    };
    Q_ENUM( SymbolFlag )
    //! Symbol flags
    Q_DECLARE_FLAGS( SymbolFlags, SymbolFlag )

    /**
     * Flags for controlling how symbol preview images are generated.
     *
     * \since QGIS 3.20
     */
    enum class SymbolPreviewFlag SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsSymbol, PreviewFlag ) : int
      {
      FlagIncludeCrosshairsForMarkerSymbols = 1 << 0, //!< Include a crosshairs reference image in the background of marker symbol previews
    };
    Q_ENUM( SymbolPreviewFlag )
    //! Symbol preview flags
    Q_DECLARE_FLAGS( SymbolPreviewFlags, SymbolPreviewFlag ) SIP_MONKEYPATCH_FLAGS_UNNEST( QgsSymbol, SymbolPreviewFlags )

    /**
     * \brief Flags controlling behavior of symbol layers
     *
     * \since QGIS 3.22
     */
    enum class SymbolLayerFlag : int
    {
      DisableFeatureClipping = 1 << 0, //!< If present, indicates that features should never be clipped to the map extent during rendering
    };
    Q_ENUM( SymbolLayerFlag )
    //! Symbol layer flags
    Q_DECLARE_FLAGS( SymbolLayerFlags, SymbolLayerFlag )

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
    enum class BrowserItemCapability SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsDataItem, Capability ) : int
      {
      NoCapabilities = 0, //!< Item has no capabilities
      SetCrs = 1 << 0, //!< Can set CRS on layer or group of layers. \deprecated since QGIS 3.6 -- no longer used by QGIS and will be removed in QGIS 4.0
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
      PointCloud //!< Point cloud layer
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
     * \since 3.22
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
     * SqlLayerDefinitionCapability enum lists the arguments supported by the provider when creating SQL query layers.
     * \since QGIS 3.22
     */
    enum class SqlLayerDefinitionCapability : int
    {
      SubsetStringFilter = 1 << 1,  //!< SQL layer definition supports subset string filter
      GeometryColumn = 1 << 2,      //!< SQL layer definition supports geometry column
      PrimaryKeys = 1 << 3,         //!< SQL layer definition supports primary keys
      UnstableFeatureIds = 1 << 4   //!< SQL layer definition supports disabling select at id
    };
    Q_ENUM( SqlLayerDefinitionCapability )
    //! SQL layer definition capabilities
    Q_DECLARE_FLAGS( SqlLayerDefinitionCapabilities, SqlLayerDefinitionCapability )

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
     * Flags which control how data providers will scan for sublayers in a dataset.
     *
     * \since QGIS 3.22
     */
    enum class SublayerQueryFlag : int
    {
      FastScan = 1 << 0, //!< Indicates that the provider must scan for sublayers using the fastest possible approach -- e.g. by first checking that a uri has an extension which is known to be readable by the provider
      ResolveGeometryType = 1 << 1, //!< Attempt to resolve the geometry type for vector sublayers
      CountFeatures = 1 << 2, //!< Count features in vector sublayers
      IncludeSystemTables = 1 << 3, //!< Include system or internal tables (these are not included by default)
    };
    //! Sublayer query flags
    Q_DECLARE_FLAGS( SublayerQueryFlags, SublayerQueryFlag )
    Q_ENUM( SublayerQueryFlag )

    /**
     * Flags which reflect the properties of sublayers in a dataset.
     *
     * \since QGIS 3.22
     */
    enum class SublayerFlag : int
    {
      SystemTable = 1 << 0, //!< Sublayer is a system or internal table, which should be hidden by default
    };
    //! Sublayer flags
    Q_DECLARE_FLAGS( SublayerFlags, SublayerFlag )
    Q_ENUM( SublayerFlag )

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
     * Babel GPS format capabilities.
     *
     * \since QGIS 3.22
     */
    enum class BabelFormatCapability : int
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

    /**
     * Babel command flags, which control how commands and arguments
     * are generated for executing GPSBabel processes.
     *
     * \since QGIS 3.22
     */
    enum class BabelCommandFlag : int
    {
      QuoteFilePaths = 1 << 0, //!< File paths should be enclosed in quotations and escaped
    };
    //! Babel command flags
    Q_DECLARE_FLAGS( BabelCommandFlags, BabelCommandFlag )
    Q_ENUM( BabelCommandFlag )

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
      /* Add part issues */
      AddPartSelectedGeometryNotFound, //!< The selected geometry cannot be found
      AddPartNotMultiGeometry, //!< The source geometry is not multi
      /* Add ring issues*/
      AddRingNotClosed, //!< The input ring is not closed
      AddRingNotValid, //!< The input ring is not valid
      AddRingCrossesExistingRings, //!< The input ring crosses existing rings (it is not disjoint)
      AddRingNotInExistingFeature, //!< The input ring doesn't have any existing ring to fit into
      /* Split features */
      SplitCannotSplitPoint, //!< Cannot split points
    };
    Q_ENUM( GeometryOperationResult )

    /**
     * Geometry validity check flags.
     *
     * \since QGIS 3.22
     */
    enum class GeometryValidityFlag SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsGeometry, ValidityFlag ) : int
      {
      AllowSelfTouchingHoles SIP_MONKEYPATCH_COMPAT_NAME( FlagAllowSelfTouchingHoles ) = 1 << 0, //!< Indicates that self-touching holes are permitted. OGC validity states that self-touching holes are NOT permitted, whilst other vendor validity checks (e.g. ESRI) permit self-touching holes.
    };
    //! Geometry validity flags
    Q_DECLARE_FLAGS( GeometryValidityFlags, GeometryValidityFlag ) SIP_MONKEYPATCH_FLAGS_UNNEST( QgsGeometry, ValidityFlags )
    Q_ENUM( GeometryValidityFlag )

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
    enum class FileOperationFlag : int
    {
      IncludeMetadataFile = 1 << 0, //!< Indicates that any associated .qmd metadata file should be included with the operation
      IncludeStyleFile = 1 << 1, //!< Indicates that any associated .qml styling file should be included with the operation
    };
    //! File operation flags
    Q_DECLARE_FLAGS( FileOperationFlags, FileOperationFlag )
    Q_ENUM( FileOperationFlag )

    /**
     * Generic map layer properties.
     *
     * \since QGIS 3.22
     */
    enum class MapLayerProperty : int
    {
      UsersCannotToggleEditing = 1 << 0, //!< Indicates that users are not allowed to toggle editing for this layer. Note that this does not imply that the layer is non-editable (see isEditable(), supportsEditing() ), rather that the editable status of the layer cannot be changed by users manually. Since QGIS 3.22.
      IsBasemapLayer = 1 << 1, //!< Layer is considered a 'basemap' layer, and certain properties of the layer should be ignored when calculating project-level properties. For instance, the extent of basemap layers is ignored when calculating the extent of a project, as these layers are typically global and extend outside of a project's area of interest. Since QGIS 3.26.
    };
    //! Map layer properties
    Q_DECLARE_FLAGS( MapLayerProperties, MapLayerProperty )
    Q_ENUM( MapLayerProperty )


    /**
     * Generic data provider flags.
     *
     * \since QGIS 3.26
     */
    enum class DataProviderFlag : int
    {
      IsBasemapSource = 1 << 1, //!< Associated source should be considered a 'basemap' layer. See Qgis::MapLayerProperty::IsBasemapLayer.
    };
    //! Data provider flags
    Q_DECLARE_FLAGS( DataProviderFlags, DataProviderFlag )
    Q_ENUM( DataProviderFlag )

    /**
     * Flags for annotation items.
     *
     * \since QGIS 3.22
     */
    enum class AnnotationItemFlag : int
    {
      ScaleDependentBoundingBox = 1 << 0, //!< Item's bounding box will vary depending on map scale
    };
    //! Annotation item flags
    Q_DECLARE_FLAGS( AnnotationItemFlags, AnnotationItemFlag )
    Q_ENUM( AnnotationItemFlag )

    /**
     * Flags for controlling how an annotation item behaves in the GUI.
     *
     * \since QGIS 3.22
     */
    enum class AnnotationItemGuiFlag : int
    {
      FlagNoCreationTools = 1 << 0,  //!< Do not show item creation tools for the item type
    };
    //! Annotation item GUI flags
    Q_DECLARE_FLAGS( AnnotationItemGuiFlags, AnnotationItemGuiFlag )
    Q_ENUM( AnnotationItemGuiFlag )

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
     * Flags which adjust the way maps are rendered.
     *
     * \since QGIS 3.22
     */
    enum class MapSettingsFlag SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsMapSettings, Flag ) : int
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
    };
    //! Map settings flags
    Q_DECLARE_FLAGS( MapSettingsFlags, MapSettingsFlag ) SIP_MONKEYPATCH_FLAGS_UNNEST( QgsMapSettings, Flags )
    Q_ENUM( MapSettingsFlag )

    /**
     * Flags which affect rendering operations.
     *
     * \since QGIS 3.22
     */
    enum class RenderContextFlag SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsRenderContext, Flag ) : int
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
    };
    //! Render context flags
    Q_DECLARE_FLAGS( RenderContextFlags, RenderContextFlag ) SIP_MONKEYPATCH_FLAGS_UNNEST( QgsRenderContext, Flags )
    Q_ENUM( RenderContextFlag )

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
    };
    Q_ENUM( MarkerShape )

    /**
     * Defines how/where the symbols should be placed on a line.
     *
     * \note Prior to QGIS 3.24 this was available as QgsTemplatedLineSymbolLayerBase::Placement
     *
     * \since QGIS 3.24
     */
    enum class MarkerLinePlacement SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsTemplatedLineSymbolLayerBase, Placement ) : int
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
    enum class TextRendererFlag : int
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
    };
    Q_ENUM( AngularDirection )

    /**
     *  Usage of the renderer.
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
     * History provider backends.
     *
     * \since QGIS 3.24
     */
    enum class HistoryProviderBackend : int
    {
      LocalProfile = 1 << 0, //!< Local profile
//      Project = 1 << 1, //!< QGIS Project  (not yet implemented)
    };
    Q_ENUM( HistoryProviderBackend )
    Q_DECLARE_FLAGS( HistoryProviderBackends, HistoryProviderBackend )
    Q_FLAG( HistoryProviderBackends )

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
     * Identify search radius in mm
     * \since QGIS 2.3
     */
    static const double DEFAULT_SEARCH_RADIUS_MM;

    //! Default threshold between map coordinates and device coordinates for map2pixel simplification
    static const float DEFAULT_MAPTOPIXEL_THRESHOLD;

    /**
     * Default highlight color.  The transparency is expected to only be applied to polygon
     * fill. Lines and outlines are rendered opaque.
     *
     *  \since QGIS 2.3
     */
    static const QColor DEFAULT_HIGHLIGHT_COLOR;

    /**
     * Default highlight buffer in mm.
     *  \since QGIS 2.3
     */
    static const double DEFAULT_HIGHLIGHT_BUFFER_MM;

    /**
     * Default highlight line/stroke minimum width in mm.
     * \since QGIS 2.3
     */
    static const double DEFAULT_HIGHLIGHT_MIN_WIDTH_MM;

    /**
     * Fudge factor used to compare two scales. The code is often going from scale to scale
     *  denominator. So it looses precision and, when a limit is inclusive, can lead to errors.
     *  To avoid that, use this factor instead of using <= or >=.
     * \since QGIS 2.15
     */
    static const double SCALE_PRECISION;

    /**
     * Default Z coordinate value.
     * This value have to be assigned to the Z coordinate for the vertex.
     * \since QGIS 3.0
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
     *  \since QGIS 3.0
     */
    static const double UI_SCALE_FACTOR;

    /**
     * Default snapping distance tolerance.
     *  \since QGIS 3.0
     */
    static const double DEFAULT_SNAP_TOLERANCE;

    /**
     * Default snapping distance units.
     *  \since QGIS 3.0
     */
    static const QgsTolerance::UnitType DEFAULT_SNAP_UNITS;

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

Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::SymbolRenderHints )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::SymbolFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::SymbolPreviewFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::SymbolLayerFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::BrowserItemCapabilities )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::SublayerQueryFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::SublayerFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::SqlLayerDefinitionCapabilities )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::BabelFormatCapabilities )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::BabelCommandFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::GeometryValidityFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::FileOperationFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::AnnotationItemFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::AnnotationItemGuiFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::MapSettingsFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::RenderContextFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::VectorLayerTypeFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::MarkerLinePlacements )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::TextRendererFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::HistoryProviderBackends )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::MapLayerProperties )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::DataProviderFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::SnappingTypes )


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
 * \since QGIS 2.16
 */
// based on Boojum's code from http://stackoverflow.com/questions/3556687/prevent-firing-signals-in-qt
template<class Object> class QgsSignalBlocker SIP_SKIP SIP_SKIP // clazy:exclude=rule-of-three
{
  public:

    /**
     * Constructor for QgsSignalBlocker
     * \param object QObject to block signals from
     */
    explicit QgsSignalBlocker( Object * object )
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
 * \since QGIS 2.16
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
  QString str = QString::number( a, 'f', precision );
  if ( precision )
  {
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

/**
 * Compare two doubles (but allow some difference)
 * \param a first double
 * \param b second double
 * \param epsilon maximum difference allowable between doubles
 */
inline bool qgsDoubleNear( double a, double b, double epsilon = 4 * std::numeric_limits<double>::epsilon() )
{
  const bool aIsNan = std::isnan( a );
  const bool bIsNan = std::isnan( b );
  if ( aIsNan || bIsNan )
    return aIsNan && bIsNan;

  const double diff = a - b;
  return diff > -epsilon && diff <= epsilon;
}

/**
 * Compare two floats (but allow some difference)
 * \param a first float
 * \param b second float
 * \param epsilon maximum difference allowable between floats
 */
inline bool qgsFloatNear( float a, float b, float epsilon = 4 * FLT_EPSILON )
{
  const bool aIsNan = std::isnan( a );
  const bool bIsNan = std::isnan( b );
  if ( aIsNan || bIsNan )
    return aIsNan && bIsNan;

  const float diff = a - b;
  return diff > -epsilon && diff <= epsilon;
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
 * \since QGIS 3.0
 */
inline double qgsRound( double number, int places )
{
  const double m = ( number < 0.0 ) ? -1.0 : 1.0;
  const double scaleFactor = std::pow( 10.0, places );
  return ( std::round( number * m * scaleFactor ) / scaleFactor ) * m;
}


#ifndef SIP_RUN

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
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    return list.toSet();
#else
    return QSet<T>( list.begin(), list.end() );
#endif
  }

  template<class T>
  QList<T> setToList( const QSet<T> &set )
  {
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    return set.toList();
#else
    return QList<T>( set.begin(), set.end() );
#endif
  }
}

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
namespace std
{
  template<> struct hash<QString>
  {
    std::size_t operator()( const QString &s ) const noexcept
    {
      return ( size_t ) qHash( s );
    }
  };
}
#endif

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
 * \since QGIS 2.9
 */
CORE_EXPORT double qgsPermissiveToDouble( QString string, bool &ok );

/**
 * Converts a string to an integer in a permissive way, e.g., allowing for incorrect
 * numbers of digits between thousand separators
 * \param string string to convert
 * \param ok will be set to TRUE if conversion was successful
 * \returns string converted to int if possible
 * \see permissiveToDouble
 * \since QGIS 2.9
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

#if __cplusplus >= 201500
#define FALLTHROUGH [[fallthrough]];
#elif defined(__clang__)
#define FALLTHROUGH [[clang::fallthrough]];
#elif defined(__GNUC__) && __GNUC__ >= 7
#define FALLTHROUGH [[gnu::fallthrough]];
#else
#define FALLTHROUGH
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
