/***************************************************************************
                                  qgsziputils.h
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

#ifndef QGSZIPUTILS_H
#define QGSZIPUTILS_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include <QStringList>

/**
 * \ingroup core
 * \brief Provides utility functions for working with zip files.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsZipUtils
{
  public:

    /**
     * Returns TRUE if the file name is a zipped file ( i.e with a '.qgz'
     *  extension, FALSE otherwise.
     * \param filename The name of the file
     * \returns TRUE if the file is zipped, FALSE otherwise
     */
    static bool isZipFile( const QString &filename );

    /**
     * Unzip a zip file in an output directory.
     * \param zip The zip filename
     * \param dir The output directory
     * \param files The absolute path of unzipped files
     * \param checkConsistency Perform additional stricter consistency checks on the archive, and error if they fail (since QGIS 3.30)
     * \returns FALSE if the zip filename does not exist, the output directory does not exist or is not writable.
     */
    static bool unzip( const QString &zip, const QString &dir, QStringList &files SIP_OUT, bool checkConsistency = true );

    /**
     * Zip the list of files in the zip file.
     *
     * If the zip file already exists (and \a overwrite is FALSE) or is empty, an error is returned.
     * If an input file does not exist, an error is also returned.
     * \param zip The zip filename
     * \param files The absolute path to files to embed within the zip
     * \param overwrite Set to TRUE to allow overwriting existing files (since QGIS 3.44)
     */
    static bool zip( const QString &zip, const QStringList &files, bool overwrite = false );

    /**
     * Decodes gzip byte stream, returns TRUE on success. Useful for reading vector tiles.
     *
     * \note Not available in Python bindings
     * \since QGIS 3.26
     */
    static bool decodeGzip( const QByteArray &bytesIn, QByteArray &bytesOut ) SIP_SKIP;

    /**
     * Decodes gzip byte stream, returns TRUE on success. Useful for reading vector tiles.
     *
     * \note Not available in Python bindings
     * \since QGIS 3.26
     */
    static bool decodeGzip( const char *bytesIn, std::size_t size, QByteArray &bytesOut ) SIP_SKIP;

    /**
     * Encodes gzip byte stream, returns TRUE on success. Useful for writing vector tiles.
     *
     * \note Not available in Python bindings
     * \since QGIS 3.26
     */
    static bool encodeGzip( const QByteArray &bytesIn, QByteArray &bytesOut ) SIP_SKIP;

    /**
     * Returns the list of files within a \a zip file
     *
     * \since QGIS 3.30
     */
    static const QStringList files( const QString &zip );

};

#endif //QGSZIPUTILS_H
