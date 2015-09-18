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
#include <stdlib.h>
#include <cfloat>
#include <cmath>
#include <qnumeric.h>

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

    static WkbType singleType( WkbType type )
    {
      switch ( type )
      {
        case WKBMultiPoint:         return WKBPoint;
        case WKBMultiLineString:    return WKBLineString;
        case WKBMultiPolygon:       return WKBPolygon;
        case WKBMultiPoint25D:      return WKBPoint25D;
        case WKBMultiLineString25D: return WKBLineString25D;
        case WKBMultiPolygon25D:    return WKBPolygon25D;
        default:                    return type;
      }
    }

    static WkbType multiType( WkbType type )
    {
      switch ( type )
      {
        case WKBPoint:         return WKBMultiPoint;
        case WKBLineString:    return WKBMultiLineString;
        case WKBPolygon:       return WKBMultiPolygon;
        case WKBPoint25D:      return WKBMultiPoint25D;
        case WKBLineString25D: return WKBMultiLineString25D;
        case WKBPolygon25D:    return WKBMultiPolygon25D;
        default:               return type;
      }
    }

    static WkbType flatType( WkbType type )
    {
      switch ( type )
      {
        case WKBPoint25D:           return WKBPoint;
        case WKBLineString25D:      return WKBLineString;
        case WKBPolygon25D:         return WKBPolygon;
        case WKBMultiPoint25D:      return WKBMultiPoint;
        case WKBMultiLineString25D: return WKBMultiLineString;
        case WKBMultiPolygon25D:    return WKBMultiPolygon;
        default:                    return type;
      }
    }

    static bool isSingleType( WkbType type )
    {
      switch ( flatType( type ) )
      {
        case WKBPoint:
        case WKBLineString:
        case WKBPolygon:
          return true;
        default:
          return false;
      }
    }

    static bool isMultiType( WkbType type )
    {
      switch ( flatType( type ) )
      {
        case WKBMultiPoint:
        case WKBMultiLineString:
        case WKBMultiPolygon:
          return true;
        default:
          return false;
      }
    }

    static int wkbDimensions( WkbType type )
    {
      switch ( type )
      {
        case WKBUnknown:            return 0;
        case WKBNoGeometry:         return 0;
        case WKBPoint25D:           return 3;
        case WKBLineString25D:      return 3;
        case WKBPolygon25D:         return 3;
        case WKBMultiPoint25D:      return 3;
        case WKBMultiLineString25D: return 3;
        case WKBMultiPolygon25D:    return 3;
        default:                    return 2;
      }
    }

    enum GeometryType
    {
      Point,
      Line,
      Polygon,
      UnknownGeometry,
      NoGeometry
    };

    //! description strings for geometry types
    static const char *vectorGeometryType( GeometryType type )
    {
      switch ( type )
      {
        case Point:           return "Point";
        case Line:            return "Line";
        case Polygon:         return "Polygon";
        case UnknownGeometry: return "Unknown geometry";
        case NoGeometry:      return "No geometry";
        default:              return "Invalid type";
      }
    }

    //! description strings for feature types
    static const char *featureType( WkbType type )
    {
      switch ( type )
      {
        case WKBUnknown:            return "WKBUnknown";
        case WKBPoint:              return "WKBPoint";
        case WKBLineString:         return "WKBLineString";
        case WKBPolygon:            return "WKBPolygon";
        case WKBMultiPoint:         return "WKBMultiPoint";
        case WKBMultiLineString:    return "WKBMultiLineString";
        case WKBMultiPolygon:       return "WKBMultiPolygon";
        case WKBNoGeometry:         return "WKBNoGeometry";
        case WKBPoint25D:           return "WKBPoint25D";
        case WKBLineString25D:      return "WKBLineString25D";
        case WKBPolygon25D:         return "WKBPolygon25D";
        case WKBMultiPoint25D:      return "WKBMultiPoint25D";
        case WKBMultiLineString25D: return "WKBMultiLineString25D";
        case WKBMultiPolygon25D:    return "WKBMultiPolygon25D";
        default:                    return "invalid wkbtype";
      }
    }

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
    enum UnitType
    {
      Meters = 0,
      Feet = 1,
      Degrees = 2, //for 1.0 api backwards compatibility
      UnknownUnit = 3,

      // for [1.4;1.8] api compatibility
      DecimalDegrees = 2,         // was 2
      DegreesMinutesSeconds = 2,  // was 4
      DegreesDecimalMinutes = 2,  // was 5
      NauticalMiles = 7
    };

    //! Provides the canonical name of the type value
    static QString toLiteral( QGis::UnitType unit );
    //! Converts from the canonical name to the type value
    static UnitType fromLiteral( QString  literal, QGis::UnitType defaultType = UnknownUnit );
    //! Provides translated version of the type value
    static QString tr( QGis::UnitType unit );
    //! Provides type value from translated version
    static UnitType fromTr( QString literal, QGis::UnitType defaultType = UnknownUnit );
    //! Returns the conversion factor between the specified units
    static double fromUnitToUnitFactor( QGis::UnitType fromUnit, QGis::UnitType toUnit );

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
    static const double DEFAULT_IDENTIFY_RADIUS;

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

//
// return a string representation of a double
//
inline QString qgsDoubleToString( const double &a, const int &precision = 17 )
{
  if ( precision )
    return QString::number( a, 'f', precision ).remove( QRegExp( "\\.?0+$" ) );
  else
    return QString::number( a, 'f', precision );
}

//
// compare two doubles (but allow some difference)
//
inline bool qgsDoubleNear( double a, double b, double epsilon = 4 * DBL_EPSILON )
{
  const double diff = a - b;
  return diff > -epsilon && diff <= epsilon;
}

//
// compare two doubles using specified number of significant digits
//
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

bool qgsVariantLessThan( const QVariant& lhs, const QVariant& rhs );

bool qgsVariantGreaterThan( const QVariant& lhs, const QVariant& rhs );

CORE_EXPORT QString qgsVsiPrefix( QString path );

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
