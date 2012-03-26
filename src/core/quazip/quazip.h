#ifndef QUA_ZIP_H
#define QUA_ZIP_H

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

#include <QString>
#include <QStringList>
#include <QTextCodec>

#include "zip.h"
#include "unzip.h"

#include "quazip_global.h"
#include "quazipfileinfo.h"

// just in case it will be defined in the later versions of the ZIP/UNZIP
#ifndef UNZ_OPENERROR
// define additional error code
#define UNZ_OPENERROR -1000
#endif

class QuaZipPrivate;

/// ZIP archive.
/** \class QuaZip quazip.h <quazip/quazip.h>
 * This class implements basic interface to the ZIP archive. It can be
 * used to read table contents of the ZIP archive and retreiving
 * information about the files inside it.
 *
 * You can also use this class to open files inside archive by passing
 * pointer to the instance of this class to the constructor of the
 * QuaZipFile class. But see QuaZipFile::QuaZipFile(QuaZip*, QObject*)
 * for the possible pitfalls.
 *
 * This class is indended to provide interface to the ZIP subpackage of
 * the ZIP/UNZIP package as well as to the UNZIP subpackage. But
 * currently it supports only UNZIP.
 *
 * The use of this class is simple - just create instance using
 * constructor, then set ZIP archive file name using setFile() function
 * (if you did not passed the name to the constructor), then open() and
 * then use different functions to work with it! Well, if you are
 * paranoid, you may also wish to call close before destructing the
 * instance, to check for errors on close.
 *
 * You may also use getUnzFile() and getZipFile() functions to get the
 * ZIP archive handle and use it with ZIP/UNZIP package API directly.
 *
 * This class supports localized file names inside ZIP archive, but you
 * have to set up proper codec with setCodec() function. By default,
 * locale codec will be used, which is probably ok for UNIX systems, but
 * will almost certainly fail with ZIP archives created in Windows. This
 * is because Windows ZIP programs have strange habit of using DOS
 * encoding for file names in ZIP archives. For example, ZIP archive
 * with cyrillic names created in Windows will have file names in \c
 * IBM866 encoding instead of \c WINDOWS-1251. I think that calling one
 * function is not much trouble, but for true platform independency it
 * would be nice to have some mechanism for file name encoding auto
 * detection using locale information. Does anyone know a good way to do
 * it?
 **/
class QUAZIP_EXPORT QuaZip {
  friend class QuaZipPrivate;
  public:
    /// Useful constants.
    enum Constants {
      MAX_FILE_NAME_LENGTH=256 /**< Maximum file name length. Taken from
                                 \c UNZ_MAXFILENAMEINZIP constant in
                                 unzip.c. */
    };
    /// Open mode of the ZIP file.
    enum Mode {
      mdNotOpen, ///< ZIP file is not open. This is the initial mode.
      mdUnzip, ///< ZIP file is open for reading files inside it.
      mdCreate, ///< ZIP file was created with open() call.
      mdAppend, /**< ZIP file was opened in append mode. This refers to
                  * \c APPEND_STATUS_CREATEAFTER mode in ZIP/UNZIP package
                  * and means that zip is appended to some existing file
                  * what is useful when that file contains
                  * self-extractor code. This is obviously \em not what
                  * you whant to use to add files to the existing ZIP
                  * archive.
                  **/
      mdAdd ///< ZIP file was opened for adding files in the archive.
    };
    /// Case sensitivity for the file names.
    /** This is what you specify when accessing files in the archive.
     * Works perfectly fine with any characters thanks to Qt's great
     * unicode support. This is different from ZIP/UNZIP API, where
     * only US-ASCII characters was supported.
     **/
    enum CaseSensitivity {
      csDefault=0, ///< Default for platform. Case sensitive for UNIX, not for Windows.
      csSensitive=1, ///< Case sensitive.
      csInsensitive=2 ///< Case insensitive.
    };
  private:
    QuaZipPrivate *p;
    // not (and will not be) implemented
    QuaZip(const QuaZip& that);
    // not (and will not be) implemented
    QuaZip& operator=(const QuaZip& that);
  public:
    /// Constructs QuaZip object.
    /** Call setName() before opening constructed object. */
    QuaZip();
    /// Constructs QuaZip object associated with ZIP file \a zipName.
    QuaZip(const QString& zipName);
    /// Constructs QuaZip object associated with ZIP file represented by \a ioDevice.
    /** The IO device must be seekable, otherwise an error will occur when opening. */
    QuaZip(QIODevice *ioDevice);
    /// Destroys QuaZip object.
    /** Calls close() if necessary. */
    ~QuaZip();
    /// Opens ZIP file.
    /**
     * Argument \a mode specifies open mode of the ZIP archive. See Mode
     * for details. Note that there is zipOpen2() function in the
     * ZIP/UNZIP API which accepts \a globalcomment argument, but it
     * does not use it anywhere, so this open() function does not have this
     * argument. See setComment() if you need to set global comment.
     *
     * If the ZIP file is accessed via explicitly set QIODevice, then
     * this device is opened in the necessary mode. If the device was
     * already opened by some other means, then the behaviour is defined by
     * the device implementation, but generally it is not a very good
     * idea. For example, QFile will at least issue a warning.
     *
     * \return \c true if successful, \c false otherwise.
     *
     * \note ZIP/UNZIP API open calls do not return error code - they
     * just return \c NULL indicating an error. But to make things
     * easier, quazip.h header defines additional error code \c
     * UNZ_ERROROPEN and getZipError() will return it if the open call
     * of the ZIP/UNZIP API returns \c NULL.
     *
     * Argument \a ioApi specifies IO function set for ZIP/UNZIP
     * package to use. See unzip.h, zip.h and ioapi.h for details. Note
     * that IO API for QuaZip is different from the original package.
     * The file path argument was changed to be of type \c voidpf, and
     * QuaZip passes a QIODevice pointer there. This QIODevice is either
     * set explicitly via setIoDevice() or the QuaZip(QIODevice*)
     * constructor, or it is created internally when opening the archive
     * by its file name. The default API (qioapi.cpp) just delegates
     * everything to the QIODevice API. Not only this allows to use a
     * QIODevice instead of file name, but also has a nice side effect
     * of raising the file size limit from 2G to 4G.
     *
     * In short: just forget about the \a ioApi argument and you'll be
     * fine.
     **/
    bool open(Mode mode, zlib_filefunc_def *ioApi =NULL);
    /// Closes ZIP file.
    /** Call getZipError() to determine if the close was successful. The
     * underlying QIODevice is also closed, regardless of whether it was
     * set explicitly or not. */
    void close();
    /// Sets the codec used to encode/decode file names inside archive.
    /** This is necessary to access files in the ZIP archive created
     * under Windows with non-latin characters in file names. For
     * example, file names with cyrillic letters will be in \c IBM866
     * encoding.
     **/
    void setFileNameCodec(QTextCodec *fileNameCodec);
    /// Sets the codec used to encode/decode file names inside archive.
    /** \overload
     * Equivalent to calling setFileNameCodec(QTextCodec::codecForName(codecName));
     **/
    void setFileNameCodec(const char *fileNameCodecName);
    /// Returns the codec used to encode/decode comments inside archive.
    QTextCodec* getFileNameCodec() const;
    /// Sets the codec used to encode/decode comments inside archive.
    /** This codec defaults to locale codec, which is probably ok.
     **/
    void setCommentCodec(QTextCodec *commentCodec);
    /// Sets the codec used to encode/decode comments inside archive.
    /** \overload
     * Equivalent to calling setCommentCodec(QTextCodec::codecForName(codecName));
     **/
    void setCommentCodec(const char *commentCodecName);
    /// Returns the codec used to encode/decode comments inside archive.
    QTextCodec* getCommentCodec() const;
    /// Returns the name of the ZIP file.
    /** Returns null string if no ZIP file name has been set, for
     * example when the QuaZip instance is set up to use a QIODevice
     * instead.
     * \sa setZipName(), setIoDevice(), getIoDevice()
     **/
    QString getZipName() const;
    /// Sets the name of the ZIP file.
    /** Does nothing if the ZIP file is open.
     *
     * Does not reset error code returned by getZipError().
     * \sa setIoDevice(), getIoDevice(), getZipName()
     **/
    void setZipName(const QString& zipName);
    /// Returns the device representing this ZIP file.
    /** Returns null string if no device has been set explicitly, for
     * example when opening a ZIP file by name.
     * \sa setIoDevice(), getZipName(), setZipName()
     **/
    QIODevice *getIoDevice() const;
    /// Sets the device representing the ZIP file.
    /** Does nothing if the ZIP file is open.
     *
     * Does not reset error code returned by getZipError().
     * \sa getIoDevice(), getZipName(), setZipName()
     **/
    void setIoDevice(QIODevice *ioDevice);
    /// Returns the mode in which ZIP file was opened.
    Mode getMode() const;
    /// Returns \c true if ZIP file is open, \c false otherwise.
    bool isOpen() const;
    /// Returns the error code of the last operation.
    /** Returns \c UNZ_OK if the last operation was successful.
     *
     * Error code resets to \c UNZ_OK every time you call any function
     * that accesses something inside ZIP archive, even if it is \c
     * const (like getEntriesCount()). open() and close() calls reset
     * error code too. See documentation for the specific functions for
     * details on error detection.
     **/
    int getZipError() const;
    /// Returns number of the entries in the ZIP central directory.
    /** Returns negative error code in the case of error. The same error
     * code will be returned by subsequent getZipError() call.
     **/
    int getEntriesCount() const;
    /// Returns global comment in the ZIP file.
    QString getComment() const;
    /// Sets global comment in the ZIP file.
    /** Comment will be written to the archive on close operation.
     *
     * \sa open()
     **/
    void setComment(const QString& comment);
    /// Sets the current file to the first file in the archive.
    /** Returns \c true on success, \c false otherwise. Call
     * getZipError() to get the error code.
     **/
    bool goToFirstFile();
    /// Sets the current file to the next file in the archive.
    /** Returns \c true on success, \c false otherwise. Call
     * getZipError() to determine if there was an error.
     *
     * Should be used only in QuaZip::mdUnzip mode.
     *
     * \note If the end of file was reached, getZipError() will return
     * \c UNZ_OK instead of \c UNZ_END_OF_LIST_OF_FILE. This is to make
     * things like this easier:
     * \code
     * for(bool more=zip.goToFirstFile(); more; more=zip.goToNextFile()) {
     *   // do something
     * }
     * if(zip.getZipError()==UNZ_OK) {
     *   // ok, there was no error
     * }
     * \endcode
     **/
    bool goToNextFile();
    /// Sets current file by its name.
    /** Returns \c true if successful, \c false otherwise. Argument \a
     * cs specifies case sensitivity of the file name. Call
     * getZipError() in the case of a failure to get error code.
     *
     * This is not a wrapper to unzLocateFile() function. That is
     * because I had to implement locale-specific case-insensitive
     * comparison.
     *
     * Here are the differences from the original implementation:
     *
     * - If the file was not found, error code is \c UNZ_OK, not \c
     *   UNZ_END_OF_LIST_OF_FILE (see also goToNextFile()).
     * - If this function fails, it unsets the current file rather than
     *   resetting it back to what it was before the call.
     *
     * If \a fileName is null string then this function unsets the
     * current file and return \c true. Note that you should close the
     * file first if it is open! See
     * QuaZipFile::QuaZipFile(QuaZip*,QObject*) for the details.
     *
     * Should be used only in QuaZip::mdUnzip mode.
     *
     * \sa setFileNameCodec(), CaseSensitivity
     **/
    bool setCurrentFile(const QString& fileName, CaseSensitivity cs =csDefault);
    /// Returns \c true if the current file has been set.
    bool hasCurrentFile() const;
    /// Retrieves information about the current file.
    /** Fills the structure pointed by \a info. Returns \c true on
     * success, \c false otherwise. In the latter case structure pointed
     * by \a info remains untouched. If there was an error,
     * getZipError() returns error code.
     *
     * Should be used only in QuaZip::mdUnzip mode.
     *
     * Does nothing and returns \c false in any of the following cases.
     * - ZIP is not open;
     * - ZIP does not have current file;
     * - \a info is \c NULL;
     *
     * In all these cases getZipError() returns \c UNZ_OK since there
     * is no ZIP/UNZIP API call.
     **/
    bool getCurrentFileInfo(QuaZipFileInfo* info)const;
    /// Returns the current file name.
    /** Equivalent to calling getCurrentFileInfo() and then getting \c
     * name field of the QuaZipFileInfo structure, but faster and more
     * convenient.
     *
     * Should be used only in QuaZip::mdUnzip mode.
     **/
    QString getCurrentFileName()const;
    /// Returns \c unzFile handle.
    /** You can use this handle to directly call UNZIP part of the
     * ZIP/UNZIP package functions (see unzip.h).
     *
     * \warning When using the handle returned by this function, please
     * keep in mind that QuaZip class is unable to detect any changes
     * you make in the ZIP file state (e. g. changing current file, or
     * closing the handle). So please do not do anything with this
     * handle that is possible to do with the functions of this class.
     * Or at least return the handle in the original state before
     * calling some another function of this class (including implicit
     * destructor calls and calls from the QuaZipFile objects that refer
     * to this QuaZip instance!). So if you have changed the current
     * file in the ZIP archive - then change it back or you may
     * experience some strange behavior or even crashes.
     **/
    unzFile getUnzFile();
    /// Returns \c zipFile handle.
    /** You can use this handle to directly call ZIP part of the
     * ZIP/UNZIP package functions (see zip.h). Warnings about the
     * getUnzFile() function also apply to this function.
     **/
    zipFile getZipFile();
    /// Changes the data descriptor writing mode.
    /**
      According to the ZIP format specification, a file inside archive
      may have a data descriptor immediately following the file
      data. This is reflected by a special flag in the local file header
      and in the central directory. By default, QuaZIP sets this flag
      and writes the data descriptor unless both method and level were
      set to 0, in which case it operates in 1.0-compatible mode and
      never writes data descriptors.

      By setting this flag to false, it is possible to disable data
      descriptor writing, thus increasing compatibility with archive
      readers that don't understand this feature of the ZIP file format.

      Setting this flag affects all the QuaZipFile instances that are
      opened after this flag is set.

      The data descriptor writing mode is enabled by default.

      \param enabled If \c true, enable local descriptor writing,
      disable it otherwise.

      \sa QuaZipFile::setDataDescriptorWritingEnabled()
      */
    void setDataDescriptorWritingEnabled(bool enabled);
    /// Returns the data descriptor default writing mode.
    /**
      \sa setDataDescriptorWritingEnabled()
      */
    bool isDataDescriptorWritingEnabled() const;
    /// Returns a list of files inside the archive.
    /**
      \return A list of file names or an empty list if there
      was an error or if the archive is empty (call getZipError() to
      figure out which).
      \sa getFileInfoList()
      */
    QStringList getFileNameList() const;
    /// Returns information list about all files inside the archive.
    /**
      \return A list of QuaZipFileInfo objects or an empty list if there
      was an error or if the archive is empty (call getZipError() to
      figure out which).
      \sa getFileNameList()
      */
    QList<QuaZipFileInfo> getFileInfoList() const;
};

#endif
