/***************************************************************************
                              qgspostrequesthandler.h
                            ------------------------------
  begin                :  2011
  copyright            : (C) 2011 by Marco Hugentobler
  email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOSTREQUESTHANDLER_H
#define QGSPOSTREQUESTHANDLER_H

#include "qgshttprequesthandler.h"

/**Request handler for HTTP POST*/
class QgsPostRequestHandler: public QgsHttpRequestHandler
{
  public:
    QgsPostRequestHandler();
    ~QgsPostRequestHandler();

    /**Parses the input and creates a request neutral Parameter/Value map*/
    std::map<QString, QString> parseInput();
};

#endif // QGSPOSTREQUESTHANDLER_H
