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

QgsSettingsEntry::QgsSettingsEntry( QString settingsName,
                                    QgsSettings::Section settingsSection,
                                    QVariant defaultValue,
                                    QString description,
                                    QObject *parent )
  : QObject( parent )
  , mSettingsName( settingsName )
  , mSettingsSection( settingsSection )
  , mDefaultValue( defaultValue )
  , mDescription( description )
{
}

QgsSettingsEntry::QgsSettingsEntry( const QgsSettingsEntry &other )
  : QObject( nullptr )
  , mSettingsName( other.mSettingsName )
  , mSettingsSection( other.mSettingsSection )
  , mDefaultValue( other.mDefaultValue )
  , mDescription( other.mDescription )
{
}

QgsSettingsEntry &QgsSettingsEntry::operator=( const QgsSettingsEntry &other )
{
  this->mSettingsName = other.mSettingsName;
  this->mSettingsSection = other.mSettingsSection;
  this->mDefaultValue = other.mDefaultValue;
  this->mDescription = other.mDescription;
}

void QgsSettingsEntry::setValue( const QVariant &value )
{
  QgsSettings().setValue( mSettingsName,
                          value,
                          mSettingsSection );
}

QVariant QgsSettingsEntry::value() const
{
  return QgsSettings().value( mSettingsName,
                              mDefaultValue,
                              mSettingsSection );
}
