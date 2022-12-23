/***************************************************************************
    qgsspatialiteexpressioncompiler.h
    ----------------------------------------------------
    date                 : March 2022
    copyright            : (C) 202 by Alessandro Pasotti
    email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSPATIALITEEXPRESSIONCOMPILER_H
#define QGSSPATIALITEEXPRESSIONCOMPILER_H

#include "qgssqliteexpressioncompiler.h"
#include "qgsexpression.h"
#include "qgsspatialitefeatureiterator.h"

class QgsSpatialiteExpressionCompiler : public QgsSQLiteExpressionCompiler
{
  public:

    explicit QgsSpatialiteExpressionCompiler( const QgsFields &fields, bool ignoreStaticNodes = false );

  protected:

    QString sqlFunctionFromFunctionName( const QString &fnName ) const override;

};

#endif // QGSSPATIALITEEXPRESSIONCOMPILER_H
