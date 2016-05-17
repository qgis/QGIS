/***************************************************************************
                             qgssqlexpressioncompiler.cpp
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

#include "qgssqlexpressioncompiler.h"

QgsSqlExpressionCompiler::QgsSqlExpressionCompiler( const QgsFields& fields, const Flags& flags )
    : mResult( None )
    , mFields( fields )
    , mFlags( flags )
{
}

QgsSqlExpressionCompiler::~QgsSqlExpressionCompiler()
{

}

QgsSqlExpressionCompiler::Result QgsSqlExpressionCompiler::compile( const QgsExpression* exp )
{
  if ( exp->rootNode() )
    return compileNode( exp->rootNode(), mResult );
  else
    return Fail;
}

QString QgsSqlExpressionCompiler::quotedIdentifier( const QString& identifier )
{
  QString quoted = identifier;
  quoted.replace( '"', "\"\"" );
  quoted = quoted.prepend( '\"' ).append( '\"' );
  return quoted;
}

QString QgsSqlExpressionCompiler::quotedValue( const QVariant& value, bool& ok )
{
  ok = true;

  if ( value.isNull() )
    return "NULL";

  switch ( value.type() )
  {
    case QVariant::Int:
    case QVariant::LongLong:
    case QVariant::Double:
      return value.toString();

    case QVariant::Bool:
      return value.toBool() ? "TRUE" : "FALSE";

    default:
    case QVariant::String:
      QString v = value.toString();
      v.replace( '\'', "''" );
      if ( v.contains( '\\' ) )
        return v.replace( '\\', "\\\\" ).prepend( "E'" ).append( '\'' );
      else
        return v.prepend( '\'' ).append( '\'' );
  }
}

QgsSqlExpressionCompiler::Result QgsSqlExpressionCompiler::compileNode( const QgsExpression::Node* node, QString& result )
{
  switch ( node->nodeType() )
  {
    case QgsExpression::ntUnaryOperator:
    {
      const QgsExpression::NodeUnaryOperator* n = static_cast<const QgsExpression::NodeUnaryOperator*>( node );
      switch ( n->op() )
      {
        case QgsExpression::uoNot:
        {
          QString right;
          if ( compileNode( n->operand(), right ) == Complete )
          {
            result = "( NOT " + right + ')';
            return Complete;
          }

          return Fail;
        }

        case QgsExpression::uoMinus:
        {
          if ( mFlags.testFlag( NoUnaryMinus ) )
            return Fail;

          QString right;
          if ( compileNode( n->operand(), right ) == Complete )
          {
            result = "( - (" + right + "))";
            return Complete;
          }

          return Fail;
        }
      }

      break;
    }

    case QgsExpression::ntBinaryOperator:
    {
      const QgsExpression::NodeBinaryOperator* n = static_cast<const QgsExpression::NodeBinaryOperator*>( node );

      QString op;
      bool partialCompilation = false;
      bool failOnPartialNode = false;
      switch ( n->op() )
      {
        case QgsExpression::boEQ:
          if ( mFlags.testFlag( CaseInsensitiveStringMatch ) && n->opLeft()->nodeType() == QgsExpression::ntColumnRef && n->opRight()->nodeType() == QgsExpression::ntColumnRef )
          {
            // equality between column refs results in a partial compilation, since provider is performing
            // case-insensitive matches between strings
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
          failOnPartialNode = mFlags.testFlag( CaseInsensitiveStringMatch );
          break;

        case QgsExpression::boLike:
          op = "LIKE";
          partialCompilation = mFlags.testFlag( LikeIsCaseInsensitive );
          break;

        case QgsExpression::boILike:
          if ( mFlags.testFlag( LikeIsCaseInsensitive ) )
            op = "LIKE";
          else
            op = "ILIKE";
          break;

        case QgsExpression::boNotLike:
          op = "NOT LIKE";
          partialCompilation = mFlags.testFlag( LikeIsCaseInsensitive );
          failOnPartialNode = mFlags.testFlag( CaseInsensitiveStringMatch );
          break;

        case QgsExpression::boNotILike:
          failOnPartialNode = mFlags.testFlag( CaseInsensitiveStringMatch );
          if ( mFlags.testFlag( LikeIsCaseInsensitive ) )
            op = "NOT LIKE";
          else
            op = "NOT ILIKE";
          break;

        case QgsExpression::boOr:
          if ( mFlags.testFlag( NoNullInBooleanLogic ) )
          {
            if ( nodeIsNullLiteral( n->opLeft() ) || nodeIsNullLiteral( n->opRight() ) )
              return Fail;
          }

          op = "OR";
          break;

        case QgsExpression::boAnd:
          if ( mFlags.testFlag( NoNullInBooleanLogic ) )
          {
            if ( nodeIsNullLiteral( n->opLeft() ) || nodeIsNullLiteral( n->opRight() ) )
              return Fail;
          }

          op = "AND";
          break;

        case QgsExpression::boNE:
          failOnPartialNode = mFlags.testFlag( CaseInsensitiveStringMatch );
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
          return Fail;  // handle cast to real

        case QgsExpression::boMod:
          op = "%";
          break;

        case QgsExpression::boConcat:
          op = "||";
          break;

        case QgsExpression::boIntDiv:
          return Fail;  // handle cast to int

        case QgsExpression::boPow:
          op = "^";
          break;

        case QgsExpression::boRegexp:
          op = "~";
          break;
      }

      if ( op.isNull() )
        return Fail;

      QString left;
      Result lr( compileNode( n->opLeft(), left ) );

      QString right;
      Result rr( compileNode( n->opRight(), right ) );

      if ( failOnPartialNode && ( lr == Partial || rr == Partial ) )
        return Fail;

      result = '(' + left + ' ' + op + ' ' + right + ')';
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
      bool ok = false;
      if ( mFlags.testFlag( CaseInsensitiveStringMatch ) && n->value().type() == QVariant::String )
      {
        // provider uses case insensitive matching, so if literal was a string then we only have a Partial compilation and need to
        // double check results using QGIS' expression engine
        result = quotedValue( n->value(), ok );
        return ok ? Partial : Fail;
      }
      else
      {
        result = quotedValue( n->value(), ok );
        return ok ? Complete : Fail;
      }
    }

    case QgsExpression::ntColumnRef:
    {
      const QgsExpression::NodeColumnRef* n = static_cast<const QgsExpression::NodeColumnRef*>( node );

      if ( mFields.indexFromName( n->name() ) == -1 )
        // Not a provider field
        return Fail;

      result = quotedIdentifier( n->name() );

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
        Result r = compileNode( ln, s );
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
      Result rn = compileNode( n->node(), nd );
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

bool QgsSqlExpressionCompiler::nodeIsNullLiteral( const QgsExpression::Node* node ) const
{
  if ( node->nodeType() != QgsExpression::ntLiteral )
    return false;

  const QgsExpression::NodeLiteral* nLit = static_cast<const QgsExpression::NodeLiteral*>( node );
  return nLit->value().isNull();
}
