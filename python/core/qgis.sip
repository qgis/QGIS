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
  static const char* qgisVersion;
  // Version number used for comparing versions using the "Check QGIS Version" function
  static const int qgisVersionInt;
  // Release name
  static const char* qgisReleaseName;
  // The subversion version
  static const char* qgisSvnVersion;

  // Enumerations
  //

  //! Used for symbology operations
  // Feature types
  enum WKBTYPE
  {
    WKBPoint = 1,
    WKBLineString,
    WKBPolygon,
    WKBMultiPoint,
    WKBMultiLineString,
    WKBMultiPolygon,
    WKBUnknown
  };
  enum VectorType
  {
    Point,
    Line,
    Polygon,
    Unknown
  };
  
  // TODO:
  //static const char *qgisVectorGeometryType[];
  //! description strings for feature types
  //static const char *qgisFeatureTypes[];

  //! map units that qgis supports
  enum units
  {
    METERS,
    FEET,
    DEGREES,
    UNKNOWN
  };

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


  /** WKT string that represents a geographic coord sys */
  const  QString GEOWKT;
  
  /** PROJ4 string that represents a geographic coord sys */
  const QString GEOPROJ4;
  /** Magic number for a geographic coord sys in POSTGIS SRID */
  const long GEOSRID;
  /** Magic number for a geographic coord sys in QGIS srs.db tbl_srs.srs_id */
  const long GEOSRS_ID;
  /**  Magic number for a geographic coord sys in EPSG ID format */
  const long GEOEPSG_ID;
  /** The length of teh string "+proj=" */
  const int PROJ_PREFIX_LEN;
  /** The length of teh string "+ellps=" */
  const int ELLPS_PREFIX_LEN;
  /** Magick number that determins whether a projection srsid is a system (srs.db)
   *  or user (~/.qgis.qgis.db) defined projection. */
  const int USER_PROJECTION_START_ID;

  //
  // Constants for point symbols
  //

  /** Magic number that determines the minimum allowable point size for point symbols */
  const float MINIMUM_POINT_SIZE;
  /** Magic number that determines the minimum allowable point size for point symbols */
  const float DEFAULT_POINT_SIZE;
