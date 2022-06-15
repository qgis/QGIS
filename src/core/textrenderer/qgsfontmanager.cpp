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
#include "qgsreadwritelocker.h"
#include "qgsapplication.h"
#include "qgsnetworkcontentfetchertask.h"
#include "qgsziputils.h"

#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QDir>
#include <QFontDatabase>
#include <QTemporaryFile>
#include <QTemporaryDir>

QgsFontManager::QgsFontManager( QObject *parent )
  : QObject( parent )
{
  const QStringList replacements = settingsFontFamilyReplacements.value();
  for ( const QString &replacement : replacements )
  {
    const thread_local QRegularExpression rxReplacement( QStringLiteral( "(.*?):(.*)" ) );
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
    replacements << QStringLiteral( "%1:%2" ).arg( it.key(), it.value() );
  settingsFontFamilyReplacements.setValue( replacements );
}

void QgsFontManager::installUserFonts()
{
  const QString userProfileFontsDir = QgsApplication::qgisSettingsDirPath() + "fonts";
  QStringList fontDirs { userProfileFontsDir };

  if ( !mUserFontDirectory.isEmpty() )
    fontDirs << mUserFontDirectory;

  for ( const QString &dir : std::as_const( fontDirs ) )
  {
    const QDir localDir;
    if ( !localDir.mkpath( dir ) )
    {
      QgsDebugMsg( QStringLiteral( "Cannot create local fonts dir: %1" ).arg( dir ) );
      return;
    }

    const QFileInfoList fileInfoList = QDir( dir ).entryInfoList( QStringList( QStringLiteral( "*.*" ) ), QDir::Files );
    QFileInfoList::const_iterator infoIt = fileInfoList.constBegin();
    for ( ; infoIt != fileInfoList.constEnd(); ++infoIt )
    {
      if ( QFontDatabase::addApplicationFont( infoIt->filePath() ) == -1 )
      {
        QgsDebugMsg( QStringLiteral( "The user font %1 could not be installed" ).arg( infoIt->filePath() ) );
      }
    }
  }
}

void QgsFontManager::downloadAndInstallFont( const QUrl &url, const QString &identifier )
{
  QString description;
  if ( identifier.isEmpty() )
  {
    description = tr( "Installing fonts from %1" ).arg( url.toString() );
  }
  else
  {
    description = tr( "Installing %1" ).arg( identifier );
  }

  QgsNetworkContentFetcherTask *task = new QgsNetworkContentFetcherTask( url, QString(), QgsTask::CanCancel, description );
  connect( task, &QgsNetworkContentFetcherTask::fetched, this, [ = ]
  {
    if ( task->reply()->error() != QNetworkReply::NoError )
    {
      emit fontDownloadErrorOccurred( url, identifier, task->reply()->errorString() );
    }
    else
    {
      QString errorMessage;
      QStringList families;
      QString licenseDetails;
      if ( installFontsFromData( task->reply()->readAll(), errorMessage, families, licenseDetails ) )
      {
        emit fontDownloaded( families, licenseDetails );
      }
      else
      {
        emit fontDownloadErrorOccurred( url, identifier, errorMessage );
      }
    }
  } );
  QgsApplication::taskManager()->addTask( task );
}

bool QgsFontManager::installFontsFromData( const QByteArray &data, QString &errorMessage, QStringList &families, QString &licenseDetails )
{
  errorMessage.clear();
  families.clear();
  licenseDetails.clear();

  QTemporaryFile tempFile;
  QTemporaryDir tempDir;
  const QString userFontsDir = mUserFontDirectory.isEmpty() ? ( QgsApplication::qgisSettingsDirPath() + "fonts" ) : mUserFontDirectory;
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
  int id = QFontDatabase::addApplicationFont( sourcePath );
  if ( id != -1 )
  {
    // succesfully loaded data as a font
    const QStringList foundFamilies = QFontDatabase::applicationFontFamilies( id );
    // remove the application font, as we'll copy it to the final destination and re-add from there
    QFontDatabase::removeApplicationFont( id );

    if ( foundFamilies.empty() )
    {
      errorMessage = tr( "Could not find any families in font" );
      return false;
    }

    QgsDebugMsgLevel( QStringLiteral( "Found fonts %1" ).arg( foundFamilies.join( ',' ) ), 2 );
    families = foundFamilies;
    // guess a good name for the file, by taking the first family name from the font
    const QString family = families.at( 0 );
    const QString destPath = fontsDir.filePath( family );
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

    return true;
  }
  else
  {
    // font install failed, but maybe it's a zip file
    QStringList files;
    if ( QgsZipUtils::unzip( tempFile.fileName(), tempDir.path(), files ) )
    {
      for ( const QString &file : std::as_const( files ) )
      {
        const QFileInfo fi( file );
        if ( fi.fileName().compare( QLatin1String( "OFL.txt" ), Qt::CaseInsensitive ) == 0
             || fi.fileName().compare( QLatin1String( "LICENSE.txt" ), Qt::CaseInsensitive ) == 0 )
        {
          QFile licenseFile( file );
          if ( licenseFile.open( QIODevice::ReadOnly ) )
          {
            QTextStream in( &licenseFile );
            const QString license = in.readAll();
            licenseDetails.append( license );
          }
        }
        else if ( fi.suffix().compare( QLatin1String( "ttf" ), Qt::CaseInsensitive ) == 0 ||
                  fi.suffix().compare( QLatin1String( "otf" ), Qt::CaseInsensitive ) == 0 )
        {
          sourcePath = file;
          id = QFontDatabase::addApplicationFont( sourcePath );
          if ( id != -1 )
          {
            QFontDatabase::removeApplicationFont( id );
            const QString destPath = fontsDir.filePath( fi.fileName() );
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
            const QStringList foundFamilies = QFontDatabase::applicationFontFamilies( id );
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

void QgsFontManager::setUserFontDirectory( const QString &directory )
{
  mUserFontDirectory = directory;
  installUserFonts();
}
