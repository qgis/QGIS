/***************************************************************************
                              qgsmslayerbuilder.cpp
                              ---------------------
  begin                : July, 2008
  copyright            : (C) 2008 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmslayerbuilder.h"
#include "qgsrasterlayer.h"

#include <QDir>
#include <QFileInfo>
#include <QStringList>

#include <ctime>

QgsMSLayerBuilder::QgsMSLayerBuilder()
{

}

QgsMSLayerBuilder::~QgsMSLayerBuilder()
{

}

QString QgsMSLayerBuilder::layerNameFromUri( const QString& uri ) const
{
  //file based?
  QFileInfo f( uri );
  if ( f.exists() )
  {
    return f.baseName();
  }

  //http based?
  if ( uri.startsWith( "http", Qt::CaseInsensitive ) )
  {
    return uri;
  }

  //database?
  if ( uri.contains( "dbname" ) )
  {
    //take tablename
    QStringList spaceSplit = uri.split( " " );
    QStringList::const_iterator slIt;
    for ( slIt = spaceSplit.constBegin(); slIt != spaceSplit.constEnd(); ++slIt )
    {
      if ( slIt->startsWith( "table" ) )
      {
        return slIt->section( "=", 1, 1 );
      }
    }
  }

  return "";
}

QString QgsMSLayerBuilder::createTempFile() const
{
  //save the content of the file into a temporary location
  //generate a name considering the current time
  time_t seconds;
  time( &seconds );
  srand( seconds );
  int randomNumber = rand();
  QString tempFileName = QString::number( randomNumber );
  QString tempFilePath;
  // store temporary file in temporary qgis_wms_serv in temporary directory
  QDir tempFileDir( QDir::tempPath() + "/qgis_wms_serv" );
  if ( !tempFileDir.exists() ) //make sure the directory exists
  {
    QDir tmpDir( QDir::tempPath() );
    tmpDir.mkdir( "qgis_wms_serv" );
  }
  tempFilePath = QDir::tempPath() + "/qgis_wms_serv/" + tempFileName;
  return tempFilePath;
}

void QgsMSLayerBuilder::clearRasterSymbology( QgsRasterLayer* rl ) const
{
  if ( rl )
  {
    if ( rl->rasterType() == QgsRasterLayer::GrayOrUndefined )
    {
      rl->setDrawingStyle( QgsRasterLayer::SingleBandPseudoColor );
      rl->setRasterShaderFunction( new QgsRasterShaderFunction() );
    }
  }
}
