/***************************************************************************
    qgseditorwidgetfactory.h
     --------------------------------------
    Date                 : 21.4.2013
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

#ifndef QGSEDITORWIDGETFACTORY_H
#define QGSEDITORWIDGETFACTORY_H

#include "qgseditorwidgetwrapper.h"
#include "qgsapplication.h"
#include "qgssearchwidgetwrapper.h"

#include <QDomNode>
#include <QMap>
#include <QString>

class QgsEditorConfigWidget;

/**
 * Every attribute editor widget needs a factory, which inherits this class
 *
 * It provides metadata for the widgets such as the name (human readable), it serializes
 * the configuration to an xml structure and loads the configuration from there.
 *
 * It also has factory methods to create a widget wrapper for the attribute editor itself
 * and another factory method to create a configuration dialog.
 */
class GUI_EXPORT QgsEditorWidgetFactory
{
  public:
    /**
     * Constructor
     *
     * @param name A human readable name for this widget type
     */
    QgsEditorWidgetFactory( const QString& name );

    virtual ~QgsEditorWidgetFactory();

    /**
     * Override this in your implementation.
     * Create a new editor widget wrapper. Call {@link QgsEditorWidgetRegistry::create()}
     * instead of calling this method directly.
     *
     * @param vl       The vector layer on which this widget will act
     * @param fieldIdx The field index on which this widget will act
     * @param editor   An editor widget if already existent. If NULL is provided, a new widget will be created.
     * @param parent   The parent for the wrapper class and any created widget.
     *
     * @return         A new widget wrapper
     */
    virtual QgsEditorWidgetWrapper* create( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QWidget* parent ) const = 0;

    virtual QgsSearchWidgetWrapper* createSearchWidget( QgsVectorLayer* vl, int fieldIdx, QWidget* parent ) const;

    /**
     * Return The human readable identifier name of this widget type
     *
     * @return a name
     */
    QString name();

    /**
     * Override this in your implementation.
     * Create a new configuration widget for this widget type.
     *
     * @param vl       The layer for which the widget will be created
     * @param fieldIdx The field index for which the widget will be created
     * @param parent   The parent widget of the created config widget
     *
     * @return         A configuration widget
     */
    virtual QgsEditorConfigWidget* configWidget( QgsVectorLayer* vl, int fieldIdx, QWidget* parent ) const = 0;

    /**
     * Read the config from an XML file and map it to a proper {@link QgsEditorWidgetConfig}.
     *
     * @param configElement The configuration element from the project file
     * @param layer         The layer for which this configuration applies
     * @param fieldIdx      The field on the layer for which this configuration applies
     *
     * @return A configuration object. This will be passed to your widget wrapper later on
     */
    QgsEditorWidgetConfig readEditorConfig( const QDomElement& configElement, QgsVectorLayer* layer, int fieldIdx );

    /**
     * Serialize your configuration and save it in a xml doc.
     *
     * @param config        The configuration to serialize
     * @param configElement The element, where you can write your configuration into
     * @param doc           The document. You can use this to create new nodes
     * @param layer         The layer for which this configuration applies
     * @param fieldIdx      The field on the layer for which this configuration applies
     */
    virtual void writeConfig( const QgsEditorWidgetConfig& config, QDomElement& configElement, QDomDocument& doc, const QgsVectorLayer* layer, int fieldIdx );

    /**
     * Check if this editor widget type supports a certain field.
     *
     * @param vl        The layer
     * @param fieldIdx  The field index
     * @return          True if the type is supported for this field
     *
     * @see isFieldSupported( QgsVectorLayer* vl, ind fieldIdx )
     */
    inline bool supportsField( QgsVectorLayer* vl, int fieldIdx ) { return isFieldSupported( vl, fieldIdx ); }

    /**
     * Returns a list of widget types which this editor widget supports.
     * Each widget type can have a priority value attached, the factory with the highest one
     * will be used.
     *
     * @return A map of widget type names and weight values
     */
    virtual QMap<const char*, int> supportedWidgetTypes() { return QMap<const char*, int>(); }

    /**
     * Create a pretty String representation of the value.
     *
     * @param vl        The vector layer.
     * @param fieldIdx  The index of the field.
     * @param config    The editor widget config.
     * @param cache     The editor widget cache.
     * @param value     The value to represent.
     *
     * @return By default the string representation of the provided value as implied by the field definition is returned.
     */
    virtual QString representValue( QgsVectorLayer* vl, int fieldIdx, const QgsEditorWidgetConfig& config, const QVariant& cache, const QVariant& value ) const;

    /**
     * Create a cache for a given field.
     *
     * @param vl        The vector layer.
     * @param fieldIdx  The index of the field.
     * @param config    The editor widget config.
     *
     * @return The default implementation returns an invalid QVariant
     */
    virtual QVariant createCache( QgsVectorLayer* vl, int fieldIdx, const QgsEditorWidgetConfig& config );

    /**
     * Read the config from an XML file and map it to a proper {@link QgsEditorWidgetConfig}.
     *
     * @param configElement The configuration element from the project file
     * @param layer         The layer for which this configuration applies
     * @param fieldIdx      The field on the layer for which this configuration applies
     *
     * @return A configuration object. This will be passed to your widget wrapper later on
     */
    virtual QgsEditorWidgetConfig readConfig( const QDomElement& configElement, QgsVectorLayer* layer, int fieldIdx );

  private:
    /**
     * This method allows disabling this editor widget type for a certain field.
     * By default, it returns true for all fields.
     * Reimplement this if you only support certain fields.
     *
     * @param vl
     * @param fieldIdx
     * @return True if the field is supported.
     *
     * @see supportsField( QgsVectorLayer* vl, fieldIdx )
     */
    virtual bool isFieldSupported( QgsVectorLayer* vl, int fieldIdx );

  private:
    QString mName;
};

#endif // QGSEDITORWIDGETFACTORY_H
