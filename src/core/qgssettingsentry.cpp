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
  , mSettingsSection( settingsSection )
  , mDescription( description )
  , mValueStringMinLength( 0 )
  , mValueStringMaxLength( 1 << 30 )
{
}

QgsSettingsEntry::QgsSettingsEntry( const QString &key,
                                    QgsSettings::Section settingsSection,
                                    const QString &defaultValue,
                                    const QString &description,
                                    int minLength,
                                    int maxLength,
                                    QObject *parent )
  : QObject( parent )
  , mKey( key )
  , mDefaultValue( defaultValue )
  , mSettingsSection( settingsSection )
  , mDescription( description )
  , mValueStringMinLength( minLength )
  , mValueStringMaxLength( maxLength )
{
}

QgsSettingsEntry::QgsSettingsEntry( const QgsSettingsEntry &other )
  : QObject( nullptr )
  , mKey( other.mKey )
  , mDefaultValue( other.mDefaultValue )
  , mSettingsSection( other.mSettingsSection )
  , mDescription( other.mDescription )
  , mValueStringMinLength( 0 )
  , mValueStringMaxLength( 1 << 30 )
{
}

QgsSettingsEntry &QgsSettingsEntry::operator=( const QgsSettingsEntry &other )
{
  this->mKey = other.mKey;
  this->mSettingsSection = other.mSettingsSection;
  this->mDefaultValue = other.mDefaultValue;
  this->mDescription = other.mDescription;
  this->mValueStringMinLength = other.mValueStringMinLength;
  this->mValueStringMaxLength = other.mValueStringMaxLength;
  return *this;
}

void QgsSettingsEntry::setValue( const QVariant &value )
{
  QgsSettings().setValue( mKey,
                          value,
                          mSettingsSection );
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
