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
#include "qgis_sip.h"
#include "qgis.h"
#include "qgstaskmanager.h"
#include <QString>

class QgsFeedback;

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
     * Given a \a filter string like "GeoTIFF Files (*.tiff *.tif)", extracts
     * the wildcard portion of this filter (i.e. "*.tiff *.tif").
     *
     * \since QGIS 3.18
     */
    static QString wildcardsFromFilter( const QString &filter );

    /**
     * Returns TRUE if the given \a fileName matches a file \a filter string.
     *
     * E.g a filter of "GeoTIFF Files (*.tiff *.tif)" would return TRUE for a \a fileName
     * of "/home/test.tif", or FALSE for "/home/test.jpg".
     *
     * \since QGIS 3.18
     */
    static bool fileMatchesFilter( const QString &fileName, const QString &filter );

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
     * \param feedback pointer to the feedback instance when available.
     * \returns List of strings of the first matching path in unix format.
     * \since QGIS 3.12
     */
    static QStringList findFile( const QString file, const QString basepath = QString(), int maxClimbs = 4, int searchCeiling = 4, const QString currentDir = QString(), QgsFeedback *feedback = nullptr );

    /**
     * Returns the drive type for the given \a path.
     *
     * \throws QgsNotSupportedException if determining the drive type is not supported on the current platform.
     *
     * \since QGIS 3.20
     */
    static Qgis::DriveType driveType( const QString &path ) SIP_THROW( QgsNotSupportedException );

    /**
     * Returns TRUE if the specified \a path is assumed to reside on a slow device, e.g. a remote
     * network drive or other non-fixed device.
     *
     * \since QGIS 3.20
     */
    static bool pathIsSlowDevice( const QString &path );

    /**
     * Returns a list of the sidecar files which exist for the dataset a the specified \a path.
     *
     * In this context a sidecar file is defined as a file which shares the same base filename
     * as a dataset, but which differs in file extension. It defines the list of additional
     * files which must be renamed or deleted alongside the main file associated with the
     * dataset in order to completely rename/delete the dataset.
     *
     * For instance, if \a path specified a .shp file then the corresponding .dbf, .idx
     * and .prj files would be returned (amongst others).
     *
     * \note QGIS metadata files (.qmd) and map layer styling files (.qml) are NOT included
     * in the returned list.
     *
     * \since QGIS 3.22
     */
    static QSet< QString > sidecarFilesForPath( const QString &path );

    /**
     * Renames the dataset at \a oldPath to \a newPath, renaming both the file at \a oldPath and
     * all associated sidecar files which exist for it.
     *
     * For instance, if \a oldPath specified a .shp file then the corresponding .dbf, .idx
     * and .prj files would be renamed (amongst others).
     *
     * The destination directory must already exist.
     *
     * The optional \a flags argument can be used to control whether QMD metadata files and
     * QML styling files should also be renamed accordingly. By default these will be renamed,
     * but manually specifying a different set of flags allows callers to avoid this when
     * desired.
     *
     * \param oldPath original path to dataset
     * \param newPath new path for dataset
     * \param error will be set to a descriptive error message if the rename operation fails
     * \param flags optional flags to control file operation behavior
     *
     * \returns TRUE if the dataset was successfully renamed, or FALSE if an error occurred
     *
     * \since QGIS 3.22
     */
    static bool renameDataset( const QString &oldPath, const QString &newPath, QString &error SIP_OUT, Qgis::FileOperationFlags flags = Qgis::FileOperationFlag::IncludeMetadataFile | Qgis::FileOperationFlag::IncludeStyleFile );

    /**
     * Returns the limit of simultaneously opened files by the process.
     *
     * Currently only implemented on Unix.
     *
     * \returns Limit, or -1 if unknown.
     * \note not available in Python bindings
     * \since QGIS 3.22
     */
    static int openedFileLimit() SIP_SKIP;

    /**
     * Returns the number of currently opened files by the process.
     *
     * Currently only implemented on Linux.
     *
     * \returns Number of files, or -1 if unknown
     * \note not available in Python bindings
     * \since QGIS 3.22
     */
    static int openedFileCount() SIP_SKIP;

    /**
     * Returns whether when opening new file(s) will reach, or nearly reach,
     * the limit of simultaneously opened files by the process.
     *
     * Currently only implemented on Linux.
     *
     * \param filesToBeOpened Number of files that will be opened.
     * \returns true if close to the limit, or false if not, or unknown.
     * \note not available in Python bindings
     * \since QGIS 3.22
     */
    static bool isCloseToLimitOfOpenedFiles( int filesToBeOpened = 1 ) SIP_SKIP;
};


/**
 * \ingroup core
 * \class QgsFileSearchTask
 * \brief Background thread handler for findFile.
 * \since QGIS 3.24
 */
class CORE_EXPORT QgsFileSearchTask: public QgsTask
{
    Q_OBJECT
  public:

    /**
     * Background task handler for findFile.
     * \param file Name or full path of the file to find
     * \param basePath current basepath of the file, needed if only the name is specified in file
     * \param maxClimbs limit the number of time the search can move up from the basepath
     * \param searchCeiling limits where in the folder hierarchy the search can be performed, 1 = root/drive, 2 = first folder level, 3 = sub folders ( Unix: /usr/bin, Win: C:/Users/Admin ), etc.
     * \param currentDir alternative default directory to override the actual current directory during the search
     * \since QGIS 3.24
     */
    QgsFileSearchTask( const QString file, const QString basePath, int maxClimbs, int searchCeiling, const QString currentDir );

    /**
     * Returns the result of the process.
     * \since QGIS 3.24
     */
    QStringList results();

  protected:

    bool run() override;

  private:
    const QString mFile;
    const QString mBasePath;
    const int mMaxClimbs;
    const int mSearchCeil;
    const QString mCurrentDir;
    QStringList mResults;
    std::unique_ptr< QgsFeedback > mFeedback;


};

#endif // QGSFILEUTILS_H
