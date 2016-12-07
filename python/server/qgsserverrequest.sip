/***************************************************************************
                          qgsserverrequest.h

  Define ruquest class for getting request contents
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
 * Class defining request intreface passed to services QgsService::executeRequest() method
 *
 *  Note about design: this intreface must be passed along to python and thus signatures methods must be 
 *  compatible with pyQGS/pyQT api and rules.
 */
class QgsServerRequest
{
%TypeHeaderCode
#include "qgsserverrequest.h"
%End
    public:
         
       enum Method {
           HeadMethod, PutMethod, GetMethod, PostMethod, DeleteMethod 
       };

       /**
        * Constructor
        *
        * @param url the lurl string
        * @param method the request method
        */
       QgsServerRequest( const QString& url, Method method );
       
       /**
        * Constructor
        *
        * @param url QUrl
        * @param method the rquest method
        */
       QgsServerRequest( const QUrl& url, Method method );

       //! destructor
       virtual ~QgsServerRequest();

       /**
        * @return  the request url
        */
       virtual const QUrl& url() const; 

       /**
        * @return the rquest method
        */
       virtual Method method() const;

       /**
        * Return post/put data
        * The default implementation retfurn nullptr
        * @return a QByteArray pointer or nullptr
        */
       virtual const QByteArray* data() const;

};

