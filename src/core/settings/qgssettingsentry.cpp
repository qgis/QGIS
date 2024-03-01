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
#include "qgssettings.h"
#include "qgssettingstreenode.h"
#include "qgssettingsproxy.h"
#include "qgslogger.h"

#include <QRegularExpression>
#include <QDir>


QgsSettingsEntryBase::QgsSettingsEntryBase( const QString &key, QgsSettingsTreeNode *parent, const QVariant &defaultValue, const QString &description, Qgis::SettingsOptions options )
  : mParentTreeElement( parent )
  , mName( key )
  , mDefaultValue( defaultValue )
  , mDescription( description )
  , mOptions( options )
{
  mKey = QDir::cleanPath( QStringLiteral( "%1/%2" ).arg( parent ? parent->completeKey() : QString(), key ) );

  if ( parent )
  {
    parent->registerChildSetting( this, key );
  }
}

QgsSettingsEntryBase::~QgsSettingsEntryBase()
{
  if ( mParentTreeElement )
    mParentTreeElement->unregisterChildSetting( this );
}

QString QgsSettingsEntryBase::typeId() const
{
  return QString::fromUtf8( sSettingsTypeMetaEnum.valueToKey( static_cast<int>( settingsType() ) ) );
}


QString QgsSettingsEntryBase::key( const QString &dynamicKeyPart ) const
{
  return key( dynamicKeyPartToList( dynamicKeyPart ) );
}

QString QgsSettingsEntryBase::key( const QStringList &dynamicKeyPartList ) const
{
  return completeKeyPrivate( mKey, dynamicKeyPartList );
}

QString QgsSettingsEntryBase::completeKeyPrivate( const QString &key, const QStringList &dynamicKeyPartList ) const
{
  QString completeKey = key;

  if ( dynamicKeyPartList.isEmpty() )
  {
    if ( hasDynamicKey() )
      QgsDebugError( QStringLiteral( "Settings '%1' have a dynamic key but the dynamic key part was not provided" ).arg( completeKey ) );

    return completeKey;
  }
  else
  {
    if ( !hasDynamicKey() )
    {
      QgsDebugError( QStringLiteral( "Settings '%1' don't have a dynamic key, the provided dynamic key part will be ignored" ).arg( completeKey ) );
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

  const thread_local QRegularExpression digitRx( QStringLiteral( "%\\d+" ) );
  const QRegularExpression regularExpression( definitionKey().replace( digitRx, QStringLiteral( ".*" ) ) );
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
  return QgsSettings::get()->contains( key( dynamicKeyPart ) );
}

bool QgsSettingsEntryBase::exists( const QStringList &dynamicKeyPartList ) const
{
  return QgsSettings::get()->contains( key( dynamicKeyPartList ) );
}

Qgis::SettingsOrigin QgsSettingsEntryBase::origin( const QStringList &dynamicKeyPartList ) const
{
  return QgsSettings::get()->origin( key( dynamicKeyPartList ) );
}

void QgsSettingsEntryBase::remove( const QString &dynamicKeyPart ) const
{
  QgsSettings::get()->remove( key( dynamicKeyPart ) );
}

void QgsSettingsEntryBase::remove( const QStringList &dynamicKeyPartList ) const
{
  QgsSettings::get()->remove( key( dynamicKeyPartList ) );
}

int QgsSettingsEntryBase::section() const
{
  return QgsSettings::NoSection;
}

bool QgsSettingsEntryBase::setVariantValue( const QVariant &value, const QString &dynamicKeyPart ) const
{
  return setVariantValue( value, dynamicKeyPartToList( dynamicKeyPart ) );
}

bool QgsSettingsEntryBase::setVariantValue( const QVariant &value, const QStringList &dynamicKeyPartList ) const
{
  mHasChanged = true;
  auto settings = QgsSettings::get();
  if ( mOptions.testFlag( Qgis::SettingsOption::SaveFormerValue ) )
  {
    if ( exists( dynamicKeyPartList ) )
    {
      QVariant currentValue = valueAsVariant( key( dynamicKeyPartList ) );
      if ( value != currentValue )
      {
        settings->setValue( formerValuekey( dynamicKeyPartList ), currentValue );
      }
    }
  }
  settings->setValue( key( dynamicKeyPartList ), value );
  return true;
}

QStringList QgsSettingsEntryBase::dynamicKeyPartToList( const QString &dynamicKeyPart )
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
  return QgsSettings::get()->value( key( dynamicKeyPartList ), mDefaultValue );
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
    return QgsSettings::get()->value( key( dynamicKeyPartList ), defaultValueOverride );
  else
    return QgsSettings::get()->value( key( dynamicKeyPartList ), mDefaultValue );
}

QVariant QgsSettingsEntryBase::valueAsVariantWithDefaultOverride( const QVariant &defaultValueOverride, const QString &dynamicKeyPart ) const
{
  return QgsSettings::get()->value( key( dynamicKeyPart ), defaultValueOverride );
}

QVariant QgsSettingsEntryBase::valueAsVariantWithDefaultOverride( const QVariant &defaultValueOverride, const QStringList &dynamicKeyPartList ) const
{
  return QgsSettings::get()->value( key( dynamicKeyPartList ), defaultValueOverride );
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
  return QgsSettings::get()->value( formerValuekey( dynamicKeyPartList ), defaultValueOverride );
}

bool QgsSettingsEntryBase::copyValueFromKey( const QString &key, const QStringList &dynamicKeyPartList, bool removeSettingAtKey ) const
{
  auto settings = QgsSettings::get();

  const QString oldCompleteKey = completeKeyPrivate( key, dynamicKeyPartList );

  if ( settings->contains( oldCompleteKey ) )
  {
    if ( !exists( dynamicKeyPartList ) )
    {
      QVariant oldValue = settings->value( oldCompleteKey, mDefaultValue );
      // do not copy if it is equal to the default value
      if ( oldValue != defaultValueAsVariant() )
        setVariantValue( oldValue, dynamicKeyPartList );
    }
    if ( removeSettingAtKey )
      settings->remove( oldCompleteKey );
    return true;
  }

  return false;
}

void QgsSettingsEntryBase::copyValueToKey( const QString &key, const QStringList &dynamicKeyPartList ) const
{
  const QString completeKey = completeKeyPrivate( key, dynamicKeyPartList );
  QgsSettings::get()->setValue( completeKey, valueAsVariant( dynamicKeyPartList ) );
}

void QgsSettingsEntryBase::copyValueToKeyIfChanged( const QString &key, const QStringList &dynamicKeyPartList ) const
{
  if ( hasChanged() )
  {
    copyValueToKey( key, dynamicKeyPartList );
  }
}

QString QgsSettingsEntryBase::formerValuekey( const QStringList &dynamicKeyPartList ) const
{
  return key( dynamicKeyPartList ) + QStringLiteral( "_formervalue" );
}
