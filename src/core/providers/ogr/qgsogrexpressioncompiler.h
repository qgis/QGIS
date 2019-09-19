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

#include "qgsogrfeatureiterator.h"
#include "qgssqlexpressioncompiler.h"
#include "qgis_sip.h"

///@cond PRIVATE
#define SIP_NO_FILE

class QgsExpressionNode;
class QgsExpression;

class QgsOgrExpressionCompiler : public QgsSqlExpressionCompiler
{
  public:

    explicit QgsOgrExpressionCompiler( QgsOgrFeatureSource *source );

    Result compile( const QgsExpression *exp ) override;

  protected:

    Result compileNode( const QgsExpressionNode *node, QString &str ) override;
    QString quotedIdentifier( const QString &identifier ) override;
    QString quotedValue( const QVariant &value, bool &ok ) override;
    QString castToReal( const QString &value ) const override;
    QString castToInt( const QString &value ) const override;

  private:

    QgsOgrFeatureSource *mSource = nullptr;
};

///@endcond
#endif // QGSOGREXPRESSIONCOMPILER_H
