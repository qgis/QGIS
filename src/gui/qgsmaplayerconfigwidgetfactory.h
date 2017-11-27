/***************************************************************************
    qgsmaplayerconfigwidgetfactoryfactory.h
     --------------------------------------
    Date                 : 9.7.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPLAYERCONFIGWIDGETFACTORY_H
#define QGSMAPLAYERCONFIGWIDGETFACTORY_H

#include <QListWidgetItem>
#include "qgis_gui.h"
#include "qgis_sip.h"

class QgsMapLayer;
class QgsMapLayerConfigWidget;
class QgsMapCanvas;

/**
 * \ingroup gui
 * \class QgsMapLayerConfigWidgetFactory
 * \since QGIS 2.16
 * Factory class for creating custom map layer property pages
 */
class GUI_EXPORT QgsMapLayerConfigWidgetFactory
{
  public:

    //! Constructor
    QgsMapLayerConfigWidgetFactory() = default;

    //! Constructor
    QgsMapLayerConfigWidgetFactory( const QString &title, const QIcon &icon );

    virtual ~QgsMapLayerConfigWidgetFactory() = default;

    /**
     * \brief The icon that will be shown in the UI for the panel.
     * \returns A QIcon for the panel icon.
     */
    virtual QIcon icon() const { return mIcon; }

    /**
     * Set the icon for the factory object.
     * \param icon The icon to show in the interface.
     */
    void setIcon( const QIcon &icon ) { mIcon = icon; }

    /**
     * \brief The title of the panel.
     * \note This may or may not be shown to the user.
     * \returns Title of the panel
     */
    virtual QString title() const { return mTitle; }

    /**
     * Set the title for the interface
     * \note Not all users may show this as a label
     * e.g style dock uses this as a tooltip.
     * \param title The title to set.
     */
    void setTitle( const QString &title ) { mTitle = title; }

    /**
     * Flag if widget is supported for use in style dock.
     * The default implementation returns false.
     * \returns True if supported
     */
    virtual bool supportsStyleDock() const { return false; }

    /**
     * Set support flag for style dock
     * \param supports True if this widget is supported in the style dock.
     */
    void setSupportsStyleDock( bool supports ) { mSupportsDock = supports; }

    /**
     * Flag if widget is supported for use in layer properties dialog.
     * The default implementation returns false.
     * \returns True if supported
     */
    virtual bool supportLayerPropertiesDialog() const { return false; }

    /**
     * Set support flag for style dock
     * \param supports True if this widget is supported in the style dock.
     */
    void setSupportLayerPropertiesDialog( bool supports ) { mSupportsProperties = supports; }

    /**
     * \brief Check if the layer is supported for this widget.
     * \returns True if this layer is supported for this widget
     */
    virtual bool supportsLayer( QgsMapLayer *layer ) const;

    /**
     * \brief Factory function to create the widget on demand as needed by the dock.
     * \note This function is called each time the panel is selected. Keep it light for better UX.
     * \param layer The active layer in the dock.
     * \param canvas The map canvas.
     * \param dockWidget True of the widget will be shown a dock style widget.
     * \param parent The parent of the widget.
     * \returns A new QgsMapStylePanel which is shown in the map style dock.
     */
    virtual QgsMapLayerConfigWidget *createWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, bool dockWidget = true, QWidget *parent SIP_TRANSFERTHIS = 0 ) const = 0 SIP_FACTORY;

  private:
    QIcon mIcon;
    QString mTitle;
    bool mSupportsDock = true;
    bool mSupportsProperties = true;
};

#endif // QGSMAPLAYERCONFIGWIDGETFACTORY_H
