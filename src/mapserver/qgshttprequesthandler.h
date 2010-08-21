/***************************************************************************
                              qgshttprequesthandler.h
                              -----------------------
  begin                : June 29, 2007
  copyright            : (C) 2007 by Marco Hugentobler
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

#ifndef QGSHTTPREQUESTHANDLER_H
#define QGSHTTPREQUESTHANDLER_H

#include "qgsrequesthandler.h"

/**Base class for request handler using HTTP.
It provides a method to send data to the client*/
class QgsHttpRequestHandler: public QgsRequestHandler
{
  public:
    QgsHttpRequestHandler();
    ~QgsHttpRequestHandler();

  protected:
    void sendHttpResponse( QByteArray* ba, const QString& format ) const;
};

#endif
