/***************************************************************************
                              qgserverexception.h
                              ------------------------
  begin                : January 11, 2017
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

#ifndef QGSSERVEREXCEPTION_H
#define QGSSERVEREXCEPTION_H


#include <QString>
#include <QByteArray>

#include "qgsexception.h"
#include "qgis_server.h"
#include "qgis_sip.h"
#include "nlohmann/json.hpp"

#ifndef SIP_RUN
using namespace nlohmann;
#endif


/**
 * \ingroup server
 * \class  QgsServerException
 * \brief Exception base class for server exceptions.
 * \since QGIS 3.0
 */
#ifndef SIP_RUN
class SERVER_EXPORT QgsServerException : public QgsException
{
#else
class SERVER_EXPORT QgsServerException
{
#endif
  public:
    //! Constructor
    QgsServerException( const QString &message, int responseCode = 500 );

    /**
     * \returns the return HTTP response code associated with this exception
     */
    int responseCode() const { return mResponseCode; }

    /**
     * Formats the exception for sending to client
     *
     * \param responseFormat QString to store the content type of the response format.
     * \returns QByteArray The formatted response.
     *
     * The default implementation returns text/xml format.
     */
    virtual QByteArray formatResponse( QString &responseFormat SIP_OUT ) const;

  private:
    int mResponseCode;
};

/**
 * \ingroup server
 * \class  QgsOgcServiceException
 * \brief Exception base class for service exceptions.
 *
 * Note that this exception is associated with a default return code 200 which may be
 * not appropriate in some situations.
 *
 * \since QGIS 3.0
 */
#ifndef SIP_RUN
class SERVER_EXPORT QgsOgcServiceException : public QgsServerException
{
#else
class SERVER_EXPORT QgsOgcServiceException
{
#endif
  public:
    //! Construction
    QgsOgcServiceException( const QString &code, const QString &message, const QString &locator = QString(),
                            int responseCode = 200, const QString &version = QStringLiteral( "1.3.0" ) );

    //! Returns the exception message
    QString message() const { return mMessage; }

    //! Returns the exception code
    QString code()    const { return mCode; }

    //! Returns the locator
    QString locator() const { return mLocator; }

    //! Returns the exception version
    QString version() const { return mVersion; }

    QByteArray formatResponse( QString &responseFormat SIP_OUT ) const override;

  private:
    QString mCode;
    QString mMessage;
    QString mLocator;
    QString mVersion;
};

/**
 * \ingroup server
 * \class  QgsBadRequestException
 * \brief Exception thrown in case of malformed request
 * \since QGIS 3.4
 */
#ifndef SIP_RUN
class SERVER_EXPORT QgsBadRequestException: public QgsOgcServiceException
{
  public:

    /**
     * Constructor for QgsBadRequestException (HTTP error code 400).
     * \param code Error code name
     * \param message Exception message to return to the client
     * \param locator Locator attribute according to OGC specifications
     */
    QgsBadRequestException( const QString &code, const QString &message, const QString &locator = QString() )
      : QgsOgcServiceException( code, message, locator, 400 )
    {}
};
#endif

#ifndef SIP_RUN // No API exceptions for SIP, see python/server/qgsserverexception.sip

/**
 * \ingroup server
 * \class  QgsServerApiException
 * \brief Exception base class for API exceptions.
 *
 * Note that this exception is associated with a default return code 200 which may be
 * not appropriate in some situations.
 *
 * \since QGIS 3.10
 */
class SERVER_EXPORT QgsServerApiException: public QgsServerException
{
  public:
    //! Construction
    QgsServerApiException( const QString &code, const QString &message, const QString &mimeType = QStringLiteral( "application/json" ), int responseCode = 200 )
      : QgsServerException( message, responseCode )
      , mCode( code )
      , mMimeType( mimeType )
    {
    }

    QByteArray formatResponse( QString &responseFormat SIP_OUT ) const override
    {
      responseFormat = mMimeType;
      const json data
      {
        {
          { "code", mCode.toStdString() },
          { "description", what().toStdString() },
        }
      };

      return QByteArray::fromStdString( data.dump() );
    }

  private:
    QString mCode;
    QString mMimeType;
};


/**
 * \ingroup server
 * \class  QgsServerApiInternalServerError
 * \brief Internal server error API exception.
 *
 * Note that this exception is associated with a default return code 500 which may be
 * not appropriate in some situations.
 *
 * \since QGIS 3.10
 */
class SERVER_EXPORT QgsServerApiInternalServerError: public QgsServerApiException
{
  public:
    //! Construction
    QgsServerApiInternalServerError( const QString &message = QStringLiteral( "Internal server error" ), const QString &mimeType = QStringLiteral( "application/json" ), int responseCode = 500 )
      : QgsServerApiException( QStringLiteral( "Internal server error" ), message, mimeType, responseCode )
    {
    }
};


/**
 * \ingroup server
 * \class  QgsServerApiNotFoundError
 * \brief Not found error API exception.
 *
 * Note that this exception is associated with a default return code 404 which may be
 * not appropriate in some situations.
 *
 * \since QGIS 3.10
 */
class SERVER_EXPORT QgsServerApiNotFoundError: public QgsServerApiException
{
  public:
    //! Construction
    QgsServerApiNotFoundError( const QString &message, const QString &mimeType = QStringLiteral( "application/json" ), int responseCode = 404 )
      : QgsServerApiException( QStringLiteral( "API not found error" ), message, mimeType, responseCode )
    {
    }
};


/**
 * \ingroup server
 * \class  QgsServerApiBadRequestException
 * \brief Bad request error API exception.
 *
 * Note that this exception is associated with a default return code 400 which may be
 * not appropriate in some situations.
 *
 * \since QGIS 3.10
 */
class SERVER_EXPORT QgsServerApiBadRequestException: public QgsServerApiException
{
  public:
    //! Construction
    QgsServerApiBadRequestException( const QString &message, const QString &mimeType = QStringLiteral( "application/json" ), int responseCode = 400 )
      : QgsServerApiException( QStringLiteral( "Bad request error" ), message, mimeType, responseCode )
    {
    }
};


/**
 * \ingroup server
 * \class  QgsServerApiPermissionDeniedException
 * \brief Forbidden (permission denied) 403
 *
 * Note that this exception is associated with a default return code 403 which may be
 * not appropriate in some situations.
 *
 * \since QGIS 3.12
 */
class SERVER_EXPORT QgsServerApiPermissionDeniedException: public QgsServerApiException
{
  public:
    //! Construction
    QgsServerApiPermissionDeniedException( const QString &message, const QString &mimeType = QStringLiteral( "application/json" ), int responseCode = 403 )
      : QgsServerApiException( QStringLiteral( "Forbidden" ), message, mimeType, responseCode )
    {
    }
};

/**
 * \ingroup server
 * \class  QgsServerApiImproperlyConfiguredException
 * \brief  configuration error on the server prevents to serve the request, which would be valid otherwise.
 *
 * Note that this exception is associated with a default return code 500 which may be
 * not appropriate in some situations.
 *
 * \since QGIS 3.10
 */
class SERVER_EXPORT QgsServerApiImproperlyConfiguredException: public QgsServerApiException
{
  public:
    //! Construction
    QgsServerApiImproperlyConfiguredException( const QString &message, const QString &mimeType = QStringLiteral( "application/json" ), int responseCode = 500 )
      : QgsServerApiException( QStringLiteral( "Improperly configured error" ), message, mimeType, responseCode )
    {
    }
};


/**
 * \ingroup server
 * \class  QgsServerApiNotImplementedException
 * \brief  this method is not yet implemented
 *
 * Note that this exception is associated with a default return code 500 which may be
 * not appropriate in some situations.
 *
 * \since QGIS 3.10
 */
class SERVER_EXPORT QgsServerApiNotImplementedException: public QgsServerApiException
{
  public:
    //! Construction
    QgsServerApiNotImplementedException( const QString &message = QStringLiteral( "Requested method is not implemented" ), const QString &mimeType = QStringLiteral( "application/json" ), int responseCode = 500 )
      : QgsServerApiException( QStringLiteral( "Not implemented error" ), message, mimeType, responseCode )
    {
    }
};


/**
 * \ingroup server
 * \class  QgsServerApiInvalidMimeTypeException
 * \brief  the client sent an invalid mime type in the "Accept" header
 *
 * Note that this exception is associated with a default return code 406
 *
 * \since QGIS 3.10
 */
class SERVER_EXPORT QgsServerApiInvalidMimeTypeException: public QgsServerApiException
{
  public:
    //! Construction
    QgsServerApiInvalidMimeTypeException( const QString &message = QStringLiteral( "The Accept header submitted in the request did not support any of the media types supported by the server for the requested resource" ), const QString &mimeType = QStringLiteral( "application/json" ), int responseCode = 406 )
      : QgsServerApiException( QStringLiteral( "Invalid mime-type" ), message, mimeType, responseCode )
    {
    }
};
#endif // no API exceptions for SIP

#endif
