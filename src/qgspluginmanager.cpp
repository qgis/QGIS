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
#include <qfiledialog.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qmessagebox.h>
#include <qlibrary.h>
#include "../plugins/qgisplugin.h"
#include "qgspluginmanager.h"
#include "qgspluginitem.h"

QgsPluginManager::QgsPluginManager(QWidget *parent, const char * name)
 : QgsPluginManagerBase(parent, name)
{

}


QgsPluginManager::~QgsPluginManager()
{
}

void QgsPluginManager::browseFiles(){
 QString s = QFileDialog::getExistingDirectory(0, this, "get existing directory", "Choose a directory",  TRUE );
 txtPluginDir->setText(s);
 getPluginDescriptions();
}

void QgsPluginManager::getPluginDescriptions(){
// check all libs in the current plugin directory and get name and descriptions
QDir pluginDir(txtPluginDir->text(), "*.so*", QDir::Name | QDir::IgnoreCase, QDir::Files | QDir::NoSymLinks);
	
	if(pluginDir.count() == 0){
		QMessageBox::information(this, "No Plugins", "No QGIS plugins found in " + txtPluginDir->text());
	}else{
		
		for(unsigned i = 0; i < pluginDir.count(); i++){
			QLibrary *myLib = new QLibrary(txtPluginDir->text() + "/" + pluginDir[i]);
			bool loaded = myLib->load();
			if (loaded) {
				name_t *pName = (name_t *) myLib->resolve("name");
				description_t *pDesc = (description_t *) myLib->resolve("description");
				if(pName && pDesc){
					QCheckListItem *pl = new QCheckListItem(lstPlugins, pName(),QCheckListItem::CheckBox); //, pDesc(), pluginDir[i]);
					pl->setText(1, pDesc());
					pl->setText(2, pluginDir[i]);
				}
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
