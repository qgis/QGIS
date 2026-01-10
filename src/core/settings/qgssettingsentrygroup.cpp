/***************************************************************************
  qgssettingsentrygroup.cpp
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


#include "qgssettingsentrygroup.h"

#include "qgslogger.h"
#include "qgssettings.h"
#include "qgssettingsentry.h"

#include <QDir>
#include <QRegularExpression>

QgsSettingsEntryGroup::QgsSettingsEntryGroup( QList<const QgsSettingsEntryBase *> settings )
  : QgsSettingsEntryGroup( settings, true )
{

}

QgsSettingsEntryGroup::QgsSettingsEntryGroup( QList<const QgsSettingsEntryBase *> settings, bool fatalErrorIfInvalid )
  : mSettings( settings )
{
  for ( const auto *setting : std::as_const( mSettings ) )
  {
    QString otherBaseKey = setting->definitionKey();
    otherBaseKey = otherBaseKey.left( otherBaseKey.lastIndexOf( '/'_L1 ) );
    if ( mDefinitionBaseKey.isEmpty() )
    {
      mDefinitionBaseKey = otherBaseKey;
    }
    else
    {
      if ( mDefinitionBaseKey != otherBaseKey )
      {
        QgsDebugError( "Settings do not share the same base definition key for this group. This will lead to unpredictable results." );
        if ( fatalErrorIfInvalid )
          Q_ASSERT( false );
        mIsValid = false;
      }
    }
  }
}

QString QgsSettingsEntryGroup::baseKey( const QStringList &dynamicKeyPartList ) const
{
  QString key = mDefinitionBaseKey;

  if ( dynamicKeyPartList.isEmpty() )
  {
    if ( hasDynamicKey() )
      QgsDebugError( u"Settings group '%1' have a dynamic key but the dynamic key part was not provided"_s.arg( key ) );

    return key;
  }
  else
  {
    if ( !hasDynamicKey() )
    {
      QgsDebugError( u"Settings group '%1' don't have a dynamic key, the provided dynamic key part will be ignored"_s.arg( key ) );
      return key;
    }

    for ( int i = 0; i < dynamicKeyPartList.size(); i++ )
    {
      key.replace( u"%"_s.append( QString::number( i + 1 ) ), dynamicKeyPartList.at( i ) );
    }
  }

  return key;
}

void QgsSettingsEntryGroup::removeAllSettingsAtBaseKey( const QStringList &dynamicKeyPartList ) const
{
  QString key = baseKey( dynamicKeyPartList );
  // https://regex101.com/r/kICr42/1
  const thread_local QRegularExpression regularExpression( u"^(\\/?(qgis\\/?)?)?$"_s );
  if ( key.contains( regularExpression ) )
  {
    QgsDebugError( u"Preventing mass removal of settings at key %1"_s.arg( key ) );
    return;
  }

  QgsSettings settings;
  settings.remove( key );
}

void QgsSettingsEntryGroup::removeAllChildrenSettings( const QString &dynamicKeyPart ) const
{
  removeAllChildrenSettings( QgsSettingsEntryBase::dynamicKeyPartToList( dynamicKeyPart ) );
}

void QgsSettingsEntryGroup::removeAllChildrenSettings( const QStringList &dynamicKeyPartList ) const
{
  for ( const auto *setting : mSettings )
    setting->remove( dynamicKeyPartList );
}

bool QgsSettingsEntryGroup::hasDynamicKey() const
{
  const thread_local QRegularExpression regularExpression( u"%\\d+"_s );
  return mDefinitionBaseKey.contains( regularExpression );
}
