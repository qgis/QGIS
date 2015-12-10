/***************************************************************************
    qgsoracleexpressioncompiler.h
    ----------------------------------------------------
    date                 : December 2015
    copyright            : (C) 2015 by Jürgen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSORACLEEXPRESSIONCOMPILER_H
#define QGSORACLEEXPRESSIONCOMPILER_H

#include "qgssqlexpressioncompiler.h"
#include "qgsexpression.h"
#include "qgsoraclefeatureiterator.h"

class QgsOracleExpressionCompiler : public QgsSqlExpressionCompiler
{
  public:

    explicit QgsOracleExpressionCompiler( QgsOracleFeatureSource* source );

  protected:
    virtual Result compileNode( const QgsExpression::Node* node, QString& result ) override;
    virtual QString quotedIdentifier( const QString& identifier ) override;
    virtual QString quotedValue( const QVariant& value, bool& ok ) override;
};

#endif // QGSORACLEEXPRESSIONCOMPILER_H
