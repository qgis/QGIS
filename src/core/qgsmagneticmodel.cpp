/***************************************************************************
                             qgsmagneticmodel.cpp
                             ---------------------------
    begin                : December 2025
    copyright            : (C) 2025 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmagneticmodel.h"

#ifdef WITH_GEOGRAPHICLIB
#include <GeographicLib/MagneticModel.hpp>
#else
#include "qgsexception.h"
#endif

QString QgsMagneticModel::defaultFilePath()
{
#ifdef WITH_GEOGRAPHICLIB
  return QString::fromStdString( GeographicLib::MagneticModel::DefaultMagneticPath() );
#else
  throw QgsNotSupportedException( u"GeographicLib is not available on this system"_s );
#endif
}

QString QgsMagneticModel::defaultModelName()
{
#ifdef WITH_GEOGRAPHICLIB
  return QString::fromStdString( GeographicLib::MagneticModel::DefaultMagneticName() );
#else
  throw QgsNotSupportedException( u"GeographicLib is not available on this system"_s );
#endif
}

QgsMagneticModel::QgsMagneticModel( const QString &name, const QString &path, int nMax, int mMax )
  : mName( name )
  , mPath( path )
{
#ifdef WITH_GEOGRAPHICLIB
  try
  {
    mModel = std::make_unique< GeographicLib::MagneticModel >( mName.toStdString(), mPath.toStdString(), GeographicLib::Geocentric::WGS84(), nMax, mMax );
  }
  catch ( GeographicLib::GeographicErr &e )
  {
    mError = QString::fromStdString( e.what() );
  }
  catch ( std::bad_alloc &e )
  {
    mError = QString::fromStdString( e.what() );
  }
#else
  ( void ) nMax;
  ( void ) mMax;
#endif
}

QgsMagneticModel::~QgsMagneticModel() = default;

bool QgsMagneticModel::isValid() const
{
#ifdef WITH_GEOGRAPHICLIB
  return static_cast< bool >( mModel );
#else
  return false;
#endif
}

QString QgsMagneticModel::description() const
{
#ifdef WITH_GEOGRAPHICLIB
  if ( !mModel )
    return QString();

  const QString desc = QString::fromStdString( mModel->Description() );
  return desc == "UNKNOWN"_L1 ? QString() : desc;
#else
  throw QgsNotSupportedException( u"GeographicLib is not available on this system"_s );
#endif
}

QDateTime QgsMagneticModel::dateTime() const
{
#ifdef WITH_GEOGRAPHICLIB
  if ( !mModel )
    return QDateTime();

  const QString dt = QString::fromStdString( mModel->DateTime() );
  if ( dt == "UNKNOWN"_L1 )
    return QDateTime();

  return QDateTime::fromString( dt, Qt::ISODate );
#else
  throw QgsNotSupportedException( u"GeographicLib is not available on this system"_s );
#endif
}

QString QgsMagneticModel::file() const
{
#ifdef WITH_GEOGRAPHICLIB
  return mModel ? QString::fromStdString( mModel->MagneticFile() ) : QString();
#else
  throw QgsNotSupportedException( u"GeographicLib is not available on this system"_s );
#endif
}

QString QgsMagneticModel::directory() const
{
#ifdef WITH_GEOGRAPHICLIB
  return mModel ? QString::fromStdString( mModel->MagneticModelDirectory() ) : QString();
#else
  throw QgsNotSupportedException( u"GeographicLib is not available on this system"_s );
#endif
}

QString QgsMagneticModel::name() const
{
#ifdef WITH_GEOGRAPHICLIB
  return mModel ? QString::fromStdString( mModel->MagneticModelName() ) : QString();
#else
  throw QgsNotSupportedException( u"GeographicLib is not available on this system"_s );
#endif
}

double QgsMagneticModel::minimumHeight() const
{
#ifdef WITH_GEOGRAPHICLIB
  return mModel ? mModel->MinHeight() : 0;
#else
  throw QgsNotSupportedException( u"GeographicLib is not available on this system"_s );
#endif
}

double QgsMagneticModel::maximumHeight() const
{
#ifdef WITH_GEOGRAPHICLIB
  return mModel ? mModel->MaxHeight() : 0;
#else
  throw QgsNotSupportedException( u"GeographicLib is not available on this system"_s );
#endif
}

double QgsMagneticModel::minimumYear() const
{
#ifdef WITH_GEOGRAPHICLIB
  return mModel ? mModel->MinTime() : 0;
#else
  throw QgsNotSupportedException( u"GeographicLib is not available on this system"_s );
#endif
}

double QgsMagneticModel::maximumYear() const
{
#ifdef WITH_GEOGRAPHICLIB
  return mModel ? mModel->MaxTime() : 0;
#else
  throw QgsNotSupportedException( u"GeographicLib is not available on this system"_s );
#endif
}

int QgsMagneticModel::degree() const
{
#ifdef WITH_GEOGRAPHICLIB
  return mModel ? mModel->Degree() : 0;
#else
  throw QgsNotSupportedException( u"GeographicLib is not available on this system"_s );
#endif
}

int QgsMagneticModel::order() const
{
#ifdef WITH_GEOGRAPHICLIB
  return mModel ? mModel->Order() : 0;
#else
  throw QgsNotSupportedException( u"GeographicLib is not available on this system"_s );
#endif
}

bool QgsMagneticModel::declination( double years, double latitude, double longitude, double height, double &declination ) const
{
  declination = 0;
#ifdef WITH_GEOGRAPHICLIB
  double Bx = 0;
  double By = 0;
  double Bz = 0;
  if ( !getComponents( years, latitude, longitude, height, Bx, By, Bz ) )
    return false;

  double H = 0;
  double F = 0;
  double I = 0;
  if ( !fieldComponents( Bx, By, Bz, H, F, declination, I ) )
    return false;

  return true;
#else
  ( void )years;
  ( void )latitude;
  ( void )longitude;
  ( void )height;
  throw QgsNotSupportedException( u"GeographicLib is not available on this system"_s );
#endif
}

bool QgsMagneticModel::inclination( double years, double latitude, double longitude, double height, double &inclination ) const
{
  inclination = 0;
#ifdef WITH_GEOGRAPHICLIB
  double Bx = 0;
  double By = 0;
  double Bz = 0;
  if ( !getComponents( years, latitude, longitude, height, Bx, By, Bz ) )
    return false;

  double H = 0;
  double F = 0;
  double D = 0;
  if ( !fieldComponents( Bx, By, Bz, H, F, D, inclination ) )
    return false;

  return true;
#else
  ( void )years;
  ( void )latitude;
  ( void )longitude;
  ( void )height;
  throw QgsNotSupportedException( u"GeographicLib is not available on this system"_s );
#endif
}

bool QgsMagneticModel::getComponents( double years, double latitude, double longitude, double height, double &Bx, double &By, double &Bz ) const
{
  Bx = 0;
  By = 0;
  Bz = 0;
#ifdef WITH_GEOGRAPHICLIB
  if ( !mModel )
    return false;

  ( *mModel )( years, latitude, longitude, height, Bx, By, Bz );
  return true;
#else
  ( void ) years;
  ( void ) latitude;
  ( void ) longitude;
  ( void ) height;
  throw QgsNotSupportedException( u"GeographicLib is not available on this system"_s );
#endif
}

bool QgsMagneticModel::getComponentsWithTimeDerivatives( double years, double latitude, double longitude, double height, double &Bx, double &By, double &Bz, double &Bxt, double &Byt, double &Bzt ) const
{
  Bx = 0;
  By = 0;
  Bz = 0;
  Bxt = 0;
  Byt = 0;
  Bzt = 0;
#ifdef WITH_GEOGRAPHICLIB
  if ( !mModel )
    return false;

  ( *mModel )( years, latitude, longitude, height, Bx, By, Bz, Bxt, Byt, Bzt );
  return true;
#else
  ( void ) years;
  ( void ) latitude;
  ( void ) longitude;
  ( void ) height;
  throw QgsNotSupportedException( u"GeographicLib is not available on this system"_s );
#endif
}

bool QgsMagneticModel::fieldComponents( double Bx, double By, double Bz, double &H, double &F, double &D, double &I )
{
  H = 0;
  F = 0;
  D = 0;
  I = 0;
#ifdef WITH_GEOGRAPHICLIB
  GeographicLib::MagneticModel::FieldComponents( Bx, By, Bz, H, F, D, I );
  return true;
#else
  ( void ) Bx;
  ( void ) By;
  ( void ) Bz;
  throw QgsNotSupportedException( u"GeographicLib is not available on this system"_s );
#endif
}

bool QgsMagneticModel::fieldComponentsWithTimeDerivatives( double Bx, double By, double Bz, double Bxt, double Byt, double Bzt, double &H, double &F, double &D, double &I, double &Ht, double &Ft, double &Dt, double &It )
{
  H = 0;
  F = 0;
  D = 0;
  I = 0;
  Ht = 0;
  Ft = 0;
  Dt = 0;
  It = 0;
#ifdef WITH_GEOGRAPHICLIB
  GeographicLib::MagneticModel::FieldComponents( Bx, By, Bz, Bxt, Byt, Bzt, H, F, D, I, Ht, Ft, Dt, It );
  return true;
#else
  ( void ) Bx;
  ( void ) By;
  ( void ) Bz;
  ( void ) Bxt;
  ( void ) Byt;
  ( void ) Bzt;
  throw QgsNotSupportedException( u"GeographicLib is not available on this system"_s );
#endif
}
