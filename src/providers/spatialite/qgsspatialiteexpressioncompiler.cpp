/***************************************************************************
    qgsspatialiteexpressioncompiler.cpp
    ----------------------------------------------------
    date                 : March 2022
    copyright            : (C) 2022 by Alessandro Pasotti
    email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsspatialiteexpressioncompiler.h"
#include "qgsexpressionnodeimpl.h"

QgsSpatialiteExpressionCompiler::QgsSpatialiteExpressionCompiler( const QgsFields &fields, bool ignoreStaticNodes )
  : QgsSQLiteExpressionCompiler( fields, ignoreStaticNodes )
{
}

QString QgsSpatialiteExpressionCompiler::sqlFunctionFromFunctionName( const QString &fnName ) const
{
  static const QMap<QString, QString> FN_NAMES
  {
    { "abs", "abs" },
    { "char", "char" },
    { "coalesce", "coalesce" },
    { "lower", "lower" },
    { "round", "round" },
    { "trim", "trim" },
    { "upper", "upper" },
    // { "make_datetime", "" }, Not reliable when Z doesn't match
    { "make_date", "" },
    { "make_time", "" },
  };

  return FN_NAMES.value( fnName, QString() );
}


