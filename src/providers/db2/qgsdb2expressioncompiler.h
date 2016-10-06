/***************************************************************************
  qgsdb2expressioncompiler.h - DB2 expression compiler
  --------------------------------------
  Date      : 2016-01-27
  Copyright : (C) 2016 by David Adler
                          Shirley Xiao, David Nguyen
  Email     : dadler at adtechgeospatial.com
              xshirley2012 at yahoo.com, davidng0123 at gmail.com
****************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/

#ifndef QGSDB2LEXPRESSIONCOMPILER_H
#define QGSDB2LEXPRESSIONCOMPILER_H

#include "qgssqlexpressioncompiler.h"
#include "qgsexpression.h"
#include "qgsdb2featureiterator.h"

class QgsDb2ExpressionCompiler : public QgsSqlExpressionCompiler
{
  public:

    explicit QgsDb2ExpressionCompiler( QgsDb2FeatureSource* source );

  protected:
    virtual Result compileNode( const QgsExpression::Node* node, QString& result ) override;
    virtual QString quotedValue( const QVariant& value, bool& ok ) override;

};

#endif // QGSDB2EXPRESSIONCOMPILER_H
