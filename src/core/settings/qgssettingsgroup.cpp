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

#include "qgssettingsgroup.h"
#include "qgssettingsentry.h"
#include "qgslogger.h"

QgsSettingsGroup::QgsSettingsGroup( QString key,
                                    QgsSettingsGroup *parentGroup,
                                    QString description )
  : mKey( key )
  , mSection( QgsSettings::NoSection )
  , mSettingsGroupParent( parentGroup )
  , mDescription( description )
  , mChildSettingsGroups()
  , mChildSettingsEntries()
{
  if ( mSettingsGroupParent != nullptr )
    mSettingsGroupParent->registerChildSettingsGroup( this );
}

QgsSettingsGroup::QgsSettingsGroup( QgsSettings::Section section,
                                    QString description )
  : mKey()
  , mSection( section )
  , mSettingsGroupParent( nullptr )
  , mDescription( description )
  , mChildSettingsGroups()
  , mChildSettingsEntries()
{
}

void QgsSettingsGroup::setKey( const QString &key )
{
  mKey = key;
}

QString QgsSettingsGroup::key() const
{
  if ( mSettingsGroupParent == nullptr )
    return mKey;

  if ( mSettingsGroupParent->key().isEmpty() )
    return mKey;

  return QString( "%1/%2" )
         .arg( mSettingsGroupParent->key() )
         .arg( mKey );
}

QgsSettings::Section QgsSettingsGroup::section()
{
  if ( mSettingsGroupParent == nullptr )
    return mSection;

  return mSettingsGroupParent->section();
}

QString QgsSettingsGroup::description() const
{
  return mDescription;
}

void QgsSettingsGroup::registerChildSettingsGroup( QgsSettingsGroup *childSettingsGroup )
{
  QgsLogger::warning( QStringLiteral( "Settings group '%1' registering child group '%2' current size '%3'" ).arg( key() ).arg( childSettingsGroup->key() ).arg( mChildSettingsGroups.size() ) );

  if ( mChildSettingsGroups.contains( childSettingsGroup ) )
  {
    QgsLogger::warning( QStringLiteral( "Settings group '%1' already contains child group '%2'" ).arg( key() ).arg( childSettingsGroup->key() ) );
    return;
  }

  mChildSettingsGroups.append( childSettingsGroup );
}

void QgsSettingsGroup::registerChildSettingsEntry( QgsSettingsEntry *childSettingsEntry )
{
  if ( mChildSettingsEntries.contains( childSettingsEntry ) )
  {
    QgsLogger::warning( QStringLiteral( "Settings group '%1' already contains child entry '%2'" ).arg( key() ).arg( childSettingsEntry->key() ) );
    return;
  }

  mChildSettingsEntries.append( childSettingsEntry );
}


