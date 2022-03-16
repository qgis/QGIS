/***************************************************************************
  qgslocalizeddatapathregistry.h
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


#ifndef QGSLOCALIZEDDATAPATHREGISTRY_H
#define QGSLOCALIZEDDATAPATHREGISTRY_H


#include <QDir>
#include <QList>
#include <QReadWriteLock>

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgssettingsentryimpl.h"

/**
 * \ingroup core
 * \brief A registry class to hold localized data paths which can be used for basemaps, logos, etc.
 * Paths are meant to be absolute paths and are stored by order of preference.
 *
 * If a layer from one of the paths is loaded, it will be saved as localized in the project file.
 * For instance, if you have `C:/my_maps` in your localized paths,
 * `C:/my_maps/my_country/ortho.tif` will be save in your project as `localized:my_country/ortho.tif`.
 *
 * The resolving of the file paths happens in QgsPathResolver.
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsLocalizedDataPathRegistry
{
  public:
    QgsLocalizedDataPathRegistry();

    //! Returns the global path if the file has been found in one of the paths, an empty string otherwise
    QString globalPath( const QString &localizedPath ) const;

    //! Returns the localized path if the file has been found in one of the path, an empty string otherwise
    QString localizedPath( const QString &globalPath ) const;

    //! Returns a list of registered localized paths
    QStringList paths() const;

    //! Sets the complete list of localized path
    void setPaths( const QStringList &paths ) SIP_SKIP;

    /**
     * Registers a localized path
     * If \a position is given, the path is inserted at the given position in the list
     * Since the paths are stored by order of preference, lower positions in the list take precedence.
     */
    void registerPath( const QString &path, int position = -1 );

    //! Unregisters a localized path
    void unregisterPath( const QString &path );

#ifndef SIP_RUN
    //! Settings entry localized data paths
    static const inline QgsSettingsEntryStringList settingsLocalizedDataPaths = QgsSettingsEntryStringList( QStringLiteral( "localized_data_paths" ), QgsSettings::Prefix::QGIS, QStringList() );
#endif

  private:
#ifdef SIP_RUN
    QgsLocalizedDataPathRegistry( const QgsLocalizedDataPathRegistry &other )
    {}
#endif

    void readFromSettings();
    void writeToSettings();

    QList<QDir> mPaths;
    mutable QReadWriteLock mLock;
};

#endif // QGSLOCALIZEDDATAPATHREGISTRY_H
