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
#include "qgslogger.h"
#include "qgssettings.h"
#include "qgssettingsproxy.h"

Qgis::SettingsType QgsSettingsEntryVariant::settingsType() const
{
  return Qgis::SettingsType::Variant;
}


bool QgsSettingsEntryString::checkValuePrivate( const QString &value ) const
{
  if ( value.length() < mMinLength )
  {
    QgsDebugError( QStringLiteral( "Can't set value for settings. String length '%1' is shorter than minimum length '%2'." )
                   .arg( value.length() )
                   .arg( mMinLength ) );
    return false;
  }

  if ( mMaxLength >= 0
       && value.length() > mMaxLength )
  {
    QgsDebugError( QStringLiteral( "Can't set value for settings. String length '%1' is longer than maximum length '%2'." )
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

int QgsSettingsEntryString::minLength() const
{
  return mMinLength;
}

int QgsSettingsEntryString::maxLength() const
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


bool QgsSettingsEntryInteger::checkValuePrivate( const int &value ) const
{
  if ( value < mMinValue )
  {
    QgsDebugError( QObject::tr( "Can't set value for setting. Value '%1' is less than minimum value '%2'." )
                   .arg( QString::number( value ) )
                   .arg( QString::number( mMinValue ) ) );
    return false;
  }

  if ( value > mMaxValue )
  {
    QgsDebugError( QObject::tr( "Can't set value for setting. Value '%1' is greater than maximum value '%2'." )
                   .arg( QString::number( value ) )
                   .arg( QString::number( mMaxValue ) ) );
    return false;
  }

  return true;
}

int QgsSettingsEntryInteger::convertFromVariant( const QVariant &value ) const
{
  return value.toLongLong();
}

Qgis::SettingsType QgsSettingsEntryInteger::settingsType() const
{
  return Qgis::SettingsType::Integer;
}

int QgsSettingsEntryInteger::maxValue() const
{
  return mMaxValue;
}

int QgsSettingsEntryInteger::minValue() const
{
  return mMaxValue;
}

bool QgsSettingsEntryInteger64::checkValuePrivate( const qlonglong &value ) const
{
  if ( value < mMinValue )
  {
    QgsDebugError( QObject::tr( "Can't set value for setting. Value '%1' is less than minimum value '%2'." )
                   .arg( QString::number( value ) )
                   .arg( QString::number( mMinValue ) ) );
    return false;
  }

  if ( value > mMaxValue )
  {
    QgsDebugError( QObject::tr( "Can't set value for setting. Value '%1' is greater than maximum value '%2'." )
                   .arg( QString::number( value ) )
                   .arg( QString::number( mMaxValue ) ) );
    return false;
  }

  return true;
}

qlonglong QgsSettingsEntryInteger64::convertFromVariant( const QVariant &value ) const
{
  return value.toLongLong();
}

Qgis::SettingsType QgsSettingsEntryInteger64::settingsType() const
{
  return Qgis::SettingsType::Integer;
}

qlonglong QgsSettingsEntryInteger64::maxValue() const
{
  return mMaxValue;
}

qlonglong QgsSettingsEntryInteger64::minValue() const
{
  return mMaxValue;
}



bool QgsSettingsEntryDouble::checkValuePrivate( const double &value ) const
{
  if ( value < mMinValue )
  {
    QgsDebugError( QObject::tr( "Can't set value for setting. Value '%1' is less than minimum value '%2'." )
                   .arg( QString::number( value ), QString::number( mMinValue ) ) );
    return false;
  }

  if ( value > mMaxValue )
  {
    QgsDebugError( QObject::tr( "Can't set value for setting. Value '%1' is greater than maximum value '%2'." )
                   .arg( QString::number( value ), QString::number( mMaxValue ) ) );
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

double QgsSettingsEntryDouble::minValue() const
{
  return mMinValue;
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

bool QgsSettingsEntryColor::checkValuePrivate( const QColor &value ) const
{
  if ( !mAllowAlpha && value.alpha() != 255 )
  {
    QgsDebugError( QStringLiteral( "Setting %1 doesn't allow transparency and the given color has transparency." ).arg( definitionKey() ) );
    return false;
  }

  return true;
}

bool QgsSettingsEntryColor::copyValueFromKeys( const QString &redKey, const QString &greenKey, const QString &blueKey, const QString &alphaKey, bool removeSettingAtKey ) const
{
  auto settings = QgsSettings::get();
  if ( settings->contains( redKey ) && settings->contains( greenKey ) && settings->contains( blueKey ) && ( alphaKey.isNull() || settings->contains( alphaKey ) ) )
  {
    QVariant oldValue;
    if ( alphaKey.isNull() )
      oldValue = QColor( settings->value( redKey ).toInt(), settings->value( greenKey ).toInt(), settings->value( blueKey ).toInt() );
    else
      oldValue = QColor( settings->value( redKey ).toInt(), settings->value( greenKey ).toInt(), settings->value( blueKey ).toInt(), settings->value( alphaKey ).toInt() );

    if ( removeSettingAtKey )
    {
      settings->remove( redKey );
      settings->remove( greenKey );
      settings->remove( blueKey );
      settings->remove( alphaKey );
    }

    if ( value() != oldValue )
      setVariantValue( oldValue );
    return true;
  }
  return false;
}

void QgsSettingsEntryColor::copyValueToKeys( const QString &redKey, const QString &greenKey, const QString &blueKey, const QString &alphaKey ) const
{
  auto settings = QgsSettings::get();
  const QColor color = value();
  settings->setValue( redKey, color.red() );
  settings->setValue( greenKey, color.green() );
  settings->setValue( blueKey, color.blue() );
  if ( !alphaKey.isNull() )
    settings->setValue( alphaKey, color.alpha() );
}

QVariantMap QgsSettingsEntryVariantMap::convertFromVariant( const QVariant &value ) const
{
  return value.value<QVariantMap>();
}

Qgis::SettingsType QgsSettingsEntryVariantMap::settingsType() const
{
  return Qgis::SettingsType::VariantMap;
}

