#ifndef QGSSPATIALREFSYS_H
#define QGSSPATIALREFSYS_H

//Standard includes
#include <ostream>
#include <istream>

//qt includes
#include <qstring.h>
#include <qstringlist.h>
#include <qregexp.h>

//qgis includes


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

        // Misc helper functions -----------------------
        
        /*! Set up this srs by fetching the appropriate information from the 
         * sqlite backend. First the system level read only srs.db will be checked
         * and then the users ~/.qgis/qgis.db database will be checked for a match.
         * @note Any members will be overwritten during this process.
         * @param theSrid The postgis SRID for the desired spatial reference system.
         */
        void createFromSrid(const long theSrid);
        /*! Set up this srs using a WKT spatial ref sys definition. 
         * The wkt will be converted to a proj4 string using OGR helper
         * functions. After this the srs databasses will be searched for matches.
         * First the system level read only srs.db will be checked
         * and then the users ~/.qgis/qgis.db database will be checked for a match.
         * @note Any members will be overwritten during this process.
         * @note SRID and EPSG may be blank if no match can be found on srs db.
         * @param theWkt The WKT for the desired spatial reference system.
         */
        void createFromWkt(const QString theWkt);
        /*! Set up this srs by fetching the appropriate information from the 
         * sqlite backend. First the system level read only srs.db will be checked
         * and then the users ~/.qgis/qgis.db database will be checked for a match.
         * @note Any members will be overwritten during this process.
         * @param theEpsg The EPSG for the desired spatial reference system.
         */
        void createFromEpsg(const long theEpsg);
        /*! Set up this srs by fetching the appropriate information from the 
         * sqlite backend. If the srsid is < 100000, only the system srs.db 
         * will be checked. If the srsid > 100000 the srs will be retrieved from 
         * the ~/.qgis/qgis.db
         * @note Any members will be overwritten during this process.
         * @param theSrsId The QGIS SrsId for the desired spatial reference system.
         */
        void createFromSrsId (const long theSrsId);

        /*! Set up this srs by passing it a proj4 style formatted string.
         * The string will be parsed and the projection and ellipsoid
         * members set and the remainder of the proj4 string will be stored
         * in the parameters member. The reason for this is so that we
         * can easily present the user with 'natural language' representation
         * of the projection and ellipsoid by looking them up in the srs.bs sqlite 
         * database.
         * @param theProjString A proj4 format string
         */
        void createFromProj4 (const QString theProjString);

        /*! Find out whether this SRS is correctly initialised and useable */
        bool isValid() const;
        /*! Perform some validation on this SRS. If the sts doesnt validate the
         *  default behaviour settings for layers with unknown SRS will be 
         * consulted and acted on accordingly. By hell or high water this
         * method will do its best to make sure that this SRS is valid - even
         * if that involves resorting to a hard coded default of geocs:wgs84.
         */
        void validate();

        // Accessors -----------------------------------

        /*! Get the SrsId
         *  @return  long theSrsId The internal sqlite3 srs.db primary key for this srs 
         */
        long srid() const;
        /*! Get the Description
         * @return  QString the Description A textual description of the srs.
         */
        QString description () const;
        /*! Get the Projection Acronym
         * @return  QString theProjectionAcronym The official proj4 acronym for the projection family
         */
        QString projectionAcronym() const;
        /*! Get the Ellipsoid Acronym
         * @return  QString theEllipsoidAcronym The official proj4 acronym for the ellipoid
         */
        QString ellipsoidAcronym () const;
        /* Get the Proj Proj4String. If proj and ellps keys are found in the parameters,
         * they will be stripped out and the Projection and ellipsoid acronyms will be
         * overridden with these.
         * @return  QString theProj4String Proj4 format specifies that define this srs.
         */
        QString proj4String() const;
        /*! Get this Geographic? flag
         * @return  bool theGeoFlag Whether this is a geographic or projected coordinate system
         */
        bool geographicFlag () const;

        /*! Set the postgis srid for this srs
         * @return  long theSRID the Postgis spatial_ref_sys identifier for this srs (defaults to 0)
         */
        long postgisSrid () const;
        /*! Set the EPSG identifier for this srs
         * @return  long theEpsg the ESPG identifier for this srs (defaults to 0)
         */
        long epsg () const;

        // Mutators -----------------------------------


        /*! Set the SrsId
         *  @param  long theSrsId The internal sqlite3 srs.db primary key for this srs 
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

        /*! Set the postgis srid for this srs
         * @param  long theSRID the Postgis spatial_ref_sys identifier for this srs (defaults to 0)
         */
        void setPostgisSrid (long theSrid);
        /*! Set the EPSG identifier for this srs
         * @param  long theEpsg the ESPG identifier for this srs (defaults to 0)
         */
        void setEpsg (long theEpsg);

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
        //!If available, the Postgis spatial_ref_sys identifier for this srs (defaults to 0)
        long    mSRID;
        //!If available the ESPG identifier for this srs (defaults to 0)
        long    mEpsg ;
        //! Wehter this srs is properly defined and valid
        bool isValidFlag;
};


//! Output stream operator
inline std::ostream& operator << (std::ostream& os, const QgsSpatialRefSys &r)
{
  return os << "\n"
	    << "Description : " << r.description().latin1() << "\n"
	    << "Projection  : " << r.projectionAcronym().latin1() << "\n"
	    << "Ellipsoid   : " << r.ellipsoidAcronym().latin1() << "\n"
	    << "Proj4String  : " << r.proj4String().latin1() << "\n"
	    << std::endl; 
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
