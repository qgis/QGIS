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

#include "qgsremotedatasourcebuilder.h"
#include "qgsftptransaction.h"
#include "qgshttptransaction.h"
#include "qgsmapserverlogger.h"
#include "qgsrasterlayer.h"
#include "qgsvectorlayer.h"
#include <QDomElement>
#include <QFile>
#include <QTemporaryFile>

QgsRemoteDataSourceBuilder::QgsRemoteDataSourceBuilder(): QgsMSLayerBuilder()
{

}

QgsRemoteDataSourceBuilder::~QgsRemoteDataSourceBuilder()
{

}

QgsMapLayer* QgsRemoteDataSourceBuilder::createMapLayer(const QDomElement& elem, const QString& layerName, QList<QTemporaryFile*>& filesToRemove, QList<QgsMapLayer*>& layersToRemove, bool allowCaching) const
{
  QgsMSDebugMsg("QgsRDSBuilder::createMapLayer");
  QgsMapLayer* theLayer = 0;
  if(elem.tagName() == "RemoteRDS")
    {
      theLayer = rasterLayerFromRemoteRDS(elem, layerName, filesToRemove, layersToRemove, allowCaching);
    }
  else if(elem.tagName() == "RemoteVDS")
    {
      theLayer = vectorLayerFromRemoteVDS(elem, layerName, filesToRemove, layersToRemove, allowCaching);
    }
  else
    {
      return 0;
    }
  return theLayer;
}

QgsRasterLayer* QgsRemoteDataSourceBuilder::rasterLayerFromRemoteRDS(const QDomElement& remoteRDSElem, const QString& layerName, QList<QTemporaryFile*>& filesToRemove, QList<QgsMapLayer*>& layersToRemove, bool allowCaching) const
{
  QgsMSDebugMsg("QgsRDSBuilder::rasterLayerFromRemoteRDS");
  //load file with QgsHttpTransaction or QgsFtpTransaction
  QByteArray fileContents;
  QString uri = remoteRDSElem.text();

  QgsRasterLayer* rl = 0;
  if(loadData(uri, fileContents) != 0)
  {
    return 0;
  }

  QTemporaryFile* tmpFile = new QTemporaryFile();
  if(tmpFile->open())
  {
    tmpFile->write(fileContents);
    tmpFile->flush();
  }
  else
  {
    QgsMSDebugMsg("Error, creation of temp file failed");
    delete tmpFile;
    return 0;
  }
      
  //create rasterlayer
  rl = new QgsRasterLayer(tmpFile->fileName(), layerNameFromUri(tmpFile->fileName()));
  layersToRemove.push_back(rl);
  filesToRemove.push_back(tmpFile);
  clearRasterSymbology(rl);
  return rl;
}

QgsVectorLayer* QgsRemoteDataSourceBuilder::vectorLayerFromRemoteVDS(const QDomElement& remoteVDSElem, const QString& layerName, QList<QTemporaryFile*>& filesToRemove, QList<QgsMapLayer*>& layersToRemove, bool allowCaching) const
{
  QString providerString;
  QString formatString = remoteVDSElem.attribute("format");
  if(formatString.compare("gml", Qt::CaseInsensitive) == 0)
    {
      providerString = "WFS";
    }
  else
    {
      providerString = formatString;
    }
  
  //load file with QgsHttpTransaction
  QByteArray fileContents;
  QString uri = remoteVDSElem.text();
  
  QgsVectorLayer* vl = 0;

  if(loadData(uri, fileContents) != 0)
    {
      return 0;
    }

  //store content into temporary file
  QTemporaryFile* tmpFile = new QTemporaryFile();
  if(tmpFile->open())
    {
      tmpFile->write(fileContents);
      tmpFile->flush();
    }
  else
    {
      delete tmpFile;
      return 0;
    }

  //create vector layer

  //SOS has a special datasource key...
  if(formatString.compare("SOS", Qt::CaseInsensitive) == 0)
  {
    QString url = "url=" + tmpFile->fileName() + " method=FILE xml=";
    vl =  new QgsVectorLayer(url, layerNameFromUri(tmpFile->fileName()), providerString);
  }
  else
  {
    vl =  new QgsVectorLayer(tmpFile->fileName(), layerNameFromUri(tmpFile->fileName()), providerString);
  }

  if(!(vl->isValid()))
  {
    QgsMapServerLogger::instance()->printMessage("vl is not valid");
  }
  
  layersToRemove.push_back(vl);
  filesToRemove.push_back(tmpFile);
  return vl;
}

int QgsRemoteDataSourceBuilder::loadData(const QString& url, QByteArray& data) const
{
  if(url.startsWith("http", Qt::CaseInsensitive))
	{
	  QgsHttpTransaction http(url);
	  if(!http.getSynchronously(data))
	    {
	      QgsMSDebugMsg("Error, loading from http failed")
	      return 1; //no success
	    }
	}
      else if(url.startsWith("ftp", Qt::CaseInsensitive))
	{
	  QgsFtpTransaction ftp;
	  if( ftp.get(url, data) != 0 )
	    {
	      return 1;
	    }
	}
  return 0;
}
