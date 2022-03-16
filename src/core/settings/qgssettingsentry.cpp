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
  return key( dynamicKeyPartToList( dynamicKeyPart ) );
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
    else
      return key == definitionKey();
  }

  const QRegularExpression regularExpression( definitionKey().replace( QRegularExpression( QStringLiteral( "%\\d+" ) ), QStringLiteral( ".*" ) ) );
  const QRegularExpressionMatch regularExpressionMatch = regularExpression.match( key );
  return regularExpressionMatch.hasMatch();
}

QString QgsSettingsEntryBase::definitionKey() const
{
  return mKey;
}

bool QgsSettingsEntryBase::hasDynamicKey() const
{
  const thread_local QRegularExpression regularExpression( QStringLiteral( "%\\d+" ) );
  return mKey.contains( regularExpression );
}

bool QgsSettingsEntryBase::exists( const QString &dynamicKeyPart ) const
{
  return QgsSettings().contains( key( dynamicKeyPart ) );
}

bool QgsSettingsEntryBase::exists( const QStringList &dynamicKeyPartList ) const
{
  return QgsSettings().contains( key( dynamicKeyPartList ) );
}

void QgsSettingsEntryBase::remove( const QString &dynamicKeyPart ) const
{
  QgsSettings().remove( key( dynamicKeyPart ) );
}

void QgsSettingsEntryBase::remove( const QStringList &dynamicKeyPartList ) const
{
  QgsSettings().remove( key( dynamicKeyPartList ) );
}

bool QgsSettingsEntryBase::setVariantValue( const QVariant &value, const QString &dynamicKeyPart ) const
{
  return setVariantValuePrivate( value, dynamicKeyPartToList( dynamicKeyPart ) );
}

bool QgsSettingsEntryBase::setVariantValue( const QVariant &value, const QStringList &dynamicKeyPartList ) const
{
  return setVariantValuePrivate( value, dynamicKeyPartList );
}

bool QgsSettingsEntryBase::setVariantValuePrivate( const QVariant &value, const QStringList &dynamicKeyPartList ) const
{
  if ( mOptions.testFlag( Qgis::SettingsOption::SaveFormerValue ) )
  {
    if ( exists( dynamicKeyPartList ) )
    {
      QVariant currentValue = valueAsVariant( key( dynamicKeyPartList ) );
      if ( value != currentValue )
      {
        QgsSettings().setValue( formerValuekey( dynamicKeyPartList ), currentValue );
      }
    }
  }
  QgsSettings().setValue( key( dynamicKeyPartList ), value );
  return true;
}

QStringList QgsSettingsEntryBase::dynamicKeyPartToList( const QString &dynamicKeyPart ) const
{
  QStringList dynamicKeyPartList;
  if ( !dynamicKeyPart.isNull() )
    dynamicKeyPartList.append( dynamicKeyPart );
  return dynamicKeyPartList;
}

QVariant QgsSettingsEntryBase::valueAsVariant( const QString &dynamicKeyPart ) const
{
  return valueAsVariant( dynamicKeyPartToList( dynamicKeyPart ) );
}

QVariant QgsSettingsEntryBase::valueAsVariant( const QStringList &dynamicKeyPartList ) const
{
  return QgsSettings().value( key( dynamicKeyPartList ), mDefaultValue );
}

QVariant QgsSettingsEntryBase::valueAsVariant( const QString &dynamicKeyPart, bool useDefaultValueOverride, const QVariant &defaultValueOverride ) const
{
  Q_NOWARN_DEPRECATED_PUSH
  return valueAsVariant( dynamicKeyPartToList( dynamicKeyPart ), useDefaultValueOverride, defaultValueOverride );
  Q_NOWARN_DEPRECATED_POP
}

QVariant QgsSettingsEntryBase::valueAsVariant( const QStringList &dynamicKeyPartList, bool useDefaultValueOverride, const QVariant &defaultValueOverride ) const
{
  if ( useDefaultValueOverride )
    return QgsSettings().value( key( dynamicKeyPartList ), defaultValueOverride );
  else
    return QgsSettings().value( key( dynamicKeyPartList ), mDefaultValue );
}

QVariant QgsSettingsEntryBase::valueAsVariantWithDefaultOverride( const QVariant &defaultValueOverride, const QString &dynamicKeyPart ) const
{
  return QgsSettings().value( key( dynamicKeyPart ), defaultValueOverride );
}

QVariant QgsSettingsEntryBase::valueAsVariantWithDefaultOverride( const QVariant &defaultValueOverride, const QStringList &dynamicKeyPartList ) const
{
  return QgsSettings().value( key( dynamicKeyPartList ), defaultValueOverride );
}


QVariant QgsSettingsEntryBase::defaultValueAsVariant() const
{
  return mDefaultValue;
}

QString QgsSettingsEntryBase::description() const
{
  return mDescription;
}

QVariant QgsSettingsEntryBase::formerValueAsVariant( const QString &dynamicKeyPart ) const
{
  return formerValueAsVariant( dynamicKeyPartToList( dynamicKeyPart ) );
}

QVariant QgsSettingsEntryBase::formerValueAsVariant( const QStringList &dynamicKeyPartList ) const
{
  Q_ASSERT( mOptions.testFlag( Qgis::SettingsOption::SaveFormerValue ) );
  QVariant defaultValueOverride = valueAsVariant( key( dynamicKeyPartList ) );
  return  QgsSettings().value( formerValuekey( dynamicKeyPartList ), defaultValueOverride );
}

QString QgsSettingsEntryBase::formerValuekey( const QStringList &dynamicKeyPartList ) const
{
  return key( dynamicKeyPartList ) + QStringLiteral( "_formervalue" );
}


