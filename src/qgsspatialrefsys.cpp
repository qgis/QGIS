#include "qgsspatialrefsys.h"
QgsSpatialRefSys::QgsSpatialRefSys(){}
QgsSpatialRefSys::QgsSpatialRefSys(QString srid, QString authName, QString authSrid, 
    QString srtext, QString proj4Text, QString name):mSrid(srid), mAuthName(authName), mAuthSrid(authSrid), mSrtext(srtext), mProj4text(proj4Text), mName(name)
{
}
QString QgsSpatialRefSys::srid() const
{
  return mSrid;
}

QString QgsSpatialRefSys::authName() const
{
  return mAuthName;
}
QString QgsSpatialRefSys::authSrid() const
{
  return mAuthSrid;
}
QString QgsSpatialRefSys::srText() const
{
  return mSrtext;
}
QString QgsSpatialRefSys::proj4Text() const
{
  return mProj4text;
}
QString QgsSpatialRefSys::name() const
{
  return mName;
}

void QgsSpatialRefSys::setSrid(QString& srid)
{
  mSrid = srid;
}
void QgsSpatialRefSys::setAuthName(QString &authname)
{
  mAuthName = authname;
}
void QgsSpatialRefSys::setAuthSrid(QString &authsrid)
{
  mAuthSrid = authsrid;
}
void QgsSpatialRefSys::setSrText(QString &srtext)
{
  mSrtext = srtext;
}
void QgsSpatialRefSys::setProjText(QString &projtext)
{
  mProj4text = projtext;
}
void QgsSpatialRefSys::setName(QString &shortname)
{
  mName = shortname;
}
bool QgsSpatialRefSys::isGeographic()
{
  return mGeographic;
}
void QgsSpatialRefSys::setGeographic(bool isGeogCS)
{
  mGeographic = isGeogCS;
}
