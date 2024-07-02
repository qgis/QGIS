/***************************************************************************
                             qgsprocessingfavoritealgorithmlog.cpp
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

#include "qgsprocessingfavoritealgorithmlog.h"
#include "qgssettingstree.h"
#include "qgssettingsentryimpl.h"

///@cond PRIVATE

const QgsSettingsEntryStringList *QgsProcessingFavoriteAlgorithmLog::settingsFavoriteAlgorithms = new QgsSettingsEntryStringList( QStringLiteral( "favorite-algorithms" ), QgsSettingsTree::sTreeProcessing, QStringList(), QObject::tr( "Favorite Processing algorithms" ) );

QgsProcessingFavoriteAlgorithmLog::QgsProcessingFavoriteAlgorithmLog( QObject *parent )
  : QObject( parent )
{
  mFavoriteAlgorithmIds = QgsProcessingFavoriteAlgorithmLog::settingsFavoriteAlgorithms->value();
}

QStringList QgsProcessingFavoriteAlgorithmLog::favoriteAlgorithmIds() const
{
  return mFavoriteAlgorithmIds;
}

void QgsProcessingFavoriteAlgorithmLog::add( const QString &id )
{
  if ( mFavoriteAlgorithmIds.contains( id ) )
  {
    return;
  }

  mFavoriteAlgorithmIds << id;
  QgsProcessingFavoriteAlgorithmLog::settingsFavoriteAlgorithms->setValue( mFavoriteAlgorithmIds );
  emit changed();
}

void QgsProcessingFavoriteAlgorithmLog::remove( const QString &id )
{
  if ( !mFavoriteAlgorithmIds.contains( id ) )
  {
    return;
  }

  mFavoriteAlgorithmIds.removeAll( id );
  QgsProcessingFavoriteAlgorithmLog::settingsFavoriteAlgorithms->setValue( mFavoriteAlgorithmIds );
  emit changed();
}

void QgsProcessingFavoriteAlgorithmLog::clear()
{
  mFavoriteAlgorithmIds.clear();
  QgsProcessingFavoriteAlgorithmLog::settingsFavoriteAlgorithms->setValue( mFavoriteAlgorithmIds );
  emit changed();
}

bool QgsProcessingFavoriteAlgorithmLog::isFavorite( const QString &id )
{
  return mFavoriteAlgorithmIds.contains( id );
}

///@endcond
