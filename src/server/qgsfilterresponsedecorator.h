/***************************************************************************
                          qgsfilterresponsedecorator.h

  Define response adapter for handling filter's hooks
  -------------------
  begin                : 2017-01-05
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
#ifndef QGSFILTERRESPONSEDECORATOR_H
#define QGSFILTERRESPONSEDECORATOR_H

#include "qgis.h"
#include "qgsserverexception.h"
#include "qgsserverfilter.h"
#include "qgsserverresponse.h"

#define SIP_NO_FILE

/**
 * \ingroup server
 * \class QgsFilterResponseDecorator
 * \brief A decorator for calling filter's hooks.
 */
class QgsFilterResponseDecorator : public QgsServerResponse
{
  public:
    /**
     * Constructor for QgsFilterResponseDecorator.
     * \param filters Map of filters to apply before terminating the response
     * \param response Server response
     */
    QgsFilterResponseDecorator( QgsServerFiltersMap filters, QgsServerResponse &response );

    /**
     * Call filters requestReady() method
     */
    void start() SIP_THROW( QgsServerException ) SIP_VIRTUALERRORHANDLER( server_exception_handler );

    /**
     * Call filters projectReady() method
     * \since QGIS 3.36
     */
    void ready() SIP_THROW( QgsServerException ) SIP_VIRTUALERRORHANDLER( server_exception_handler );

    // QgsServerResponse overrides

    void setHeader( const QString &key, const QString &value ) override { mResponse.setHeader( key, value ); }

    void addHeader( const QString &key, const QString &value ) override { mResponse.addHeader( key, value ); }

    void removeHeader( const QString &key ) override { mResponse.removeHeader( key ); }

    Q_DECL_DEPRECATED QString header( const QString &key ) const override
    {
      Q_NOWARN_DEPRECATED_PUSH
      return mResponse.header( key );
      Q_NOWARN_DEPRECATED_POP
    }

    Q_DECL_DEPRECATED QMap<QString, QString> headers() const override
    {
      Q_NOWARN_DEPRECATED_PUSH
      return mResponse.headers();
      Q_NOWARN_DEPRECATED_POP
    }

    virtual QList<QString> fullHeader( const QString &key ) const override { return mResponse.fullHeader( key ); }

    virtual QMap<QString, QList<QString> > fullHeaders() const override { return mResponse.fullHeaders(); }

    bool headersSent() const override { return mResponse.headersSent(); }

    void setStatusCode( int code ) override { mResponse.setStatusCode( code ); }

    int statusCode() const override { return mResponse.statusCode(); }

    void sendError( int code, const QString &message ) override { mResponse.sendError( code, message ); }

    QIODevice *io() override { return mResponse.io(); }

    void finish() override;

    void flush() override;

    void clear() override { mResponse.clear(); }

    QByteArray data() const override { return mResponse.data(); }

    void truncate() override { mResponse.truncate(); }

    QgsFeedback *feedback() const override { return mResponse.feedback(); }

  private:
    QgsServerFiltersMap mFilters;
    QgsServerResponse &mResponse;
};

#endif
