/***************************************************************************
                     qgsmssqlsqlquerybuilder.cpp
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

#include "qgsmssqlsqlquerybuilder.h"

#include "qgsmssqlutils.h"

QString QgsMsSqlSqlQueryBuilder::createLimitQueryForTable( const QString &schema, const QString &name, int limit ) const
{
  if ( schema.isEmpty() )
    return u"SELECT TOP %1 * FROM %2"_s.arg( limit ).arg( QgsMssqlUtils::quotedIdentifier( name ) );
  else
    return u"SELECT TOP %1 * FROM %2.%3"_s.arg( limit ).arg( QgsMssqlUtils::quotedIdentifier( schema ), QgsMssqlUtils::quotedIdentifier( name ) );
}

QString QgsMsSqlSqlQueryBuilder::quoteIdentifier( const QString &identifier ) const
{
  return QgsMssqlUtils::quotedIdentifier( identifier );
}
