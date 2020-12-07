/***************************************************************************
    qgsapppluginmanagerinterface.h
     --------------------------------------
    Date                 : 15-May-2013
    Copyright            : (C) 2013 by Borys Jurgiel
    Email                : info at borysjurgiel dot pl
****************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPLUGINMANAGERAPPIFACE_H
#define QGSPLUGINMANAGERAPPIFACE_H

#include "qgspluginmanagerinterface.h"

class QgsPluginManager;

/**
 * \ingroup gui
 * QgsPluginManagerInterface
 * Abstract base class to make QgsPluginManager available to pyplugin_installer.
 */
class QgsAppPluginManagerInterface : public QgsPluginManagerInterface
{
    Q_OBJECT

  public:

    //! Constructor
    explicit QgsAppPluginManagerInterface( QgsPluginManager *pluginManager );

    //! Removes Python plugins from the metadata registry (c++ plugins stay)
    void clearPythonPluginMetadata() override;

    //! Adds a single plugin to the metadata registry
    void addPluginMetadata( const QMap<QString, QString> &metadata ) override;

    //! Refreshes the plugin list model (and metadata browser content if necessary)
    void reloadModel() override;

    //! Returns the given plugin metadata
    const QMap<QString, QString> *pluginMetadata( const QString &key ) const override;

    //! Clears the repository listWidget
    void clearRepositoryList() override;

    //! Adds a repository to the repository listWidget
    void addToRepositoryList( const QMap<QString, QString> &repository ) override;

    //! Shows the Plugin Manager window and optionally open tab tabIndex
    void showPluginManager( int tabIndex = -1 ) override;

    //! Shows the given message in the Plugin Manager internal message bar
    void pushMessage( const QString &text, Qgis::MessageLevel level = Qgis::Info, int duration = -1 ) override;

  private:

    //! Pointer to QgsPluginManager object
    QgsPluginManager *mPluginManager = nullptr;
};

#endif //QGSPLUGINMANAGERAPPIFACE_H
