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
#include "qgis_gui.h"
#include "qgis.h"


/**
 * \ingroup gui
 * \class QgsPluginManagerInterface
 */
class GUI_EXPORT QgsPluginManagerInterface : public QObject
{
    Q_OBJECT

  public:

    QgsPluginManagerInterface() = default;

    //! remove Python plugins from the metadata registry (c++ plugins stay)
    virtual void clearPythonPluginMetadata() = 0;

    //! add a single plugin to the metadata registry
    virtual void addPluginMetadata( const QMap<QString, QString> &metadata ) = 0;

    //! refresh plugin list model (and metadata browser content if necessary)
    virtual void reloadModel() = 0;

    //! Returns given plugin metadata
    virtual const QMap<QString, QString> *pluginMetadata( const QString &key ) const = 0;

    //! clear the repository listWidget
    virtual void clearRepositoryList() = 0;

    //! add repository to the repository listWidget
    virtual void addToRepositoryList( const QMap<QString, QString> &repository ) = 0;

    //! show the Plugin Manager window and optionally open tab tabIndex
    virtual void showPluginManager( int tabIndex = -1 ) = 0;

    //! show the given message in the Plugin Manager internal message bar
    virtual void pushMessage( const QString &text, Qgis::MessageLevel level = Qgis::MessageLevel::Info, int duration = -1 ) = 0;
};

#endif
