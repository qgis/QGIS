/***************************************************************************
  qgssettingsentryimpl.cpp
  --------------------------------------
  Date                 : February 2022
  Copyright            : (C) 2022 by Denis Rouzaud
  Email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgssettingsentryimpl.h"


Qgis::SettingsType QgsSettingsEntryVariant::settingsType() const
{
  return Qgis::SettingsType::Variant;
}


bool QgsSettingsEntryString::checkValue( const QString &value ) const
{
  if ( value.length() < mMinLength )
  {
    QgsDebugMsg( QObject::tr( "Can't set value for settings. String length '%1' is shorter than minimum length '%2'." )
                 .arg( value.length() )
                 .arg( mMinLength ) );
    return false;
  }

  if ( mMaxLength >= 0
       && value.length() > mMaxLength )
  {
    QgsDebugMsg( QObject::tr( "Can't set value for settings. String length '%1' is longer than maximum length '%2'." )
                 .arg( value.length() )
                 .arg( mMinLength ) );
    return false;
  }

  return true;
}

QString QgsSettingsEntryString::convertFromVariant( const QVariant &value ) const
{
  return value.toString();
}

Qgis::SettingsType QgsSettingsEntryString::settingsType() const
{
  return Qgis::SettingsType::String;
}

void QgsSettingsEntryString::setMinLength( int minLength )
{
  mMinLength = minLength;
}

int QgsSettingsEntryString::minLength()
{
  return mMinLength;
}

void QgsSettingsEntryString::setMaxLength( int maxLength )
{
  mMaxLength = maxLength;
}

int QgsSettingsEntryString::maxLength()
{
  return mMaxLength;
}



QStringList QgsSettingsEntryStringList::convertFromVariant( const QVariant &value ) const
{
  return value.toStringList();
}

Qgis::SettingsType QgsSettingsEntryStringList::settingsType() const
{
  return Qgis::SettingsType::StringList;
}


bool QgsSettingsEntryBool::convertFromVariant( const QVariant &value ) const
{
  return value.toBool();
}


Qgis::SettingsType QgsSettingsEntryBool::settingsType() const
{
  return Qgis::SettingsType::Bool;
}


bool QgsSettingsEntryInteger::checkValue( qlonglong value ) const
{
  if ( value < mMinValue )
  {
    QgsDebugMsg( QObject::tr( "Can't set value for setting. Value '%1' is less than minimum value '%2'." )
                 .arg( QString::number( value ) )
                 .arg( QString::number( mMinValue ) ) );
    return false;
  }

  if ( value > mMaxValue )
  {
    QgsDebugMsg( QObject::tr( "Can't set value for setting. Value '%1' is greather than maximum value '%2'." )
                 .arg( QString::number( value ) )
                 .arg( QString::number( mMaxValue ) ) );
    return false;
  }

  return true;
}

qlonglong QgsSettingsEntryInteger::convertFromVariant( const QVariant &value ) const
{
  return value.toLongLong();
}

Qgis::SettingsType QgsSettingsEntryInteger::settingsType() const
{
  return Qgis::SettingsType::Integer;
}

void QgsSettingsEntryInteger::setMinValue( qlonglong minValue )
{
  mMinValue = minValue;
}

qlonglong QgsSettingsEntryInteger::minValue()
{
  return mMinValue;
}

void QgsSettingsEntryInteger::setMaxValue( qlonglong maxValue )
{
  mMaxValue = maxValue;
}

qlonglong QgsSettingsEntryInteger::maxValue()
{
  return mMaxValue;
}




bool QgsSettingsEntryDouble::checkValue( double value ) const
{
  if ( value < mMinValue )
  {
    QgsDebugMsg( QObject::tr( "Can't set value for setting. Value '%1' is less than minimum value '%2'." )
                 .arg( QString::number( value ) )
                 .arg( QString::number( mMinValue ) ) );
    return false;
  }

  if ( value > mMaxValue )
  {
    QgsDebugMsg( QObject::tr( "Can't set value for setting. Value '%1' is greather than maximum value '%2'." )
                 .arg( QString::number( value ) )
                 .arg( QString::number( mMaxValue ) ) );
    return false;
  }

  return true;
}

double QgsSettingsEntryDouble::convertFromVariant( const QVariant &value ) const
{
  return value.toDouble();
}

Qgis::SettingsType QgsSettingsEntryDouble::settingsType() const
{
  return Qgis::SettingsType::Double;
}

void QgsSettingsEntryDouble::setMinValue( double minValue )
{
  mMinValue = minValue;
}

double QgsSettingsEntryDouble::minValue() const
{
  return mMinValue;
}

void QgsSettingsEntryDouble::setMaxValue( double maxValue )
{
  mMaxValue = maxValue;
}

double QgsSettingsEntryDouble::maxValue() const
{
  return mMaxValue;
}

void QgsSettingsEntryDouble::setDisplayHintDecimals( int displayHintDecimals )
{
  mDisplayHintDecimals = displayHintDecimals;
}

int QgsSettingsEntryDouble::displayHintDecimals() const
{
  return mDisplayHintDecimals;
}


QColor QgsSettingsEntryColor::convertFromVariant( const QVariant &value ) const
{
  return value.value<QColor>();
}

Qgis::SettingsType QgsSettingsEntryColor::settingsType() const
{
  return Qgis::SettingsType::Color;
}

