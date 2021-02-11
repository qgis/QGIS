/***************************************************************************
  qgssettingsentry.cpp
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

#include "qgssettingsentry.h"
#include "qgslogger.h"

QgsSettingsEntry::QgsSettingsEntry( QString key,
                                    QgsSettings::Section settingsSection,
                                    QVariant defaultValue,
                                    QString description,
                                    QObject *parent )
  : QObject( parent )
  , mKey( key )
  , mDefaultValue( defaultValue )
  , mSection( settingsSection )
  , mDescription( description )
{
}

QgsSettingsEntry::QgsSettingsEntry( const QgsSettingsEntry &other )
  : QObject( nullptr )
  , mKey( other.mKey )
  , mDefaultValue( other.mDefaultValue )
  , mSection( other.mSection )
  , mDescription( other.mDescription )
{
}

QgsSettingsEntry &QgsSettingsEntry::operator=( const QgsSettingsEntry &other )
{
  this->mKey = other.mKey;
  this->mSection = other.mSection;
  this->mDefaultValue = other.mDefaultValue;
  this->mDescription = other.mDescription;
  return *this;
}

bool QgsSettingsEntry::setValue( const QVariant &value )
{
  QgsSettings().setValue( mKey,
                          value,
                          mSection );
  return true;
}

QVariant QgsSettingsEntry::valueFromPython() const
{
  return value<QVariant>();
}

QVariant QgsSettingsEntry::defaultValueFromPython() const
{
  return defaultValue<QVariant>();
}

QString QgsSettingsEntry::description() const
{
  return mDescription;
}

void QgsSettingsEntry::remove()
{
  QgsSettings().remove( mKey,
                        mSection );
}

QgsSettingsEntryString::QgsSettingsEntryString(
  const QString &key,
  QgsSettings::Section section,
  const QString &defaultValue,
  const QString &description,
  int minLength,
  int maxLength,
  QObject *parent )
  : QgsSettingsEntry( key,
                      section,
                      defaultValue,
                      description,
                      parent )
  , mMinLength( minLength )
  , mMaxLength( maxLength )
{

}

bool QgsSettingsEntryString::setValue( const QVariant &value )
{
  if ( value.canConvert<QString>() == false )
  {
    QgsDebugMsg( QObject::tr( "Can't convert value '%1' to string for settings with key '%2'" )
                 .arg( value ).arg( key ) );
    return false;
  }

  QString valueString = value.toString();
  if ( valueString.length() < mMinLength )
  {
    QgsDebugMsg( QObject::tr( "Can't set value for settings with key '%1'. String length '%2' is shorter than minimum length '%3'." )
                 .arg( key )
                 .arg( valueString.length() )
                 .arg( mMinLength ) );
    return false;
  }

  if ( mMaxLength >= 0
       && valueString.length() > mMaxLength )
  {
    QgsDebugMsg( QObject::tr( "Can't set value for settings with key '%1'. String length '%2' is longer than maximum length '%3'." )
                 .arg( key )
                 .arg( valueString.length() )
                 .arg( mMinLength ) );
    return false;
  }

  QgsSettingsEntry::setValue( value );
  return true;
}

int QgsSettingsEntryString::minLength()
{
  return mMinLength;
}

int QgsSettingsEntryString::maxLength()
{
  return mMaxLength;
}
