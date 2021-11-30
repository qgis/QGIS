/***************************************************************************
    qgsmaplayerconfigwidget.h
    -------------------------
    begin                : June 2016
    copyright            : (C) 2016 by Nathan Woodrow
    email                : woodrow dot nathan at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSMAPLAYERCONFIGWIDGET_H
#define QGSMAPLAYERCONFIGWIDGET_H

#include <QWidget>
#include <QIcon>
#include <QPointer>

#include "qgspanelwidget.h"
#include "qgslayertreegroup.h"
#include "qgis_gui.h"

class QgsMapCanvas;
class QgsMapLayer;
class QgsMessageBar;

/**
 * \ingroup gui
 * \class QgsMapLayerConfigWidgetContext
 * \brief Encapsulates the context for a QgsMapLayerConfigWidget.
 * \since QGIS 3.22
 */
class GUI_EXPORT QgsMapLayerConfigWidgetContext
{
  public:

    /**
     * Returns the item ID of the target annotation, when modifying
     * an annotation from a QgsAnnotationLayer.
     *
     * \see setAnnotationId()
     */
    QString annotationId() const { return mAnnotationId; }

    /**
     * Sets the item \a id of the target annotation, when modifying
     * an annotation from a QgsAnnotationLayer.
     *
     * \see annotationId()
     */
    void setAnnotationId( const QString &id ) { mAnnotationId = id; }

    /**
     * Sets the map \a canvas associated with the widget. This allows the widget to retrieve the current
     * map scale and other properties from the canvas.
     *
     * \see mapCanvas()
     */
    void setMapCanvas( QgsMapCanvas *canvas ) { mMapCanvas = canvas; }

    /**
     * Returns the map canvas associated with the widget.
     * \see setMapCanvas()
     */
    QgsMapCanvas *mapCanvas() const { return mMapCanvas; }

    /**
     * Sets the message \a bar associated with the widget. This allows the widget to push feedback messages
     * to the appropriate message bar.
     * \see messageBar()
     */
    void setMessageBar( QgsMessageBar *bar ) { mMessageBar = bar; }

    /**
     * Returns the message bar associated with the widget.
     * \see setMessageBar()
     */
    QgsMessageBar *messageBar() const { return mMessageBar; }

    /**
     * Sets the layer tree \a group associated with the widget.
     *
     * \see layerTreeGroup()
     * \since QGIS 3.24
     */
    void setLayerTreeGroup( QgsLayerTreeGroup *group );

    /**
     * Returns the layer tree group associated with the widget.
     *
     * \see setLayerTreeGroup()
     * \since QGIS 3.24
     */
    QgsLayerTreeGroup *layerTreeGroup() const;

  private:

    QString mAnnotationId;
    QgsMapCanvas *mMapCanvas = nullptr;
    QgsMessageBar *mMessageBar = nullptr;
    QPointer< QgsLayerTreeGroup > mLayerTreeGroup = nullptr;

};


/**
 * \ingroup gui
 * \class QgsMapLayerConfigWidget
 * \brief A panel widget that can be shown in the map style dock
 * \since QGIS 2.16
 */
class GUI_EXPORT QgsMapLayerConfigWidget : public QgsPanelWidget
{
    Q_OBJECT
  public:

    /**
       * \brief A panel widget that can be shown in the map style dock
       * \param layer The layer active in the dock.
       * \param canvas The canvas object.
       * \param parent The parent of the widget.
       * \note The widget is created each time the panel is selected in the dock.
       * Keep the loading light as possible for speed in the UI.
       */
    QgsMapLayerConfigWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, QWidget *parent = nullptr );

    /**
     * Whether this config widget changes map layer properties in a way that triggerRepaint() should
     * be called for the layer after applying changes. This is TRUE by default, but some config widgets
     * (for example 3D rendering config) do not need layer repaint as they do not modify 2D map rendering.
     * \since QGIS 3.8
     */
    virtual bool shouldTriggerLayerRepaint() const { return true; }

    /**
     * Reset to original (vector layer) values
     * \since QGIS 3.14
     */
    virtual void syncToLayer( QgsMapLayer *layer ) { Q_UNUSED( layer ) }

    /**
     * Sets the \a context under which the widget is being shown.
     *
     * Subclasses should take care to call the base class implementation when overriding this method.
     */
    virtual void setMapLayerConfigWidgetContext( const QgsMapLayerConfigWidgetContext &context );

  public slots:

    /**
     * \brief Called when changes to the layer need to be made.
     * Will be called when live update is enabled.
     */
    virtual void apply() = 0;

    /**
     * Focuses the default widget for the page.
     *
     * \since QGIS 3.22
     */
    virtual void focusDefaultWidget();

  signals:

#ifndef SIP_RUN

    /**
     * Emitted whenever the dynamic temporal control option is toggled in the associated layer
     * properties dialog.
     *
     * \note Not available in Python bindings
     * \since QGIS 3.20
     */
    void dynamicTemporalControlToggled( bool enabled );
#endif

  protected:

    QgsMapLayer *mLayer = nullptr;
    QgsMapCanvas *mMapCanvas = nullptr;
    QgsMapLayerConfigWidgetContext mMapLayerConfigWidgetContext;
};

#endif // QGSMAPLAYERCONFIGWIDGET_H
