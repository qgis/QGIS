/***************************************************************************
   qgsredshiftexpressioncompiler.h
   --------------------------------------
   Date      : 16.02.2021
   Copyright : (C) 2021 Amazon Inc. or its affiliates
   Author    : Marcel Bezdrighin
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#ifndef QGSREDSHIFTEXPRESSIONCOMPILER_H
#define QGSREDSHIFTEXPRESSIONCOMPILER_H

#include "qgsexpression.h"
#include "qgsredshiftconn.h"
#include "qgsredshiftfeatureiterator.h"
#include "qgssqlexpressioncompiler.h"

class QgsRedshiftExpressionCompiler : public QgsSqlExpressionCompiler
{
  public:
    explicit QgsRedshiftExpressionCompiler( QgsRedshiftFeatureSource *source );

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
    QgsRedshiftGeometryColumnType mSpatialColType;
    Qgis::WkbType mDetectedGeomType;
    Qgis::WkbType mRequestedGeomType;
    QString mRequestedSrid;
    QString mDetectedSrid;
};

#endif // QGSREDSHIFTEXPRESSIONCOMPILER_H
