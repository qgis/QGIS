/***************************************************************************
                              qgsserverplugins.h
                              -------------------------
  begin                : August 28, 2014
  copyright            : (C) 2014 by Alessandro Pasotti - ItOpen
  email                : apasotti at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSERVERPLUGINS_H
#define QGSSERVERPLUGINS_H

#include "qgsrequesthandler.h"
#include "qgspythonutils.h"
#include "qgsserverinterface.h"

class SERVER_EXPORT QgsServerPlugins
{
  public:
    explicit QgsServerPlugins();
    static bool initPlugins( QgsServerInterface* interface );
    static QgsPythonUtils* mPythonUtils;
    static QStringList mServerPlugins;
};

#endif // QGSSERVERPLUGINS_H
