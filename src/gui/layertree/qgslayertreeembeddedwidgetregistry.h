/***************************************************************************
  qgslayertreeembeddedwidgetregistry.h
  --------------------------------------
  Date                 : May 2016
  Copyright            : (C) 2016 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYERTREEEMBEDDEDWIDGETREGISTRY_H
#define QGSLAYERTREEEMBEDDEDWIDGETREGISTRY_H

#include <QMap>
#include "qgis.h"
#include <QWidget>
#include "qgis_gui.h"


class QgsMapLayer;

/**
 * \ingroup gui
 * \class QgsLayerTreeEmbeddedWidgetProvider
 * \brief Provider interface to be implemented in order to introduce new kinds of embedded widgets for use in layer tree.
 * Embedded widgets are assigned per individual map layers and they are shown before any legend entries.
 * \see QgsLayerTreeEmbeddedWidgetRegistry
 */
class GUI_EXPORT QgsLayerTreeEmbeddedWidgetProvider
{
  public:
    virtual ~QgsLayerTreeEmbeddedWidgetProvider() = default;

    //! Unique name of the provider (among other providers)
    virtual QString id() const = 0;

    //! Human readable name - may be translatable with tr()
    virtual QString name() const = 0;

    /**
     * Factory to create widgets. The returned widget is owned by the caller.
     * The widgetIndex argument may be used to identify which widget is being
     * created (useful when using multiple widgets from the same provider for one layer).
     */
    virtual QWidget *createWidget( QgsMapLayer *layer, int widgetIndex ) = 0 SIP_FACTORY;

    //! Whether it makes sense to use this widget for a particular layer
    virtual bool supportsLayer( QgsMapLayer *layer ) = 0;
};

/**
 * \ingroup gui
 * \class QgsLayerTreeEmbeddedWidgetRegistry
 * \brief Registry of widgets that may be embedded into layer tree view.
 * Embedded widgets are assigned per individual map layers and they are shown before any legend entries.
 * Layer tree must have UseEmbeddedWidgets flag enabled in order to show assigned widgets.
 *
 * QgsLayerTreeEmbeddedWidgetRegistry is not usually directly created, but rather accessed through
 * QgsGui::layerTreeEmbeddedWidgetRegistry().
 *
 * \see QgsLayerTreeEmbeddedWidgetRegistry
 */
class GUI_EXPORT QgsLayerTreeEmbeddedWidgetRegistry
{
  public:
    /**
     * Constructor for QgsLayerTreeEmbeddedWidgetRegistry/
     *
     * QgsLayerTreeEmbeddedWidgetRegistry is not usually directly created, but rather accessed through
     * QgsGui::layerTreeEmbeddedWidgetRegistry().
     */
    QgsLayerTreeEmbeddedWidgetRegistry();

    ~QgsLayerTreeEmbeddedWidgetRegistry();

    QgsLayerTreeEmbeddedWidgetRegistry( const QgsLayerTreeEmbeddedWidgetRegistry &other ) = delete;
    QgsLayerTreeEmbeddedWidgetRegistry &operator=( const QgsLayerTreeEmbeddedWidgetRegistry &other ) = delete;

    //! Returns list of all registered providers
    QStringList providers() const;

    //! Gets provider object from the provider's ID
    QgsLayerTreeEmbeddedWidgetProvider *provider( const QString &providerId ) const;

    /**
     * Register a provider, takes ownership of the object.
     * Returns TRUE on success, FALSE if the provider is already registered.
    */
    bool addProvider( QgsLayerTreeEmbeddedWidgetProvider *provider SIP_TRANSFER );

    /**
     * Unregister a provider, the provider object is deleted.
     * Returns TRUE on success, FALSE if the provider was not registered.
    */
    bool removeProvider( const QString &providerId );

  protected:
    //! storage of all the providers
    QMap<QString, QgsLayerTreeEmbeddedWidgetProvider *> mProviders;

  private:
#ifdef SIP_RUN
    QgsLayerTreeEmbeddedWidgetRegistry( const QgsLayerTreeEmbeddedWidgetRegistry &other );
#endif
};


#endif // QGSLAYERTREEEMBEDDEDWIDGETREGISTRY_H
