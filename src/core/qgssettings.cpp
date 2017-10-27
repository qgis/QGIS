/***************************************************************************
  qgssettings.cpp
  --------------------------------------
  Date                 : January 2017
  Copyright            : (C) 2017 by Alessandro Pasotti
  Email                : apasotti at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <cstdlib>
#include <QFileInfo>
#include <QSettings>
#include <QDir>

#include "qgssettings.h"
#include "qgslogger.h"

QString QgsSettings::sGlobalSettingsPath = QString();

bool QgsSettings::setGlobalSettingsPath( const QString &path )
{
  if ( QFileInfo::exists( path ) )
  {
    sGlobalSettingsPath = path;
    return true;
  }
  return false;
}

void QgsSettings::init()
{
  if ( ! sGlobalSettingsPath.isEmpty() )
  {
    mGlobalSettings = new QSettings( sGlobalSettingsPath, QSettings::IniFormat );
    mGlobalSettings->setIniCodec( "UTF-8" );
  }
}


QgsSettings::QgsSettings( const QString &organization, const QString &application, QObject *parent )
{
  mUserSettings = new QSettings( organization, application, parent );
  init();
}

QgsSettings::QgsSettings( QSettings::Scope scope, const QString &organization,
                          const QString &application, QObject *parent )
{
  mUserSettings = new QSettings( scope, organization, application, parent );
  init();
}

QgsSettings::QgsSettings( QSettings::Format format, QSettings::Scope scope,
                          const QString &organization, const QString &application, QObject *parent )
{
  mUserSettings = new QSettings( format, scope, organization, application, parent );
  init();
}

QgsSettings::QgsSettings( const QString &fileName, QSettings::Format format, QObject *parent )
{
  mUserSettings = new QSettings( fileName, format, parent );
  init();
}

QgsSettings::QgsSettings( QObject *parent )
{
  mUserSettings = new QSettings( parent );
  init();
}

QgsSettings::~QgsSettings()
{
  delete mUserSettings;
  delete mGlobalSettings;
}


void QgsSettings::beginGroup( const QString &prefix, const QgsSettings::Section section )
{
  QString pKey = prefixedKey( prefix, section );
  mUserSettings->beginGroup( pKey );
  if ( mGlobalSettings )
  {
    mGlobalSettings->beginGroup( pKey );
  }
}

void QgsSettings::endGroup()
{
  mUserSettings->endGroup();
  if ( mGlobalSettings )
  {
    mGlobalSettings->endGroup();
  }
}


QStringList QgsSettings::allKeys() const
{
  QStringList keys = mUserSettings->allKeys();
  if ( mGlobalSettings )
  {
    for ( auto &s : mGlobalSettings->allKeys() )
    {
      if ( ! keys.contains( s ) )
      {
        keys.append( s );
      }
    }
  }
  return keys;
}


QStringList QgsSettings::childKeys() const
{
  QStringList keys = mUserSettings->childKeys();
  if ( mGlobalSettings )
  {
    for ( auto &s : mGlobalSettings->childKeys() )
    {
      if ( ! keys.contains( s ) )
      {
        keys.append( s );
      }
    }
  }
  return keys;
}

QStringList QgsSettings::childGroups() const
{
  QStringList keys = mUserSettings->childGroups();
  if ( mGlobalSettings )
  {
    for ( auto &s : mGlobalSettings->childGroups() )
    {
      if ( ! keys.contains( s ) )
      {
        keys.append( s );
      }
    }
  }
  return keys;
}
QStringList QgsSettings::globalChildGroups() const
{
  QStringList keys;
  if ( mGlobalSettings )
  {
    keys = mGlobalSettings->childGroups();
  }
  return keys;
}

QVariant QgsSettings::value( const QString &key, const QVariant &defaultValue, const QgsSettings::Section section ) const
{
  QString pKey = prefixedKey( key, section );
  if ( !mUserSettings->value( pKey ).isNull() )
  {
    return mUserSettings->value( pKey );
  }
  if ( mGlobalSettings )
  {
    return mGlobalSettings->value( pKey, defaultValue );
  }
  return defaultValue;
}

bool QgsSettings::contains( const QString &key, const QgsSettings::Section section ) const
{
  QString pKey = prefixedKey( key, section );
  return mUserSettings->contains( pKey ) ||
         ( mGlobalSettings && mGlobalSettings->contains( pKey ) );
}

QString QgsSettings::fileName() const
{
  return mUserSettings->fileName();
}

void QgsSettings::sync()
{
  return mUserSettings->sync();
}

void QgsSettings::remove( const QString &key, const QgsSettings::Section section )
{
  QString pKey = prefixedKey( key, section );
  mUserSettings->remove( pKey );
}

QString QgsSettings::prefixedKey( const QString &key, const Section section ) const
{
  QString prefix;
  switch ( section )
  {
    case Section::Core :
      prefix = QStringLiteral( "core" );
      break;
    case Section::Server :
      prefix = QStringLiteral( "server" );
      break;
    case Section::Gui :
      prefix = QStringLiteral( "gui" );
      break;
    case Section::Plugins :
      prefix = QStringLiteral( "plugins" );
      break;
    case Section::Misc :
      prefix = QStringLiteral( "misc" );
      break;
    case Section::Auth :
      prefix = QStringLiteral( "auth" );
      break;
    case Section::App :
      prefix = QStringLiteral( "app" );
      break;
    case Section::Providers :
      prefix = QStringLiteral( "providers" );
      break;
    case Section::NoSection:
    default:
      return sanitizeKey( key );
  }
  return prefix  + "/" + sanitizeKey( key );
}


int QgsSettings::beginReadArray( const QString &prefix )
{
  int size = mUserSettings->beginReadArray( sanitizeKey( prefix ) );
  if ( 0 == size && mGlobalSettings )
  {
    size = mGlobalSettings->beginReadArray( sanitizeKey( prefix ) );
    mUsingGlobalArray = ( size > 0 );
  }
  return size;
}

void QgsSettings::beginWriteArray( const QString &prefix, int size )
{
  mUsingGlobalArray = false;
  mUserSettings->beginWriteArray( prefix, size );
}

void QgsSettings::endArray()
{
  mUserSettings->endArray();
  if ( mGlobalSettings )
  {
    mGlobalSettings->endArray();
  }
  mUsingGlobalArray = false;
}

void QgsSettings::setArrayIndex( int i )
{
  if ( mGlobalSettings && mUsingGlobalArray )
  {
    mGlobalSettings->setArrayIndex( i );
  }
  else
  {
    mUserSettings->setArrayIndex( i );
  }
}

void QgsSettings::setValue( const QString &key, const QVariant &value, const QgsSettings::Section section )
{
  // TODO: add valueChanged signal
  mUserSettings->setValue( prefixedKey( key, section ), value );
}

// To lower case and clean the path
QString QgsSettings::sanitizeKey( const QString &key ) const
{
  return QDir::cleanPath( key );
}

void QgsSettings::clear()
{
  mUserSettings->clear();
}
