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

#include <QRegularExpression>

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
  QStringList dynamicKeyPartList;
  if ( !dynamicKeyPart.isNull() )
    dynamicKeyPartList.append( dynamicKeyPart );

  return key( dynamicKeyPartList );
}

QString QgsSettingsEntryBase::key( const QStringList &dynamicKeyPartList ) const
{
  QString completeKey = mKey;
  if ( !mPluginName.isEmpty() )
  {
    if ( !completeKey.startsWith( '/' ) )
      completeKey.prepend( '/' );
    completeKey.prepend( mPluginName );
  }

  if ( dynamicKeyPartList.isEmpty() )
  {
    if ( hasDynamicKey() )
      QgsDebugMsg( QStringLiteral( "Settings '%1' have a dynamic key but the dynamic key part was not provided" ).arg( completeKey ) );

    return completeKey;
  }
  else
  {
    if ( !hasDynamicKey() )
    {
      QgsDebugMsg( QStringLiteral( "Settings '%1' don't have a dynamic key, the provided dynamic key part will be ignored" ).arg( completeKey ) );
      return completeKey;
    }

    for ( int i = 0; i < dynamicKeyPartList.size(); i++ )
    {
      completeKey.replace( QStringLiteral( "%" ).append( QString::number( i + 1 ) ), dynamicKeyPartList.at( i ) );
    }
  }
  return completeKey;
}

bool QgsSettingsEntryBase::keyIsValid( const QString &key ) const
{
  if ( !hasDynamicKey() )
  {
    if ( !key.contains( definitionKey() ) )
      return false;
  }

  // Key to check
  QString completeKeyToCheck = key;

  QString settingsPrefix = QgsSettings().prefixedKey( QString(), section() );
  settingsPrefix.chop( 1 );
  if ( !completeKeyToCheck.startsWith( settingsPrefix ) )
  {
    if ( !mPluginName.isEmpty()
         && !completeKeyToCheck.startsWith( mPluginName ) )
    {
      if ( !completeKeyToCheck.startsWith( '/' ) )
        completeKeyToCheck.prepend( '/' );
      completeKeyToCheck.prepend( mPluginName );
    }

    if ( !completeKeyToCheck.startsWith( '/' ) )
      completeKeyToCheck.prepend( '/' );
    completeKeyToCheck.prepend( settingsPrefix );
  }

  // Prefixed settings key
  QString prefixedSettingsKey = definitionKey();
  if ( !prefixedSettingsKey.startsWith( settingsPrefix ) )
  {
    if ( !prefixedSettingsKey.startsWith( '/' ) )
      prefixedSettingsKey.prepend( '/' );
    prefixedSettingsKey.prepend( settingsPrefix );
  }

  if ( !hasDynamicKey() )
    return completeKeyToCheck == prefixedSettingsKey;

  const QRegularExpression regularExpression( prefixedSettingsKey.replace( QRegularExpression( QStringLiteral( "%\\d+" ) ), QStringLiteral( ".*" ) ) );
  const QRegularExpressionMatch regularExpressionMatch = regularExpression.match( completeKeyToCheck );
  return regularExpressionMatch.hasMatch();
}

QString QgsSettingsEntryBase::definitionKey() const
{
  QString completeKey = mKey;
  if ( !mPluginName.isEmpty() )
  {
    if ( !completeKey.startsWith( '/' ) )
      completeKey.prepend( '/' );
    completeKey.prepend( mPluginName );
  }

  return completeKey;
}

bool QgsSettingsEntryBase::hasDynamicKey() const
{
  const thread_local QRegularExpression regularExpression( QStringLiteral( "%\\d+" ) );
  return mKey.contains( regularExpression );
}

bool QgsSettingsEntryBase::exists( const QString &dynamicKeyPart ) const
{
  return QgsSettings().contains( key( dynamicKeyPart ), section() );
}

bool QgsSettingsEntryBase::exists( const QStringList &dynamicKeyPartList ) const
{
  return QgsSettings().contains( key( dynamicKeyPartList ), section() );
}

void QgsSettingsEntryBase::remove( const QString &dynamicKeyPart ) const
{
  QgsSettings().remove( key( dynamicKeyPart ), section() );
}

void QgsSettingsEntryBase::remove( const QStringList &dynamicKeyPartList ) const
{
  QgsSettings().remove( key( dynamicKeyPartList ), section() );
}

QgsSettings::Section QgsSettingsEntryBase::section() const
{
  return mSection;
}

bool QgsSettingsEntryBase::setVariantValue( const QVariant &value, const QString &dynamicKeyPart ) const
{
  QStringList dynamicKeyPartList;
  if ( !dynamicKeyPart.isNull() )
    dynamicKeyPartList.append( dynamicKeyPart );

  return setVariantValue( value, dynamicKeyPartList );
}

bool QgsSettingsEntryBase::setVariantValue( const QVariant &value, const QStringList &dynamicKeyPartList ) const
{
  QgsSettings().setValue( key( dynamicKeyPartList ),
                          value,
                          section() );
  return true;
}

QVariant QgsSettingsEntryBase::valueAsVariant( const QString &dynamicKeyPart, bool useDefaultValueOverride, const QVariant &defaultValueOverride ) const
{
  QStringList dynamicKeyPartList;
  if ( !dynamicKeyPart.isNull() )
    dynamicKeyPartList.append( dynamicKeyPart );

  return valueAsVariant( dynamicKeyPartList, useDefaultValueOverride, defaultValueOverride );
}

QVariant QgsSettingsEntryBase::valueAsVariant( const QStringList &dynamicKeyPartList, bool useDefaultValueOverride, const QVariant &defaultValueOverride ) const
{
  if ( useDefaultValueOverride )
    return QgsSettings().value( key( dynamicKeyPartList ),
                                defaultValueOverride,
                                mSection );
  else
    return QgsSettings().value( key( dynamicKeyPartList ),
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

bool QgsSettingsEntryVariant::setValue( const QVariant &value, const QStringList &dynamicKeyPartList ) const
{
  return QgsSettingsEntryBase::setVariantValue( value, dynamicKeyPartList );
}

QVariant QgsSettingsEntryVariant::value( const QString &dynamicKeyPart, bool useDefaultValueOverride, const QVariant &defaultValueOverride ) const
{
  return valueAsVariant( dynamicKeyPart, useDefaultValueOverride, defaultValueOverride );
}

QVariant QgsSettingsEntryVariant::value( const QStringList &dynamicKeyPartList, bool useDefaultValueOverride, const QVariant &defaultValueOverride ) const
{
  return valueAsVariant( dynamicKeyPartList, useDefaultValueOverride, defaultValueOverride );
}

QVariant QgsSettingsEntryVariant::defaultValue() const
{
  return defaultValueAsVariant();
}

QgsSettingsEntryBase::SettingsType QgsSettingsEntryVariant::settingsType() const
{
  return QgsSettingsEntryBase::SettingsType::Variant;
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
  QStringList dynamicKeyPartList;
  if ( !dynamicKeyPart.isNull() )
    dynamicKeyPartList.append( dynamicKeyPart );

  return setValue( value, dynamicKeyPartList );
}

bool QgsSettingsEntryString::setValue( const QString &value, const QStringList &dynamicKeyPartList ) const
{
  if ( value.length() < mMinLength )
  {
    QgsDebugMsg( QObject::tr( "Can't set value for settings with key '%1'. String length '%2' is shorter than minimum length '%3'." )
                 .arg( QgsSettingsEntryBase::key( dynamicKeyPartList ) )
                 .arg( value.length() )
                 .arg( mMinLength ) );
    return false;
  }

  if ( mMaxLength >= 0
       && value.length() > mMaxLength )
  {
    QgsDebugMsg( QObject::tr( "Can't set value for settings with key '%1'. String length '%2' is longer than maximum length '%3'." )
                 .arg( QgsSettingsEntryBase::key( dynamicKeyPartList ) )
                 .arg( value.length() )
                 .arg( mMinLength ) );
    return false;
  }

  return QgsSettingsEntryBase::setVariantValue( value, dynamicKeyPartList );
}

QString QgsSettingsEntryString::value( const QString &dynamicKeyPart, bool useDefaultValueOverride, const QString &defaultValueOverride ) const
{
  return valueAsVariant( dynamicKeyPart, useDefaultValueOverride, defaultValueOverride ).toString();
}

QString QgsSettingsEntryString::value( const QStringList &dynamicKeyPartList, bool useDefaultValueOverride, const QString &defaultValueOverride ) const
{
  return valueAsVariant( dynamicKeyPartList, useDefaultValueOverride, defaultValueOverride ).toString();
}

QString QgsSettingsEntryString::defaultValue() const
{
  return defaultValueAsVariant().toString();
}

QgsSettingsEntryBase::SettingsType QgsSettingsEntryString::settingsType() const
{
  return QgsSettingsEntryBase::SettingsType::String;
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

bool QgsSettingsEntryStringList::setValue( const QStringList &value, const QStringList &dynamicKeyPartList ) const
{
  return QgsSettingsEntryBase::setVariantValue( value, dynamicKeyPartList );
}

QStringList QgsSettingsEntryStringList::value( const QString &dynamicKeyPart, bool useDefaultValueOverride, const QStringList &defaultValueOverride ) const
{
  return valueAsVariant( dynamicKeyPart, useDefaultValueOverride, defaultValueOverride ).toStringList();
}

QStringList QgsSettingsEntryStringList::value( const QStringList &dynamicKeyPartList, bool useDefaultValueOverride, const QStringList &defaultValueOverride ) const
{
  return valueAsVariant( dynamicKeyPartList, useDefaultValueOverride, defaultValueOverride ).toStringList();
}

QStringList QgsSettingsEntryStringList::defaultValue() const
{
  return defaultValueAsVariant().toStringList();
}

QgsSettingsEntryBase::SettingsType QgsSettingsEntryStringList::settingsType() const
{
  return QgsSettingsEntryBase::SettingsType::StringList;
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

bool QgsSettingsEntryBool::setValue( bool value, const QStringList &dynamicKeyPartList ) const
{
  return QgsSettingsEntryBase::setVariantValue( value, dynamicKeyPartList );
}

bool QgsSettingsEntryBool::value( const QString &dynamicKeyPart, bool useDefaultValueOverride, bool defaultValueOverride ) const
{
  return valueAsVariant( dynamicKeyPart, useDefaultValueOverride, defaultValueOverride ).toBool();
}

bool QgsSettingsEntryBool::value( const QStringList &dynamicKeyPartList, bool useDefaultValueOverride, bool defaultValueOverride ) const
{
  return valueAsVariant( dynamicKeyPartList, useDefaultValueOverride, defaultValueOverride ).toBool();
}

bool QgsSettingsEntryBool::defaultValue() const
{
  return defaultValueAsVariant().toBool();
}

QgsSettingsEntryBase::SettingsType QgsSettingsEntryBool::settingsType() const
{
  return QgsSettingsEntryBase::SettingsType::Bool;
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
  QStringList dynamicKeyPartList;
  if ( !dynamicKeyPart.isNull() )
    dynamicKeyPartList.append( dynamicKeyPart );

  return setValue( value, dynamicKeyPartList );
}

bool QgsSettingsEntryInteger::setValue( qlonglong value, const QStringList &dynamicKeyPartList ) const
{
  if ( value < mMinValue )
  {
    QgsDebugMsg( QObject::tr( "Can't set value for settings with key '%1'. Value '%2' is less than minimum value '%3'." )
                 .arg( QgsSettingsEntryBase::key( dynamicKeyPartList ),
                       QString::number( value ),
                       QString::number( mMinValue ) ) );
    return false;
  }

  if ( value > mMaxValue )
  {
    QgsDebugMsg( QObject::tr( "Can't set value for settings with key '%1'. Value '%2' is greather than maximum value '%3'." )
                 .arg( QgsSettingsEntryBase::key( dynamicKeyPartList ),
                       QString::number( value ),
                       QString::number( mMinValue ) ) );
    return false;
  }

  return QgsSettingsEntryBase::setVariantValue( value, dynamicKeyPartList );
}

qlonglong QgsSettingsEntryInteger::value( const QString &dynamicKeyPart, bool useDefaultValueOverride, qlonglong defaultValueOverride ) const
{
  return valueAsVariant( dynamicKeyPart, useDefaultValueOverride, defaultValueOverride ).toLongLong();
}

qlonglong QgsSettingsEntryInteger::value( const QStringList &dynamicKeyPartList, bool useDefaultValueOverride, qlonglong defaultValueOverride ) const
{
  return valueAsVariant( dynamicKeyPartList, useDefaultValueOverride, defaultValueOverride ).toLongLong();
}

qlonglong QgsSettingsEntryInteger::defaultValue() const
{
  return defaultValueAsVariant().toLongLong();
}

QgsSettingsEntryBase::SettingsType QgsSettingsEntryInteger::settingsType() const
{
  return QgsSettingsEntryBase::SettingsType::Integer;
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
  QStringList dynamicKeyPartList;
  if ( !dynamicKeyPart.isNull() )
    dynamicKeyPartList.append( dynamicKeyPart );

  return setValue( value, dynamicKeyPartList );
}

bool QgsSettingsEntryDouble::setValue( double value, const QStringList &dynamicKeyPartList ) const
{
  if ( value < mMinValue )
  {
    QgsDebugMsg( QObject::tr( "Can't set value for settings with key '%1'. Value '%2' is less than minimum value '%3'." )
                 .arg( QgsSettingsEntryBase::key( dynamicKeyPartList ),
                       QString::number( value ),
                       QString::number( mMinValue ) ) );
    return false;
  }

  if ( value > mMaxValue )
  {
    QgsDebugMsg( QObject::tr( "Can't set value for settings with key '%1'. Value '%2' is greather than maximum value '%3'." )
                 .arg( QgsSettingsEntryBase::key( dynamicKeyPartList ),
                       QString::number( value ),
                       QString::number( mMinValue ) ) );
    return false;
  }

  return QgsSettingsEntryBase::setVariantValue( value, dynamicKeyPartList );
}

double QgsSettingsEntryDouble::value( const QString &dynamicKeyPart, bool useDefaultValueOverride, double defaultValueOverride ) const
{
  return valueAsVariant( dynamicKeyPart, useDefaultValueOverride, defaultValueOverride ).toDouble();
}

double QgsSettingsEntryDouble::value( const QStringList &dynamicKeyPartList, bool useDefaultValueOverride, double defaultValueOverride ) const
{
  return valueAsVariant( dynamicKeyPartList, useDefaultValueOverride, defaultValueOverride ).toDouble();
}

double QgsSettingsEntryDouble::defaultValue() const
{
  return defaultValueAsVariant().toDouble();
}

QgsSettingsEntryBase::SettingsType QgsSettingsEntryDouble::settingsType() const
{
  return QgsSettingsEntryBase::SettingsType::Double;
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

QgsSettingsEntryColor::QgsSettingsEntryColor( const QString &key, QgsSettings::Section section, const QColor &defaultValue, const QString &description )
  : QgsSettingsEntryBase( key,
                          section,
                          defaultValue,
                          description )
{
}

QgsSettingsEntryColor::QgsSettingsEntryColor( const QString &key, const QString &pluginName, const QColor &defaultValue, const QString &description )
  : QgsSettingsEntryBase( key,
                          pluginName,
                          defaultValue,
                          description )
{
}

bool QgsSettingsEntryColor::setValue( const QColor &value, const QString &dynamicKeyPart ) const
{
  QStringList dynamicKeyPartList;
  if ( !dynamicKeyPart.isNull() )
    dynamicKeyPartList.append( dynamicKeyPart );

  return setValue( value, dynamicKeyPartList );
}

bool QgsSettingsEntryColor::setValue( const QColor &value, const QStringList &dynamicKeyPartList ) const
{
  return QgsSettingsEntryBase::setVariantValue( value, dynamicKeyPartList );
}

QColor QgsSettingsEntryColor::value( const QString &dynamicKeyPart, bool useDefaultValueOverride, const QString &defaultValueOverride ) const
{
  return valueAsVariant( dynamicKeyPart, useDefaultValueOverride, defaultValueOverride ).value<QColor>();
}

QColor QgsSettingsEntryColor::value( const QStringList &dynamicKeyPartList, bool useDefaultValueOverride, const QString &defaultValueOverride ) const
{
  return valueAsVariant( dynamicKeyPartList, useDefaultValueOverride, defaultValueOverride ).value<QColor>();
}

QColor QgsSettingsEntryColor::defaultValue() const
{
  return defaultValueAsVariant().value<QColor>();
}

QgsSettingsEntryBase::SettingsType QgsSettingsEntryColor::settingsType() const
{
  return QgsSettingsEntryBase::SettingsType::Color;
}

