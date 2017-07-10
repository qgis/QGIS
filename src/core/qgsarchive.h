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
 * \brief Class allowing to manage the zip/unzip actions on project
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsArchive
{
  public:

    /**
     * Constructor for QgsArchive
     */
    QgsArchive();
    ~QgsArchive();

    bool zip( const QString &zipFilename );

    bool unzip( const QString &zipFilename );

    void clear();

    void addFile( const QString &filename );

    QString filename() const;

    QString projectFile() const;

    QStringList files() const;

    QString dir() const;

  private:
    // used when unzip is performed
    std::unique_ptr<QTemporaryDir> mDir;

    // content of the archive
    QStringList mFiles;

    // zip filename
    QString mFilename;
};

#endif
