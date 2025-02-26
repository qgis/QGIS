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
#include "qgis.h"

class QgsField;

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

    /**
     * Converts a SQL Server field system type string to the equivalent QVariant type.
     */
    static QMetaType::Type convertSqlFieldType( const QString &systemTypeName );

    /**
     * Creates the equivalent QgsField corresponding to the properties of a SQL Server field.
     */
    static QgsField createField( const QString &name, const QString &systemTypeName, int length, int precision, int scale, bool nullable, bool unique, bool readOnly );

    /**
     * Converts the string values from .STGeometryType() to a QGIS WKB type.
     */
    static Qgis::WkbType wkbTypeFromGeometryType( const QString &type );
};


#endif // QGSMSSQLUTILS_H
