/***************************************************************************

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

#ifndef QGSPOSTGRESEXPRESSIONCOMPILER_H
#define QGSPOSTGRESEXPRESSIONCOMPILER_H

#include "qgsexpression.h"
#include "qgspostgresfeatureiterator.h"

class QgsPostgresExpressionCompiler
{
  public:
    enum Result
    {
      None,
      Complete,
      Partial,
      Fail
    };

    QgsPostgresExpressionCompiler( QgsPostgresFeatureSource* source );
    ~QgsPostgresExpressionCompiler();

    Result compile( const QgsExpression* exp );

    const QString& result() { return mResult; }

  private:
    Result compile( const QgsExpression::Node* node, QString& str );

  private:
    QString mResult;
    QgsPostgresFeatureSource* mSource;
};

#endif // QGSPOSTGRESEXPRESSIONCOMPILER_H
