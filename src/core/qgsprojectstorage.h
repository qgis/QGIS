/***************************************************************************
  qgsprojectstorage.h
  --------------------------------------
  Date                 : March 2018
  Copyright            : (C) 2018 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROJECTSTORAGE_H
#define QGSPROJECTSTORAGE_H

#include "qgis_core.h"
#include "qgis_sip.h"

#include <QDateTime>
#include <QString>

class QIODevice;
class QStringList;

class QgsReadWriteContext;

/**
 * \ingroup core
 * Abstract interface for project storage - to be implemented by various backends
 * and registered in QgsProjectStorageRegistry.
 *
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProjectStorage
{
  public:

    /**
     * \ingroup core
     * Metadata associated with a project
     * \since QGIS 3.2
     */
    class Metadata
    {
      public:
        //! Name of the project - equivalent to a file's base name (i.e. without path and extension).
        QString name;
        //! Date and local time when the file was last modified.
        QDateTime lastModified;
    };

    virtual ~QgsProjectStorage() = default;

    /**
     * Unique identifier of the project storage type. If type() returns "memory", all project file names
     * starting with "memory:" will have read/write redirected through that storage implementation.
     */
    virtual QString type() = 0;

    //! Returns list of all projects for given URI (specific to each storage backend)
    virtual QStringList listProjects( const QString &uri ) = 0;

    /**
     * Reads project file content stored in the backend at the specified URI to the given device
     * (could be e.g. a temporary file or a memory buffer). The device is expected to be empty
     * when passed to readProject() so that the method can write all data to it and then rewind
     * it using seek(0) to make it ready for reading in QgsProject.
     */
    virtual bool readProject( const QString &uri, QIODevice *device, QgsReadWriteContext &context ) = 0;

    /**
     * Writes project file content stored in given device (could be e.g. a temporary file or a memory buffer)
     * using the backend to the specified URI. The device is expected to contain all project file data
     * and having position at the start of the content when passed to writeProject() so that the method
     * can read all data from it until it reaches its end.
     */
    virtual bool writeProject( const QString &uri, QIODevice *device, QgsReadWriteContext &context ) = 0;

    /**
     * Removes an existing project at the given URI. Returns TRUE if the removal
     * was successful.
     */
    virtual bool removeProject( const QString &uri ) = 0;

    /**
     * Rename an existing project at the given URI to a different URI. Returns TRUE if renaming
     * was successful.
     */
    virtual bool renameProject( const QString &uri, const QString &uriNew ) { Q_UNUSED( uri ); Q_UNUSED( uriNew ); return false; }

    /**
     * Reads project metadata (e.g. last modified time) if this is supported by the storage implementation.
     * Returns TRUE if the metadata were read with success.
     */
    virtual bool readProjectStorageMetadata( const QString &uri, QgsProjectStorage::Metadata &metadata SIP_OUT ) { Q_UNUSED( uri ); Q_UNUSED( metadata ); return false; }

    /**
     * Returns human-readable name of the storage. Used as the menu item text in QGIS. Empty name
     * indicates that the storage does not implement GUI support (showLoadGui() and showSaveGui()).
     * The name may be translatable and ideally unique as well.
     */
    virtual QString visibleName() { return QString(); }

    /**
     * Opens GUI to allow user to select a project to be loaded (GUI specific to this storage type).
     * Returns project URI if user has picked a project or empty string if the GUI was canceled.
     */
    virtual QString showLoadGui() { return QString(); }

    /**
     * Opens GUI to allow user to select where a project should be saved (GUI specific to this storage type).
     * Returns project URI if user has picked a destination or empty string if the GUI was canceled.
     */
    virtual QString showSaveGui() { return QString(); }
};

#endif // QGSPROJECTSTORAGE_H
