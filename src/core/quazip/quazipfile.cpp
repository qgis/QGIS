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

#include "quazipfile.h"

using namespace std;

/// The implementation class for QuaZip.
/**
\internal

This class contains all the private stuff for the QuaZipFile class, thus
allowing to preserve binary compatibility between releases, the
technique known as the Pimpl (private implementation) idiom.
*/
class QuaZipFilePrivate {
  friend class QuaZipFile;
  private:
    /// The pointer to the associated QuaZipFile instance.
    QuaZipFile *q;
    /// The QuaZip object to work with.
    QuaZip *zip;
    /// The file name.
    QString fileName;
    /// Case sensitivity mode.
    QuaZip::CaseSensitivity caseSensitivity;
    /// Whether this file is opened in the raw mode.
    bool raw;
    /// Write position to keep track of.
    /**
      QIODevice::pos() is broken for non-seekable devices, so we need
      our own position.
      */
    qint64 writePos;
    /// Uncompressed size to write along with a raw file.
    ulong uncompressedSize;
    /// CRC to write along with a raw file.
    quint32 crc;
    /// Whether \ref zip points to an internal QuaZip instance.
    /**
      This is true if the archive was opened by name, rather than by
      supplying an existing QuaZip instance.
      */
    bool internal;
    /// The last error.
    int zipError;
    /// Resets \ref zipError.
    inline void resetZipError() const {setZipError(UNZ_OK);}
    /// Sets the zip error.
    /**
      This function is marked as const although it changes one field.
      This allows to call it from const functions that don't change
      anything by themselves.
      */
    void setZipError(int zipError) const;
    /// The constructor for the corresponding QuaZipFile constructor.
    inline QuaZipFilePrivate(QuaZipFile *q):
      q(q), zip(NULL), internal(true), zipError(UNZ_OK) {}
    /// The constructor for the corresponding QuaZipFile constructor.
    inline QuaZipFilePrivate(QuaZipFile *q, const QString &zipName):
      q(q), internal(true), zipError(UNZ_OK)
      {
        zip=new QuaZip(zipName);
      }
    /// The constructor for the corresponding QuaZipFile constructor.
    inline QuaZipFilePrivate(QuaZipFile *q, const QString &zipName, const QString &fileName,
        QuaZip::CaseSensitivity cs):
      q(q), internal(true), zipError(UNZ_OK)
      {
        zip=new QuaZip(zipName);
        this->fileName=fileName;
        this->caseSensitivity=cs;
      }
    /// The constructor for the QuaZipFile constructor accepting a file name.
    inline QuaZipFilePrivate(QuaZipFile *q, QuaZip *zip):
      q(q), zip(zip), internal(false), zipError(UNZ_OK) {}
    /// The destructor.
    inline ~QuaZipFilePrivate()
    {
      if (internal)
        delete zip;
    }
};

QuaZipFile::QuaZipFile():
  p(new QuaZipFilePrivate(this))
{
}

QuaZipFile::QuaZipFile(QObject *parent):
  QIODevice(parent),
  p(new QuaZipFilePrivate(this))
{
}

QuaZipFile::QuaZipFile(const QString& zipName, QObject *parent):
  QIODevice(parent),
  p(new QuaZipFilePrivate(this, zipName))
{
}

QuaZipFile::QuaZipFile(const QString& zipName, const QString& fileName,
    QuaZip::CaseSensitivity cs, QObject *parent):
  QIODevice(parent),
  p(new QuaZipFilePrivate(this, zipName, fileName, cs))
{
}

QuaZipFile::QuaZipFile(QuaZip *zip, QObject *parent):
  QIODevice(parent),
  p(new QuaZipFilePrivate(this, zip))
{
}

QuaZipFile::~QuaZipFile()
{
  if (isOpen())
    close();
  delete p;
}

QString QuaZipFile::getZipName() const
{
  return p->zip==NULL ? QString() : p->zip->getZipName();
}

QuaZip *QuaZipFile::getZip() const
{
    return p->internal ? NULL : p->zip;
}

QString QuaZipFile::getActualFileName()const
{
  p->setZipError(UNZ_OK);
  if (p->zip == NULL || (openMode() & WriteOnly))
    return QString();
  QString name=p->zip->getCurrentFileName();
  if(name.isNull())
    p->setZipError(p->zip->getZipError());
  return name;
}

void QuaZipFile::setZipName(const QString& zipName)
{
  if(isOpen()) {
    qWarning("QuaZipFile::setZipName(): file is already open - can not set ZIP name");
    return;
  }
  if(p->zip!=NULL && p->internal)
    delete p->zip;
  p->zip=new QuaZip(zipName);
  p->internal=true;
}

void QuaZipFile::setZip(QuaZip *zip)
{
  if(isOpen()) {
    qWarning("QuaZipFile::setZip(): file is already open - can not set ZIP");
    return;
  }
  if(p->zip!=NULL && p->internal)
    delete p->zip;
  p->zip=zip;
  p->fileName=QString();
  p->internal=false;
}

void QuaZipFile::setFileName(const QString& fileName, QuaZip::CaseSensitivity cs)
{
  if(p->zip==NULL) {
    qWarning("QuaZipFile::setFileName(): call setZipName() first");
    return;
  }
  if(!p->internal) {
    qWarning("QuaZipFile::setFileName(): should not be used when not using internal QuaZip");
    return;
  }
  if(isOpen()) {
    qWarning("QuaZipFile::setFileName(): can not set file name for already opened file");
    return;
  }
  p->fileName=fileName;
  p->caseSensitivity=cs;
}

void QuaZipFilePrivate::setZipError(int zipError) const
{
  QuaZipFilePrivate *fakeThis = const_cast<QuaZipFilePrivate*>(this); // non-const
  fakeThis->zipError=zipError;
  if(zipError==UNZ_OK)
    q->setErrorString(QString());
  else
    q->setErrorString(q->tr("ZIP/UNZIP API error %1").arg(zipError));
}

bool QuaZipFile::open(OpenMode mode)
{
  return open(mode, NULL);
}

bool QuaZipFile::open(OpenMode mode, int *method, int *level, bool raw, const char *password)
{
  p->resetZipError();
  if(isOpen()) {
    qWarning("QuaZipFile::open(): already opened");
    return false;
  }
  if(mode&Unbuffered) {
    qWarning("QuaZipFile::open(): Unbuffered mode is not supported");
    return false;
  }
  if((mode&ReadOnly)&&!(mode&WriteOnly)) {
    if(p->internal) {
      if(!p->zip->open(QuaZip::mdUnzip)) {
        p->setZipError(p->zip->getZipError());
        return false;
      }
      if(!p->zip->setCurrentFile(p->fileName, p->caseSensitivity)) {
        p->setZipError(p->zip->getZipError());
        p->zip->close();
        return false;
      }
    } else {
      if(p->zip==NULL) {
        qWarning("QuaZipFile::open(): zip is NULL");
        return false;
      }
      if(p->zip->getMode()!=QuaZip::mdUnzip) {
        qWarning("QuaZipFile::open(): file open mode %d incompatible with ZIP open mode %d",
            (int)mode, (int)p->zip->getMode());
        return false;
      }
      if(!p->zip->hasCurrentFile()) {
        qWarning("QuaZipFile::open(): zip does not have current file");
        return false;
      }
    }
    p->setZipError(unzOpenCurrentFile3(p->zip->getUnzFile(), method, level, (int)raw, password));
    if(p->zipError==UNZ_OK) {
      setOpenMode(mode);
      p->raw=raw;
      return true;
    } else
      return false;
  }
  qWarning("QuaZipFile::open(): open mode %d not supported by this function", (int)mode);
  return false;
}

bool QuaZipFile::open(OpenMode mode, const QuaZipNewInfo& info,
    const char *password, quint32 crc,
    int method, int level, bool raw,
    int windowBits, int memLevel, int strategy)
{
  zip_fileinfo info_z;
  p->resetZipError();
  if(isOpen()) {
    qWarning("QuaZipFile::open(): already opened");
    return false;
  }
  if((mode&WriteOnly)&&!(mode&ReadOnly)) {
    if(p->internal) {
      qWarning("QuaZipFile::open(): write mode is incompatible with internal QuaZip approach");
      return false;
    }
    if(p->zip==NULL) {
      qWarning("QuaZipFile::open(): zip is NULL");
      return false;
    }
    if(p->zip->getMode()!=QuaZip::mdCreate&&p->zip->getMode()!=QuaZip::mdAppend&&p->zip->getMode()!=QuaZip::mdAdd) {
      qWarning("QuaZipFile::open(): file open mode %d incompatible with ZIP open mode %d",
          (int)mode, (int)p->zip->getMode());
      return false;
    }
    info_z.tmz_date.tm_year=info.dateTime.date().year();
    info_z.tmz_date.tm_mon=info.dateTime.date().month() - 1;
    info_z.tmz_date.tm_mday=info.dateTime.date().day();
    info_z.tmz_date.tm_hour=info.dateTime.time().hour();
    info_z.tmz_date.tm_min=info.dateTime.time().minute();
    info_z.tmz_date.tm_sec=info.dateTime.time().second();
    info_z.dosDate = 0;
    info_z.internal_fa=(uLong)info.internalAttr;
    info_z.external_fa=(uLong)info.externalAttr;
    if (!p->zip->isDataDescriptorWritingEnabled())
        zipClearFlags(p->zip->getZipFile(), ZIP_WRITE_DATA_DESCRIPTOR);
    p->setZipError(zipOpenNewFileInZip3(p->zip->getZipFile(),
          p->zip->getFileNameCodec()->fromUnicode(info.name).constData(), &info_z,
          info.extraLocal.constData(), info.extraLocal.length(),
          info.extraGlobal.constData(), info.extraGlobal.length(),
          p->zip->getCommentCodec()->fromUnicode(info.comment).constData(),
          method, level, (int)raw,
          windowBits, memLevel, strategy,
          password, (uLong)crc));
    if(p->zipError==UNZ_OK) {
      p->writePos=0;
      setOpenMode(mode);
      p->raw=raw;
      if(raw) {
        p->crc=crc;
        p->uncompressedSize=info.uncompressedSize;
      }
      return true;
    } else
      return false;
  }
  qWarning("QuaZipFile::open(): open mode %d not supported by this function", (int)mode);
  return false;
}

bool QuaZipFile::isSequential()const
{
  return true;
}

qint64 QuaZipFile::pos()const
{
  if(p->zip==NULL) {
    qWarning("QuaZipFile::pos(): call setZipName() or setZip() first");
    return -1;
  }
  if(!isOpen()) {
    qWarning("QuaZipFile::pos(): file is not open");
    return -1;
  }
  if(openMode()&ReadOnly)
      // QIODevice::pos() is broken for sequential devices,
      // but thankfully bytesAvailable() returns the number of
      // bytes buffered, so we know how far ahead we are.
    return unztell(p->zip->getUnzFile()) - QIODevice::bytesAvailable();
  else
    return p->writePos;
}

bool QuaZipFile::atEnd()const
{
  if(p->zip==NULL) {
    qWarning("QuaZipFile::atEnd(): call setZipName() or setZip() first");
    return false;
  }
  if(!isOpen()) {
    qWarning("QuaZipFile::atEnd(): file is not open");
    return false;
  }
  if(openMode()&ReadOnly)
      // the same problem as with pos()
    return QIODevice::bytesAvailable() == 0
        && unzeof(p->zip->getUnzFile())==1;
  else
    return true;
}

qint64 QuaZipFile::size()const
{
  if(!isOpen()) {
    qWarning("QuaZipFile::atEnd(): file is not open");
    return -1;
  }
  if(openMode()&ReadOnly)
    return p->raw?csize():usize();
  else
    return p->writePos;
}

qint64 QuaZipFile::csize()const
{
  unz_file_info info_z;
  p->setZipError(UNZ_OK);
  if(p->zip==NULL||p->zip->getMode()!=QuaZip::mdUnzip) return -1;
  p->setZipError(unzGetCurrentFileInfo(p->zip->getUnzFile(), &info_z, NULL, 0, NULL, 0, NULL, 0));
  if(p->zipError!=UNZ_OK)
    return -1;
  return info_z.compressed_size;
}

qint64 QuaZipFile::usize()const
{
  unz_file_info info_z;
  p->setZipError(UNZ_OK);
  if(p->zip==NULL||p->zip->getMode()!=QuaZip::mdUnzip) return -1;
  p->setZipError(unzGetCurrentFileInfo(p->zip->getUnzFile(), &info_z, NULL, 0, NULL, 0, NULL, 0));
  if(p->zipError!=UNZ_OK)
    return -1;
  return info_z.uncompressed_size;
}

bool QuaZipFile::getFileInfo(QuaZipFileInfo *info)
{
  if(p->zip==NULL||p->zip->getMode()!=QuaZip::mdUnzip) return false;
  p->zip->getCurrentFileInfo(info);
  p->setZipError(p->zip->getZipError());
  return p->zipError==UNZ_OK;
}

void QuaZipFile::close()
{
  p->resetZipError();
  if(p->zip==NULL||!p->zip->isOpen()) return;
  if(!isOpen()) {
    qWarning("QuaZipFile::close(): file isn't open");
    return;
  }
  if(openMode()&ReadOnly)
    p->setZipError(unzCloseCurrentFile(p->zip->getUnzFile()));
  else if(openMode()&WriteOnly)
    if(isRaw()) p->setZipError(zipCloseFileInZipRaw(p->zip->getZipFile(), p->uncompressedSize, p->crc));
    else p->setZipError(zipCloseFileInZip(p->zip->getZipFile()));
  else {
    qWarning("Wrong open mode: %d", (int)openMode());
    return;
  }
  if(p->zipError==UNZ_OK) setOpenMode(QIODevice::NotOpen);
  else return;
  if(p->internal) {
    p->zip->close();
    p->setZipError(p->zip->getZipError());
  }
}

qint64 QuaZipFile::readData(char *data, qint64 maxSize)
{
  p->setZipError(UNZ_OK);
  qint64 bytesRead=unzReadCurrentFile(p->zip->getUnzFile(), data, (unsigned)maxSize);
  if(bytesRead<0) p->setZipError((int)bytesRead);
  return bytesRead;
}

qint64 QuaZipFile::writeData(const char* data, qint64 maxSize)
{
  p->setZipError(ZIP_OK);
  p->setZipError(zipWriteInFileInZip(p->zip->getZipFile(), data, (uint)maxSize));
  if(p->zipError!=ZIP_OK) return -1;
  else {
    p->writePos+=maxSize;
    return maxSize;
  }
}

QString QuaZipFile::getFileName() const
{
  return p->fileName;
}

QuaZip::CaseSensitivity QuaZipFile::getCaseSensitivity() const
{
  return p->caseSensitivity;
}

bool QuaZipFile::isRaw() const
{
  return p->raw;
}

int QuaZipFile::getZipError() const
{
  return p->zipError;
}

qint64 QuaZipFile::bytesAvailable() const
{
    return size() - pos();
}
