#ifndef QGSSPATIALREFSYS_H
#define QGSSPATIALREFSYS_H
#include <ostream>
#include <istream>
#include <qstring.h>
#include <qstringlist.h>
#include <qregexp.h>
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
         * Constructs a SRS object from the following component parts
         *
         * @param  long theSrsId The internal sqlite3 srs.db primary key for this srs 
         * @param  QString the Description A textual description of the srs.
         * @param  QString theProjectionAcronym The official proj4 acronym for the projection family
         * @param  QString theEllipsoidAcronym The official proj4 acronym for the ellipoid
         * @param  QString theParameters Proj4 format specifies (excluding proj and ellips) that define this srs.
         * @param  bool theGeoFlag Whether this is a geographic or projected coordinate system
         * @param  long theSRID If available, the Postgis spatial_ref_sys identifier for this srs (defaults to 0)
         * @param  long theEpsg If available the ESPG identifier for this srs (defaults to 0)
         */
        QgsSpatialRefSys(long theSrsId, 
                QString theDescription, 
                QString theProjectionAcronym, 
                QString theEllipsoidAcronym, 
                QString theParameters,
                long theSRID,
                long theEpsg,
                bool theGeoFlag);

        // Accessors -----------------------------------
        
        /*! Get the SrsId
         *  @return  long theSrsId The internal sqlite3 srs.db primary key for this srs 
         */
        long srid();
        /*! Get the Description
         * @return  QString the Description A textual description of the srs.
         */
        QString description ();
        /*! Get the Projection Acronym
         * @return  QString theProjectionAcronym The official proj4 acronym for the projection family
         */
        QString projectionAcronym();
        /*! Get the Ellipsoid Acronym
         * @return  QString theEllipsoidAcronym The official proj4 acronym for the ellipoid
         */
        QString ellipsoid ();
        /* Get the Proj Parameters. If proj and ellps keys are found in the parameters,
         * they will be stripped out and the Projection and ellipsoid acronyms will be
         * overridden with these.
         * @return  QString theParameters Proj4 format specifies (excluding proj and ellips) that define this srs.
         */
        QString parameters ();
        /*! Get this Geographic? flag
         * @return  bool theGeoFlag Whether this is a geographic or projected coordinate system
         */
        bool geographicFlag ();

        /*! Set the postgis srid for this srs
         * @return  long theSRID the Postgis spatial_ref_sys identifier for this srs (defaults to 0)
         */
        long postgisSrid ();
        /*! Set the EPSG identifier for this srs
         * @return  long theEpsg the ESPG identifier for this srs (defaults to 0)
         */
        long epsg ();

        // Mutators -----------------------------------


        /*! Set the SrsId
         *  @param  long theSrsId The internal sqlite3 srs.db primary key for this srs 
         */
        void setSrid(long theSrid);
        /*! Set the Description
         * @param  QString the Description A textual description of the srs.
         */
        void setDescription (QString theDescription);
        /*! Set the Projection Acronym
         * @param  QString theProjectionAcronym The official proj4 acronym for the projection family
         */
        void setProjectionAcronym(QString theProjectionAcronym);
        /*! Set the Ellipsoid Acronym
         * @param  QString theEllipsoidAcronym The official proj4 acronym for the ellipoid
         */
        void setEllipsoid (QString theEllipsoidAcronym);
        /* Set the Proj Parameters. If proj and ellps keys are found in the parameters,
         * they will be stripped out and the Projection and ellipsoid acronyms will be
         * overridden with these.
         * @param  QString theParameters Proj4 format specifies (excluding proj and ellips) that define this srs.
         */
        void setParameters (QString theParameters);
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
            QString mParameters ;
            //!Whether this is a geographic or projected coordinate system
            bool    mGeoFlag;
            //!If available, the Postgis spatial_ref_sys identifier for this srs (defaults to 0)
            long    mSRID;
            //!If available the ESPG identifier for this srs (defaults to 0)
            long    mEpsg ;
};


//! Output stream operator
inline std::ostream& operator << (std::ostream& os, const QgsSpatialRefSys &r)
{
    return os << "FIXME FIXME" << __FILE__ << ":" << __LINE__ << std::endl;
    /*
    return os << r.srid() <<  "\t" << r.authName() << "\t" << r.authSrid()
      << "\t" << r.srText() << "\t" <<  r.proj4Text() << "\t" << r.name() << std::endl; 
      */
}
//! Input stream operator
inline std::istream& operator>> (std::istream& str, QgsSpatialRefSys& r)
{
  //std::cout << "FIXME FIXME" << __FILE__ << ":" << __LINE__ << std::endl;
  std::string s;
  str >> s;
  /*
  QString srs = s.c_str();
  // split the string into the parts to created the object
  QStringList parts = QStringList::split(QRegExp("\t"),srs);
   r.setSrid(parts[0]);
   r.setAuthName(parts[1]);
   r.setAuthSrid(parts[2]);
   r.setSrText(parts[3]);
   r.setProjText(parts[4]);
   r.setName(parts[5]);
   */
  return str;
  
}  
#endif // QGSSPATIALREFSYS_H
