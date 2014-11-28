/***************************************************************************
                              qgscapabilitiescache.h
                              ----------------------
  begin                : May 11th, 2011
  copyright            : (C) 2011 by Marco Hugentobler
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

#ifndef QGSCAPABILITIESCACHE_H
#define QGSCAPABILITIESCACHE_H

#include <QDomDocument>
#include <QFileSystemWatcher>
#include <QHash>
#include <QObject>

/**A cache for capabilities xml documents (by configuration file path)*/
class SERVER_EXPORT QgsCapabilitiesCache : public QObject
{
    Q_OBJECT
  public:
    QgsCapabilitiesCache();
    ~QgsCapabilitiesCache();

    /**Returns cached capabilities document (or 0 if document for configuration file not in cache)*/
    const QDomDocument* searchCapabilitiesDocument( QString configFilePath, QString version );
    /**Inserts new capabilities document (creates a copy of the document, does not take ownership)*/
    void insertCapabilitiesDocument( QString configFilePath, QString version, const QDomDocument* doc );

  private:
    QHash< QString, QHash< QString, QDomDocument > > mCachedCapabilities;
    QFileSystemWatcher mFileSystemWatcher;

  private slots:
    /**Removes changed entry from this cache*/
    void removeChangedEntry( const QString &path );
};

#endif // QGSCAPABILITIESCACHE_H
