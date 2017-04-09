/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsuserprofile.h                                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/


class QgsUserProfile
{
%Docstring
 User profile contains information about the user profile folders on the machine.
 In QGIS 3 all settings, plugins, etc were moved into a %APPDATA%/profiles folder for each platform.
 This allows for manage different user profiles per machine vs the single default one that was allowed in the
 pass.

 A user profile is all settings and anything that used to be found in .qgis3 in the users home folder.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsuserprofile.h"
%End
  public:

    QgsUserProfile( QString folder );
%Docstring
 Reference to a existing user profile folder.
 Profile folder should be created using QgsProfileManager.
 @param folder An existing profile folder as the base of the user profile.
%End

    const QString folder() const;
%Docstring
 The base folder for the user profile.
 @return
 :rtype: str
%End

    const QString parentFolder() const;
%Docstring
 The parent folder for the profile folder.
 @return
 :rtype: str
%End

    const QString name() const;
%Docstring
 The name for the user profile.
 @return
 :rtype: str
%End

    void initSettings() const;
%Docstring
 Init the settings from the user folder.
%End

    const QString alias() const;
%Docstring
 Return the alias for the user profile. Reads the alias from .profile
 in the profile folder.
 @return If no alais is set name() is returned.
 :rtype: str
%End

    bool setAlias( const QString &alias );
%Docstring
 Set the alias of the profile.  The alias is a user friendly name.
 @param alias A user friendly name for the profile.
 @return True of setting the alias was successful.
 :rtype: bool
%End

    const QIcon icon() const;
%Docstring
 The icon for the user profile.
 @return
 :rtype: QIcon
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsuserprofile.h                                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
