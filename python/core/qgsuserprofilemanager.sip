/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsuserprofilemanager.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsUserProfileManager : QObject
{
%Docstring
User profile manager is used to manager list, and manage user profiles on the users machine.

In QGIS 3 all settings, plugins, etc were moved into a %APPDATA%/profiles folder for each platform.
This allows for manage different user profiles per machine vs the single default one that was allowed in the
past.

A user profile is all settings and anything that used to be found in .qgis3 in the users home folder.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsuserprofilemanager.h"
%End
  public:

    QgsUserProfileManager( const QString &rootLocation = QString(), QObject *parent = 0 );
%Docstring
User profile manager used to manage user profiles for the instance of QGIS.
%End

    static QString resolveProfilesFolder( const QString &basePath = QString() );
%Docstring
Resolves the profiles folder for the given path. Path will have \\profiles appended to the path

:param basePath: The base path to resolve the path from to append the \\profiles folder to.

:return: The root path to store user profiles.
%End

    QgsUserProfile *getProfile( const QString &defaultProfile = "default", bool createNew = true, bool initSettings = true ) /Factory/;
%Docstring
Return the profile from the given root profile location.
If no name is given it returns a profile called "default".
By default will create the profile folder if not found.
By default will init the user settings.

:param defaultProfile: The profile name to find. Empty profile name will return "default" for the name.
:param createNew: Create the profile folder if it doesn't exist.
:param initSettings: if the settings should be initialized

:return: The user profile

.. note::

   Returns a new :py:class:`QgsUserProfile`. Ownership transferred to caller.
%End

    void setRootLocation( const QString &rootProfileLocation );
%Docstring
Set the root profile location for the profile manager. All profiles are loaded from this
location. Will also contain a profiles.ini for holding profile settings.

:param rootProfileLocation: Path to the top level profile folder which contains folders for each profile.
%End

    QString rootLocation();
%Docstring
Returns the path to the root profiles location.

:return: The root path to the profiles folder.
%End

    void setNewProfileNotificationEnabled( bool enabled );
%Docstring
Sets whether the manager should watch for the creation of new user profiles and emit
the profilesChanged() signal when this occurs. By default new profile notification
is disabled.

Before calling this, ensure that the correct root location has been set via
calling setRootLocation().

.. seealso:: :py:func:`isNewProfileNotificationEnabled`
%End

    bool isNewProfileNotificationEnabled() const;
%Docstring
Returns whether the manager is watching for the creation of new user profiles and emitting
the profilesChanged() signal when this occurs. By default new profile notification
is disabled.

.. seealso:: :py:func:`setNewProfileNotificationEnabled`
%End

    bool rootLocationIsSet() const;
%Docstring
Check if the root location has been set for the manager.

:return: True if the root location has been set.
%End

    QStringList allProfiles() const;
%Docstring
A list of all found profile names.

:return:
%End

    bool profileExists( const QString &name ) const;
%Docstring
Check if a profile exists.

:return: False if the profile can't be found.
%End

    QString defaultProfileName() const;
%Docstring
Returns the name of the default profile that has been set in .default.
First checks profile.ini in \\profiles folder
Then checks defaultProfile in global settings
Finally returns "default" if all else fails

:return: The name of the default profile.

.. note::

   Setting overrideLocalProfile in global settings will always ignore profiles.ini
%End

    void setDefaultProfileName( const QString &name );
%Docstring
Sets the default profile name. The default profile name is used when loading QGIS
with no arguments.

:param name: The name of the profile to save.
%End

    void setDefaultFromActive();
%Docstring
Set the default profile name from the current active profile.
%End

    QgsUserProfile *profileForName( const QString &name ) const /Factory/;
%Docstring
Return the profile found for a given name.

:param name: The name of the profile to return.

:return: A QgsUserprofile pointing to the location of the user profile.
%End

    QgsError createUserProfile( const QString &name );
%Docstring
Create a user profile given by the name

:param name:

:return: A QgsError which report if there was any error creating the user profile.
%End

    QgsError deleteProfile( const QString &name );
%Docstring
Deletes a profile from the root profiles folder.

:param name: The name of the profile to delete.

:return: A QgsError with a message if the profile failed to be deleted.

.. note::

   There is no undo on this as it deletes the folder from the machine.
%End

    QgsUserProfile *userProfile();
%Docstring
The currently active user profile.

:return: The currently active user profile.
%End

    void setActiveUserProfile( const QString &profile );
%Docstring
Sets the active profile in the manager.
This can only be set once.
Setting this again does nothing.

:param profile: The name of the active profile
%End

    void loadUserProfile( const QString &name );
%Docstring
Starts a new instance of QGIS for the given profile.

:param name: The profile to start QGIS with.
%End

  signals:

    void profilesChanged();
%Docstring
Emitted when the list of profiles is changed.

This signal will only be emitted when isNewProfileNotificationEnabled() is true.
By default new profile notification is disabled.

.. seealso:: :py:func:`isNewProfileNotificationEnabled`

.. seealso:: :py:func:`setNewProfileNotificationEnabled`
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsuserprofilemanager.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
