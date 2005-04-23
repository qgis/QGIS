#include "qgsspatialrefsys.h"
QgsSpatialRefSys::QgsSpatialRefSys(){}

QgsSpatialRefSys::QgsSpatialRefSys(long theSrsId, 
        QString theDescription, 
        QString theProjectionAcronym, 
        QString theEllipsoidAcronym, 
        QString theParameters,
        long theSRID,
        long theEpsg,
        bool theGeoFlag)

{
}
// Accessors -----------------------------------

/*! Get the SrsId
 *  @return  long theSrsId The internal sqlite3 srs.db primary key for this srs 
 */
long QgsSpatialRefSys::srid()
{
  return mSRID;
}
/*! Get the Description
 * @return  QString the Description A textual description of the srs.
 */
QString QgsSpatialRefSys::description ()
{
  return mDescription;
}
/*! Get the Projection Acronym
 * @return  QString theProjectionAcronym The official proj4 acronym for the projection family
 */
QString QgsSpatialRefSys::projectionAcronym()
{
  return mProjectionAcronym;
}
/*! Get the Ellipsoid Acronym
 * @return  QString theEllipsoidAcronym The official proj4 acronym for the ellipoid
 */
QString QgsSpatialRefSys::ellipsoid ()
{
  return mEllipsoidAcronym;
}
/* Get the Proj Parameters. If proj and ellps keys are found in the parameters,
 * they will be stripped out and the Projection and ellipsoid acronyms will be
 * overridden with these.
 * @return  QString theParameters Proj4 format specifies (excluding proj and ellips) that define this srs.
 */
QString QgsSpatialRefSys::parameters ()
{
  return mParameters;
}
/*! Get this Geographic? flag
 * @return  bool theGeoFlag Whether this is a geographic or projected coordinate system
 */
bool QgsSpatialRefSys::geographicFlag ()
{
  return mGeoFlag;
}

/*! Set the postgis srid for this srs
 * @return  long theSRID the Postgis spatial_ref_sys identifier for this srs (defaults to 0)
 */
long QgsSpatialRefSys::postgisSrid ()
{
  return mSRID ;
}
/*! Set the EPSG identifier for this srs
 * @return  long theEpsg the ESPG identifier for this srs (defaults to 0)
 */
long QgsSpatialRefSys::epsg ()
{
  return mEpsg;
}

// Mutators -----------------------------------


/*! Set the SrsId
 *  @param  long theSrsId The internal sqlite3 srs.db primary key for this srs 
 */
void QgsSpatialRefSys::setSrid(long theSrid)
{
  mSRID=theSrid;
}
/*! Set the Description
 * @param  QString the Description A textual description of the srs.
 */
void QgsSpatialRefSys::setDescription (QString theDescription)
{
  mDescription = theDescription;
}
/*! Set the Projection Acronym
 * @param  QString theProjectionAcronym The official proj4 acronym for the projection family
 */
void QgsSpatialRefSys::setProjectionAcronym(QString theProjectionAcronym)
{
  mProjectionAcronym=theProjectionAcronym;
}
/*! Set the Ellipsoid Acronym
 * @param  QString theEllipsoidAcronym The official proj4 acronym for the ellipoid
 */
void QgsSpatialRefSys::setEllipsoid (QString theEllipsoidAcronym)
{
  mEllipsoidAcronym=theEllipsoidAcronym;
}
/* Set the Proj Parameters. If proj and ellps keys are found in the parameters,
 * they will be stripped out and the Projection and ellipsoid acronyms will be
 * overridden with these.
 * @param  QString theParameters Proj4 format specifies (excluding proj and ellips) that define this srs.
 */
void QgsSpatialRefSys::setParameters (QString theParameters)
{
  mParameters=theParameters;
}
/*! Set this Geographic? flag
 * @param  bool theGeoFlag Whether this is a geographic or projected coordinate system
 */
void QgsSpatialRefSys::setGeographicFlag (bool theGeoFlag)
{
  mGeoFlag=theGeoFlag;
}

/*! Set the postgis srid for this srs
 * @param  long theSRID the Postgis spatial_ref_sys identifier for this srs (defaults to 0)
 */
void QgsSpatialRefSys::setPostgisSrid (long theSrid)
{
  mSRID=theSrid;
}
/*! Set the EPSG identifier for this srs
 * @param  long theEpsg the ESPG identifier for this srs (defaults to 0)
 */
void QgsSpatialRefSys::setEpsg (long theEpsg)
{
  mEpsg=theEpsg;
}
