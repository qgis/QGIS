/***************************************************************************
                             qgsspatialiteexpressioncompiler.h
                             ---------------------------------
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

#ifndef QGSSPATIALITEEXPRESSIONCOMPILER_H
#define QGSSPATIALITEEXPRESSIONCOMPILER_H

#include "qgssqlexpressioncompiler.h"
#include "qgsexpression.h"
#include "qgsspatialitefeatureiterator.h"

class QgsSpatiaLiteExpressionCompiler : public QgsSqlExpressionCompiler
{
  public:

    explicit QgsSpatiaLiteExpressionCompiler( QgsSpatiaLiteFeatureSource* source );

  protected:

    virtual Result compileNode( const QgsExpression::Node* node, QString& str ) override;
    virtual QString quotedIdentifier( const QString& identifier ) override;
    virtual QString quotedValue( const QVariant& value, bool& ok ) override;

};

#endif // QGSSPATIALITEEXPRESSIONCOMPILER_H
