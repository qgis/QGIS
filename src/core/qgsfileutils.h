/***************************************************************************
                             qgsfileutils.h
                             ---------------------------
    begin                : November 2017
    copyright            : (C) 2017 by Etienne Trimaille
    email                : etienne dot trimaille at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFILEUTILS_H
#define QGSFILEUTILS_H

#include "qgis_core.h"
#include <QString>

/**
 * \ingroup core
 * \class QgsFileUtils
 * \brief Class for file utilities.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsFileUtils
{
  public:

    /**
     * Returns the human size from bytes
     */
    static QString representFileSize( qint64 bytes );


    /**
     * Returns a list of the extensions contained within a file \a filter string.
     * E.g. a \a filter of "GeoTIFF Files (*.tiff *.tif)" would return a list
     * containing "tiff", "tif". The initial '.' is stripped off the extension.
     * \see ensureFileNameHasExtension()
     * \see addExtensionFromFilter()
     */
    static QStringList extensionsFromFilter( const QString &filter );

    /**
     * Ensures that a \a fileName ends with an extension from the provided list of
     * \a extensions.
     *
     * E.g. if extensions contains "tif" and "tiff", then a \a fileName of
     * "d:/my_file" will return "d:/my_file.tif". A \a fileName of
     * "d:/my_file.TIFF" or "d:/my_file.TIF" will be returned unchanged.
     *
     * \see extensionsFromFilter()
     * \see addExtensionFromFilter()
     */
    static QString ensureFileNameHasExtension( const QString &fileName, const QStringList &extensions );

    /**
     * Ensures that a \a fileName ends with an extension from the specified \a filter
     * string.
     *
     * E.g. a \a fileName of "d:/my_file" with a filter of "GeoTIFF Files (*.tiff *.tif)"
     * will return "d:/my_file.tif", where as \a fileName of "d:/my_file.TIFF" or "d:/my_file.TIF"
     * will be returned unchanged.
     *
     * \see extensionsFromFilter()
     * \see ensureFileNameHasExtension()
     */
    static QString addExtensionFromFilter( const QString &fileName, const QString &filter );

    /**
     * Converts a \a string to a safe filename, replacing characters which are not safe
     * for filenames with an '_' character.
     *
     * \warning This method strips slashes from the filename, so it is safe to call with file names only, not complete paths.
     */
    static QString stringToSafeFilename( const QString &string );

    /**
     * Returns the top-most existing folder from \a path. E.g. if \a path is "/home/user/projects/2018/P4343"
     * and "/home/user/projects" exists but no "2018" subfolder exists, then the function will return "/home/user/projects".
     *
     * \since QGIS 3.2
     */
    static QString findClosestExistingPath( const QString &path );

    /**
     * Will check \a basepath in an outward spiral up to \a maxClimbs levels to check if \a file exists.
     * \param file Name or full path of the file to find
     * \param basepath current basepath of the file, needed if only the name is specified in file
     * \param maxClimbs limit the number of time the search can move up from the basepath
     * \param searchCeiling limits where in the folder hierarchy the search can be performed, 1 = root/drive, 2 = first folder level, 3 = sub folders ( Unix: /usr/bin, Win: C:/Users/Admin ), etc.
     * \param currentDir alternative default directory to override the actual current directory during the search
     * \returns List of strings of the first matching path in unix format.
     * \since QGIS 3.12
     */
    static QStringList findFile( const QString &file, const QString &basepath = QString(), int maxClimbs = 4, int searchCeiling = 4, const QString &currentDir = QString() );

};

#endif // QGSFILEUTILS_H
