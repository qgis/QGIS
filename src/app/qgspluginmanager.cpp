/***************************************************************************
                          qgspluginmanager.cpp  -  description
                             -------------------
    begin                : Someday 2003
    copyright            : (C) 2003 by Gary E.Sherman
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

#include "qgsconfig.h"

#include <iostream>
#include <QApplication>
#include <QFileDialog>
#include <QLineEdit>
#include <Q3ListView>
#include <QMessageBox>
#include <QLibrary>
#include <QSettings>
#include "qgisplugin.h"
#include "qgslogger.h"
#include "qgspluginmanager.h"
#include "qgspluginitem.h"
#include "qgsproviderregistry.h"
#include "qgspluginregistry.h"

#ifdef HAVE_PYTHON
#include "qgspythonutils.h"
#endif

#define TESTLIB
#ifdef TESTLIB
// This doesn't work on WIN32 and causes problems with plugins
// on OS X (the code doesn't cause a problem but including dlfcn.h
// renders plugins unloadable)
#if !defined(WIN32) && !defined(Q_OS_MACX) 
#include <dlfcn.h>
#endif
#endif
QgsPluginManager::QgsPluginManager(QWidget * parent, Qt::WFlags fl)
: QDialog(parent, fl)
{
  setupUi(this);
  // set the default lib dir to the qgis install directory/lib (this info is
  // available from the provider registry so we use it here)
  QgsProviderRegistry *pr = QgsProviderRegistry::instance();
  /*  char **argv = qApp->argv();
     QString appDir = argv[0];
     int bin = appDir.findRev("/bin", -1, false);
     QString baseDir = appDir.left(bin);
     QString libDir = baseDir + "/lib"; */

  txtPluginDir->setText(pr->libraryDirectory().path());
  getPluginDescriptions();
  getPythonPluginDescriptions();
}


QgsPluginManager::~QgsPluginManager()
{
}


void QgsPluginManager::getPythonPluginDescriptions()
{
#ifdef HAVE_PYTHON
  if (!QgsPythonUtils::isEnabled())
    return;
  
  // look for plugins systemwide
  QStringList pluginList = QgsPythonUtils::pluginList();
  
  for (int i = 0; i < pluginList.size(); i++)
  {
    QString packageName = pluginList[i];

    // import plugin's package - skip loading it if an error occured
    if (!QgsPythonUtils::loadPlugin(packageName))
      continue;
    
    // get information from the plugin
    QString pluginName  = QgsPythonUtils::getPluginMetadata(packageName, "name");
    QString description = QgsPythonUtils::getPluginMetadata(packageName, "description");
    QString version     = QgsPythonUtils::getPluginMetadata(packageName, "version");
    
    if (pluginName == "???" || description == "???" || version == "???")
      continue;
    
    // add to the list box
    Q3CheckListItem *pl = new Q3CheckListItem(lstPlugins, pluginName, Q3CheckListItem::CheckBox);
    pl->setText(1, version);
    pl->setText(2, description);
    pl->setText(3, "python:" + packageName);
  
    // check to see if the plugin is loaded and set the checkbox accordingly
    QgsPluginRegistry *pRegistry = QgsPluginRegistry::instance();
    
    QString libName = pRegistry->library(pluginName);
    if (libName.length() == 0 || !pRegistry->isPythonPlugin(pluginName))
    {
      QgsDebugMsg("Couldn't find library name in the registry");
    }
    else
    {
      QgsDebugMsg("Found library name in the registry");
      if (libName == packageName)
      {
        // set the checkbox
        pl->setOn(true);
      }
    }
  }
#endif
}


void QgsPluginManager::getPluginDescriptions()
{
QString sharedLibExtension;
#ifdef WIN32
sharedLibExtension = "*.dll";
#else
sharedLibExtension = "*.so*";
#endif

// check all libs in the current plugin directory and get name and descriptions
  QDir pluginDir(txtPluginDir->text(), sharedLibExtension, QDir::Name | QDir::IgnoreCase, QDir::Files | QDir::NoSymLinks);

  if (pluginDir.count() == 0)
  {
      QMessageBox::information(this, tr("No Plugins"), tr("No QGIS plugins found in ") + txtPluginDir->text());
      return;
  }

  QgsDebugMsg("PLUGIN MANAGER:");
  for (uint i = 0; i < pluginDir.count(); i++)
  {
    QString lib = QString("%1/%2").arg(txtPluginDir->text()).arg(pluginDir[i]);

#ifdef TESTLIB
          // This doesn't work on WIN32 and causes problems with plugins
          // on OS X (the code doesn't cause a problem but including dlfcn.h
          // renders plugins unloadable)
#if !defined(WIN32) && !defined(Q_OS_MACX)
          // test code to help debug loading problems
          // This doesn't work on WIN32 and causes problems with plugins
          // on OS X (the code doesn't cause a problem but including dlfcn.h
          // renders plugins unloadable)

//          void *handle = dlopen((const char *) lib, RTLD_LAZY);
    void *handle = dlopen(lib.toLocal8Bit().data(), RTLD_LAZY | RTLD_GLOBAL);
    if (!handle)
    {
      QgsDebugMsg("Error in dlopen: ");
      QgsDebugMsg(dlerror());
    }
    else
    {
      QgsDebugMsg("dlopen suceeded for " + lib);
      dlclose(handle);
    }
#endif //#ifndef WIN32 && Q_OS_MACX
#endif //#ifdef TESTLIB

    QgsDebugMsg("Examining: " + lib);
    QLibrary *myLib = new QLibrary(lib);
    bool loaded = myLib->load();
    if (!loaded)
    {
      QgsDebugMsg("Failed to load: " + myLib->library());
      delete myLib;
      continue;
    }

    QgsDebugMsg("Loaded library: " + myLib->library());

    // Don't bother with libraries that are providers
    //if(!myLib->resolve("isProvider"))

    //MH: Replaced to allow for plugins that are linked to providers
    //type is only used in non-provider plugins 
    if (!myLib->resolve("type"))
    {
      delete myLib;
      continue;
    }
    
    // resolve the metadata from plugin
    name_t *pName = (name_t *) myLib->resolve("name");
    description_t *pDesc = (description_t *) myLib->resolve("description");
    version_t *pVersion = (version_t *) myLib->resolve("version");

    // show the values (or lack of) for each function
    if(pName){
      QgsDebugMsg("Plugin name: " + pName());
    }else{
      QgsDebugMsg("Plugin name not returned when queried\n");
    }
    if(pDesc){
      QgsDebugMsg("Plugin description: " + pDesc());
    }else{
      QgsDebugMsg("Plugin description not returned when queried\n");
    }
    if(pVersion){
      QgsDebugMsg("Plugin version: " + pVersion());
    }else{
      QgsDebugMsg("Plugin version not returned when queried\n");
    }

    if (!pName || !pDesc || !pVersion)
    {
      QgsDebugMsg("Failed to get name, description, or type for " + myLib->library());
      delete myLib;
      continue;
    }

    Q3CheckListItem *pl = new Q3CheckListItem(lstPlugins, pName(), Q3CheckListItem::CheckBox); //, pDesc(), pluginDir[i])
    pl->setText(1, pVersion());
    pl->setText(2, pDesc());
    pl->setText(3, pluginDir[i]);

    QgsDebugMsg("Getting an instance of the QgsPluginRegistry");

    // check to see if the plugin is loaded and set the checkbox accordingly
    QgsPluginRegistry *pRegistry = QgsPluginRegistry::instance();

    // get the library using the plugin description
    QString libName = pRegistry->library(pName());
    if (libName.length() == 0)
    {
      QgsDebugMsg("Couldn't find library name in the registry");
    }
    else
    {
      QgsDebugMsg("Found library name in the registry");
      if (libName == myLib->library())
      {
        // set the checkbox
        pl->setOn(true);
      }
    }

    delete myLib;
  }
}


void QgsPluginManager::on_btnOk_clicked()
{
  unload();
  accept();
}

void QgsPluginManager::unload()
{
  QSettings settings;
#ifdef QGISDEBUG
  std::cout << "Checking for plugins to unload" << std::endl;
#endif
  Q3CheckListItem *lvi = (Q3CheckListItem *) lstPlugins->firstChild();
  while (lvi != 0)
    {
      if (!lvi->isOn())
        {
          // its off -- see if it is loaded and if so, unload it
          QgsPluginRegistry *pRegistry = QgsPluginRegistry::instance();
#ifdef QGISDEBUG
          std::cout << "Checking to see if " << lvi->text(0).toLocal8Bit().data() << " is loaded" << std::endl;
#endif
          
          QString pluginName = lvi->text(0);

          if (pRegistry->isPythonPlugin(pluginName))
          {
#ifdef HAVE_PYTHON
            QString packageName = pRegistry->library(pluginName);
            QgsPythonUtils::unloadPlugin(packageName);

            //disable it to the qsettings file
            settings.writeEntry("/PythonPlugins/" + packageName, false);
#endif
          }
          else // C++ plugin
          {
            QgisPlugin *plugin = pRegistry->plugin(pluginName);
            if (plugin)
            {
              plugin->unload();
            }
            //disable it to the qsettings file [ts]
            settings.writeEntry("/Plugins/" + pluginName, false);
          }

          // remove the plugin from the registry
          pRegistry->removePlugin(pluginName);
        }
      lvi = (Q3CheckListItem *) lvi->nextSibling();
    }
}

std::vector < QgsPluginItem > QgsPluginManager::getSelectedPlugins()
{
  std::vector < QgsPluginItem > pis;
  Q3CheckListItem *lvi = (Q3CheckListItem *) lstPlugins->firstChild();
  while (lvi != 0)
  {
    if (lvi->isOn())
    {
      QString pluginName = lvi->text(0);
      bool pythonic = false;
      
      QString library = lvi->text(3);
      if (library.left(7) == "python:")
      {
        library = library.mid(7);
        pythonic = true;
      }
      else // C++ plugin
      {
        library = txtPluginDir->text() + "/" + library;
      }
      
      pis.push_back(QgsPluginItem(pluginName, lvi->text(2), library, 0, pythonic));
    }
    lvi = (Q3CheckListItem *) lvi->nextSibling();
  }
  return pis;
}

void QgsPluginManager::on_btnSelectAll_clicked()
{
  // select all plugins
  Q3CheckListItem *child = dynamic_cast<Q3CheckListItem *>(lstPlugins->firstChild());
  while(child)
  {
    child->setOn(true);
    child = dynamic_cast<Q3CheckListItem *>(child->nextSibling());
  }

}

void QgsPluginManager::on_btnClearAll_clicked()
{
  // clear all selection checkboxes 
  Q3CheckListItem *child = dynamic_cast<Q3CheckListItem *>(lstPlugins->firstChild());
  while(child)
  {
    child->setOn(false);
    child = dynamic_cast<Q3CheckListItem *>(child->nextSibling());
  }
}

void QgsPluginManager::on_btnClose_clicked()
{
  reject();
}
