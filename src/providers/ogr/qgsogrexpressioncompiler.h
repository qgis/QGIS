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

class QgsOgrExpressionCompiler
{
  public:
    enum Result
    {
      None,
      Complete,
      Partial,
      Fail
    };

    explicit QgsOgrExpressionCompiler( QgsOgrFeatureSource* source );
    ~QgsOgrExpressionCompiler();

    Result compile( const QgsExpression* exp );

    const QString& result() { return mResult; }

  private:
    Result compile( const QgsExpression::Node* node, QString& str );

  private:
    QString mResult;
    QgsOgrFeatureSource* mSource;
};

#endif // QGSOGREXPRESSIONCOMPILER_H
