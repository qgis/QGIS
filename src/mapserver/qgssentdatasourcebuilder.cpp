/***************************************************************************
                              qgssentdatasourcebuilder.cpp
                              ----------------------------
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

#include "qgssentdatasourcebuilder.h"
#include "qgslogger.h"
#include "qgsrasterlayer.h"
#include "qgsvectorlayer.h"
#include <QDataStream>
#include <QDomElement>
#include <QTemporaryFile>
#include <QTextStream>

QgsSentDataSourceBuilder::QgsSentDataSourceBuilder()
{

}

QgsSentDataSourceBuilder::~QgsSentDataSourceBuilder()
{

}

QgsMapLayer* QgsSentDataSourceBuilder::createMapLayer( const QDomElement& elem,
    const QString& layerName,
    QList<QTemporaryFile*>& filesToRemove,
    QList<QgsMapLayer*>& layersToRemove,
    bool allowCaching ) const
{
  Q_UNUSED( layerName );
  Q_UNUSED( allowCaching );
  if ( elem.tagName() == "SentRDS" )
  {
    return rasterLayerFromSentRDS( elem, filesToRemove, layersToRemove );
  }
  else if ( elem.tagName() == "SentVDS" )
  {
    return vectorLayerFromSentVDS( elem, filesToRemove, layersToRemove );
  }
  return 0;
}

QgsVectorLayer* QgsSentDataSourceBuilder::vectorLayerFromSentVDS( const QDomElement& sentVDSElem, QList<QTemporaryFile*>& filesToRemove, QList<QgsMapLayer*>& layersToRemove ) const
{
  if ( sentVDSElem.attribute( "format" ) == "GML" )
  {
    QTemporaryFile* tmpFile = new QTemporaryFile();
    if ( tmpFile->open() )
    {
      filesToRemove.push_back( tmpFile ); //make sure the temporary file gets deleted after each request
      QTextStream tempFileStream( tmpFile );
      sentVDSElem.save( tempFileStream, 4 );
      tmpFile->close();
    }
    else
    {
      return 0;
    }

    QgsVectorLayer* theVectorLayer = new QgsVectorLayer( tmpFile->fileName(), layerNameFromUri( tmpFile->fileName() ), "WFS" );
    if ( !theVectorLayer || !theVectorLayer->isValid() )
    {
      QgsDebugMsg( "invalid maplayer" );
      return 0;
    }
    QgsDebugMsg( "returning maplayer" );

    layersToRemove.push_back( theVectorLayer ); //make sure the layer gets deleted after each request

    if ( !theVectorLayer || !theVectorLayer->isValid() )
    {
      return 0;
    }
    return theVectorLayer;
  }
  return 0;
}

QgsRasterLayer* QgsSentDataSourceBuilder::rasterLayerFromSentRDS( const QDomElement& sentRDSElem, QList<QTemporaryFile*>& filesToRemove, QList<QgsMapLayer*>& layersToRemove ) const
{
  QgsDebugMsg( "Entering" );
  QString tempFilePath = createTempFile();
  if ( tempFilePath.isEmpty() )
  {
    return 0;
  }
  QFile tempFile( tempFilePath );

  QTemporaryFile* tmpFile = new QTemporaryFile();

  QString encoding = sentRDSElem.attribute( "encoding" );

  if ( encoding == "base64" )
  {
    if ( tmpFile->open() )
    {
      QByteArray binaryContent = QByteArray::fromBase64( sentRDSElem.text().toAscii() );
      QDataStream ds( tmpFile );
      ds.writeRawData( binaryContent.data(), binaryContent.length() );
    }
    else
    {
      delete tmpFile;
      return 0;
    }

  }
  else //assume text (e.g. ascii grid)
  {
    if ( tmpFile->open() )
    {
      QTextStream tempFileStream( tmpFile );
      tempFileStream << sentRDSElem.text();
    }
    else
    {
      delete tmpFile;
      return 0;
    }
  }

  QgsDebugMsg( "TempFilePath is: " + tempFilePath );
  tmpFile->close();

  QgsRasterLayer* rl = new QgsRasterLayer( tmpFile->fileName(), layerNameFromUri( tmpFile->fileName() ) );
  filesToRemove.push_back( tmpFile ); //make sure the temporary file gets deleted after each request
  layersToRemove.push_back( rl ); //make sure the layer gets deleted after each request
  return rl;
}
