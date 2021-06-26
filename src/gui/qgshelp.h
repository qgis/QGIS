/***************************************************************************
  qgshelp.h
  --------------------------------------
  Date                 : December 2016
  Copyright            : (C) 2016 by Alexander Bruy
  Email                : alexander dot bruy at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSHELP_H
#define QGSHELP_H

#include <QtCore>

#include "qgis_gui.h"

/**
 * \ingroup gui
 * \class QgsHelp
 * \brief Helper class for showing help topic URI for the given key.
 *
 * Help can be stored online, on the local directory or on the intranet
 * server. Location of the QGIS help can be configured in QGIS options.
 * Multiple locations are supported, they will be used in order of
 * preference, from top to bottom.
 *
 * URI construction takes in account following information:
 *
 * - QGIS version
 * - language of the QGIS UI
 *
 * If no help found, default error page with information how to setup
 * help system will be shown.
 *
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsHelp
{
  public:

    /**
     * Opens help topic for the given help key using default system
     * web browser. If help topic not found, builtin error page shown.
     * \param key key which identified help topic
     * \since QGIS 3.0
     */
    static void openHelp( const QString &key );

    /**
     * Returns URI of the help topic for the given key. If help topic
     * not found, URI of the builtin error page returned.
     * \param key key which identified help topic
     * \since QGIS 3.0
     */
    static QUrl helpUrl( const QString &key );

  private:

    /**
     * Check if given URL accessible by issuing HTTP HEAD request.
     * Returns TRUE if URL accessible, FALSE otherwise.
     * \param url URL to check
     * \since QGIS 3.0
     */
    static bool urlExists( const QString &url );
};

#endif // QGSHELP_H
