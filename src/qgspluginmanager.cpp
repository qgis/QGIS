//
//
// C++ Implementation: $MODULE$
//
// Description: 
//
//
// Author: Gary Sherman <sherman at mrcc.com>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//
/* $Id$ */
#include <iostream>
#include <qapplication.h>
#include <qfiledialog.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qmessagebox.h>
#include <qlibrary.h>
#include "../plugins/qgisplugin.h"
#include "qgspluginmanager.h"
#include "qgspluginitem.h"

#define TESTLIB
#ifdef TESTLIB
#include <dlfcn.h>
#endif
QgsPluginManager::QgsPluginManager(QWidget *parent, const char * name)
 : QgsPluginManagerBase(parent, name)
{
  // set the default lib dir to the qgis install directory/lib
  QString appDir = qApp->applicationDirPath();
  int bin = appDir.findRev("/bin", -1, false);
  QString baseDir = appDir.left(bin);
  QString libDir = baseDir + "/lib";
  txtPluginDir->setText(libDir);
  getPluginDescriptions();
}


QgsPluginManager::~QgsPluginManager()
{
}

void QgsPluginManager::browseFiles(){
 QString s = QFileDialog::getExistingDirectory(0, this, "get existing directory", tr("Choose a directory"),  TRUE );
 txtPluginDir->setText(s);
 getPluginDescriptions();
}

void QgsPluginManager::getPluginDescriptions(){
// check all libs in the current plugin directory and get name and descriptions
QDir pluginDir(txtPluginDir->text(), "*.so*", QDir::Name | QDir::IgnoreCase, QDir::Files | QDir::NoSymLinks);
	
	if(pluginDir.count() == 0){
		QMessageBox::information(this, tr("No Plugins"), tr("No QGIS plugins found in ") + txtPluginDir->text());
	}else{
		std::cout << "PLUGIN MANAGER:" << std::endl;
		for(unsigned i = 0; i < pluginDir.count(); i++){
      #ifdef TESTLIB
  // test code to help debug loading problems
  QString lib = QString("%1/%2").arg(txtPluginDir->text()).arg(pluginDir[i]);
    void *handle = dlopen((const char *)lib, RTLD_LAZY);
    if (!handle) {
      std::cout << "Error in dlopen: " << dlerror() << std::endl;
  
    } else {
      std::cout << "dlopen suceeded" << std::endl;
      dlclose(handle);
    }
  
  #endif
      
      
      std::cout << "Examining " << txtPluginDir->text() << "/" << pluginDir[i] << std::endl;
			QLibrary *myLib = new QLibrary(txtPluginDir->text() + "/" + pluginDir[i]);
			bool loaded = myLib->load();
			if (loaded) {
        std::cout << "Loaded " << myLib->library() << std::endl;
				name_t *pName = (name_t *) myLib->resolve("name");
				description_t *pDesc = (description_t *) myLib->resolve("description");
        type_t *pType = (type_t *) myLib->resolve("type");
				if(pName && pDesc && pType){
					QCheckListItem *pl = new QCheckListItem(lstPlugins, pName(),QCheckListItem::CheckBox); //, pDesc(), pluginDir[i]);
					pl->setText(1, pDesc());
					pl->setText(2, pluginDir[i]);
          pl->setText(3, QString().setNum(pType()));
				}else{
          std::cout << "Failed to get name, description, or type for " << myLib->library() << std::endl;
        }
			}else{
        std::cout << "Failed to load " << myLib->library() << std::endl;
      }
		}
}
}
std::vector<QgsPluginItem> QgsPluginManager::getSelectedPlugins(){
	std::vector<QgsPluginItem> pis;
	QCheckListItem *lvi = (QCheckListItem *)lstPlugins->firstChild();
	while(lvi != 0){
		if(lvi->isOn()){
			
			pis.push_back(QgsPluginItem(lvi->text(0), lvi->text(1), txtPluginDir->text() + "/" + lvi->text(2)));
		}
		lvi = (QCheckListItem *)lvi->nextSibling();
	}
	return pis;
}
