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

QgsSettingsEntryBase::QgsSettingsEntryBase( QString key,
    QgsSettings::Section section,
    QVariant defaultValue,
    QString description )
  : mKey( key )
  , mDefaultValue( defaultValue )
  , mSection( section )
  , mDescription( description )
{
}

QgsSettingsEntryBase::~QgsSettingsEntryBase()
{
}

QString QgsSettingsEntryBase::key( const QString &dynamicKeyPart ) const
{
  if ( dynamicKeyPart.isEmpty() == false )
  {
    if ( hasDynamicKey() == false )
    {
      QgsLogger::warning( QStringLiteral( "Settings '%1' don't have a dynamic key, the provided dynamic key part will be ignored" ).arg( mKey ) );
      return mKey;
    }

    QString completeKey = mKey;
    return completeKey.replace( '%', dynamicKeyPart );
  }
  else
  {
    if ( hasDynamicKey() == true )
      QgsLogger::warning( QStringLiteral( "Settings '%1' have a dynamic key but the dynamic key part was not provided" ).arg( mKey ) );

    return mKey;
  }
}

bool QgsSettingsEntryBase::hasDynamicKey() const
{
  return mKey.contains( '%' );
}

bool QgsSettingsEntryBase::exists( const QString &dynamicKeyPart ) const
{
  return QgsSettings().contains( key( dynamicKeyPart ), section() );
}

void QgsSettingsEntryBase::remove( const QString &dynamicKeyPart ) const
{
  QgsSettings().remove( key( dynamicKeyPart ), section() );
}

QgsSettings::Section QgsSettingsEntryBase::section() const
{
  return mSection;
}

bool QgsSettingsEntryBase::setValue( const QVariant &value, const QString &dynamicKeyPart ) const
{
  QgsSettings().setValue( key( dynamicKeyPart ),
                          value,
                          section() );
  return true;
}

QVariant QgsSettingsEntryBase::valueAsVariant( const QString &dynamicKeyPart ) const
{
  return QgsSettings().value( key( dynamicKeyPart ),
                              mDefaultValue,
                              mSection );
}

QVariant QgsSettingsEntryBase::defaultValueAsVariant() const
{
  return mDefaultValue;
}

QString QgsSettingsEntryBase::description() const
{
  return mDescription;
}


QgsSettingsEntryVariant::QgsSettingsEntryVariant( const QString &key, QgsSettings::Section section, const QVariant &defaultValue, const QString &description )
  : QgsSettingsEntryBase( key,
                          section,
                          defaultValue,
                          description )
{
}

bool QgsSettingsEntryVariant::setValue( const QVariant &value, const QString &dynamicKeyPart ) const
{
  return QgsSettingsEntryBase::setValue( value, dynamicKeyPart );
}

QVariant QgsSettingsEntryVariant::value( const QString &dynamicKeyPart ) const
{
  return valueAsVariant( dynamicKeyPart );
}

QVariant QgsSettingsEntryVariant::defaultValue() const
{
  return defaultValueAsVariant();
}

QgsSettingsEntryBase::SettingsType QgsSettingsEntryVariant::settingsType() const
{
  return QgsSettingsEntryBase::Variant;
}

QgsSettingsEntryString::QgsSettingsEntryString( const QString &key, QgsSettings::Section section, const QString &defaultValue, const QString &description, int minLength, int maxLength )
  : QgsSettingsEntryBase( key,
                          section,
                          defaultValue,
                          description )
  , mMinLength( minLength )
  , mMaxLength( maxLength )
{
}

bool QgsSettingsEntryString::setValue( const QVariant &value, const QString &dynamicKeyPart ) const
{
  if ( value.canConvert<QString>() == false )
  {
    QgsDebugMsg( QObject::tr( "Can't convert value '%1' to string for settings with key '%2'" )
                 .arg( value.toString(),
                       QgsSettingsEntryBase::key() ) );
    return false;
  }

  QString valueString = value.toString();
  if ( valueString.length() < mMinLength )
  {
    QgsDebugMsg( QObject::tr( "Can't set value for settings with key '%1'. String length '%2' is shorter than minimum length '%3'." )
                 .arg( QgsSettingsEntryBase::key(),
                       valueString.length(),
                       mMinLength ) );
    return false;
  }

  if ( mMaxLength >= 0
       && valueString.length() > mMaxLength )
  {
    QgsDebugMsg( QObject::tr( "Can't set value for settings with key '%1'. String length '%2' is longer than maximum length '%3'." )
                 .arg( QgsSettingsEntryBase::key(),
                       valueString.length(),
                       mMinLength ) );
    return false;
  }

  return QgsSettingsEntryBase::setValue( value, dynamicKeyPart );
}

QString QgsSettingsEntryString::value( const QString &dynamicKeyPart ) const
{
  return valueAsVariant( dynamicKeyPart ).toString();
}

QString QgsSettingsEntryString::defaultValue() const
{
  return defaultValueAsVariant().toString();
}

QgsSettingsEntryBase::SettingsType QgsSettingsEntryString::settingsType() const
{
  return QgsSettingsEntryBase::String;
}

int QgsSettingsEntryString::minLength()
{
  return mMinLength;
}

int QgsSettingsEntryString::maxLength()
{
  return mMaxLength;
}

QgsSettingsEntryStringList::QgsSettingsEntryStringList( const QString &key,
    QgsSettings::Section section,
    const QStringList &defaultValue,
    const QString &description )
  : QgsSettingsEntryBase( key,
                          section,
                          defaultValue,
                          description )
{
}

bool QgsSettingsEntryStringList::setValue( const QVariant &value, const QString &dynamicKeyPart ) const
{
  if ( value.canConvert<QStringList>() == false )
  {
    QgsDebugMsg( QObject::tr( "Can't convert value '%1' to string list for settings with key '%2'" )
                 .arg( value.toString(),
                       QgsSettingsEntryBase::key() ) );
    return false;
  }

  return QgsSettingsEntryBase::setValue( value, dynamicKeyPart );
}

QStringList QgsSettingsEntryStringList::value( const QString &dynamicKeyPart ) const
{
  return valueAsVariant( dynamicKeyPart ).toStringList();
}

QStringList QgsSettingsEntryStringList::defaultValue() const
{
  return defaultValueAsVariant().toStringList();
}

QgsSettingsEntryBase::SettingsType QgsSettingsEntryStringList::settingsType() const
{
  return QgsSettingsEntryBase::StringList;
}

QgsSettingsEntryBool::QgsSettingsEntryBool( const QString &key,
    QgsSettings::Section section,
    bool defaultValue,
    const QString &description )
  : QgsSettingsEntryBase( key,
                          section,
                          defaultValue,
                          description )
{
}

bool QgsSettingsEntryBool::setValue( const QVariant &value, const QString &dynamicKeyPart ) const
{
  if ( value.canConvert<bool>() == false )
  {
    QgsDebugMsg( QObject::tr( "Can't convert value '%1' to bool for settings with key '%2'" )
                 .arg( value.toString(),
                       QgsSettingsEntryBase::key() ) );
    return false;
  }

  return QgsSettingsEntryBase::setValue( value, dynamicKeyPart );
}

bool QgsSettingsEntryBool::value( const QString &dynamicKeyPart ) const
{
  return valueAsVariant( dynamicKeyPart ).toBool();
}

bool QgsSettingsEntryBool::defaultValue() const
{
  return defaultValueAsVariant().toBool();
}

QgsSettingsEntryBase::SettingsType QgsSettingsEntryBool::settingsType() const
{
  return QgsSettingsEntryBase::Bool;
}

QgsSettingsEntryInteger::QgsSettingsEntryInteger( const QString &key, QgsSettings::Section section, qlonglong defaultValue, const QString &description, qlonglong minValue, qlonglong maxValue )
  : QgsSettingsEntryBase( key,
                          section,
                          defaultValue,
                          description )
  , mMinValue( minValue )
  , mMaxValue( maxValue )
{

}

bool QgsSettingsEntryInteger::setValue( const QVariant &value, const QString &dynamicKeyPart ) const
{
  if ( value.canConvert<qlonglong>() == false )
  {
    QgsDebugMsg( QObject::tr( "Can't convert value '%1' to qlonglong for settings with key '%2'" )
                 .arg( value.toString(),
                       QgsSettingsEntryBase::key() ) );
    return false;
  }

  qlonglong valueLongLong = value.toLongLong();
  if ( valueLongLong < mMinValue )
  {
    QgsDebugMsg( QObject::tr( "Can't set value for settings with key '%1'. Value '%2' is less than minimum value '%3'." )
                 .arg( QgsSettingsEntryBase::key(),
                       QString::number( valueLongLong ),
                       QString::number( mMinValue ) ) );
    return false;
  }

  if ( valueLongLong > mMaxValue )
  {
    QgsDebugMsg( QObject::tr( "Can't set value for settings with key '%1'. Value '%2' is greather than maximum value '%3'." )
                 .arg( QgsSettingsEntryBase::key(),
                       QString::number( valueLongLong ),
                       QString::number( mMinValue ) ) );
    return false;
  }

  return QgsSettingsEntryBase::setValue( value, dynamicKeyPart );
}

qlonglong QgsSettingsEntryInteger::value( const QString &dynamicKeyPart ) const
{
  return valueAsVariant( dynamicKeyPart ).toLongLong();
}

qlonglong QgsSettingsEntryInteger::defaultValue() const
{
  return defaultValueAsVariant().toLongLong();
}

QgsSettingsEntryBase::SettingsType QgsSettingsEntryInteger::settingsType() const
{
  return QgsSettingsEntryBase::Integer;
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

QgsSettingsEntryDouble::QgsSettingsEntryDouble( const QString &key, QgsSettings::Section section, double defaultValue, const QString &description, double minValue, double maxValue, int displayDecimals )
  : QgsSettingsEntryBase( key,
                          section,
                          defaultValue,
                          description )
  , mMinValue( minValue )
  , mMaxValue( maxValue )
  , mDisplayHintDecimals( displayDecimals )
{

}

bool QgsSettingsEntryDouble::setValue( const QVariant &value, const QString &dynamicKeyPart ) const
{
  if ( value.canConvert<double>() == false )
  {
    QgsDebugMsg( QObject::tr( "Can't convert value '%1' to double for settings with key '%2'" )
                 .arg( value.toString(),
                       QgsSettingsEntryBase::key() ) );
    return false;
  }

  double valueDouble = value.toDouble();
  if ( valueDouble < mMinValue )
  {
    QgsDebugMsg( QObject::tr( "Can't set value for settings with key '%1'. Value '%2' is less than minimum value '%3'." )
                 .arg( QgsSettingsEntryBase::key(),
                       QString::number( valueDouble ),
                       QString::number( mMinValue ) ) );
    return false;
  }

  if ( valueDouble > mMaxValue )
  {
    QgsDebugMsg( QObject::tr( "Can't set value for settings with key '%1'. Value '%2' is greather than maximum value '%3'." )
                 .arg( QgsSettingsEntryBase::key(),
                       QString::number( valueDouble ),
                       QString::number( mMinValue ) ) );
    return false;
  }

  return QgsSettingsEntryBase::setValue( value, dynamicKeyPart );
}

double QgsSettingsEntryDouble::value( const QString &dynamicKeyPart ) const
{
  return valueAsVariant( dynamicKeyPart ).toDouble();
}

double QgsSettingsEntryDouble::defaultValue() const
{
  return defaultValueAsVariant().toDouble();
}

QgsSettingsEntryBase::SettingsType QgsSettingsEntryDouble::settingsType() const
{
  return QgsSettingsEntryBase::Double;
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



