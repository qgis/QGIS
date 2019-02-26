/***************************************************************************
  qgsprojectstorageregistry.h
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

#ifndef QGSPROJECTSTORAGEREGISTRY_H
#define QGSPROJECTSTORAGEREGISTRY_H

#include "qgis_core.h"
#include "qgis_sip.h"

#include <QHash>

class QgsProjectStorage;

/**
 * \ingroup core
 * Registry of storage backends that QgsProject may use.
 * This is a singleton that should be accessed through QgsApplication::projectStorageRegistry().
 *
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProjectStorageRegistry
{
  public:
    ~QgsProjectStorageRegistry();

    //! Returns storage implementation if the storage type matches one. Returns NULLPTR otherwise (it is a normal file)
    QgsProjectStorage *projectStorageFromType( const QString &type );

    //! Returns storage implementation if the URI matches one. Returns NULLPTR otherwise (it is a normal file)
    QgsProjectStorage *projectStorageFromUri( const QString &uri );

    //! Returns a list of registered project storage implementations
    QList<QgsProjectStorage *> projectStorages() const;

    //! Registers a storage backend and takes ownership of it
    void registerProjectStorage( QgsProjectStorage *storage SIP_TRANSFER );

    //! Unregisters a storage backend and destroys its instance
    void unregisterProjectStorage( QgsProjectStorage *storage );

  private:
    QHash<QString, QgsProjectStorage *> mBackends;
};

#endif // QGSPROJECTSTORAGEREGISTRY_H
