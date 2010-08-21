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

#include <QMap>
#include <QString>

class QgsConfigParser;

/**A cache for configuration XML (usefull because of the mapfile parameter)*/
class QgsConfigCache
{
  public:
    QgsConfigCache();
    ~QgsConfigCache();

    /**Returns configuration for given config file path. The calling function does _not_ take ownership*/
    QgsConfigParser* searchConfiguration( const QString& filePath );

  private:
    /**Creates configuration parser depending on the file type and, if successfull, inserts it to the cached configuration map
        @param filePath path of the configuration file
        @return the inserted config parser or 0 in case of error*/
    QgsConfigParser* insertConfiguration( const QString& filePath );
    /**Cached XML configuration documents. Key: file path, value: config parser. Default configuration has key '$default$'*/
    QMap<QString, QgsConfigParser*> mCachedConfigurations;
};

#endif // QGSCONFIGCACHE_H
