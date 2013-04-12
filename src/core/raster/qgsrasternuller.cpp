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

QgsRasterInterface * QgsRasterNuller::clone() const
{
  QgsDebugMsg( "Entered" );
  QgsRasterNuller * nuller = new QgsRasterNuller( 0 );
  nuller->mNoData = mNoData;
  return nuller;
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
  QgsDebugMsg( "Entered" );
  QgsRasterBlock *outputBlock = new QgsRasterBlock();
  if ( !mInput )
  {
    return outputBlock;
  }

  //void * rasterData = mInput->block( bandNo, extent, width, height );
  QgsRasterBlock *inputBlock = mInput->block( bandNo, extent, width, height );

  // Input may be without no data value
  //double noDataValue = mInput->noDataValue( bandNo );
  double noDataValue = mOutputNoData;

  for ( int i = 0; i < height; i++ )
  {
    for ( int j = 0; j < width; j++ )
    {
      //int index = i * width + j;

      //double value = readValue( rasterData, dataType, index );
      double value = inputBlock->value( i, j );

      foreach ( NoData noData, mNoData )
      {
        if (( value >= noData.min && value <= noData.max ) ||
            qgsDoubleNear( value, noData.min ) ||
            qgsDoubleNear( value, noData.max ) )
        {
          inputBlock->setValue( i, j, noDataValue );
        }
      }
    }
  }

  return inputBlock;
}

