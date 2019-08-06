/***************************************************************************
                              qgsmapserviceexception.h
                              ------------------------
  begin                : June 13, 2006
  copyright            : (C) 2006 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPSERVICEEXCEPTION
#define QGSMAPSERVICEEXCEPTION

#define SIP_NO_FILE

#include <QString>

#include "qgsserverexception.h"
#include "qgis_server.h"

/**
 * \ingroup server
 * \class  QgsMapServiceException
 * \brief Exception class for WMS service exceptions (for compatibility only).
 *
 *
 * The most important codes are:
 *  * "InvalidFormat"
 *  * "Invalid CRS"
 *  * "LayerNotDefined" / "StyleNotDefined"
 *  * "OperationNotSupported"
 * \deprecated Use QsgServerException
 */
class SERVER_EXPORT QgsMapServiceException : public QgsOgcServiceException
{
  public:

    /**
     * Constructor for QgsMapServiceException.
     * \param code HTTP error code
     * \param message Exception message to return to the client
     */
    QgsMapServiceException( const QString &code, const QString &message )
      : QgsOgcServiceException( code, message )
    {}
};

#endif
