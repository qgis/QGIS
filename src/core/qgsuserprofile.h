/***************************************************************************
                        qgsuserprofile.h
     --------------------------------------
    Date                 :  Jul-2017
    Copyright            : (C) 2017 by Nathan Woodrow
    Email                : woodrow.nathan at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSUSERPROFILE_H
#define QGSUSERPROFILE_H

#include "qgis_core.h"
#include "qgserror.h"
#include <QIcon>

/**
 * \ingroup core
 * \brief User profile contains information about the user profile folders on the machine.
 * In QGIS 3 all settings, plugins, etc were moved into a %APPDATA%/profiles folder for each platform.
 * This allows for manage different user profiles per machine vs the single default one that was allowed in the
 * past.
 *
 * A user profile is all settings and anything that used to be found in .qgis3 in the users home folder.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsUserProfile
{
  public:

    /**
     * Reference to a existing user profile folder.
     * Profile folder should be created using QgsProfileManager.
     * \param folder An existing profile folder as the base of the user profile.
     */
    QgsUserProfile( const QString &folder );

    /**
     * The base folder for the user profile.
     */
    const QString folder() const;

    /**
     * Check of the profile is in a valid state.
     */
    QgsError validate() const;

    /**
     * The name for the user profile.
     */
    const QString name() const;

    /**
     * Init the settings from the user folder.
     */
    void initSettings() const;

    /**
     * Returns the alias for the user profile.
     * \return If no alias is set name() is returned.
     */
    const QString alias() const;

    /**
     * Set the alias of the profile. The alias is a user friendly name.
     * \param alias A user friendly name for the profile.
     * \return TRUE of setting the alias was successful.
     */
    QgsError setAlias( const QString &alias ) const;

    /**
     * The icon for the user profile.
     * \return A QIcon for the users
     */
    const QIcon icon() const;

  private:
    QString qgisDB() const;
    QString mProfileFolder;
};

#endif
