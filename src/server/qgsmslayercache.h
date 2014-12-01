/***************************************************************************
                              qgsmslayercache.h
                              -------------------
  begin                : September 21, 2007
  copyright            : (C) 2007 by Marco Hugentobler
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

#ifndef QGSMSLAYERCACHE_H
#define QGSMSLAYERCACHE_H

#include <time.h>
#include <QFileSystemWatcher>
#include <QHash>
#include <QObject>
#include <QPair>
#include <QString>

class QgsMapLayer;

struct QgsMSLayerCacheEntry
{
  time_t creationTime; //time this layer was created
  time_t lastUsedTime; //last time this layer was in use
  QString url; //datasource url
  QgsMapLayer* layerPointer;
  QList<QString> temporaryFiles; //path to the temporary files written for the layer
  QString configFile; //path to the project file associated with the layer
};

/**A singleton class that caches layer objects for the
QGIS mapserver*/
class QgsMSLayerCache: public QObject
{
    Q_OBJECT
  public:
    static QgsMSLayerCache* instance();
    ~QgsMSLayerCache();

    /**Inserts a new layer into the cash
    @param url the layer datasource
    @param layerName the layer name (to distinguish between different layers in a request using the same datasource
    @param configFile path of the config file (to invalidate entries if file changes). Can be empty (e.g. layers from sld)
    @param tempFiles some layers have temporary files. The cash makes sure they are removed when removing the layer from the cash*/
    void insertLayer( const QString& url, const QString& layerName, QgsMapLayer* layer, const QString& configFile = QString(), const QList<QString>& tempFiles = QList<QString>() );
    /**Searches for the layer with the given url.
     @return a pointer to the layer or 0 if no such layer*/
    QgsMapLayer* searchLayer( const QString& url, const QString& layerName );

    int projectsMaxLayers() const { return mProjectMaxLayers; }

    void setProjectMaxLayers( int n ) { mProjectMaxLayers = n; }

  protected:
    /**Protected singleton constructor*/
    QgsMSLayerCache();
    /**Goes through the list and removes entries and layers
     depending on their time stamps and the number of other
    layers*/
    void updateEntries();
    /**Removes the cash entry with the lowest 'lastUsedTime'*/
    void removeLeastUsedEntry();
    /**Frees memory and removes temporary files of an entry*/
    void freeEntryRessources( QgsMSLayerCacheEntry& entry );

  private:
    /**Cash entries with pair url/layer name as a key. The layer name is necessary for cases where the same
      url is used several time in a request. It ensures that different layer instances are created for different
      layer names*/
    QHash<QPair<QString, QString>, QgsMSLayerCacheEntry> mEntries;

    /**Config files used in the cache (with reference counter)*/
    QHash< QString, int > mConfigFiles;

    /**Check for configuration file updates (remove layers from cache if configuration file changes)*/
    QFileSystemWatcher mFileSystemWatcher;

    /**Maximum number of layers in the cache*/
    int mDefaultMaxLayers;

    /**Maximum number of layers in the cache, overrides DEFAULT_MAX_N_LAYERS if larger*/
    int mProjectMaxLayers;

  private slots:

    /**Removes entries from a project (e.g. if a project file has changed)*/
    void removeProjectFileLayers( const QString& project );
};

#endif
