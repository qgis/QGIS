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
#include "qgspluginmanager.h"

/** \ingroup gui
 * QgsPluginManagerInterface
 * Abstract base class to make QgsPluginManager available to plugins.
 */
class QgsAppPluginManagerInterface : public QgsPluginManagerInterface
{
    Q_OBJECT

  public:

    //! Constructor
    explicit QgsAppPluginManagerInterface( QgsPluginManager * pluginManager );

    //! Destructor
    ~QgsAppPluginManagerInterface();

    //! remove metadata of all python plugins from the registry (c++ plugins stay)
    void clearPythonPluginMetadata();

    //! add a single plugin to the metadata registry
    void addPluginMetadata( QMap<QString,QString> metadata );

    //! refresh listView model and textView content
    void reloadModel();

    //! get given plugin metadata
    QMap<QString, QString> * pluginMetadata( QString key );

    //! clear the repository listWidget
    void clearRepositoryList();

    //! add repository to the repository listWidget
    void addToRepositoryList( QMap<QString,QString> repository );

  public slots:

    //! show the Plugin Manager window when remote repositories are fetched.
    //! Display a progress dialog when fetching.
    void showPluginManagerWhenReady( );

    //! promptly show the Plugin Manager window and optionally open tab tabIndex:
    void showPluginManager( int tabIndex = -1 );

  private:

    //! Pointer to QgsPluginManager object
    QgsPluginManager *mPluginManager;
};

#endif //QGSPLUGINMANAGERAPPIFACE_H
