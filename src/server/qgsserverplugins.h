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

#define SIP_NO_FILE


#include "qgsserverinterface.h"

// This is needed by SIP otherwise it doesn't find QgsPythonUtils header
class QgsPythonUtils;

/**
 * \ingroup server
 * \brief Initializes Python server plugins and stores a list of server plugin names
 */
class SERVER_EXPORT QgsServerPlugins
{
  public:
    /**
     * Default constructor for QgsServerPlugins.
     */
    explicit QgsServerPlugins() = default;

    /**
     * Initializes the Python plugins
     * \param interface QgsServerInterface
     * \returns bool TRUE on success
     */
    static bool initPlugins( QgsServerInterface *interface );
    //! List of available server plugin names
    static QStringList &serverPlugins();
    //! Pointer to QgsPythonUtils
    static QgsPythonUtils *sPythonUtils;
};

#endif // QGSSERVERPLUGINS_H
