
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
#include "qgspluginmanagerbase.h"

class QgsPluginItem;
/**
@author Gary Sherman
*/
class QgsPluginManager : public QgsPluginManagerBase
{
Q_OBJECT
public:
    QgsPluginManager(QWidget *parent=0, const char *name=0);

    ~QgsPluginManager();
	void browseFiles();
	void getPluginDescriptions();
  void unload();
	std::vector<QgsPluginItem> getSelectedPlugins();
public slots:
  void apply();
};

#endif
