/***************************************************************************
                             qgsogrexpressioncompiler.h
                             --------------------------
    begin                : November 2015
    copyright            : (C) 2015 Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSOGREXPRESSIONCOMPILER_H
#define QGSOGREXPRESSIONCOMPILER_H

#include "qgsexpression.h"
#include "qgsogrfeatureiterator.h"
#include "qgssqlexpressioncompiler.h"

class QgsOgrExpressionCompiler : public QgsSqlExpressionCompiler
{
  public:

    explicit QgsOgrExpressionCompiler( QgsOgrFeatureSource* source );

    virtual Result compile( const QgsExpression* exp ) override;

  protected:

    virtual Result compileNode( const QgsExpression::Node* node, QString& str ) override;
    virtual QString quotedIdentifier( const QString& identifier ) override;
    virtual QString quotedValue( const QVariant& value, bool& ok ) override;

  private:

    QgsOgrFeatureSource* mSource;
};

#endif // QGSOGREXPRESSIONCOMPILER_H
