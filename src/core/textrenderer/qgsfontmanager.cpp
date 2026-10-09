/***************************************************************************
    qgsfontmanager.cpp
    ------------------
    Date                 : June 2022
    Copyright            : (C) 2022 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsfontmanager.h"

#include <nlohmann/json.hpp>

#include "qgsapplication.h"
#include "qgsblockingnetworkrequest.h"
#include "qgsfileutils.h"
#include "qgsreadwritelocker.h"
#include "qgssetrequestinitiator_p.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingstree.h"
#include "qgsziputils.h"

#include <QDir>
#include <QFontDatabase>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QString>
#include <QTemporaryDir>
#include <QTemporaryFile>

#include "moc_qgsfontmanager.cpp"

using namespace Qt::StringLiterals;

const QgsSettingsEntryStringList *QgsFontManager::settingsFontFamilyReplacements
  = new QgsSettingsEntryStringList( u"fontFamilyReplacements"_s, QgsSettingsTree::sTreeFonts, QStringList(), u"Automatic font family replacements"_s );

const QgsSettingsEntryBool *QgsFontManager::settingsDownloadMissingFonts
  = new QgsSettingsEntryBool( u"downloadMissingFonts"_s, QgsSettingsTree::sTreeFonts, true, u"Automatically download missing fonts whenever possible"_s );

//
// QgsFontDownloadDetails
//

QgsFontDownloadDetails::QgsFontDownloadDetails() = default;

QgsFontDownloadDetails::QgsFontDownloadDetails( const QString &family, const QStringList &fontUrls, const QString &licenseUrl )
  : mFamily( family )
  , mStandardizedFamily( standardizeFamily( family ) )
  , mFontUrls( fontUrls )
  , mLicenseUrl( licenseUrl )
{}

QString QgsFontDownloadDetails::standardizeFamily( const QString &family )
{
  const thread_local QRegularExpression charsToRemove( u"[^a-z]"_s );
  const thread_local QRegularExpression styleNames( u"(?:normal|regular|light|bold|black|demi|italic|oblique|medium|thin)"_s );

  QString processed = family.toLower();
  processed.replace( styleNames, QString() );
  return processed.replace( charsToRemove, QString() );
}

//
// QgsFontManager
//

QgsFontManager::QgsFontManager( QObject *parent )
  : QObject( parent )
{
  const QStringList replacements = settingsFontFamilyReplacements->value();
  for ( const QString &replacement : replacements )
  {
    const thread_local QRegularExpression rxReplacement( u"(.*?):(.*)"_s );
    const QRegularExpressionMatch match = rxReplacement.match( replacement );
    if ( match.hasMatch() )
    {
      mFamilyReplacements.insert( match.captured( 1 ), match.captured( 2 ) );
      mLowerCaseFamilyReplacements.insert( match.captured( 1 ).toLower(), match.captured( 2 ) );
    }
  }
}

QMap<QString, QString> QgsFontManager::fontFamilyReplacements() const
{
  QgsReadWriteLocker locker( mReplacementLock, QgsReadWriteLocker::Read );
  return mFamilyReplacements;
}

void QgsFontManager::addFontFamilyReplacement( const QString &original, const QString &replacement )
{
  QgsReadWriteLocker locker( mReplacementLock, QgsReadWriteLocker::Write );
  if ( !replacement.isEmpty() )
  {
    mFamilyReplacements.insert( original, replacement );
    mLowerCaseFamilyReplacements.insert( original.toLower(), replacement );
  }
  else
  {
    mFamilyReplacements.remove( original );
    mLowerCaseFamilyReplacements.remove( original.toLower() );
  }
  storeFamilyReplacements();
}

void QgsFontManager::setFontFamilyReplacements( const QMap<QString, QString> &replacements )
{
  QgsReadWriteLocker locker( mReplacementLock, QgsReadWriteLocker::Write );
  mFamilyReplacements = replacements;
  mLowerCaseFamilyReplacements.clear();
  for ( auto it = mFamilyReplacements.constBegin(); it != mFamilyReplacements.constEnd(); ++it )
    mLowerCaseFamilyReplacements.insert( it.key().toLower(), it.value() );

  storeFamilyReplacements();
}

QString QgsFontManager::processFontFamilyName( const QString &name ) const
{
  QgsReadWriteLocker locker( mReplacementLock, QgsReadWriteLocker::Read );
  auto it = mLowerCaseFamilyReplacements.constFind( name.toLower() );
  if ( it != mLowerCaseFamilyReplacements.constEnd() )
    return it.value();
  else
    return name;
}

void QgsFontManager::storeFamilyReplacements()
{
  QStringList replacements;
  for ( auto it = mFamilyReplacements.constBegin(); it != mFamilyReplacements.constEnd(); ++it )
    replacements << u"%1:%2"_s.arg( it.key(), it.value() );
  settingsFontFamilyReplacements->setValue( replacements );
}

void QgsFontManager::installUserFonts()
{
  QgsReadWriteLocker locker( mReplacementLock, QgsReadWriteLocker::Write );
  const QString userProfileFontsDir = QgsApplication::qgisSettingsDirPath() + "fonts";
  QStringList fontDirs { userProfileFontsDir };

  fontDirs.append( mUserFontDirectories );

  for ( const QString &dir : std::as_const( fontDirs ) )
  {
    if ( !QFile::exists( dir ) && !QDir().mkpath( dir ) )
    {
      QgsDebugError( u"Cannot create local fonts dir: %1"_s.arg( dir ) );
      return;
    }

    installFontsFromDirectory( dir );
  }
}

void QgsFontManager::installFontsFromDirectory( const QString &dir )
{
  const QFileInfoList fileInfoList = QDir( dir ).entryInfoList( QStringList( u"*"_s ), QDir::Files );
  QFileInfoList::const_iterator infoIt = fileInfoList.constBegin();
  for ( ; infoIt != fileInfoList.constEnd(); ++infoIt )
  {
    const int id = QFontDatabase::addApplicationFont( infoIt->filePath() );
    if ( id == -1 )
    {
      QgsDebugError( u"The user font %1 could not be installed"_s.arg( infoIt->filePath() ) );
      mUserFontToFamilyMap.remove( infoIt->filePath() );
      mUserFontToIdMap.remove( infoIt->filePath() );
    }
    else
    {
      mUserFontToFamilyMap.insert( infoIt->filePath(), QFontDatabase::applicationFontFamilies( id ) );
      mUserFontToIdMap.insert( infoIt->filePath(), id );
    }
  }
}

bool QgsFontManager::tryToDownloadFontFamily( const QString &family, QString &matchedFamily )
{
  matchedFamily.clear();
  if ( !settingsDownloadMissingFonts->value() )
    return false;

  QgsReadWriteLocker locker( mReplacementLock, QgsReadWriteLocker::Read );
  auto it = mPendingFontDownloads.constFind( family );
  if ( it != mPendingFontDownloads.constEnd() )
  {
    matchedFamily = it.value();
    return true;
  }
  locker.unlock();

  const QgsFontDownloadDetails details = detailsForFontDownload( family, matchedFamily );
  if ( !details.isValid() )
    return false;

  // It's possible that the font family laundering applied in urlForFontDownload has cleaned up the font
  // family to a valid font which already exists on the system. In this case we shouldn't try to download
  // the font again.
  const QFont testFont( matchedFamily );
  if ( testFont.exactMatch() )
    return true;

  locker.changeMode( QgsReadWriteLocker::Write );
  mPendingFontDownloads.insert( family, matchedFamily );
  if ( !mEnableFontDownloads )
  {
    mDeferredFontDownloads.insert( matchedFamily, details );
  }
  else
  {
    locker.unlock();
    downloadAndInstallFont( details, family );
  }
  return true;
}

void QgsFontManager::enableFontDownloadsForSession()
{
  if ( mEnableFontDownloads )
    return;

  mEnableFontDownloads = true;
  QgsReadWriteLocker locker( mReplacementLock, QgsReadWriteLocker::Read );
  if ( !mDeferredFontDownloads.isEmpty() )
  {
    locker.changeMode( QgsReadWriteLocker::Write );
    for ( auto it = mDeferredFontDownloads.constBegin(); it != mDeferredFontDownloads.constEnd(); ++it )
    {
      downloadAndInstallFont( it.value(), it.key() );
    }
    mDeferredFontDownloads.clear();
  }
}

QgsFontDownloadDetails GoogleFontDetails( const QString &family, const QStringList &downloadPaths, const QString &licensePath = QString() )
{
  QStringList fontUrls;
  fontUrls.reserve( downloadPaths.size() );
  for ( const QString &path : downloadPaths )
  {
    fontUrls.append( u"https://github.com/google/fonts/raw/main/%1"_s.arg( path ) );
  }
  return QgsFontDownloadDetails( family, fontUrls, !licensePath.isEmpty() ? u"https://github.com/google/fonts/raw/main/%1"_s.arg( licensePath ) : QString() );
}

std::vector< QgsFontDownloadDetails > loadGoogleFontsFromJson()
{
  std::vector< QgsFontDownloadDetails > fonts;
  // this json is built using scripts/process_google_fonts.py
  const QString jsonPath = QgsApplication::pkgDataPath() + u"/resources/data/google_fonts.json"_s;

  QFile file( jsonPath );
  if ( !file.open( QIODevice::ReadOnly ) )
  {
    QgsDebugError( u"Failed to open Google fonts JSON file: %1"_s.arg( jsonPath ) );
    return fonts;
  }

  const QByteArray jsonContent = file.readAll();
  try
  {
    const json fontsJson = json::parse( jsonContent.toStdString() );
    if ( fontsJson.is_array() )
    {
      fonts.reserve( fontsJson.size() );
      for ( const json &fontJson : fontsJson )
      {
        const QString family = QString::fromStdString( fontJson["family"].get<std::string>() );
        const QString license = QString::fromStdString( fontJson["license"].get<std::string>() );

        QStringList paths;
        const json &pathsArray = fontJson["paths"];
        if ( !pathsArray.is_array() )
        {
          QgsDebugError( u"Failed to parse Google font %1, expected array for paths."_s.arg( family ) );
          return fonts;
        }
        for ( const json &pathJson : pathsArray )
        {
          paths.append( QString::fromStdString( pathJson.get<std::string>() ) );
        }

        fonts.push_back( GoogleFontDetails( family, paths, license ) );
      }
    }
    else
    {
      QgsDebugError( u"Failed to parse Google fonts JSON, expected array."_s );
      return fonts;
    }
  }
  catch ( nlohmann::json::exception &ex )
  {
    QgsDebugError( u"Failed to parse Google fonts JSON: %1"_s.arg( ex.what() ) );
    return fonts;
  }

  return fonts;
}

QgsFontDownloadDetails QgsFontManager::detailsForFontDownload( const QString &family, QString &matchedFamily ) const
{
  static const std::vector< QgsFontDownloadDetails > sGoogleFonts = loadGoogleFontsFromJson();

  matchedFamily.clear();
  const QString cleanedFamily = QgsFontDownloadDetails::standardizeFamily( family );

  for ( const QgsFontDownloadDetails &candidate : sGoogleFonts )
  {
    if ( candidate.standardizedFamily() == cleanedFamily )
    {
      matchedFamily = candidate.family();
      return candidate;
    }
  }

  return QgsFontDownloadDetails();
}

QString QgsFontManager::urlForFontDownload( const QString &family, QString &matchedFamily ) const
{
  const QgsFontDownloadDetails details = detailsForFontDownload( family, matchedFamily );
  return details.isValid() ? details.fontUrls().value( 0 ) : QString();
};

void QgsFontManager::downloadAndInstallFont( const QgsFontDownloadDetails &details, const QString &identifier )
{
  if ( !details.isValid() )
    return;

  QString description;
  if ( identifier.isEmpty() )
  {
    description = tr( "Installing %1" ).arg( details.family() );
  }
  else
  {
    description = tr( "Installing %1" ).arg( identifier );
  }

  QgsFontDownloadTask *task = new QgsFontDownloadTask( description, details );
  connect( task, &QgsFontDownloadTask::taskTerminated, this, [this, task, identifier] {
    QgsReadWriteLocker locker( mReplacementLock, QgsReadWriteLocker::Write );
    mPendingFontDownloads.remove( identifier );
    locker.unlock();

    emit fontDownloadErrorOccurred( QUrl( task->failedUrl() ), identifier, task->errorMessage() );
  } );

  connect( task, &QgsFontDownloadTask::taskCompleted, this, [this, task, details, identifier] {
    const QList<QByteArray > allFontData = task->fontData();
    QStringList allFamilies;
    QStringList allLicenseDetails;

    QString errorMessage;
    for ( int i = 0; i < allFontData.size(); ++i )
    {
      QStringList thisUrlFamilies;
      const QByteArray fontData = allFontData[i];
      const QString contentDispositionFilename = task->contentDispositionFilenames().at( i );
      QString extension;
      if ( contentDispositionFilename.isEmpty() )
      {
        const QUrl originalUrl = details.fontUrls().value( i );
        const thread_local QRegularExpression rxExtension( u"^.*\\.(\\w+?)$"_s );
        extension = rxExtension.match( originalUrl.toString() ).captured( 1 );
      }
      QString thisLicenseDetails;
      if ( !installFontsFromData( fontData, errorMessage, thisUrlFamilies, thisLicenseDetails, contentDispositionFilename, extension ) )
      {
        QgsReadWriteLocker locker( mReplacementLock, QgsReadWriteLocker::Write );
        mPendingFontDownloads.remove( identifier );
        locker.unlock();

        emit fontDownloadErrorOccurred( details.fontUrls().value( i ), identifier, errorMessage );
        return;
      }
      else
      {
        for ( const QString &family : std::as_const( thisUrlFamilies ) )
        {
          if ( !allFamilies.contains( family ) )
            allFamilies.append( family );
        }
        if ( !thisLicenseDetails.isEmpty() && !allLicenseDetails.contains( thisLicenseDetails ) )
        {
          allLicenseDetails.append( thisLicenseDetails );
        }
      }
    }

    if ( !task->licenseData().isEmpty() && !allLicenseDetails.contains( task->licenseData() ) )
    {
      allLicenseDetails.append( task->licenseData() );
    }

    QgsReadWriteLocker locker( mReplacementLock, QgsReadWriteLocker::Write );
    mPendingFontDownloads.remove( identifier );
    locker.unlock();

    emit fontDownloaded( allFamilies, allLicenseDetails.isEmpty() ? QString() : allLicenseDetails.join( "\n\n" ) );
  } );

  QgsApplication::taskManager()->addTask( task );
}

void QgsFontManager::downloadAndInstallFont( const QUrl &url, const QString &identifier )
{
  downloadAndInstallFont( QgsFontDownloadDetails( identifier, { url.toString() } ) );
}

bool QgsFontManager::installFontsFromData( const QByteArray &data, QString &errorMessage, QStringList &families, QString &licenseDetails, const QString &filename, const QString &extension )
{
  errorMessage.clear();
  families.clear();
  licenseDetails.clear();

  QTemporaryFile tempFile;
  if ( !extension.isEmpty() )
  {
    QString cleanedExtension = extension;
    if ( cleanedExtension.startsWith( '.' ) )
      cleanedExtension = cleanedExtension.mid( 1 );
    tempFile.setFileTemplate( u"%1/XXXXXX.%2"_s.arg( QDir::tempPath(), cleanedExtension ) );
  }
  QTemporaryDir tempDir;

  QgsReadWriteLocker locker( mReplacementLock, QgsReadWriteLocker::Read );
  const QString userFontsDir = mUserFontDirectories.empty() ? ( QgsApplication::qgisSettingsDirPath() + "fonts" ) : mUserFontDirectories.at( 0 );
  locker.unlock();

  const QDir fontsDir( userFontsDir );

  if ( !tempFile.open() )
  {
    errorMessage = tr( "Could not write font data to a temporary file" );
    return false;
  }

  tempFile.write( data );
  tempFile.close();

  QString sourcePath = tempFile.fileName();

  //try to install the data directly as a font
  int id = QFontDatabase::addApplicationFontFromData( data );
  if ( id != -1 )
  {
    // successfully loaded data as a font
    const QStringList foundFamilies = QFontDatabase::applicationFontFamilies( id );
    // remove the application font, as we'll copy it to the final destination and re-add from there
    QFontDatabase::removeApplicationFont( id );

    if ( foundFamilies.empty() )
    {
      errorMessage = tr( "Could not find any families in font" );
      return false;
    }

    QgsDebugMsgLevel( u"Found fonts %1"_s.arg( foundFamilies.join( ',' ) ), 2 );
    families = foundFamilies;
    // guess a good name for the file, by taking the first family name from the font
    const QString family = families.at( 0 );
    const QString destPath = QgsFileUtils::uniquePath( fontsDir.filePath( filename.isEmpty() ? family : filename ) );

    if ( !QFile::copy( sourcePath, destPath ) )
    {
      errorMessage = tr( "Could not copy font to %1" ).arg( destPath );
      return false;
    }

    id = QFontDatabase::addApplicationFont( destPath );
    if ( id == -1 )
    {
      errorMessage = tr( "Could not install font from %1" ).arg( destPath );
      return false;
    }
    else
    {
      locker.changeMode( QgsReadWriteLocker::Write );
      mUserFontToFamilyMap.insert( destPath, foundFamilies );
      mUserFontToIdMap.insert( destPath, id );
    }
    return true;
  }
  else
  {
    // font install failed, but maybe it's a zip file
    QStringList files;
    if ( QgsZipUtils::unzip( tempFile.fileName(), tempDir.path(), files ) )
    {
      locker.changeMode( QgsReadWriteLocker::Write );
      for ( const QString &file : std::as_const( files ) )
      {
        const QFileInfo fi( file );
        if ( fi.fileName().compare( "OFL.txt"_L1, Qt::CaseInsensitive ) == 0 || fi.fileName().compare( "LICENSE.txt"_L1, Qt::CaseInsensitive ) == 0 )
        {
          QFile licenseFile( file );
          if ( licenseFile.open( QIODevice::ReadOnly ) )
          {
            QTextStream in( &licenseFile );
            const QString license = in.readAll();
            licenseDetails.append( license );
          }
        }
        else if ( fi.suffix().compare( "ttf"_L1, Qt::CaseInsensitive ) == 0 || fi.suffix().compare( "otf"_L1, Qt::CaseInsensitive ) == 0 )
        {
          sourcePath = file;
          id = QFontDatabase::addApplicationFont( sourcePath );
          if ( id != -1 )
          {
            QFontDatabase::removeApplicationFont( id );
            const QString destPath = fontsDir.filePath( fi.fileName() );
            // dest path may already exist for zip files -- e.g if a single zip contains a number of font variants
            if ( !QFile::exists( destPath ) && !QFile::copy( sourcePath, destPath ) )
            {
              errorMessage = tr( "Could not copy font to %1" ).arg( destPath );
              return false;
            }
            id = QFontDatabase::addApplicationFont( destPath );
            if ( id == -1 )
            {
              errorMessage = tr( "Could not install font from %1" ).arg( destPath );
              return false;
            }
            const QStringList foundFamilies = QFontDatabase::applicationFontFamilies( id );
            mUserFontToFamilyMap.insert( destPath, foundFamilies );
            mUserFontToIdMap.insert( destPath, id );
            for ( const QString &found : foundFamilies )
            {
              if ( !families.contains( found ) )
                families << found;
            }
          }
        }
      }
      return true;
    }
  }

  errorMessage = tr( "Could not read fonts from data" );
  return false;
}

void QgsFontManager::addUserFontDirectory( const QString &directory )
{
  QgsReadWriteLocker locker( mReplacementLock, QgsReadWriteLocker::Read );
  if ( mUserFontDirectories.contains( directory ) )
    return;

  locker.changeMode( QgsReadWriteLocker::Write );
  mUserFontDirectories.append( directory );
  locker.unlock();

  if ( !QFile::exists( directory ) && !QDir().mkpath( directory ) )
  {
    QgsDebugError( u"Cannot create local fonts dir: %1"_s.arg( directory ) );
    return;
  }

  installFontsFromDirectory( directory );
}

QMap<QString, QStringList> QgsFontManager::userFontToFamilyMap() const
{
  QgsReadWriteLocker locker( mReplacementLock, QgsReadWriteLocker::Read );
  return mUserFontToFamilyMap;
}

bool QgsFontManager::removeUserFont( const QString &path )
{
  QgsReadWriteLocker locker( mReplacementLock, QgsReadWriteLocker::Write );
  const int id = mUserFontToIdMap.value( path, -1 );
  if ( id != -1 )
    QFontDatabase::removeApplicationFont( id );
  QFile::remove( path );
  mUserFontToIdMap.remove( path );
  mUserFontToFamilyMap.remove( path );
  return true;
}

/// @cond PRIVATE
//
// QgsFontDownloadTask
//

QgsFontDownloadTask::QgsFontDownloadTask( const QString &description, const QgsFontDownloadDetails &details )
  : QgsTask( description, QgsTask::CanCancel )
  , mDetails( details )
{}

bool QgsFontDownloadTask::run()
{
  mFeedback = std::make_unique< QgsFeedback >();
  mResult = true;

  for ( const QString &url : mDetails.fontUrls() )
  {
    // TODO: We should really do this async, but I'm trying to minimize the impact of this change for backport friendliness
    QgsBlockingNetworkRequest req;
    QNetworkRequest networkRequest( url );
    QgsSetRequestInitiatorClass( networkRequest, u"QgsFontDownloadTask"_s );
    switch ( req.get( networkRequest, false, mFeedback.get() ) )
    {
      case QgsBlockingNetworkRequest::NoError:
        mFontData.append( req.reply().content() );
        mContentDispositionFilenames.append( QgsNetworkReplyContent::extractFileNameFromContentDispositionHeader( req.reply().rawHeader( "Content-Disposition" ) ) );
        break;

      case QgsBlockingNetworkRequest::NetworkError:
      case QgsBlockingNetworkRequest::TimeoutError:
      case QgsBlockingNetworkRequest::ServerExceptionError:
        mResult = false;
        mErrorMessage = req.errorMessage();
        mFailedUrl = url;
        break;
    }

    if ( !mResult )
      break;
  }

  if ( mResult && !mDetails.licenseUrl().isEmpty() )
  {
    QgsBlockingNetworkRequest req;
    QNetworkRequest networkRequest( mDetails.licenseUrl() );
    QgsSetRequestInitiatorClass( networkRequest, u"QgsFontDownloadTask"_s );
    switch ( req.get( networkRequest, false, mFeedback.get() ) )
    {
      case QgsBlockingNetworkRequest::NoError:
        mLicenseData = req.reply().content();
        break;

      case QgsBlockingNetworkRequest::NetworkError:
      case QgsBlockingNetworkRequest::TimeoutError:
      case QgsBlockingNetworkRequest::ServerExceptionError:
        mResult = false;
        mErrorMessage = req.errorMessage();
        mFailedUrl = mDetails.licenseUrl();
        break;
    }
  }

  return mResult;
}

void QgsFontDownloadTask::cancel()
{
  if ( mFeedback )
    mFeedback->cancel();
  QgsTask::cancel();
}

///@endcond PRIVATE
