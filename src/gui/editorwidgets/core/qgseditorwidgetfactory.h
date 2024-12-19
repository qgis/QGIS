/***************************************************************************
    qgseditorwidgetfactory.h
     --------------------------------------
    Date                 : 21.4.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias at opengis dot ch
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

#include <QDomNode>
#include "qgis_sip.h"
#include <QMap>
#include <QString>
#include <QVariant>
#include "qgis_gui.h"

class QgsEditorConfigWidget;
class QgsEditorWidgetWrapper;
class QgsVectorLayer;
class QWidget;
class QgsSearchWidgetWrapper;

/**
 * \ingroup gui
 * \brief Every attribute editor widget needs a factory, which inherits this class
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
     * \param name A human readable name for this widget type
     */
    QgsEditorWidgetFactory( const QString &name );

    virtual ~QgsEditorWidgetFactory() = default;

    /**
     * Override this in your implementation.
     * Create a new editor widget wrapper. Call QgsEditorWidgetRegistry::create()
     * instead of calling this method directly.
     *
     * \param vl       The vector layer on which this widget will act
     * \param fieldIdx The field index on which this widget will act
     * \param editor   An editor widget if already existent. If NULLPTR is provided, a new widget will be created.
     * \param parent   The parent for the wrapper class and any created widget.
     *
     * \returns         A new widget wrapper
     */
    virtual QgsEditorWidgetWrapper *create( QgsVectorLayer *vl, int fieldIdx, QWidget *editor, QWidget *parent ) const = 0 SIP_FACTORY;

    virtual QgsSearchWidgetWrapper *createSearchWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const SIP_FACTORY;

    /**
     * Returns The human readable identifier name of this widget type
     *
     * \returns a name
     */
    QString name() const;

    /**
     * Override this in your implementation.
     * Create a new configuration widget for this widget type.
     *
     * \param vl       The layer for which the widget will be created
     * \param fieldIdx The field index for which the widget will be created
     * \param parent   The parent widget of the created config widget
     *
     * \returns         A configuration widget
     */
    virtual QgsEditorConfigWidget *configWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const = 0 SIP_FACTORY;

    /**
     * Check if this editor widget type supports a certain field.
     *
     * \param vl        The layer
     * \param fieldIdx  The field index
     * \returns          TRUE if the type is supported for this field
     *
     * \see fieldScore()
     */
    inline bool supportsField( const QgsVectorLayer *vl, int fieldIdx ) const { return fieldScore( vl, fieldIdx ) > 0; }

    /**
     * Returns a list of widget types which this editor widget supports.
     * Each widget type can have a priority value attached, the factory with the highest one
     * will be used.
     *
     * \returns A map of widget type names and weight values
     * \note not available in Python bindings
     */
    virtual QHash<const char *, int> supportedWidgetTypes() { return QHash<const char *, int>(); } SIP_SKIP

    /**
     * This method allows disabling this editor widget type for a certain field.
     * By default, it returns 5 for every fields.
     * Reimplement this if you only support certain fields.
     *
     * Typical return values are:
     *
     * - 0: not supported
     * - 5: maybe support (for example, Datetime support strings depending on their content)
     * - 10: basic support (this is what returns TextEdit for example, since it supports everything in a crude way)
     * - 20: specialized support
     *
     * \param vl
     * \param fieldIdx
     * \returns 0 if the field is not supported or a bigger number if it can (the widget with the biggest number will be
     *      taken by default). The default implementation returns 5..
     *
     * \see supportsField()
     */
    virtual unsigned int fieldScore( const QgsVectorLayer *vl, int fieldIdx ) const;

  private:
    QString mName;
};

#endif // QGSEDITORWIDGETFACTORY_H
