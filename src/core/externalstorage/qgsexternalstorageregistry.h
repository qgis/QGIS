/***************************************************************************
  qgsexternalstorageregistry.h
  --------------------------------------
  Date                 : March 2021
  Copyright            : (C) 2021 by Julien Cabieces
  Email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSEXTERNALSTORAGEREGISTRY_H
#define QGSEXTERNALSTORAGEREGISTRY_H

#include "qgis_core.h"
#include "qgis_sip.h"

#include <QList>
#include <QHash>

class QgsExternalStorage;

/**
 * \ingroup core
 * \brief Registry of external storage backends used by QgsExternalResourceWidget
 *
 * QgsExternalStorageRegistry is not usually directly created, but rather accessed through
 * QgsApplication::projectStorageRegistry().
 *
 * \since QGIS 3.22
 */
class CORE_EXPORT QgsExternalStorageRegistry
{
  public:

    /**
     * Constructor - creates a registry of external storage backends
     */
    QgsExternalStorageRegistry();

    /**
     * Destructor
     */
    ~QgsExternalStorageRegistry();

    /**
     * Returns external storage implementation if the storage \a type matches one.
     * Returns nullptr otherwise
     */
    QgsExternalStorage *externalStorageFromType( const QString &type ) const;

    /**
     * Returns a list of registered project storage implementations
     */
    QList<QgsExternalStorage *> externalStorages() const;

    /**
     * Registers a \a storage backend and takes ownership of it
     */
    void registerExternalStorage( QgsExternalStorage *storage SIP_TRANSFER );

    /**
     * Unregisters a \a storage backend and destroys its instance
     */
    void unregisterExternalStorage( QgsExternalStorage *storage );

  private:
    QList< QgsExternalStorage * > mBackends;
};

#endif // QGSEXTERNALSTORAGEREGISTRY_H
