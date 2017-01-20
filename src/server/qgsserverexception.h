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

/** \ingroup server
 * \class  QgsServerException
 * \brief Exception base class for server exceptions.
 *
 * @note added in QGIS 3.0
 */
class SERVER_EXPORT QgsServerException : public QgsException
{
  public:
    //! Constructor
    QgsServerException( const QString& message, int responseCode = 500 );

    /**
     * @return the return HTTP response code associated with this exception
     */
    int responseCode() const { return mResponseCode; }

    /** Format the exception for sending to client
     *
     * @param responseFormat QString to store the content type of the response format.
     * @return QByteArray the fermatted response.
     *
     * The defaolt implementation return text/xml format.
     */
    virtual QByteArray formatResponse( QString& responseFormat ) const;

  private:
    int mResponseCode;
};

/** \ingroup server
 * \class  QgsOgcServiceException
 * \brief Exception base class for service exceptions.
 *
 * Note that this exception is associated with a default return code 200 which may be
 * not appropriate in some situations.
 *
 * @note added in QGIS 3.0
 */
class SERVER_EXPORT QgsOgcServiceException : public QgsServerException
{
  public:
    //! Construction
    QgsOgcServiceException( const QString& code, const QString& message, const QString& locator = QString(),
                            int responseCode = 200, const QString& version = QStringLiteral( "1.3.0" ) );

    //! @return message
    QString message() const { return mMessage; }

    //! @return code
    QString code()    const { return mCode; }

    //! @return locator
    QString locator() const { return mLocator; }

    //!return exception version
    QString version() const { return mVersion; }

    //! Overridden from QgsServerException
    virtual QByteArray formatResponse( QString& responseFormat ) const override;

  private:
    QString mCode;
    QString mMessage;
    QString mLocator;
    QString mVersion;
};

#endif

