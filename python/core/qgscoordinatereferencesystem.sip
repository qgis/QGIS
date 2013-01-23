
/*!
 * \class QgsCoordinateReferenceSystem
 * \brief Class for storing a spatial reference system (CRS)
 */
class QgsCoordinateReferenceSystem
{
%TypeHeaderCode
#include <qgscoordinatereferencesystem.h>
%End
    public:

//   typedef void (*CUSTOM_CRS_VALIDATION)(QgsCoordinateReferenceSystem*);

    enum CrsType
    {
      InternalCrsId,
      PostgisCrsId,
      EpsgCrsId
    };

    //! Default constructor
    QgsCoordinateReferenceSystem();

    ~QgsCoordinateReferenceSystem();

    /*!
     * Constructs a CRS object from a string definition as defined in the createFromString
     * member function (by default a WKT definition).
     * @param theDefinition A String containing a coordinate reference system definition.
     */
    explicit QgsCoordinateReferenceSystem( QString theDefinition );

    /*! Use this constructor when you want to create a CRS object using
     *  a postgis SRID, an EpsgCrsId id or a QGIS CRS_ID.
     * @note We encourage you to use EpsgCrsId, Wkt or Proj4 to describe CRS's in your code
     * wherever possible. QGSI CRS_IDs are not guaranteed to be permanent / involatile.
     * @param theId The ID no valid for the chosen coordinate system id type
     * @param theType One of the types described in QgsCoordinateReferenceSystem::CrsType
     */
    QgsCoordinateReferenceSystem( const long theId, CrsType theType = PostgisCrsId );

    // Misc helper functions -----------------------

    bool createFromId( const long theId, CrsType theType = PostgisCrsId );

    /**
     * \brief Set up this CRS from the given OGC CRS
     *
     * Sets this CRS to the given OGC WMS-format Coordinate Reference Systems.
     *
     * \retval false if not given an valid label
     */
    bool createFromOgcWmsCrs( QString theCrs );

    /*! Set up this srs by fetching the appropriate information from the
     * sqlite backend. First the system level read only srs.db will be checked
     * and then the users ~/.qgis/qgis.db database will be checked for a match.
     * @note Any members will be overwritten during this process.
     * @param theSrid The postgis SRID for the desired spatial reference system.
     */
    bool createFromSrid( const long theSrid );

    /*! Set up this srs using a Wkt spatial ref sys definition.
     * The wkt will be converted to a proj4 string using OGR helper
     * functions. After this the srs databasses will be searched for matches.
     * First the system level read only srs.db will be checked
     * and then the users ~/.qgis/qgis.db database will be checked for a match.
     * @note Any members will be overwritten during this process.
     * @note SRID and EpsgCrsId may be blank if no match can be found on srs db.
     * @param theWkt The Wkt for the desired spatial reference system.
     * @return bool TRUE if success else false
     */
    bool createFromWkt( const QString theWkt );

    /*! Set up this srs by fetching the appropriate information from the
     * sqlite backend. If the srsid is < 100000, only the system srs.db
     * will be checked. If the srsid > 100000 the srs will be retrieved from
     * the ~/.qgis/qgis.db
     * @note Any members will be overwritten during this process.
     * @param theSrsId The QGIS SrsId for the desired spatial reference system.
     * @return bool TRUE if success else false
     */
    bool createFromSrsId( const long theSrsId );

    /*! Set up this srs by passing it a proj4 style formatted string.
     * The string will be parsed and the projection and ellipsoid
     * members set and the remainder of the proj4 string will be stored
     * in the parameters member. The reason for this is so that we
     * can easily present the user with 'natural language' representation
     * of the projection and ellipsoid by looking them up in the srs.bs sqlite
     * database. Also having the ellpse and proj elements stripped out
     * is helpful to speed up globbing queries (see below).
     *
     * We try to match the proj string to and srsid using the following logic:
     *
     * - perform a whole text search on srs name (if not null). The srs name will
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
     * @note If the srs was not matched, we will create a new entry on the users tbl_srs
     *    for this srs.
     *
     * @param theProjString A proj4 format string
     * @return bool TRUE if success else false
     */
    bool createFromProj4( const QString theProjString );

    /*! Set up this srs from a string definition, by default a WKT definition.  Otherwise
     * the string defines a authority, followed by a colon, followed by the definition.
     * The authority can be one of "epsg", "postgis", "internal" for integer definitions,
     * and "wkt" or "proj4" for string definitions.  The implementation of each authority
     * uses the corresponding createFrom... function.
     * @param theDefinition A String containing a coordinate reference system definition.
     */
    bool createFromString( const QString theDefinition );

    /*! Set up this srs from a various text formats.
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
     *
     * @note added in 1.8
     */
    bool createFromUserInput( const QString theDefinition );

    /*! Make sure that ESRI WKT import is done properly.
     * This is required for proper shapefile CRS import when using gdal>= 1.9.
     * @note This function is called by createFromUserInput() and QgsOgrProvider::crs(), there is usually
     * no need to call it from elsewhere.
     * @note This function sets CPL config option GDAL_FIX_ESRI_WKT to a proper value,
     * unless it has been set by the user through the commandline or an environment variable.
     * For more details refer to OGRSpatialReference::morphFromESRI() .
     *
     * @note added in 1.8
     */
    static void setupESRIWktFix();

    /*! Find out whether this CRS is correctly initialised and usable */
    bool isValid() const;

    /*! Perform some validation on this CRS. If the sts doesn't validate the
     * default behaviour settings for layers with unknown CRS will be
     * consulted and acted on accordingly. By hell or high water this
     * method will do its best to make sure that this CRS is valid - even
     * if that involves resorting to a hard coded default of geocs:wgs84.
     *
     * @note It is not usually necessary to use this function, unless you
     * are trying to force this srs to be valid.
     */
    void validate();

    /*! This is a globbing function to try to find a record in the database
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

    /*! Overloaded == operator used to compare to CRS's.
     *  Internally it will delegate to the equals method described below
     */
    bool operator==( const QgsCoordinateReferenceSystem &theSrs ) const;
    /*! Overloaded != operator used to compare to CRS's.
      *  Returns opposite bool value to operator ==
     */
    bool operator!=( const QgsCoordinateReferenceSystem &theSrs ) const;

    /*! Restores state from the given Dom node.
     * @param theNode The node from which state will be restored
     * @return bool True on success, False on failure
     */
    bool readXML( QDomNode & theNode );
    /*! Stores state to the given Dom node in the given document.
     * Below is an example of the generated tag.
     \verbatim
      <spatialrefsys>
          <proj4>+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs </proj4>
           <srsid>2585</srsid>
           <srid>4326</srid>
           <epsg>4326</epsg>
           <description>WGS 84</description>
           <projectionacronym>longlat</projectionacronym>
           <ellipsoidacronym>WGS84</ellipsoidacronym>
       </spatialrefsys>
     \endverbatim
     * @param theNode The node in which state will be restored
     * @param theDoc The document in which state will be stored
     * @return bool True on success, False on failure
     */
    bool writeXML( QDomNode & theNode, QDomDocument & theDoc ) const;


    /** Sets custom function to force valid CRS
     *  QGIS uses implementation in QgisGui::customSrsValidation
     */
    // static void setCustomSrsValidation( CUSTOM_CRS_VALIDATION f );

    /** Gets custom function
     * @note not available in python bindings
     */
    // static CUSTOM_CRS_VALIDATION customSrsValidation();

    // Accessors -----------------------------------

    /*! Get the SrsId - if possible
    *  @return  long theSrsId The internal sqlite3 srs.db primary key for this srs
    */
    long srsid() const;

    /*! Get the postgis srid for this srs
     * @return  long theSRID the Postgis spatial_ref_sys identifier for this srs (defaults to 0)
     */
    long postgisSrid() const;

    /*! Get the authority identifier for this srs
     * @return  QString the Authority identifier for this srs
     * @note added in 1.5
     */
    QString authid() const;

    /*! Get the Description
     * @return  QString the Description A textual description of the srs.
     * @note A zero length string will be returned if the description is uninitialised
     */
    QString description() const;

    /*! Get the Projection Acronym
     * @return  QString theProjectionAcronym The official proj4 acronym for the projection family
     * @note A zero length string will be returned if the projectionAcronym is uninitialised
     */
    QString projectionAcronym() const;

    /*! Get the Ellipsoid Acronym
     * @return  QString theEllipsoidAcronym The official proj4 acronym for the ellipoid
     * @note A zero length string will be returned if the ellipsoidAcronym is uninitialised
     */
    QString ellipsoidAcronym() const;

    /*! A helper to get an wkt representation of this srs
     * @return string containing Wkt of the srs
     */
    QString toWkt() const;

    /** Get the Proj Proj4 string representation of this srs.
     * If proj and ellps keys are found in the parameters,
     * they will be stripped out and the Projection and ellipsoid acronyms will be
     * overridden with these.
     * @return  QString theProj4String Proj4 format specifies that define this srs.
     * @note A zero length string will be returned if the toProj4 is uninitialised
     */
    QString toProj4() const;

    /*! Get this Geographic? flag
     * @return  bool theGeoFlag Whether this is a geographic or projected coordinate system
     */
    bool geographicFlag() const;

    /*! return if axis is inverted (eg. for WMS 1.3)
     * @return  bool Whether this is crs axis is inverted
     * @note added in 1.8
     */
    bool axisInverted() const;

    /*! Get the units that the projection is in
     * @return QGis::UnitType that gives the units for the coordinate system
     */
    QGis::UnitType mapUnits() const;


    // Mutators -----------------------------------
    /*! Set user hint for validation
     */
    void setValidationHint( QString html );

    /*! Get user hint for validation
     */
    QString validationHint();
    /*! Update proj.4 parameters in our database from proj.4
     * @returns number of updated CRS on success and
     *   negative number of failed updates in case of errors.
     * @note added in 1.8
     */
    static int syncDb();
};
