/***************************************************************************
                          qgsservice.h

  Class defining the service interface for QGIS server services.
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
 * QgsService
 * Class defining interfaces for QGIS server services
 *
 * This class provides methods for executing server requests
 * They are registered at runtime for a given service name.
 *
 */
class QgsService
{
%TypeHeaderCode
#include "qgsservice.h"
#include "qgsserverrequest.h"
#include "qgsserverresponse.h"
%End

  public:

    //! Constructor
    QgsService();

    //! Destructor
    virtual ~QgsService();

    /**
     * @return the name of the service
     */
    virtual QString name() const = 0;

    /**
     * @return the version of the service
     */
    virtual QString version() const = 0;

    /**
     * Return true if the given method is supported for that
     * service.
     * @param method QGSMethodType the
     * @return QString containing the configuration file path
     */
    virtual bool allowMethod( QgsServerRequest::Method ) const = 0;

    /**
     * Execute the requests and set result in QgsServerRequest
     * @param request a QgsServerRequest instance
     * @param response a QgsServerResponse instance
     */
    virtual void executeRequest( const QgsServerRequest& request, QgsServerResponse& response ) = 0;
};

