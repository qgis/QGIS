/***************************************************************************
    qgsstacprovider.cpp
    ---------------------
    begin                : October 2024
    copyright            : (C) 2024 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsstacprovider.h"

QgsStacProvider::QgsStacProvider( const QString &name, const QString &description, const QStringList &roles, const QString &url )
  : mName( name )
  , mDescription( description )
  , mRoles( roles )
  , mUrl( url )
{
}

QString QgsStacProvider::name() const
{
  return mName;
}

QString QgsStacProvider::description() const
{
  return mDescription;
}

QStringList QgsStacProvider::roles() const
{
  return mRoles;
}

QString QgsStacProvider::url() const
{
  return mUrl;
}
