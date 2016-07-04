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

#include <QEvent>
#include <QString>
#include <QRegExp>
#include <QMetaType>
#include <QVariant>
#include <QDateTime>
#include <QDate>
#include <QTime>
#include <QHash>
#include <stdlib.h>
#include <cfloat>
#include <cmath>
#include <qnumeric.h>

#include <qgswkbtypes.h>

/** \ingroup core
 * The QGis class provides global constants for use throughout the application.
 */
class CORE_EXPORT QGis
{
  public:
    // Version constants
    //
    // Version string
    static const char* QGIS_VERSION;
    // Version number used for comparing versions using the "Check QGIS Version" function
    static const int QGIS_VERSION_INT;
    // Release name
    static const char* QGIS_RELEASE_NAME;
    // The development version
    static const char* QGIS_DEV_VERSION;

    // Enumerations
    //

    //! Used for symbology operations
    // Feature types
    // @deprecated use QgsWKBTypes::Type
    /* Q_DECL_DEPRECATED */
    enum WkbType
    {
      WKBUnknown = 0,
      WKBPoint = 1,
      WKBLineString,
      WKBPolygon,
      WKBMultiPoint,
      WKBMultiLineString,
      WKBMultiPolygon,
      WKBNoGeometry = 100, //attributes only
      WKBPoint25D = 0x80000001,
      WKBLineString25D,
      WKBPolygon25D,
      WKBMultiPoint25D,
      WKBMultiLineString25D,
      WKBMultiPolygon25D,
    };

    //! Map multi to single type
    // @deprecated use QgsWKBTypes::singleType
    /* Q_DECL_DEPRECATED */
    static WkbType singleType( WkbType type );

    //! Map single to multitype type
    // @deprecated use QgsWKBTypes::multiType
    /* Q_DECL_DEPRECATED */
    static WkbType multiType( WkbType type );

    //! Map 2d+ to 2d type
    // @deprecated use QgsWKBTypes::flatType
    /* Q_DECL_DEPRECATED */
    static WkbType flatType( WkbType type );

    //! Return if type is a single type
    // @deprecated use QgsWKBTypes::isSingleType
    /* Q_DECL_DEPRECATED */
    static bool isSingleType( WkbType type );

    //! Return if type is a multi type
    // @deprecated use QgsWKBTypes::isMultiType
    /* Q_DECL_DEPRECATED */
    static bool isMultiType( WkbType type );

    // get dimension of points
    // @deprecated use QgsWKBTypes::coordDimensions()
    /* Q_DECL_DEPRECATED */
    static int wkbDimensions( WkbType type );

    //! Converts from old (pre 2.10) WKB type (OGR) to new WKB type
    static QgsWKBTypes::Type fromOldWkbType( QGis::WkbType type );

    //! Converts from new (post 2.10) WKB type (OGC) to old WKB type
    static QGis::WkbType fromNewWkbType( QgsWKBTypes::Type type );

    enum GeometryType
    {
      Point,
      Line,
      Polygon,
      UnknownGeometry,
      NoGeometry
    };

    //! description strings for geometry types
    static const char *vectorGeometryType( GeometryType type );

    //! description strings for feature types
    static const char *featureType( WkbType type );

    /** Raster data types.
     *  This is modified and extended copy of GDALDataType.
     */
    enum DataType
    {
      /** Unknown or unspecified type */                UnknownDataType = 0,
      /** Eight bit unsigned integer (quint8) */        Byte = 1,
      /** Sixteen bit unsigned integer (quint16) */     UInt16 = 2,
      /** Sixteen bit signed integer (qint16) */        Int16 = 3,
      /** Thirty two bit unsigned integer (quint32) */  UInt32 = 4,
      /** Thirty two bit signed integer (qint32) */     Int32 = 5,
      /** Thirty two bit floating point (float) */      Float32 = 6,
      /** Sixty four bit floating point (double) */     Float64 = 7,
      /** Complex Int16 */                              CInt16 = 8,
      /** Complex Int32 */                              CInt32 = 9,
      /** Complex Float32 */                            CFloat32 = 10,
      /** Complex Float64 */                            CFloat64 = 11,
      /** Color, alpha, red, green, blue, 4 bytes the same as
          QImage::Format_ARGB32 */                      ARGB32 = 12,
      /** Color, alpha, red, green, blue, 4 bytes  the same as
          QImage::Format_ARGB32_Premultiplied */        ARGB32_Premultiplied = 13
    };


    /** Map units that qgis supports
     * @note that QGIS < 1.4 api had only Meters, Feet, Degrees and UnknownUnit
     * @note and QGIS >1.8 returns to that
     */
    //TODO QGIS 3.0 - clean up and move to QgsUnitTypes and rename to DistanceUnit
    enum UnitType
    {
      Meters = 0, /*!< meters */
      Feet = 1, /*!< imperial feet */
      Degrees = 2, /*!< degrees, for planar geographic CRS distance measurements */ //for 1.0 api backwards compatibility
      NauticalMiles = 7, /*!< nautical miles */
      Kilometers = 8, /*!< kilometers */
      Yards = 9, /*!< imperial yards */
      Miles = 10, /*!< terrestial miles */

      UnknownUnit = 3, /*!< unknown distance unit */

      // for [1.4;1.8] api compatibility
      DecimalDegrees = 2,         // was 2
      DegreesMinutesSeconds = 2,  // was 4
      DegreesDecimalMinutes = 2,  // was 5
    };

    //! Provides the canonical name of the type value
    //! @deprecated use QgsUnitTypes::encodeUnit() instead
    Q_DECL_DEPRECATED static QString toLiteral( QGis::UnitType unit );

    //! Converts from the canonical name to the type value
    //! @deprecated use QgsUnitTypes::decodeDistanceUnit() instead
    Q_DECL_DEPRECATED static UnitType fromLiteral( const QString& literal, QGis::UnitType defaultType = UnknownUnit );

    //! Provides translated version of the type value
    //! @deprecated use QgsUnitTypes::toString() instead
    Q_DECL_DEPRECATED static QString tr( QGis::UnitType unit );

    //! Provides type value from translated version
    //! @deprecated use QgsUnitTypes::stringToDistanceUnit() instead
    Q_DECL_DEPRECATED static UnitType fromTr( const QString& literal, QGis::UnitType defaultType = UnknownUnit );

    //! Returns the conversion factor between the specified units
    //! @deprecated use QgsUnitTyoes::fromUnitToUnitFactor() instead
    Q_DECL_DEPRECATED static double fromUnitToUnitFactor( QGis::UnitType fromUnit, QGis::UnitType toUnit );

    /** Converts a string to a double in a permissive way, eg allowing for incorrect
     * numbers of digits between thousand separators
     * @param string string to convert
     * @param ok will be set to true if conversion was successful
     * @returns string converted to double if possible
     * @note added in version 2.9
     * @see permissiveToInt
     */
    static double permissiveToDouble( QString string, bool& ok );

    /** Converts a string to an integer in a permissive way, eg allowing for incorrect
     * numbers of digits between thousand separators
     * @param string string to convert
     * @param ok will be set to true if conversion was successful
     * @returns string converted to int if possible
     * @note added in version 2.9
     * @see permissiveToDouble
     */
    static int permissiveToInt( QString string, bool& ok );

    //! User defined event types
    enum UserEvent
    {
      // These first two are useful for threads to alert their parent data providers

      //! The extents have been calculated by a provider of a layer
      ProviderExtentCalcEvent = ( QEvent::User + 1 ),

      //! The row count has been calculated by a provider of a layer
      ProviderCountCalcEvent
    };

    /** Old search radius in % of canvas width
     *  @deprecated since 2.3, use DEFAULT_SEARCH_RADIUS_MM */
    Q_DECL_DEPRECATED static const double DEFAULT_IDENTIFY_RADIUS;

    /** Identify search radius in mm
     *  @note added in 2.3 */
    static const double DEFAULT_SEARCH_RADIUS_MM;

    //! Default threshold between map coordinates and device coordinates for map2pixel simplification
    static const float DEFAULT_MAPTOPIXEL_THRESHOLD;

    /** Default highlight color.  The transparency is expected to only be applied to polygon
     *  fill. Lines and outlines are rendered opaque.
     *  @note added in 2.3 */
    static const QColor DEFAULT_HIGHLIGHT_COLOR;

    /** Default highlight buffer in mm.
     *  @note added in 2.3 */
    static double DEFAULT_HIGHLIGHT_BUFFER_MM;

    /** Default highlight line/outline minimum width in mm.
     *  @note added in 2.3 */
    static double DEFAULT_HIGHLIGHT_MIN_WIDTH_MM;

    /** Fudge factor used to compare two scales. The code is often going from scale to scale
     *  denominator. So it looses precision and, when a limit is inclusive, can lead to errors.
     *  To avoid that, use this factor instead of using <= or >=.
     * @note added in 2.15*/
    static double SCALE_PRECISION;

  private:
    // String representation of unit types (set in qgis.cpp)
    static const char *qgisUnitTypes[];

};

// hack to workaround warnings when casting void pointers
// retrieved from QLibrary::resolve to function pointers.
// It's assumed that this works on all systems supporting
// QLibrary
#if QT_VERSION >= 0x050000
#define cast_to_fptr(f) f
#else
inline void ( *cast_to_fptr( void *p ) )()
{
  union
  {
    void *p;
    void ( *f )();
  } u;

  u.p = p;
  return u.f;
}
#endif

/** \ingroup core
 * RAII signal blocking class. Used for temporarily blocking signals from a QObject
 * for the lifetime of QgsSignalBlocker object.
 * @see whileBlocking()
 * @note added in QGIS 2.16
 * @note not available in Python bindings
 */
// based on Boojum's code from http://stackoverflow.com/questions/3556687/prevent-firing-signals-in-qt
template<class Object> class QgsSignalBlocker
{
  public:

    /** Constructor for QgsSignalBlocker
     * @param object QObject to block signals from
     */
    explicit QgsSignalBlocker( Object* object )
        : mObject( object )
        , mPreviousState( object->blockSignals( true ) )
    {}

    ~QgsSignalBlocker()
    {
      mObject->blockSignals( mPreviousState );
    }

    //! Returns pointer to blocked QObject
    Object* operator->() { return mObject; }

  private:

    Object* mObject;
    bool mPreviousState;

};

/** Temporarily blocks signals from a QObject while calling a single method from the object.
 *
 * Usage:
 *   whileBlocking( checkBox )->setChecked( true );
 *   whileBlocking( spinBox )->setValue( 50 );
 *
 * No signals will be emitted when calling these methods.
 *
 * @note added in QGIS 2.16
 * @see QgsSignalBlocker
 * @note not available in Python bindings
 */
// based on Boojum's code from http://stackoverflow.com/questions/3556687/prevent-firing-signals-in-qt
template<class Object> inline QgsSignalBlocker<Object> whileBlocking( Object* object )
{
  return QgsSignalBlocker<Object>( object );
}

//! Returns a string representation of a double
//! @param a double value
//! @param precision number of decimal places to retain
inline QString qgsDoubleToString( double a, int precision = 17 )
{
  if ( precision )
    return QString::number( a, 'f', precision ).remove( QRegExp( "\\.?0+$" ) );
  else
    return QString::number( a, 'f', precision );
}

//! Compare two doubles (but allow some difference)
//! @param a first double
//! @param b second double
//! @param epsilon maximum difference allowable between doubles
inline bool qgsDoubleNear( double a, double b, double epsilon = 4 * DBL_EPSILON )
{
  const double diff = a - b;
  return diff > -epsilon && diff <= epsilon;
}

//! Compare two floats (but allow some difference)
//! @param a first float
//! @param b second float
//! @param epsilon maximum difference allowable between floats
inline bool qgsFloatNear( float a, float b, float epsilon = 4 * FLT_EPSILON )
{
  const float diff = a - b;
  return diff > -epsilon && diff <= epsilon;
}

//! Compare two doubles using specified number of significant digits
inline bool qgsDoubleNearSig( double a, double b, int significantDigits = 10 )
{
  // The most simple would be to print numbers as %.xe and compare as strings
  // but that is probably too costly
  // Then the fastest would be to set some bits directly, but little/big endian
  // has to be considered (maybe TODO)
  // Is there a better way?
  int aexp, bexp;
  double ar = frexp( a, &aexp );
  double br = frexp( b, &bexp );

  return aexp == bexp &&
         qRound( ar * pow( 10.0, significantDigits ) ) == qRound( br * pow( 10.0, significantDigits ) );
}

//! A round function which returns a double to guard against overflows
inline double qgsRound( double x )
{
  return x < 0.0 ? std::ceil( x - 0.5 ) : std::floor( x + 0.5 );
}

// Add missing qHash implementation for QDate, QTime, QDateTime
// implementations taken from upstream Qt5 versions
#if QT_VERSION < 0x050000

//! Hash implementation for QDateTime
//! @note not available in Python bindings
inline uint qHash( const QDateTime &key )
{
  return qHash( key.toMSecsSinceEpoch() );
}

//! Hash implementation for QDate
//! @note not available in Python bindings
inline uint qHash( const QDate &key )
{
  return qHash( key.toJulianDay() );
}

//! Hash implementation for QTime
//! @note not available in Python bindings
inline uint qHash( const QTime &key )
{
  return QTime( 0, 0, 0, 0 ).msecsTo( key );
}
#endif

//! Compares two QVariant values and returns whether the first is less than the second.
//! Useful for sorting lists of variants, correctly handling sorting of the various
//! QVariant data types (such as strings, numeric values, dates and times)
//! @see qgsVariantGreaterThan()
CORE_EXPORT bool qgsVariantLessThan( const QVariant& lhs, const QVariant& rhs );

//! Compares two QVariant values and returns whether the first is greater than the second.
//! Useful for sorting lists of variants, correctly handling sorting of the various
//! QVariant data types (such as strings, numeric values, dates and times)
//! @see qgsVariantLessThan()
CORE_EXPORT bool qgsVariantGreaterThan( const QVariant& lhs, const QVariant& rhs );

CORE_EXPORT QString qgsVsiPrefix( const QString& path );

/** Allocates size bytes and returns a pointer to the allocated  memory.
    Works like C malloc() but prints debug message by QgsLogger if allocation fails.
    @param size size in bytes
 */
void CORE_EXPORT *qgsMalloc( size_t size );

/** Allocates  memory for an array of nmemb elements of size bytes each and returns
    a pointer to the allocated memory. Works like C calloc() but prints debug message
    by QgsLogger if allocation fails.
    @param nmemb number of elements
    @param size size of element in bytes
 */
void CORE_EXPORT *qgsCalloc( size_t nmemb, size_t size );

/** Frees the memory space  pointed  to  by  ptr. Works like C free().
    @param ptr pointer to memory space
 */
void CORE_EXPORT qgsFree( void *ptr );

/** Wkt string that represents a geographic coord sys
 * @note added to replace GEOWkt
 */
extern CORE_EXPORT const QString GEOWKT;
extern CORE_EXPORT const QString PROJECT_SCALES;

/** PROJ4 string that represents a geographic coord sys */
extern CORE_EXPORT const QString GEOPROJ4;
/** Magic number for a geographic coord sys in POSTGIS SRID */
const long GEOSRID = 4326;
/** Magic number for a geographic coord sys in QGIS srs.db tbl_srs.srs_id */
const long GEOCRS_ID = 3452;
/** Magic number for a geographic coord sys in EpsgCrsId ID format */
const long GEO_EPSG_CRS_ID = 4326;
/** Geographic coord sys from EPSG authority */
extern CORE_EXPORT const QString GEO_EPSG_CRS_AUTHID;
/** The length of the string "+proj=" */
const int PROJ_PREFIX_LEN = 6;
/** The length of the string "+ellps=" */
const int ELLPS_PREFIX_LEN = 7;
/** The length of the string "+lat_1=" */
const int LAT_PREFIX_LEN = 7;
/** Magick number that determines whether a projection crsid is a system (srs.db)
 *  or user (~/.qgis.qgis.db) defined projection. */
const int USER_CRS_START_ID = 100000;

//! Constant that holds the string representation for "No ellips/No CRS"
extern CORE_EXPORT const QString GEO_NONE;

//
// Constants for point symbols
//

/** Magic number that determines the minimum allowable point size for point symbols */
const double MINIMUM_POINT_SIZE = 0.1;
/** Magic number that determines the default point size for point symbols */
const double DEFAULT_POINT_SIZE = 2.0;
const double DEFAULT_LINE_WIDTH = 0.26;

/** Default snapping tolerance for segments */
const double DEFAULT_SEGMENT_EPSILON = 1e-8;

typedef QMap<QString, QString> QgsStringMap;

/** Qgssize is used instead of size_t, because size_t is stdlib type, unknown
 *  by SIP, and it would be hard to define size_t correctly in SIP.
 *  Currently used "unsigned long long" was introduced in C++11 (2011)
 *  but it was supported already before C++11 on common platforms.
 *  "unsigned long long int" gives syntax error in SIP.
 *  KEEP IN SYNC WITH qgssize defined in SIP! */
typedef unsigned long long qgssize;

#if (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)) || defined(__clang__)
#define Q_NOWARN_DEPRECATED_PUSH \
  _Pragma("GCC diagnostic push") \
  _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"");
#define Q_NOWARN_DEPRECATED_POP \
  _Pragma("GCC diagnostic pop");
#elif defined(_MSC_VER)
#define Q_NOWARN_DEPRECATED_PUSH \
  __pragma(warning(push)) \
  __pragma(warning(disable:4996))
#define Q_NOWARN_DEPRECATED_POP \
  __pragma(warning(pop))
#else
#define Q_NOWARN_DEPRECATED_PUSH
#define Q_NOWARN_DEPRECATED_POP
#endif

#ifndef QGISEXTERN
#ifdef Q_OS_WIN
#  define QGISEXTERN extern "C" __declspec( dllexport )
#  ifdef _MSC_VER
// do not warn about C bindings returing QString
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

#if defined(__clang__)
#define FALLTHROUGH [[clang::fallthrough]]
#else
#define FALLTHROUGH
#endif
