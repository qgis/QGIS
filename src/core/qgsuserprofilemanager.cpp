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

#include <memory>

#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgssettings.h"
#include "qgsuserprofile.h"

#include <QDir>
#include <QFile>
#include <QProcess>
#include <QStandardPaths>
#include <QTextStream>

#include "moc_qgsuserprofilemanager.cpp"

QgsUserProfileManager::QgsUserProfileManager( const QString &rootLocation, QObject *parent )
  : QObject( parent )
{
  setRootLocation( rootLocation );
}

QString QgsUserProfileManager::resolveProfilesFolder( const QString &basePath )
{
  return basePath + QDir::separator() + "profiles";
}

std::unique_ptr< QgsUserProfile > QgsUserProfileManager::getProfile( const QString &defaultProfile, bool createNew, bool initSettings )
{
  const QString profileName = defaultProfile.isEmpty() ? defaultProfileName() : defaultProfile;

  if ( createNew && !profileExists( defaultProfile ) )
  {
    createUserProfile( profileName );
  }

  std::unique_ptr< QgsUserProfile > profile = profileForName( profileName );
  if ( initSettings )
    profile->initSettings();

  return profile;
}

void QgsUserProfileManager::setRootLocation( const QString &rootProfileLocation )
{
  mRootProfilePath = rootProfileLocation;

  //updates (or removes) profile file watcher for new root location
  setNewProfileNotificationEnabled( mWatchProfiles );

  mSettings = std::make_unique<QSettings>( settingsFile(), QSettings::IniFormat );
}

void QgsUserProfileManager::setNewProfileNotificationEnabled( bool enabled )
{
  mWatchProfiles = enabled;
  if ( mWatchProfiles && !mRootProfilePath.isEmpty() && QDir( mRootProfilePath ).exists() )
  {
    mWatcher = std::make_unique<QFileSystemWatcher>( );
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
  const QString defaultName = u"default"_s;
  // If the profiles.ini doesn't have the default profile we grab it from
  // global settings as it might be set by the admin.
  // If the overrideProfile flag is set then no matter what the profiles.ini says we always take the
  // global profile.
  const QgsSettings globalSettings;
  if ( !mSettings->contains( u"/core/defaultProfile"_s ) || globalSettings.value( u"overrideLocalProfile"_s, false, QgsSettings::Core ).toBool() )
  {
    return globalSettings.value( u"defaultProfile"_s, defaultName, QgsSettings::Core ).toString();
  }
  return mSettings->value( u"/core/defaultProfile"_s, defaultName ).toString();
}

void QgsUserProfileManager::setDefaultProfileName( const QString &name )
{
  mSettings->setValue( u"/core/defaultProfile"_s, name );
  mSettings->sync();
}

void QgsUserProfileManager::setDefaultFromActive()
{
  setDefaultProfileName( userProfile()->name() );
}

QString QgsUserProfileManager::lastProfileName() const
{
  return mSettings->value( u"/core/lastProfile"_s, QString() ).toString();
}

void QgsUserProfileManager::updateLastProfileName( )
{
  mSettings->setValue( u"/core/lastProfile"_s, userProfile()->name() );
  mSettings->sync();
}

Qgis::UserProfileSelectionPolicy QgsUserProfileManager::userProfileSelectionPolicy() const
{
  return static_cast< Qgis::UserProfileSelectionPolicy >( mSettings->value( u"/core/selectionPolicy"_s, 0 ).toInt() );
}

void QgsUserProfileManager::setUserProfileSelectionPolicy( Qgis::UserProfileSelectionPolicy policy )
{
  mSettings->setValue( u"/core/selectionPolicy"_s, static_cast< int >( policy ) );
  mSettings->sync();
}

QStringList QgsUserProfileManager::allProfiles() const
{
  return QDir( mRootProfilePath ).entryList( QDir::Dirs | QDir::NoDotAndDotDot );
}

bool QgsUserProfileManager::profileExists( const QString &name ) const
{
  return allProfiles().contains( name );
}

std::unique_ptr< QgsUserProfile > QgsUserProfileManager::profileForName( const QString &name ) const
{
  const QString profilePath = mRootProfilePath + QDir::separator() + name;
  return std::make_unique< QgsUserProfile >( profilePath );
}

QgsError QgsUserProfileManager::createUserProfile( const QString &name )
{
  QgsError error;

  // TODO Replace with safe folder name

  const QDir folder( mRootProfilePath + QDir::separator() + name );
  if ( !folder.exists() )
  {
    if ( !QDir().mkpath( folder.absolutePath() ) )
    {
      error.append( tr( "Cannot write '%1'" ).arg( folder.absolutePath() ) );
      return error;
    }
  }

  QFile qgisPrivateDbFile( folder.absolutePath() + QDir::separator() + "qgis.db" );

  // first we look for ~/.qgis/qgis.db
  if ( !qgisPrivateDbFile.exists() )
  {
    // if it doesn't exist we copy it from the global resources dir
    const QString qgisMasterDbFileName = QgsApplication::qgisMasterDatabaseFilePath();
    QFile masterFile( qgisMasterDbFileName );

    //now copy the master file into the users .qgis dir
    if ( !masterFile.copy( qgisPrivateDbFile.fileName() ) )
    {
      error.append( tr( "Could not copy master database to %1" ).arg( qgisPrivateDbFile.fileName() ) );
    }
    else
    {
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

QSettings *QgsUserProfileManager::settings()
{
  return mSettings.get();
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
  arguments << u"--profile"_s << name;
  QgsDebugMsgLevel( u"Starting instance from %1 with %2"_s.arg( path ).arg( arguments.join( " " ) ), 2 );
  QProcess::startDetached( path, arguments, QDir::toNativeSeparators( QCoreApplication::applicationDirPath() ) );
#else
  Q_UNUSED( name )
  Q_ASSERT( "Starting the user profile is not supported on the platform" );
#endif //QT_CONFIG(process)
}

void QgsUserProfileManager::setActiveUserProfile( const QString &profile )
{
  if ( ! mUserProfile )
  {
    mUserProfile = profileForName( profile );
  }
}
