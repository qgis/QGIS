#include "qgsgeometrycheckfix.h"

QgsGeometryCheckFix::QgsGeometryCheckFix( int id, const QString &name, const QString &description, bool isStable )
{
  mId = id;
  mName = name;
  mDescription = description;
  mIsStable = isStable;
}

int QgsGeometryCheckFix::id() const
{
  return mId;
}

bool QgsGeometryCheckFix::isStable() const
{
  return mIsStable;
}

QString QgsGeometryCheckFix::name() const
{
  return mName;
}

QString QgsGeometryCheckFix::description() const
{
  return mDescription;
}
