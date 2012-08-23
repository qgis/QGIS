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

QgsRasterInterface::DataType QgsRasterNuller::dataType( int bandNo ) const
{
  if ( mInput ) return mInput->dataType( bandNo );
  return QgsRasterInterface::UnknownDataType;
}

void * QgsRasterNuller::readBlock( int bandNo, QgsRectangle  const & extent, int width, int height )
{
  QgsDebugMsg( "Entered" );
  if ( !mInput ) return 0;

  //QgsRasterDataProvider *provider = dynamic_cast<QgsRasterDataProvider*>( mInput->srcInput() );

  void * rasterData = mInput->block( bandNo, extent, width, height );

  QgsRasterInterface::DataType dataType =  mInput->dataType( bandNo ); 
  int pixelSize = mInput->typeSize( dataType ) / 8; 

  double noDataValue = mInput->noDataValue ( bandNo );

  for ( int i = 0; i < height; ++i )
  {
    for ( int j = 0; j < width; ++j )
    {
      int index = pixelSize * ( i * width + j );

      double value = readValue( rasterData, dataType, index );
  
      foreach ( NoData noData, mNoData )
      {
        if ( ( value >= noData.min && value <= noData.max ) ||
             doubleNear( value, noData.min ) ||
             doubleNear( value, noData.max ) )
        {
          writeValue( rasterData, dataType, index, noDataValue );
        }
      }
    }
  }

  return rasterData;
}

