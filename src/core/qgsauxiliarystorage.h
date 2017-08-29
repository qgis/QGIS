/***************************************************************************
                          qgsauxiliarystorage.h  -  description
                           -------------------
    begin                : Aug 28, 2017
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

#ifndef QGSAUXILIARYSTORAGE_H
#define QGSAUXILIARYSTORAGE_H

#include "qgis_core.h"
#include "qgsdatasourceuri.h"

#include <sqlite3.h>

#include <QString>

class QgsProject;

/**
 * \class QgsAuxiliaryStorage
 *
 * \ingroup core
 *
 * \brief Class providing some utility methods to manage auxiliary storage.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsAuxiliaryStorage
{
  public:

    /**
     * Constructor.
     *
     * \param project The project for which the auxiliary storage has to be used
     * \param copy Parameter indicating if a copy of the database has to be used
     */
    QgsAuxiliaryStorage( const QgsProject &project, bool copy = true );

    /**
     * Constructor.
     *
     * \param filename The path of the database
     * \param copy Parameter indicating if a copy of the database has to be used
     */
    QgsAuxiliaryStorage( const QString &filename = QString(), bool copy = true );

    /**
     * Destructor.
     */
    virtual ~QgsAuxiliaryStorage();

    /**
     * Returns the status of the auxiliary storage currently definied.
     *
     * \returns true if the auxiliary storage is valid, false otherwise
     */
    bool isValid() const;

    /**
     * Returns the target filename of the database.
     */
    QString fileName() const;

    /**
     * Returns the path of current database used. It may be different from the
     * target filename if the auxiliary storage is opened in copy mode.
     */
    QString currentFileName() const;

    /**
     * Saves the current database to a new path.
     *
     * \returns true if everything is saved, false otherwise
     */
    bool saveAs( const QString &filename ) const;

    /**
     * Saves the current database to a new path for a specific project.
     * Actually, the current filename of the project is used to deduce the
     * path of the database to save.
     *
     * \returns true if everything is saved, false otherwise
     */
    bool saveAs( const QgsProject &project ) const;

    /**
     * Saves the current database.
     *
     * \returns true if everything is saved, false otherwise
     */
    bool save() const;

    /**
     * Returns the extension used for auxiliary databases.
     */
    static QString extension();

  private:
    sqlite3 *open( const QString &filename = QString() );
    sqlite3 *open( const QgsProject &project );

    void initTmpFileName();

    static QString filenameForProject( const QgsProject &project );
    static sqlite3 *createDB( const QString &filename );
    static sqlite3 *openDB( const QString &filename );
    static void close( sqlite3 *handler );

    static bool exec( const QString &sql, sqlite3 *handler );
    static void debugMsg( const QString &sql, sqlite3 *handler );

    bool mValid;
    QString mFileName; // original filename
    QString mTmpFileName; // temporary filename used in copy mode
    bool mCopy;
};

#endif
