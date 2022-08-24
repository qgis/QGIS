/***************************************************************************
                     qgsprovidersqlquerybuilder.h
begin                : August 2022
copyright            : (C) 2022 by Nyall Dawson
email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPROVIDERSQLQUERYBUILDER_H
#define QGSPROVIDERSQLQUERYBUILDER_H

#include <QString>

#include "qgis_core.h"
#include "qgis_sip.h"

/**
 * \brief Provides an interface for provider-specific creation of SQL queries.
 *
 * The QgsProviderSqlQueryBuilder provides an interface for creation of SQL queries, which
 * can be overridden for backend provider specific SQL syntax.
 *
 * \ingroup core
 * \since QGIS 3.28
 */
class CORE_EXPORT QgsProviderSqlQueryBuilder
{

  public:

    virtual ~QgsProviderSqlQueryBuilder();

    /**
      * Returns a result size limited SQL query string generated for the given \a schema and table \a name, retrieving all columns for the first \a limit rows.
      *
      * The base class method returns the SQL query "SELECT * FROM table LIMIT 10". Subclasses may return database specific equivalents to this query.
     */
    virtual QString createLimitQueryForTable( const QString &schema, const QString &name, int limit = 10 ) const;

    /**
     * Returns a properly quoted version of a table/schema \a identifier.
     */
    virtual QString quoteIdentifier( const QString &identifier ) const;
};

#endif // QGSPROVIDERSQLQUERYBUILDER_H
