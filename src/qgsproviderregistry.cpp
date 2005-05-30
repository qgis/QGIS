/***************************************************************************
                    qgsproviderregistry.cpp  -  Singleton class for
                    registering data providers.
                             -------------------
    begin                : Sat Jan 10 2004
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

#include <iostream>
#include <qmessagebox.h>
#include <qstring.h>
#include <qdir.h>
#include <qlibrary.h>
#include <qapplication.h>
#include "qgsprovidermetadata.h"
#include "qgsproviderregistry.h"
// typedefs for provider plugin functions of interest
typedef QString providerkey_t();
typedef QString description_t();
typedef bool isprovider_t();

QgsProviderRegistry *QgsProviderRegistry::_instance = 0;
QgsProviderRegistry *QgsProviderRegistry::instance(const char *pluginPath)
{
  if (_instance == 0)
    {
      _instance = new QgsProviderRegistry(pluginPath);
    }
  return _instance;
}

QgsProviderRegistry::QgsProviderRegistry(const char *pluginPath)
{
// At startup, examine the libs in the qgis/lib dir and store those that
// are a provider shared lib
// check all libs in the current plugin directory and get name and descriptions
//TODO figure out how to register and identify data source plugin for a specific
//TODO layer type
/* char **argv = qApp->argv();
QString appDir = argv[0];
int bin = appDir.findRev("/bin", -1, false);
QString baseDir = appDir.left(bin);
QString libDir = baseDir + "/lib"; */
  libDir = pluginPath;
#ifdef WIN32
  QDir pluginDir(libDir, "*.dll", QDir::Name | QDir::IgnoreCase, QDir::Files | QDir::NoSymLinks);
#else
  QDir pluginDir(libDir, "*.so*", QDir::Name | QDir::IgnoreCase, QDir::Files | QDir::NoSymLinks);
#endif
#ifdef QGISDEBUG
  std::cerr << "Checking " << libDir << " for provider plugins" << std::endl;
#endif
  if (pluginDir.count() == 0)
    {
      QString msg = QObject::tr("No Data Provider Plugins", "No QGIS data provider plugins found in:");
      msg += "\n" + libDir + "\n\n";
      msg += QObject::tr("No vector layers can be loaded. Check your QGIS installation");
      QMessageBox::critical(0, QObject::tr("No Data Providers"), msg);
  } else
    {

      for (unsigned i = 0; i < pluginDir.count(); i++)
        {
          QLibrary *myLib = new QLibrary(libDir + "/" + pluginDir[i]);
          bool loaded = myLib->load();
          if (loaded)
            {
#ifdef QGISDEBUG		    
              std::cout << "Checking  " << myLib->library() << std::endl;
#endif
              // get the description and the key for the provider plugin

              isprovider_t *isProvider = (isprovider_t *) myLib->resolve("isProvider");

              if (isProvider)
                {
                  // check to see if this is a provider plugin
                  if (isProvider())
                    {
                      // looks like a provider. get the key and description
                      description_t *pDesc = (description_t *) myLib->resolve("description");
                      providerkey_t *pKey = (providerkey_t *) myLib->resolve("providerKey");
                      if (pDesc && pKey)
                        {
							const char *foo = pKey();
                          // add this provider to the provider map
                          provider[pKey()] = new QgsProviderMetadata(pKey(), pDesc(), myLib->library());
#ifdef QGISDEBUG
                          std::cout << "Loaded " << pDesc() << std::endl;
#endif
                      } else
                        {
                          std::cout << myLib->
                            library() << " Unable to find one of the required provider functions:\n\tproviderKey() or description()" <<
                            std::endl;
                        }
                    }
                }
            }
          delete myLib;
        }
    }
}
QString QgsProviderRegistry::library(QString providerKey)
{
  QString retval;
  QgsProviderMetadata *md = provider[providerKey];
  if (md)
    {
      retval = md->library();
    }
  return retval;
}

QString QgsProviderRegistry::pluginList(bool asHTML)
{
  std::map < QString, QgsProviderMetadata * >::iterator it = provider.begin();
  QString list;
  if (provider.size() == 0)
    {
      list = QObject::tr("No data provider plugins are available. No vector layers can be loaded");
  } else
    {
      if (asHTML)
        {
          list += "<ol>";
        }
      while (it != provider.end())
        {
          QgsProviderMetadata *mp = (*it).second;
          if (asHTML)
            {
              list += "<li>" + mp->description() + "<br>";
          } else
            {
              list += mp->description() + "\n";
            }
          it++;
        }
      if (asHTML)
        {
          list += "</ol>";
        }
    }
  return list;
}

void QgsProviderRegistry::setLibDirectory(QString path)
{
  libDir = path;
}

QString QgsProviderRegistry::libDirectory()
{
  return libDir;
}

// typedef for the QgsDataProvider class factory
typedef QgsDataProvider * create_it(const char * uri);
 


/** Copied from QgsVectorLayer::setDataProvider 
 *  TODO: Make it work in the generic environment
 *
 *  TODO: Is this class really the best place to put a data provider loader?
 *        It seems more sensible to provide the code in one place rather than
 *        in qgsrasterlayer, qgsvectorlayer, serversourceselect, etc.
 */
QgsDataProvider* QgsProviderRegistry::getProvider( QString const & providerKey, 
                                                   QString const & dataSource )
{
  // XXX should I check for and possibly delete any pre-existing providers?
  // XXX How often will that scenario occur?

  // load the plugin
  QString lib = library(providerKey);

#ifdef TESTPROVIDERLIB
  const char *cLib = (const char *) lib;

  // test code to help debug provider loading problems
  //  void *handle = dlopen(cLib, RTLD_LAZY);
  void *handle = dlopen(cOgrLib, RTLD_LAZY | RTLD_GLOBAL);
  if (!handle)
  {
    std::cout << "Error in dlopen: " << dlerror() << std::endl;

  }
  else
  {
    std::cout << "dlopen suceeded" << std::endl;
    dlclose(handle);
  }

#endif

  // load the data provider
  QLibrary* myLib = new QLibrary((const char *) lib);
#ifdef QGISDEBUG
  std::cout << "QgsProviderRegistry::getRasterProvider: Library name is " << myLib->library() << std::endl;
#endif
  bool loaded = myLib->load();

  if (loaded)
  {
#ifdef QGISDEBUG
    std::cout << "QgsProviderRegistry::getProvider: Loaded data provider library" << std::endl;
    std::cout << "QgsProviderRegistry::getProvider: Attempting to resolve the classFactory function" << std::endl;
#endif
    create_it * classFactory = (create_it *) myLib->resolve("classFactory");

    if (classFactory)
    {
#ifdef QGISDEBUG
      std::cout << "QgsProviderRegistry::getProvider: Getting pointer to a dataProvider object from the library\n";
#endif
      //XXX - This was a dynamic cast but that kills the Windows
      //      version big-time with an abnormal termination error
      QgsDataProvider* dataProvider = (QgsDataProvider*)
                                      (classFactory((const char*)(dataSource.utf8())));

      if (dataProvider)
      {
#ifdef QGISDEBUG
        std::cout << "QgsProviderRegistry::getProvider: Instantiated the data provider plugin\n";
#endif
        if (dataProvider->isValid())
        {
          return dataProvider;
        }
      }
      else
      {
#ifdef QGISDEBUG
        std::cout << "QgsProviderRegistry::getProvider: Unable to instantiate the data provider plugin\n";
#endif
        return 0;
      }
    }
  }
  else
  {
    return 0;
#ifdef QGISDEBUG
    std::cout << "QgsProviderRegistry::getProvider: Failed to load " << "../providers/libproviders.so" << "\n";
#endif

  }
  
#ifdef QGISDEBUG
    std::cout << "QgsProviderRegistry::getProvider: exiting." << "\n";
#endif

  

} // QgsProviderRegistry::setDataProvider
