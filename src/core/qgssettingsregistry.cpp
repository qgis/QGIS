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
  const QString &settingsName,
  const QVariant &defaultValue,
  const QString &description )
{
  if ( isRegistered( settingsName ) == true )
  {
    QgsDebugMsg( QObject::tr( "Settings name '%1' already registered" ).arg( settingsName ) );
    return;
  }

  mMapSettingsEntry.insert( settingsName,
                            QgsSettingsEntry(
                              settingsName,
                              mSettingsSection,
                              defaultValue,
                              description,
                              this ) );
}

void QgsSettingsRegistry::registerSettingsString(
  const QString &settingsName,
  const QString &defaultValue,
  const QString &description,
  int minLength,
  int maxLength )
{
  if ( isRegistered( settingsName ) == true )
  {
    QgsDebugMsg( QObject::tr( "Settings name '%1' already registered" ).arg( settingsName ) );
    return;
  }

  mMapSettingsEntry.insert( settingsName,
                            QgsSettingsEntry(
                              settingsName,
                              mSettingsSection,
                              defaultValue,
                              description,
                              minLength,
                              maxLength,
                              this ) );
}

bool QgsSettingsRegistry::isRegistered( const QString &settingsName ) const
{
  return mMapSettingsEntry.contains( settingsName );
}

void QgsSettingsRegistry::unregister( const QString &settingsName )
{
  if ( isRegistered( settingsName ) == false )
  {
    QgsDebugMsg( QObject::tr( "No such settings name found in registry '%1'" ).arg( settingsName ) );
    return;
  }

  mMapSettingsEntry.remove( settingsName );
}

bool QgsSettingsRegistry::setValue( const QString &settingsName,
                                    const QVariant &value )
{
  if ( isRegistered( settingsName ) == false )
  {
    QgsDebugMsg( QObject::tr( "No such settings name found in registry '%1'" ).arg( settingsName ) );
    return false;
  }

  mMapSettingsEntry[settingsName].setValue( value );
  return true;
}

#ifdef SIP_RUN
QVariant QgsSettingsRegistry::value( const QString &settingsName ) const
{
  if ( isRegistered( settingsName ) == false )
  {
    QgsDebugMsg( QObject::tr( "No such settings name found in registry '%1'" ).arg( settingsName ) );
    return QVariant();
  }

  return mMapSettingsEntry.value( settingsName ).value();
}
#endif

#ifdef SIP_RUN
QVariant QgsSettingsRegistry::defaultValue( const QString &settingsName ) const
{
  if ( isRegistered( settingsName ) == false )
  {
    QgsDebugMsg( QObject::tr( "No such settings name found in registry '%1'" ).arg( settingsName ) );
    return QVariant();
  }

  return mMapSettingsEntry.value( settingsName ).defaultValue();
}
#endif

QString QgsSettingsRegistry::description( const QString &settingsName ) const
{
  if ( isRegistered( settingsName ) == false )
  {
    QgsDebugMsg( QObject::tr( "No such settings name found in registry '%1'" ).arg( settingsName ) );
    return QString();
  }

  return mMapSettingsEntry.value( settingsName ).description();
}
