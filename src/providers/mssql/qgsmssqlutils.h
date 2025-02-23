/***************************************************************************
  qgsmssqlutils.h
  --------------------------------------
  Date                 : February 2025
  Copyright            : (C) 2025 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMSSQLUTILS_H
#define QGSMSSQLUTILS_H

#include <QString>
#include <QVariant>

/**
 * Contains utility functions for working with Microsoft SQL Server databases.
 */
class QgsMssqlUtils
{
  public:
    /**
     * Returns a quoted string version of \a value, for safe use in a SQL query.
     */
    static QString quotedValue( const QVariant &value );

    /**
     * Returns a quoted string version of a database \a identifier, for safe use in a SQL query.
     */
    static QString quotedIdentifier( const QString &identifier );
};


#endif // QGSMSSQLUTILS_H
