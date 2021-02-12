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

QgsSettingsRegistry::QgsSettingsRegistry( QgsSettings::Section section, QObject *parent )
  : QObject( parent )
  , mSection( section )
  , mMapSettingsEntry()
{}

QgsSettingsRegistry::~QgsSettingsRegistry()
{
  qDeleteAll( mMapSettingsEntry );
  mMapSettingsEntry.clear();
}

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
                            new QgsSettingsEntry(
                              key,
                              mSection,
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
                            new QgsSettingsEntryString(
                              key,
                              mSection,
                              defaultValue,
                              description,
                              minLength,
                              maxLength,
                              this ) );
}

void QgsSettingsRegistry::registerSettingsInteger( const QString &key,
    qlonglong defaultValue,
    const QString &description,
    qlonglong minValue,
    qlonglong maxValue )
{
  if ( isRegistered( key ) == true )
  {
    QgsDebugMsg( QObject::tr( "Settings key '%1' already registered" ).arg( key ) );
    return;
  }

  mMapSettingsEntry.insert( key,
                            new QgsSettingsEntryInteger(
                              key,
                              mSection,
                              defaultValue,
                              description,
                              minValue,
                              maxValue,
                              this ) );
}

void QgsSettingsRegistry::registerSettingsDouble( const QString &key,
    double defaultValue,
    const QString &description,
    double minValue,
    double maxValue,
    double displayDecimals )
{
  if ( isRegistered( key ) == true )
  {
    QgsDebugMsg( QObject::tr( "Settings key '%1' already registered" ).arg( key ) );
    return;
  }

  mMapSettingsEntry.insert( key,
                            new QgsSettingsEntryDouble(
                              key,
                              mSection,
                              defaultValue,
                              description,
                              minValue,
                              maxValue,
                              displayDecimals,
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

  QgsSettingsEntry *settingsEntry = mMapSettingsEntry.take( key );

  settingsEntry->remove();
  delete settingsEntry;
}

QgsSettingsEntry *QgsSettingsRegistry::settingsEntry( const QString &key ) const
{
  if ( isRegistered( key ) == false )
  {
    QgsDebugMsg( QObject::tr( "No such settings key found in registry '%1'" ).arg( key ) );
    return nullptr;
  }

  return mMapSettingsEntry.value( key );
}

bool QgsSettingsRegistry::setValue( const QString &key,
                                    const QVariant &value )
{
  if ( isRegistered( key ) == false )
  {
    QgsDebugMsg( QObject::tr( "No such settings key found in registry '%1'" ).arg( key ) );
    return false;
  }

  return mMapSettingsEntry.value( key )->setValue( value );
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

  return mMapSettingsEntry.value( key )->description();
}

