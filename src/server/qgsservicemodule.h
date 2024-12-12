/***************************************************************************
                          qgsservicemodule.h

  Class defining the service module interface for QGIS server services.
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


#ifndef QGSSERVICEMODULE_H
#define QGSSERVICEMODULE_H

#include "qgis_server.h"

class QgsServiceRegistry;
class QgsServerInterface;

/**
 * \ingroup server
 * \class QgsServiceModule
 * \brief Class defining the service module interface for QGIS server services
 *
 * This class acts as a service registrar for services.
 *
 * For dynamic modules, a QgsServiceModule instance is returned from the
 * QGS_ServiceModule_Init() entry point.
 *
 */
class SERVER_EXPORT QgsServiceModule
{
  public:
    QgsServiceModule() = default;
    virtual ~QgsServiceModule() = default;

    /**
     * Asks the module to register all provided services.
     * \param registry Service registry
     * \param serverIface Interface for plugins
     */
    virtual void registerSelf( QgsServiceRegistry &registry, QgsServerInterface *serverIface = nullptr ) = 0;
};

#endif
