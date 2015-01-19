/***************************************************************************
                              qgsgetrequesthandler.h
                 class for reading from/ writing to HTTP GET
                              -------------------
  begin                : May 16, 2006
  copyright            : (C) 2006 by Marco Hugentobler & Ionut Iosifescu
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

#include "qgshttprequesthandler.h"

#ifndef QGSGETPREQUESTHANDLER_H
#define QGSGETPREQUESTHANDLER_H


class QgsGetRequestHandler: public QgsHttpRequestHandler
{
  public:
    QgsGetRequestHandler();
    void parseInput() override;
};

#endif
