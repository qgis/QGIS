/***************************************************************************
                                  qgshstoreutils.h
                              ---------------------
    begin                : Sept 2018
    copyright            : (C) 2018 by Mathieu Pellerin
    email                : nirvn dot asia at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSHSTOREUTILS_H
#define QGSHSTOREUTILS_H

#include "qgis_core.h"
#include "qgis.h"

/**
 * \ingroup core
 * \brief Provides utility functions for handling hstore-formatted strings.
 * \since QGIS 3.4
 */
class CORE_EXPORT QgsHstoreUtils
{
  public:

    /**
     * Returns a QVariantMap object containing the key and values from a hstore-formatted string.
     * \param string The hstored-formatted string
     * \since QGIS 3.4
     */
    static QVariantMap parse( const QString &string );

    /**
     * Build a hstore-formatted string from a QVariantMap.
     * \param map The map to format as a string
     * \since QGIS 3.4
     */
    static QString build( const QVariantMap &map );

};

#endif //QGSHSTOREUTILS_H
