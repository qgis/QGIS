/***************************************************************************
                              qgswfsserviceexception.h
                              ------------------------
  begin                : January 17, 2017
  copyright            : (C) 2017 by David Marteau
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

#ifndef QGSWFSSERVICEEXCEPTION_H
#define QGSWFSSERVICEEXCEPTION_H

#include <QString>

#include "qgsserverexception.h"

namespace QgsWfs
{

  /**
   * \ingroup server
   * \class  QgsserviceException
   * \brief Exception class for WFS service exceptions.
   */
  class QgsServiceException : public QgsOgcServiceException
  {
    public:
      QgsServiceException( const QString &code, const QString &message,
                           int responseCode = 200 )
        : QgsOgcServiceException( code, message, QString(), responseCode, QStringLiteral( "1.2.0" ) )
      {}

      QgsServiceException( const QString &code, const QString &message, const QString &locator,
                           int responseCode = 200 )
        : QgsOgcServiceException( code, message, locator, responseCode, QStringLiteral( "1.2.0" ) )
      {}

  };


  /**
   * \ingroup server
   * \class  QgsSecurityAccessException
   * \brief Exception thrown when data access violates access controls
   */
  class QgsSecurityAccessException: public QgsServiceException
  {
    public:
      QgsSecurityAccessException( const QString &message, const QString &locator = QString() )
        : QgsServiceException( QStringLiteral( "Security" ), message, locator, 403 )
      {}
  };

  /**
   * \ingroup server
   * \class  QgsRequestNotWellFormedException
   * \brief Exception thrown in case of malformed request
   */
  class QgsRequestNotWellFormedException: public QgsServiceException
  {
    public:
      QgsRequestNotWellFormedException( const QString &message, const QString &locator = QString() )
        : QgsServiceException( QStringLiteral( "RequestNotWellFormed" ), message, locator, 400 )
      {}
  };

  /**
   * \ingroup server
   * \class  QgsBadRequestException
   * \brief Exception thrown in case of malformed request
   */
  class QgsBadRequestException: public QgsServiceException
  {
    public:
      QgsBadRequestException( const QString &code, const QString &message, const QString &locator = QString() )
        : QgsServiceException( code, message, locator, 400 )
      {}
  };


} // namespace QgsWfs

#endif

