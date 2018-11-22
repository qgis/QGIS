/***************************************************************************
  qgsdataitemguiproviderregistry.h
  --------------------------------------
  Date                 : October 2018
  Copyright            : (C) 2018 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDATAITEMGUIPROVIDERREGISTRY_H
#define QGSDATAITEMGUIPROVIDERREGISTRY_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include <QList>

class QgsDataItemGuiProvider;

/**
 * \class QgsDataItemGuiProviderRegistry
 * \ingroup gui
 * This class keeps a list of data item GUI providers that may affect how QgsDataItems
 * behave within the application GUI.
 *
 * QgsDataItemGuiProviderRegistry is not usually directly created, but rather accessed through
 * QgsGui::instance()->dataItemGuiProviderRegistry().
 *
 * \since QGIS 3.6
 */
class GUI_EXPORT QgsDataItemGuiProviderRegistry
{
  public:

    QgsDataItemGuiProviderRegistry();

    ~QgsDataItemGuiProviderRegistry();

    //! QgsDataItemGuiProviderRegistry cannot be copied.
    QgsDataItemGuiProviderRegistry( const QgsDataItemGuiProviderRegistry &rh ) = delete;
    //! QgsDataItemGuiProviderRegistry cannot be copied.
    QgsDataItemGuiProviderRegistry &operator=( const QgsDataItemGuiProviderRegistry &rh ) = delete;

    /**
     * Returns the list of available providers.
     */
    QList<QgsDataItemGuiProvider *> providers() const { return mProviders; }

    /**
     * Adds a \a provider implementation to the registry. Ownership of the provider
     * is transferred to the registry.
     */
    void addProvider( QgsDataItemGuiProvider *provider SIP_TRANSFER );

    /**
     * Removes a \a provider implementation from the registry.
     * The provider object is automatically deleted.
     */
    void removeProvider( QgsDataItemGuiProvider *provider );

  private:
#ifdef SIP_RUN
    QgsDataItemGuiProviderRegistry( const QgsDataItemGuiProviderRegistry &rh );
#endif

    //! Available providers, owned by this class
    QList<QgsDataItemGuiProvider *> mProviders;

};

#endif // QGSDATAITEMGUIPROVIDERREGISTRY_H
