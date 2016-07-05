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
#include <ostream>

//qt includes
#include <QString>
#include <QMap>
#include <QHash>

class QDomNode;
class QDomDocument;
class QgsCoordinateReferenceSystemPrivate;

// forward declaration for sqlite3
typedef struct sqlite3 sqlite3;

//qgis includes
#include "qgis.h"

#ifdef DEBUG
typedef struct OGRSpatialReferenceHS *OGRSpatialReferenceH;
#else
typedef void *OGRSpatialReferenceH;
#endif

class QgsCoordinateReferenceSystem;
typedef void ( *CUSTOM_CRS_VALIDATION )( QgsCoordinateReferenceSystem& );

/** \ingroup core
 * Class for storing a coordinate reference system (CRS)
 * \note Since QGIS 2.16 QgsCoordinateReferenceSystem objects are implicitly shared.
 */
class CORE_EXPORT QgsCoordinateReferenceSystem
{
  public:

    enum CrsType
    {
      InternalCrsId,
      PostgisCrsId,
      EpsgCrsId     // deprecated
    };

    QgsCoordinateReferenceSystem();

    ~QgsCoordinateReferenceSystem();

    /*!
     * Constructs a CRS object from a string definition as defined in the createFromString
     * member function (by default a WKT definition).
     * @param theDefinition A String containing a coordinate reference system definition.
     */
    explicit QgsCoordinateReferenceSystem( const QString& theDefinition );

    /** Use this constructor when you want to create a CRS object using
     *  a postgis SRID, an EpsgCrsId id or a QGIS CRS_ID.
     * @note We encourage you to use EpsgCrsId, WKT or Proj4 to describe CRS's in your code
     * wherever possible. QGIS CRS_IDs are not guaranteed to be permanent / involatile.
     * @param theId The ID no valid for the chosen coordinate system id type
     * @param theType One of the types described in QgsCoordinateReferenceSystem::CrsType
     */
    QgsCoordinateReferenceSystem( const long theId, CrsType theType = PostgisCrsId );

    //! Copy constructor
    QgsCoordinateReferenceSystem( const QgsCoordinateReferenceSystem& srs );

    //! Assignment operator
    QgsCoordinateReferenceSystem& operator=( const QgsCoordinateReferenceSystem& srs );

    // Misc helper functions -----------------------

    bool createFromId( const long theId, CrsType theType = PostgisCrsId );

    /**
     * Sets this CRS to the given OGC WMS-format Coordinate Reference Systems.
     * @returns false if not given an valid label
     * @note this method is expensive. Consider using QgsCRSCache::crsByOgcWmsCrs() instead.
     */
    bool createFromOgcWmsCrs( const QString& theCrs );

    /** Set up this CRS by fetching the appropriate information from the
     * sqlite backend. First the system level read only srs.db will be checked
     * and then the users ~/.qgis/qgis.db database will be checked for a match.
     * @note Any members will be overwritten during this process.
     * @param theSrid The postgis SRID for the desired spatial reference system.
     */
    bool createFromSrid( const long theSrid );

    /** Set up this CRS using a WKT spatial ref sys definition.
     * The WKT will be converted to a proj4 string using OGR helper
     * functions. After this the SRS databases will be searched for matches.
     * First the system level read only srs.db will be checked
     * and then the users ~/.qgis/qgis.db database will be checked for a match.
     * @note Any members will be overwritten during this process.
     * @note SRID and EpsgCrsId may be blank if no match can be found on SRS db.
     * @param theWkt The WKT for the desired spatial reference system.
     * @return bool TRUE if success else false
     * @note this method is expensive. Consider using QgsCRSCache::crsByWkt() instead.
     */
    bool createFromWkt( const QString &theWkt );

    /** Set up this CRS by fetching the appropriate information from the
     * sqlite backend. If the srsid is < 100000, only the system srs.db
     * will be checked. If the srsid > 100000 the srs will be retrieved from
     * the ~/.qgis/qgis.db
     * @note Any members will be overwritten during this process.
     * @param theSrsId The QGIS SrsId for the desired spatial reference system.
     * @return bool TRUE if success else false
     * @note this method is expensive. Consider using QgsCRSCache::crsBySrsId() instead.
     */
    bool createFromSrsId( const long theSrsId );

    /** Set up this CRS by passing it a proj4 style formatted string.
     * The string will be parsed and the projection and ellipsoid
     * members set and the remainder of the proj4 string will be stored
     * in the parameters member. The reason for this is so that we
     * can easily present the user with 'natural language' representation
     * of the projection and ellipsoid by looking them up in the srs.bs sqlite
     * database. Also having the ellipse and proj elements stripped out
     * is helpful to speed up globbing queries (see below).
     *
     * We try to match the proj string to and srsid using the following logic:
     *
     * - perform a whole text search on CRS name (if not null). The CRS name will
     *   have been set if this method has been delegated to from createFromWkt.
     * - if the above does not match perform a whole text search on proj4 string (if not null)
     * - if none of the above match convert the proj4 string to an OGR CRS
     *   then check if its a geocs or a proj cs (using ogr isGeographic)
     *   then sequentially walk through the database (first users qgis.db srs tbl then
     *   system srs.db tbl), converting each entry into an ogr srs and using isSame
     *   or isSameGeocs (essentially calling the == overloaded operator). We'll try to
     *   be smart about this and first parse out the proj and ellpse strings and only
     *   check for a match in entities that have the same ellps and proj entries so
     *   that it doesn't munch yer cpu so much.
     *
     * @param theProjString A proj4 format string
     * @return bool TRUE if success else false
     * @note this method is expensive. Consider using QgsCRSCache::crsByProj4() instead.
     */
    bool createFromProj4( const QString &theProjString );

    /** Set up this CRS from a string definition, by default a WKT definition.  Otherwise
     * the string defines a authority, followed by a colon, followed by the definition.
     * The authority can be one of "epsg", "postgis", "internal" for integer definitions,
     * and "wkt" or "proj4" for string definitions.  The implementation of each authority
     * uses the corresponding createFrom... function.
     * @param theDefinition A String containing a coordinate reference system definition.
     */
    bool createFromString( const QString &theDefinition );

    /** Set up this CRS from a various text formats.
     *
     * Valid formats: WKT string, "EPSG:n", "EPSGA:n", "AUTO:proj_id,unit_id,lon0,lat0",
     * "urn:ogc:def:crs:EPSG::n", PROJ.4 string, filename (with WKT, XML or PROJ.4 string),
     * well known name (such as NAD27, NAD83, WGS84 or WGS72),
     * ESRI::[WKT string] (directly or in a file), "IGNF:xxx"
     *
     * For more details on supported formats see OGRSpatialReference::SetFromUserInput()
     * ( http://www.gdal.org/ogr/classOGRSpatialReference.html#aec3c6a49533fe457ddc763d699ff8796 )
     * @note this function generates a WKT string using OSRSetFromUserInput() and
     * passes it to createFromWkt() function.
     * @param theDefinition A String containing a coordinate reference system definition.
     */
    bool createFromUserInput( const QString &theDefinition );

    /** Make sure that ESRI WKT import is done properly.
     * This is required for proper shapefile CRS import when using gdal>= 1.9.
     * @note This function is called by createFromUserInput() and QgsOgrProvider::crs(), there is usually
     * no need to call it from elsewhere.
     * @note This function sets CPL config option GDAL_FIX_ESRI_WKT to a proper value,
     * unless it has been set by the user through the commandline or an environment variable.
     * For more details refer to OGRSpatialReference::morphFromESRI() .
     */
    static void setupESRIWktFix();

    /** Returns whether this CRS is correctly initialized and usable */
    bool isValid() const;

    /** Perform some validation on this CRS. If the sts doesn't validate the
     * default behaviour settings for layers with unknown CRS will be
     * consulted and acted on accordingly. By hell or high water this
     * method will do its best to make sure that this CRS is valid - even
     * if that involves resorting to a hard coded default of geocs:wgs84.
     *
     * @note It is not usually necessary to use this function, unless you
     * are trying to force this CRS to be valid.
     */
    void validate();

    /** This is a globbing function to try to find a record in the database
     *  that matches a CRS defined only by a proj4string. The goal is to
     *  learn what the tbl_srs.srs_id value is for the CRS. Internally
     *  the source CRS is converted to an OGR srs object using the proj4string call
     *  and then every record in the database that matches projection and ellipsoid
     *  will be converted to an OGR srs in turn and compared to the source CRS.
     *  There are some gotchas with using ogr isSame() srs comparison, but
     *  its more effective than using straight string comparison of proj4params.
     *  @note The ellipsoid and projection acronyms must be set as well as the proj4string!
     *  @return long the SrsId of the matched CRS
     */
    long findMatchingProj();

    /** Overloaded == operator used to compare to CRS's.
     *  Internally it will delegate to the equals method described below
     */
    bool operator==( const QgsCoordinateReferenceSystem &theSrs ) const;
    /** Overloaded != operator used to compare to CRS's.
      *  Returns opposite bool value to operator ==
     */
    bool operator!=( const QgsCoordinateReferenceSystem &theSrs ) const;

    /** Restores state from the given Dom node.
     * @param theNode The node from which state will be restored
     * @return bool True on success, False on failure
     */
    bool readXML( const QDomNode & theNode );
    /** Stores state to the given Dom node in the given document.
     * Below is an example of the generated tag.
     \code{.xml}
      <spatialrefsys>
          <proj4>+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs </proj4>
           <srsid>2585</srsid>
           <srid>4326</srid>
           <epsg>4326</epsg>
           <description>WGS 84</description>
           <projectionacronym>longlat</projectionacronym>
           <ellipsoidacronym>WGS84</ellipsoidacronym>
       </spatialrefsys>
     \endcode
     * @param theNode The node in which state will be restored
     * @param theDoc The document in which state will be stored
     * @return bool True on success, False on failure
     */
    bool writeXML( QDomNode & theNode, QDomDocument & theDoc ) const;


    /** Sets custom function to force valid CRS
     *  QGIS uses implementation in QgisGui::customSrsValidation
     * @note not available in python bindings
     */
    static void setCustomSrsValidation( CUSTOM_CRS_VALIDATION f );

    /** Gets custom function
     * @note not available in python bindings
     */
    static CUSTOM_CRS_VALIDATION customSrsValidation();

    // Accessors -----------------------------------

    /** Returns the SrsId, if available.
     *  @return the internal sqlite3 srs.db primary key for this srs
     * @see postgisSrid()
     */
    long srsid() const;

    /** Returns PostGIS SRID for the CRS.
     * @return the PostGIS spatial_ref_sys identifier for this CRS (defaults to 0)
     */
    long postgisSrid() const;

    /** Returns the authority identifier for the CRS, which includes both the authority (eg EPSG)
     * and the CRS number (eg 4326). This is the best method to use when showing a very short CRS
     * identifier to a user, eg "EPSG:4326".
     * @returns the authority identifier for this CRS
     * @see description()
     */
    QString authid() const;

    /** Returns the descriptive name of the CRS, eg "WGS 84" or "GDA 94 / Vicgrid94". In most
     * cases this is the best method to use when showing a friendly identifier for the CRS to a
     * user.
     * @returns descriptive name of the CRS
     * @note an empty string will be returned if the description is not available for the CRS
     * @see authid()
     */
    QString description() const;

    /** Returns the projection acronym for the projection used by the CRS.
     * @returns the official proj4 acronym for the projection family
     * @note an empty string will be returned if the projectionAcronym is not available for the CRS
     * @see ellipsoidAcronym()
     */
    QString projectionAcronym() const;

    /** Returns the ellipsoid acronym for the ellipsoid used by the CRS.
     * @returns the official proj4 acronym for the ellipoid
     * @note an empty string will be returned if the ellipsoidAcronym is not available for the CRS
     * @see projectionAcronym()
     */
    QString ellipsoidAcronym() const;

    /** Returns a WKT representation of this CRS.
     * @return string containing WKT of the CRS
     * @see toProj4()
     */
    QString toWkt() const;

    /** Returns a Proj4 string representation of this CRS.
     * If proj and ellps keys are found in the parameters,
     * they will be stripped out and the projection and ellipsoid acronyms will be
     * overridden with these.
     * @return Proj4 format string that defines this CRS.
     * @note an empty string will be returned if the CRS could not be represented by a Proj4 string
     * @see toWkt()
     */
    QString toProj4() const;

    /** Returns whether the CRS is a geographic CRS.
     * @returns true if CRS is geographic, or false if it is a projected CRS
     */
    //TODO QGIS 3.0 - rename to isGeographic
    bool geographicFlag() const;

    /** Returns whether axis is inverted (eg. for WMS 1.3) for the CRS.
     * @returns true if CRS axis is inverted
     */
    bool axisInverted() const;

    /** Returns the units for the projection used by the CRS.
     */
    QGis::UnitType mapUnits() const;


    // Mutators -----------------------------------
    /** Set user hint for validation
     */
    void setValidationHint( const QString& html );

    /** Get user hint for validation
     */
    QString validationHint();
    /** Update proj.4 parameters in our database from proj.4
     * @returns number of updated CRS on success and
     *   negative number of failed updates in case of errors.
     */
    static int syncDb();


    /** Save the proj4-string as a custom CRS
     * @returns bool true if success else false
     */
    bool saveAsUserCRS( const QString& name );

    /** Returns auth id of related geographic CRS*/
    QString geographicCRSAuthId() const;

    /** Returns a list of recently used projections
     * @returns list of srsid for recently used projections
     * @note added in QGIS 2.7
     */
    static QStringList recentProjections();

    // Mutators -----------------------------------
    // We don't want to expose these to the public api since they wont create
    // a fully valid crs. Programmers should use the createFrom* methods rather
  private:
    /** A static helper function to find out the proj4 string for a srsid
     * @param theSrsId The srsid used for the lookup
     * @return QString The proj4 string
     */
    static QString proj4FromSrsId( const int theSrsId );

    /** Set the QGIS  SrsId
     *  @param theSrsId The internal sqlite3 srs.db primary key for this CRS
     */
    void setInternalId( long theSrsId );
    /** Set the postgis srid
     *  @param theSrid The postgis spatial_ref_sys key for this CRS
     */
    void setSrid( long theSrid );
    /** Set the Description
     * @param theDescription A textual description of the CRS.
     */
    void setDescription( const QString& theDescription );

    /** Set the Proj Proj4String.
     * @param theProj4String Proj4 format specifies
     * (excluding proj and ellips) that define this CRS.
     * @note some content of the PROJ4 string may be stripped off by this
     * method due to the parsing of the string by OSRNewSpatialReference .
     * For example input:
     * +proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs
     * Gets stored in the CRS as:
     * +proj=longlat +datum=WGS84 +no_defs
     */
    void setProj4String( const QString& theProj4String );

    /** Set this Geographic? flag
     * @param theGeoFlag Whether this is a geographic or projected coordinate system
     */
    void setGeographicFlag( bool theGeoFlag );

    /** Set the EpsgCrsId identifier for this CRS
     * @param theEpsg the ESPG identifier for this CRS (defaults to 0)
     */
    void setEpsg( long theEpsg );

    /** Set the authority identifier for this CRS
     * @param theID the authority identifier for this CRS (defaults to 0)
     */
    void setAuthId( const QString& theID );
    /** Set the projection acronym
     * @param theProjectionAcronym the acronym (must be a valid proj4 projection acronym)
     */
    void setProjectionAcronym( const QString& theProjectionAcronym );
    /** Set the ellipsoid acronym
     * @param theEllipsoidAcronym the acronym (must be a valid proj4 ellipsoid acronym)
     */
    void setEllipsoidAcronym( const QString& theEllipsoidAcronym );

    /** Print the description if debugging
     */
    void debugPrint();

    /** A string based associative array used for passing records around */
    typedef QMap<QString, QString> RecordMap;
    /** Get a record from the srs.db or qgis.db backends, given an sql statment.
     * @note only handles queries that return a single record.
     * @note it will first try the system srs.db then the users qgis.db!
     * @param theSql The sql query to execute
     * @return An associative array of field name <-> value pairs
     */
    RecordMap getRecord( const QString& theSql );

    // Open SQLite db and show message if cannot be opened
    // returns the same code as sqlite3_open
    static int openDb( const QString& path, sqlite3 **db, bool readonly = true );

    //! Work out the projection units and set the appropriate local variable
    void setMapUnits();

    //! Helper for getting number of user CRS already in db
    long getRecordCount();

    //! Helper for sql-safe value quoting
    static QString quotedValue( QString value );

    bool loadFromDb( const QString& db, const QString& expression, const QString& value );

    static bool loadIDs( QHash<int, QString> &wkts );
    static bool loadWkts( QHash<int, QString> &wkts, const char *filename );
    static bool syncDatumTransform( const QString& dbPath );

    QExplicitlySharedDataPointer<QgsCoordinateReferenceSystemPrivate> d;

    static CUSTOM_CRS_VALIDATION mCustomSrsValidation;
};


//! Output stream operator
inline std::ostream& operator << ( std::ostream& os, const QgsCoordinateReferenceSystem &r )
{
  QString mySummary( "\n\tSpatial Reference System:" );
  mySummary += "\n\t\tDescription : ";
  if ( !r.description().isNull() )
  {
    mySummary += r.description();
  }
  else
  {
    mySummary += "Undefined";
  }
  mySummary += "\n\t\tProjection  : ";
  if ( !r.projectionAcronym().isNull() )
  {
    mySummary += r.projectionAcronym();
  }
  else
  {
    mySummary += "Undefined";
  }

  mySummary += "\n\t\tEllipsoid   : ";
  if ( !r.ellipsoidAcronym().isNull() )
  {
    mySummary += r.ellipsoidAcronym();
  }
  else
  {
    mySummary += "Undefined";
  }

  mySummary += "\n\t\tProj4String  : ";
  if ( !r.toProj4().isNull() )
  {
    mySummary += r.toProj4();
  }
  else
  {
    mySummary += "Undefined";
  }
  // Using streams we need to use local 8 Bit
  return os << mySummary.toLocal8Bit().data() << std::endl;
}

#endif // QGSCOORDINATEREFERENCESYSTEM_H
