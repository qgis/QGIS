/***************************************************************************
                              qgsconfigcache.h
                              ----------------
  begin                : July 24th, 2010
  copyright            : (C) 2010 by Marco Hugentobler
  email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCONFIGCACHE_H
#define QGSCONFIGCACHE_H

#include <QCache>
#include <QFileSystemWatcher>
#include <QMap>
#include <QObject>

class QgsServerProjectParser;
class QgsWCSProjectParser;
class QgsWFSProjectParser;
class QgsWMSConfigParser;

class QDomDocument;

class QgsConfigCache: public QObject
{
    Q_OBJECT
  public:
    static QgsConfigCache* instance();
    ~QgsConfigCache();

    QgsServerProjectParser* serverConfiguration( const QString& filePath );
    QgsWCSProjectParser* wcsConfiguration( const QString& filePath );
    QgsWFSProjectParser* wfsConfiguration( const QString& filePath );
    QgsWMSConfigParser* wmsConfiguration( const QString& filePath, const QMap<QString, QString>& parameterMap = ( QMap< QString, QString >() ) );

  private:
    QgsConfigCache();
    static QgsConfigCache* mInstance;

    /**Check for configuration file updates (remove entry from cache if file changes)*/
    QFileSystemWatcher mFileSystemWatcher;

    /**Returns xml document for project file / sld or 0 in case of errors*/
    QDomDocument* xmlDocument( const QString& filePath );

    QCache<QString, QgsWMSConfigParser> mWMSConfigCache;
    QCache<QString, QgsWFSProjectParser> mWFSConfigCache;
    QCache<QString, QgsWCSProjectParser> mWCSConfigCache;

  private slots:
    /**Removes changed entry from this cache*/
    void removeChangedEntry( const QString& path );
};

#endif // QGSCONFIGCACHE_H
