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
 * Constructs a SRS object from the five component parts
 * @param srid Spatial reference id (SRID)
 * @param authName Name of the authority for the SRS
 * @param authSrid SRID of the SRS assigned by the authority
 * @param srtext Well known text (WKT) of the SRS
 * @param proj4text Proj4 parameter string
 */
QgsSpatialRefSys(QString srid, QString authName, QString authSrid, QString srtext, QString proj4text, QString name);
/*!
 * Get the SRID
 * @return SRID of the SRS
 */
QString srid() const;
/*!
 * Get the authority name
 * @return authority name of the SRS creator
 */
QString authName() const;
/*!
 * Get the SRID assigned by the SRS creator
 * @return SRID 
 */
QString authSrid() const;
/*!
 * Get the WKT of the SRS
 * @return Well known text of the SRS
 */
QString srText() const;
/*!
 * Get the proj4 parameter string  for the SRS
 * @return proj4 parameters
 */
QString proj4Text() const;
/*!
 * Get the short name of the projection
 * @return name including the projection system
 */
QString name() const;
/*!
 * Test to see if the SRS is geographic
 * @return true if geographic or false if the SRS is projected
 */
bool isGeographic();
/*!
 * Set the flag to indicate if the SRS is geographic
 * @param isGeo true if the SRS is geographic; false if its projected
 */
void setGeographic(bool isGeo);
/*!
 * Set the SRID
 * @param srid Spatial reference id to assign to the object
 */
void setSrid(QString &srid);
/*!
 * Set the authority name
 * @param authname Authority name
 */
void setAuthName(QString &authname);
/*!
 * Set the authority assigned SRID 
 * @param authsrid Authority assigned SRID
 */
void setAuthSrid(QString &authsrid);
/*!
 * Set the WKT
 * @param srtext Well known text of the SRS
 */
void setSrText(QString &srtext);
/*!
 * Set the Proj4 parameters
 * @param projtext Proj4 parameter string
 */
void setProjText(QString &projtext);
/*! 
 * Set the short name
 * @param shortname Short name of the SRS
 */
void setName(QString &shortname);
  private:
//! SRID
QString mSrid;
//! Authority name
QString mAuthName;
//! Authority assigned SRID
QString mAuthSrid;
//! WKT for the SRS
QString mSrtext;
//! Proj4 paramters
QString mProj4text;
//! Short name
QString mName;
//! Flag to indicate if the SRS is geographic (unprojected)
bool mGeographic;
};
//! Output stream operator
inline std::ostream& operator << (std::ostream& os, const QgsSpatialRefSys &r)
{
    return os << r.srid() <<  "\t" << r.authName() << "\t" << r.authSrid()
      << "\t" << r.srText() << "\t" <<  r.proj4Text() << "\t" << r.name() << std::endl; 
}
//! Input stream operator
inline std::istream& operator>> (std::istream& str, QgsSpatialRefSys& r)
{
  std::string s;
  str >> s;
  QString srs = s.c_str();
  // split the string into the parts to created the object
  QStringList parts = QStringList::split(QRegExp("\t"),srs);
   r.setSrid(parts[0]);
   r.setAuthName(parts[1]);
   r.setAuthSrid(parts[2]);
   r.setSrText(parts[3]);
   r.setProjText(parts[4]);
   r.setName(parts[5]);
  return str;
}  
#endif // QGSSPATIALREFSYS_H
