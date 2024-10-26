/***************************************************************************
                             qgsprocessingfavoritealgorithmmanager.cpp
                             ------------------------------------
    Date                 : February 2024
    Copyright            : (C) 2024 Alexander Bruy
    Email                : alexander dot bruy at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingfavoritealgorithmmanager.h"
#include "moc_qgsprocessingfavoritealgorithmmanager.cpp"
#include "qgssettingstree.h"
#include "qgssettingsentryimpl.h"

///@cond PRIVATE

const QgsSettingsEntryStringList *QgsProcessingFavoriteAlgorithmManager::settingsFavoriteAlgorithms = new QgsSettingsEntryStringList( QStringLiteral( "favorite-algorithms" ), QgsSettingsTree::sTreeProcessing, QStringList(), QObject::tr( "Favorite Processing algorithms" ) );

QgsProcessingFavoriteAlgorithmManager::QgsProcessingFavoriteAlgorithmManager( QObject *parent )
  : QObject( parent )
{
  mFavoriteAlgorithmIds = QgsProcessingFavoriteAlgorithmManager::settingsFavoriteAlgorithms->value();
}

QStringList QgsProcessingFavoriteAlgorithmManager::favoriteAlgorithmIds() const
{
  return mFavoriteAlgorithmIds;
}

void QgsProcessingFavoriteAlgorithmManager::add( const QString &id )
{
  if ( mFavoriteAlgorithmIds.contains( id ) )
  {
    return;
  }

  mFavoriteAlgorithmIds << id;
  QgsProcessingFavoriteAlgorithmManager::settingsFavoriteAlgorithms->setValue( mFavoriteAlgorithmIds );
  emit changed();
}

void QgsProcessingFavoriteAlgorithmManager::remove( const QString &id )
{
  if ( !mFavoriteAlgorithmIds.contains( id ) )
  {
    return;
  }

  mFavoriteAlgorithmIds.removeAll( id );
  QgsProcessingFavoriteAlgorithmManager::settingsFavoriteAlgorithms->setValue( mFavoriteAlgorithmIds );
  emit changed();
}

void QgsProcessingFavoriteAlgorithmManager::clear()
{
  mFavoriteAlgorithmIds.clear();
  QgsProcessingFavoriteAlgorithmManager::settingsFavoriteAlgorithms->setValue( mFavoriteAlgorithmIds );
  emit changed();
}

bool QgsProcessingFavoriteAlgorithmManager::isFavorite( const QString &id )
{
  return mFavoriteAlgorithmIds.contains( id );
}

///@endcond
