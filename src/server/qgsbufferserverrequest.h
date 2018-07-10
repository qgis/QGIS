/***************************************************************************
                          qgsbufferserverrequest.h

  Define response wrapper for storing request in buffer
  -------------------
  begin                : 2017-01-03
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
#ifndef QGSBUFFERSERVERREQUEST_H
#define QGSBUFFERSERVERREQUEST_H

#include "qgis_server.h"
#include "qgsserverrequest.h"

#include <QBuffer>
#include <QByteArray>
#include <QMap>

/**
 * \ingroup server
 * \class QgsBufferServerRequest
 * \brief Class defining request with data
 * \since QGIS 3.0
 */
class SERVER_EXPORT QgsBufferServerRequest : public QgsServerRequest
{
  public:

    /**
    * Constructor
    *
    * \param url the url string
    * \param method the request method, default to GET
    * \param headers optional dictionary of header name-values
    * \param data optional POST data
    */
    QgsBufferServerRequest( const QString &url, QgsServerRequest::Method method = QgsServerRequest::GetMethod, const QgsServerRequest::Headers &headers = QgsServerRequest::Headers(), QByteArray *data = nullptr );

    /**
     * Constructor
     *
     * \param url QUrl
     * \param method the request method, default to GET
     * \param headers optional dictionary of header name-values
     * \param data optional POST data
     */
    QgsBufferServerRequest( const QUrl &url, QgsServerRequest::Method method = QgsServerRequest::GetMethod, const QgsServerRequest::Headers &headers = QgsServerRequest::Headers(), QByteArray *data = nullptr );

    QByteArray data() const override { return mData; }

  private:
    QByteArray mData;
};

#endif
