/***************************************************************************
    qgsgeometrycheckresolutionmethod.cpp
     --------------------------------------
    Date                 : January 2020
    Copyright            : (C) 2020 Matthias Kuhn
    Email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeometrycheckresolutionmethod.h"

QgsGeometryCheckResolutionMethod::QgsGeometryCheckResolutionMethod( int id, const QString &name, const QString &description, bool isStable )
{
  mId = id;
  mName = name;
  mDescription = description;
  mIsStable = isStable;
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
