/***************************************************************************
                            qgsarchive.h
                          ----------------

    begin                : July 07, 2017
    copyright            : (C) 2017 by Paul Blottiere
    email                : paul.blottiere@oslandia.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSARCHIVE_H
#define QGSARCHIVE_H

#include "qgis_core.h"

#include <QStringList>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include <memory>

/**
 * \class QgsArchive
 * \ingroup core
 * \brief Class allowing to manage the zip/unzip actions
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsArchive
{
  public:

    /**
     * Constructor
     */
    QgsArchive();

    /**
     * Copy constructor
     */
    QgsArchive( const QgsArchive &other );

    QgsArchive &operator=( const QgsArchive &other );

    /**
     * Destructor
     */
    virtual ~QgsArchive() = default;

    /**
     * Zip the content of this archive
     * \param zipFilename The name of the zip to generate
     * \returns FALSE if something goes wrong, TRUE otherwise
     */
    bool zip( const QString &zipFilename );

    /**
     * Clear the current content of this archive and unzip. Files are unzipped
     * in the temporary directory.
     * \param zipFilename The zip file to unzip
     * \returns TRUE if unzip action is a success, FALSE otherwise
     */
    virtual bool unzip( const QString &zipFilename );

    /**
     * Clear the current content of this archive and create a new temporary
     * directory.
     */
    void clear();

    /**
     * Add a new file to this archive. During a zip action, this file will be
     * part of the resulting zipped file.
     * \param filename A file to add when zipping this archive
     */
    void addFile( const QString &filename );

    /**
     * Remove a file from this archive and from the filesystem.
     * \param filename The path of the file to remove
     * \returns TRUE if the file has been removed from the filesystem, FALSE otherwise
     */
    bool removeFile( const QString &filename );

    /**
     * Returns the list of files within this archive
     */
    QStringList files() const;

    /**
     * Returns the current temporary directory.
     */
    QString dir() const;

  private:
    // content of the archive
    QStringList mFiles;

    // used when unzip is performed
    std::unique_ptr<QTemporaryDir> mDir;
};

/**
 * \class QgsProjectArchive
 * \ingroup core
 * \brief Class allowing to manage the zip/unzip actions on project file
 * \since QGIS 3.0
 */
class  CORE_EXPORT QgsProjectArchive : public QgsArchive
{
  public:

    /**
     * Clear the current content of this archive and unzip. If a project file
     * is found in the content, then this archive may be considered as a valid
     * one. Files are unzipped in the temporary directory.
     * \param zipFilename The zip file to unzip
     * \returns TRUE if a project file has been found, FALSE otherwise
     */
    bool unzip( const QString &zipFilename ) override;

    /**
     * Returns the current .qgs project file or an empty string if there's none
     */
    QString projectFile() const;

    /**
     * Remove the current .qgs project file from the temporary directory.
     * \returns TRUE if the file is well removed, FALSE otherwise
     */
    bool clearProjectFile();

    /**
     * Returns the current .qgd auxiliary storage file or an empty string if
     * there's none
     */
    QString auxiliaryStorageFile() const;
};

#endif
