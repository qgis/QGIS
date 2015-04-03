/***************************************************************************
    qgsrelationreferencefactory.h
     --------------------------------------
    Date                 : 29.5.2013
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

#ifndef QGSRELATIONREFERENCEFACTORY_H
#define QGSRELATIONREFERENCEFACTORY_H

#include "qgsattributeeditorcontext.h"
#include "qgseditorwidgetfactory.h"

class QgsMapCanvas;
class QgsMessageBar;

class GUI_EXPORT QgsRelationReferenceFactory : public QgsEditorWidgetFactory
{
  public:
    QgsRelationReferenceFactory( QString name, QgsMapCanvas* canvas, QgsMessageBar* messageBar );

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
    virtual QgsEditorWidgetWrapper* create( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QWidget* parent ) const override;

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
    virtual QgsEditorConfigWidget* configWidget( QgsVectorLayer* vl, int fieldIdx, QWidget* parent ) const override;

    /**
     * Read the config from an XML file and map it to a proper {@link QgsEditorWidgetConfig}.
     *
     * @param configElement The configuration element from the project file
     * @param layer         The layer for which this configuration applies
     * @param fieldIdx      The field on the layer for which this configuration applies
     *
     * @return A configuration object. This will be passed to your widget wrapper later on
     */
    virtual QgsEditorWidgetConfig readConfig( const QDomElement& configElement, QgsVectorLayer* layer, int fieldIdx ) override;

    /**
     * Serialize your configuration and save it in a xml doc.
     *
     * @param config        The configuration to serialize
     * @param configElement The element, where you can write your configuration into
     * @param doc           The document. You can use this to create new nodes
     * @param layer         The layer for which this configuration applies
     * @param fieldIdx      The field on the layer for which this configuration applies
     */
    virtual void writeConfig( const QgsEditorWidgetConfig& config, QDomElement& configElement, QDomDocument& doc, const QgsVectorLayer* layer, int fieldIdx ) override;

  private:
    QgsAttributeEditorContext mEditorContext;
    QgsMapCanvas* mCanvas;
    QgsMessageBar* mMessageBar;
};

#endif // QGSRELATIONREFERENCEFACTORY_H
