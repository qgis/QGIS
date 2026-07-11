/***************************************************************************
  qgsqgisprofileimporter.cpp
  --------------------------
  begin                : June 2026
  copyright            : (C) 2026 by Francesco Mazzi
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsqgisprofileimporter.h"

#include "qgsapplication.h"
#include "qgsfileutils.h"
#include "qgslogger.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QObject>
#include <QSet>
#include <QSettings>
#include <QStandardPaths>
#include <QString>
#include <QStringList>
#include <QUuid>

using namespace Qt::StringLiterals;

namespace
{

  QString legacyAppDataLocation( const QString &org, const QString &app )
  {
#if defined( Q_OS_WIN )
    const QString base = qEnvironmentVariable( u"APPDATA"_s );
#elif defined( Q_OS_MACOS )
    const QString base = QDir::homePath() + u"/Library/Application Support"_s;
#else
    const QString base = QStandardPaths::writableLocation( QStandardPaths::GenericDataLocation );
#endif
    if ( base.isEmpty() )
      return QString();

    return QDir( base ).filePath( org + u'/' + app );
  }

  const QString IMPORT_TAG = u"Strata QGIS profile import"_s;

  QString canonicalOrAbsolutePath( const QString &path )
  {
    const QFileInfo info( path );
    const QString canonicalPath = info.canonicalFilePath();
    if ( !canonicalPath.isEmpty() )
      return QDir::cleanPath( canonicalPath );

    return QDir::cleanPath( info.absoluteFilePath() );
  }

  QString profilesIniPath( const QString &rootProfileFolder )
  {
    return QDir( rootProfileFolder ).filePath( u"profiles.ini"_s );
  }

  QString targetSettingsFileName()
  {
    QString applicationName = QCoreApplication::applicationName();
    if ( applicationName.isEmpty() )
      applicationName = QString::fromLatin1( QgsApplication::QGIS_APPLICATION_NAME );

    return applicationName + u".ini"_s;
  }

  QString cleanProfileName( QString name )
  {
    name = name.trimmed();
    name.replace( '/', '_' );
    name.replace( '\\', '_' );
    if ( name.isEmpty() )
      name = u"default"_s;

    return name;
  }

  int countPythonPlugins( const QString &profilePath )
  {
    const QDir pluginsDir( QDir( profilePath ).filePath( u"python/plugins"_s ) );
    if ( !pluginsDir.exists() )
      return 0;

    return pluginsDir.entryList( QDir::Dirs | QDir::NoDotAndDotDot ).size();
  }

  bool containsCandidateName( const QList<QgsQgisProfileImporter::Candidate> &candidates, const QString &profileName, const QString &sourceRoot = QString() )
  {
    for ( const QgsQgisProfileImporter::Candidate &candidate : candidates )
    {
      if ( candidate.profileName == profileName && ( sourceRoot.isEmpty() || canonicalOrAbsolutePath( candidate.sourceRoot ) == canonicalOrAbsolutePath( sourceRoot ) ) )
        return true;
    }

    return false;
  }

  QString sourceProfilesSetting( const QString &sourceRoot, const QString &key )
  {
    QSettings settings( profilesIniPath( sourceRoot ), QSettings::IniFormat );
    return settings.value( key ).toString();
  }

  QString sourceSelectionPolicy( const QString &sourceRoot )
  {
    QSettings settings( profilesIniPath( sourceRoot ), QSettings::IniFormat );
    return settings.value( u"/core/selectionPolicy"_s ).toString();
  }

  QStringList copyExcludePatterns()
  {
    return {
      u".*\\b__pycache__$"_s,
      u".*\\.[pP][yY][cC]$"_s,
      u".*[\\\\/]cache$"_s,
      u".*[\\\\/]previewImages$"_s,
      u".*[\\\\/]gdal_pam$"_s,
    };
  }

  bool prepareSettingsFile( const QString &profilePath, QString *settingsPath, QgsError *errors )
  {
    QDir qgisSettingsDir( QDir( profilePath ).filePath( u"QGIS"_s ) );
    if ( !qgisSettingsDir.exists() && !qgisSettingsDir.mkpath( "." ) )
    {
      if ( errors )
        errors->append( QObject::tr( "Cannot create settings folder for imported profile: %1" ).arg( qgisSettingsDir.path() ), IMPORT_TAG );
      return false;
    }

    const QString targetIniPath = qgisSettingsDir.filePath( targetSettingsFileName() );
    if ( QFile::exists( targetIniPath ) )
    {
      if ( settingsPath )
        *settingsPath = targetIniPath;
      return true;
    }

    const QStringList candidateSettingsFiles {
      qgisSettingsDir.filePath( u"QGIS4.ini"_s ),
      qgisSettingsDir.filePath( u"QGIS3.ini"_s ),
    };

    for ( const QString &candidatePath : candidateSettingsFiles )
    {
      if ( candidatePath == targetIniPath || !QFile::exists( candidatePath ) )
        continue;

      if ( QFile::rename( candidatePath, targetIniPath ) )
      {
        if ( settingsPath )
          *settingsPath = targetIniPath;
        return true;
      }

      if ( errors )
        errors->append( QObject::tr( "Cannot rename settings file %1 to %2" ).arg( candidatePath, targetIniPath ), IMPORT_TAG );
      return false;
    }

    QFile newSettingsFile( targetIniPath );
    if ( !newSettingsFile.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
      if ( errors )
        errors->append( QObject::tr( "Cannot create settings file for imported profile: %1" ).arg( targetIniPath ), IMPORT_TAG );
      return false;
    }

    newSettingsFile.close();
    if ( settingsPath )
      *settingsPath = targetIniPath;
    return true;
  }

  void replaceProfilePathsInSettings( const QString &settingsPath, const QString &sourceProfilePath, const QString &targetProfilePath )
  {
    const QString cleanSourcePath = QDir::cleanPath( QFileInfo( sourceProfilePath ).absoluteFilePath() );
    const QString cleanTargetPath = QDir::cleanPath( QFileInfo( targetProfilePath ).absoluteFilePath() );

    QgsFileUtils::replaceTextInFile( settingsPath, cleanSourcePath, cleanTargetPath );
    QgsFileUtils::replaceTextInFile( settingsPath, QDir::toNativeSeparators( cleanSourcePath ), QDir::toNativeSeparators( cleanTargetPath ) );
  }

  void writeImportMarkers( const QString &settingsPath, const QgsQgisProfileImporter::Candidate &candidate )
  {
    QSettings settings( settingsPath, QSettings::IniFormat );
    settings.remove( u"/PythonPlugins/watchDogTimestamp"_s );
    settings.remove( u"PythonPlugins/watchDogTimestamp"_s );
    settings.remove( u"/PythonPlugins/watchDog"_s );
    settings.remove( u"PythonPlugins/watchDog"_s );
    settings.remove( u"/Plugins/watchDogTimestamp"_s );
    settings.remove( u"Plugins/watchDogTimestamp"_s );

    settings.setValue( u"migration/migrated_from_3"_s, true );
    settings.setValue( u"strata/qgisImport/imported"_s, true );
    settings.setValue( u"strata/qgisImport/sourceProfileName"_s, QFileInfo( candidate.profilePath ).fileName() );
    settings.setValue( u"strata/qgisImport/sourceProfilePath"_s, candidate.profilePath );
    settings.setValue( u"strata/qgisImport/sourceVersion"_s, candidate.sourceVersionLabel );
    settings.setValue( u"strata/qgisImport/importedAt"_s, QDateTime::currentDateTimeUtc().toString( Qt::ISODate ) );
    settings.sync();
  }

  bool postProcessProfile( const QgsQgisProfileImporter::Candidate &candidate, const QString &stagedProfilePath, const QString &finalProfilePath, QgsError *errors )
  {
    QString settingsPath;
    if ( !prepareSettingsFile( stagedProfilePath, &settingsPath, errors ) )
      return false;

    replaceProfilePathsInSettings( settingsPath, candidate.profilePath, finalProfilePath );
    writeImportMarkers( settingsPath, candidate );
    return true;
  }

  void appendErrors( QgsError &destination, const QgsError &source )
  {
    const QList<QgsErrorMessage> messages = source.messageList();
    for ( const QgsErrorMessage &message : messages )
      destination.append( message );
  }

} // namespace

QList<QgsQgisProfileImporter::Candidate> QgsQgisProfileImporter::detectCandidates( const QString &configLocalStorageLocation, const QString &targetRootProfileFolder )
{
  QList<Candidate> candidates;
  QSet<QString> seenRoots;
  QSet<QString> seenProfiles;
  const QString targetRootKey = canonicalOrAbsolutePath( targetRootProfileFolder );

  auto addRoot = [&]( const QString &path, const QString &versionLabel ) {
    const QDir rootDir( QDir::cleanPath( path ) );
    if ( !rootDir.exists() )
      return;

    const QString rootKey = canonicalOrAbsolutePath( rootDir.path() );
    if ( rootKey == targetRootKey || seenRoots.contains( rootKey ) )
      return;

    seenRoots.insert( rootKey );

    const QStringList profileNames = rootDir.entryList( QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name );
    for ( const QString &profileName : profileNames )
    {
      const QString profilePath = rootDir.filePath( profileName );
      const QString profileKey = canonicalOrAbsolutePath( profilePath );
      if ( seenProfiles.contains( profileKey ) )
        continue;

      seenProfiles.insert( profileKey );
      Candidate candidate;
      candidate.profileName = profileName;
      candidate.profilePath = QDir::cleanPath( profilePath );
      candidate.sourceRoot = QDir::cleanPath( rootDir.path() );
      candidate.sourceVersionLabel = versionLabel;
      candidate.pluginCount = countPythonPlugins( profilePath );
      candidates.append( candidate );
    }
  };

  const QDir configRootPath( configLocalStorageLocation );
  addRoot( QDir( legacyAppDataLocation( u"QGIS"_s, u"QGIS3"_s ) ).filePath( u"profiles"_s ), u"QGIS 3"_s );
  addRoot( QDir( legacyAppDataLocation( u"QGIS"_s, u"QGIS4"_s ) ).filePath( u"profiles"_s ), u"Profilo QGIS legacy"_s );
  addRoot( QDir::cleanPath( configRootPath.filePath( u"../QGIS3/profiles"_s ) ), u"QGIS 3"_s );
  addRoot( QDir::cleanPath( configRootPath.filePath( u"../QGIS4/profiles"_s ) ), u"Profilo QGIS legacy"_s );
  addRoot( QDir::home().filePath( u".qgis3/profiles"_s ), u"QGIS 3 legacy"_s );

  return candidates;
}

bool QgsQgisProfileImporter::shouldOfferFirstRunImport( const QString &targetRootProfileFolder, const QList<Candidate> &candidates, bool guiAvailable, bool explicitProfile, bool preventSettingsMigration )
{
  if ( !guiAvailable || explicitProfile || preventSettingsMigration || candidates.isEmpty() )
    return false;

  QSettings settings( profilesIniPath( targetRootProfileFolder ), QSettings::IniFormat );
  if ( settings.value( u"strata/qgisImport/decisionMade"_s, false ).toBool() )
    return false;

  const QDir targetRoot( targetRootProfileFolder );
  if ( !targetRoot.exists() )
    return true;

  return targetRoot.entryList( QDir::Dirs | QDir::NoDotAndDotDot ).isEmpty();
}

QgsQgisProfileImporter::ImportResult QgsQgisProfileImporter::importProfiles( const QList<Candidate> &candidates, const QString &targetRootProfileFolder, const QString &activeProfileHint )
{
  ImportResult result;
  if ( candidates.isEmpty() )
  {
    result.errors.append( QObject::tr( "No QGIS profiles were selected for import." ), IMPORT_TAG );
    return result;
  }

  QDir targetRoot( targetRootProfileFolder );
  if ( !targetRoot.exists() && !targetRoot.mkpath( "." ) )
  {
    result.errors.append( QObject::tr( "Cannot create Strata profiles folder: %1" ).arg( targetRootProfileFolder ), IMPORT_TAG );
    return result;
  }

  const QString stageName = u".strata-profile-import-%1"_s.arg( QUuid::createUuid().toString( QUuid::WithoutBraces ) );
  const QString stageRootPath = targetRoot.filePath( stageName );
  if ( !targetRoot.mkpath( stageName ) )
  {
    result.errors.append( QObject::tr( "Cannot create temporary import folder: %1" ).arg( stageRootPath ), IMPORT_TAG );
    return result;
  }

  struct PendingImport
  {
      Candidate candidate;
      QString targetName;
      QString stagedPath;
      QString finalPath;
  };

  QList<PendingImport> pendingImports;
  QStringList reservedNames;
  QgsError copyErrors;

  for ( const Candidate &candidate : candidates )
  {
    const QString targetName = uniqueProfileName( candidate.profileName, targetRootProfileFolder, reservedNames );
    reservedNames << targetName;

    const QString stagedPath = QDir( stageRootPath ).filePath( targetName );
    const QString finalPath = targetRoot.filePath( targetName );
    if ( !QgsFileUtils::copyDirectory( candidate.profilePath, stagedPath, QgsFileUtils::CopyFlag::NoSymLinks, copyExcludePatterns() ) )
    {
      copyErrors.append( QObject::tr( "Could not fully copy QGIS profile %1." ).arg( candidate.profilePath ), IMPORT_TAG );
      continue;
    }

    if ( !postProcessProfile( candidate, stagedPath, finalPath, &copyErrors ) )
      continue;

    pendingImports.append( { candidate, targetName, stagedPath, finalPath } );
  }

  if ( !copyErrors.isEmpty() || pendingImports.isEmpty() )
  {
    appendErrors( result.errors, copyErrors );
    if ( pendingImports.isEmpty() )
      result.errors.append( QObject::tr( "No QGIS profiles were imported." ), IMPORT_TAG );
    QDir( stageRootPath ).removeRecursively();
    return result;
  }

  QStringList movedProfilePaths;
  for ( const PendingImport &pendingImport : std::as_const( pendingImports ) )
  {
    if ( !QDir().rename( pendingImport.stagedPath, pendingImport.finalPath ) )
    {
      result.errors.append( QObject::tr( "Cannot move imported profile into place: %1" ).arg( pendingImport.finalPath ), IMPORT_TAG );
      for ( const QString &path : std::as_const( movedProfilePaths ) )
        QDir( path ).removeRecursively();
      QDir( stageRootPath ).removeRecursively();
      return result;
    }

    movedProfilePaths << pendingImport.finalPath;
    result.importedProfileNames << pendingImport.targetName;
  }

  QDir( stageRootPath ).removeRecursively();

  const QString preferredSourceProfile = activeProfileHint.isEmpty() ? preferredActiveProfileName( candidates ) : activeProfileHint;
  for ( const PendingImport &pendingImport : std::as_const( pendingImports ) )
  {
    if ( pendingImport.candidate.profileName == preferredSourceProfile )
    {
      result.activeProfileName = pendingImport.targetName;
      break;
    }
  }
  if ( result.activeProfileName.isEmpty() )
    result.activeProfileName = result.importedProfileNames.value( 0 );

  QSettings targetSettings( profilesIniPath( targetRootProfileFolder ), QSettings::IniFormat );
  const QString sourceRoot = candidates.constFirst().sourceRoot;
  const QString sourcePolicy = sourceSelectionPolicy( sourceRoot );
  if ( !sourcePolicy.isEmpty() )
    targetSettings.setValue( u"/core/selectionPolicy"_s, sourcePolicy.toInt() );

  QString defaultTargetProfile = result.activeProfileName;
  const QString defaultSourceProfile = sourceProfilesSetting( sourceRoot, u"/core/defaultProfile"_s );
  for ( const PendingImport &pendingImport : std::as_const( pendingImports ) )
  {
    if ( canonicalOrAbsolutePath( pendingImport.candidate.sourceRoot ) == canonicalOrAbsolutePath( sourceRoot ) && pendingImport.candidate.profileName == defaultSourceProfile )
    {
      defaultTargetProfile = pendingImport.targetName;
      break;
    }
  }

  targetSettings.setValue( u"/core/defaultProfile"_s, defaultTargetProfile );
  targetSettings.setValue( u"/core/lastProfile"_s, result.activeProfileName );
  targetSettings.setValue( u"strata/qgisImport/decisionMade"_s, true );
  targetSettings.setValue( u"strata/qgisImport/importedAt"_s, QDateTime::currentDateTimeUtc().toString( Qt::ISODate ) );
  targetSettings.sync();

  return result;
}

QgsQgisProfileImporter::ImportResult QgsQgisProfileImporter::importProfileAsNewProfile( const Candidate &candidate, const QString &targetRootProfileFolder, const QString &targetProfileName )
{
  Candidate renamedCandidate = candidate;
  const QString requestedName = targetProfileName.isEmpty() ? candidate.profileName : targetProfileName;
  const QString uniqueName = uniqueProfileName( requestedName, targetRootProfileFolder );
  renamedCandidate.profileName = uniqueName;

  ImportResult result = importProfiles( { renamedCandidate }, targetRootProfileFolder, uniqueName );
  return result;
}

QString QgsQgisProfileImporter::preferredActiveProfileName( const QList<Candidate> &candidates )
{
  if ( candidates.isEmpty() )
    return QString();

  const QString sourceRoot = candidates.constFirst().sourceRoot;
  const QString lastProfile = sourceProfilesSetting( sourceRoot, u"/core/lastProfile"_s );
  if ( containsCandidateName( candidates, lastProfile, sourceRoot ) )
    return lastProfile;

  const QString defaultProfile = sourceProfilesSetting( sourceRoot, u"/core/defaultProfile"_s );
  if ( containsCandidateName( candidates, defaultProfile, sourceRoot ) )
    return defaultProfile;

  if ( containsCandidateName( candidates, u"default"_s, sourceRoot ) )
    return u"default"_s;

  return candidates.constFirst().profileName;
}

QString QgsQgisProfileImporter::uniqueProfileName( const QString &requestedName, const QString &targetRootProfileFolder, const QStringList &reservedNames )
{
  const QDir targetRoot( targetRootProfileFolder );
  QSet<QString> reserved;
  for ( const QString &reservedName : reservedNames )
    reserved.insert( reservedName );
  const QString baseName = cleanProfileName( requestedName );

  auto isAvailable = [&]( const QString &name ) { return !reserved.contains( name ) && !QFileInfo::exists( targetRoot.filePath( name ) ); };

  if ( isAvailable( baseName ) )
    return baseName;

  const QString importBaseName = baseName + u"-qgis"_s;
  if ( isAvailable( importBaseName ) )
    return importBaseName;

  int suffix = 2;
  while ( true )
  {
    const QString candidateName = u"%1-%2"_s.arg( importBaseName ).arg( suffix );
    if ( isAvailable( candidateName ) )
      return candidateName;
    ++suffix;
  }
}

void QgsQgisProfileImporter::markFirstRunImportDeclined( const QString &targetRootProfileFolder )
{
  QDir().mkpath( targetRootProfileFolder );
  QSettings settings( profilesIniPath( targetRootProfileFolder ), QSettings::IniFormat );
  settings.setValue( u"strata/qgisImport/decisionMade"_s, true );
  settings.setValue( u"strata/qgisImport/declinedAt"_s, QDateTime::currentDateTimeUtc().toString( Qt::ISODate ) );
  settings.sync();
}
