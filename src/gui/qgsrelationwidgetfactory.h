/***************************************************************************
                         qgsrelationwidgetfactory.h
                         ----------------------
    begin                : October 2020
    copyright            : (C) 2020 by Ivan Ivanov
    email                : ivan@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRELATIONEDITORBASEWIDGETFACTORY_H
#define QGSRELATIONEDITORBASEWIDGETFACTORY_H

#include <QWidget>
#include "qgsrelationwidget.h"
#include "qgis_gui.h"

class QgsRelationConfigWidget;

/**
 * Factory class for creating relation widgets and their corresponding config widgets
 * \ingroup gui
 * \class QgsRelationWidgetFactory
 * \since QGIS 3.18
 */
class GUI_EXPORT QgsRelationWidgetFactory
{
  public:

    /**
     * Creates a new relation widget factory with given \a name
     */
    QgsRelationWidgetFactory();

    virtual ~QgsRelationWidgetFactory() = default;

    /**
     * Returns the machine readable identifier name of this widget type
     */
    virtual QString type() const = 0;

    /**
     * Returns the human readable identifier name of this widget type
     */
    virtual QString name() const = 0;

    /**
     * Override this in your implementation.
     * Create a new relation widget. Call QgsEditorWidgetRegistry::create()
     * instead of calling this method directly.
     *
     * \param config   The widget configuration to build the widget with
     * \param parent   The parent for the wrapper class and any created widget.
     *
     * \returns         A new widget wrapper
     */
    virtual QgsRelationWidget *create( const QVariantMap &config, QWidget *parent = nullptr ) const = 0 SIP_FACTORY;

    /**
     * Override this in your implementation.
     * Create a new configuration widget for this widget type.
     *
     * \param relation The relation for which the widget will be created
     * \param parent   The parent widget of the created config widget
     *
     * \returns         A configuration widget
     */
    virtual QgsRelationConfigWidget *configWidget( const QgsRelation &relation, QWidget *parent ) const = 0 SIP_FACTORY;
};



#endif // QGSRELATIONEDITORBASEWIDGETFACTORY_H
