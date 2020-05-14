/***************************************************************************
  qgsbasemappathregistry.h
  --------------------------------------
  Date                 : May 2020
  Copyright            : (C) 2020 by Denis Rouzaud
  Email                : denis.rouzaud
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSBASEMAPPATHREGISTRY_H
#define QGSBASEMAPPATHREGISTRY_H


#include <QDir>
#include <QList>
#include <QReadWriteLock>

#include "qgis_core.h"
#include "qgis_sip.h"

/**
 * \ingroup core
 * A registry class to hold paths of basemaps
 * Paths are meant to be absolute paths and are stored by order of preference.
 *
 * If a layer from one of the paths is loaded, it will be saved as basemap in the project file.
 * For instance, if you have C:\my_maps in your basemap paths,
 * C:\my_maps\my_country\ortho.tif will be save in your project as basemap:my_country\ortho.tif
 *
 * The resolving of the file paths happens in QgsPathResolver.
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsBasemapPathRegistry
{
  public:
    QgsBasemapPathRegistry();

    ~QgsBasemapPathRegistry() = default;

    //! Returns the full path if the file has been found in one of the paths, an empty string otherwise
    QString fullPath( const QString &relativePath ) const;

    //! Returns the relative path if the file has been found in one of the path, an empty string otherwise
    QString relativePath( const QString &fullPath ) const;

    //! Returns a list of registered basemap paths
    QStringList paths() const;

    //! Sets the complete list of basemap path
    void setPaths( const QStringList &paths ) SIP_SKIP;

    /**
     * Registers a basemap path
     * If \a position is given, the path is inserted at the given position in the list
     * Since the paths are stored by order of preference, lower positions in the list take precedence.
     */
    void registerPath( const QString &path, int position = -1 );

    //! Unregisters a basemap path
    void unregisterPath( const QString &path );

  private:
#ifdef SIP_RUN
    QgsBasemapPathRegistry( const QgsBasemapPathRegistry &other )
    {}
#endif

    void readFromSettings();
    void writeToSettings();

    QList<QDir> mPaths;
    mutable QReadWriteLock mLock;
};

#endif // QGSBASEMAPPATHREGISTRY_H
