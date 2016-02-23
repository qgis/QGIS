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
#include "qgsserverinterface.h"

// This is needed by SIP otherwise it doesn't find QgsPythonUtils header
class QgsPythonUtils;

/**
 * @brief Init Python server plugins and store a list of server plugin names
 */
class SERVER_EXPORT QgsServerPlugins
{
  public:
    explicit QgsServerPlugins();
    /**
     * Initialise the python plugins
     * @param interface QgsServerInterface
     * @return bool true on success
     */
    static bool initPlugins( QgsServerInterface* interface );
    //! List of available server plugin names
    static QStringList& serverPlugins();
    //! Pointer to QgsPythonUtils
    static QgsPythonUtils* sPythonUtils;
};

#endif // QGSSERVERPLUGINS_H
