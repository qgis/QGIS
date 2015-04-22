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

#include "qgspostgresexpressioncompiler.h"

QgsPostgresExpressionCompiler::QgsPostgresExpressionCompiler( QgsPostgresFeatureSource* source )
  : mResult( None )
  , mSource( source )
{
}

QgsPostgresExpressionCompiler::~QgsPostgresExpressionCompiler()
{

}

QgsPostgresExpressionCompiler::Result QgsPostgresExpressionCompiler::compile( const QgsExpression* exp )
{
  if ( exp->rootNode() )
    return compile ( exp->rootNode(), mResult );
  else
    return Fail;
}

QgsPostgresExpressionCompiler::Result QgsPostgresExpressionCompiler::compile( const QgsExpression::Node* node, QString& result )
{
  switch( node->nodeType() )
  {
    case QgsExpression::ntUnaryOperator:
    {
      const QgsExpression::NodeUnaryOperator* n = static_cast<const QgsExpression::NodeUnaryOperator*>( node );
      switch( n->op() )
      {
        case QgsExpression::uoNot:
          break;

        case QgsExpression::uoMinus:
          break;
      }

      break;
    }

    case QgsExpression::ntBinaryOperator:
    {
      QString op;
      QString left;
      QString right;
      Result lr;
      Result rr;

      const QgsExpression::NodeBinaryOperator* n = static_cast<const QgsExpression::NodeBinaryOperator*>( node );

      switch ( n->op() )
      {
        case QgsExpression::boEQ:
          op = "=";
          break;

        case QgsExpression::boGE:
          op = ">=";
          break;

        case QgsExpression::boGT:
          op = ">";
          break;

        case QgsExpression::boLE:
          op = "<=";
          break;

        case QgsExpression::boLT:
          op = "<";
          break;

        case QgsExpression::boLike:
          op = "LIKE";
          break;

        case QgsExpression::boILike:
          op = "ILIKE";
          break;

        case QgsExpression::boOr:
          op = "OR";
          break;

        case QgsExpression::boAnd:
          op = "AND";
          break;

        case QgsExpression::boNE:
          op = "<>";
          break;
      }

      if ( !op.isNull() )
      {
        lr = compile( n->opLeft(), left );
        rr = compile( n->opRight(), right );
        result = left + " " + op + " " + right;
        return ( lr == Complete && rr == Complete ) ? Complete : Fail;
      }
      else
      {
        return Fail;
      }

      break;
    }

    case QgsExpression::ntFunction:
      return Fail;
      break;

    case QgsExpression::ntLiteral:
    {
      const QgsExpression::NodeLiteral* n = static_cast<const QgsExpression::NodeLiteral*>( node );
      result = QgsPostgresConn::quotedValue( n->value() );
      return Complete;
    }

    case QgsExpression::ntColumnRef:
    {
      const QgsExpression::NodeColumnRef* n = static_cast<const QgsExpression::NodeColumnRef*>( node );

      if ( mSource->mFields.indexFromName( n->name() ) != -1 )
      {
        result = QgsPostgresConn::quotedIdentifier( n->name() );
        return Complete;
      }
      else
      {
        // Not a provider field
        return Fail;
      }
    }
  }
  return Fail;
}
