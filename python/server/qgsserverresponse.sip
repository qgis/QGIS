/***************************************************************************
                          qgsserverresponse.h

  Define response class for services
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
 * QgsServerResponse
 * Class defining response interface passed to services QgsService::executeRequest() method
 *
 *  Note:
 *  This class is intended to be used from python code: method signatures and return types should be 
 *  compatible with pyQGIS/pyQT types and rules.
 *
 */
class QgsServerResponse
{
%TypeHeaderCode
#include "qgsserverresponse.h"
%End
    public:
       
        //!constructor
        QgsServerResponse();

        //! destructor
        virtual ~QgsServerResponse();

       /** Set header entry 
        *  Add header entry to the response 
        *  Note that it is usually an error to set hedaer after writng data
        */
        virtual void setHeader( const QString& key, const QString& value ) = 0;
        
       /** Set the http return code 
        */
        virtual void setReturnCode( int code ) = 0;

        /**
         * Send error 
         */
        virtual void sendError( int code,  const QString& message ) = 0;

        /**
         * Write string 
         */
        virtual void write(const QString& data );

       /**
        * Write chunk af data
        * They are convenience method that will write directly to the 
        * underlying I/O device
        */
        virtual qint64 write(const QByteArray& byteArray );


        /**
         * Return the underlying QIODevice
         */
        virtual QIODevice* io() = 0;

};

