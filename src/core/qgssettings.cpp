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


#include <stdlib.h>

#include "qgssettings.h"
#include <QFileInfo>
#include <QSettings>
#include <QDir>

QString QgsSettings::sGlobalSettingsPath = QString();

bool QgsSettings::setGlobalSettingsPath( QString path )
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
  if ( ! sGlobalSettingsPath.isEmpty( ) )
  {
    mGlobalSettings = new QSettings( sGlobalSettingsPath, QSettings::IniFormat );
    mGlobalSettings->setIniCodec( "UTF-8" );
  }
}


QgsSettings::QgsSettings( const QString &organization, const QString &application, QObject *parent ):
    QSettings( organization, application, parent )
{
  init( );
}

QgsSettings::QgsSettings( QSettings::Scope scope, const QString &organization,
                          const QString &application, QObject *parent ) :
    QSettings( scope, organization, application, parent )
{
  init( );
}

QgsSettings::QgsSettings( QSettings::Format format, QSettings::Scope scope,
                          const QString &organization, const QString &application, QObject *parent ) :
    QSettings( format, scope, organization, application, parent )
{
  init( );
}

QgsSettings::QgsSettings( const QString &fileName, QSettings::Format format, QObject *parent ) :
    QSettings( fileName, format, parent )
{
  init( );
}

QgsSettings::QgsSettings( QObject *parent ) :
    QSettings( parent )
{
  init( );
}

QgsSettings::~QgsSettings()
{
  delete mGlobalSettings;
}


void QgsSettings::beginGroup( const QString &prefix )
{
  QSettings::beginGroup( prefix );
  if ( mGlobalSettings )
  {
    mGlobalSettings->beginGroup( prefix );
  }
}

void QgsSettings::endGroup()
{
  QSettings::endGroup( );
  if ( mGlobalSettings )
  {
    mGlobalSettings->endGroup( );
  }
}


QStringList QgsSettings::allKeys() const
{
  QStringList keys = QSettings::allKeys( );
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
  QStringList keys = QSettings::childKeys( );
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
  QStringList keys = QSettings::childGroups( );
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


QVariant QgsSettings::value( const QString &key, const QVariant &defaultValue ) const
{
  if ( ! QSettings::value( key ).isNull() )
  {
    return QSettings::value( key );
  }
  if ( mGlobalSettings )
  {
    return mGlobalSettings->value( key, defaultValue );
  }
  return defaultValue;
}

QVariant QgsSettings::sectionValue( const QString &key, const Section section, const QVariant &defaultValue ) const
{
  return value( prefixedKey( key, section ), defaultValue );
}

QVariant QgsSettings::coreValue( const QString &key, const QVariant &defaultValue ) const
{
  return sectionValue( key, Section::Core, defaultValue );
}

QVariant QgsSettings::serverValue( const QString &key, const QVariant &defaultValue ) const
{
  return sectionValue( key, Section::Server, defaultValue );
}

QVariant QgsSettings::guiValue( const QString &key, const QVariant &defaultValue ) const
{
  return sectionValue( key, Section::Gui, defaultValue );
}

QVariant QgsSettings::pluginsValue( const QString &key, const QVariant &defaultValue ) const
{
  return sectionValue( key, Section::Plugins, defaultValue );
}

QVariant QgsSettings::miscValue( const QString &key, const QVariant &defaultValue ) const
{
  return sectionValue( key, Section::Misc, defaultValue );
}

void QgsSettings::setSectionValue( const QString &key, const Section section, const QVariant &value )
{
  QSettings::setValue( prefixedKey( key, section ), value );
}

void QgsSettings::setCoreValue( const QString &key, const QVariant &value )
{
  setSectionValue( key , Section::Core, value );
}

void QgsSettings::setServerValue( const QString &key, const QVariant &value )
{
  setSectionValue( key , Section::Server, value );
}

void QgsSettings::setGuiValue( const QString &key, const QVariant &value )
{
  setSectionValue( key , Section::Gui, value );
}

void QgsSettings::setPluginsValue( const QString &key, const QVariant &value )
{
  setSectionValue( key , Section::Plugins, value );
}

void QgsSettings::setMiscValue( const QString &key, const QVariant &value )
{
  setSectionValue( key , Section::Misc, value );
}

QString QgsSettings::prefixedKey( const QString &key, const Section section ) const
{
  QString prefix;
  switch ( section )
  {
    case Section::Core :
      prefix = "core";
      break;
    case Section::Server :
      prefix = "server";
      break;
    case Section::Gui :
      prefix = "gui";
      break;
    case Section::Plugins :
      prefix = "plugins";
      break;
    case Section::Misc :
      prefix = "misc";
      break;
    default:
      break;
  }
  return prefix  + "/" + sanitizeKey( key );
}


int QgsSettings::beginReadArray( const QString &prefix )
{
  int size = QSettings::beginReadArray( prefix );
  if ( 0 == size && mGlobalSettings )
  {
    size = mGlobalSettings->beginReadArray( prefix );
    mUsingGlobalArray = ( size > 0 );
  }
  return size;
}

void QgsSettings::endArray()
{
  QSettings::endArray();
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
    QSettings::setArrayIndex( i );
  }
}

// To lower case and clean the path
QString QgsSettings::sanitizeKey( QString key ) const
{
  return QDir::cleanPath( key.toLower() );
}
