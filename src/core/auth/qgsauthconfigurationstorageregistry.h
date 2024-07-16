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
#include "qgis_sip.h"

#include <QObject>
#include <QMap>
#include <QMutex>
#include <memory>

class QgsAuthConfigurationStorage;


/**
 * \ingroup core
 * \brief Registry for authentication configuration storages.
 *
 * This class manages a list of authentication configuration storages.
 * It is a singleton class.
 * \since QGIS 3.40
 */
class CORE_EXPORT QgsAuthConfigurationStorageRegistry: public QObject
{
    Q_OBJECT
  public:

    /**
     * Creates a new QgsAuthConfigurationStorageRegistry instance.
     */
    QgsAuthConfigurationStorageRegistry();

    virtual ~QgsAuthConfigurationStorageRegistry();

    /**
     * Add an authentication configuration storage to the registry.
     * The registry takes ownership of the storage object.
     * \param storage The storage to add
     * \returns TRUE if the storage was added, FALSE if it was already present in the registry.
     * \note The storage object must have a unique id.
     * \note This method must be called from the same thread the registry was created in.
     */
    bool addStorage( QgsAuthConfigurationStorage *storage SIP_TRANSFER );

    /**
     * Remove the authentication configuration storage identified by \a id from the registry.
     * \returns TRUE if the storage was removed, FALSE if it was not present in the registry.
     * \note This method must be called from the same thread the registry was created in.
     */
    bool removeStorage( const QString &id );

    /**
     * Returns the list of all registered authentication configuration storages.
     * \note This method must be called from the same thread the registry was created in.
     */
    QList<QgsAuthConfigurationStorage *> storages() const;

    /**
     * Returns the list of all ready (and enabled) authentication configuration storage.
     * \note This method must be called from the same thread the registry was created in.
     */
    QList<QgsAuthConfigurationStorage *> readyStorages() const;

    /**
     * Returns the storage with the specified \a id or NULL if not found in the registry.
     * \param id The id of the storage to retrieve
     * \note This method must be called from the same thread the registry was created in.
     */
    QgsAuthConfigurationStorage *storage( const QString &id ) const;

    /**
     * Order the storages by the specified \a orderIds.
     * \param orderIds The ordered list of storage Ids to apply, storages not in the list will be appended at the end.
     * \note This method must be called from the same thread the registry was created in.
     */
    void orderStorages( const QStringList &orderIds );


  signals:

    /**
     * Emitted after a storage was added
     * \param id The id of the added storage
     */
    void storageAdded( const QString &id );

    /**
     * Emitted after a storage was changed
     * \param id The id of the changed storage
     */
    void storageChanged( const QString &id );

    /**
     * Emitted after a storage was removed
     * \param id The id of the removed storage
     */
    void storageRemoved( const QString &id );

  private:

    mutable QMutex mMutex;

    std::vector<std::unique_ptr<QgsAuthConfigurationStorage>> mStorages;
};

#endif // QGSAUTHCONFIGURATIONSTORAGEREGISTRY_H
