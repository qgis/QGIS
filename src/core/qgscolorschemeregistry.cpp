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

//
// Static calls to enforce singleton behaviour
//
QgsColorSchemeRegistry *QgsColorSchemeRegistry::mInstance = nullptr;
QgsColorSchemeRegistry *QgsColorSchemeRegistry::instance()
{
  if ( !mInstance )
  {
    mInstance = new QgsColorSchemeRegistry();

    //add default color schemes
    mInstance->addDefaultSchemes();
    //add user schemes
    mInstance->addUserSchemes();
  }

  return mInstance;
}

//
// Main class begins now...
//

QgsColorSchemeRegistry::QgsColorSchemeRegistry()
{
}

QgsColorSchemeRegistry::~QgsColorSchemeRegistry()
{
  qDeleteAll( mColorSchemeList );
  mColorSchemeList.clear();
}

void QgsColorSchemeRegistry::populateFromInstance()
{
  //get schemes from global instance
  QList< QgsColorScheme* > schemeList = QgsColorSchemeRegistry::instance()->schemes();

  //add to this scheme registry
  QList< QgsColorScheme* >::iterator it = schemeList.begin();
  for ( ; it != schemeList.end(); ++it )
  {
    addColorScheme(( *it )->clone() );
  }
}

void QgsColorSchemeRegistry::addDefaultSchemes()
{
  //default color schemes
  addColorScheme( new QgsRecentColorScheme() );
  addColorScheme( new QgsCustomColorScheme() );
  addColorScheme( new QgsProjectColorScheme() );

}

void QgsColorSchemeRegistry::addUserSchemes()
{
  QString palettesDir = QgsApplication::qgisSettingsDirPath() + "/palettes";

  QDir localDir;
  if ( !localDir.mkpath( palettesDir ) )
  {
    return;
  }

  QFileInfoList fileInfoList = QDir( palettesDir ).entryInfoList( QStringList( "*.gpl" ), QDir::Files );
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
  QList< QgsColorScheme* > allSchemes;
  QList<QgsColorScheme*>::const_iterator schemeIt;
  for ( schemeIt = mColorSchemeList.constBegin(); schemeIt != mColorSchemeList.constEnd(); ++schemeIt )
  {
    allSchemes.append(( *schemeIt ) );
  }
  return allSchemes;
}

QList<QgsColorScheme *> QgsColorSchemeRegistry::schemes( const QgsColorScheme::SchemeFlag flag ) const
{
  QList< QgsColorScheme* > matchingSchemes;
  QList<QgsColorScheme*>::const_iterator schemeIt;
  for ( schemeIt = mColorSchemeList.constBegin(); schemeIt != mColorSchemeList.constEnd(); ++schemeIt )
  {
    if (( *schemeIt )->flags().testFlag( flag ) )
    {
      matchingSchemes.append(( *schemeIt ) );
    }
  }
  return matchingSchemes;
}

bool QgsColorSchemeRegistry::removeColorScheme( QgsColorScheme *scheme )
{
  if ( mColorSchemeList.indexOf( scheme ) != -1 )
  {
    mColorSchemeList.removeAll( scheme );
    return true;
  }

  //not found
  return false;
}


