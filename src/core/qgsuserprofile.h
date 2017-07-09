#ifndef QGSUSERPROFILE_H
#define QGSUSERPROFILE_H

#include "qgis_core.h"
#include "qgserror.h"
#include <QIcon>

/** \ingroup core
 * User profile contains information about the user profile folders on the machine.
 * In QGIS 3 all settings, plugins, etc were moved into a %APPDATA%/profiles folder for each platform.
 * This allows for manage different user profiles per machine vs the single default one that was allowed in the
 * pass.
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
     * @param folder An existing profile folder as the base of the user profile.
     */
    QgsUserProfile( QString folder );

    /**
     * The base folder for the user profile.
     * @return
     */
    const QString folder() const;

    /**
     * Check of the profile is in a valid state.
     * @return Any errors the profile has.
     */
    QgsError validate() const;

    /**
     * The name for the user profile.
     * @return
     */
    const QString name() const;

    /**
     * Init the settings from the user folder.
     */
    void initSettings() const;

    /**
     * Return the alias for the user profile. Reads the alias from .profile
     * in the profile folder.
     * @return If no alais is set name() is returned.
     */
    const QString alias() const;

    /**
     * Set the alias of the profile.  The alias is a user friendly name.
     * @param alias A user friendly name for the profile.
     * @return True of setting the alias was successful.
     */
    bool setAlias( const QString &alias );

    /**
     * The icon for the user profile.
     * @return
     */
    const QIcon icon() const;

  private:
    QString qgisDB() const;
    QString mProfileFolder;
};

#endif
