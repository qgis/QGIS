/***************************************************************************
  qgssettingsregistry.cpp
  --------------------------------------
  Date                 : February 2021
  Copyright            : (C) 2021 by Damiano Lombardi
  Email                : damiano at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssettingsregistry.h"
#include "qgslogger.h"

QgsSettingsRegistry::QgsSettingsRegistry( QgsSettings::Section settingsSection, QObject *parent )
  : QObject( parent )
  , mSettingsSection( settingsSection )
  , mMapSettingsEntry()
{}

void QgsSettingsRegistry::registerSettings(
  const QString &key,
  const QVariant &defaultValue,
  const QString &description )
{
  if ( isRegistered( key ) == true )
  {
    QgsDebugMsg( QObject::tr( "Settings key '%1' already registered" ).arg( key ) );
    return;
  }

  mMapSettingsEntry.insert( key,
                            QgsSettingsEntry(
                              key,
                              mSettingsSection,
                              defaultValue,
                              description,
                              this ) );
}

void QgsSettingsRegistry::registerSettingsString( const QString &key,
    const QString &defaultValue,
    const QString &description,
    int minLength,
    int maxLength )
{
  if ( isRegistered( key ) == true )
  {
    QgsDebugMsg( QObject::tr( "Settings key '%1' already registered" ).arg( key ) );
    return;
  }

  mMapSettingsEntry.insert( key,
                            QgsSettingsEntry(
                              key,
                              mSettingsSection,
                              defaultValue,
                              description,
                              minLength,
                              maxLength,
                              this ) );
}

bool QgsSettingsRegistry::isRegistered( const QString &key ) const
{
  return mMapSettingsEntry.contains( key );
}

void QgsSettingsRegistry::unregister( const QString &key )
{
  if ( isRegistered( key ) == false )
  {
    QgsDebugMsg( QObject::tr( "No such settings key found in registry '%1'" ).arg( key ) );
    return;
  }

  mMapSettingsEntry.remove( key );
}

bool QgsSettingsRegistry::setValue( const QString &key,
                                    const QVariant &value )
{
  if ( isRegistered( key ) == false )
  {
    QgsDebugMsg( QObject::tr( "No such settings key found in registry '%1'" ).arg( key ) );
    return false;
  }

  mMapSettingsEntry[key].setValue( value );
  return true;
}

QVariant QgsSettingsRegistry::valueFromPython( const QString &key ) const
{
  return value<QVariant>( key );
}

QVariant QgsSettingsRegistry::defaultValueFromPython( const QString &key ) const
{
  return defaultValue<QVariant>( key );
}

QString QgsSettingsRegistry::description( const QString &key ) const
{
  if ( isRegistered( key ) == false )
  {
    QgsDebugMsg( QObject::tr( "No such settings key found in registry '%1'" ).arg( key ) );
    return QString();
  }

  return mMapSettingsEntry.value( key ).description();
}
