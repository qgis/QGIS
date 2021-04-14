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

QgsSettingsEntryBase::QgsSettingsEntryBase( const QString &key, QgsSettings::Section section, const QVariant &defaultValue, const QString &description )
  : mKey( key )
  , mDefaultValue( defaultValue )
  , mSection( section )
  , mDescription( description )
  , mPluginName()
{
}

QgsSettingsEntryBase::QgsSettingsEntryBase( const QString &key, const QString &pluginName, const QVariant &defaultValue, const QString &description )
  : mKey( key )
  , mDefaultValue( defaultValue )
  , mSection( QgsSettings::Plugins )
  , mDescription( description )
  , mPluginName( pluginName )
{
}

QgsSettingsEntryBase::~QgsSettingsEntryBase()
{
}

QString QgsSettingsEntryBase::key( const QString &dynamicKeyPart ) const
{
  QString completeKey = mKey;
  if ( !mPluginName.isEmpty() )
    completeKey.prepend( mPluginName + "/" );

  if ( dynamicKeyPart.isEmpty() )
  {
    if ( hasDynamicKey() )
      QgsLogger::warning( QStringLiteral( "Settings '%1' have a dynamic key but the dynamic key part was not provided" ).arg( completeKey ) );

    return completeKey;
  }
  else
  {
    if ( !hasDynamicKey() )
    {
      QgsLogger::warning( QStringLiteral( "Settings '%1' don't have a dynamic key, the provided dynamic key part will be ignored" ).arg( completeKey ) );
      return completeKey;
    }

    return completeKey.replace( '%', dynamicKeyPart );
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

bool QgsSettingsEntryBase::setVariantValue( const QVariant &value, const QString &dynamicKeyPart ) const
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

QgsSettingsEntryVariant::QgsSettingsEntryVariant( const QString &key, const QString &pluginName, const QVariant &defaultValue, const QString &description )
  : QgsSettingsEntryBase( key,
                          pluginName,
                          defaultValue,
                          description )
{
}

bool QgsSettingsEntryVariant::setValue( const QVariant &value, const QString &dynamicKeyPart ) const
{
  return QgsSettingsEntryBase::setVariantValue( value, dynamicKeyPart );
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

QgsSettingsEntryString::QgsSettingsEntryString( const QString &key, const QString &pluginName, const QString &defaultValue, const QString &description )
  : QgsSettingsEntryBase( key,
                          pluginName,
                          defaultValue,
                          description )
  , mMinLength( 0 )
  , mMaxLength( -1 )
{
}

bool QgsSettingsEntryString::setValue( const QString &value, const QString &dynamicKeyPart ) const
{
  if ( value.length() < mMinLength )
  {
    QgsDebugMsg( QObject::tr( "Can't set value for settings with key '%1'. String length '%2' is shorter than minimum length '%3'." )
                 .arg( QgsSettingsEntryBase::key(),
                       value.length(),
                       mMinLength ) );
    return false;
  }

  if ( mMaxLength >= 0
       && value.length() > mMaxLength )
  {
    QgsDebugMsg( QObject::tr( "Can't set value for settings with key '%1'. String length '%2' is longer than maximum length '%3'." )
                 .arg( QgsSettingsEntryBase::key(),
                       value.length(),
                       mMinLength ) );
    return false;
  }

  return QgsSettingsEntryBase::setVariantValue( value, dynamicKeyPart );
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

QgsSettingsEntryStringList::QgsSettingsEntryStringList( const QString &key, QgsSettings::Section section, const QStringList &defaultValue, const QString &description )
  : QgsSettingsEntryBase( key,
                          section,
                          defaultValue,
                          description )
{
}

QgsSettingsEntryStringList::QgsSettingsEntryStringList( const QString &key, const QString &pluginName, const QStringList &defaultValue, const QString &description )
  : QgsSettingsEntryBase( key,
                          pluginName,
                          defaultValue,
                          description )
{
}

bool QgsSettingsEntryStringList::setValue( const QStringList &value, const QString &dynamicKeyPart ) const
{
  return QgsSettingsEntryBase::setVariantValue( value, dynamicKeyPart );
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

QgsSettingsEntryBool::QgsSettingsEntryBool( const QString &key, QgsSettings::Section section, bool defaultValue, const QString &description )
  : QgsSettingsEntryBase( key,
                          section,
                          defaultValue,
                          description )
{
}

QgsSettingsEntryBool::QgsSettingsEntryBool( const QString &key, const QString &pluginName, bool defaultValue, const QString &description )
  : QgsSettingsEntryBase( key,
                          pluginName,
                          defaultValue,
                          description )
{
}

bool QgsSettingsEntryBool::setValue( bool value, const QString &dynamicKeyPart ) const
{
  return QgsSettingsEntryBase::setVariantValue( value, dynamicKeyPart );
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

QgsSettingsEntryInteger::QgsSettingsEntryInteger( const QString &key, const QString &pluginName, qlonglong defaultValue, const QString &description )
  : QgsSettingsEntryBase( key,
                          pluginName,
                          defaultValue,
                          description )
  , mMinValue( std::numeric_limits<qlonglong>::min() )
  , mMaxValue( std::numeric_limits<qlonglong>::max() )
{
}

bool QgsSettingsEntryInteger::setValue( qlonglong value, const QString &dynamicKeyPart ) const
{
  if ( value < mMinValue )
  {
    QgsDebugMsg( QObject::tr( "Can't set value for settings with key '%1'. Value '%2' is less than minimum value '%3'." )
                 .arg( QgsSettingsEntryBase::key(),
                       QString::number( value ),
                       QString::number( mMinValue ) ) );
    return false;
  }

  if ( value > mMaxValue )
  {
    QgsDebugMsg( QObject::tr( "Can't set value for settings with key '%1'. Value '%2' is greather than maximum value '%3'." )
                 .arg( QgsSettingsEntryBase::key(),
                       QString::number( value ),
                       QString::number( mMinValue ) ) );
    return false;
  }

  return QgsSettingsEntryBase::setVariantValue( value, dynamicKeyPart );
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

QgsSettingsEntryDouble::QgsSettingsEntryDouble( const QString &key, const QString &pluginName, double defaultValue, const QString &description )
  : QgsSettingsEntryBase( key,
                          pluginName,
                          defaultValue,
                          description )
  , mMinValue( std::numeric_limits<double>::lowest() )
  , mMaxValue( std::numeric_limits<double>::max() )
  , mDisplayHintDecimals( 1 )
{
}

bool QgsSettingsEntryDouble::setValue( double value, const QString &dynamicKeyPart ) const
{
  if ( value < mMinValue )
  {
    QgsDebugMsg( QObject::tr( "Can't set value for settings with key '%1'. Value '%2' is less than minimum value '%3'." )
                 .arg( QgsSettingsEntryBase::key(),
                       QString::number( value ),
                       QString::number( mMinValue ) ) );
    return false;
  }

  if ( value > mMaxValue )
  {
    QgsDebugMsg( QObject::tr( "Can't set value for settings with key '%1'. Value '%2' is greather than maximum value '%3'." )
                 .arg( QgsSettingsEntryBase::key(),
                       QString::number( value ),
                       QString::number( mMinValue ) ) );
    return false;
  }

  return QgsSettingsEntryBase::setVariantValue( value, dynamicKeyPart );
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



