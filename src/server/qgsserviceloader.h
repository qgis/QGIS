/***************************************************************************
                          qgsserviceloader.h

  Define abstract loader class for service modules
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
#ifndef QGSSERVICELOADER_H
#define QGSSERVICELOADER_H

#include <QString>

class QgsServiceModule;
class QgsServiceRegistry;

/**
 * \ingroup server
 * QgsServiceLoader
 * Abstract base  Class defining the native service module loader for QGIS server services
 */
class SERVER_EXPORT QgsServiceLoader
{
  public:

    //! Constructor
    // XXX if not defined then dynamic linker complains about missing symbol
    QgsServiceLoader();

    //! Destructor
    virtual ~QgsServiceLoader() = 0;

    /**
     * Lead all medules from path
     * @param modulePath the path to look for module
     * @param registrar QgsServiceRegistry instance for registering services
     */
    virtual void loadModules( const QString& modulePath, QgsServiceRegistry& registrar ) = 0;

    /**
     * Unload all modules
     */
    virtual void unloadModules() = 0;
};

#endif


