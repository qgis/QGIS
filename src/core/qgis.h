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
  VectorLayer,
  RasterLayer,
  PluginLayer,
  MeshLayer,      //!< Added in 3.2
  VectorTileLayer, //!< Added in 3.14
  AnnotationLayer, //!< Contains freeform, georeferenced annotations. Added in QGIS 3.16
  PointCloudLayer, //!< Added in 3.18
};


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
     * \brief Flags controlling behavior of symbols during rendering
     *
     * \since QGIS 3.20
     */
    enum class SymbolRenderHint SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsSymbol, RenderHint ) : int
      {
      DynamicRotation = 2, //!< Rotation of symbol may be changed during rendering and symbol should not be cached
    };
    Q_ENUM( SymbolRenderHint )
    Q_DECLARE_FLAGS( SymbolRenderHints, SymbolRenderHint )

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
    Q_DECLARE_FLAGS( SymbolPreviewFlags, SymbolPreviewFlag )

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
    };
    Q_ENUM( BrowserItemCapability )
    Q_DECLARE_FLAGS( BrowserItemCapabilities, BrowserItemCapability )

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
    };
    Q_DECLARE_FLAGS( SublayerQueryFlags, SublayerQueryFlag )
    Q_ENUM( SublayerQueryFlag )

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

Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::SymbolRenderHints )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::SymbolFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::SymbolPreviewFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::BrowserItemCapabilities )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::SublayerQueryFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( Qgis::SqlLayerDefinitionCapabilities )

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
  double ar = std::frexp( a, &aexp );
  double br = std::frexp( b, &bexp );

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
  double m = ( number < 0.0 ) ? -1.0 : 1.0;
  double scaleFactor = std::pow( 10.0, places );
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
///@endcond
#endif

/**
 * Returns a map of all enum entries.
 * The map has the enum values (int) as keys and the enum keys (QString) as values.
 * The enum must have been declared using Q_ENUM or Q_FLAG.
 */
template<class T> const QMap<T, QString> qgsEnumMap() SIP_SKIP
{
  QMetaEnum metaEnum = QMetaEnum::fromType<T>();
  Q_ASSERT( metaEnum.isValid() );
  QMap<T, QString> enumMap;
  for ( int idx = 0; idx < metaEnum.keyCount(); ++idx )
  {
    const char *enumKey = metaEnum.key( idx );
    enumMap.insert( static_cast<T>( metaEnum.keyToValue( enumKey ) ), QString( enumKey ) );
  }
  return enumMap;
}

/**
 * Returns the value for the given key of an enum.
 * \since QGIS 3.6
 */
template<class T> QString qgsEnumValueToKey( const T &value ) SIP_SKIP
{
  QMetaEnum metaEnum = QMetaEnum::fromType<T>();
  Q_ASSERT( metaEnum.isValid() );
  return QString::fromUtf8( metaEnum.valueToKey( static_cast<int>( value ) ) );
}

/**
 * Returns the value corresponding to the given \a key of an enum.
 * If the key is invalid, it will return the \a defaultValue.
 * If \a tryValueAsKey is TRUE, it will try to convert the string key to an enum value
 * \since QGIS 3.6
 */
template<class T> T qgsEnumKeyToValue( const QString &key, const T &defaultValue, bool tryValueAsKey = true ) SIP_SKIP
{
  QMetaEnum metaEnum = QMetaEnum::fromType<T>();
  Q_ASSERT( metaEnum.isValid() );
  bool ok = false;
  T v = static_cast<T>( metaEnum.keyToValue( key.toUtf8().data(), &ok ) );
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
      int intValue = key.toInt( &canConvert );
      if ( canConvert && metaEnum.valueToKey( intValue ) )
      {
        return static_cast<T>( intValue );
      }
    }
  }
  return defaultValue;
}

/**
 * Returns the value for the given keys of a flag.
 * \since QGIS 3.16
 */
template<class T> QString qgsFlagValueToKeys( const T &value ) SIP_SKIP
{
  QMetaEnum metaEnum = QMetaEnum::fromType<T>();
  Q_ASSERT( metaEnum.isValid() );
  return QString::fromUtf8( metaEnum.valueToKeys( static_cast<int>( value ) ) );
}

/**
 * Returns the value corresponding to the given \a keys of a flag.
 * If the keys are invalid, it will return the \a defaultValue.
 * \since QGIS 3.16
 */
template<class T> T qgsFlagKeysToValue( const QString &keys, const T &defaultValue ) SIP_SKIP
{
  QMetaEnum metaEnum = QMetaEnum::fromType<T>();
  Q_ASSERT( metaEnum.isValid() );
  bool ok = false;
  T v = static_cast<T>( metaEnum.keysToValue( keys.toUtf8().constData(), &ok ) );
  if ( ok )
    return v;
  else
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
 * Allocates  memory for an array of nmemb elements of size bytes each and returns
 * a pointer to the allocated memory. Works like C calloc() but prints debug message
 * by QgsLogger if allocation fails.
 * \param nmemb number of elements
 * \param size size of element in bytes
 */
void CORE_EXPORT *qgsCalloc( size_t nmemb, size_t size ) SIP_SKIP;

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
#  ifdef _MSC_VER
// do not warn about C bindings returning QString
#    pragma warning(disable:4190)
#  endif
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
#if defined(__GNUC__) && !defined(__clang__)
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
#define DEFAULT_BUILTIN_UNREACHABLE \
  default: \
  __builtin_unreachable();
#else
#define DEFAULT_BUILTIN_UNREACHABLE
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
