/***************************************************************************
   qgshanaexpressioncompiler.cpp
   --------------------------------------
   Date      : 31-05-2019
   Copyright : (C) SAP SE
   Author    : Maksim Rylov
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#include "qgsexpressionnodeimpl.h"
#include "qgshanaexpressioncompiler.h"
#include "qgshanautils.h"
#include "qgssqlexpressioncompiler.h"

QgsHanaExpressionCompiler::QgsHanaExpressionCompiler(QgsHanaFeatureSource *source)
  : QgsSqlExpressionCompiler(source->mFields, QgsSqlExpressionCompiler::IntegerDivisionResultsInInteger)
  , mGeometryColumn(source->mGeometryColumn)
{
}

QString QgsHanaExpressionCompiler::quotedIdentifier(const QString &identifier)
{
  return QgsHanaUtils::quotedIdentifier(identifier);
}

QString QgsHanaExpressionCompiler::quotedValue(const QVariant &value, bool &ok)
{
  ok = true;
  switch (value.type())
  {
  case QVariant::Bool:
    return value.toBool() ? "(1=1)" : "(1=0)";

  default:
    return QgsHanaUtils::quotedValue(value);
  }
}

static const QMap<QString, QString> FUNCTION_NAMES_SQL_FUNCTIONS_MAP
{
  // mathematical functions
  { "sign", "sign" },
  { "abs", "abs" },
  { "round", "round" },
  // geometry functions
  { "x", "ST_X" },
  { "y", "ST_Y" },
  { "x_min", "ST_XMin" },
  { "y_min", "ST_YMin" },
  { "x_max", "ST_XMax" },
  { "y_max", "ST_YMax" },
  { "area", "ST_Area" },
  { "length", "ST_Length" },
  { "perimeter", "ST_Perimeter" },
  { "intersects", "ST_Intersects" },
  { "crosses", "ST_Crosses" },
  { "contains", "ST_Contains" },
  { "overlaps", "ST_Overlaps" },
  { "within", "ST_Within" },
  { "translate", "ST_Translate" },
  { "buffer", "ST_Buffer" },
  { "centroid", "ST_Centroid" },
  { "point_on_surface", "ST_PointOnSurface" },
  { "distance", "ST_Distance" },
  { "geom_from_wkt", "ST_GeomFromWKT" },
  { "lower", "lower" },
  { "trim", "trim" },
  { "upper", "upper" },
};

QString QgsHanaExpressionCompiler::sqlFunctionFromFunctionName(const QString &fnName) const
{
  return FUNCTION_NAMES_SQL_FUNCTIONS_MAP.value(fnName, QString());
}

QString QgsHanaExpressionCompiler::castToReal(const QString &value) const
{
  return QStringLiteral("CAST((%1) AS REAL)").arg(value);
}

QString QgsHanaExpressionCompiler::castToInt(const QString &value) const
{
  return QStringLiteral("CAST((%1) AS INTEGER)").arg(value);
}

QString QgsHanaExpressionCompiler::castToText(const QString &value) const
{
  return QStringLiteral("CAST((%1) AS NVARCHAR)").arg(value);
}

QgsSqlExpressionCompiler::Result QgsHanaExpressionCompiler::compileNode(
  const QgsExpressionNode *node, QString &result)
{
  switch (node->nodeType())
  {
  case QgsExpressionNode::ntFunction:
  {
    const QgsExpressionNodeFunction *n = static_cast<const QgsExpressionNodeFunction *>(node);

    QgsExpressionFunction *fd = QgsExpression::Functions()[n->fnIndex()];
    if (fd->name() == QLatin1String("$geometry"))
    {
      result = quotedIdentifier(mGeometryColumn);
      return Complete;
    }

    FALLTHROUGH;
  }
  default:
    break;
  }

  return QgsSqlExpressionCompiler::compileNode(node, result);
}
