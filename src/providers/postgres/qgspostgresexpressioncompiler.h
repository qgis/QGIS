/***************************************************************************
              qgspostgresexpressioncompiler.h
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

#include "qgssqlexpressioncompiler.h"
#include "qgsexpression.h"
#include "qgspostgresfeatureiterator.h"

class QgsPostgresExpressionCompiler : public QgsSqlExpressionCompiler
{
  public:

    explicit QgsPostgresExpressionCompiler( QgsPostgresFeatureSource* source );

  protected:

    virtual QString quotedIdentifier( const QString& identifier ) override;
    virtual QString quotedValue( const QVariant& value, bool& ok ) override;
};

#endif // QGSPOSTGRESEXPRESSIONCOMPILER_H
