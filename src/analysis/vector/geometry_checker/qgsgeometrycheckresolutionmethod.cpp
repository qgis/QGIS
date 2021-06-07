/***************************************************************************
    qgsgeometrycheckresolutionmethod.cpp
     --------------------------------------
    Date                 : January 2020
    Copyright            : (C) 2020 Matthias Kuhn
    Email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeometrycheckresolutionmethod.h"

QgsGeometryCheckResolutionMethod::QgsGeometryCheckResolutionMethod( int id, const QString &name, const QString &description, bool isStable )
  : mId( id )
  , mIsStable( isStable )
  , mName( name )
  , mDescription( description )
{
}

int QgsGeometryCheckResolutionMethod::id() const
{
  return mId;
}

bool QgsGeometryCheckResolutionMethod::isStable() const
{
  return mIsStable;
}

QString QgsGeometryCheckResolutionMethod::name() const
{
  return mName;
}

QString QgsGeometryCheckResolutionMethod::description() const
{
  return mDescription;
}
