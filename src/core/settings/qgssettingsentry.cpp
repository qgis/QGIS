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
  QgsSettings().setValue( key( dynamicKeyPartList ), value, section() );
  return true;
}

QVariant QgsSettingsEntryBase::valueAsVariant( const QString &dynamicKeyPart ) const
{
  QStringList dynamicKeyPartList;
  if ( !dynamicKeyPart.isNull() )
    dynamicKeyPartList.append( dynamicKeyPart );

  return valueAsVariant( dynamicKeyPartList );
}

QVariant QgsSettingsEntryBase::valueAsVariant( const QStringList &dynamicKeyPartList ) const
{
  return QgsSettings().value( key( dynamicKeyPartList ), mDefaultValue, mSection );
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
    return QgsSettings().value( key( dynamicKeyPartList ), defaultValueOverride, mSection );
  else
    return QgsSettings().value( key( dynamicKeyPartList ), mDefaultValue, mSection );
}

QVariant QgsSettingsEntryBase::valueAsVariantWithDefaultOverride( const QVariant &defaultValueOverride ) const
{
  return QgsSettings().value( key(), defaultValueOverride, mSection );
}

QVariant QgsSettingsEntryBase::valueAsVariantWithDefaultOverride( const QString &dynamicKeyPart, const QVariant &defaultValueOverride ) const
{
  return QgsSettings().value( key( dynamicKeyPart ), defaultValueOverride, mSection );
}

QVariant QgsSettingsEntryBase::valueAsVariantWithDefaultOverride( const QStringList &dynamicKeyPartList, const QVariant &defaultValueOverride ) const
{
  return QgsSettings().value( key( dynamicKeyPartList ), defaultValueOverride, mSection );
}




QVariant QgsSettingsEntryBase::defaultValueAsVariant() const
{
  return mDefaultValue;
}

QString QgsSettingsEntryBase::description() const
{
  return mDescription;
}



