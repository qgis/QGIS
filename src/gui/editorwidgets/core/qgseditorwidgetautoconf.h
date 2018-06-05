/***************************************************************************
    qgseditorwidgetautoconf.h
    ---------------------
    begin                : July 2016
    copyright            : (C) 2016 by Patrick Valsecchi
    email                : patrick.valsecchi at camptocamp.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSEDITORWIDGETAUTOCONF_H
#define QGSEDITORWIDGETAUTOCONF_H

#include <QList>
#include "qgis.h"
#include "qgis_gui.h"
#include <memory>

class QgsVectorLayer;
class QgsEditorWidgetSetup;

/**
 * \ingroup gui
 * Base class for plugins allowing to pick automatically a widget type for editing fields.
 *
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsEditorWidgetAutoConfPlugin
{
  public:

    virtual ~QgsEditorWidgetAutoConfPlugin() = default;

    /**
     * Typical scores are:
     *   * 0: no matching type found.
     *   * 10: a widget has been guessed from the type of field.
     *   * 20: a widget has been determined from an external configuration (for example a database table)
     *
     * \param vl        The vector layer for which this widget will be created
     * \param fieldName The field name on the specified layer for which this widget will be created
     * \param score     Where the score is returned (default to 0)
     *
     * \returns and integer value rating how good is the setup provided by this plugin.
     */
    virtual QgsEditorWidgetSetup editorWidgetSetup( const QgsVectorLayer *vl, const QString &fieldName, int &score SIP_OUT ) const = 0;

};


///@cond PRIVATE

/**
 * \ingroup gui
 * Class that allows registering plugins to pick automatically a widget type for editing fields.
 * This class has only one instance, owned by the QgsEditorWidgetRegistry singleton
 *
 * The plugins are instances of QgsEditorWidgetAutoConfPlugin.
 * \note not available in Python bindings
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsEditorWidgetAutoConf SIP_SKIP
{
  public:

    /**
     * Register the default plugins.
     */
    QgsEditorWidgetAutoConf();

    /**
     * Iterate over the plugins and return the setup of the plugin returning the highest score.
     *
     * \param vl        The vector layer for which this widget will be created
     * \param fieldName The field name on the specified layer for which this widget will be created
     *
     * \returns The best widget setup that was found
     */
    QgsEditorWidgetSetup editorWidgetSetup( const QgsVectorLayer *vl, const QString &fieldName ) const;

    /**
     * Register a new plugin.
     *
     * \param plugin The plugin (ownership is transferred)
     */
    void registerPlugin( QgsEditorWidgetAutoConfPlugin *plugin );

  private:
    QList<std::shared_ptr<QgsEditorWidgetAutoConfPlugin> > plugins;
};
///@endcond

#endif // QGSEDITORWIDGETAUTOCONF_H
