#ifndef QGSSPATIALREFSYS_H
#define QGSSPATIALREFSYS_H

//Standard includes
#include <ostream>
#include <istream>

//qt includes
#include <qstring.h>
#include <qstringlist.h>
#include <qregexp.h>
#include <qmap.h>
class QDomNode;
class QDomDocument;

//qgis includes
#include <qgis.h>

//gdal and ogr includes
#include <ogr_api.h>
#include <ogr_spatialref.h>
#include <cpl_error.h>

/*!
 * \class QgsSpatialRefSys
 * \brief Class for storing a spatial reference system (SRS)
 */ 
class QgsSpatialRefSys
{
    public:
        //! Default constructor

        QgsSpatialRefSys();
        /*! 
         * Constructs a SRS object from a WKT string
         * @param theWkt A String containing a valid Wkt def
         */
        explicit QgsSpatialRefSys(QString theWkt);

        /*!
         * Constructs a SRS object from the following component parts
         *
         * @param  long theSrsId The internal sqlite3 srs.db primary key for this srs 
         * @param  QString the Description A textual description of the srs.
         * @param  QString theProjectionAcronym The official proj4 acronym for the projection family
         * @param  QString theEllipsoidAcronym The official proj4 acronym for the ellipoid
         * @param  QString theProj4String Proj4 format specifies (excluding proj and ellips) that define this srs.
         * @param  bool theGeoFlag Whether this is a geographic or projected coordinate system
         * @param  long theSRID If available, the Postgis spatial_ref_sys identifier for this srs (defaults to 0)
         * @param  long theEpsg If available the ESPG identifier for this srs (defaults to 0)
         *
         * @note THIS CTOR WILL PROABBLY BE REMOVED!!!!!!!!!!!!!!!!
         */
        QgsSpatialRefSys(long theSrsId, 
                QString theDescription, 
                QString theProjectionAcronym, 
                QString theEllipsoidAcronym, 
                QString theProj4String,
                long theSRID,
                long theEpsg,
                bool theGeoFlag);

         enum SRS_TYPE {QGIS_SRSID,POSTGIS_SRID, EPSG};
         /*! Use this constructor when you want to create a SRS object using 
          *  a postgis SRID, an EPSG id or a QGIS SRS_ID.
          * @param theId The ID no valid for the chosen coordinate system id type
          * @param theType One of the types described in QgsSpatialRefSys::SRS_TYPE
          */
         QgsSpatialRefSys(const long theId, SRS_TYPE theType=POSTGIS_SRID);

         // Assignment operator
         QgsSpatialRefSys& operator=(const QgsSpatialRefSys& srs);

        // Misc helper functions -----------------------

         void createFromId(const long theId, SRS_TYPE theType=POSTGIS_SRID);
        
        /*! Set up this srs by fetching the appropriate information from the 
         * sqlite backend. First the system level read only srs.db will be checked
         * and then the users ~/.qgis/qgis.db database will be checked for a match.
         * @note Any members will be overwritten during this process.
         * @param theSrid The postgis SRID for the desired spatial reference system.
         */
        bool createFromSrid(const long theSrid);
        /*! Set up this srs using a WKT spatial ref sys definition. 
         * The wkt will be converted to a proj4 string using OGR helper
         * functions. After this the srs databasses will be searched for matches.
         * First the system level read only srs.db will be checked
         * and then the users ~/.qgis/qgis.db database will be checked for a match.
         * @note Any members will be overwritten during this process.
         * @note SRID and EPSG may be blank if no match can be found on srs db.
         * @param theWkt The WKT for the desired spatial reference system.
         * @return bool TRUE if sucess else false
         */
        bool createFromWkt(const QString theWkt);
        /*! Set up this srs by fetching the appropriate information from the 
         * sqlite backend. First the system level read only srs.db will be checked
         * and then the users ~/.qgis/qgis.db database will be checked for a match.
         * @note Any members will be overwritten during this process.
         * @param theEpsg The EPSG for the desired spatial reference system.
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
         * - if none of the above match convert the proj4 string to an OGR SRS
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

        /*! Find out whether this SRS is correctly initialised and useable */
        bool isValid() const;
        /*! Perform some validation on this SRS. If the sts doesnt validate the
         *  default behaviour settings for layers with unknown SRS will be 
         * consulted and acted on accordingly. By hell or high water this
         * method will do its best to make sure that this SRS is valid - even
         * if that involves resorting to a hard coded default of geocs:wgs84.
         *
         * @note It is not usually neccessary to use this function, unless you
         * are trying to force theis srs to be valid.
         */
        void validate();

        /*! This is a globbing function to try to find a record in the database
         *  that matches a SRS defined only by a proj4string. The goal is to 
         *  learn what the tbl_srs.srs_id value is for the SRS. Internally 
         *  the source SRS is converted to and OGR srs object using the proj4string
         *  and then every record in the database that matches projection and ellipsoid
         *  will be converted to an OGR srs in turn and compared to the source SRS.
         *  There are some gotchas with using ogr isSame() srs comparison, but
         *  its more effective than using straight string comparison of proj4params.
         *  @note The ellipsoid and projection acronyms must be set as well as the proj4string!
         *  @return lomg the SrsId of the matched SRS
         */
        long findMatchingProj();
         
        /*! A string based associative array used for passing records around */
        typedef QMap<QString, QString> RecordMap;
        /*! Get a record from the srs.db or qgis.db backends, given an sql statment.
         * @note only handles queries that return a single record.
         * @note it will first try the system srs.db then the users qgis.db!
         * @param QString The sql query to execute
         * @return QMap An associative array of field name <-> value pairs
         */
         RecordMap getRecord(QString theSql);
        /*! Overloaded == operator used to compare to SRS's.
         *  Internally it will delegate to the equals method described below
         */
         bool operator==(const QgsSpatialRefSys &theSrs);
        /*! Overloaded == operator used to compare to SRS's.
         *  Internally it will use OGR isSameSRS() or isSameGeoSRS() methods as appropriate.
         *  Additionally logic may also be applied if the result from the OGR methods
         *  is inconclusive.
         */
         bool equals(const char *theProj4CharArray);
         /*! A helper to get an ogr representation of this srs
          * @return OGRSpatialReference
          */
         OGRSpatialReference toOgrSrs();

         /*! Restores state from the given DOM node.
         * @param theNode The node from which state will be restored
         * @return bool True on success, False on failure
         */
          bool readXML( QDomNode & theNode );
        /*! Stores state to the given DOM node in the given document.
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

        // Accessors -----------------------------------

         /*! Get the SrsId - if possible
         *  @return  long theSrsId The internal sqlite3 srs.db primary key for this srs 
         */
        long srsid() const;
        /*! Get the Postgis SRID - if possible.
        *  @return  long theSRID The internal postgis SRID for this SRS
        */
        long srid() const;
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
        /** Get the Proj Proj4String. If proj and ellps keys are found in the parameters,
         * they will be stripped out and the Projection and ellipsoid acronyms will be
         * overridden with these.
         * @return  QString theProj4String Proj4 format specifies that define this srs.
         * @note A zero length string will be returned if the proj4String is uninitialised
         */
        QString proj4String() const;
        /*! Get this Geographic? flag
         * @return  bool theGeoFlag Whether this is a geographic or projected coordinate system
         */
        bool geographicFlag () const;
        /*! Get the units that the projection is in
         * @return QGis::units that gives the units for the coordinate system
         */
        QGis::units mapUnits() const;

        /*! Set the postgis srid for this srs
         * @return  long theSRID the Postgis spatial_ref_sys identifier for this srs (defaults to 0)
         */
        long postgisSrid () const;
        /*! Set the EPSG identifier for this srs
         * @return  long theEpsg the ESPG identifier for this srs (defaults to 0)
         */
        long epsg () const;

        // Mutators -----------------------------------

        /*! Set the QGIS  SrsId
         *  @param  long theSrsId The internal sqlite3 srs.db primary key for this srs 
         */
        void setSrsId(long theSrsId);
        /*! Set the postgis srid
         *  @param  long theSrsId The postgis spatial_ref_sys key for this srs 
         */
        void setSrid(long theSrid);
        /*! Set the Description
         * @param  QString the Description A textual description of the srs.
         */
        void setDescription (QString theDescription);
        /* Set the Proj Proj4String. 
         * @param  QString theProj4String Proj4 format specifies (excluding proj and ellips) that define this srs.
         */
        void setProj4String (QString theProj4String);
        /*! Set this Geographic? flag
         * @param  bool theGeoFlag Whether this is a geographic or projected coordinate system
         */
        void setGeographicFlag (bool theGeoFlag);
        /*! Set the EPSG identifier for this srs
         * @param  long theEpsg the ESPG identifier for this srs (defaults to 0)
         */
        void setEpsg (long theEpsg);
        /*! Set the projection acronym
         * @param QString the acronym (must be a valid proj4 projection acronym)
         */
        void setProjectionAcronym(QString theProjectionAcronym);
        /*! Set the ellipsoid acronym
         * @param QString the acronym (must be a valid proj4 ellipsoid acronym)
         */
        void setEllipsoidAcronym(QString theEllipsoidAcronym);
    private:
        //!The internal sqlite3 srs.db primary key for this srs 
        long    mSrsId;
        //!A textual description of the srs.
        QString mDescription;
        //!The official proj4 acronym for the projection family
        QString mProjectionAcronym ;
        //!The official proj4 acronym for the ellipoid
        QString mEllipsoidAcronym;
        //!Proj4 format specifies (excluding proj and ellips) that define this srs.
        QString mProj4String ;
        //!Whether this is a geographic or projected coordinate system
        bool    mGeoFlag;
        //! The map units
        QGis::units mMapUnits;
        //!If available, the Postgis spatial_ref_sys identifier for this srs (defaults to 0)
        long    mSRID;
        //!If available the ESPG identifier for this srs (defaults to 0)
        long    mEpsg ;
        //! Wehter this srs is properly defined and valid
        bool mIsValidFlag;

        //! Work out the projection units and set the appropriate local variable
        void setMapUnits();
};


//! Output stream operator
inline std::ostream& operator << (std::ostream& os, const QgsSpatialRefSys &r)
{
  QString mySummary ("\n\tSpatial Reference System:");
  mySummary += "\n\t\tDescription : ";
  if (r.description()) 
  {
    mySummary += r.description().latin1();
  }
  else
  {
    mySummary += "Undefined" ;
  }
  mySummary += "\n\t\tProjection  : " ;
  if (r.projectionAcronym()) 
  {
    mySummary += r.projectionAcronym().latin1();
  }
  else
  {
    mySummary += "Undefined" ;
  }

  mySummary += "\n\t\tEllipsoid   : "; 
  if (r.ellipsoidAcronym()) 
  {
    mySummary += r.ellipsoidAcronym().latin1();
  }
  else
  {
    mySummary += "Undefined" ;
  }

  mySummary += "\n\t\tProj4String  : " ;
  if (r.proj4String()) 
  {
    mySummary += r.proj4String().latin1();
  }
  else
  {
    mySummary += "Undefined" ;
  }
  return os << mySummary << std::endl;
}


/*
//! Input stream operator
inline std::istream& operator>> (std::istream& str, QgsSpatialRefSys& r)
{
  //std::cout << "FIXME FIXME" << __FILE__ << ":" << __LINE__ << std::endl;
  std::string s;
  str >> s;
  
  //QString srs = s.c_str();
  // split the string into the parts to created the object
  //QStringList parts = QStringList::split(QRegExp("\t"),srs);
  // r.setSrid(parts[0]);
  // r.setAuthName(parts[1]);
  // r.setAuthSrid(parts[2]);
  // r.setSrText(parts[3]);
  // r.setProjText(parts[4]);
  // r.setName(parts[5]);
  //
  return str;
  
} 
*/
#endif // QGSSPATIALREFSYS_H
