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

QgsSqlExpressionCompiler::QgsSqlExpressionCompiler( const QgsFields& fields, Flags flags )
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
  quoted.replace( '"', QLatin1String( "\"\"" ) );
  quoted = quoted.prepend( '\"' ).append( '\"' );
  return quoted;
}

QString QgsSqlExpressionCompiler::quotedValue( const QVariant& value, bool& ok )
{
  ok = true;

  if ( value.isNull() )
    return QStringLiteral( "NULL" );

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
      v.replace( '\'', QLatin1String( "''" ) );
      if ( v.contains( '\\' ) )
        return v.replace( '\\', QLatin1String( "\\\\" ) ).prepend( "E'" ).append( '\'' );
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

          op = QStringLiteral( "=" );
          break;

        case QgsExpression::boGE:
          op = QStringLiteral( ">=" );
          break;

        case QgsExpression::boGT:
          op = QStringLiteral( ">" );
          break;

        case QgsExpression::boLE:
          op = QStringLiteral( "<=" );
          break;

        case QgsExpression::boLT:
          op = QStringLiteral( "<" );
          break;

        case QgsExpression::boIs:
          op = QStringLiteral( "IS" );
          break;

        case QgsExpression::boIsNot:
          op = QStringLiteral( "IS NOT" );
          failOnPartialNode = mFlags.testFlag( CaseInsensitiveStringMatch );
          break;

        case QgsExpression::boLike:
          op = QStringLiteral( "LIKE" );
          partialCompilation = mFlags.testFlag( LikeIsCaseInsensitive );
          break;

        case QgsExpression::boILike:
          if ( mFlags.testFlag( LikeIsCaseInsensitive ) )
            op = QStringLiteral( "LIKE" );
          else
            op = QStringLiteral( "ILIKE" );
          break;

        case QgsExpression::boNotLike:
          op = QStringLiteral( "NOT LIKE" );
          partialCompilation = mFlags.testFlag( LikeIsCaseInsensitive );
          failOnPartialNode = mFlags.testFlag( CaseInsensitiveStringMatch );
          break;

        case QgsExpression::boNotILike:
          failOnPartialNode = mFlags.testFlag( CaseInsensitiveStringMatch );
          if ( mFlags.testFlag( LikeIsCaseInsensitive ) )
            op = QStringLiteral( "NOT LIKE" );
          else
            op = QStringLiteral( "NOT ILIKE" );
          break;

        case QgsExpression::boOr:
          if ( mFlags.testFlag( NoNullInBooleanLogic ) )
          {
            if ( nodeIsNullLiteral( n->opLeft() ) || nodeIsNullLiteral( n->opRight() ) )
              return Fail;
          }

          op = QStringLiteral( "OR" );
          break;

        case QgsExpression::boAnd:
          if ( mFlags.testFlag( NoNullInBooleanLogic ) )
          {
            if ( nodeIsNullLiteral( n->opLeft() ) || nodeIsNullLiteral( n->opRight() ) )
              return Fail;
          }

          op = QStringLiteral( "AND" );
          break;

        case QgsExpression::boNE:
          failOnPartialNode = mFlags.testFlag( CaseInsensitiveStringMatch );
          op = QStringLiteral( "<>" );
          break;

        case QgsExpression::boMul:
          op = QStringLiteral( "*" );
          break;

        case QgsExpression::boPlus:
          op = QStringLiteral( "+" );
          break;

        case QgsExpression::boMinus:
          op = QStringLiteral( "-" );
          break;

        case QgsExpression::boDiv:
          return Fail;  // handle cast to real

        case QgsExpression::boMod:
          op = QStringLiteral( "%" );
          break;

        case QgsExpression::boConcat:
          op = QStringLiteral( "||" );
          break;

        case QgsExpression::boIntDiv:
          return Fail;  // handle cast to int

        case QgsExpression::boPow:
          op = QStringLiteral( "^" );
          break;

        case QgsExpression::boRegexp:
          op = QStringLiteral( "~" );
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

      result = QStringLiteral( "%1 %2IN(%3)" ).arg( nd, n->isNotIn() ? "NOT " : "", list.join( QStringLiteral( "," ) ) );
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
