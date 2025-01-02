/***************************************************************************
    qgsfileutils.cpp
    ---------------------
    begin                : November 2017
    copyright            : (C) 2017 by Etienne Trimaille
    email                : etienne.trimaille at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsfileutils.h"
#include "qgis.h"
#include "qgsexception.h"
#include "qgsconfig.h"
#include "qgsproviderregistry.h"
#include "qgsprovidermetadata.h"

#include <QObject>
#include <QRegularExpression>
#include <QFileInfo>
#include <QDir>
#include <QSet>
#include <QDirIterator>

#ifdef Q_OS_UNIX
// For getrlimit()
#include <sys/resource.h>
#include <sys/time.h>
#include <dirent.h>
#endif

#ifdef _MSC_VER
#include <Windows.h>
#include <ShlObj.h>
#pragma comment(lib,"Shell32.lib")
#endif

QString QgsFileUtils::representFileSize( qint64 bytes )
{
  QStringList list;
  list << QObject::tr( "KB" ) << QObject::tr( "MB" ) << QObject::tr( "GB" ) << QObject::tr( "TB" );

  QStringListIterator i( list );
  QString unit = QObject::tr( "B" );

  double fileSize = bytes;
  while ( fileSize >= 1024.0 && i.hasNext() )
  {
    fileSize /= 1024.0;
    unit = i.next();
  }
  return QStringLiteral( "%1 %2" ).arg( QString::number( fileSize, 'f', bytes >= 1048576 ? 2 : 0 ), unit );
}

QStringList QgsFileUtils::extensionsFromFilter( const QString &filter )
{
  const thread_local QRegularExpression rx( QStringLiteral( "\\*\\.([a-zA-Z0-9\\.]+)" ) );
  QStringList extensions;
  QRegularExpressionMatchIterator matches = rx.globalMatch( filter );

  while ( matches.hasNext() )
  {
    const QRegularExpressionMatch match = matches.next();
    if ( match.hasMatch() )
    {
      QStringList newExtensions = match.capturedTexts();
      newExtensions.pop_front(); // remove whole match
      extensions.append( newExtensions );
    }
  }
  return extensions;
}

QString QgsFileUtils::wildcardsFromFilter( const QString &filter )
{
  const thread_local QRegularExpression globPatternsRx( QStringLiteral( ".*\\((.*?)\\)$" ) );
  const QRegularExpressionMatch matches = globPatternsRx.match( filter );
  if ( matches.hasMatch() )
    return matches.captured( 1 );
  else
    return QString();
}

bool QgsFileUtils::fileMatchesFilter( const QString &fileName, const QString &filter )
{
  QFileInfo fi( fileName );
  const QString name = fi.fileName();
  const QStringList parts = filter.split( QStringLiteral( ";;" ) );
  for ( const QString &part : parts )
  {
    const QStringList globPatterns = wildcardsFromFilter( part ).split( ' ', Qt::SkipEmptyParts );
    for ( const QString &glob : globPatterns )
    {
      const QString re = QRegularExpression::wildcardToRegularExpression( glob );

      const QRegularExpression globRx( re );
      if ( globRx.match( name ).hasMatch() )
        return true;
    }
  }
  return false;
}

QString QgsFileUtils::ensureFileNameHasExtension( const QString &f, const QStringList &extensions )
{
  if ( extensions.empty() || f.isEmpty() )
    return f;

  QString fileName = f;
  bool hasExt = false;
  for ( const QString &extension : std::as_const( extensions ) )
  {
    const QString extWithDot = extension.startsWith( '.' ) ? extension : '.' + extension;
    if ( fileName.endsWith( extWithDot, Qt::CaseInsensitive ) )
    {
      hasExt = true;
      break;
    }
  }

  if ( !hasExt )
  {
    const QString extension = extensions.at( 0 );
    const QString extWithDot = extension.startsWith( '.' ) ? extension : '.' + extension;
    fileName += extWithDot;
  }

  return fileName;
}

QString QgsFileUtils::addExtensionFromFilter( const QString &fileName, const QString &filter )
{
  const QStringList extensions = extensionsFromFilter( filter );
  return ensureFileNameHasExtension( fileName, extensions );
}

QString QgsFileUtils::stringToSafeFilename( const QString &string )
{
  const thread_local QRegularExpression rx( QStringLiteral( "[/\\\\\\?%\\*\\:\\|\"<>]" ) );
  QString s = string;
  s.replace( rx, QStringLiteral( "_" ) );
  return s;
}

QString QgsFileUtils::findClosestExistingPath( const QString &path )
{
  if ( path.isEmpty() )
    return QString();

  QDir currentPath;
  QFileInfo fi( path );
  if ( fi.isFile() )
    currentPath = fi.dir();
  else
    currentPath = QDir( path );

  QSet< QString > visited;
  while ( !currentPath.exists() )
  {
    const QString parentPath = QDir::cleanPath( currentPath.absolutePath() + QStringLiteral( "/.." ) );
    if ( visited.contains( parentPath ) )
      return QString(); // break circular links

    if ( parentPath.isEmpty() || parentPath == QLatin1String( "." ) )
      return QString();
    currentPath = QDir( parentPath );
    visited << parentPath;
  }

  const QString res = QDir::cleanPath( currentPath.absolutePath() );

  if ( res == QDir::currentPath() )
    return QString(); // avoid default to binary folder if a filename alone is specified

  return res == QLatin1String( "." ) ? QString() : res;
}

QStringList QgsFileUtils::findFile( const QString &file, const QString &basePath, int maxClimbs, int searchCeilling, const QString &currentDir )
{
  int depth = 0;
  QString originalFolder;
  QDir folder;
  const QString fileName( basePath.isEmpty() ? QFileInfo( file ).fileName() : file );
  const QString baseFolder( basePath.isEmpty() ? QFileInfo( file ).path() : basePath );

  if ( QFileInfo( baseFolder ).isDir() )
  {
    folder = QDir( baseFolder ) ;
    originalFolder = folder.absolutePath();
  }
  else // invalid folder or file path
  {
    folder = QDir( QFileInfo( baseFolder ).absolutePath() );
    originalFolder = folder.absolutePath();
  }

  QStringList searchedFolder = QStringList();
  QString existingBase;
  QString backupDirectory = QDir::currentPath();
  QStringList foundFiles;

  if ( !currentDir.isEmpty() && backupDirectory != currentDir && QDir( currentDir ).exists() )
    QDir::setCurrent( currentDir );

  // find the nearest existing folder
  while ( !folder.exists() && folder.absolutePath().count( '/' ) > searchCeilling )
  {

    existingBase = folder.path();
    if ( !folder.cdUp() )
      folder = QFileInfo( existingBase ).absoluteDir(); // using fileinfo to move up one level

    depth += 1;

    if ( depth > ( maxClimbs + 4 ) ) //break early when no folders can be found
      break;
  }
  bool folderExists = folder.exists();

  if ( depth > maxClimbs )
    maxClimbs = depth;

  if ( folder.absolutePath().count( '/' ) < searchCeilling )
    searchCeilling = folder.absolutePath().count( '/' ) - 1;

  while ( depth <= maxClimbs && folderExists && folder.absolutePath().count( '/' ) >= searchCeilling )
  {

    QDirIterator localFinder( folder.path(), QStringList() << fileName, QDir::Files, QDirIterator::NoIteratorFlags );
    searchedFolder.append( folder.absolutePath() );
    if ( localFinder.hasNext() )
    {
      foundFiles << localFinder.next();
      return foundFiles;
    }


    const QFileInfoList subdirs = folder.entryInfoList( QDir::AllDirs );
    for ( const QFileInfo &subdir : subdirs )
    {
      if ( ! searchedFolder.contains( subdir.absolutePath() ) )
      {
        QDirIterator subDirFinder( subdir.path(), QStringList() << fileName, QDir::Files, QDirIterator::Subdirectories );
        if ( subDirFinder.hasNext() )
        {
          QString possibleFile = subDirFinder.next();
          if ( !subDirFinder.hasNext() )
          {
            foundFiles << possibleFile;
            return foundFiles;
          }

          foundFiles << possibleFile;
          while ( subDirFinder.hasNext() )
          {
            foundFiles << subDirFinder.next();
          }
          return foundFiles;
        }
      }
    }
    depth += 1;

    if ( depth > maxClimbs )
      break;

    folderExists = folder.cdUp();
  }

  if ( QDir::currentPath() == currentDir && currentDir != backupDirectory )
    QDir::setCurrent( backupDirectory );

  return foundFiles;
}

#ifdef _MSC_VER
std::unique_ptr< wchar_t[] > pathToWChar( const QString &path )
{
  const QString nativePath = QDir::toNativeSeparators( path );

  std::unique_ptr< wchar_t[] > pathArray( new wchar_t[static_cast< uint>( nativePath.length() + 1 )] );
  nativePath.toWCharArray( pathArray.get() );
  pathArray[static_cast< size_t >( nativePath.length() )] = 0;
  return pathArray;
}


void fileAttributesOld( HANDLE handle, DWORD &fileAttributes, bool &hasFileAttributes )
{
  hasFileAttributes = false;
  BY_HANDLE_FILE_INFORMATION info;
  if ( GetFileInformationByHandle( handle, &info ) )
  {
    hasFileAttributes = true;
    fileAttributes = info.dwFileAttributes;
  }
}

// File attributes for Windows starting from version 8.
void fileAttributesNew( HANDLE handle, DWORD &fileAttributes, bool &hasFileAttributes )
{
  hasFileAttributes = false;
#if WINVER >= 0x0602
  _FILE_BASIC_INFO infoEx;
  if ( GetFileInformationByHandleEx(
         handle,
         FileBasicInfo,
         &infoEx, sizeof( infoEx ) ) )
  {
    hasFileAttributes = true;
    fileAttributes = infoEx.FileAttributes;
  }
  else
  {
    // GetFileInformationByHandleEx() is observed to fail for FAT32, QTBUG-74759
    fileAttributesOld( handle, fileAttributes, hasFileAttributes );
  }
#else
  fileAttributesOld( handle, fileAttributes, hasFileAttributes );
#endif
}

bool pathIsLikelyCloudStorage( QString path )
{
  // For OneDrive detection need the attributes of a file from the path, not the directory itself.
  // So just grab the first file in the path.
  QDirIterator dirIt( path, QDir::Files );
  if ( dirIt.hasNext() )
  {
    path = dirIt.next();
  }

  std::unique_ptr< wchar_t[] > pathArray = pathToWChar( path );
  const HANDLE handle = CreateFileW( pathArray.get(), 0, FILE_SHARE_READ,
                                     nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr );
  if ( handle != INVALID_HANDLE_VALUE )
  {
    bool hasFileAttributes = false;
    DWORD attributes = 0;
    fileAttributesNew( handle, attributes, hasFileAttributes );
    CloseHandle( handle );
    if ( hasFileAttributes )
    {
      /* From the Win32 API documentation:
         *
         * FILE_ATTRIBUTE_RECALL_ON_OPEN:
         * When this attribute is set, it means that the file or directory has no physical representation
         * on the local system; the item is virtual. Opening the item will be more expensive than normal,
         * e.g. it will cause at least some of it to be fetched from a remote store
         *
         * FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS
         * When this attribute is set, it means that the file or directory is not fully present locally.
         * For a file that means that not all of its data is on local storage (e.g. it may be sparse with
         * some data still in remote storage).
         */
      return ( attributes & FILE_ATTRIBUTE_RECALL_ON_OPEN )
             || ( attributes & FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS );
    }
  }
  return false;
}
#endif

Qgis::DriveType QgsFileUtils::driveType( const QString &path )
{
#ifdef _MSC_VER
  auto pathType = [ = ]( const QString & path ) -> Qgis::DriveType
  {
    std::unique_ptr< wchar_t[] > pathArray = pathToWChar( path );
    const UINT type = GetDriveTypeW( pathArray.get() );
    switch ( type )
    {
      case DRIVE_UNKNOWN:
        return Qgis::DriveType::Unknown;

      case DRIVE_NO_ROOT_DIR:
        return Qgis::DriveType::Invalid;

      case DRIVE_REMOVABLE:
        return Qgis::DriveType::Removable;

      case DRIVE_FIXED:
        return Qgis::DriveType::Fixed;

      case DRIVE_REMOTE:
        return Qgis::DriveType::Remote;

      case DRIVE_CDROM:
        return Qgis::DriveType::CdRom;

      case DRIVE_RAMDISK:
        return Qgis::DriveType::RamDisk;
    }

    return Qgis::DriveType::Unknown;

  };

  const QString originalPath = QDir::cleanPath( path );
  QString currentPath = originalPath;
  QString prevPath;
  while ( currentPath != prevPath )
  {
    if ( pathIsLikelyCloudStorage( currentPath ) )
      return Qgis::DriveType::Cloud;

    prevPath = currentPath;
    currentPath = QFileInfo( currentPath ).path();

    const Qgis::DriveType type = pathType( currentPath );
    if ( type != Qgis::DriveType::Unknown && type != Qgis::DriveType::Invalid )
      return type;
  }
  return Qgis::DriveType::Unknown;

#else
  ( void )path;
  throw QgsNotSupportedException( QStringLiteral( "Determining drive type is not supported on this platform" ) );
#endif
}

bool QgsFileUtils::pathIsSlowDevice( const QString &path )
{
#ifdef ENABLE_TESTS
  if ( path.contains( QLatin1String( "fake_slow_path_for_unit_tests" ) ) )
    return true;
#endif

  try
  {
    const Qgis::DriveType type = driveType( path );
    switch ( type )
    {
      case Qgis::DriveType::Unknown:
      case Qgis::DriveType::Invalid:
      case Qgis::DriveType::Fixed:
      case Qgis::DriveType::RamDisk:
        return false;

      case Qgis::DriveType::Removable:
      case Qgis::DriveType::Remote:
      case Qgis::DriveType::CdRom:
      case Qgis::DriveType::Cloud:
        return true;
    }
  }
  catch ( QgsNotSupportedException & )
  {

  }
  return false;
}

QSet<QString> QgsFileUtils::sidecarFilesForPath( const QString &path )
{
  QSet< QString > res;
  const QStringList providers = QgsProviderRegistry::instance()->providerList();
  for ( const QString &provider : providers )
  {
    const QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( provider );
    if ( metadata->providerCapabilities() & QgsProviderMetadata::FileBasedUris )
    {
      const QStringList possibleSidecars = metadata->sidecarFilesForUri( path );
      for ( const QString &possibleSidecar : possibleSidecars )
      {
        if ( QFile::exists( possibleSidecar ) )
          res.insert( possibleSidecar );
      }
    }
  }
  return res;
}

bool QgsFileUtils::renameDataset( const QString &oldPath, const QString &newPath, QString &error, Qgis::FileOperationFlags flags )
{
  if ( !QFile::exists( oldPath ) )
  {
    error = QObject::tr( "File does not exist" );
    return false;
  }

  const QFileInfo oldPathInfo( oldPath );
  QSet< QString > sidecars = sidecarFilesForPath( oldPath );
  if ( flags & Qgis::FileOperationFlag::IncludeMetadataFile )
  {
    const QString qmdPath = oldPathInfo.dir().filePath( oldPathInfo.completeBaseName() + QStringLiteral( ".qmd" ) );
    if ( QFile::exists( qmdPath ) )
      sidecars.insert( qmdPath );
  }
  if ( flags & Qgis::FileOperationFlag::IncludeStyleFile )
  {
    const QString qmlPath = oldPathInfo.dir().filePath( oldPathInfo.completeBaseName() + QStringLiteral( ".qml" ) );
    if ( QFile::exists( qmlPath ) )
      sidecars.insert( qmlPath );
  }

  const QFileInfo newPathInfo( newPath );

  bool res = true;
  QStringList errors;
  errors.reserve( sidecars.size() );
  // first check if all sidecars CAN be renamed -- we don't want to get partly through the rename and then find a clash
  for ( const QString &sidecar : std::as_const( sidecars ) )
  {
    const QFileInfo sidecarInfo( sidecar );
    const QString newSidecarName = newPathInfo.dir().filePath( newPathInfo.completeBaseName() + '.' + sidecarInfo.suffix() );
    if ( newSidecarName != sidecar && QFile::exists( newSidecarName ) )
    {
      res = false;
      errors.append( QDir::toNativeSeparators( newSidecarName ) );
    }
  }
  if ( !res )
  {
    error = QObject::tr( "Destination files already exist %1" ).arg( errors.join( QLatin1String( ", " ) ) );
    return false;
  }

  if ( !QFile::rename( oldPath, newPath ) )
  {
    error = QObject::tr( "Could not rename %1" ).arg( QDir::toNativeSeparators( oldPath ) );
    return false;
  }

  for ( const QString &sidecar : std::as_const( sidecars ) )
  {
    const QFileInfo sidecarInfo( sidecar );
    const QString newSidecarName = newPathInfo.dir().filePath( newPathInfo.completeBaseName() + '.' + sidecarInfo.suffix() );
    if ( newSidecarName == sidecar )
      continue;

    if ( !QFile::rename( sidecar, newSidecarName ) )
    {
      errors.append( QDir::toNativeSeparators( sidecar ) );
      res = false;
    }
  }
  if ( !res )
  {
    error = QObject::tr( "Could not rename %1" ).arg( errors.join( QLatin1String( ", " ) ) );
  }

  return res;
}

int QgsFileUtils::openedFileLimit()
{
#ifdef Q_OS_UNIX
  struct rlimit rescLimit;
  if ( getrlimit( RLIMIT_NOFILE, &rescLimit ) == 0 )
  {
    return rescLimit.rlim_cur;
  }
#endif
  return -1;
}

int QgsFileUtils::openedFileCount()
{
#ifdef Q_OS_LINUX
  int fileCount = 0;

  DIR *dirp = opendir( "/proc/self/fd" );
  if ( !dirp )
    return -1;

  while ( struct dirent *entry = readdir( dirp ) )
  {
    if ( entry->d_type == DT_REG )
    {
      fileCount++;
    }
  }
  closedir( dirp );
  return fileCount;
#else
  return -1;
#endif
}

bool QgsFileUtils::isCloseToLimitOfOpenedFiles( int filesToBeOpened )
{
  const int nFileLimit = QgsFileUtils::openedFileLimit();
  const int nFileCount = QgsFileUtils::openedFileCount();
  // We need some margin as Qt will crash if it cannot create some file descriptors
  constexpr int SOME_MARGIN = 20;
  return nFileCount > 0 && nFileLimit > 0 && nFileCount + filesToBeOpened > nFileLimit - SOME_MARGIN;
}

QStringList QgsFileUtils::splitPathToComponents( const QString &input )
{
  QStringList result;
  QString path = QDir::cleanPath( input );
  if ( path.isEmpty() )
    return result;

  const QString fileName = QFileInfo( path ).fileName();
  if ( !fileName.isEmpty() )
    result << fileName;
  else if ( QFileInfo( path ).path() == path )
    result << path;

  QString prevPath = path;
  while ( ( path = QFileInfo( path ).path() ).length() < prevPath.length() )
  {
    const QString dirName = QDir( path ).dirName();
    if ( dirName == QLatin1String( "." ) )
      break;

    result << ( !dirName.isEmpty() ? dirName : path );
    prevPath = path;
  }

  std::reverse( result.begin(), result.end() );
  return result;
}

QString QgsFileUtils::uniquePath( const QString &path )
{
  if ( ! QFileInfo::exists( path ) )
  {
    return path;
  }

  QFileInfo info { path };
  const QString suffix { info.completeSuffix() };
  const QString pathPattern { QString( suffix.isEmpty() ? path : path.chopped( suffix.length() + 1 ) ).append( suffix.isEmpty() ? QStringLiteral( "_%1" ) : QStringLiteral( "_%1." ) ).append( suffix ) };
  int i { 2 };
  QString uniquePath { pathPattern.arg( i ) };
  while ( QFileInfo::exists( uniquePath ) )
  {
    ++i;
    uniquePath = pathPattern.arg( i );
  }
  return uniquePath;
}
