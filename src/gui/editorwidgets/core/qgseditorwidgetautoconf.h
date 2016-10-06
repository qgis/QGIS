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
#include <QSharedPointer>

class QgsVectorLayer;
class QgsEditorWidgetSetup;

/** \ingroup gui
 * Base class for plugins allowing to pick automatically a widget type for editing fields.
 *
 * @note added in QGIS 3.0
 */
class GUI_EXPORT QgsEditorWidgetAutoConfPlugin
{
  public:
    /**
     * Typical scores are:
     *   * 0: no matching type found.
     *   * 10: a widget has been guessed from the type of field.
     *   * 20: a widget has been determined from an external configuration (for example a database table)
     *
     * @param vl        The vector layer for which this widget will be created
     * @param fieldName The field name on the specified layer for which this widget will be created
     * @param score     Where the score is returned (default to 0)
     *
     * @return and integer value rating how good is the setup provided by this plugin.
     */
    virtual QgsEditorWidgetSetup editorWidgetSetup( const QgsVectorLayer* vl, const QString& fieldName, int& score ) const = 0;

};


///@cond PRIVATE
/** \ingroup gui
 * Class that allows to register plugins to pick automatically a widget type for editing fields.
 * This class has only one instance, owned by the QgsEditorWidgetRegistry singleton
 *
 * The plugins are instances of QgsEditorWidgetAutoConfPlugin.
 * @note added in QGIS 3.0
 * @note not available in Python bindings
 */
class GUI_EXPORT QgsEditorWidgetAutoConf
{
  public:
    /**
     * Register the default plugins.
     */
    QgsEditorWidgetAutoConf();

    /**
     * Iterate over the plugins and return the setup of the plugin returning the highest score.
     *
     * @param vl        The vector layer for which this widget will be created
     * @param fieldName The field name on the specified layer for which this widget will be created
     *
     * @return The best widget setup that was found
     */
    QgsEditorWidgetSetup editorWidgetSetup( const QgsVectorLayer* vl, const QString& fieldName ) const;

    /**
     * Register a new plugin.
     *
     * @param plugin The plugin (ownership is transfered)
     */
    void registerPlugin( QgsEditorWidgetAutoConfPlugin* plugin );

  private:
    QList<QSharedPointer<QgsEditorWidgetAutoConfPlugin> > plugins;
};
///@endcond

#endif // QGSEDITORWIDGETAUTOCONF_H