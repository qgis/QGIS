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

#endif
