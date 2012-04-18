/*
Copyright (C) 2005-2011 Sergey A. Tachenov

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

See COPYING file for the full LGPL text.

Original ZIP package is copyrighted by Gilles Vollant, see
quazip/(un)zip.h files for details, basically it's zlib license.
 **/

#include <QFile>
#include <QFlags>

#include "quazip.h"

/// All the internal stuff for the QuaZip class.
/**
  \internal

  This class keeps all the private stuff for the QuaZip class so it can
  be changed without breaking binary compatibility, according to the
  Pimpl idiom.
  */
class QuaZipPrivate
{
    friend class QuaZip;
  private:
    /// The pointer to the corresponding QuaZip instance.
    QuaZip *q;
    /// The codec for file names.
    QTextCodec *fileNameCodec;
    /// The codec for comments.
    QTextCodec *commentCodec;
    /// The archive file name.
    QString zipName;
    /// The device to access the archive.
    QIODevice *ioDevice;
    /// The global comment.
    QString comment;
    /// The open mode.
    QuaZip::Mode mode;
    union
    {
      /// The internal handle for UNZIP modes.
      unzFile unzFile_f;
      /// The internal handle for ZIP modes.
      zipFile zipFile_f;
    };
    /// Whether a current file is set.
    bool hasCurrentFile_f;
    /// The last error.
    int zipError;
    /// Whether \ref QuaZip::setDataDescriptorWritingEnabled() "the data descriptor writing mode" is enabled.
    bool dataDescriptorWritingEnabled;
    /// The constructor for the corresponding QuaZip constructor.
    inline QuaZipPrivate( QuaZip *q ):
        q( q ),
        fileNameCodec( QTextCodec::codecForLocale() ),
        commentCodec( QTextCodec::codecForLocale() ),
        ioDevice( NULL ),
        mode( QuaZip::mdNotOpen ),
        hasCurrentFile_f( false ),
        zipError( UNZ_OK ),
        dataDescriptorWritingEnabled( true ) {}
    /// The constructor for the corresponding QuaZip constructor.
    inline QuaZipPrivate( QuaZip *q, const QString &zipName ):
        q( q ),
        fileNameCodec( QTextCodec::codecForLocale() ),
        commentCodec( QTextCodec::codecForLocale() ),
        zipName( zipName ),
        ioDevice( NULL ),
        mode( QuaZip::mdNotOpen ),
        hasCurrentFile_f( false ),
        zipError( UNZ_OK ),
        dataDescriptorWritingEnabled( true ) {}
    /// The constructor for the corresponding QuaZip constructor.
    inline QuaZipPrivate( QuaZip *q, QIODevice *ioDevice ):
        q( q ),
        fileNameCodec( QTextCodec::codecForLocale() ),
        commentCodec( QTextCodec::codecForLocale() ),
        ioDevice( ioDevice ),
        mode( QuaZip::mdNotOpen ),
        hasCurrentFile_f( false ),
        zipError( UNZ_OK ),
        dataDescriptorWritingEnabled( true ) {}
    /// Returns either a list of file names or a list of QuaZipFileInfo.
    template<typename TFileInfo>
    bool getFileInfoList( QList<TFileInfo> *result ) const;
};

QuaZip::QuaZip():
    p( new QuaZipPrivate( this ) )
{
}

QuaZip::QuaZip( const QString& zipName ):
    p( new QuaZipPrivate( this, zipName ) )
{
}

QuaZip::QuaZip( QIODevice *ioDevice ):
    p( new QuaZipPrivate( this, ioDevice ) )
{
}

QuaZip::~QuaZip()
{
  if ( isOpen() )
    close();
  delete p;
}

bool QuaZip::open( Mode mode, zlib_filefunc_def* ioApi )
{
  p->zipError = UNZ_OK;
  if ( isOpen() )
  {
    qWarning( "QuaZip::open(): ZIP already opened" );
    return false;
  }
  QIODevice *ioDevice = p->ioDevice;
  if ( ioDevice == NULL )
  {
    if ( p->zipName.isEmpty() )
    {
      qWarning( "QuaZip::open(): set either ZIP file name or IO device first" );
      return false;
    }
    else
    {
      ioDevice = new QFile( p->zipName );
    }
  }
  switch ( mode )
  {
    case mdUnzip:
      p->unzFile_f = unzOpen2( ioDevice, ioApi );
      if ( p->unzFile_f != NULL )
      {
        p->mode = mode;
        p->ioDevice = ioDevice;
        return true;
      }
      else
      {
        p->zipError = UNZ_OPENERROR;
        if ( !p->zipName.isEmpty() )
          delete ioDevice;
        return false;
      }
    case mdCreate:
    case mdAppend:
    case mdAdd:
      p->zipFile_f = zipOpen2( ioDevice,
                               mode == mdCreate ? APPEND_STATUS_CREATE :
                               mode == mdAppend ? APPEND_STATUS_CREATEAFTER :
                               APPEND_STATUS_ADDINZIP,
                               NULL,
                               ioApi );
      if ( p->zipFile_f != NULL )
      {
        p->mode = mode;
        p->ioDevice = ioDevice;
        return true;
      }
      else
      {
        p->zipError = UNZ_OPENERROR;
        if ( !p->zipName.isEmpty() )
          delete ioDevice;
        return false;
      }
    default:
      qWarning( "QuaZip::open(): unknown mode: %d", ( int )mode );
      if ( !p->zipName.isEmpty() )
        delete ioDevice;
      return false;
      break;
  }
}

void QuaZip::close()
{
  p->zipError = UNZ_OK;
  switch ( p->mode )
  {
    case mdNotOpen:
      qWarning( "QuaZip::close(): ZIP is not open" );
      return;
    case mdUnzip:
      p->zipError = unzClose( p->unzFile_f );
      break;
    case mdCreate:
    case mdAppend:
    case mdAdd:
      p->zipError = zipClose( p->zipFile_f, p->commentCodec->fromUnicode( p->comment ).constData() );
      break;
    default:
      qWarning( "QuaZip::close(): unknown mode: %d", ( int )p->mode );
      return;
  }
  // opened by name, need to delete the internal IO device
  if ( !p->zipName.isEmpty() )
  {
    delete p->ioDevice;
    p->ioDevice = NULL;
  }
  if ( p->zipError == UNZ_OK )
    p->mode = mdNotOpen;
}

void QuaZip::setZipName( const QString& zipName )
{
  if ( isOpen() )
  {
    qWarning( "QuaZip::setZipName(): ZIP is already open!" );
    return;
  }
  p->zipName = zipName;
  p->ioDevice = NULL;
}

void QuaZip::setIoDevice( QIODevice *ioDevice )
{
  if ( isOpen() )
  {
    qWarning( "QuaZip::setIoDevice(): ZIP is already open!" );
    return;
  }
  p->ioDevice = ioDevice;
  p->zipName = QString();
}

int QuaZip::getEntriesCount()const
{
  QuaZip *fakeThis = ( QuaZip* )this; // non-const
  fakeThis->p->zipError = UNZ_OK;
  if ( p->mode != mdUnzip )
  {
    qWarning( "QuaZip::getEntriesCount(): ZIP is not open in mdUnzip mode" );
    return -1;
  }
  unz_global_info globalInfo;
  if (( fakeThis->p->zipError = unzGetGlobalInfo( p->unzFile_f, &globalInfo ) ) != UNZ_OK )
    return p->zipError;
  return ( int )globalInfo.number_entry;
}

QString QuaZip::getComment()const
{
  QuaZip *fakeThis = ( QuaZip* )this; // non-const
  fakeThis->p->zipError = UNZ_OK;
  if ( p->mode != mdUnzip )
  {
    qWarning( "QuaZip::getComment(): ZIP is not open in mdUnzip mode" );
    return QString();
  }
  unz_global_info globalInfo;
  QByteArray comment;
  if (( fakeThis->p->zipError = unzGetGlobalInfo( p->unzFile_f, &globalInfo ) ) != UNZ_OK )
    return QString();
  comment.resize( globalInfo.size_comment );
  if (( fakeThis->p->zipError = unzGetGlobalComment( p->unzFile_f, comment.data(), comment.size() ) ) < 0 )
    return QString();
  fakeThis->p->zipError = UNZ_OK;
  return p->commentCodec->toUnicode( comment );
}

bool QuaZip::setCurrentFile( const QString& fileName, CaseSensitivity cs )
{
  p->zipError = UNZ_OK;
  if ( p->mode != mdUnzip )
  {
    qWarning( "QuaZip::setCurrentFile(): ZIP is not open in mdUnzip mode" );
    return false;
  }
  if ( fileName.isEmpty() )
  {
    p->hasCurrentFile_f = false;
    return true;
  }
  // Unicode-aware reimplementation of the unzLocateFile function
  if ( p->unzFile_f == NULL )
  {
    p->zipError = UNZ_PARAMERROR;
    return false;
  }
  if ( fileName.length() > MAX_FILE_NAME_LENGTH )
  {
    p->zipError = UNZ_PARAMERROR;
    return false;
  }
  bool sens;
  if ( cs == csDefault )
  {
#ifdef Q_WS_WIN
    sens = false;
#else
    sens = true;
#endif
  }
  else sens = cs == csSensitive;
  QString lower, current;
  if ( !sens ) lower = fileName.toLower();
  p->hasCurrentFile_f = false;
  for ( bool more = goToFirstFile(); more; more = goToNextFile() )
  {
    current = getCurrentFileName();
    if ( current.isEmpty() ) return false;
    if ( sens )
    {
      if ( current == fileName ) break;
    }
    else
    {
      if ( current.toLower() == lower ) break;
    }
  }
  return p->hasCurrentFile_f;
}

bool QuaZip::goToFirstFile()
{
  p->zipError = UNZ_OK;
  if ( p->mode != mdUnzip )
  {
    qWarning( "QuaZip::goToFirstFile(): ZIP is not open in mdUnzip mode" );
    return false;
  }
  p->zipError = unzGoToFirstFile( p->unzFile_f );
  p->hasCurrentFile_f = p->zipError == UNZ_OK;
  return p->hasCurrentFile_f;
}

bool QuaZip::goToNextFile()
{
  p->zipError = UNZ_OK;
  if ( p->mode != mdUnzip )
  {
    qWarning( "QuaZip::goToFirstFile(): ZIP is not open in mdUnzip mode" );
    return false;
  }
  p->zipError = unzGoToNextFile( p->unzFile_f );
  p->hasCurrentFile_f = p->zipError == UNZ_OK;
  if ( p->zipError == UNZ_END_OF_LIST_OF_FILE )
    p->zipError = UNZ_OK;
  return p->hasCurrentFile_f;
}

bool QuaZip::getCurrentFileInfo( QuaZipFileInfo *info )const
{
  QuaZip *fakeThis = ( QuaZip* )this; // non-const
  fakeThis->p->zipError = UNZ_OK;
  if ( p->mode != mdUnzip )
  {
    qWarning( "QuaZip::getCurrentFileInfo(): ZIP is not open in mdUnzip mode" );
    return false;
  }
  unz_file_info info_z;
  QByteArray fileName;
  QByteArray extra;
  QByteArray comment;
  if ( info == NULL ) return false;
  if ( !isOpen() || !hasCurrentFile() ) return false;
  if (( fakeThis->p->zipError = unzGetCurrentFileInfo( p->unzFile_f, &info_z, NULL, 0, NULL, 0, NULL, 0 ) ) != UNZ_OK )
    return false;
  fileName.resize( info_z.size_filename );
  extra.resize( info_z.size_file_extra );
  comment.resize( info_z.size_file_comment );
  if (( fakeThis->p->zipError = unzGetCurrentFileInfo( p->unzFile_f, NULL,
                                fileName.data(), fileName.size(),
                                extra.data(), extra.size(),
                                comment.data(), comment.size() ) ) != UNZ_OK )
    return false;
  info->versionCreated = info_z.version;
  info->versionNeeded = info_z.version_needed;
  info->flags = info_z.flag;
  info->method = info_z.compression_method;
  info->crc = info_z.crc;
  info->compressedSize = info_z.compressed_size;
  info->uncompressedSize = info_z.uncompressed_size;
  info->diskNumberStart = info_z.disk_num_start;
  info->internalAttr = info_z.internal_fa;
  info->externalAttr = info_z.external_fa;
  info->name = p->fileNameCodec->toUnicode( fileName );
  info->comment = p->commentCodec->toUnicode( comment );
  info->extra = extra;
  info->dateTime = QDateTime(
                     QDate( info_z.tmu_date.tm_year, info_z.tmu_date.tm_mon + 1, info_z.tmu_date.tm_mday ),
                     QTime( info_z.tmu_date.tm_hour, info_z.tmu_date.tm_min, info_z.tmu_date.tm_sec ) );
  return true;
}

QString QuaZip::getCurrentFileName()const
{
  QuaZip *fakeThis = ( QuaZip* )this; // non-const
  fakeThis->p->zipError = UNZ_OK;
  if ( p->mode != mdUnzip )
  {
    qWarning( "QuaZip::getCurrentFileName(): ZIP is not open in mdUnzip mode" );
    return QString();
  }
  if ( !isOpen() || !hasCurrentFile() ) return QString();
  QByteArray fileName( MAX_FILE_NAME_LENGTH, 0 );
  if (( fakeThis->p->zipError = unzGetCurrentFileInfo( p->unzFile_f, NULL, fileName.data(), fileName.size(),
                                NULL, 0, NULL, 0 ) ) != UNZ_OK )
    return QString();
  return p->fileNameCodec->toUnicode( fileName.constData() );
}

void QuaZip::setFileNameCodec( QTextCodec *fileNameCodec )
{
  p->fileNameCodec = fileNameCodec;
}

void QuaZip::setFileNameCodec( const char *fileNameCodecName )
{
  p->fileNameCodec = QTextCodec::codecForName( fileNameCodecName );
}

QTextCodec *QuaZip::getFileNameCodec()const
{
  return p->fileNameCodec;
}

void QuaZip::setCommentCodec( QTextCodec *commentCodec )
{
  p->commentCodec = commentCodec;
}

void QuaZip::setCommentCodec( const char *commentCodecName )
{
  p->commentCodec = QTextCodec::codecForName( commentCodecName );
}

QTextCodec *QuaZip::getCommentCodec()const
{
  return p->commentCodec;
}

QString QuaZip::getZipName() const
{
  return p->zipName;
}

QIODevice *QuaZip::getIoDevice() const
{
  if ( !p->zipName.isEmpty() ) // opened by name, using an internal QIODevice
    return NULL;
  return p->ioDevice;
}

QuaZip::Mode QuaZip::getMode()const
{
  return p->mode;
}

bool QuaZip::isOpen()const
{
  return p->mode != mdNotOpen;
}

int QuaZip::getZipError() const
{
  return p->zipError;
}

void QuaZip::setComment( const QString& comment )
{
  p->comment = comment;
}

bool QuaZip::hasCurrentFile()const
{
  return p->hasCurrentFile_f;
}

unzFile QuaZip::getUnzFile()
{
  return p->unzFile_f;
}

zipFile QuaZip::getZipFile()
{
  return p->zipFile_f;
}

void QuaZip::setDataDescriptorWritingEnabled( bool enabled )
{
  p->dataDescriptorWritingEnabled = enabled;
}

bool QuaZip::isDataDescriptorWritingEnabled() const
{
  return p->dataDescriptorWritingEnabled;
}

template<typename TFileInfo>
static TFileInfo getFileInfo( QuaZip *zip, bool *ok );

template<>
QuaZipFileInfo getFileInfo( QuaZip *zip, bool *ok )
{
  QuaZipFileInfo info;
  *ok = zip->getCurrentFileInfo( &info );
  return info;
}

template<>
QString getFileInfo( QuaZip *zip, bool *ok )
{
  QString name = zip->getCurrentFileName();
  *ok = !name.isEmpty();
  return name;
}

template<typename TFileInfo>
bool QuaZipPrivate::getFileInfoList( QList<TFileInfo> *result ) const
{
  QuaZipPrivate *fakeThis = const_cast<QuaZipPrivate*>( this );
  fakeThis->zipError = UNZ_OK;
  if ( mode != QuaZip::mdUnzip )
  {
    qWarning( "QuaZip::getFileNameList/getFileInfoList(): "
              "ZIP is not open in mdUnzip mode" );
    return false;
  }
  QString currentFile;
  if ( q->hasCurrentFile() )
  {
    currentFile = q->getCurrentFileName();
  }
  if ( q->goToFirstFile() )
  {
    do
    {
      bool ok;
      result->append( getFileInfo<TFileInfo>( q, &ok ) );
      if ( !ok )
        return false;
    }
    while ( q->goToNextFile() );
  }
  if ( zipError != UNZ_OK )
    return false;
  if ( currentFile.isEmpty() )
  {
    if ( !q->setCurrentFile( currentFile ) )
      return false;
  }
  else
  {
    if ( !q->goToFirstFile() )
      return false;
  }
  return true;
}

QStringList QuaZip::getFileNameList() const
{
  QStringList list;
  if ( p->getFileInfoList( &list ) )
    return list;
  else
    return QStringList();
}

QList<QuaZipFileInfo> QuaZip::getFileInfoList() const
{
  QList<QuaZipFileInfo> list;
  if ( p->getFileInfoList( &list ) )
    return list;
  else
    return QList<QuaZipFileInfo>();
}
