/*!  \mainpage Quantum GIS
*
*  \section about  About QGIS
* Quantum GIS (QGIS) is a user friendly Open Source Geographic Information
* System (GIS) that runs on Linux, Unix, Mac OSX, and Windows. QGIS supports
* vector, raster, and database formats. QGIS is licensed under the GNU Public
* License.
*
* This API documentation provides information about all classes that make up QGIS.
*/

%ModuleHeaderCode
#include <qgis.h>
%End

%ModuleCode
  int QgisEvent = QEvent::User + 1;
%End


class QGis
{
%TypeHeaderCode
#include <qgis.h>
%End
  
public:
  // Version constants
  //
  // Version string 
  static const char* QGIS_VERSION;
  // Version number used for comparing versions using the "Check QGIS Version" function
  static const int QGIS_VERSION_INT;
  // Release name
  static const char* QGIS_RELEASE_NAME;
  // The subversion version
  static const char* QGIS_DEV_VERSION;

  // Enumerations
  //

  //! Used for symbology operations
  // Feature types
  enum WkbType
  {
    WKBPoint = 1,
    WKBLineString,
    WKBPolygon,
    WKBMultiPoint,
    WKBMultiLineString,
    WKBMultiPolygon,
    WKBUnknown,
    WKBPoint25D = 0x80000001,
    WKBLineString25D,
    WKBPolygon25D,
    WKBMultiPoint25D,
    WKBMultiLineString25D,
    WKBMultiPolygon25D
  };
  enum GeometryType
  {
    Point,
    Line,
    Polygon,
    UnknownGeometry
  };
  
  // TODO:
  //static const char *qgisVectorGeometryType[];
  //! description strings for feature types
  //static const char *qgisFeatureTypes[];

  /** Map units that qgis supports
   * @note that QGIS < 1.4 api had only Meters, Feet, Degrees and UnknownUnit
   */
  enum UnitType
  {
    Meters = 0,
    Feet = 1,
    Degrees = 2, //for 1.0 api backwards compatibility
    DecimalDegrees = 2,
    DegreesMinutesSeconds = 4,
    DegreesDecimalMinutes = 5,
    UnknownUnit = 3
  } ;

  //! User defined event types
  enum UserEvent
  {
    // These first two are useful for threads to alert their parent data providers

    //! The extents have been calculated by a provider of a layer
    ProviderExtentCalcEvent = QgisEvent,

    //! The row count has been calculated by a provider of a layer
    ProviderCountCalcEvent
  };
  
  static const int DEFAULT_IDENTIFY_RADIUS;
};


  /** Wkt string that represents a geographic coord sys
   * @note added in 1.8 to replace GEOWkt
  */
  const QString GEOWKT;
  /** Wkt string that represents a geographic coord sys
  * @note deprecated in 1.8 due to violation of coding conventions (globals
  *  should be in all caps).
  */
  const QString GEOWkt /Deprecated/;
  
  /** PROJ4 string that represents a geographic coord sys */
  const QString GEOPROJ4;
  /** Magic number for a geographic coord sys in POSTGIS SRID */
  const long GEOSRID;
  /** Magic number for a geographic coord sys in QGIS srs.db tbl_srs.srs_id */
  const long GEOCRS_ID;
  /**  Magic number for a geographic coord sys in EpsgCrsId ID format */
  const long GEO_EPSG_CRS_ID;
  /** The length of teh string "+proj=" */
  const int PROJ_PREFIX_LEN;
  /** The length of teh string "+ellps=" */
  const int ELLPS_PREFIX_LEN;
  /** The length of the string "+lat_1=" */
  const int LAT_PREFIX_LEN;
  /** Magick number that determins whether a projection srsid is a system (srs.db)
   *  or user (~/.qgis.qgis.db) defined projection. */
  const int USER_CRS_START_ID;

  //
  // Constants for point symbols
  //

  /** Magic number that determines the minimum allowable point size for point symbols */
  const float MINIMUM_POINT_SIZE;
  /** Magic number that determines the minimum allowable point size for point symbols */
  const float DEFAULT_POINT_SIZE;
  const double DEFAULT_LINE_WIDTH;

