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

Q_GLOBAL_STATIC( QString, sGlobalSettingsPath )

bool QgsSettings::setGlobalSettingsPath( const QString &path )
{
  if ( QFileInfo::exists( path ) )
  {
    *sGlobalSettingsPath() = path;
    return true;
  }
  return false;
}

void QgsSettings::init()
{
  if ( ! sGlobalSettingsPath()->isEmpty() )
  {
    mGlobalSettings = new QSettings( *sGlobalSettingsPath(), QSettings::IniFormat );
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    mGlobalSettings->setIniCodec( "UTF-8" );
#endif
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
  const QString pKey = prefixedKey( prefix, section );
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

QString QgsSettings::group() const
{
  return mUserSettings->group();
}

QStringList QgsSettings::allKeys() const
{
  QStringList keys = mUserSettings->allKeys();
  if ( mGlobalSettings )
  {
    const QStringList constAllKeys = mGlobalSettings->allKeys();
    std::copy_if( constAllKeys.constBegin(), constAllKeys.constEnd(), std::back_inserter( keys ), [&keys]( const QString & key ) {return !keys.contains( key );} );
  }
  return keys;
}


QStringList QgsSettings::childKeys() const
{
  QStringList keys = mUserSettings->childKeys();
  if ( mGlobalSettings )
  {
    const QStringList constChildKeys = mGlobalSettings->childKeys();
    std::copy_if( constChildKeys.constBegin(), constChildKeys.constEnd(), std::back_inserter( keys ), [&keys]( const QString & key ) {return !keys.contains( key );} );
  }
  return keys;
}

QStringList QgsSettings::childGroups() const
{
  QStringList keys = mUserSettings->childGroups();
  if ( mGlobalSettings )
  {
    const QStringList constChildGroups = mGlobalSettings->childGroups();
    std::copy_if( constChildGroups.constBegin(), constChildGroups.constEnd(), std::back_inserter( keys ), [&keys]( const QString & key ) {return !keys.contains( key );} );
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

QString QgsSettings::globalSettingsPath()
{
  return *sGlobalSettingsPath();
}

QVariant QgsSettings::value( const QString &key, const QVariant &defaultValue, const QgsSettings::Section section ) const
{
  const QString pKey = prefixedKey( key, section );
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
  const QString pKey = prefixedKey( key, section );
  return mUserSettings->contains( pKey ) ||
         ( mGlobalSettings && mGlobalSettings->contains( pKey ) );
}

QString QgsSettings::fileName() const
{
  return mUserSettings->fileName();
}

void QgsSettings::sync()
{
  mUserSettings->sync();
}

void QgsSettings::remove( const QString &key, const QgsSettings::Section section )
{
  const QString pKey = prefixedKey( key, section );
  mUserSettings->remove( pKey );
}

QString QgsSettings::prefixedKey( const QString &key, const Section section ) const
{
  QString prefix;
  switch ( section )
  {
    case Section::Core:
      prefix = QStringLiteral( "core" );
      break;
    case Section::Server:
      prefix = QStringLiteral( "server" );
      break;
    case Section::Gui:
      prefix = QStringLiteral( "gui" );
      break;
    case Section::Plugins:
      prefix = QStringLiteral( "plugins" );
      break;
    case Section::Misc:
      prefix = QStringLiteral( "misc" );
      break;
    case Section::Auth:
      prefix = QStringLiteral( "auth" );
      break;
    case Section::App:
      prefix = QStringLiteral( "app" );
      break;
    case Section::Providers:
      prefix = QStringLiteral( "providers" );
      break;
    case Section::Expressions:
      prefix = QStringLiteral( "expressions" );
      break;
    case Section::Gps:
      prefix = QStringLiteral( "gps" );
      break;
    case Section::NoSection:
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
  // Do not store if it hasn't changed from default value
  // First check if the values are different and if at least one of them is valid.
  // The valid check is required because different invalid QVariant types
  // like QVariant(QVariant::String) and QVariant(QVariant::Int))
  // may be considered different and we don't want to store the value in that case.
  const QVariant currentValue = QgsSettings::value( prefixedKey( key, section ) );
  if ( ( currentValue.isValid() || value.isValid() ) && ( currentValue != value ) )
  {
    mUserSettings->setValue( prefixedKey( key, section ), value );
  }
  // Deliberately an "else if" because we want to remove a value from the user settings
  // only if the value is different than the one stored in the global settings (because
  // it would be the default anyway). The first check is necessary because the global settings
  // might be a nullptr (for example in case of standalone scripts or apps).
  else if ( mGlobalSettings && mGlobalSettings->value( prefixedKey( key, section ) ) == currentValue )
  {
    mUserSettings->remove( prefixedKey( key, section ) );
  }
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
