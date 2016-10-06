/***************************************************************************
                          qgsgrassundocommand.cpp
                             -------------------
    begin                : November, 2015
    copyright            : (C) 2015 by Radim Blazek
    email                : radim.blazek@gmail.com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsgrassundocommand.h"

#include "qgsgrassprovider.h"
#include "qgslogger.h"

QgsGrassUndoCommandChangeAttribute::QgsGrassUndoCommandChangeAttribute( QgsGrassProvider * provider, int fid, int lid, int field, int cat, bool deleteCat, bool deleteRecord )
    : mProvider( provider )
    , mFid( fid )
    , mLid( lid )
    , mField( field )
    , mCat( cat )
    , mDeleteCat( deleteCat )
    , mDeleteRecord( deleteRecord )
{
  Q_UNUSED( mField );
}

void QgsGrassUndoCommandChangeAttribute::undo()
{
  QgsDebugMsg( QString( "mLid = %1 mField = %2, mCat = %3" ).arg( mLid ).arg( mField ).arg( mCat ) );
  if ( mDeleteCat )
  {
    int realLine = mLid;
    if ( mProvider->mLayer->map()->newLids().contains( mLid ) )
    {
      realLine = mProvider->mLayer->map()->newLids().value( mLid );
    }
    QgsDebugMsg( QString( "realLine = %1" ).arg( realLine ) );

    int type = mProvider->readLine( mProvider->mPoints, mProvider->mCats, realLine );
    if ( type <= 0 )
    {
      QgsDebugMsg( "cannot read line" );
    }
    else
    {
      if ( Vect_field_cat_del( mProvider->mCats, mProvider->mLayerField, mCat ) == 0 )
      {
        // should not happen
        QgsDebugMsg( "the line does not have the category" );
      }
      else
      {
        mProvider->mLayer->map()->lockReadWrite();
        int newLid = mProvider->rewriteLine( realLine, type, mProvider->mPoints, mProvider->mCats );
        Q_UNUSED( newLid );
        mProvider->mLayer->map()->newCats().remove( mFid );
        mProvider->mLayer->map()->unlockReadWrite();
      }
    }
  }
  if ( mDeleteRecord )
  {
    QString error;
    mProvider->mLayer->deleteAttribute( mCat, error );
    if ( !error.isEmpty() )
    {
      QgsGrass::warning( error );
    }
  }
}

