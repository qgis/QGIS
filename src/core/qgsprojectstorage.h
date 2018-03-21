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

class QIODevice;
class QString;
class QStringList;

class QgsReadWriteContext;

/**
 * Abstract interface for project storage - to be implemented by various backends
 * and registered in QgsProjectStorageRegistry.
 *
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProjectStorage
{
  public:
    virtual ~QgsProjectStorage();

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

    // TODO: rename / remove ?

    // TODO: load/save GUI
};

#endif // QGSPROJECTSTORAGE_H
