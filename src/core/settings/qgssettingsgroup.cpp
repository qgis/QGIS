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
#include "qgslogger.h"

QgsSettingsGroup::QgsSettingsGroup( QString key,
                                    QgsSettingsGroup *parentGroup,
                                    QString description )
  : mKey( key )
  , mSettingsGroupParent( parentGroup )
  , mDescription( description )
{
}

QgsSettingsGroup::~QgsSettingsGroup()
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

  return QString( "%1/%2" )
         .arg( mSettingsGroupParent->key() )
         .arg( mKey );
}

QString QgsSettingsGroup::description() const
{
  return mDescription;
}


