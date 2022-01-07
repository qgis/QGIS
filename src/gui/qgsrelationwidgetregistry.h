/***************************************************************************
                         qgsrelationwidgetregistry.h
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

#ifndef QGSRELATIONWIDGETREGISTRY_H
#define QGSRELATIONWIDGETREGISTRY_H

#include "qgsabstractrelationeditorwidget.h"
#include "qgis_gui.h"

/**
 * Keeps track of the registered relations widgets. New widgets can be registered, old ones deleted.
 * The default {\see QgsRelationEditorWidget} is protected from removing.
 * \ingroup gui
 * \class QgsRelationWidgetRegistry
 * \since QGIS 3.18
 */
class GUI_EXPORT QgsRelationWidgetRegistry
{
  public:

    /**
     * Constructor
     */
    QgsRelationWidgetRegistry();

    ~QgsRelationWidgetRegistry();

    /**
     * Adds a new registered relation \a widgetFactory
     */
    void addRelationWidget( QgsAbstractRelationEditorWidgetFactory *widgetFactory SIP_TRANSFER );

    /**
     * Removes a registered relation widget with given \a widgetType
     */
    void removeRelationWidget( const QString &widgetType );

    /**
     * Returns a list of names of registered relation widgets
     */
    QStringList relationWidgetNames() const;

    /**
     * Gets access to all registered factories
     */
    QMap<QString, QgsAbstractRelationEditorWidgetFactory *> factories() const;

    /**
     * Create a relation widget of a given type for a given field.
     *
     * \param widgetType  The widget type to create a relation editor for
     * \param config  The configuration of the widget
     * \param parent
     */
    QgsAbstractRelationEditorWidget *create( const QString &widgetType, const QVariantMap &config, QWidget *parent = nullptr ) const SIP_TRANSFERBACK;

    /**
     * Creates a configuration widget
     *
     * \param widgetType  The widget type to create a configuration widget for
     * \param relation  The relation for which this widget will be created
     * \param parent    The parent widget for the created widget
     */
    QgsAbstractRelationEditorConfigWidget *createConfigWidget( const QString &widgetType, const QgsRelation &relation, QWidget *parent = nullptr ) const SIP_TRANSFERBACK;


    /**
     * Sets the default editor widget type. Does nothing if the provided widget type is not present.
     * \param widgetType The widget type to be used by default.
     * \since QGIS 3.20
     */
    void setDefaultWidgetType( const QString &widgetType );

    /**
     * Returns the default editor widget type.
     * \since QGIS 3.20
     */
    QString defaultWidgetType() const;

  private:

    QMap<QString, QgsAbstractRelationEditorWidgetFactory *> mRelationWidgetFactories;

    QString mDefaultWidgetType;
};

#endif // QGSRELATIONWIDGETREGISTRY_H
