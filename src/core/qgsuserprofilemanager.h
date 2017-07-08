#ifndef QGSUSERprofileMANAGER_H
#define QGSUSERprofileMANAGER_H

#include <QSettings>


#include "qgis_core.h"
#include "qgserror.h"

class QgsUserProfile;

/** \ingroup core
 * User profile manager is used to manager list, and manage user profiles on the users machine.
 *
 * In QGIS 3 all settings, plugins, etc were moved into a %APPDATA%/profiles folder for each platform.
 * This allows for manage different user profiles per machine vs the single default one that was allowed in the
 * pass.
 *
 * A user profile is all settings and anything that used to be found in .qgis3 in the users home folder.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsUserProfileManager : public QObject
{
    Q_OBJECT

  public:

    /**
     * Return the profile from the given root profile location. If no name is given it returns a profile called "default".
     * @param rootLocation The root location to search for the profile.
     * @param roamingConfig True if to use Qt to find the roaming folder based on the OS. e.g APPDATA on Windows. Default true.
     * @param defaultProfile The profile name to find. Empty profile name will return "default" for the name.
     * @return The QgsUserProfile for the given profile name.
     */
    static QgsUserProfile *getProfile( const QString &rootLocation = QString(), bool roamingConfig = true, const QString &defaultProfile = "default" );

    /**
     * Set the root profile location for the profile manager. All profiles are loaded from this
     * location.
     * @param rootProfileLocation Path to the top level profile folder which contains folders for each profile.
     */
    void setRootLocation( QString rootProfileLocation );

    /**
     * Returns the path to the root profiles location.
     * @return The root path to the profiles folder.
     */
    QString rootLocation() { return mRootProfilePath; }

    /**
     * Check if the root location has been set for the manager.
     * @return True if the root location has been set.
     */
    bool rootLocationIsSet() const;

    /**
     * A list of all found profile names.
     * @return
     */
    QStringList allProfiles() const;

    /**
     * Returns the name of the default profile that has been set in .default.
     * @return The name of the default profile.
     */
    QString defaultProfileName() const;

    /**
     * Sets the default profile name.  The default profile name is used when loading QGIS
     * with no arguments.
     * @param name The name of the profile to save.
     */
    void setDefaultProfileName( const QString &name );

    /**
     * Set the default profile name from the current active profile.
     */
    void setDefaultFromActive();

    /**
     * Return the profile found for a given name.
     * @param name The name of the profile to return.
     * @return A QgsUserprofile pointing to the location of the user profile.
     */
    QgsUserProfile *profileForName( const QString name ) const;

    /**
     * Create a user profile given by the name
     * @param name
     * @return A QgsError which report if there was any error creating the user profile.
     */
    QgsError createUserProfile( const QString &name );

    /**
     * Deletes a profile from the root profiles folder.
     * @note There is no undo on this as it deletes the folder from the machine.
     * @param name The name of the profile to delete.
     * @return A QgsError with a message if the profile failed to be deleted.
     */
    QgsError deleteProfile( const QString name );

    /**
     * The currently active user profile.
     * @return The currently active user profile.
     */
    QgsUserProfile *userProfile();

    /**
     * Sets the active profile in the manager.
     * This can only be set once.
     * Setting this again does nothing.
     *
     * @param profile The name of the active profile
     */
    void setActiveUserProfile( const QString &profile );

    /**
     * Starts a new instance of QGIS for the given profile.
     * @param name The profile to start QGIS with.
     */
    void loadUserProfile( const QString &name );

    /**
     * Starts a new instance of QGIS for the given profile.
     * @param profile The profile to start QGIS with.
     */
    void loadUserProfile( const QgsUserProfile *profile );

  private:
    QString mRootProfilePath;

    QgsUserProfile *mUserProfile = nullptr;

    QString settingsFile();

    QSettings *mSettings = nullptr;
};

#endif // QGSUSERprofileMANAGER_H
