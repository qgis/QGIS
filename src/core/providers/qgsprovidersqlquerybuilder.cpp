/***************************************************************************
                     qgsprovidersqlquerybuilder.cpp
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

#include "qgsprovidersqlquerybuilder.h"

#include "qgssqliteutils.h"

QgsProviderSqlQueryBuilder::~QgsProviderSqlQueryBuilder() = default;

QString QgsProviderSqlQueryBuilder::createLimitQueryForTable( const QString &schema, const QString &name, int limit ) const
{
  if ( schema.isEmpty() )
    return u"SELECT * FROM %1 LIMIT %2"_s.arg( quoteIdentifier( name ) ).arg( limit );
  else
    return u"SELECT * FROM %1.%2 LIMIT %3"_s.arg( quoteIdentifier( schema ), quoteIdentifier( name ) ).arg( limit );
}

QString QgsProviderSqlQueryBuilder::quoteIdentifier( const QString &identifier ) const
{
  // TODO: handle backend-specific identifier quoting...
  return QgsSqliteUtils::quotedIdentifier( identifier );
}
