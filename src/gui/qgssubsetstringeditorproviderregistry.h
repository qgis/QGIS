/***************************************************************************
    qgssubsetstringeditorproviderregistry.h
     --------------------------------------
    Date                 : 15-Nov-2020
    Copyright            : (C) 2020 by Even Rouault
    Email                : even.rouault at spatials.com
****************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSUBSETSTRINGEDITORPROVIDERREGISTRY_H
#define QGSSUBSETSTRINGEDITORPROVIDERREGISTRY_H

#include <QWidget>

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgsguiutils.h"

class QgsSubsetStringEditorInterface;
class QgsSubsetStringEditorProvider;
class QgsProviderGuiRegistry;
class QgsVectorLayer;

/**
 * \ingroup gui
 * \brief This class keeps a list of subset string editor providers.
 *
 * QgsSubsetStringEditorProviderRegistry is not usually directly created, but rather accessed through
 * QgsGui::subsetStringEditorProvideRegistry().
 *
 * \since QGIS 3.18
 */
class GUI_EXPORT QgsSubsetStringEditorProviderRegistry
{
  public:
    QgsSubsetStringEditorProviderRegistry();
    ~QgsSubsetStringEditorProviderRegistry();

    QgsSubsetStringEditorProviderRegistry( const QgsSubsetStringEditorProviderRegistry &rh ) = delete;
    QgsSubsetStringEditorProviderRegistry &operator=( const QgsSubsetStringEditorProviderRegistry &rh ) = delete;

    //! Gets list of available providers
    QList<QgsSubsetStringEditorProvider *> providers();

    //! Add a \a provider implementation. Takes ownership of the object.
    void addProvider( QgsSubsetStringEditorProvider *provider SIP_TRANSFER );

    /**
     * Remove \a provider implementation from the list (\a provider object is deleted)
     * \returns TRUE if the provider was actually removed and deleted
     */
    bool removeProvider( QgsSubsetStringEditorProvider *provider SIP_TRANSFER );

    /**
     * Initializes the registry. The registry needs to be passed explicitly
     * (instead of using singleton) because this gets called from QgsGui constructor.
     */
    void initializeFromProviderGuiRegistry( QgsProviderGuiRegistry *providerGuiRegistry );

    //! Returns a provider by \a name or NULLPTR if not found
    QgsSubsetStringEditorProvider *providerByName( const QString &name );

    //! Returns a (possibly empty) list of providers by data \a providerkey
    QList<QgsSubsetStringEditorProvider *> providersByKey( const QString &providerKey );

    /**
     * Creates a new dialog to edit the subset string of the provided \a layer.
     * It will default to returning a QgsQueryBuilder if no provider was found.
     * The returned object must be destroyed by the caller.
     */
    QgsSubsetStringEditorInterface *createDialog( QgsVectorLayer *layer, QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags ) SIP_TRANSFERBACK;

  private:
#ifdef SIP_RUN
    QgsSubsetStringEditorProviderRegistry( const QgsSubsetStringEditorProviderRegistry &rh );
#endif

    //! available providers. this class owns the pointers
    QList<QgsSubsetStringEditorProvider *> mProviders;
};

#endif // QGSSUBSETSTRINGEDITORPROVIDERREGISTRY_H
