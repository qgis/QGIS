/***************************************************************************
                          qgspluginmanager.h 
               Plugin manager for loading/unloading QGIS plugins
                             -------------------
    begin                : 2004-02-12
    copyright            : (C) 2004 by Gary E.Sherman
    email                : sherman at mrcc.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 /* $Id$ */
#ifndef QGSPLUGINMANAGER_H
#define QGSPLUGINMANAGER_H
#include <vector>
#ifdef WIN32
#include "qgspluginmanagerbase.h"
#else
#include "qgspluginmanagerbase.uic.h"
#endif

class QgsPluginItem;
/*!
 * \brief Plugin manager for loading/unloading plugins
@author Gary Sherman
*/
class QgsPluginManager : public QgsPluginManagerBase
{
  Q_OBJECT
  public:
    //! Constructor
    QgsPluginManager(QWidget *parent=0, const char *name=0);
    //! Destructor
    ~QgsPluginManager();
    //! Browse to a location (directory) containing QGIS plugins
    void browseFiles();
    //! Get description of plugins (name, etc)
    void getPluginDescriptions();
    //! Unload the selected plugins
    void unload();
    //! Gets the selected plugins
    std::vector<QgsPluginItem> getSelectedPlugins();
    public slots:
    //! Load selected plugins and close the dialog
    void apply();
    //! Select all plugins by setting their checkbox on
    void selectAll();
    //! Clear all selections by clearing the plugins checkbox
    void clearAll();
};

#endif
