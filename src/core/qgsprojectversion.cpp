/***************************************************************************
                          qgsprojectfile.h  -  description
                             -------------------
    begin                : Sun 15 dec 2007
    copyright            : (C) 2007 by Magnus Homann
    email                : magnus at homann.se
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QString>
#include <QStringList>

#include "qgslogger.h"
#include "qgsprojectversion.h"

QgsProjectVersion::QgsProjectVersion( int major, int minor, int sub, const QString& name )
{
  mMajor = major;
  mMinor = minor;
  mSub   = sub;
  mName  = name;
}

QgsProjectVersion::QgsProjectVersion( const QString& string )
{
  QString pre = string.section( '-', 0, 0 );

  QStringList fileVersionParts = pre.section( '-', 0 ).split( '.' );

  mMinor = 0;
  mSub   = 0;
  mMajor = fileVersionParts.at( 0 ).toInt();

  if ( fileVersionParts.size() > 1 )
  {
    mMinor = fileVersionParts.at( 1 ).toInt();
  }
  if ( fileVersionParts.size() > 2 )
  {
    mSub   = fileVersionParts.at( 2 ).toInt();
  }
  mName  = string.section( '-', 1 );

  QgsDebugMsg( QString( "Version is set to " ) + text() );

}

/** Boolean equal operator
 */
bool QgsProjectVersion::operator==( const QgsProjectVersion &other ) const
{
  return (( mMajor == other.mMajor ) &&
          ( mMinor == other.mMinor ) &&
          ( mSub == other.mSub ) );
}

/** Boolean equal operator
 */
bool QgsProjectVersion::operator!=( const QgsProjectVersion &other ) const
{
  return (( mMajor != other.mMajor ) ||
          ( mMinor != other.mMinor ) ||
          ( mSub != other.mSub ) );
}

/** Boolean >= operator
 */
bool QgsProjectVersion::operator>=( const QgsProjectVersion &other ) const
{
  return (( mMajor >= other.mMajor ) ||
          (( mMajor == other.mMajor ) && ( mMinor >= other.mMinor ) ) ||
          (( mMajor == other.mMajor ) && ( mMinor == other.mMinor ) && ( mSub >= other.mSub ) ) );
}

/** Boolean > operator
 */
bool QgsProjectVersion::operator>( const QgsProjectVersion &other ) const
{
  return (( mMajor > other.mMajor ) ||
          (( mMajor == other.mMajor ) && ( mMinor > other.mMinor ) ) ||
          (( mMajor == other.mMajor ) && ( mMinor == other.mMinor ) && ( mSub > other.mSub ) ) );
}

QString QgsProjectVersion::text()
{
  if ( mName.isEmpty() )
  {
    return QString( "%1.%2.%3" ).arg( mMajor ).arg( mMinor ).arg( mSub );
  }
  else
  {
    return QString( "%1.%2.%3-%4" ).arg( mMajor ).arg( mMinor ).arg( mSub ).arg( mName );
  }
}
