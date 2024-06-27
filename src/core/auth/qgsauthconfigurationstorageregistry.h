/***************************************************************************
  qgsauthconfigurationstorageregistry.h - QgsAuthConfigurationStorageRegistry

 ---------------------
 begin                : 20.6.2024
 copyright            : (C) 2024 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSAUTHCONFIGURATIONSTORAGEREGISTRY_H
#define QGSAUTHCONFIGURATIONSTORAGEREGISTRY_H

#include "qgis_core.h"

#include <QObject>

class QgsAuthConfigurationStorage;


/**
 * \ingroup core
 * \brief Registry for authentication configuration storages.
 *
 * This class manages a list of authentication configuration storages.
 * It is a singleton class.
 */
class CORE_EXPORT QgsAuthConfigurationStorageRegistry: public QObject
{
    Q_OBJECT
  public:

    //! Means of accessing canonical single instance
    static QgsAuthConfigurationStorageRegistry *instance( );


    QgsAuthConfigurationStorageRegistry();

    virtual ~QgsAuthConfigurationStorageRegistry();

    /**
     * Add an authentication configuration storage to the registry.
     * The registry takes ownership of the storage object.
     * \param storage The storage to add
     * \param after The storage after which to add the new storage. If nullptr, the storage is added at the end.
     * \returns TRUE if the storage was added, FALSE if it was already present in the registry.
     */
    bool addStorage( QgsAuthConfigurationStorage *storage, QgsAuthConfigurationStorage *after = nullptr );

    /**
     * Returns a list of all registered authentication configuration storages.
     */
    QList<QgsAuthConfigurationStorage *> storages() const;

    /**
     * Returns a list of all ready (and enabled) authentication configuration storage.
     */
    QList<QgsAuthConfigurationStorage *> readyStorages() const;


  signals:

    /**
     * Emitted when a storage was added
     */
    void storageAdded( QgsAuthConfigurationStorage *storage );

    /**
     * Emitted when a storage was changed
     */
    void storageChanged( QgsAuthConfigurationStorage *storage );

    /**
     * Emitted when a storage was removed
     * \todo removal is not implemented yet
     */
    void storageRemoved( QgsAuthConfigurationStorage *storage );

  private:

    QList<QgsAuthConfigurationStorage *> mStorages;
};

#endif // QGSAUTHCONFIGURATIONSTORAGEREGISTRY_H
