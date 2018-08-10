/***************************************************************************
                              qgswmtsserviceexception.h
                              ------------------------
  begin                : July 23, 2018
  copyright            : (C) 2018 by René-Luc D'Hont
  email                : rldhont at 3liz dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSWMTSSERVICEEXCEPTION_H
#define QGSWMTSSERVICEEXCEPTION_H

#include <QString>

#include "qgsserverexception.h"

namespace QgsWmts
{

  /**
   * \ingroup server
   * \class  QgsWmts::QgsServiceException
   * \brief Exception class for WMTS services
   * \since QGIS 3.4
   */
  class QgsServiceException : public QgsOgcServiceException
  {
    public:

      /**
       * Constructor for QgsServiceException (empty locator attribute).
       * \param code Error code name
       * \param message Exception message to return to the client
       * \param responseCode HTTP error code
       */
      QgsServiceException( const QString &code, const QString &message,
                           int responseCode = 200 )
        : QgsOgcServiceException( code, message, QString(), responseCode, QStringLiteral( "1.0.0" ) )
      {}

      /**
       * Constructor for QgsServiceException.
       * \param code Error code name
       * \param message Exception message to return to the client
       * \param locator Locator attribute according to OGC specifications
       * \param responseCode HTTP error code
       */
      QgsServiceException( const QString &code, const QString &message, const QString &locator,
                           int responseCode = 200 )
        : QgsOgcServiceException( code, message, locator, responseCode, QStringLiteral( "1.0.0" ) )
      {}

  };

  /**
   * \ingroup server
   * \class  QgsWmts::QgsSecurityAccessException
   * \brief Exception thrown when data access violates access controls
   * \since QGIS 3.4
   */
  class QgsSecurityAccessException: public QgsServiceException
  {
    public:

      /**
       * Constructor for QgsSecurityAccessException (Security code name).
       * \param message Exception message to return to the client
       * \param locator Locator attribute according to OGC specifications
       */
      QgsSecurityAccessException( const QString &message, const QString &locator = QString() )
        : QgsServiceException( QStringLiteral( "Security" ), message, locator, 403 )
      {}
  };

  /**
   * \ingroup server
   * \class  QgsWmts::QgsRequestNotWellFormedException
   * \brief Exception thrown in case of malformed request
   * \since QGIS 3.4
   */
  class QgsRequestNotWellFormedException: public QgsServiceException
  {
    public:

      /**
       * Constructor for QgsRequestNotWellFormedException (RequestNotWellFormed code name).
       * \param message Exception message to return to the client
       * \param locator Locator attribute according to OGC specifications
       */
      QgsRequestNotWellFormedException( const QString &message, const QString &locator = QString() )
        : QgsServiceException( QStringLiteral( "RequestNotWellFormed" ), message, locator, 400 )
      {}
  };
} // namespace QgsWmts

#endif

