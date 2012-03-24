#ifndef QUA_ZIPNEWINFO_H
#define QUA_ZIPNEWINFO_H

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

#include <QDateTime>
#include <QString>

#include "quazip_global.h"

/// Information about a file to be created.
/** This structure holds information about a file to be created inside
 * ZIP archive. At least name should be set to something correct before
 * passing this structure to
 * QuaZipFile::open(OpenMode,const QuaZipNewInfo&,int,int,bool).
 **/
struct QUAZIP_EXPORT QuaZipNewInfo {
  /// File name.
  /** This field holds file name inside archive, including path relative
   * to archive root.
   **/
  QString name;
  /// File timestamp.
  /** This is the last file modification date and time. Will be stored
   * in the archive central directory. It is a good practice to set it
   * to the source file timestamp instead of archive creating time. Use
   * setFileDateTime() or QuaZipNewInfo(const QString&, const QString&).
   **/
  QDateTime dateTime;
  /// File internal attributes.
  quint16 internalAttr;
  /// File external attributes.
  quint32 externalAttr;
  /// File comment.
  /** Will be encoded using QuaZip::getCommentCodec().
   **/
  QString comment;
  /// File local extra field.
  QByteArray extraLocal;
  /// File global extra field.
  QByteArray extraGlobal;
  /// Uncompressed file size.
  /** This is only needed if you are using raw file zipping mode, i. e.
   * adding precompressed file in the zip archive.
   **/
  ulong uncompressedSize;
  /// Constructs QuaZipNewInfo instance.
  /** Initializes name with \a name, dateTime with current date and
   * time. Attributes are initialized with zeros, comment and extra
   * field with null values.
   **/
  QuaZipNewInfo(const QString& name);
  /// Constructs QuaZipNewInfo instance.
  /** Initializes name with \a name and dateTime with timestamp of the
   * file named \a file. If the \a file does not exists or its timestamp
   * is inaccessible (e. g. you do not have read permission for the
   * directory file in), uses current date and time. Attributes are
   * initialized with zeros, comment and extra field with null values.
   * 
   * \sa setFileDateTime()
   **/
  QuaZipNewInfo(const QString& name, const QString& file);
  /// Sets the file timestamp from the existing file.
  /** Use this function to set the file timestamp from the existing
   * file. Use it like this:
   * \code
   * QuaZipFile zipFile(&zip);
   * QFile file("file-to-add");
   * file.open(QIODevice::ReadOnly);
   * QuaZipNewInfo info("file-name-in-archive");
   * info.setFileDateTime("file-to-add"); // take the timestamp from file
   * zipFile.open(QIODevice::WriteOnly, info);
   * \endcode
   *
   * This function does not change dateTime if some error occured (e. g.
   * file is inaccessible).
   **/
  void setFileDateTime(const QString& file);
};

#endif
