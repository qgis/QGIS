//
//
// C++ Interface: $MODULE$
//
// Description: 
//
//
// Author: Gary Sherman <sherman at mrcc.com>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//
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
