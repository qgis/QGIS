/***************************************************************************
              qgspostgresexpressioncompiler.cpp
              ----------------------------------------------------
              date                 : 22.4.2015
              copyright            : (C) 2015 by Matthias Kuhn
              email                : matthias (at) opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspostgresexpressioncompiler.h"
#include "qgssqlexpressioncompiler.h"

QgsPostgresExpressionCompiler::QgsPostgresExpressionCompiler( QgsPostgresFeatureSource* source )
    : QgsSqlExpressionCompiler( source->mFields )
{
}

QString QgsPostgresExpressionCompiler::quotedIdentifier( const QString& identifier )
{
  return QgsPostgresConn::quotedIdentifier( identifier );
}

QString QgsPostgresExpressionCompiler::quotedValue( const QVariant& value, bool& ok )
{
  ok = true;
  return QgsPostgresConn::quotedValue( value );
}

