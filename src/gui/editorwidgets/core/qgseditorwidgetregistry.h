/***************************************************************************
    qgseditorwidgetregistry.h
     --------------------------------------
    Date                 : 24.4.2013
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

#ifndef QGSEDITORWIDGETREGISTRY_H
#define QGSEDITORWIDGETREGISTRY_H

#include <QObject>
#include "qgis_sip.h"
#include <QMap>
#include "qgseditorwidgetfactory.h"
#include "qgsattributeeditorcontext.h"
#include "qgseditorwidgetautoconf.h"
#include "qgis_gui.h"

class QgsMapLayer;
class QDomNode;
class QgsMapCanvas;
class QgsMessageBar;
class QgsSearchWidgetWrapper;
class QgsEditorWidgetWrapper;
class QgsEditorConfigWidget;
class QgsVectorLayer;


/**
 * \ingroup gui
 * This class manages all known edit widget factories.
 *
 * QgsEditorWidgetRegistry is not usually directly created, but rather accessed through
 * QgsGui::editorWidgetRegistry().
 */
class GUI_EXPORT QgsEditorWidgetRegistry : public QObject
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsEditorWidgetRegistry. QgsEditorWidgetRegistry is not usually directly created, but rather accessed through
     * QgsGui::editorWidgetRegistry().
     */
    QgsEditorWidgetRegistry();

    /**
     * Registers all the default widgets.
     * Only call this once on startup of an application.
     *
     * \param mapCanvas  Specify a map canvas with which the widgets (relation reference) work
     * \param messageBar Specify a message bar on which messages by widgets will be shown while working with the map canvas
     *
     * \note Not required for plugins, the QGIS application does that already
     * \since QGIS 2.8
     */
    void initEditors( QgsMapCanvas *mapCanvas = nullptr, QgsMessageBar *messageBar = nullptr );

    /**
     * Destructor
     *
     * Deletes all the registered widgets
     */
    ~QgsEditorWidgetRegistry() override;

    /**
     * Find the best editor widget and its configuration for a given field.
     *
     * \param vl        The vector layer for which this widget will be created
     * \param fieldName The field name on the specified layer for which this widget will be created
     *
     * \returns The id of the widget type to use and its config
     */
    QgsEditorWidgetSetup findBest( const QgsVectorLayer *vl, const QString &fieldName ) const;

    /**
     * Create an attribute editor widget wrapper of a given type for a given field.
     * The editor may be NULL if you want the widget wrapper to create a default widget.
     *
     * \param widgetId  The id of the widget type to create an attribute editor for
     * \param vl        The vector layer for which this widget will be created
     * \param fieldIdx  The field index on the specified layer for which this widget will be created
     * \param config    A configuration which should be used for the widget creation
     * \param editor    An editor widget which will be used instead of an autocreated widget
     * \param parent    The parent which will be used for the created wrapper and the created widget
     * \param context   The editor context (not available in Python bindings)
     *
     * \returns A new widget wrapper
     */
    QgsEditorWidgetWrapper *create( const QString &widgetId,
                                    QgsVectorLayer *vl,
                                    int fieldIdx,
                                    const QVariantMap &config,
                                    QWidget *editor,
                                    QWidget *parent SIP_TRANSFERTHIS,
                                    const QgsAttributeEditorContext &context  SIP_PYARGREMOVE = QgsAttributeEditorContext() ) SIP_FACTORY;

    /**
     * Create an attribute editor widget wrapper of the best type for a given field.
     * The editor may be NULL if you want the widget wrapper to create a default widget.
     *
     * \param vl        The vector layer for which this widget will be created
     * \param fieldIdx  The field index on the specified layer for which this widget will be created
     * \param editor    An editor widget which will be used instead of an autocreated widget
     * \param parent    The parent which will be used for the created wrapper and the created widget
     * \param context   The editor context (not available in Python bindings)
     *
     * \returns A new widget wrapper
     */
    QgsEditorWidgetWrapper *create( QgsVectorLayer *vl,
                                    int fieldIdx,
                                    QWidget *editor,
                                    QWidget *parent SIP_TRANSFERTHIS,
                                    const QgsAttributeEditorContext &context SIP_PYARGREMOVE = QgsAttributeEditorContext() ) SIP_FACTORY;

    QgsSearchWidgetWrapper *createSearchWidget( const QString &widgetId,
        QgsVectorLayer *vl,
        int fieldIdx,
        const QVariantMap &config,
        QWidget *parent SIP_TRANSFERTHIS,
        const QgsAttributeEditorContext &context SIP_PYARGREMOVE = QgsAttributeEditorContext() ) SIP_FACTORY;

    /**
     * Creates a configuration widget
     *
     * \param widgetId  The id of the widget type to create a configuration widget for
     * \param vl        The vector layer for which this widget will be created
     * \param fieldIdx  The field index on the specified layer for which this widget will be created
     * \param parent    The parent widget for the created widget
     *
     * \returns A new configuration widget
     */
    QgsEditorConfigWidget *createConfigWidget( const QString &widgetId, QgsVectorLayer *vl, int fieldIdx, QWidget *parent SIP_TRANSFERTHIS ) SIP_FACTORY;

    /**
     * Gets the human readable name for a widget type
     *
     * \param widgetId The widget type to get the name for
     *
     * \returns A human readable name
     */
    QString name( const QString &widgetId );

    /**
     * Gets access to all registered factories
     *
     * \returns All ids and factories
     */
    QMap<QString, QgsEditorWidgetFactory *> factories();

    /**
     * Gets a factory for the given widget type id.
     *
     * \returns A factory or Null if not existent
     */
    QgsEditorWidgetFactory *factory( const QString &widgetId );

    /**
     * Register a new widget factory with the given id
     *
     * \param widgetId      The id which will be used later to refer to this widget type
     * \param widgetFactory The factory which will create this widget type
     *
     * \returns true, if successful, false, if the widgetId is already in use or widgetFactory is NULL
     */
    bool registerWidget( const QString &widgetId, QgsEditorWidgetFactory *widgetFactory SIP_TRANSFER );

    /**
     * Register a new auto-conf plugin.
     *
     * \param plugin The plugin (ownership is transferred)
     */
    void registerAutoConfPlugin( QgsEditorWidgetAutoConfPlugin *plugin ) { mAutoConf.registerPlugin( plugin ); }

  private:
    QString findSuitableWrapper( QWidget *editor, const QString &defaultWidget );

    QMap<QString, QgsEditorWidgetFactory *> mWidgetFactories;
    QMap<const char *, QPair<int, QString> > mFactoriesByType;
    QgsEditorWidgetAutoConf mAutoConf;
    std::unique_ptr<QgsEditorWidgetFactory> mFallbackWidgetFactory = nullptr;
};

#endif // QGSEDITORWIDGETREGISTRY_H
