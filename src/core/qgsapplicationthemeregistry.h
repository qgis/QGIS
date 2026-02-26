/***************************************************************************
                             qgsapplicationthemeregistry.h
                             ------------------------
    begin                : January 2026
    copyright            : (C) 2026 by Mathieu Pellerin
    email                : mathieu at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSUSERINTERFACETHEMEREGISTRY_H
#define QGSUSERINTERFACETHEMEREGISTRY_H

#include "qgis_core.h"

#include <QHash>
#include <QList>

/**
 * \ingroup core
 * \class QgsApplicationThemeRegistry
 * \brief Registry of user interface themes.
 *
 * A registry of user interface themes. This class should be accessed via
 * QgsApplication::userInterfaceThemeRegistry().
 *
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsApplicationThemeRegistry
{

  public:

    /**
     * Constructor for an empty user interface theme registry
     */
    QgsApplicationThemeRegistry();

    ~QgsApplicationThemeRegistry() = default;

    /**
     * Adds a user interface theme into the registry. If the provided theme
     * name is already in the registry, the theme will not be added.
     * \param name The theme name to be added
     * \param folder The theme folder where theme-related files are located
     * \returns TRUE if the theme was added into the registry
     */
    bool addTheme( const QString &name, const QString &folder );

    /**
     * Removes a user interface theme with a matching name from the
     * registry.
     * \param name The theme name to be removed
     * \returns TRUE if the theme was removed from the registry
     */
    bool removeTheme( const QString &name );

    /**
     * Returns the list of available user interface themes.
     */
    QStringList themes() const;

    /**
     * Returns the user interface theme folder for a matching \a name.
     */
    QString themeFolder( const QString &name ) const;

    /**
     * Returns a map of user interface theme names and folders.
     */
    QHash<QString, QString> themeFolders() const;

  private:

    /**
     * Adds all default user interface themes.
     */
    void addApplicationThemes();

    QHash<QString, QString> mThemes;

};

#endif
