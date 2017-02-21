/***************************************************************************
                              qgsremotedatasourcebuilder.cpp
                              ------------------------------
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

#include "qgis.h"
#include "qgsremotedatasourcebuilder.h"
#include "qgslogger.h"
#include "qgsrasterlayer.h"
#include "qgsvectorlayer.h"
#include <QDomElement>
#include <QFile>
#include <QTemporaryFile>

QgsRemoteDataSourceBuilder::QgsRemoteDataSourceBuilder(): QgsMSLayerBuilder()
{
}

QgsMapLayer* QgsRemoteDataSourceBuilder::createMapLayer( const QDomElement& elem, const QString& layerName, QList<QTemporaryFile*>& filesToRemove, QList<QgsMapLayer*>& layersToRemove, bool allowCaching ) const
{
  QgsMapLayer* layer = nullptr;
  if ( elem.tagName() == QLatin1String( "RemoteRDS" ) )
  {
    layer = rasterLayerFromRemoteRDS( elem, layerName, filesToRemove, layersToRemove, allowCaching );
  }
  else if ( elem.tagName() == QLatin1String( "RemoteVDS" ) )
  {
    layer = vectorLayerFromRemoteVDS( elem, layerName, filesToRemove, layersToRemove, allowCaching );
  }
  else
  {
    return nullptr;
  }
  return layer;
}

QgsRasterLayer* QgsRemoteDataSourceBuilder::rasterLayerFromRemoteRDS( const QDomElement& remoteRDSElem,
    const QString& layerName,
    QList<QTemporaryFile*> &filesToRemove,
    QList<QgsMapLayer*> &layersToRemove,
    bool allowCaching ) const
{
  Q_UNUSED( layerName );
  Q_UNUSED( allowCaching );

  //load file with QgsHttpTransaction or QgsFtpTransaction
  QByteArray fileContents;
  QString uri = remoteRDSElem.text();

  QgsRasterLayer* rl = nullptr;
  if ( loadData( uri, fileContents ) != 0 )
  {
    return nullptr;
  }

  QTemporaryFile* tmpFile = new QTemporaryFile();
  if ( tmpFile->open() )
  {
    tmpFile->write( fileContents );
    tmpFile->flush();
  }
  else
  {
    QgsDebugMsg( "Error, creation of temp file failed" );
    delete tmpFile;
    return nullptr;
  }

  //create rasterlayer
  rl = new QgsRasterLayer( tmpFile->fileName(), layerNameFromUri( tmpFile->fileName() ) );
  layersToRemove.push_back( rl );
  filesToRemove.push_back( tmpFile );
  clearRasterSymbology( rl );
  return rl;
}

QgsVectorLayer* QgsRemoteDataSourceBuilder::vectorLayerFromRemoteVDS( const QDomElement& remoteVDSElem,
    const QString& layerName,
    QList<QTemporaryFile*>& filesToRemove,
    QList<QgsMapLayer*>& layersToRemove,
    bool allowCaching ) const
{
  Q_UNUSED( layerName );
  Q_UNUSED( allowCaching );
  QString providerString;
  QString formatString = remoteVDSElem.attribute( QStringLiteral( "format" ) );
  if ( formatString.compare( QLatin1String( "gml" ), Qt::CaseInsensitive ) == 0 )
  {
    providerString = QStringLiteral( "WFS" );
  }
  else
  {
    providerString = formatString;
  }

  //load file with QgsHttpTransaction
  QByteArray fileContents;
  QString uri = remoteVDSElem.text();

  QgsVectorLayer* vl = nullptr;

  if ( loadData( uri, fileContents ) != 0 )
  {
    return nullptr;
  }

  //store content into temporary file
  QTemporaryFile* tmpFile = new QTemporaryFile();
  if ( tmpFile->open() )
  {
    tmpFile->write( fileContents );
    tmpFile->flush();
  }
  else
  {
    delete tmpFile;
    return nullptr;
  }

  //create vector layer

  //SOS has a special datasource key...
  if ( formatString.compare( QLatin1String( "SOS" ), Qt::CaseInsensitive ) == 0 )
  {
    QString url = "url=" + tmpFile->fileName() + " method=FILE xml=";
    vl =  new QgsVectorLayer( url, layerNameFromUri( tmpFile->fileName() ), providerString );
  }
  else
  {
    vl =  new QgsVectorLayer( tmpFile->fileName(), layerNameFromUri( tmpFile->fileName() ), providerString );
  }

  if ( !( vl->isValid() ) )
  {
    QgsDebugMsg( "vl is not valid" );
  }

  layersToRemove.push_back( vl );
  filesToRemove.push_back( tmpFile );
  return vl;
}

int QgsRemoteDataSourceBuilder::loadData( const QString& url, QByteArray& data ) const
{
  Q_UNUSED( url )
  Q_UNUSED( data )
  QgsDebugMsg( "http and ftp remote datasources not supported with Qt5" );
  return 0;
}
