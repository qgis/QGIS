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

/** \ingroup server
 * A cache for capabilities xml documents (by configuration file path)*/
class SERVER_EXPORT QgsCapabilitiesCache : public QObject
{
    Q_OBJECT
  public:
    QgsCapabilitiesCache();
    ~QgsCapabilitiesCache();

    /** Returns cached capabilities document (or 0 if document for configuration file not in cache)
     * @param configFilePath the progect file path
     * @param key key used to separate different version in different cache
     */
    const QDomDocument* searchCapabilitiesDocument( const QString& configFilePath, const QString& key );

    /** Inserts new capabilities document (creates a copy of the document, does not take ownership)
     * @param configFilePath the project file path
     * @param key key used to separate different version in different cache
     * @param doc the DOM document
     */
    void insertCapabilitiesDocument( const QString& configFilePath, const QString& key, const QDomDocument* doc );

    /** Remove capabilities document
     * @param path the project file path
     * @note added in QGIS 2.16
     */
    void removeCapabilitiesDocument( const QString& path );

  private:
    QHash< QString, QHash< QString, QDomDocument > > mCachedCapabilities;
    QFileSystemWatcher mFileSystemWatcher;

  private slots:
    /** Removes changed entry from this cache*/
    void removeChangedEntry( const QString &path );
};

#endif // QGSCAPABILITIESCACHE_H
