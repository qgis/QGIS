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
#ifndef QGSSERVERREQUEST_H
#define QGSSERVERREQUEST_H

#include <QUrl> 

/**
 * \ingroup server
 * QgsServerRequest
 * Class defining request interface passed to services QgsService::executeRequest() method
 */

// Note about design: this intreface must be passed along to python and thus signatures methods must be 
// compatible with pyQGIS/pyQT api and rules.

class SERVER_EXPORT QgsServerRequest
{
    public:
         
       enum Method {
           HeadMethod, PutMethod, GetMethod, PostMethod, DeleteMethod 
       };

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
        * @param method the request method
        */
       QgsServerRequest( const QUrl& url, Method method = GetMethod );

       //! destructor
       virtual ~QgsServerRequest();

       /**
        * @return the value of the header field for that request
        */
       virtual QString getHeader( const QString& name ) const; 

       /**
        * @return  the request url
        */
       virtual QUrl url() const; 

       /**
        * @return the request method
        */
       virtual Method method() const;

       /**
        * Return post/put data
        * Check for QByteArray::isNull() to check if data
        * is available.
        */
        virtual QByteArray data() const;

    protected:
        QUrl        mUrl;
        Method      mMethod;

};

#endif
