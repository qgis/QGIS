/***************************************************************************
  qgsvector.cpp - QgsVector

 ---------------------
 begin                : 24.2.2017
 copyright            : (C) 2017 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgsvector.h"
#include "qgis.h"
#include "qgsexception.h"

QgsVector QgsVector::rotateBy( double rot ) const
{
  double angle = std::atan2( mY, mX ) + rot;
  double len = length();
  return QgsVector( len * std::cos( angle ), len * std::sin( angle ) );
}

QgsVector QgsVector::normalized() const
{
  double len = length();

  if ( len == 0.0 )
  {
    throw QgsException( QStringLiteral( "normalized vector of null vector undefined" ) );
  }

  return *this / len;
}
