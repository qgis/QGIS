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
