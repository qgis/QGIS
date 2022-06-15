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

QgsFontManager::QgsFontManager( QObject *parent )
  : QObject( parent )
{

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
}

void QgsFontManager::setFontFamilyReplacements( const QMap<QString, QString> &replacements )
{
  mFamilyReplacements = replacements;
}
