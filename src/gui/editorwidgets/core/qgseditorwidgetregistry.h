/***************************************************************************
    qgseditorwidgetregistry.h
     --------------------------------------
    Date                 : 24.4.2013
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

#ifndef QGSEDITORWIDGETREGISTRY_H
#define QGSEDITORWIDGETREGISTRY_H

#include <QObject>
#include <QMap>

#include "qgseditorwidgetfactory.h"

class QgsMapLayer;
class QDomNode;
class QgsMapCanvas;
class QgsMessageBar;

/**
 * This class manages all known edit widget factories
 */
class GUI_EXPORT QgsEditorWidgetRegistry : public QObject
{
    Q_OBJECT

  public:
    /**
     * This class is a singleton and has therefore to be accessed with this method instead
     * of a constructor.
     *
     * @return The one and only instance of the editor widget registry
     */
    static QgsEditorWidgetRegistry* instance();

    /**
     * Registers all the default widgets.
     * Only call this once on startup of an application.
     *
     * @param mapCanvas  Specify a map canvas with which the widgets (relation reference) work
     * @param messageBar Specify a message bar on which messages by widgets will be shown while working with the map canvas
     *
     * @note Added in QGIS 2.8
     * @note Not required for plugins, the QGIS application does that already
     */
    static void initEditors( QgsMapCanvas* mapCanvas = 0, QgsMessageBar* messageBar = 0 );

    /**
     * Destructor
     *
     * Deletes all the registered widgets
     */
    ~QgsEditorWidgetRegistry();

    /**
     * Create an attribute editor widget wrapper of a given type for a given field.
     * The editor may be NULL if you want the widget wrapper to create a default widget.
     *
     * @param widgetId  The id of the widget type to create an attribute editor for
     * @param vl        The vector layer for which this widget will be created
     * @param fieldIdx  The field index on the specified layer for which this widget will be created
     * @param config    A configuration which should be used for the widget creation
     * @param editor    An editor widget which will be used instead of an autocreated widget
     * @param parent    The parent which will be used for the created wrapper and the created widget
     * @param context   The editor context (not available in python bindings)
     *
     * @return A new widget wrapper
     */
    QgsEditorWidgetWrapper* create( const QString& widgetId,
                                    QgsVectorLayer* vl,
                                    int fieldIdx,
                                    const QgsEditorWidgetConfig& config,
                                    QWidget* editor,
                                    QWidget* parent,
                                    const QgsAttributeEditorContext& context = QgsAttributeEditorContext() );

    QgsSearchWidgetWrapper* createSearchWidget( const QString& widgetId,
        QgsVectorLayer* vl,
        int fieldIdx,
        const QgsEditorWidgetConfig& config,
        QWidget* parent,
        const QgsAttributeEditorContext& context = QgsAttributeEditorContext() );

    /**
     * Creates a configuration widget
     *
     * @param widgetId  The id of the widget type to create a configuration widget for
     * @param vl        The vector layer for which this widget will be created
     * @param fieldIdx  The field index on the specified layer for which this widget will be created
     * @param parent    The parent widget for the created widget
     *
     * @return A new configuration widget
     */
    QgsEditorConfigWidget* createConfigWidget( const QString& widgetId, QgsVectorLayer* vl, int fieldIdx, QWidget* parent );

    /**
     * Get the human readable name for a widget type
     *
     * @param widgetId The widget type to get the name for
     *
     * @return A human readable name
     */
    QString name( const QString& widgetId );

    /**
     * Get access to all registered factories
     *
     * @return All ids and factories
     */
    const QMap<QString, QgsEditorWidgetFactory*>& factories();

    /**
     * Get a factory for the given widget type id.
     *
     * @return A factory or Null if not existent
     */
    QgsEditorWidgetFactory* factory( const QString& widgetId );

    /**
     * Register a new widget factory with the given id
     *
     * @param widgetId      The id which will be used later to refer to this widget type
     * @param widgetFactory The factory which will create this widget type
     *
     * @return true, if successful, false, if the widgetId is already in use or widgetFactory is NULL
     */
    bool registerWidget( const QString& widgetId, QgsEditorWidgetFactory* widgetFactory );

  protected:
    QgsEditorWidgetRegistry();

  private slots:
    /**
     * Read all the editor widget information from a map layer XML node
     * @param mapLayer
     * @param layerElem
     */
    void readMapLayer( QgsMapLayer* mapLayer, const QDomElement& layerElem );

    /**
     * Read all old-style editor widget configuration from a map node. Will update
     * a project file to the new version on next save
     * @param vl         The layer in question
     * @param layerElem  The layer element from the project file
     * @param cfg        Writable config element
     *
     * @deprecated
     */
    Q_DECL_DEPRECATED const QString readLegacyConfig( QgsVectorLayer* vl, const QDomElement& layerElem, QgsEditorWidgetConfig& cfg );

    /**
     * Write all the widget config to a layer XML node
     *
     * @param mapLayer   The layer for which the config is being written
     * @param layerElem  The XML element to which the config will be written
     * @param doc        The document from which to create elements
     */
    void writeMapLayer( QgsMapLayer* mapLayer, QDomElement& layerElem, QDomDocument& doc ) const;

    /**
     * Will connect to appropriate signals from map layers to load and save style
     *
     * @param mapLayer The layer to connect
     */
    void mapLayerAdded( QgsMapLayer* mapLayer );

    /**
     * Loads layer symbology for the layer that emitted the signal
     *
     * @param element The XML element containing the style information
     *
     * @param errorMessage Errors will be written into this string (unused)
     */
    void readSymbology( const QDomElement& element, QString& errorMessage );

    /**
     * Saves layer symbology for the layer that emitted the signal
     *
     * @param element The XML element where the style information be written to
     * @param doc     The XML document where the style information be written to
     *
     * @param errorMessage Errors will be written into this string (unused)
     */
    void writeSymbology( QDomElement& element, QDomDocument& doc, QString& errorMessage );

  private:
    QMap<QString, QgsEditorWidgetFactory*> mWidgetFactories;
};

#endif // QGSEDITORWIDGETREGISTRY_H
