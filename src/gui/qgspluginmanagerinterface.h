/***************************************************************************
    qgspluginmanagerinterface.h
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

#ifndef QGSPLUGINMANAGERINTERFACE_H
#define QGSPLUGINMANAGERINTERFACE_H

#include <QObject>
#include <QString>
#include <QMap>

class GUI_EXPORT QgsPluginManagerInterface : public QObject
{
    Q_OBJECT

  public:

    //! Constructor
    QgsPluginManagerInterface();

    //! Virtual destructor
    virtual ~QgsPluginManagerInterface();

    //! remove metadata of all python plugins from the registry (c++ plugins stay)
    virtual void clearPythonPluginMetadata() = 0;

    //! add a single plugin to the metadata registry
    virtual void addPluginMetadata( QMap<QString,QString> metadata ) = 0;

    //! refresh listView model and textView content
    virtual void reloadModel() = 0;

    //! get given plugin metadata
    virtual QMap<QString, QString> * pluginMetadata( QString key ) = 0;

    //! clear the repository listWidget
    virtual void clearRepositoryList() = 0;

    //! add repository to the repository listWidget
    virtual void addToRepositoryList( QMap<QString,QString> repository ) = 0;

  signals:

    //! emitted when the Python Plugin Installer should show the fetching repositories window
    void fetchingStillInProgress( );

  public slots:

    //! show the Plugin Manager window when remote repositories are fetched.
    //! Display a progress dialog when fetching.
    virtual void showPluginManagerWhenReady( ) = 0;

    //! promptly show the Plugin Manager window and optionally open tab tabIndex:
    virtual void showPluginManager( int tabIndex = -1 ) = 0;

};

#endif
