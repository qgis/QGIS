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
#include <qsettings.h>
#include "../plugins/qgisplugin.h"
#include "qgspluginmanager.h"
#include "qgspluginitem.h"
#include "qgsproviderregistry.h"
#include "qgspluginregistry.h"


#define TESTLIB
#ifdef TESTLIB
#ifndef WIN32
#include <dlfcn.h>
#endif
#endif
QgsPluginManager::QgsPluginManager(QWidget * parent, const char *name):QgsPluginManagerBase(parent, name)
{
  // set the default lib dir to the qgis install directory/lib (this info is
  // available from the provider registry so we use it here)
  QgsProviderRegistry *pr = QgsProviderRegistry::instance();
  /*  char **argv = qApp->argv();
     QString appDir = argv[0];
     int bin = appDir.findRev("/bin", -1, false);
     QString baseDir = appDir.left(bin);
     QString libDir = baseDir + "/lib"; */

  txtPluginDir->setText(pr->libDirectory());
  getPluginDescriptions();
}


QgsPluginManager::~QgsPluginManager()
{
}

void QgsPluginManager::browseFiles()
{
  QString s = QFileDialog::getExistingDirectory(0, this, "get existing directory", tr("Choose a directory"), TRUE);
  txtPluginDir->setText(s);
  getPluginDescriptions();
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
  } else
    {
      std::cout << "PLUGIN MANAGER:" << std::endl;
      for (unsigned i = 0; i < pluginDir.count(); i++)
        {
#ifdef TESTLIB
#ifndef WIN32
          // test code to help debug loading problems
          QString lib = QString("%1/%2").arg(txtPluginDir->text()).arg(pluginDir[i]);
//          void *handle = dlopen((const char *) lib, RTLD_LAZY);
          void *handle = dlopen((const char *) lib, RTLD_LAZY | RTLD_GLOBAL);
          if (!handle)
            {
              std::cout << "Error in dlopen: " << dlerror() << std::endl;

          } else
            {
              std::cout << "dlopen suceeded" << std::endl;
              dlclose(handle);
            }
#endif //#ifndef WIN32
#endif //#ifdef TESTLIB


          std::cout << "Examining " << txtPluginDir->text() << "/" << pluginDir[i] << std::endl;
          QLibrary *myLib = new QLibrary(txtPluginDir->text() + "/" + pluginDir[i]);
          bool loaded = myLib->load();
          if (loaded)
            {
              std::cout << "Loaded " << myLib->library() << std::endl;
              name_t *pName = (name_t *) myLib->resolve("name");
              description_t *pDesc = (description_t *) myLib->resolve("description");
              version_t *pVersion = (version_t *) myLib->resolve("version");
              if (pName && pDesc && pVersion)
                {
                  QCheckListItem *pl = new QCheckListItem(lstPlugins, pName(), QCheckListItem::CheckBox); //, pDesc(), pluginDir[i])
                  pl->setText(1, pVersion());
                  pl->setText(2, pDesc());
                  pl->setText(3, pluginDir[i]);

#ifdef QGISDEBUG
                  std::cout << "Getting an instance of the QgsPluginRegistry" << std::endl;
#endif
                  // check to see if the plugin is loaded and set the checkbox accordingly
                  QgsPluginRegistry *pRegistry = QgsPluginRegistry::instance();
                  // get the library using the plugin description
#ifdef QGISDEBUG
                  std::cout << "Getting library name from the registry" << std::endl;
#endif
                  QString libName = pRegistry->library(pName());
                  if (libName.length() > 0)
                    {
#ifdef QGISDEBUG
                      std::cout << "Found library name in the registry" << std::endl;
#endif
                      if (libName == myLib->library())
                        {
                          // set the checkbox
                          pl->setOn(true);
                        }
                    }
              } else
                {
                  std::cout << "Failed to get name, description, or type for " << myLib->library() << std::endl;
                }
          } else
            {
              std::cout << "Failed to load " << myLib->library() << std::endl;
            }
        }
    }
}
void QgsPluginManager::apply()
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
  QCheckListItem *lvi = (QCheckListItem *) lstPlugins->firstChild();
  while (lvi != 0)
    {
      if (!lvi->isOn())
        {
          // its off -- see if it is loaded and if so, unload it
          QgsPluginRegistry *pRegistry = QgsPluginRegistry::instance();
#ifdef QGISDEBUG
          std::cout << "Checking to see if " << lvi->text(0) << " is loaded" << std::endl;
#endif
          QgisPlugin *plugin = pRegistry->plugin(lvi->text(0));
          if (plugin)
          {
            plugin->unload();
            // remove the plugin from the registry
            pRegistry->removePlugin(lvi->text(0));
            //disable it to the qsettings file [ts]
            settings.writeEntry("/qgis/Plugins/" + lvi->text(0), false);
          }
        }
      lvi = (QCheckListItem *) lvi->nextSibling();
    }
}

std::vector < QgsPluginItem > QgsPluginManager::getSelectedPlugins()
{
  std::vector < QgsPluginItem > pis;
  QCheckListItem *lvi = (QCheckListItem *) lstPlugins->firstChild();
  while (lvi != 0)
    {
      if (lvi->isOn())
        {

          pis.push_back(QgsPluginItem(lvi->text(0), lvi->text(2), txtPluginDir->text() + "/" + lvi->text(3)));
      } else
        {

        }
      lvi = (QCheckListItem *) lvi->nextSibling();
    }
  return pis;
}
