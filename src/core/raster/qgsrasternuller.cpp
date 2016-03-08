/***************************************************************************
                         qgsrasternuller.cpp
                         ---------------------
    begin                : August 2012
    copyright            : (C) 2012 by Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrasterdataprovider.h"
#include "qgsrasternuller.h"

QgsRasterNuller::QgsRasterNuller( QgsRasterInterface* input )
    : QgsRasterInterface( input )
{
}

QgsRasterNuller::~QgsRasterNuller()
{
}

QgsRasterNuller* QgsRasterNuller::clone() const
{
  QgsDebugMsgLevel( "Entered", 4 );
  QgsRasterNuller * nuller = new QgsRasterNuller( nullptr );
  nuller->mNoData = mNoData;
  nuller->mOutputNoData = mOutputNoData;
  nuller->mHasOutputNoData = mHasOutputNoData;
  return nuller;
}

void QgsRasterNuller::setOutputNoDataValue( int bandNo, double noData )
{
  if ( bandNo > mOutputNoData.size() )
  {
    mOutputNoData.resize( bandNo );
    mHasOutputNoData.resize( bandNo );
  }
  mOutputNoData[bandNo-1] = noData;
  mHasOutputNoData[bandNo-1] = true;
}

void QgsRasterNuller::setNoData( int bandNo, const QgsRasterRangeList& noData )
{
  if ( bandNo > mNoData.size() )
  {
    mNoData.resize( bandNo );
  }
  mNoData[bandNo-1] = noData;
}

int QgsRasterNuller::bandCount() const
{
  if ( mInput ) return mInput->bandCount();
  return 0;
}

QGis::DataType QgsRasterNuller::dataType( int bandNo ) const
{
  if ( mInput ) return mInput->dataType( bandNo );
  return QGis::UnknownDataType;
}

QgsRasterBlock * QgsRasterNuller::block( int bandNo, QgsRectangle  const & extent, int width, int height )
{
  QgsDebugMsgLevel( "Entered", 4 );
  if ( !mInput )
  {
    return new QgsRasterBlock();
  }

  QgsRasterBlock *inputBlock = mInput->block( bandNo, extent, width, height );
  if ( !inputBlock )
  {
    return new QgsRasterBlock();
  }

  // We don't support nuller for color types
  if ( QgsRasterBlock::typeIsColor( inputBlock->dataType() ) )
  {
    return inputBlock;
  }

  QgsRasterBlock *outputBlock = nullptr;

  if ( mHasOutputNoData.value( bandNo - 1 ) || inputBlock->hasNoDataValue() )
  {
    double noDataValue;
    if ( mHasOutputNoData.value( bandNo - 1 ) )
    {
      noDataValue = mOutputNoData.value( bandNo - 1 );
    }
    else
    {
      noDataValue = inputBlock->noDataValue();
    }
    outputBlock = new QgsRasterBlock( inputBlock->dataType(), width, height, noDataValue );
  }
  else
  {
    outputBlock = new QgsRasterBlock( inputBlock->dataType(), width, height );
  }

  for ( int i = 0; i < height; i++ )
  {
    for ( int j = 0; j < width; j++ )
    {
      double value = inputBlock->value( i, j );

      bool isNoData = inputBlock->isNoData( i, j );
      if ( QgsRasterRange::contains( value, mNoData.value( bandNo - 1 ) ) )
      {
        isNoData = true;
      }
      outputBlock->setValue( i, j, inputBlock->value( i, j ) );
      if ( isNoData )
      {
        outputBlock->setIsNoData( i, j );
      }
      else
      {
        outputBlock->setValue( i, j, value );
      }
    }
  }
  delete inputBlock;

  return outputBlock;
}

