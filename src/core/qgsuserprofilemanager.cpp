#include "qgsuserprofilemanager.h"
#include "qgsuserprofile.h"
#include "qgsapplication.h"
#include "qgslogger.h"

#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QProcess>
#include <QStandardPaths>


QPair<QgsUserProfile *, QString> QgsUserProfileManager::getProfile( const QString &rootLocation, bool roamingConfig, const QString &defaultProfile, bool createNew )
{
  QgsUserProfileManager manager;

  if ( rootLocation.isEmpty() )
  {
    QStandardPaths::StandardLocation location = roamingConfig ? QStandardPaths::AppDataLocation : QStandardPaths::AppLocalDataLocation;
    QString rootFolder = QStandardPaths::standardLocations( location ).at( 0 );
    manager.setRootLocation( rootFolder );
  }
  else
  {
    manager.setRootLocation( rootLocation );
  }

  QString profileName = defaultProfile.isEmpty() ? manager.defaultProfileName() : defaultProfile;
  if ( createNew )
  {
    manager.createUserProfile( profileName );
  }
  QgsUserProfile *profile = manager.profileForName( profileName );
  QStringList parts = manager.rootLocation().split( QDir::separator() );
  parts.removeLast();
  QString basePath = parts.join( QDir::separator() );

  return qMakePair( profile, basePath );
}

void QgsUserProfileManager::setRootLocation( QString rootProfileLocation )
{
  mRootProfilePath = rootProfileLocation + QDir::separator() + "profiles";
  mSettings = new QSettings( settingsFile(), QSettings::IniFormat );
}

bool QgsUserProfileManager::rootLocationIsSet() const
{
  return !mRootProfilePath.isEmpty();
}

QString QgsUserProfileManager::defaultProfileName() const
{
  QString profileName = "default";
  mSettings->value( "/defaultProfile", "default" );
  return profileName;
}

void QgsUserProfileManager::setDefaultProfileName( const QString &name )
{
  mSettings->setValue( "/defaultProfile", name );
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
    // if it doesn't exist we copy it in from the global resources dir
    QString qgisMasterDbFileName = QgsApplication::qgisMasterDatabaseFilePath();
    QFile masterFile( qgisMasterDbFileName );

    //now copy the master file into the users .qgis dir
    masterFile.copy( qgisPrivateDbFile.fileName() );
  }

  return error;
}

QgsError QgsUserProfileManager::deleteProfile( const QString name )
{
  QgsError error;
  QDir folder( mRootProfilePath + QDir::separator() + name );

  // This might have be changed to something better.
  bool deleted = folder.removeRecursively();
  if ( !deleted )
  {
    error.append( ( tr( "Unable to fully delete user profile folder" ) ) );
  }
  return error;
}

QString QgsUserProfileManager::settingsFile()
{
  return  mRootProfilePath + QDir::separator() + "profiles.ini";
}

QgsUserProfile *QgsUserProfileManager::userProfile()
{
  return mUserProfile;
}

void QgsUserProfileManager::loadUserProfile( const QgsUserProfile *profile )
{
  QString path = QDir::toNativeSeparators( QCoreApplication::applicationFilePath() );
  QStringList arguments;
  arguments << "--profile" << profile->name();
  QProcess::startDetached( path, arguments, QDir::toNativeSeparators( QCoreApplication::applicationDirPath() ) );
}

void QgsUserProfileManager::setActiveUserProfile( const QString &profile )
{
  if ( ! mUserProfile )
  {
    mUserProfile = profileForName( profile );
  }
}

void QgsUserProfileManager::loadUserProfile( const QString &name )
{
  QgsUserProfile *profile = profileForName( name );
  loadUserProfile( profile );
}
