/***************************************************************************
                             qgsogrexpressioncompiler.cpp
                             ----------------------------
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

#include "qgsogrexpressioncompiler.h"
#include "qgsogrprovider.h"

QgsOgrExpressionCompiler::QgsOgrExpressionCompiler( QgsOgrFeatureSource* source )
    : mResult( None )
    , mSource( source )
{
}

QgsOgrExpressionCompiler::~QgsOgrExpressionCompiler()
{

}

QgsOgrExpressionCompiler::Result QgsOgrExpressionCompiler::compile( const QgsExpression* exp )
{
  if ( exp->rootNode() )
    return compile( exp->rootNode(), mResult );
  else
    return Fail;
}

QgsOgrExpressionCompiler::Result QgsOgrExpressionCompiler::compile( const QgsExpression::Node* node, QString& result )
{
  switch ( node->nodeType() )
  {
    case QgsExpression::ntUnaryOperator:
    {
      const QgsExpression::NodeUnaryOperator* n = static_cast<const QgsExpression::NodeUnaryOperator*>( node );
      switch ( n->op() )
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
      const QgsExpression::NodeBinaryOperator* n = static_cast<const QgsExpression::NodeBinaryOperator*>( node );

      QString op;
      bool partialCompilation = false;
      switch ( n->op() )
      {
        case QgsExpression::boEQ:
          if ( n->opLeft()->nodeType() == QgsExpression::ntColumnRef && n->opRight()->nodeType() == QgsExpression::ntColumnRef )
          {
            // equality between column refs results in a partial compilation, since OGR will case-insensitive match strings
            // in columns
            partialCompilation = true;
          }
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

        case QgsExpression::boIs:
          op = "IS";
          break;

        case QgsExpression::boIsNot:
          op = "IS NOT";
          break;

        case QgsExpression::boLike:
          op = "LIKE";
          break;

        case QgsExpression::boILike:
          op = "ILIKE";
          return Fail; //disabled until https://trac.osgeo.org/gdal/ticket/5132 is fixed

        case QgsExpression::boNotLike:
          op = "NOT LIKE";
          break;

        case QgsExpression::boNotILike:
          op = "NOT ILIKE";
          return Fail; //disabled until https://trac.osgeo.org/gdal/ticket/5132 is fixed

        case QgsExpression::boOr:
          op = "OR";
          break;

        case QgsExpression::boAnd:
          op = "AND";
          break;

        case QgsExpression::boNE:
          op = "<>";
          break;

        case QgsExpression::boMul:
          op = "*";
          break;

        case QgsExpression::boPlus:
          op = "+";
          break;

        case QgsExpression::boMinus:
          op = "-";
          break;

        case QgsExpression::boDiv:
        case QgsExpression::boMod:
        case QgsExpression::boConcat:
        case QgsExpression::boIntDiv:
        case QgsExpression::boPow:
        case QgsExpression::boRegexp:
          return Fail; //not supported
      }

      if ( op.isNull() )
        return Fail;

      QString left;
      Result lr( compile( n->opLeft(), left ) );

      QString right;
      Result rr( compile( n->opRight(), right ) );

      result = left + ' ' + op + ' ' + right;
      if ( lr == Complete && rr == Complete )
        return ( partialCompilation ? Partial : Complete );
      else if (( lr == Partial && rr == Complete ) || ( lr == Complete && rr == Partial ) || ( lr == Partial && rr == Partial ) )
        return Partial;
      else
        return Fail;
    }

    case QgsExpression::ntLiteral:
    {
      const QgsExpression::NodeLiteral* n = static_cast<const QgsExpression::NodeLiteral*>( node );
      result = QgsOgrUtils::quotedValue( n->value() );

      // OGR SQL is case insensitive, so if literal was a string then we only have a Partial compilation and need to
      // double check results using QGIS' expression engine

      if ( n->value().type() == QVariant::String )
        return Partial;
      else
        return Complete;
    }

    case QgsExpression::ntColumnRef:
    {
      const QgsExpression::NodeColumnRef* n = static_cast<const QgsExpression::NodeColumnRef*>( node );

      if ( mSource->mFields.fieldNameIndex( n->name() ) == -1 )
        // Not a provider field
        return Fail;

      result = mSource->mProvider->quotedIdentifier( n->name().toUtf8() );

      return Complete;
    }

    case QgsExpression::ntInOperator:
    {
      const QgsExpression::NodeInOperator* n = static_cast<const QgsExpression::NodeInOperator*>( node );
      QStringList list;

      Result inResult = Complete;
      Q_FOREACH ( const QgsExpression::Node* ln, n->list()->list() )
      {
        QString s;
        Result r = compile( ln, s );
        if ( r == Complete || r == Partial )
        {
          list << s;
          if ( r == Partial )
            inResult = Partial;
        }
        else
          return r;
      }

      QString nd;
      Result rn = compile( n->node(), nd );
      if ( rn != Complete && rn != Partial )
        return rn;

      result = QString( "%1 %2IN(%3)" ).arg( nd, n->isNotIn() ? "NOT " : "", list.join( "," ) );
      return ( inResult == Partial || rn == Partial ) ? Partial : Complete;
    }

    case QgsExpression::ntFunction:
    case QgsExpression::ntCondition:
      break;
  }

  return Fail;
}
