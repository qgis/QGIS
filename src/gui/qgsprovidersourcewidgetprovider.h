/***************************************************************************
    qgsprovidersourcewidgetprovider.h
     --------------------------------------
    Date                 : December 2020
    Copyright            : (C) 2020 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
****************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROVIDERSOURCEWIDGETPROVIDER_H
#define QGSPROVIDERSOURCEWIDGETPROVIDER_H

#include "qgis.h"
#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgsguiutils.h"

class QWidget;
class QgsMapLayer;
class QgsProviderSourceWidget;

/**
 * \ingroup gui
 * \class QgsProviderSourceWidgetProvider
 *
 * \brief An interface for providers of widgets designed to configure a data provider's source.
 *
 * \since QGIS 3.18
 */
class GUI_EXPORT QgsProviderSourceWidgetProvider
{
  public:
    virtual ~QgsProviderSourceWidgetProvider();

    //! Provider key
    virtual QString providerKey() const = 0;

    /**
     * Source widget provider name, this is useful to retrieve
     * a particular source widget provider in case the provider has more
     * than one, it should be unique among all providers.
     *
     * The default implementation returns the providerKey()
     */
    virtual QString name() const { return providerKey(); }

    //! Returns TRUE if the provider can handle the specified \a layer.
    virtual bool canHandleLayer( QgsMapLayer *layer ) const = 0;

    /**
     * Creates a new widget to configure the source of the specified \a layer.
     * It may return NULLPTR if it cannot handle the layer.
     * The returned object must be destroyed by the caller.
     */
    virtual QgsProviderSourceWidget *createWidget( QgsMapLayer *layer, QWidget *parent SIP_TRANSFERTHIS = nullptr ) = 0 SIP_FACTORY;
};

#endif // QGSPROVIDERSOURCEWIDGETPROVIDER_H
