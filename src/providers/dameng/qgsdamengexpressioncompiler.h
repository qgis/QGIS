/***************************************************************************
    qgsdamengexpressioncompiler.h
    ----------------------------------------------------
    begin                : 2025/01/14
    copyright            : ( C ) 2025 by Haiyang Zhao
    email                : zhaohaiyang@dameng.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   ( at your option ) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSDAMENGEXPRESSIONCOMPILER_H
#define QGSDAMENGEXPRESSIONCOMPILER_H

#include "qgssqlexpressioncompiler.h"
#include "qgsexpression.h"
#include "qgsdamengconn.h"
#include "qgsdamengfeatureiterator.h"

class QgsDamengExpressionCompiler : public QgsSqlExpressionCompiler
{
  public:
    explicit QgsDamengExpressionCompiler( QgsDamengFeatureSource *source, bool ignoreStaticNodes = false );

  protected:
    QString quotedIdentifier( const QString &identifier ) override;
    QString quotedValue( const QVariant &value, bool &ok ) override;
    Result compileNode( const QgsExpressionNode *node, QString &str ) override;
    QString sqlFunctionFromFunctionName( const QString &fnName ) const override;
    QStringList sqlArgumentsFromFunctionName( const QString &fnName, const QStringList &fnArgs ) const override;
    QString castToReal( const QString &value ) const override;
    QString castToInt( const QString &value ) const override;
    QString castToText( const QString &value ) const override;

    QString mGeometryColumn;
    QgsDamengGeometryColumnType mSpatialColType;
    Qgis::WkbType mDetectedGeomType;
    Qgis::WkbType mRequestedGeomType;
    QString mRequestedSrid;
    QString mDetectedSrid;
};

#endif // QGSDAMENGEXPRESSIONCOMPILER_H
