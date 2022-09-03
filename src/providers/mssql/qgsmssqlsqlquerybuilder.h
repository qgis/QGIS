/***************************************************************************
                     qgsmssqlsqlquerybuilder.h
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
#ifndef QGSMSSQLSQLQUERYBUILDER_H
#define QGSMSSQLSQLQUERYBUILDER_H

#include <QString>

#include "qgsprovidersqlquerybuilder.h"

class  QgsMsSqlSqlQueryBuilder : public QgsProviderSqlQueryBuilder
{

  public:
    QString createLimitQueryForTable( const QString &schema, const QString &name, int limit = 10 ) const override;
    QString quoteIdentifier( const QString &identifier ) const override;
};

#endif // QGSMSSQLSQLQUERYBUILDER_H
