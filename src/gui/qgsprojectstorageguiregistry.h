/***************************************************************************
   qgsprojectstorageguiregistry.h
   -------------------------------
    begin                : June 2019
    copyright            : (C) 2019 by Peter Petrik
    email                : zilolv at google dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROJECTSTORAGEGUIREGISTRY_H
#define QGSPROJECTSTORAGEGUIREGISTRY_H

#include <map>

#include <QString>
#include <QList>
#include <QHash>

#include "qgis_gui.h"
#include "qgis_sip.h"

class QgsProjectStorageGuiProvider;
class QgsProviderGuiRegistry;

/**
 * \ingroup gui
 * A registry / canonical manager of GUI parts of project storage backends.
 *
 * QgsProjectStorageGuiRegistry is not usually directly created, but rather accessed through
 * QgsGui::projectStorageGuiRegistry().
 *
 * \see QgsProjectStorageRegistry
 *
 * \since QGIS 3.10
*/
class GUI_EXPORT QgsProjectStorageGuiRegistry
{
  public:
    QgsProjectStorageGuiRegistry();
    ~QgsProjectStorageGuiRegistry();

    //! QgsProjectStorageGuiRegistry cannot be copied.
    QgsProjectStorageGuiRegistry( const QgsProjectStorageGuiRegistry &rh ) = delete;
    //! QgsProjectStorageGuiRegistry cannot be copied.
    QgsProjectStorageGuiRegistry &operator=( const QgsProjectStorageGuiRegistry &rh ) = delete;

    //! Returns storage implementation if the storage type matches one. Returns NULLPTR otherwise (it is a normal file)
    QgsProjectStorageGuiProvider *projectStorageFromType( const QString &type );

    //! Returns storage implementation if the URI matches one. Returns NULLPTR otherwise (it is a normal file)
    QgsProjectStorageGuiProvider *projectStorageFromUri( const QString &uri );

    //! Returns a list of registered project storage implementations
    QList<QgsProjectStorageGuiProvider *> projectStorages() const;

    //! Registers a storage backend and takes ownership of it
    void registerProjectStorage( QgsProjectStorageGuiProvider *storage SIP_TRANSFER );

    //! Unregisters a storage backend and destroys its instance
    void unregisterProjectStorage( QgsProjectStorageGuiProvider *storage );

    /**
     * Initializes the registry. The registry needs to be passed explicitly
     * (instead of using singleton) because this gets called from QgsGui constructor.
     */
    void initializeFromProviderGuiRegistry( QgsProviderGuiRegistry *providerGuiRegistry );

  private:
#ifdef SIP_RUN
    QgsProjectStorageGuiRegistry( const QgsProjectStorageGuiRegistry &rh );
#endif

    QHash<QString, QgsProjectStorageGuiProvider *> mBackends;
};

#endif // QGSPROJECTSTORAGEGUIREGISTRY_H
