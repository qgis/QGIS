/***************************************************************************
   qgshanaexpressioncompiler.h
   --------------------------------------
   Date      : 31-05-2019
   Copyright : (C) SAP SE
   Author    : Maxim Rylov
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#ifndef QGSHANAEXPRESSIONCOMPILER_H
#define QGSHANAEXPRESSIONCOMPILER_H

#include "qgsexpression.h"
#include "qgshanafeatureiterator.h"
#include "qgssqlexpressioncompiler.h"

class QgsHanaExpressionCompiler : public QgsSqlExpressionCompiler
{
  public:
    explicit QgsHanaExpressionCompiler( QgsHanaFeatureSource *source, bool ignoreStaticNodes = false );

  protected:
    QString quotedIdentifier( const QString &identifier ) override;
    QString quotedValue( const QVariant &value, bool &ok ) override;
    QString sqlFunctionFromFunctionName( const QString &fnName ) const override;
    QStringList sqlArgumentsFromFunctionName( const QString &fnName, const QStringList &fnArgs ) const override;
    QString castToReal( const QString &value ) const override;
    QString castToInt( const QString &value ) const override;
    QString castToText( const QString &value ) const override;
    Result compileNode( const QgsExpressionNode *node, QString &str ) override;

  private:
    QString mGeometryColumn;
};

#endif // QGSHANAEXPRESSIONCOMPILER_H
