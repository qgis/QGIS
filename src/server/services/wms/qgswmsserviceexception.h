/***************************************************************************
                              qgsserviceexception.h
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

#ifndef QGSWMSSERVICEEXCEPTION_H
#define QGSWMSSERVICEEXCEPTION_H

#include <QString>

#include "qgsserverexception.h"

namespace QgsWms
{

  /**
   * \ingroup server
   * \class  QgsWms::QgsServiceException
   * \brief Exception class for WMS service exceptions.
   *
   * The most important codes are:
   *  * "InvalidFormat"
   *  * "Invalid CRS"
   *  * "LayerNotDefined" / "StyleNotDefined"
   *  * "OperationNotSupported"
   *
   * \since QGIS 3.0
   */
  class QgsServiceException : public QgsOgcServiceException
  {
    public:

      /**
       * Constructor for QgsServiceException.
       * \param code Error code name
       * \param message Exception message to return to the client
       * \param locator Locator attribute according to OGC specifications
       * \param responseCode HTTP error code
       */
      QgsServiceException( const QString &code, const QString &message, const QString &locator = QString(),
                           int responseCode = 200 )
        : QgsOgcServiceException( code, message, locator, responseCode, QStringLiteral( "1.3.0" ) )
      {}

      /**
       * Constructor for QgsServiceException (empty locator attribute).
       * \param code Error code name
       * \param message Exception message to return to the client
       * \param responseCode HTTP error code
       */
      QgsServiceException( const QString &code, const QString &message, int responseCode )
        : QgsOgcServiceException( code, message, QString(), responseCode, QStringLiteral( "1.3.0" ) )
      {}

  };

  /**
   * \ingroup server
   * \class  QgsWms::QgsSecurityException
   * \brief Exception thrown when data access violates access controls
   * \since QGIS 3.0
   */
  class QgsSecurityException: public QgsServiceException
  {
    public:

      /**
       * Constructor for QgsSecurityException (HTTP error code 403 with
       * Security code name).
       * \param message Exception message to return to the client
       * \param locator Locator attribute according to OGC specifications
       */
      QgsSecurityException( const QString &message, const QString &locator = QString() )
        : QgsServiceException( QStringLiteral( "Security" ), message, locator, 403 )
      {}
  };

  /**
   * \ingroup server
   * \class  QgsWms::QgsBadRequestException
   * \brief Exception thrown in case of malformed request
   * \since QGIS 3.0
   */
  class QgsBadRequestException: public QgsServiceException
  {
    public:

      /**
       * Constructor for QgsBadRequestException (HTTP error code 400).
       * \param code Error code name
       * \param message Exception message to return to the client
       * \param locator Locator attribute according to OGC specifications
       */
      QgsBadRequestException( const QString &code, const QString &message, const QString &locator = QString() )
        : QgsServiceException( code, message, locator, 400 )
      {}
  };
} // namespace QgsWms

#endif
