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
#include <QMessageBox>
#include <QLibrary>
#include <QSettings>
#include <QStandardItem>
#include <QPushButton>
#include <QRegExp>

#include "qgspluginmanager.h"
#include <qgisplugin.h>
#include <qgslogger.h>
#include <qgspluginitem.h>
#include <qgsproviderregistry.h>
#include <qgspluginregistry.h>
#include <qgsdetaileditemdelegate.h>
#include <qgsdetaileditemwidget.h>
#include <qgsdetaileditemdata.h>

#ifdef HAVE_PYTHON
#include <qgspythonutils.h>
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

  lblPluginDir->setText(pr->libraryDirectory().path());
  setTable();
  getPluginDescriptions();
  getPythonPluginDescriptions();
  //
  // Create the select all and clear all buttons and add them to the 
  // buttonBox
  //
  QPushButton * btnSelectAll = new QPushButton(tr("&Select All"));
  QPushButton * btnClearAll = new QPushButton(tr("&Clear All"));
  btnSelectAll->setDefault(true);
  buttonBox->addButton(btnSelectAll, QDialogButtonBox::ActionRole);
  buttonBox->addButton(btnClearAll, QDialogButtonBox::ActionRole);
  // connect the slot up to catch when a bookmark is deleted 
  connect(btnSelectAll, SIGNAL(clicked()), this, SLOT(selectAll()));
  // connect the slot up to catch when a bookmark is zoomed to
  connect(btnClearAll, SIGNAL(clicked()), this, SLOT(clearAll()));
  
}


QgsPluginManager::~QgsPluginManager()
{
  delete mModelProxy;
  delete mModelPlugins;
}

void QgsPluginManager::setTable()
{
  mModelPlugins= new QStandardItemModel(0,1);
  mModelProxy = new QSortFilterProxyModel(this);
  mModelProxy->setSourceModel(mModelPlugins);
  //mModelProxy->setFilterKeyColumn(0);
  vwPlugins->setModel(mModelProxy);
  vwPlugins->setFocus();
  vwPlugins->setItemDelegateForColumn(0,new QgsDetailedItemDelegate());
}

void QgsPluginManager::resizeColumnsToContents()
{
  // Resize columns to contents.
  QgsDebugMsg("QgsPluginManager::resizeColumnsToContents\n");
}

void QgsPluginManager::sortModel(int column)
{
  // Sort column ascending.
  mModelPlugins->sort(column);
  QgsDebugMsg("QgsPluginManager::sortModel\n");
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

    // We will create two items for each row:
    // The first has the following roles :
    // - DisplayRole : the name and version of the version of the plugin e.g. Plugin Foo (version 1.0)
    // - UserRole : the description of the plugin.
    // And the second has these roles :
    // - DisplayRole : the library name (without path) for the plugin e.g. libfoo.so
    // - UserRole : the name of the plugin without version e.g. plugin foor
    // The plugin name without version is used elsewhere in this class to match
    // aganst plugins loaded in the plugin registry
    QStandardItem * mypDetailItem = new QStandardItem(
        pluginName + " (" + version + ")");
    mypDetailItem->setData(description,Qt::UserRole);
    QString myLibraryName = "python:" + packageName;;
    QStandardItem * mypLibraryNameItem = new QStandardItem(myLibraryName);
    mypLibraryNameItem->setData(pluginName,Qt::UserRole);
    // myName have a checkbox
    mypDetailItem->setCheckable(true);
    mypDetailItem->setEditable(false);

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
        mypDetailItem->setCheckState(Qt::Checked);
      }
    }
    // Add items to model
    QList<QStandardItem *> myItems;
    myItems << mypDetailItem << mypLibraryNameItem;
    mModelPlugins->appendRow(myItems);
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
  QDir pluginDir(lblPluginDir->text(), sharedLibExtension, QDir::Name | QDir::IgnoreCase, QDir::Files | QDir::NoSymLinks);

  if (pluginDir.count() == 0)
  {
      QMessageBox::information(this, tr("No Plugins"), tr("No QGIS plugins found in ") + lblPluginDir->text());
      return;
  }

  QgsDebugMsg("PLUGIN MANAGER:");
  for (uint i = 0; i < pluginDir.count(); i++)
  {
    QString lib = QString("%1/%2").arg(lblPluginDir->text()).arg(pluginDir[i]);

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

    // We will create two items for each row:
    // The first has the following roles :
    // - DisplayRole : the name and version of the version of the plugin e.g. Plugin Foo (version 1.0)
    // - UserRole : the description of the plugin.
    // And the second has these roles :
    // - DisplayRole : the library name (without path) for the plugin e.g. libfoo.so
    // - UserRole : the name of the plugin without version e.g. plugin foor
    // The plugin name without version is used elsewhere in this class to match
    // aganst plugins loaded in the plugin registry
        
    QStandardItem * mypDetailItem = new QStandardItem(
        pName() + " (" + pVersion() + ")");
    //
    // Uncommnet this block to render item using simple painter technique
    // 
    mypDetailItem->setData(pDesc(),Qt::UserRole);
    //
    //Uncomment this block to used widget based detail items (experimental)
    //
    //QgsDetailedItemData myData;
    //myData.setTitle(pName());
    //myData.setDetail(pDesc());
    //QVariant myVariant = qVariantFromValue(myData);
    //round trip test - delete this...no need to uncomment
    //QgsDetailedItemData myData2 = qVariantValue<QgsDetailedItemData>(myVariant);
    //Q_ASSERT(myData.title() == myData2.title());
    //round trip test ends
    //mypDetailItem->setData(myVariant,Qt::UserRole);
    QString myLibraryName = pluginDir[i];
    QStandardItem * mypLibraryNameItem = new QStandardItem(myLibraryName);
    mypLibraryNameItem->setData(pName(),Qt::UserRole);
    // myName have a checkbox
    mypDetailItem->setCheckable(true);
    mypDetailItem->setEditable(false);

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
        mypDetailItem->setCheckState(Qt::Checked);
      }
    }
    // Add items to model
    QList<QStandardItem *> myItems;
    myItems << mypDetailItem << mypLibraryNameItem;
    mModelPlugins->appendRow(myItems);

    delete myLib;
  }
}

void QgsPluginManager::accept()
{
  unload();
  done(1);
}

void QgsPluginManager::unload()
{
  QSettings settings;
#ifdef QGISDEBUG
  std::cout << "Checking for plugins to unload" << std::endl;
#endif
  for (int row=0;row < mModelPlugins->rowCount();row++)
  {
    // FPV - I want to use index. You can do evrething with item.
    QModelIndex myIndex=mModelPlugins->index(row,0);
    if (mModelPlugins->data(myIndex,Qt::CheckStateRole).toInt() == 0)
    {
      // Get the second col index now since it stores the 
      // plugin name without version string in its data UserRole [ts]
      myIndex=mModelPlugins->index(row,1);
      // its off -- see if it is loaded and if so, unload it
      QgsPluginRegistry *pRegistry = QgsPluginRegistry::instance();
#ifdef QGISDEBUG
      std::cout << "Checking to see if " << mModelPlugins->data(myIndex, Qt::UserRole).toString().toLocal8Bit().data() << " is loaded" << std::endl;
#endif
      QString pluginName = mModelPlugins->data(myIndex,Qt::UserRole).toString();
      if (pRegistry->isPythonPlugin(pluginName))
      {
#ifdef HAVE_PYTHON
        QString packageName = pRegistry->library(pluginName);
        QgsPythonUtils::unloadPlugin(packageName);
          //disable it to the qsettings file
        settings.setValue("/PythonPlugins/" + packageName, false);
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
        settings.setValue("/Plugins/" + pluginName, false);
      }
      // remove the plugin from the registry
      pRegistry->removePlugin(pluginName);
    }
  }
}

std::vector < QgsPluginItem > QgsPluginManager::getSelectedPlugins()
{
  std::vector < QgsPluginItem > pis;
  // FPV - I want to use item here. You can do everything with index if you want.
  for (int row=0;row < mModelPlugins->rowCount();row++)
  {
    if (mModelPlugins->item(row,0)->checkState() == Qt::Checked)
    {
      QString pluginName = mModelPlugins->item(row,1)->data(Qt::UserRole).toString();
      bool pythonic = false;

      QString library = mModelPlugins->item(row,1)->text();
      if (library.left(7) == "python:")
      {
        library = library.mid(7);
        pythonic = true;
      }
      else // C++ plugin
      {
        library = lblPluginDir->text() + QDir::separator() + library;
      }
      pis.push_back(QgsPluginItem(pluginName, mModelPlugins->item(row,0)->text(), library, 0, pythonic));
    }

  }
  return pis;
}

void QgsPluginManager::selectAll()
{
  // select all plugins
  for (int row=0;row < mModelPlugins->rowCount();row++)
  {
    QStandardItem *myItem=mModelPlugins->item(row,0);
    myItem->setCheckState(Qt::Checked);
  }
}

void QgsPluginManager::clearAll()
{
  // clear all selection checkboxes
  for (int row=0;row < mModelPlugins->rowCount();row++)
  {
    QStandardItem *myItem=mModelPlugins->item(row,0);
    myItem->setCheckState(Qt::Unchecked);
  }
}

void QgsPluginManager::on_vwPlugins_clicked(const QModelIndex &theIndex )
{
  if (theIndex.column() == 0)
  {
    //
    // If the model has been filtered, the index row in the proxy wont match 
    // the index row in the underlying model so we need to jump through this 
    // little hoop to get the correct item
    //
    QStandardItem * mypItem = 
      mModelPlugins->findItems(theIndex.data(Qt::DisplayRole).toString()).first();
    if ( mypItem->checkState() == Qt::Checked )
    {
      mypItem->setCheckState(Qt::Unchecked);
    } 
    else 
    {
      mypItem->setCheckState(Qt::Checked);
    }
  }
}

void QgsPluginManager::on_leFilter_textChanged(QString theText)
{
  QgsDebugMsg("PluginManager filter changed to :" + theText);
  QRegExp::PatternSyntax mySyntax = QRegExp::PatternSyntax(QRegExp::RegExp);
  Qt::CaseSensitivity myCaseSensitivity = Qt::CaseInsensitive;
  QRegExp myRegExp(theText, myCaseSensitivity, mySyntax);
  mModelProxy->setFilterRegExp(myRegExp);
}
