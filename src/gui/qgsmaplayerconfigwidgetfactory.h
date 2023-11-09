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
class QgsLayerTreeGroup;

/**
 * \ingroup gui
 * \class QgsMapLayerConfigWidgetFactory
 * \brief Factory class for creating custom map layer property pages
 * \since QGIS 2.16
 */
class GUI_EXPORT QgsMapLayerConfigWidgetFactory
{
  public:

    /**
     * Available parent pages, for factories which create a widget which is a sub-component
     * of a standard page.
     *
     * \since QGIS 3.20
     */
    enum class ParentPage : int
    {
      NoParent, //!< Factory creates pages itself, not sub-components
      Temporal, //!< Factory creates sub-components of the temporal properties page (only supported for raster layer temporal properties)
    };

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
     * \returns Title of the panel
     * \note This may or may not be shown to the user.
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
     * The default implementation returns FALSE.
     * \returns TRUE if supported
     */
    virtual bool supportsStyleDock() const { return false; }

    /**
     * Set support flag for style dock
     * \param supports TRUE if this widget is supported in the style dock.
     */
    void setSupportsStyleDock( bool supports ) { mSupportsDock = supports; }

    /**
     * Flag if widget is supported for use in layer properties dialog.
     * The default implementation returns FALSE.
     * \returns TRUE if supported
     */
    virtual bool supportLayerPropertiesDialog() const { return false; }

    /**
     * Returns a tab name hinting at where this page should be inserted into the
     * layer properties tab list.
     *
     * If the returned string is non-empty, the config widget page will be inserted
     * before the existing page with matching object name.
     *
     * The default implementation returns an empty string, which causes the widget
     * to be placed at the end of the dialog page list.
     *
     * \since QGIS 3.14
     */
    virtual QString layerPropertiesPagePositionHint() const;

    /**
     * Set support flag for style dock
     * \param supports TRUE if this widget is supported in the style dock.
     */
    void setSupportLayerPropertiesDialog( bool supports ) { mSupportsProperties = supports; }

    /**
     * \brief Check if the layer is supported for this widget.
     * \returns TRUE if this layer is supported for this widget
     */
    virtual bool supportsLayer( QgsMapLayer *layer ) const;

    /**
     * \brief Check if a layer tree group is supported for this widget.
     * \returns TRUE if the group is supported for this widget
     * \since QGIS 3.24
     */
    virtual bool supportsLayerTreeGroup( QgsLayerTreeGroup *group ) const;

    /**
     * Returns the associated parent page, for factories which create sub-components of a standard page.
     *
     * The default implementation returns QgsMapLayerConfigWidgetFactory::ParentPage::NoParent, indicating that the
     * factory creates top-level pages which are not subcomponents.
     *
     * \since QGIS 3.20
     */
    virtual ParentPage parentPage() const;

    /**
     * \brief Factory function to create the widget on demand as needed by the dock.
     * \param layer The active layer in the dock.
     * \param canvas The map canvas.
     * \param dockWidget TRUE of the widget will be shown a dock style widget.
     * \param parent The parent of the widget.
     * \returns A new QgsMapStylePanel which is shown in the map style dock.
     * \note This function is called each time the panel is selected. Keep it light for better UX.
     */
    virtual QgsMapLayerConfigWidget *createWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, bool dockWidget = true, QWidget *parent = nullptr ) const = 0 SIP_FACTORY;

  private:
    QIcon mIcon;
    QString mTitle;
    bool mSupportsDock = true;
    bool mSupportsProperties = true;
};

#endif // QGSMAPLAYERCONFIGWIDGETFACTORY_H
