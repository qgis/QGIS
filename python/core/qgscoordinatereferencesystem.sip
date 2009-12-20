
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

//      typedef void (*CUSTOM_CRS_VALIDATION)(QgsCoordinateReferenceSystem*);
      
        enum CrsType {
          InternalCrsId,
          PostgisCrsId,
          EpsgCrsId
        };
        
        //! Default constructor
        QgsCoordinateReferenceSystem();

        ~QgsCoordinateReferenceSystem();
        
        /*! 
         * Constructs a CRS object from a Wkt string
         * @param theWkt A String containing a valid Wkt def
         */
        explicit QgsCoordinateReferenceSystem(QString theWkt);

        /*! Use this constructor when you want to create a CRS object using 
         *  a postgis SRID, an Epsg Id id or a QGIS CRS_ID.
         * @param theId The ID no valid for the chosen coordinate system id type
         * @param theType One of the types described in QgsCoordinateReferenceSystem::CrsType
         */
        QgsCoordinateReferenceSystem(const long theId, CrsType theType=PostgisCrsId);

        // Misc helper functions -----------------------

        void createFromId(const long theId, CrsType theType=PostgisCrsId);

        /**
         * \brief Set up this CRS from the given OGC CRS
         *
         * Sets this CRS to the given OGC WMS-format Coordinate Reference Systems.
         *
         * \note This function only deals with Epsg Id labels only at this time.
         *
         * \retval FALSE if not given an Epsg Id label
         */
        bool createFromOgcWmsCrs(QString theCrs);

        /*! Set up this srs by fetching the appropriate information from the 
         * sqlite backend. First the system level read only srs.db will be checked
         * and then the users ~/.qgis/qgis.db database will be checked for a match.
         * @note Any members will be overwritten during this process.
         * @param theSrid The postgis SRID for the desired spatial reference system.
         */
        bool createFromSrid(const long theSrid);

        /*! Set up this srs using a Wkt spatial ref sys definition. 
         * The wkt will be converted to a proj4 string using OGR helper
         * functions. After this the srs databasses will be searched for matches.
         * First the system level read only srs.db will be checked
         * and then the users ~/.qgis/qgis.db database will be checked for a match.
         * @note Any members will be overwritten during this process.
         * @note SRID and EpsgCrsId may be blank if no match can be found on srs db.
         * @param theWkt The Wkt for the desired spatial reference system.
         * @return bool TRUE if sucess else false
         */
        bool createFromWkt(const QString theWkt);

        /*! Set up this srs by fetching the appropriate information from the 
         * sqlite backend. First the system level read only srs.db will be checked
         * and then the users ~/.qgis/qgis.db database will be checked for a match.
         * @note Any members will be overwritten during this process.
         * @param theEpsg The EpsgCrsId for the desired spatial reference system.
         * @return bool TRUE if sucess else false
         */
        bool createFromEpsg(const long theEpsg);

        /*! Set up this srs by fetching the appropriate information from the 
         * sqlite backend. If the srsid is < 100000, only the system srs.db 
         * will be checked. If the srsid > 100000 the srs will be retrieved from 
         * the ~/.qgis/qgis.db
         * @note Any members will be overwritten during this process.
         * @param theSrsId The QGIS SrsId for the desired spatial reference system.
         * @return bool TRUE if sucess else false
         */
        bool createFromSrsId (const long theSrsId);

        /*! Set up this srs by passing it a proj4 style formatted string.
         * The string will be parsed and the projection and ellipsoid
         * members set and the remainder of the proj4 string will be stored
         * in the parameters member. The reason for this is so that we
         * can easily present the user with 'natural language' representation
         * of the projection and ellipsoid by looking them up in the srs.bs sqlite 
         * database. Also having the ellpse and proj elements stripped out
         * is hepful to speed up globbing queries (see below).
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
         *   that it doesnt munch yer cpu so much.
         *
         * @note If the srs was not matched, we will create a new entry on the users tbl_srs 
         *    for this srs.
         *
         * @param theProjString A proj4 format string
         * @return bool TRUE if sucess else false
         */
        bool createFromProj4 (const QString theProjString);

        /*! Find out whether this CRS is correctly initialised and usable */
        bool isValid() const;
        /*! Perform some validation on this CRS. If the sts doesnt validate the
         *  default behaviour settings for layers with unknown CRS will be 
         * consulted and acted on accordingly. By hell or high water this
         * method will do its best to make sure that this CRS is valid - even
         * if that involves resorting to a hard coded default of geocs:wgs84.
         *
         * @note It is not usually necessary to use this function, unless you
         * are trying to force theis srs to be valid.
         */
        void validate();

        /*! This is a globbing function to try to find a record in the database
         *  that matches a CRS defined only by a proj4string. The goal is to 
         *  learn what the tbl_srs.srs_id value is for the CRS. Internally 
         *  the source CRS is converted to and OGR srs object using the proj4string
         *  and then every record in the database that matches projection and ellipsoid
         *  will be converted to an OGR srs in turn and compared to the source CRS.
         *  There are some gotchas with using ogr isSame() srs comparison, but
         *  its more effective than using straight string comparison of proj4params.
         *  @note The ellipsoid and projection acronyms must be set as well as the proj4string!
         *  @return lomg the SrsId of the matched CRS
         */
        long findMatchingProj();
         
        /*! Overloaded == operator used to compare to CRS's.
         *  Internally it will delegate to the equals method described below
         */
         bool operator==(const QgsCoordinateReferenceSystem &theSrs);
        /*! Overloaded != operator used to compare to CRS's.
          *  Returns opposite bool value to operator ==
         */
         bool operator!=(const QgsCoordinateReferenceSystem &theSrs);
        /*! Overloaded == operator used to compare to CRS's.
         *  Internally it will use OGR isSameCRS() or isSameGeoCRS() methods as appropriate.
         *  Additionally logic may also be applied if the result from the OGR methods
         *  is inconclusive.
         */
         bool equals(QString theProj4String);

         /*! Restores state from the given Dom node.
         * @param theNode The node from which state will be restored
         * @return bool True on success, False on failure
         */
          bool readXML( QDomNode & theNode );
        /*! Stores state to the given Dom node in the given document.
         * Below is an example of the generated tag.
         *  <spatialrefsys>
         *      <proj4>+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs </proj4>
         *       <srsid>2585</srsid>
         *       <srid>4326</srid>
         *       <epsg>4326</epsg>
         *       <description>WGS 84</description>
         *       <projectionacronym>longlat</projectionacronym>
         *       <ellipsoidacronym>WGS84</ellipsoidacronym>
         *   </spatialrefsys>
         * @param theNode The node in which state will be restored
         * @param theDom The document in which state will be stored
         * @return bool True on success, False on failure
         */
          bool writeXML( QDomNode & theNode, QDomDocument & theDoc );

         /** Sets custom function to force valid CRS
          *  QGIS uses implementation in QgisGui::customSrsValidation
          */
         // TODO: disabled for now, needs usage of %Method directive to pass the function
         //static void setCustomSrsValidation(CUSTOM_CRS_VALIDATION f);
         
        // Accessors -----------------------------------

         /*! Get the SrsId - if possible
         *  @return  long theSrsId The internal sqlite3 srs.db primary key for this srs 
         */
        long srsid() const;
        /*! Get the postgis srid for this srs
         * @return  long theSRID the Postgis spatial_ref_sys identifier for this srs (defaults to 0)
         */
        long postgisSrid () const;
        /*! Get the EpsgCrsId identifier for this srs
         * @return  long theEpsg the ESPG identifier for this srs (defaults to 0)
         */
        long epsg () const;
        /*! Get the Description
         * @return  QString the Description A textual description of the srs.
         * @note A zero length string will be returned if the description is uninitialised
         */
        QString description () const;
        /*! Get the Projection Acronym
         * @return  QString theProjectionAcronym The official proj4 acronym for the projection family
         * @note A zero length string will be returned if the projectionAcronym is uninitialised
         */
        QString projectionAcronym() const;
        /*! Get the Ellipsoid Acronym
         * @return  QString theEllipsoidAcronym The official proj4 acronym for the ellipoid
         * @note A zero length string will be returned if the ellipsoidAcronym is uninitialised
         */
        QString ellipsoidAcronym () const;
         /*! A helper to get an wkt representation of this srs
          * @return string containing Wkt of the srs
          */
         QString toWkt() const;
        /** Get the Proj Proj4String. If proj and ellps keys are found in the parameters,
         * they will be stripped out and the Projection and ellipsoid acronyms will be
         * overridden with these.
         * @return  QString theProj4String Proj4 format specifies that define this srs.
         * @note A zero length string will be returned if the toProj4 is uninitialised
         */
        QString toProj4() const;
        /*! Get this Geographic? flag
         * @return  bool theGeoFlag Whether this is a geographic or projected coordinate system
         */
        bool geographicFlag () const;
        /*! Get the units that the projection is in
         * @return QGis::UnitType that gives the units for the coordinate system
         */
        QGis::UnitType mapUnits() const;


};


