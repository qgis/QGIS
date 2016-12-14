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

/**
 * \ingroup server
 * QgsServiceModule
 * Class defining the service module interface for QGIS server services
 *
 * This class acts as a service registrar for services.
 *
 * For dynamic modules, a QgsServiceModule instance is returned from the QGS_ServiceModule_Init() entry point
 */
class QgsServiceModule
{
%TypeHeaderCode
#include "qgsservicemodule.h"
#include "qgsserviceregistry.h"
%End
  public:

    //! Constructor
    QgsServiceModule();

    //! Destructor
    virtual ~QgsServiceModule() = 0;

    /**
     * Ask module to registe all provided services
     * @param registry QgsServiceRegistry
     */
    virtual void registerSelf( QgsServiceRegistry& registry ) = 0;
};


