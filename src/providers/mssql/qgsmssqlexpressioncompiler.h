/***************************************************************************
 qgsmssqlexpressioncompiler.h
 ----------------------------
 begin                : 9.12.2015
 copyright            : (C) 2015 by Nyall Dawson
 email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMSSQLEXPRESSIONCOMPILER_H
#define QGSMSSQLEXPRESSIONCOMPILER_H

#include "qgssqlexpressioncompiler.h"
#include "qgsexpression.h"
#include "qgsmssqlfeatureiterator.h"

class QgsMssqlExpressionCompiler : public QgsSqlExpressionCompiler
{
  public:
    explicit QgsMssqlExpressionCompiler( QgsMssqlFeatureSource *source, bool ignoreStaticNodes = false );

  protected:
    Result compileNode( const QgsExpressionNode *node, QString &result ) override;
    QString quotedValue( const QVariant &value, bool &ok ) override;
    QString quotedIdentifier( const QString &identifier ) override;
    QString castToReal( const QString &value ) const override;
    QString castToInt( const QString &value ) const override;
    QString sqlFunctionFromFunctionName( const QString &fnName ) const override;
    QStringList sqlArgumentsFromFunctionName( const QString &fnName, const QStringList &fnArgs ) const override;
};

#endif // QGSMSSQLEXPRESSIONCOMPILER_H
