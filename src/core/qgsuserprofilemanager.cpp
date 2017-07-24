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
  mWatcher.reset( new QFileSystemWatcher() );
  setRootLocation( rootLocation );
  connect( mWatcher.get(), &QFileSystemWatcher::directoryChanged, this, [this]
  {
    emit profilesChanged();
  } );
}

QString QgsUserProfileManager::resolveProfilesFolder( const QString &basePath )
{
  return basePath + QDir::separator() + "profiles";
}

QgsUserProfile *QgsUserProfileManager::getProfile( const QString &defaultProfile, bool createNew, bool initSettings )
{
  QString profileName = defaultProfile.isEmpty() ? defaultProfileName() : defaultProfile;

  if ( createNew && !profileExists( defaultProfile ) )
  {
    createUserProfile( profileName );
  }

  QgsUserProfile *profile = profileForName( profileName );
  if ( initSettings )
    profile->initSettings();

  return profile;
}

void QgsUserProfileManager::setRootLocation( QString rootProfileLocation )
{
  mRootProfilePath = rootProfileLocation;

  if ( !mWatcher->directories().isEmpty() )
  {
    mWatcher->removePaths( mWatcher->directories() );
  }

  if ( QDir( mRootProfilePath ).exists() )
  {
    mWatcher->addPath( mRootProfilePath );
  }
  mSettings = new QSettings( settingsFile(), QSettings::IniFormat );
}

bool QgsUserProfileManager::rootLocationIsSet() const
{
  return !mRootProfilePath.isEmpty();
}

QString QgsUserProfileManager::defaultProfileName() const
{
  QString defaultName = "default";
  // If the profiles.ini doesn't have the default profile we grab it from
  // global settings as it might be set by the admin.
  // If the overrideProfile flag is set then no matter what the profiles.ini says we always take the
  // global profile.
  QgsSettings globalSettings;
  if ( !mSettings->contains( "/core/defaultProfile" ) || globalSettings.value( "overrideLocalProfile", false, QgsSettings::Core ).toBool() )
  {
    return globalSettings.value( "defaultProfile", defaultName, QgsSettings::Core ).toString();
  }
  return mSettings->value( "/core/defaultProfile", defaultName ).toString();
}

void QgsUserProfileManager::setDefaultProfileName( const QString &name )
{
  mSettings->setValue( "/core/defaultProfile", name );
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

QgsUserProfile *QgsUserProfileManager::profileForName( const QString name ) const
{
  QString profilePath = mRootProfilePath + QDir::separator() + name;
  return new QgsUserProfile( profilePath );
}

QgsError QgsUserProfileManager::createUserProfile( const QString &name )
{
  QgsError error;

  // TODO Replace with safe folder name

  QDir folder( mRootProfilePath + QDir::separator() + name );
  if ( !folder.exists() )
  {
    QDir().mkdir( folder.absolutePath() );
  }

  QFile qgisPrivateDbFile( folder.absolutePath() + QDir::separator() + "qgis.db" );

  // first we look for ~/.qgis/qgis.db
  if ( !qgisPrivateDbFile.exists() )
  {
    // if it doesn't exist we copy it from the global resources dir
    QString qgisMasterDbFileName = QgsApplication::qgisMasterDatabaseFilePath();
    QFile masterFile( qgisMasterDbFileName );

    //now copy the master file into the users .qgis dir
    masterFile.copy( qgisPrivateDbFile.fileName() );
  }

  if ( error.isEmpty() )
  {
    emit profilesChanged();
  }

  return error;
}

QgsError QgsUserProfileManager::deleteProfile( const QString name )
{
  QgsError error;
  QDir folder( mRootProfilePath + QDir::separator() + name );

  // This might have to be changed to something better.
  bool deleted = folder.removeRecursively();
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

QString QgsUserProfileManager::settingsFile()
{
  return  mRootProfilePath + QDir::separator() + "profiles.ini";
}

QgsUserProfile *QgsUserProfileManager::userProfile()
{
  return mUserProfile.get();
}

void QgsUserProfileManager::loadUserProfile( const QString &name )
{
  QString path = QDir::toNativeSeparators( QCoreApplication::applicationFilePath() );
  QStringList arguments;
  arguments << QCoreApplication::arguments();
  // The first is the path to the application
  // on Windows this might not be case so we need to handle that
  // http://doc.qt.io/qt-5/qcoreapplication.html#arguments
  arguments.removeFirst();

  arguments << "--profile" << name;
  QgsDebugMsg( QString( "Starting instance from %1 with %2" ).arg( path ).arg( arguments.join( " " ) ) );
  QProcess::startDetached( path, arguments, QDir::toNativeSeparators( QCoreApplication::applicationDirPath() ) );
}

void QgsUserProfileManager::setActiveUserProfile( const QString &profile )
{
  if ( ! mUserProfile.get() )
  {
    mUserProfile.reset( profileForName( profile ) );
  }
}
