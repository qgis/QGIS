/***************************************************************************
                             qgscoordinatereferencesystem.h

                             -------------------
    begin                : 2007
    copyright            : (C) 2007 by Gary E. Sherman
    email                : sherman@mrcc.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCOORDINATEREFERENCESYSTEM_H
#define QGSCOORDINATEREFERENCESYSTEM_H

//Standard includes
#include "qgis_core.h"
#include <ostream>

//qt includes
#include <QString>
#include <QMap>
#include <QHash>
#include <QReadWriteLock>
#include <QExplicitlySharedDataPointer>
#include <QObject>

//qgis includes
#include "qgis_sip.h"
#include "qgsconfig.h"
#include "qgsunittypes.h"
#include "qgsrectangle.h"
#include "qgssqliteutils.h"

class QDomNode;
class QDomDocument;
class QgsCoordinateReferenceSystemPrivate;
class QgsDatumEnsemble;
class QgsProjectionFactors;
class QgsProjOperation;

#ifndef SIP_RUN
struct PJconsts;
typedef struct PJconsts PJ;

#if PROJ_VERSION_MAJOR>=8
struct pj_ctx;
typedef struct pj_ctx PJ_CONTEXT;
#else
struct projCtx_t;
typedef struct projCtx_t PJ_CONTEXT;
#endif
#endif

// forward declaration for sqlite3
typedef struct sqlite3 sqlite3 SIP_SKIP;

#ifdef DEBUG
typedef struct OGRSpatialReferenceHS *OGRSpatialReferenceH SIP_SKIP;
#else
typedef void *OGRSpatialReferenceH SIP_SKIP;
#endif

class QgsCoordinateReferenceSystem;
typedef void ( *CUSTOM_CRS_VALIDATION )( QgsCoordinateReferenceSystem & ) SIP_SKIP;

/**
 * \ingroup core
 * \brief This class represents a coordinate reference system (CRS).
 *
 * Coordinate reference system object defines a specific map projection, as well as transformations
 * between different coordinate reference systems. There are various ways how a CRS can be defined:
 * using well-known text (WKT), PROJ string or combination of authority and code (e.g. EPSG:4326).
 * QGIS comes with its internal database of coordinate reference systems (stored in SQLite) that
 * allows lookups of CRS and seamless conversions between the various definitions.
 *
 * Most commonly one comes across two types of coordinate systems:
 *
 * - Geographic coordinate systems: based on a geodetic datum, normally with coordinates being
 *   latitude/longitude in degrees. The most common one is World Geodetic System 84 (WGS84).
 * - Projected coordinate systems: based on a geodetic datum with coordinates projected to a plane,
 *   typically using meters or feet as units. Common projected coordinate systems are Universal
 *   Transverse Mercator or Albers Equal Area.
 *
 * Internally QGIS uses proj library for all the math behind coordinate transformations, so in case
 * of any troubles with projections it is best to examine the PROJ representation within the object,
 * as that is the representation that will be ultimately used.
 *
 * Methods that allow inspection of CRS instances include isValid(), authid(), description(),
 * toWkt(), toProj(), mapUnits() and others.
 * Creation of CRS instances is further described in \ref crs_construct_and_copy section below.
 * Transformations between coordinate reference systems are done using QgsCoordinateTransform class.
 *
 * For example, the following code will create and inspect "British national grid" CRS:
 *
 * \code{.py}
 * crs = QgsCoordinateReferenceSystem("EPSG:27700")
 * if crs.isValid():
 *     print("CRS Description: {}".format(crs.description()))
 *     print("CRS PROJ text: {}".format(crs.toProj()))
 * else:
 *     print("Invalid CRS!")
 * \endcode
 *
 * This will produce the following output:
 *
 * \code{.unparsed}
 * CRS Description: OSGB 1936 / British National Grid
 * CRS PROJ text: +proj=tmerc +lat_0=49 +lon_0=-2 +k=0.9996012717 +x_0=400000 +y_0=-100000 [output trimmed]
 * \endcode
 *
 * \section crs_def_formats CRS Definition Formats
 *
 * This section gives an overview of various supported CRS definition formats:
 *
 * - Authority and Code: Also referred to as OGC WMS format within QGIS as they have been widely
 *   used in OGC standards. These are encoded as `<auth>:<code>`, for example `EPSG:4326` refers
 *   to WGS84 system. EPSG is the most commonly used authority that covers a wide range
 *   of coordinate systems around the world.
 *
 *   An extended variant of this format is OGC URN. Syntax of URN for CRS definition is
 *   `urn:ogc:def:crs:<auth>:[<version>]:<code>`. This class can also parse URNs (versions
 *   are currently ignored). For example, WGS84 may be encoded as `urn:ogc:def:crs:OGC:1.3:CRS84`.
 *
 *   QGIS adds support for "USER" authority that refers to IDs used internally in QGIS. This variant
 *   is best avoided or used with caution as the IDs are not permanent and they refer to different CRS
 *   on different machines or user profiles.
 *
 *    \see authid()
 *   \see createFromOgcWmsCrs()
 *
 * - PROJ string: This is a string consisting of a series of key/value pairs in the following
 *   format: `+param1=value1 +param2=value2 [...]`. This is the format natively used by the
 *   underlying proj library. For example, the definition of WGS84 looks like this:
 *
 *   \code{.unparsed}
 *   +proj=longlat +datum=WGS84 +no_defs
 *   \endcode
 *
 *   \see toProj()
 *   \see createFromProj()
 *
 * - Well-known text (WKT): Defined by Open Geospatial Consortium (OGC), this is another common
 *   format to define CRS. For WGS84 the OGC WKT definition is the following:
 *
 *   \code{.unparsed}
 *       GEOGCS["WGS 84",
 *              DATUM["WGS_1984",
 *                SPHEROID["WGS 84",6378137,298.257223563,AUTHORITY["EPSG","7030"]],
 *                AUTHORITY["EPSG","6326"]],
 *              PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],
 *              UNIT["degree",0.0174532925199433,AUTHORITY["EPSG","9122"]],
 *              AUTHORITY["EPSG","4326"]]
 *   \endcode
 *
 *   \see toWkt()
 *   \see createFromWkt()
 *
 * \section crs_db_and_custom CRS Database and Custom CRS
 *
 * The database of CRS shipped with QGIS is stored in a SQLite database (see QgsApplication::srsDatabaseFilePath())
 * and it is based on the data files maintained by GDAL project (a variety of .csv and .wkt files).
 *
 * Sometimes it happens that users need to use a CRS definition that is not well known
 * or that has been only created with a specific purpose (and thus its definition is not
 * available in our database of CRS). Whenever a new CRS definition is seen, it will
 * be added to the local database (in user's home directory, see QgsApplication::qgisUserDatabaseFilePath()).
 * QGIS also features a GUI for management of local custom CRS definitions.
 *
 * There are therefore two databases: one for shipped CRS definitions and one for custom CRS definitions.
 * Custom CRS have internal IDs (accessible with srsid()) greater or equal to \ref USER_CRS_START_ID.
 * The local CRS databases should never be accessed directly with SQLite functions, instead
 * you should use QgsCoordinateReferenceSystem API for CRS lookups and for managements of custom CRS.
 *
 * \section validation Validation
 *
 * In some cases (most prominently when loading a map layer), QGIS will try to ensure
 * that the given map layer CRS is valid using validate() call. If not, a custom
 * validation function will be called - such function may for example show a GUI
 * for manual CRS selection. The validation function is configured using setCustomCrsValidation().
 * If validation fails or no validation function is set, the default CRS is assigned
 * (WGS84). QGIS application registers its validation function that will act according
 * to user's settings (either show CRS selector dialog or use project/custom CRS).
 *
 * \section crs_construct_and_copy Object Construction and Copying
 *
 * The easiest way of creating CRS instances is to use QgsCoordinateReferenceSystem(const QString&)
 * constructor that automatically recognizes definition format from the given string.
 *
 * Creation of CRS object involves some queries in a local SQLite database, which may
 * be potentially expensive. Consequently, CRS creation methods use an internal cache to avoid
 * unnecessary database lookups. If the CRS database is modified, then it is necessary to call
 * invalidateCache() to ensure that outdated records are not being returned from the cache.
 *
 * Since QGIS 2.16 QgsCoordinateReferenceSystem objects are implicitly shared.
 *
 * \section caveats Caveats
 *
 * There are two different flavors of WKT: one is defined by OGC, the other is the standard
 * used by ESRI. They look very similar, but they are not the same. QGIS is able to consume
 * both flavors.
 *
 * \see QgsCoordinateTransform
 */

class CORE_EXPORT QgsCoordinateReferenceSystem
{
    Q_GADGET

    Q_PROPERTY( QgsUnitTypes::DistanceUnit mapUnits READ mapUnits )
    Q_PROPERTY( bool isGeographic READ isGeographic )
    Q_PROPERTY( QString authid READ authid )
    Q_PROPERTY( QString description READ description )

  public:

    //! Enumeration of types of IDs accepted in createFromId() method
    enum CrsType
    {
      InternalCrsId,  //!< Internal ID used by QGIS in the local SQLite database
      PostgisCrsId,   //!< SRID used in PostGIS. DEPRECATED -- DO NOT USE
      EpsgCrsId       //!< EPSG code
    };

    //! Constructs an invalid CRS object
    QgsCoordinateReferenceSystem();

    ~QgsCoordinateReferenceSystem();

    // TODO QGIS 4: remove "POSTGIS" and "INTERNAL"

    /**
     * Constructs a CRS object from a string definition using createFromString()
     *
     * It supports the following formats:
     *
     * - "EPSG:<code>" - handled with createFromOgcWms()
     * - "POSTGIS:<srid>" - handled with createFromSrid()
     * - "INTERNAL:<srsid>" - handled with createFromSrsId()
     * - "PROJ:<proj>" - handled with createFromProj()
     * - "WKT:<wkt>" - handled with createFromWkt()
     *
     * If no prefix is specified, WKT definition is assumed.
     * \param definition A String containing a coordinate reference system definition.
     * \see createFromString()
     */
    explicit QgsCoordinateReferenceSystem( const QString &definition );

    // TODO QGIS 4: remove type and always use EPSG code

    /**
     * Constructor
     *
     * A CRS object using a PostGIS SRID, an EPSG code or an internal QGIS CRS ID.
     * \note We encourage you to use EPSG code or WKT to describe CRSes in your code
     * wherever possible. Internal QGIS CRS IDs are not guaranteed to be permanent / involatile,
     * and proj strings are a lossy format.
     * \param id The ID valid for the chosen CRS ID type
     * \param type One of the types described in CrsType
     * \deprecated QGIS 3.10 We encourage you to use EPSG codes or WKT to describe CRSes in your code wherever possible. Internal QGIS CRS IDs are not guaranteed to be permanent / involatile, and Proj strings are a lossy format.
     */
    Q_DECL_DEPRECATED explicit QgsCoordinateReferenceSystem( long id, CrsType type = PostgisCrsId ) SIP_DEPRECATED;

    //! Copy constructor
    QgsCoordinateReferenceSystem( const QgsCoordinateReferenceSystem &srs );

    //! Assignment operator
    QgsCoordinateReferenceSystem &operator=( const QgsCoordinateReferenceSystem &srs );

    //! Allows direct construction of QVariants from QgsCoordinateReferenceSystem.
    operator QVariant() const
    {
      return QVariant::fromValue( *this );
    }

    /**
     * Returns a list of all valid SRS IDs present in the CRS database. Any of the
     * returned values can be safely passed to fromSrsId() to create a new, valid
     * QgsCoordinateReferenceSystem object.
     * \see fromSrsId()
     * \since QGIS 3.0
     */
    static QList< long > validSrsIds();

    // static creators

    /**
     * Creates a CRS from a given OGC WMS-format Coordinate Reference System string.
     * \param ogcCrs OGR compliant CRS definition, e.g., "EPSG:4326"
     * \returns matching CRS, or an invalid CRS if string could not be matched
     * \see createFromOgcWmsCrs()
     * \since QGIS 3.0
    */
    static QgsCoordinateReferenceSystem fromOgcWmsCrs( const QString &ogcCrs );

    /**
     * Creates a CRS from a given EPSG ID.
     * \param epsg epsg CRS ID
     * \returns matching CRS, or an invalid CRS if string could not be matched
     * \since QGIS 3.0
    */
    Q_INVOKABLE static QgsCoordinateReferenceSystem fromEpsgId( long epsg );

    /**
     * Creates a CRS from a proj style formatted string.
     * \returns matching CRS, or an invalid CRS if string could not be matched
     * \see createFromProj()
     * \deprecated QGIS 3.10 Use fromProj() instead.
    */
    Q_DECL_DEPRECATED static QgsCoordinateReferenceSystem fromProj4( const QString &proj4 ) SIP_DEPRECATED;

    /**
     * Creates a CRS from a proj style formatted string.
     * \param proj proj format string
     * \returns matching CRS, or an invalid CRS if string could not be matched
     * \see createFromProj()
     * \since QGIS 3.10.3
    */
    static QgsCoordinateReferenceSystem fromProj( const QString &proj );

    /**
     * Creates a CRS from a WKT spatial ref sys definition string.
     * \param wkt WKT for the desired spatial reference system.
     * \returns matching CRS, or an invalid CRS if string could not be matched
     * \see createFromWkt()
     * \since QGIS 3.0
    */
    static QgsCoordinateReferenceSystem fromWkt( const QString &wkt );

    /**
     * Creates a CRS from a specified QGIS SRS ID.
     * \param srsId internal QGIS SRS ID
     * \returns matching CRS, or an invalid CRS if ID could not be found
     * \see createFromSrsId()
     * \see validSrsIds()
     * \since QGIS 3.0
    */
    static QgsCoordinateReferenceSystem fromSrsId( long srsId );

    // Misc helper functions -----------------------

    // TODO QGIS 4: remove type and always use EPSG code, rename to createFromEpsg

    /**
     * Sets this CRS by lookup of the given ID in the CRS database.
     * \returns TRUE on success else FALSE
     * \deprecated QGIS 3.10 We encourage you to use EPSG code or WKT to describe CRSes in your code wherever possible. Internal QGIS CRS IDs are not guaranteed to be permanent / involatile, and Proj strings are a lossy format.
     */
    Q_DECL_DEPRECATED bool createFromId( long id, CrsType type = PostgisCrsId ) SIP_DEPRECATED;

    // TODO QGIS 4: remove "QGIS" and "CUSTOM", only support "USER" (also returned by authid())

    /**
     * Sets this CRS to the given OGC WMS-format Coordinate Reference Systems.
     *
     * Accepts both "<auth>:<code>" format and OGC URN "urn:ogc:def:crs:<auth>:[<version>]:<code>".
     * It also recognizes "QGIS", "USER", "CUSTOM" authorities, which all have the same meaning
     * and refer to QGIS internal CRS IDs.
     * \returns TRUE on success else FALSE
     * \note this method uses an internal cache. Call invalidateCache() to clear the cache.
     * \see fromOgcWmsCrs()
     */
    bool createFromOgcWmsCrs( const QString &crs );

    // TODO QGIS 4: remove unless really necessary - let's use EPSG codes instead

    /**
     * Sets this CRS by lookup of the given PostGIS SRID in the CRS database.
     * \param srid The PostGIS SRID for the desired spatial reference system.
     * \returns TRUE on success else FALSE
     *
     * \deprecated QGIS 3.10 Use alternative methods for SRS construction instead -- this method was specifically created for use by the postgres provider alone, and using it elsewhere will lead to subtle bugs.
     */
    Q_DECL_DEPRECATED bool createFromSrid( long srid ) SIP_DEPRECATED;

    /**
     * Sets this CRS using a WKT definition.
     *
     * If EPSG code of the WKT definition can be determined, it is extracted
     * and createFromOgcWmsCrs() is used to initialize the object.
     *
     * \param wkt The WKT for the desired spatial reference system.
     * \returns TRUE on success else FALSE
     * \note Some members may be left blank if no match can be found in CRS database.
     * \note this method uses an internal cache. Call invalidateCache() to clear the cache.
     * \see fromWkt()
     */
    bool createFromWkt( const QString &wkt );

    /**
     * Sets this CRS by lookup of internal QGIS CRS ID in the CRS database.
     *
     * If the srsid is < USER_CRS_START_ID, system CRS database is used, otherwise
     * user's local CRS database from home directory is used.
     * \param srsId The internal QGIS CRS ID for the desired spatial reference system.
     * \returns TRUE on success else FALSE
     * \note this method uses an internal cache. Call invalidateCache() to clear the cache.
     * \see fromSrsId()
     * \warning This method is highly discouraged, and CRS objects should instead be constructed
     * using auth:id codes or WKT strings
     */
    bool createFromSrsId( long srsId );

    /**
     * Sets this CRS by passing it a PROJ style formatted string.
     *
     * The string will be parsed and the projection and ellipsoid
     * members set and the remainder of the Proj string will be stored
     * in the parameters member. The reason for this is so that we
     * can easily present the user with 'natural language' representation
     * of the projection and ellipsoid by looking them up in the srs.db sqlite
     * database.
     *
     * We try to match the Proj string to internal QGIS CRS ID using the following logic:
     *
     * - ask the Proj library to identify the CRS to a standard registered CRS (e.g. EPSG codes)
     * - if no match is found, compare the CRS to all user CRSes, using the Proj library to determine CRS equivalence (hence making the match parameter order insensitive)
     * - if none of the above match, use the Proj string to create the CRS and do not associated an internal CRS ID to it.
     *
     * \param projString A Proj format string
     * \returns TRUE on success else FALSE
     * \note Some members may be left blank if no match can be found in CRS database.
     * \note This method uses an internal cache. Call invalidateCache() to clear the cache.
     * \see fromProj()
     * \deprecated QGIS 3.10 Use createFromProj() instead
     */
    Q_DECL_DEPRECATED bool createFromProj4( const QString &projString ) SIP_DEPRECATED;

    /**
     * Sets this CRS by passing it a PROJ style formatted string.
     *
     * The string will be parsed and the projection and ellipsoid
     * members set and the remainder of the Proj string will be stored
     * in the parameters member. The reason for this is so that we
     * can easily present the user with 'natural language' representation
     * of the projection and ellipsoid by looking them up in the srs.db sqlite
     * database.
     *
     * We try to match the Proj string to internal QGIS CRS ID using the following logic:
     *
     * - ask the Proj library to identify the CRS to a standard registered CRS (e.g. EPSG codes)
     * - if no match is found, compare the CRS to all user CRSes, using the Proj library to determine CRS equivalence (hence making the match parameter order insensitive)
     * - if none of the above match, use the Proj string to create the CRS and do not associated an internal CRS ID to it.
     *
     * \param projString A Proj format string
     * \param identify if FALSE, no attempts will be made to match the proj string against known CRS authorities. This is much
     * faster, but should only ever be used when it is known in advance that the definition does not correspond to a known or user CRS. This
     * argument is not available in Python.
     *
     * \returns TRUE on success else FALSE
     * \note Some members may be left blank if no match can be found in CRS database.
     * \note This method uses an internal cache. Call invalidateCache() to clear the cache.
     * \see fromProj()
     * \since QGIS 3.10.3
     */
#ifndef SIP_RUN
    bool createFromProj( const QString &projString, bool identify = true );
#else
    bool createFromProj( const QString &projString );
#endif

    /**
     * Set up this CRS from a string definition.
     *
     * It supports the following formats:
     *
     * - "EPSG:<code>" - handled with createFromOgcWms()
     * - "POSTGIS:<srid>" - handled with createFromSrid()
     * - "INTERNAL:<srsid>" - handled with createFromSrsId()
     * - "PROJ:<proj>" - handled with createFromProj()
     * - "WKT:<wkt>" - handled with createFromWkt()
     *
     * If no prefix is specified, WKT definition is assumed.
     * \param definition A String containing a coordinate reference system definition.
     * \returns TRUE on success else FALSE
     */
    bool createFromString( const QString &definition );

    // TODO QGIS 4: rename to createFromStringOGR so it is clear it's similar to createFromString, just different backend

    /**
     * Set up this CRS from various text formats.
     *
     * Valid formats: WKT string, "EPSG:n", "EPSGA:n", "AUTO:proj_id,unit_id,lon0,lat0",
     * "urn:ogc:def:crs:EPSG::n", PROJ string, filename (with WKT, XML or PROJ string),
     * well known name (such as NAD27, NAD83, WGS84 or WGS72),
     * ESRI::[WKT string] (directly or in a file), "IGNF:xxx"
     *
     * For more details on supported formats see OGRSpatialReference::SetFromUserInput()
     * ( https://gdal.org/doxygen/classOGRSpatialReference.html#aec3c6a49533fe457ddc763d699ff8796 )
     * \param definition A String containing a coordinate reference system definition.
     * \returns TRUE on success else FALSE
     * \note this function generates a WKT string using OSRSetFromUserInput() and
     * passes it to createFromWkt() function.
     */
    bool createFromUserInput( const QString &definition );

    /**
     * Make sure that ESRI WKT import is done properly.
     * This is required for proper shapefile CRS import when using gdal>= 1.9.
     * \note This function is called by createFromUserInput() and QgsOgrProvider::crs(), there is usually
     * no need to call it from elsewhere.
     * \note This function sets CPL config option GDAL_FIX_ESRI_WKT to a proper value,
     * unless it has been set by the user through the commandline or an environment variable.
     * For more details refer to OGRSpatialReference::morphFromESRI() .
     * \deprecated QGIS 3.10 Not used on builds based on Proj version 6 or later
     */
    Q_DECL_DEPRECATED static void setupESRIWktFix() SIP_DEPRECATED;

    //! Returns whether this CRS is correctly initialized and usable
    bool isValid() const;

    /**
     * Perform some validation on this CRS. If the CRS doesn't validate the
     * default behavior settings for layers with unknown CRS will be
     * consulted and acted on accordingly. By hell or high water this
     * method will do its best to make sure that this CRS is valid - even
     * if that involves resorting to a hard coded default of geocs:wgs84.
     *
     * \note It is not usually necessary to use this function, unless you
     * are trying to force this CRS to be valid.
     * \see setCustomCrsValidation(), customCrsValidation()
     */
    void validate();

    // TODO QGIS 4: seems completely obsolete now (only compares proj4 - already done in createFromProj4)

    /**
     * Walks the CRS databases (both system and user database) trying to match
     *  stored PROJ string to a database entry in order to fill in further
     *  pieces of information about CRS.
     *  \note The ellipsoid and projection acronyms must be set as well as the proj string!
     *  \returns long the SrsId of the matched CRS, zero if no match was found
     * \deprecated QGIS 3.10 Not used in Proj >= 6 based builds
     */
    Q_DECL_DEPRECATED long findMatchingProj() SIP_DEPRECATED;

    /**
     * Overloaded == operator used to compare to CRS's.
     *
     *  Internally it will use authid() for comparison.
     */
    bool operator==( const QgsCoordinateReferenceSystem &srs ) const;

    /**
     * Overloaded != operator used to compare to CRS's.
     *
     *  Returns opposite bool value to operator ==
     */
    bool operator!=( const QgsCoordinateReferenceSystem &srs ) const;

    /**
     * Restores state from the given DOM node.
     * If it fails or if the node is empty, a default empty CRS will be returned.
     * \param node The node from which state will be restored
     * \returns bool TRUE on success, FALSE on failure
     */
    bool readXml( const QDomNode &node );

    /**
     * Stores state to the given Dom node in the given document.
     * \param node The node in which state will be restored
     * \param doc The document in which state will be stored
     * \returns bool TRUE on success, FALSE on failure
     */
    bool writeXml( QDomNode &node, QDomDocument &doc ) const;


    /**
     * Sets custom function to force valid CRS
     * \note not available in Python bindings
     */
    static void setCustomCrsValidation( CUSTOM_CRS_VALIDATION f ) SIP_SKIP;

    /**
     * Gets custom function
     * \note not available in Python bindings
     */
    static CUSTOM_CRS_VALIDATION customCrsValidation() SIP_SKIP;

    // Accessors -----------------------------------

    /**
     * Returns the internal CRS ID, if available.
     *  \returns the internal sqlite3 srs.db primary key for this CRS
     */
    long srsid() const;

    // TODO QGIS 4: remove unless really necessary - let's use EPSG codes instead

    /**
     * Returns PostGIS SRID for the CRS.
     * \returns the PostGIS spatial_ref_sys identifier for this CRS (defaults to 0)
     */
    long postgisSrid() const;

    /**
     * Returns the authority identifier for the CRS.
     *
     * The identifier includes both the authority (e.g., EPSG) and the CRS number (e.g., 4326).
     * This is the best method to use when showing a very short CRS identifier to a user,
     * e.g., "EPSG:4326".
     *
     * If CRS object is a custom CRS (not found in database), the method will return
     * internal QGIS CRS ID with "QGIS" authority, for example "QGIS:100005"
     * \returns the authority identifier for this CRS
     * \see description()
     */
    QString authid() const;

    /**
     * Returns the descriptive name of the CRS, e.g., "WGS 84" or "GDA 94 / Vicgrid94".

     * \note an empty string will be returned if the description is not available for the CRS
     * \see authid()
     * \see userFriendlyIdentifier()
     */
    QString description() const;

    /**
     * Type of identifier string to create.
     *
     * \since QGIS 3.10.3
     */
    enum IdentifierType
    {
      ShortString, //!< A heavily abbreviated string, for use when a compact representation is required
      MediumString, //!< A medium-length string, recommended for general purpose use
      FullString, //!< Full definition -- possibly a very lengthy string, e.g. with no truncation of custom WKT definitions
    };

    /**
     * Returns a user friendly identifier for the CRS.
     *
     * Depending on the format of the CRS, this may reflect the CRSes registered name, or for
     * CRSes not saved in the database it may reflect the underlying WKT or Proj string definition
     * of the CRS.
     *
     * In most cases this is the best method to use when showing a friendly identifier for the CRS to a
     * user.
     *
     * \see description()
     * \since QGIS 3.10.3
     */
    QString userFriendlyIdentifier( IdentifierType type = MediumString ) const;

    /**
     * Returns the projection acronym for the projection used by the CRS.
     * \returns the official Proj acronym for the projection family
     * \note an empty string will be returned if the projectionAcronym is not available for the CRS
     * \see ellipsoidAcronym()
     */
    QString projectionAcronym() const;

    /**
     * Returns the ellipsoid acronym for the ellipsoid used by the CRS.
     * \returns the official authority:code identifier for the ellipsoid, or PARAMETER:MAJOR:MINOR for custom ellipsoids
     * \note an empty string will be returned if the ellipsoidAcronym is not available for the CRS
     * \see projectionAcronym()
     */
    QString ellipsoidAcronym() const;

    //! WKT formatting variants, only used for builds based on Proj >= 6
    enum WktVariant
    {
      WKT1_GDAL, //!< WKT1 as traditionally output by GDAL, deriving from OGC 01-009. A notable departure from WKT1_GDAL with respect to OGC 01-009 is that in WKT1_GDAL, the unit of the PRIMEM value is always degrees.
      WKT1_ESRI, //!< WKT1 as traditionally output by ESRI software, deriving from OGC 99-049.
      WKT2_2015, //!< Full WKT2 string, conforming to ISO 19162:2015(E) / OGC 12-063r5 with all possible nodes and new keyword names.
      WKT2_2015_SIMPLIFIED, //!< Same as WKT2_2015 with the following exceptions: UNIT keyword used. ID node only on top element. No ORDER element in AXIS element. PRIMEM node omitted if it is Greenwich.  ELLIPSOID.UNIT node omitted if it is UnitOfMeasure::METRE. PARAMETER.UNIT / PRIMEM.UNIT omitted if same as AXIS. AXIS.UNIT omitted and replaced by a common GEODCRS.UNIT if they are all the same on all axis.
      WKT2_2018, //!< Alias for WKT2_2019
      WKT2_2018_SIMPLIFIED, //!< Alias for WKT2_2019_SIMPLIFIED
      WKT2_2019 = WKT2_2018, //!< Full WKT2 string, conforming to ISO 19162:2019 / OGC 18-010, with all possible nodes and new keyword names. Non-normative list of differences: WKT2_2019 uses GEOGCRS / BASEGEOGCRS keywords for GeographicCRS.
      WKT2_2019_SIMPLIFIED = WKT2_2018_SIMPLIFIED, //!< WKT2_2019 with the simplification rule of WKT2_SIMPLIFIED

      WKT_PREFERRED = WKT2_2019, //!< Preferred format, matching the most recent WKT ISO standard. Currently an alias to WKT2_2019, but may change in future versions.
      WKT_PREFERRED_SIMPLIFIED = WKT2_2019_SIMPLIFIED, //!< Preferred simplified format, matching the most recent WKT ISO standard. Currently an alias to WKT2_2019_SIMPLIFIED, but may change in future versions.
      WKT_PREFERRED_GDAL = WKT2_2019, //!< Preferred format for conversion of CRS to WKT for use with the GDAL library.
    };

    /**
     * Returns a WKT representation of this CRS.
     *
     * The \a variant argument specifies the formatting variant to use when creating the WKT string. This is
     * only used on builds based on Proj >= 6, with earlier versions always using WKT1_GDAL.
     *
     * If \a multiline is TRUE then a formatted multiline string will be returned, using the specified \a indentationWidth.
     * This is only used on builds based on Proj >= 6.
     *
     * \see toProj()
     */
    QString toWkt( WktVariant variant = WKT1_GDAL, bool multiline = false, int indentationWidth = 4 ) const;

    /**
     * Returns a Proj string representation of this CRS.
     *
     * If proj and ellps keys are found in the parameters,
     * they will be stripped out and the projection and ellipsoid acronyms will be
     * overridden with these.
     * \returns Proj format string that defines this CRS.
     * \warning Not all CRS definitions can be represented by Proj strings. An empty
     * string will be returned if the CRS could not be represented by a Proj string.
     * \see toWkt()
     * \deprecated QGIS 3.10 Use toProj() instead.
     */
    Q_DECL_DEPRECATED QString toProj4() const SIP_DEPRECATED;

    /**
     * Returns a Proj string representation of this CRS.
     *
     * If proj and ellps keys are found in the parameters,
     * they will be stripped out and the projection and ellipsoid acronyms will be
     * overridden with these.
     * \returns Proj format string that defines this CRS.
     * \warning Not all CRS definitions can be represented by Proj strings. An empty
     * string will be returned if the CRS could not be represented by a Proj string.
     * \see toWkt()
     * \since QGIS 3.10.3
     */
    QString toProj() const;

    /**
     * Returns whether the CRS is a geographic CRS (using lat/lon coordinates)
     * \returns TRUE if CRS is geographic, or FALSE if it is a projected CRS
     */
    bool isGeographic() const;

    /**
     * Returns TRUE if the CRS is a dynamic CRS.
     *
     * A dynamic CRS relies on a dynamic datum, that is a datum that is not
     * plate-fixed.
     *
     * \since QGIS 3.20
     */
    bool isDynamic() const;

    /**
     * Attempts to retrieve datum ensemble details from the CRS.
     *
     * If the CRS does not use a datum ensemble then an invalid QgsDatumEnsemble will
     * be returned.
     *
     * \warning This method requires PROJ 8.0 or later
     *
     * \throws QgsNotSupportedException on QGIS builds based on PROJ 7 or earlier.
     *
     * \since QGIS 3.20
     */
    QgsDatumEnsemble datumEnsemble() const SIP_THROW( QgsNotSupportedException );

    /**
     * Attempts to retrieve the name of the celestial body associated with the CRS (e.g. "Earth").
     *
     * \warning This method requires PROJ 8.1 or later
     *
     * \throws QgsNotSupportedException on QGIS builds based on PROJ 8.0 or earlier.
     *
     * \since QGIS 3.20
     */
    QString celestialBodyName() const SIP_THROW( QgsNotSupportedException );

    /**
     * Sets the coordinate \a epoch, as a decimal year.
     *
     * In a dynamic CRS (see isDynamic()), coordinates of a point on the surface of the Earth may
     * change with time. To be unambiguous the coordinates must always be qualified
     * with the epoch at which they are valid. The coordinate epoch is not necessarily
     * the epoch at which the observation was collected.
     *
     * Pedantically the coordinate epoch of an observation belongs to the
     * observation, and not to the CRS, however it is often more practical to
     * bind it to the CRS. The coordinate epoch should be specified for dynamic
     * CRS (see isDynamic()).
     *
     * \param epoch Coordinate epoch as decimal year (e.g. 2021.3)
     *
     * \warning The QgsCoordinateTransform class can perform time-dependent transformations
     * between a static and dynamic CRS based on either the source or destination CRS coordinate epoch,
     * however dynamic CRS to dynamic CRS transformations are not currently supported.
     *
     * \see coordinateEpoch()
     *
     * \since QGIS 3.20
     */
    void setCoordinateEpoch( double epoch );

    /**
     * Returns the coordinate epoch, as a decimal year.
     *
     * In a dynamic CRS, coordinates of a point on the surface of the Earth may
     * change with time. To be unambiguous the coordinates must always be qualified
     * with the epoch at which they are valid. The coordinate epoch is not necessarily
     * the epoch at which the observation was collected.
     *
     * Pedantically the coordinate epoch of an observation belongs to the
     * observation, and not to the CRS, however it is often more practical to
     * bind it to the CRS. The coordinate epoch should be specified for dynamic
     * CRS (see isDynamic()).
     *
     * \warning The QgsCoordinateTransform class can perform time-dependent transformations
     * between a static and dynamic CRS based on either the source or destination CRS coordinate epoch,
     * however dynamic CRS to dynamic CRS transformations are not currently supported.
     *
     * \returns Coordinate epoch as decimal year (e.g. 2021.3), or NaN if not set, or relevant.
     *
     * \see setCoordinateEpoch()
     *
     * \since QGIS 3.20
     */
    double coordinateEpoch() const;

    /**
     * Calculate various cartographic properties, such as scale factors, angular distortion and meridian convergence for
     * the CRS at the given geodetic \a point (in geographic coordinates).
     *
     * Depending on the underlying projection values will be calculated either numerically (default) or analytically.
     * The function also calculates the partial derivatives of the given coordinate.
     *
     * \note Internally uses the proj library proj_factors API to calculate the factors.
     *
     * \since QGIS 3.20
     */
    QgsProjectionFactors factors( const QgsPoint &point ) const;

    /**
     * Returns information about the PROJ operation associated with the coordinate reference system, for example
     * the projection method used by the CRS.
     *
     * \since QGIS 3.20
     */
    QgsProjOperation operation() const;

    /**
     * Returns whether axis is inverted (e.g., for WMS 1.3) for the CRS.
     * \returns TRUE if CRS axis is inverted
     */
    bool hasAxisInverted() const;

    /**
     * Returns the units for the projection used by the CRS.
     */
    QgsUnitTypes::DistanceUnit mapUnits() const;

    /**
     * Returns the approximate bounds for the region the CRS is usable within.
     *
     * The returned bounds represent the latitude and longitude extent for the
     * projection in the WGS 84 CRS.
     *
     * \since QGIS 3.0
     */
    QgsRectangle bounds() const;

    // Mutators -----------------------------------

    /**
     * Updates the definition and parameters of the coordinate reference system to their
     * latest values.
     *
     * This only has an effect if the CRS is a user defined custom CRS, and the definition
     * of that custom CRS has changed. In this case the parameters of the object (such as the
     * proj and WKT string definitions, and other related properties) will be updated to
     * reflect the current definition of the custom CRS.
     *
     * Any objects which store CRS objects should connect to the QgsApplication::coordinateReferenceSystemRegistry()'s
     * QgsCoordinateReferenceSystemRegistry::userCrsChanged() signal and call this method
     * on their stored CRS objects whenever the signal is emitted in order to update these
     * CRSes to their new definitions.
     *
     * \since QGIS 3.18
     */
    void updateDefinition();

    /**
     * Set user hint for validation
     */
    void setValidationHint( const QString &html );

    /**
     * Gets user hint for validation
     */
    QString validationHint();

    /**
     * Update proj.4 parameters in our database from proj.4
     * \returns number of updated CRS on success and
     *   negative number of failed updates in case of errors.
     * \note This is used internally and should not be necessary to call in client code
     */
    static int syncDatabase();

    /**
     * Saves the CRS as a new custom ("USER") CRS.
     *
     * Returns the new CRS srsid(), or -1 if the CRS could not be saved.
     *
     * The \a nativeFormat argument specifies the format to use when saving the CRS
     * definition. FormatWkt is recommended as it is a lossless format.
     *
     * \warning Not all CRS definitions can be represented as a Proj string, so
     * take care when using the FormatProj option.
     *
     * \note Since QGIS 3.18, internally this calls QgsCoordinateReferenceSystemRegistry::addUserCrs().
     */
    long saveAsUserCrs( const QString &name, Qgis::CrsDefinitionFormat nativeFormat = Qgis::CrsDefinitionFormat::Wkt );

    /**
     * Sets the native \a format for the CRS definition.
     *
     * \note This has no effect on the underlying definition of the CRS, rather it controls what format
     * to use when displaying the CRS's definition to users.
     *
     * \see nativeFormat()
     * \since QGIS 3.24
     */
    void setNativeFormat( Qgis::CrsDefinitionFormat format );

    /**
     * Returns the native format for the CRS definition.
     *
     * \note This has no effect on the underlying definition of the CRS, rather it controls what format
     * to use when displaying the CRS's definition to users.
     *
     * \see setNativeFormat()
     * \since QGIS 3.24
     */
    Qgis::CrsDefinitionFormat nativeFormat() const;

    /**
     * Returns the geographic CRS associated with this CRS object.
     *
     * May return an invalid CRS if the geographic CRS could not be determined.
     *
     * \note This method will always return a longitude, latitude ordered CRS.
     *
     * \since QGIS 3.24
     */
    QgsCoordinateReferenceSystem toGeographicCrs() const;

    //! Returns auth id of related geographic CRS
    QString geographicCrsAuthId() const;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    const QString str = sipCpp->isValid() ? QStringLiteral( "<QgsCoordinateReferenceSystem: %1%2>" ).arg( !sipCpp->authid().isEmpty() ? sipCpp->authid() : sipCpp->toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED ),
                        std::isfinite( sipCpp->coordinateEpoch() ) ? QStringLiteral( " @ %1" ).arg( sipCpp->coordinateEpoch() ) : QString() )
                        : QStringLiteral( "<QgsCoordinateReferenceSystem: invalid>" );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

#ifndef SIP_RUN

    /**
     * Returns the underlying PROJ PJ object corresponding to the CRS, or NULLPTR
     * if the CRS is invalid.
     *
     * This object is only valid for the lifetime of the QgsCoordinateReferenceSystem.
     *
     * \note Not available in Python bindings.
     * \since QGIS 3.8
     */
    PJ *projObject() const;

    /**
     * Constructs a QgsCoordinateReferenceSystem from a PROJ PJ object.
     *
     * The \a object must correspond to a PROJ CRS object.
     *
     * Ownership of \a object is not transferred.
     *
     * \note Not available in Python bindings
     * \since QGIS 3.24
     */
    static QgsCoordinateReferenceSystem fromProjObject( PJ *object );

    /**
     * Sets this CRS by passing it a PROJ PJ \a object, corresponding to a PROJ CRS object.
     *
     * Ownership of \a object is not transferred.
     *
     * \note Not available in Python bindings
     * \since QGIS 3.24
     */
    bool createFromProjObject( PJ *object );
#endif

    /**
     * Returns a list of recently used projections
     * \returns list of srsid for recently used projections
     * \deprecated QGIS 3.10 Use recentCoordinateReferenceSystems() instead.
     */
    Q_DECL_DEPRECATED static QStringList recentProjections() SIP_DEPRECATED;

    /**
     * Returns a list of recently used CRS.
     * \since QGIS 3.10.3
    */
    static QList< QgsCoordinateReferenceSystem > recentCoordinateReferenceSystems();

    /**
     * Pushes a recently used CRS to the top of the recent CRS list.
     * \since QGIS 3.10.3
     */
    static void pushRecentCoordinateReferenceSystem( const QgsCoordinateReferenceSystem &crs );

#ifndef SIP_RUN

    /**
     * Clears the internal cache used to initialize QgsCoordinateReferenceSystem objects.
     * This should be called whenever the srs database has been modified in order to ensure
     * that outdated CRS objects are not created.
     *
     * If \a disableCache is TRUE then the inbuilt cache will be completely disabled. This
     * argument is for internal use only.
     *
     * \since QGIS 3.0
     */
    static void invalidateCache( bool disableCache = false );
#else

    /**
     * Clears the internal cache used to initialize QgsCoordinateReferenceSystem objects.
     * This should be called whenever the srs database has been modified in order to ensure
     * that outdated CRS objects are not created.
     *
     * \since QGIS 3.0
     */
    static void invalidateCache( bool disableCache SIP_PYARGREMOVE = false );
#endif

    // Mutators -----------------------------------
    // We don't want to expose these to the public api since they won't create
    // a fully valid crs. Programmers should use the createFrom* methods rather
  private:

    /**
     * A static helper function to find out the proj string for a srsid
     * \param srsId The srsid used for the lookup
     * \returns QString The proj string
     */
    static QString projFromSrsId( int srsId );

    /**
     * Set the Proj string.
     * \param projString Proj format specifies
     * (excluding proj and ellips) that define this CRS.
     */
    void setProjString( const QString &projString );

    /**
     * Set the WKT string
     */
    bool setWktString( const QString &wkt );

    /**
     * Print the description if debugging
     */
    void debugPrint();

    //! A string based associative array used for passing records around
    typedef QMap<QString, QString> RecordMap;

    /**
     * Gets a record from the srs.db or qgis.db backends, given an sql statement.
     * \param sql The sql query to execute
     * \returns An associative array of field name <-> value pairs
     * \note only handles queries that return a single record.
     * \note it will first try the system srs.db then the users qgis.db!
     */
    RecordMap getRecord( const QString &sql );

    /**
     * Open SQLite db and show message if cannot be opened
     * \returns the same code as sqlite3_open
     */
    static int openDatabase( const QString &path, sqlite3_database_unique_ptr &database, bool readonly = true );

    //! Work out the projection units and set the appropriate local variable
    void setMapUnits();

    //! Helper for getting number of user CRS already in db
    static long getRecordCount();

    bool loadFromAuthCode( const QString &auth, const QString &code );

    /**
     * Returns a list of all users SRS IDs present in the CRS database.
     */
    static QList< long > userSrsIds();

    /**
     * Tries to match the current definition of the CRS to user CRSes.
     *
     * Uses proj's equivalent testing API so that matches are tolerant to differences in
     * parameter order and naming for proj or WKT strings (internally, uses the PJ_COMP_EQUIVALENT
     * criteria).
     */
    long matchToUserCrs() const;

    /**
     * Initialize the CRS object by looking up CRS database in path given in db argument,
     * using first CRS entry where expression = 'value'
     */
    bool loadFromDatabase( const QString &db, const QString &expression, const QString &value );

    bool createFromWktInternal( const QString &wkt, const QString &description );

    QExplicitlySharedDataPointer<QgsCoordinateReferenceSystemPrivate> d;

    QString mValidationHint;

    Qgis::CrsDefinitionFormat mNativeFormat = Qgis::CrsDefinitionFormat::Wkt;

    friend class QgsProjContext;

    // Only meant to be called by QgsProjContext::~QgsProjContext()
    static void removeFromCacheObjectsBelongingToCurrentThread( PJ_CONTEXT *pj_context );

    //! Function for CRS validation. May be NULLPTR.
    static CUSTOM_CRS_VALIDATION sCustomSrsValidation;

    // cache

    static bool sDisableSrIdCache;
    static bool sDisableOgcCache;
    static bool sDisableProjCache;
    static bool sDisableWktCache;
    static bool sDisableSrsIdCache;
    static bool sDisableStringCache;

    // for tests
    static const QHash< QString, QgsCoordinateReferenceSystem > &stringCache();
    static const QHash< QString, QgsCoordinateReferenceSystem > &projCache();
    static const QHash< QString, QgsCoordinateReferenceSystem > &ogcCache();
    static const QHash< QString, QgsCoordinateReferenceSystem > &wktCache();
    static const QHash< long, QgsCoordinateReferenceSystem > &srsIdCache();
    static const QHash< long, QgsCoordinateReferenceSystem > &srIdCache();

    friend class TestQgsCoordinateReferenceSystem;
    friend class QgsCoordinateReferenceSystemRegistry;
    friend bool CORE_EXPORT operator> ( const QgsCoordinateReferenceSystem &c1, const QgsCoordinateReferenceSystem &c2 );
    friend bool CORE_EXPORT operator< ( const QgsCoordinateReferenceSystem &c1, const QgsCoordinateReferenceSystem &c2 );
    friend bool CORE_EXPORT operator>= ( const QgsCoordinateReferenceSystem &c1, const QgsCoordinateReferenceSystem &c2 );
    friend bool CORE_EXPORT operator<= ( const QgsCoordinateReferenceSystem &c1, const QgsCoordinateReferenceSystem &c2 );

    bool createFromPostgisSrid( const long id );
};

Q_DECLARE_METATYPE( QgsCoordinateReferenceSystem )

//! Output stream operator
#ifndef SIP_RUN
inline std::ostream &operator << ( std::ostream &os, const QgsCoordinateReferenceSystem &r )
{
  QString mySummary( QStringLiteral( "\n\tSpatial Reference System:" ) );
  mySummary += QLatin1String( "\n\t\tDescription : " );
  if ( !r.description().isNull() )
  {
    mySummary += r.description();
  }
  else
  {
    mySummary += QLatin1String( "Undefined" );
  }
  mySummary += QLatin1String( "\n\t\tProjection  : " );
  if ( !r.projectionAcronym().isNull() )
  {
    mySummary += r.projectionAcronym();
  }
  else
  {
    mySummary += QLatin1String( "Undefined" );
  }

  mySummary += QLatin1String( "\n\t\tEllipsoid   : " );
  if ( !r.ellipsoidAcronym().isNull() )
  {
    mySummary += r.ellipsoidAcronym();
  }
  else
  {
    mySummary += QLatin1String( "Undefined" );
  }

  mySummary += QLatin1String( "\n\t\tProjString  : " );
  if ( !r.toProj().isNull() )
  {
    mySummary += r.toProj();
  }
  else
  {
    mySummary += QLatin1String( "Undefined" );
  }
  // Using streams we need to use local 8 Bit
  return os << mySummary.toLocal8Bit().data() << std::endl;
}

bool CORE_EXPORT operator> ( const QgsCoordinateReferenceSystem &c1, const QgsCoordinateReferenceSystem &c2 );
bool CORE_EXPORT operator< ( const QgsCoordinateReferenceSystem &c1, const QgsCoordinateReferenceSystem &c2 );
bool CORE_EXPORT operator>= ( const QgsCoordinateReferenceSystem &c1, const QgsCoordinateReferenceSystem &c2 );
bool CORE_EXPORT operator<= ( const QgsCoordinateReferenceSystem &c1, const QgsCoordinateReferenceSystem &c2 );
#endif

#endif // QGSCOORDINATEREFERENCESYSTEM_H
