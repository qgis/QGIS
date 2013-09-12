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

    /**
     * Return The human readable name of this widget type
     *
     * By default returns the name specified when constructing and does not need to be overwritten
     *
     * @return a name
     */
    virtual QString name();

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
    virtual QgsEditorWidgetConfig readConfig( const QDomElement& configElement, QgsVectorLayer* layer, int fieldIdx );

    /**
     * Serialize your configuration and save it in a xml doc.
     *
     * @param config        The configuration to serialize
     * @param configElement The element, where you can write your configuration into
     * @param doc           The document. You can use this to create new nodes
     * @param layer         The layer for which this configuration applies
     * @param fieldIdx      The field on the layer for which this configuration applies
     */
    virtual void writeConfig( const QgsEditorWidgetConfig& config, QDomElement& configElement, const QDomDocument& doc, const QgsVectorLayer* layer, int fieldIdx );

  private:
    QString mName;
};

/**
 * This is a templated wrapper class, which inherits QgsEditWidgetFactory and does the boring work for you.
 * C++ only
 */
template<typename F, typename G>
class GUI_EXPORT QgsEditWidgetFactoryHelper : public QgsEditorWidgetFactory
{
  public:
    QgsEditWidgetFactoryHelper( QString name )
        : QgsEditorWidgetFactory( name ) {}

    QgsEditorWidgetWrapper* create( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QWidget* parent ) const
    {
      return new F( vl, fieldIdx, editor, parent );
    }

    QgsEditorConfigWidget* configWidget( QgsVectorLayer* vl, int fieldIdx, QWidget* parent )
    {
      return new G( vl, fieldIdx, parent );
    }

    /**
     * Read the config from an XML file and map it to a proper {@link QgsEditorWidgetConfig}.
     *
     * Implement this method yourself somewhere with the class template parameters
     * specified. To keep things clean, every implementation of this class should be placed
     * next to the associated widget factory implementation.
     *
     * @param configElement The configuration element from the project file
     * @param layer         The layer for which this configuration applies
     * @param fieldIdx      The field on the layer for which this configuration applies
     *
     * @return A configuration object. This will be passed to your widget wrapper later on
     */

    virtual QgsEditorWidgetConfig readConfig( const QDomElement& configElement, QgsVectorLayer* layer, int fieldIdx );

    /**
     * Serialize your configuration and save it in a xml doc.
     *
     * Implement this method yourself somewhere with the class template parameters
     * specified. To keep things clean, every implementation of this class should be placed
     * next to the associated widget factory implementation.
     *
     * @param config        The configuration to serialize
     * @param configElement The element, where you can write your configuration into
     * @param doc           The document. You can use this to create new nodes
     * @param layer         The layer for which this configuration applies
     * @param fieldIdx      The field on the layer for which this configuration applies
     */
    virtual void writeConfig( const QgsEditorWidgetConfig& config, QDomElement& configElement, const QDomDocument& doc, const QgsVectorLayer* layer, int fieldIdx );
};

#endif // QGSEDITORWIDGETFACTORY_H
