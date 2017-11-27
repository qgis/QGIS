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
   * \class  QgsserviceException
   * \brief Exception class for WMS service exceptions.
   *
   * The most important codes are:
   *  * "InvalidFormat"
   *  * "Invalid CRS"
   *  * "LayerNotDefined" / "StyleNotDefined"
   *  * "OperationNotSupported"
   */
  class QgsServiceException : public QgsOgcServiceException
  {
    public:
      QgsServiceException( const QString &code, const QString &message, const QString &locator = QString(),
                           int responseCode = 200 )
        : QgsOgcServiceException( code, message, locator, responseCode, QStringLiteral( "1.3.0" ) )
      {}

      QgsServiceException( const QString &code, const QString &message, int responseCode )
        : QgsOgcServiceException( code, message, QString(), responseCode, QStringLiteral( "1.3.0" ) )
      {}

  };

  /**
   * \ingroup server
   * \class  QgsSecurityException
   * \brief Exception thrown when data access violates access controls
   */
  class QgsSecurityException: public QgsServiceException
  {
    public:
      QgsSecurityException( const QString &message, const QString &locator = QString() )
        : QgsServiceException( QStringLiteral( "Security" ), message, locator, 403 )
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


} // namespace QgsWms

#endif
