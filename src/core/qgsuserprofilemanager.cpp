/***************************************************************************
                        qgsuserprofilemanager.cpp
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

#include "qgsuserprofilemanager.h"
#include "qgsuserprofile.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgssettings.h"

#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QProcess>
#include <QStandardPaths>


QgsUserProfileManager::QgsUserProfileManager( const QString &rootLocation, QObject *parent )
  : QObject( parent )
{
  setRootLocation( rootLocation );
}

QString QgsUserProfileManager::resolveProfilesFolder( const QString &basePath )
{
  return basePath + QDir::separator() + "profiles";
}

QgsUserProfile *QgsUserProfileManager::getProfile( const QString &defaultProfile, bool createNew, bool initSettings )
{
  const QString profileName = defaultProfile.isEmpty() ? defaultProfileName() : defaultProfile;

  if ( createNew && !profileExists( defaultProfile ) )
  {
    createUserProfile( profileName );
  }

  QgsUserProfile *profile = profileForName( profileName );
  if ( initSettings )
    profile->initSettings();

  return profile;
}

void QgsUserProfileManager::setRootLocation( const QString &rootProfileLocation )
{
  mRootProfilePath = rootProfileLocation;

  //updates (or removes) profile file watcher for new root location
  setNewProfileNotificationEnabled( mWatchProfiles );

  mSettings.reset( new QSettings( settingsFile(), QSettings::IniFormat ) );
}

void QgsUserProfileManager::setNewProfileNotificationEnabled( bool enabled )
{
  mWatchProfiles = enabled;
  if ( mWatchProfiles && !mRootProfilePath.isEmpty() && QDir( mRootProfilePath ).exists() )
  {
    mWatcher.reset( new QFileSystemWatcher() );
    mWatcher->addPath( mRootProfilePath );
    connect( mWatcher.get(), &QFileSystemWatcher::directoryChanged, this, [this]
    {
      emit profilesChanged();
    } );
  }
  else
  {
    mWatcher.reset();
  }
}

bool QgsUserProfileManager::isNewProfileNotificationEnabled() const
{
  return static_cast< bool >( mWatcher.get() );
}

bool QgsUserProfileManager::rootLocationIsSet() const
{
  return !mRootProfilePath.isEmpty();
}

QString QgsUserProfileManager::defaultProfileName() const
{
  const QString defaultName = QStringLiteral( "default" );
  // If the profiles.ini doesn't have the default profile we grab it from
  // global settings as it might be set by the admin.
  // If the overrideProfile flag is set then no matter what the profiles.ini says we always take the
  // global profile.
  const QgsSettings globalSettings;
  if ( !mSettings->contains( QStringLiteral( "/core/defaultProfile" ) ) || globalSettings.value( QStringLiteral( "overrideLocalProfile" ), false, QgsSettings::Core ).toBool() )
  {
    return globalSettings.value( QStringLiteral( "defaultProfile" ), defaultName, QgsSettings::Core ).toString();
  }
  return mSettings->value( QStringLiteral( "/core/defaultProfile" ), defaultName ).toString();
}

void QgsUserProfileManager::setDefaultProfileName( const QString &name )
{
  mSettings->setValue( QStringLiteral( "/core/defaultProfile" ), name );
  mSettings->sync();
}

void QgsUserProfileManager::setDefaultFromActive()
{
  setDefaultProfileName( userProfile()->name() );
}

QStringList QgsUserProfileManager::allProfiles() const
{
  return QDir( mRootProfilePath ).entryList( QDir::Dirs | QDir::NoDotAndDotDot );
}

bool QgsUserProfileManager::profileExists( const QString &name ) const
{
  return allProfiles().contains( name );
}

QgsUserProfile *QgsUserProfileManager::profileForName( const QString &name ) const
{
  const QString profilePath = mRootProfilePath + QDir::separator() + name;
  return new QgsUserProfile( profilePath );
}

QgsError QgsUserProfileManager::createUserProfile( const QString &name )
{
  QgsError error;

  // TODO Replace with safe folder name

  const QDir folder( mRootProfilePath + QDir::separator() + name );
  if ( !folder.exists() )
  {
    QDir().mkpath( folder.absolutePath() );
  }

  QFile qgisPrivateDbFile( folder.absolutePath() + QDir::separator() + "qgis.db" );

  // first we look for ~/.qgis/qgis.db
  if ( !qgisPrivateDbFile.exists() )
  {
    // if it doesn't exist we copy it from the global resources dir
    const QString qgisMasterDbFileName = QgsApplication::qgisMasterDatabaseFilePath();
    QFile masterFile( qgisMasterDbFileName );

    //now copy the master file into the users .qgis dir
    masterFile.copy( qgisPrivateDbFile.fileName() );

    // In some packaging systems, the master can be read-only. Make sure to make
    // the copy user writable.
    const QFile::Permissions perms = QFile( qgisPrivateDbFile.fileName() ).permissions();
    if ( !( perms & QFile::WriteOwner ) )
    {
      if ( !qgisPrivateDbFile.setPermissions( perms | QFile::WriteOwner ) )
      {
        error.append( tr( "Can not make '%1' user writable" ).arg( qgisPrivateDbFile.fileName() ) );
      }
    }
  }

  if ( error.isEmpty() )
  {
    emit profilesChanged();
  }

  return error;
}

QgsError QgsUserProfileManager::deleteProfile( const QString &name )
{
  QgsError error;
  QDir folder( mRootProfilePath + QDir::separator() + name );

  // This might have to be changed to something better.
  const bool deleted = folder.removeRecursively();
  if ( !deleted )
  {
    error.append( ( tr( "Unable to fully delete user profile folder" ) ) );
  }
  else
  {
    emit profilesChanged();
  }
  return error;
}

QString QgsUserProfileManager::settingsFile() const
{
  return  mRootProfilePath + QDir::separator() + "profiles.ini";
}

QgsUserProfile *QgsUserProfileManager::userProfile()
{
  return mUserProfile.get();
}

void QgsUserProfileManager::loadUserProfile( const QString &name )
{
#if QT_CONFIG(process)
  const QString path = QDir::toNativeSeparators( QCoreApplication::applicationFilePath() );
  QStringList arguments;
  arguments << QCoreApplication::arguments();
  // The first is the path to the application
  // on Windows this might not be case so we need to handle that
  // http://doc.qt.io/qt-5/qcoreapplication.html#arguments
  arguments.removeFirst();
  arguments << QStringLiteral( "--profile" ) << name;
  QgsDebugMsg( QStringLiteral( "Starting instance from %1 with %2" ).arg( path ).arg( arguments.join( " " ) ) );
  QProcess::startDetached( path, arguments, QDir::toNativeSeparators( QCoreApplication::applicationDirPath() ) );
#else
  Q_UNUSED( name )
  Q_ASSERT( "Starting the user profile is not supported on the platform" );
#endif //QT_CONFIG(process)
}

void QgsUserProfileManager::setActiveUserProfile( const QString &profile )
{
  if ( ! mUserProfile.get() )
  {
    mUserProfile.reset( profileForName( profile ) );
  }
}
