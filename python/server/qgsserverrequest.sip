/***************************************************************************
                          qgsserverrequest.h

  Define request class for getting request contents
  -------------------
  begin                : 2016-12-05
  copyright            : (C) 2016 by David Marteau
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

/**
 * \ingroup server
 * QgsServerRequest
 * Class defining request interface passed to services QgsService::executeRequest() method
 */

class QgsServerRequest
{
%TypeHeaderCode
#include "qgsserverrequest.h"
%End
  public:

    enum Method
    {
      HeadMethod, PutMethod, GetMethod, PostMethod, DeleteMethod
    };

    /**
     * Constructor
     */
    QgsServerRequest();

    /**
     * Constructor
     *
     * @param url the url string
     * @param method the request method
     */
    QgsServerRequest( const QString& url, Method method );

    /**
     * Constructor
     *
     * @param url QUrl
     * @param method the request method (default to GetMethod)
     */
    QgsServerRequest( const QUrl& url, Method method = GetMethod );

    //! destructor
    virtual ~QgsServerRequest();

    /**
     * @return the value of the header field for that request
     */
    virtual QString getHeader( const QString& name ) const;

    /**
     * @return the request url
     */
    QUrl url() const;

    /**
     * @return the request method
     */
    Method method() const;

    /**
     * @return query params
     */
    QMap<QString, QString> parameters() const;

    /**
     * Set a parameter
     */
    void setParameter( const QString& key, const QString& value );

    /**
     * Get a parameter value
     */
    QString getParameter( const QString& key ) const;

    /**
     * Remove a parameter
     */
    void removeParameter( const QString& key );

    /**
     * Return post/put data
     * Check for QByteArray::isNull() to check if data
     * is available.
     */
    virtual QByteArray data() const;

    /**
     * Set the request url
     */
    void setUrl( const QUrl& url );

    /**
     * Set the request method
     */
    void setMethod( Method method );

};

