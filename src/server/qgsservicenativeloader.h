/***************************************************************************
                          qgsservicerenativeloader.h

  Define Loader for native service modules
  -------------------
  begin                : 2016-12-05
  copyright            : (C) 2016 by David Marteau
  email                : david dot marteau at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSSERVICENATIVELOADER_H
#define QGSSERVICENATIVELOADER_H

#define SIP_NO_FILE


class QgsServiceModule;
class QgsServiceRegistry;
class QgsServiceNativeModuleEntry;
class QgsServerInterface;

#include "qgis_server.h"

#include <QHash>
#include <memory>

/**
 * \ingroup server
 * QgsServiceNativeLoader
 * Class defining the native service module loader for QGIS server services
 *
 * This class provides methods for loading and managing hook for native (C++) modules
 *
 */
class SERVER_EXPORT QgsServiceNativeLoader
{
  public:

    //! Constructor
    QgsServiceNativeLoader() = default;

    /**
     * Lead all medules from path
     * \param modulePath the path to look for module
     * \param registrar QgsServiceRegistry instance for registering services
     * \param serverIface QgsServerInterface instarce
     */
    void loadModules( const QString &modulePath, QgsServiceRegistry &registrar,
                      QgsServerInterface *serverIface = nullptr );

    /**
     * Unload all modules
     */
    void unloadModules();

    /**
     * Load the native module from path
     *
     * \param location QString location holding the module relalive path
     * \returns a qgsservicemodule instance
     */
    QgsServiceModule *loadNativeModule( const QString &location );


  private:
    typedef QHash<QString, std::shared_ptr<QgsServiceNativeModuleEntry> > ModuleTable;

    /**
     * Find module
     * \param path the module path
     * \returns a module hook entry
     */
    QgsServiceNativeModuleEntry *findModuleEntry( const QString &path );

    /**
     *  Unload medule hook
     */
    void unloadModuleEntry( QgsServiceNativeModuleEntry *entry );

    //! Associative storage for module handles
    ModuleTable mModules;
};

#endif

