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
#include <QWidget>


class QgsMapLayer;

/** \ingroup gui
 * \class QgsLayerTreeEmbeddedWidgetProvider
 * Provider interface to be implemented in order to introduce new kinds of embedded widgets for use in layer tree.
 * Embedded widgets are assigned per individual map layers and they are shown before any legend entries.
 * @see QgsLayerTreeEmbeddedWidgetRegistry
 * @note introduced in QGIS 2.16
 */
class GUI_EXPORT QgsLayerTreeEmbeddedWidgetProvider
{
  public:
    virtual ~QgsLayerTreeEmbeddedWidgetProvider() {}

    //! Unique name of the provider (among other providers)
    virtual QString id() const = 0;

    //! Human readable name - may be translatable with tr()
    virtual QString name() const = 0;

    //! Factory to create widgets. The returned widget is owned by the caller.
    //! The widgetIndex argument may be used to identify which widget is being
    //! created (useful when using multiple widgets from the same provider for one layer).
    virtual QWidget* createWidget( QgsMapLayer* layer, int widgetIndex ) = 0;

    //! Whether it makes sense to use this widget for a particular layer
    virtual bool supportsLayer( QgsMapLayer* layer ) = 0;
};

/** \ingroup gui
 * \class QgsLayerTreeEmbeddedWidgetRegistry
 * Registry of widgets that may be embedded into layer tree view.
 * Embedded widgets are assigned per individual map layers and they are shown before any legend entries.
 * Layer tree must have UseEmbeddedWidgets flag enabled in order to show assigned widgets.
 *
 * @see QgsLayerTreeEmbeddedWidgetRegistry
 * @note introduced in QGIS 2.16
 */
class GUI_EXPORT QgsLayerTreeEmbeddedWidgetRegistry
{
  public:

    /** Means of accessing canonical single instance  */
    static QgsLayerTreeEmbeddedWidgetRegistry* instance();

    ~QgsLayerTreeEmbeddedWidgetRegistry();

    /** Return list of all registered providers */
    QStringList providers() const;

    /** Get provider object from the provider's ID */
    QgsLayerTreeEmbeddedWidgetProvider* provider( const QString& providerId ) const;

    /** Register a provider, takes ownership of the object.
     * Returns true on success, false if the provider is already registered. */
    bool addProvider( QgsLayerTreeEmbeddedWidgetProvider* provider );

    /** Unregister a provider, the provider object is deleted.
     * Returns true on success, false if the provider was not registered. */
    bool removeProvider( const QString& providerId );

  protected:
    //! Protected constructor - use instance() to access the registry.
    QgsLayerTreeEmbeddedWidgetRegistry();

    //! storage of all the providers
    QMap<QString, QgsLayerTreeEmbeddedWidgetProvider*> mProviders;
};


#endif // QGSLAYERTREEEMBEDDEDWIDGETREGISTRY_H
