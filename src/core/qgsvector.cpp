/***************************************************************************
  qgsvector.cpp - QgsVector

 ---------------------
 begin                : 24.2.2017
 copyright            : (C) 2017 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvector.h"
#include "qgis.h"
#include "qgsexception.h"

QgsVector QgsVector::rotateBy( double rot ) const
{
  const double angle = std::atan2( mY, mX ) + rot;
  const double len = length();
  return QgsVector( len * std::cos( angle ), len * std::sin( angle ) );
}

QgsVector QgsVector::normalized() const
{
  const double len = length();

  if ( len == 0.0 )
  {
    throw QgsException( QStringLiteral( "normalized vector of null vector undefined" ) );
  }

  return *this / len;
}
