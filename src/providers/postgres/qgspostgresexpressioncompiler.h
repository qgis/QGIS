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
#include "qgspostgresconn.h"
#include "qgspostgresfeatureiterator.h"

class QgsPostgresExpressionCompiler : public QgsSqlExpressionCompiler
{
  public:

    explicit QgsPostgresExpressionCompiler( QgsPostgresFeatureSource *source );

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
    QgsPostgresGeometryColumnType mSpatialColType;
    QgsWkbTypes::Type mDetectedGeomType;
    QgsWkbTypes::Type mRequestedGeomType;
    QString mRequestedSrid;
    QString mDetectedSrid;
};

#endif // QGSPOSTGRESEXPRESSIONCOMPILER_H
