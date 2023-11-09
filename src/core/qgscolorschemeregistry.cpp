/***************************************************************************
                           qgscolorschemeregistry.cpp
                             -------------------
    begin                : July 2014
    copyright            : (C) 2014 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscolorschemeregistry.h"
#include "qgscolorscheme.h"
#include "qgsapplication.h"
#include <QDir>
#include <QFileInfoList>
#include <QMutex>
#include <random>

QgsColorSchemeRegistry::~QgsColorSchemeRegistry()
{
  qDeleteAll( mColorSchemeList );
  mColorSchemeList.clear();
}

void QgsColorSchemeRegistry::populateFromInstance()
{
  //get schemes from global instance
  QList< QgsColorScheme * > schemeList = QgsApplication::colorSchemeRegistry()->schemes();

  //add to this scheme registry
  QList< QgsColorScheme * >::iterator it = schemeList.begin();
  for ( ; it != schemeList.end(); ++it )
  {
    addColorScheme( ( *it )->clone() );
  }
}

void QgsColorSchemeRegistry::addDefaultSchemes()
{
  //default color schemes
  addColorScheme( new QgsRecentColorScheme() );
  addColorScheme( new QgsCustomColorScheme() );
  addColorScheme( new QgsProjectColorScheme() );
  addUserSchemes();
}

void QgsColorSchemeRegistry::initStyleScheme()
{
  const QString stylePalette = QgsApplication::pkgDataPath() + QStringLiteral( "/resources/palettes/new_layer_colors.gpl" );
  if ( QFileInfo::exists( stylePalette ) )
  {
    QgsUserColorScheme *scheme = new QgsUserColorScheme( stylePalette );
    addColorScheme( scheme );
    setRandomStyleColorScheme( scheme );
  }
}

void QgsColorSchemeRegistry::addUserSchemes()
{
  const QString palettesDir = QgsApplication::qgisSettingsDirPath() + "palettes";

  const QDir localDir;
  if ( !localDir.mkpath( palettesDir ) )
  {
    return;
  }

  const QFileInfoList fileInfoList = QDir( palettesDir ).entryInfoList( QStringList( QStringLiteral( "*.gpl" ) ), QDir::Files );
  QFileInfoList::const_iterator infoIt = fileInfoList.constBegin();
  for ( ; infoIt != fileInfoList.constEnd(); ++infoIt )
  {
    addColorScheme( new QgsUserColorScheme( infoIt->fileName() ) );
  }
}

void QgsColorSchemeRegistry::addColorScheme( QgsColorScheme *scheme )
{
  mColorSchemeList.append( scheme );
}

QList<QgsColorScheme *> QgsColorSchemeRegistry::schemes() const
{
  QList< QgsColorScheme * > allSchemes;
  QList<QgsColorScheme *>::const_iterator schemeIt;
  for ( schemeIt = mColorSchemeList.constBegin(); schemeIt != mColorSchemeList.constEnd(); ++schemeIt )
  {
    allSchemes.append( ( *schemeIt ) );
  }
  return allSchemes;
}

QList<QgsColorScheme *> QgsColorSchemeRegistry::schemes( const QgsColorScheme::SchemeFlag flag ) const
{
  QList< QgsColorScheme * > matchingSchemes;
  QList<QgsColorScheme *>::const_iterator schemeIt;
  for ( schemeIt = mColorSchemeList.constBegin(); schemeIt != mColorSchemeList.constEnd(); ++schemeIt )
  {
    if ( ( *schemeIt )->flags().testFlag( flag ) )
    {
      matchingSchemes.append( ( *schemeIt ) );
    }
  }
  return matchingSchemes;
}

void QgsColorSchemeRegistry::setRandomStyleColorScheme( QgsColorScheme *scheme )
{
  mRandomStyleColorScheme = scheme;
  if ( scheme )
  {
    mRandomStyleColors = scheme->fetchColors();

    if ( mRandomStyleColors.count() > 0 )
    {
      std::random_device rd;
      std::mt19937 mt( rd() );
      std::uniform_int_distribution<int> colorDist( 0, mRandomStyleColors.count() - 1 );
      mNextRandomStyleColorIndex = colorDist( mt );
      std::uniform_int_distribution<int> colorDir( 0, 1 );
      mNextRandomStyleColorDirection = colorDir( mt ) == 0 ? -1 : 1;
    }
  }
  else
  {
    mRandomStyleColors.clear();
  }
}

QgsColorScheme *QgsColorSchemeRegistry::randomStyleColorScheme()
{
  return mRandomStyleColorScheme;
}

QColor QgsColorSchemeRegistry::fetchRandomStyleColor() const
{
  if ( mRandomStyleColors.empty() )
  {
    // no random color scheme available - so just use totally random colors

    // Make sure we use get uniquely seeded random numbers, and not the same sequence of numbers
    std::random_device rd;
    std::mt19937 mt( rd() );
    std::uniform_int_distribution<int> hueDist( 0, 359 );
    std::uniform_int_distribution<int> satDist( 64, 255 );
    std::uniform_int_distribution<int> valueDist( 128, 255 );
    return QColor::fromHsv( hueDist( mt ), satDist( mt ), valueDist( mt ) );
  }
  else
  {
    static QMutex sMutex;
    const QMutexLocker locker( &sMutex );
    QColor res = mRandomStyleColors.at( mNextRandomStyleColorIndex ).first;
    mNextRandomStyleColorIndex += mNextRandomStyleColorDirection;
    if ( mNextRandomStyleColorIndex < 0 )
      mNextRandomStyleColorIndex = mRandomStyleColors.count() - 1;
    if ( mNextRandomStyleColorIndex >= mRandomStyleColors.count() )
      mNextRandomStyleColorIndex = 0;
    return res;
  }
}

bool QgsColorSchemeRegistry::removeColorScheme( QgsColorScheme *scheme )
{
  if ( mRandomStyleColorScheme == scheme )
  {
    mRandomStyleColorScheme = nullptr;
    mRandomStyleColors.clear();
  }

  if ( mColorSchemeList.indexOf( scheme ) != -1 )
  {
    mColorSchemeList.removeAll( scheme );
    return true;
  }

  //not found
  return false;
}
