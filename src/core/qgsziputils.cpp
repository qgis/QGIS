/***************************************************************************
                            qgsziputils.cpp
                          ---------------------
    begin                : Jul 2017
    copyright            : (C) 2017 by Paul Blottiere
    email                : paul.blottiere@oslandia.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsziputils.h"

#include <fstream>
#include <iostream>
#include <zip.h>
#include <zlib.h>

#include "qgslogger.h"
#include "qgsmessagelog.h"

#include <QDir>
#include <QFileInfo>

bool QgsZipUtils::isZipFile( const QString &filename )
{
  return QFileInfo( filename ).suffix().compare( "qgz"_L1, Qt::CaseInsensitive ) == 0;
}

bool QgsZipUtils::unzip( const QString &zipFilename, const QString &dir, QStringList &files, bool checkConsistency )
{
  files.clear();

  if ( !QFileInfo::exists( zipFilename ) )
  {
    QgsMessageLog::logMessage( QObject::tr( "Error zip file does not exist: '%1'" ).arg( zipFilename ) );
    return false;
  }
  else if ( zipFilename.isEmpty() )
  {
    QgsMessageLog::logMessage( QObject::tr( "Error zip filename is empty" ) );
    return false;
  }
  else if ( !QDir( dir ).exists( dir ) )
  {
    QgsMessageLog::logMessage( QObject::tr( "Error output dir does not exist: '%1'" ).arg( dir ) );
    return false;
  }
  else if ( !QFileInfo( dir ).isDir() )
  {
    QgsMessageLog::logMessage( QObject::tr( "Error output dir is not a directory: '%1'" ).arg( dir ) );
    return false;
  }
  else if ( !QFileInfo( dir ).isWritable() )
  {
    QgsMessageLog::logMessage( QObject::tr( "Error output dir is not writable: '%1'" ).arg( dir ) );
    return false;
  }

  int rc = 0;
  const QByteArray fileNamePtr = zipFilename.toUtf8();
  struct zip *z = zip_open( fileNamePtr.constData(), checkConsistency ? ZIP_CHECKCONS : 0, &rc );

  if ( rc == ZIP_ER_OK && z )
  {
    const int count = zip_get_num_entries( z, ZIP_FL_UNCHANGED );
    if ( count != -1 )
    {
      struct zip_stat stat;

      for ( int i = 0; i < count; i++ )
      {
        zip_stat_index( z, i, 0, &stat );
        const size_t len = stat.size;

        struct zip_file *file = zip_fopen_index( z, i, 0 );
        const std::unique_ptr< char[] > buf( new char[len] );
        if ( zip_fread( file, buf.get(), len ) != -1 )
        {
          const QString fileName( stat.name );
          if ( fileName.endsWith( "/" ) )
          {
            continue;
          }

          const QFileInfo newFile( QDir( dir ), fileName );

          if ( !QString( QDir::cleanPath( newFile.absolutePath() ) + u"/"_s ).startsWith( QDir( dir ).absolutePath() + u"/"_s ) )
          {
            QgsMessageLog::logMessage( QObject::tr( "Skipped file %1 outside of the directory %2" ).arg(
                                         newFile.absoluteFilePath(),
                                         QDir( dir ).absolutePath()
                                       ) );
            continue;
          }

          // Create path for a new file if it does not exist.
          if ( !newFile.absoluteDir().exists() )
          {
            if ( !QDir( dir ).mkpath( newFile.absolutePath() ) )
              QgsMessageLog::logMessage( QObject::tr( "Failed to create a subdirectory %1/%2" ).arg( dir ).arg( fileName ) );
          }

          QFile outFile( newFile.absoluteFilePath() );
          if ( !outFile.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
          {
            QgsMessageLog::logMessage( QObject::tr( "Could not write to %1" ).arg( newFile.absoluteFilePath() ) );
          }
          else
          {
            outFile.write( buf.get(), len );
          }
          zip_fclose( file );
          files.append( newFile.absoluteFilePath() );
        }
        else
        {
          zip_fclose( file );
          QgsMessageLog::logMessage( QObject::tr( "Error reading file: '%1'" ).arg( zip_strerror( z ) ) );
          return false;
        }
      }
    }
    else
    {
      zip_close( z );
      QgsMessageLog::logMessage( QObject::tr( "Error getting files: '%1'" ).arg( zip_strerror( z ) ) );
      return false;
    }

    zip_close( z );
  }
  else
  {
    QgsMessageLog::logMessage( QObject::tr( "Error opening zip archive: '%1' (Error code: %2)" ).arg( z ? zip_strerror( z ) : zipFilename ).arg( rc ) );
    return false;
  }

  return true;
}

bool QgsZipUtils::zip( const QString &zipFilename, const QStringList &files, bool overwrite )
{
  if ( zipFilename.isEmpty() )
  {
    QgsMessageLog::logMessage( QObject::tr( "Error zip filename is empty" ) );
    return false;
  }

  int rc = 0;
  const QByteArray zipFileNamePtr = zipFilename.toUtf8();
  struct zip *z = zip_open( zipFileNamePtr.constData(), overwrite ? ( ZIP_CREATE | ZIP_TRUNCATE ) : ZIP_CREATE, &rc );

  if ( rc == ZIP_ER_OK && z )
  {
    for ( const auto &file : files )
    {
      const QFileInfo fileInfo( file );
      if ( !fileInfo.exists() )
      {
        QgsMessageLog::logMessage( QObject::tr( "Error input file does not exist: '%1'" ).arg( file ) );
        zip_close( z );
        return false;
      }

      const QByteArray fileNamePtr = file.toUtf8();
      zip_source *src = zip_source_file( z, fileNamePtr.constData(), 0, 0 );
      if ( src )
      {
        const QByteArray fileInfoPtr = fileInfo.fileName().toUtf8();
#if LIBZIP_VERSION_MAJOR < 1
        rc = ( int ) zip_add( z, fileInfoPtr.constData(), src );
#else
        rc = ( int ) zip_file_add( z, fileInfoPtr.constData(), src, 0 );
#endif
        if ( rc == -1 )
        {
          QgsMessageLog::logMessage( QObject::tr( "Error adding file '%1': %2" ).arg( file, zip_strerror( z ) ) );
          zip_close( z );
          return false;
        }
      }
      else
      {
        QgsMessageLog::logMessage( QObject::tr( "Error creating data source '%1': %2" ).arg( file, zip_strerror( z ) ) );
        zip_close( z );
        return false;
      }
    }

    zip_close( z );
  }
  else
  {
    QgsMessageLog::logMessage( QObject::tr( "Error creating zip archive '%1': %2" ).arg( zipFilename, z ? zip_strerror( z ) : zipFilename ) );
    return false;
  }

  return true;
}

bool QgsZipUtils::decodeGzip( const QByteArray &bytesIn, QByteArray &bytesOut )
{
  return decodeGzip( bytesIn.constData(), bytesIn.count(), bytesOut );
}

bool QgsZipUtils::decodeGzip( const char *bytesIn, std::size_t size, QByteArray &bytesOut )
{
  unsigned char *bytesInPtr = reinterpret_cast<unsigned char *>( const_cast<char *>( bytesIn ) );
  uint bytesInLeft = static_cast<uint>( size );

  const uint CHUNK = 16384;
  unsigned char out[CHUNK];
  const int DEC_MAGIC_NUM_FOR_GZIP = 16;

  // allocate inflate state
  z_stream strm;
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.avail_in = 0;
  strm.next_in = Z_NULL;

  int ret = inflateInit2( &strm, MAX_WBITS + DEC_MAGIC_NUM_FOR_GZIP );
  if ( ret != Z_OK )
  {
    inflateEnd( &strm );
    return false;
  }

  while ( ret != Z_STREAM_END ) // done when inflate() says it's done
  {
    // prepare next chunk
    const uint bytesToProcess = std::min( CHUNK, bytesInLeft );
    strm.next_in = bytesInPtr;
    strm.avail_in = bytesToProcess;
    bytesInPtr += bytesToProcess;
    bytesInLeft -= bytesToProcess;

    if ( bytesToProcess == 0 )
      break;  // we end with an error - no more data but inflate() wants more data

    // run inflate() on input until output buffer not full
    do
    {
      strm.avail_out = CHUNK;
      strm.next_out = out;
      ret = inflate( &strm, Z_NO_FLUSH );
      Q_ASSERT( ret != Z_STREAM_ERROR ); // state not clobbered
      if ( ret == Z_NEED_DICT || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR )
      {
        inflateEnd( &strm );
        return false;
      }
      const unsigned have = CHUNK - strm.avail_out;
      bytesOut.append( QByteArray::fromRawData( reinterpret_cast<const char *>( out ), static_cast<int>( have ) ) );
    }
    while ( strm.avail_out == 0 );
  }

  inflateEnd( &strm );
  return ret == Z_STREAM_END;
}

bool QgsZipUtils::encodeGzip( const QByteArray &bytesIn, QByteArray &bytesOut )
{
  unsigned char *bytesInPtr = reinterpret_cast<unsigned char *>( const_cast<char *>( bytesIn.constData() ) );
  const uint bytesInLeft = static_cast<uint>( bytesIn.count() );

  const uint CHUNK = 16384;
  unsigned char out[CHUNK];
  const int DEC_MAGIC_NUM_FOR_GZIP = 16;

  // allocate deflate state
  z_stream strm;
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;

  int ret = deflateInit2( &strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, MAX_WBITS + DEC_MAGIC_NUM_FOR_GZIP, 8, Z_DEFAULT_STRATEGY );
  if ( ret != Z_OK )
    return false;

  strm.avail_in = bytesInLeft;
  strm.next_in = bytesInPtr;

  // run deflate() on input until output buffer not full, finish
  // compression if all of source has been read in
  do
  {
    strm.avail_out = CHUNK;
    strm.next_out = out;
    ret = deflate( &strm, Z_FINISH );  // no bad return value
    Q_ASSERT( ret != Z_STREAM_ERROR ); // state not clobbered

    const unsigned have = CHUNK - strm.avail_out;
    bytesOut.append( QByteArray::fromRawData( reinterpret_cast<const char *>( out ), static_cast<int>( have ) ) );
  }
  while ( strm.avail_out == 0 );
  Q_ASSERT( ret == Z_STREAM_END );      // stream will be complete

  // clean up and return
  deflateEnd( &strm );
  return true;
}

const QStringList QgsZipUtils::files( const QString &zip )
{
  if ( zip.isEmpty() && !QFileInfo::exists( zip ) )
  {
    return QStringList();
  }
  QStringList files;

  int rc = 0;
  const QByteArray fileNamePtr = zip.toUtf8();
  struct zip *z = zip_open( fileNamePtr.constData(), 0, &rc );

  if ( rc == ZIP_ER_OK && z )
  {
    const int count = zip_get_num_entries( z, ZIP_FL_UNCHANGED );
    if ( count != -1 )
    {
      struct zip_stat stat;

      for ( int i = 0; i < count; i++ )
      {
        zip_stat_index( z, i, 0, &stat );
        files << QString( stat.name );
      }
    }

    zip_close( z );
  }

  return files;
}

bool QgsZipUtils::extractFileFromZip( const QString &zipFilename, const QString &filenameInZip, QByteArray &bytesOut )
{
  if ( !QFileInfo::exists( zipFilename ) )
  {
    QgsMessageLog::logMessage( QObject::tr( "Error zip file does not exist: '%1'" ).arg( zipFilename ) );
    return false;
  }

  if ( filenameInZip.isEmpty() )
  {
    QgsMessageLog::logMessage( QObject::tr( "Error file name in zip is empty" ) );
    return false;
  }

  int err = 0;
  const QByteArray zipFilenamePtr = zipFilename.toUtf8();
  struct zip *z = zip_open( zipFilenamePtr.constData(), 0, &err );
  if ( !z )
  {
    zip_error_t error;
    zip_error_init_with_code( &error, err );
    QgsMessageLog::logMessage( QObject::tr( "Error opening zip archive '%1': %2" ).arg( zipFilename, zip_error_strerror( &error ) ) );
    zip_error_fini( &error );
    return false;
  }

  const QByteArray filenameInZipPtr = filenameInZip.toUtf8();
  struct zip_stat st;
  zip_stat_init( &st );
  if ( zip_stat( z, filenameInZipPtr.constData(), 0, &st ) != 0 )
  {
    QgsMessageLog::logMessage( QObject::tr( "File '%1' not found in zip archive '%2': %3" ).arg( filenameInZip, zipFilename, zip_strerror( z ) ) );
    zip_close( z );
    return false;
  }

  zip_file *zf = zip_fopen( z, filenameInZipPtr.constData(), 0 );
  if ( !zf )
  {
    QgsMessageLog::logMessage( QObject::tr( "Could not open file '%1' in zip archive '%2': %3" ).arg( filenameInZip, zipFilename, zip_strerror( z ) ) );
    zip_close( z );
    return false;
  }

  bytesOut.resize( static_cast<int>( st.size ) );
  zip_int64_t readBytes = zip_fread( zf, bytesOut.data(), st.size );

  zip_fclose( zf );
  zip_close( z );

  // If successful, the number of bytes actually read is returned.
  if ( static_cast<zip_uint64_t>( readBytes ) != st.size )
  {
    QgsMessageLog::logMessage( QObject::tr( "Error reading file '%1' from zip archive '%2'." ).arg( filenameInZip, zipFilename ) );
    return false;
  }

  return true;
}
