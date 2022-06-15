/***************************************************************************
    qgsfontmanager.cpp
    ------------------
    Date                 : June 2022
    Copyright            : (C) 2022 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsfontmanager.h"

#include <QRegularExpression>
#include <QRegularExpressionMatch>

QgsFontManager::QgsFontManager( QObject *parent )
  : QObject( parent )
{
  const QStringList replacements = settingsFontFamilyReplacements.value();
  for ( const QString &replacement : replacements )
  {
    const thread_local QRegularExpression rxReplacement( QStringLiteral( "(.*?):(.*)" ) );
    const QRegularExpressionMatch match = rxReplacement.match( replacement );
    if ( match.hasMatch() )
      mFamilyReplacements.insert( match.captured( 1 ), match.captured( 2 ) );
  }
}

QMap<QString, QString> QgsFontManager::fontFamilyReplacements() const
{
  return mFamilyReplacements;
}

void QgsFontManager::addFontFamilyReplacement( const QString &original, const QString &replacement )
{
  if ( !replacement.isEmpty() )
    mFamilyReplacements.insert( original, replacement );
  else
    mFamilyReplacements.remove( original );
  storeFamilyReplacements();
}

void QgsFontManager::setFontFamilyReplacements( const QMap<QString, QString> &replacements )
{
  mFamilyReplacements = replacements;
  storeFamilyReplacements();
}

void QgsFontManager::storeFamilyReplacements()
{
  QStringList replacements;
  for ( auto it = mFamilyReplacements.constBegin(); it != mFamilyReplacements.constEnd(); ++it )
    replacements << QStringLiteral( "%1:%2" ).arg( it.key(), it.value() );
  settingsFontFamilyReplacements.setValue( replacements );
}
