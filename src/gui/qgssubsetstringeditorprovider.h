/***************************************************************************
    qgssubsetstringeditorprovider.h
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

#ifndef QGSSUBSETSTRINGEDITORPROVIDER_H
#define QGSSUBSETSTRINGEDITORPROVIDER_H

#include "qgis.h"
#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgsguiutils.h"

class QWidget;
class QgsVectorLayer;
class QgsSubsetStringEditorInterface;

/**
 * \ingroup gui
 * \class QgsSubsetStringEditorProvider
 * \brief This is the interface for those who want to provide a dialog to edit a
 * subset string.
 *
 * \since QGIS 3.18
 */
class GUI_EXPORT QgsSubsetStringEditorProvider
{
  public:
    virtual ~QgsSubsetStringEditorProvider();

    //! Provider key
    virtual QString providerKey() const = 0;

    /**
     * Subset string editor provider name, this is useful to retrieve
     * a particular subset string editor in case the provider has more
     * than one, it should be unique among all providers.
     *
     * The default implementation returns the providerKey()
     */
    virtual QString name() const { return providerKey(); }

    //! Returns true if the provider can handle the layer
    virtual bool canHandleLayer( QgsVectorLayer *layer ) const = 0;

    /**
     * Returns true if the provider can handle specifically the
     * layer->provider()->storageType()
     * This method will only be called if canHandleLayer() returned true.
     * Typically a generic SQL provider for the OGR provider will return false,
     * whereas a dedicated plugin with a specific behavior for a OGR driver
     * will return true.
     */
    virtual bool canHandleLayerStorageType( QgsVectorLayer *layer ) const
    {
      Q_UNUSED( layer );
      return false;
    }

    /**
     * Creates a new dialog to edit the subset string of the provided \a layer.
     * It may return nullptr if it cannot handle the layer.
     * The returned object must be destroyed by the caller.
     * On successful accept(), the QgsSubsetStringEditorInterface implementation
     * is responsible for setting the updated string on layer.
     */
    virtual QgsSubsetStringEditorInterface *createDialog( QgsVectorLayer *layer, QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags ) = 0 SIP_FACTORY;
};

#endif
