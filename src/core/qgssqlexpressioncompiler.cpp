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
#include "qgsexpressionnodeimpl.h"
#include "qgsexpressionfunction.h"
#include "qgsexpression.h"

QgsSqlExpressionCompiler::QgsSqlExpressionCompiler( const QgsFields &fields, Flags flags )
  : mResult( None )
  , mFields( fields )
  , mFlags( flags )
{
}

QgsSqlExpressionCompiler::Result QgsSqlExpressionCompiler::compile( const QgsExpression *exp )
{
  if ( exp->rootNode() )
    return compileNode( exp->rootNode(), mResult );
  else
    return Fail;
}

QString QgsSqlExpressionCompiler::result()
{
  return mResult;
}

bool QgsSqlExpressionCompiler::opIsStringComparison( QgsExpressionNodeBinaryOperator::BinaryOperator op )
{
  if ( op == QgsExpressionNodeBinaryOperator::BinaryOperator::boILike ||
       op == QgsExpressionNodeBinaryOperator::BinaryOperator::boLike ||
       op == QgsExpressionNodeBinaryOperator::BinaryOperator::boNotILike ||
       op == QgsExpressionNodeBinaryOperator::BinaryOperator::boNotLike ||
       op == QgsExpressionNodeBinaryOperator::BinaryOperator::boRegexp )
    return true;
  else
    return false;
}

QString QgsSqlExpressionCompiler::quotedIdentifier( const QString &identifier )
{
  QString quoted = identifier;
  quoted.replace( '"', QLatin1String( "\"\"" ) );
  quoted = quoted.prepend( '\"' ).append( '\"' );
  return quoted;
}

QString QgsSqlExpressionCompiler::quotedValue( const QVariant &value, bool &ok )
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
      return value.toBool() ? QStringLiteral( "TRUE" ) : QStringLiteral( "FALSE" );

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

QgsSqlExpressionCompiler::Result QgsSqlExpressionCompiler::compileNode( const QgsExpressionNode *node, QString &result )
{
  switch ( node->nodeType() )
  {
    case QgsExpressionNode::ntUnaryOperator:
    {
      const QgsExpressionNodeUnaryOperator *n = static_cast<const QgsExpressionNodeUnaryOperator *>( node );
      switch ( n->op() )
      {
        case QgsExpressionNodeUnaryOperator::uoNot:
        {
          QString right;
          if ( compileNode( n->operand(), right ) == Complete )
          {
            result = "( NOT " + right + ')';
            return Complete;
          }

          return Fail;
        }

        case QgsExpressionNodeUnaryOperator::uoMinus:
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

    case QgsExpressionNodeBinaryOperator::ntBinaryOperator:
    {
      const QgsExpressionNodeBinaryOperator *n = static_cast<const QgsExpressionNodeBinaryOperator *>( node );

      QString op;
      bool partialCompilation = false;
      bool failOnPartialNode = false;
      switch ( n->op() )
      {
        case QgsExpressionNodeBinaryOperator::boEQ:
          if ( mFlags.testFlag( CaseInsensitiveStringMatch ) && n->opLeft()->nodeType() == QgsExpressionNode::ntColumnRef && n->opRight()->nodeType() == QgsExpressionNode::ntColumnRef )
          {
            // equality between column refs results in a partial compilation, since provider is performing
            // case-insensitive matches between strings
            partialCompilation = true;
          }

          op = QStringLiteral( "=" );
          break;

        case QgsExpressionNodeBinaryOperator::boGE:
          op = QStringLiteral( ">=" );
          break;

        case QgsExpressionNodeBinaryOperator::boGT:
          op = QStringLiteral( ">" );
          break;

        case QgsExpressionNodeBinaryOperator::boLE:
          op = QStringLiteral( "<=" );
          break;

        case QgsExpressionNodeBinaryOperator::boLT:
          op = QStringLiteral( "<" );
          break;

        case QgsExpressionNodeBinaryOperator::boIs:
          op = QStringLiteral( "IS" );
          break;

        case QgsExpressionNodeBinaryOperator::boIsNot:
          op = QStringLiteral( "IS NOT" );
          failOnPartialNode = mFlags.testFlag( CaseInsensitiveStringMatch );
          break;

        case QgsExpressionNodeBinaryOperator::boLike:
          op = QStringLiteral( "LIKE" );
          partialCompilation = mFlags.testFlag( LikeIsCaseInsensitive );
          break;

        case QgsExpressionNodeBinaryOperator::boILike:
          if ( mFlags.testFlag( LikeIsCaseInsensitive ) )
            op = QStringLiteral( "LIKE" );
          else
            op = QStringLiteral( "ILIKE" );
          break;

        case QgsExpressionNodeBinaryOperator::boNotLike:
          op = QStringLiteral( "NOT LIKE" );
          partialCompilation = mFlags.testFlag( LikeIsCaseInsensitive );
          failOnPartialNode = mFlags.testFlag( CaseInsensitiveStringMatch );
          break;

        case QgsExpressionNodeBinaryOperator::boNotILike:
          failOnPartialNode = mFlags.testFlag( CaseInsensitiveStringMatch );
          if ( mFlags.testFlag( LikeIsCaseInsensitive ) )
            op = QStringLiteral( "NOT LIKE" );
          else
            op = QStringLiteral( "NOT ILIKE" );
          break;

        case QgsExpressionNodeBinaryOperator::boOr:
          if ( mFlags.testFlag( NoNullInBooleanLogic ) )
          {
            if ( nodeIsNullLiteral( n->opLeft() ) || nodeIsNullLiteral( n->opRight() ) )
              return Fail;
          }

          op = QStringLiteral( "OR" );
          break;

        case QgsExpressionNodeBinaryOperator::boAnd:
          if ( mFlags.testFlag( NoNullInBooleanLogic ) )
          {
            if ( nodeIsNullLiteral( n->opLeft() ) || nodeIsNullLiteral( n->opRight() ) )
              return Fail;
          }

          op = QStringLiteral( "AND" );
          break;

        case QgsExpressionNodeBinaryOperator::boNE:
          failOnPartialNode = mFlags.testFlag( CaseInsensitiveStringMatch );
          op = QStringLiteral( "<>" );
          break;

        case QgsExpressionNodeBinaryOperator::boMul:
          op = QStringLiteral( "*" );
          break;

        case QgsExpressionNodeBinaryOperator::boPlus:
          op = QStringLiteral( "+" );
          break;

        case QgsExpressionNodeBinaryOperator::boMinus:
          op = QStringLiteral( "-" );
          break;

        case QgsExpressionNodeBinaryOperator::boDiv:
          op = QStringLiteral( "/" );
          break;

        case QgsExpressionNodeBinaryOperator::boMod:
          op = QStringLiteral( "%" );
          break;

        case QgsExpressionNodeBinaryOperator::boConcat:
          op = QStringLiteral( "||" );
          break;

        case QgsExpressionNodeBinaryOperator::boIntDiv:
          op = QStringLiteral( "/" );
          break;

        case QgsExpressionNodeBinaryOperator::boPow:
          op = QStringLiteral( "^" );
          break;

        case QgsExpressionNodeBinaryOperator::boRegexp:
          op = QStringLiteral( "~" );
          break;
      }

      if ( op.isNull() )
        return Fail;

      QString left;
      Result lr( compileNode( n->opLeft(), left ) );

      if ( opIsStringComparison( n ->op() ) )
        left = castToText( left );

      QString right;
      Result rr( compileNode( n->opRight(), right ) );

      if ( failOnPartialNode && ( lr == Partial || rr == Partial ) )
        return Fail;

      if ( n->op() == QgsExpressionNodeBinaryOperator::boDiv && mFlags.testFlag( IntegerDivisionResultsInInteger ) )
      {
        right = castToReal( right );
        if ( right.isEmpty() )
        {
          // not supported
          return Fail;
        }
      }

      result = '(' + left + ' ' + op + ' ' + right + ')';
      if ( n->op() == QgsExpressionNodeBinaryOperator::boIntDiv )
      {
        result = castToInt( result );
        if ( result.isEmpty() )
        {
          // not supported
          return Fail;
        }
      }

      if ( lr == Complete && rr == Complete )
        return ( partialCompilation ? Partial : Complete );
      else if ( ( lr == Partial && rr == Complete ) || ( lr == Complete && rr == Partial ) || ( lr == Partial && rr == Partial ) )
        return Partial;
      else
        return Fail;
    }

    case QgsExpressionNode::ntLiteral:
    {
      const QgsExpressionNodeLiteral *n = static_cast<const QgsExpressionNodeLiteral *>( node );
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

    case QgsExpressionNode::ntColumnRef:
    {
      const QgsExpressionNodeColumnRef *n = static_cast<const QgsExpressionNodeColumnRef *>( node );

      if ( mFields.indexFromName( n->name() ) == -1 )
        // Not a provider field
        return Fail;

      result = quotedIdentifier( n->name() );

      return Complete;
    }

    case QgsExpressionNode::ntInOperator:
    {
      const QgsExpressionNodeInOperator *n = static_cast<const QgsExpressionNodeInOperator *>( node );
      QStringList list;

      Result inResult = Complete;
      const auto constList = n->list()->list();
      for ( const QgsExpressionNode *ln : constList )
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

      result = QStringLiteral( "%1 %2IN (%3)" ).arg( nd, n->isNotIn() ? QStringLiteral( "NOT " ) : QString(), list.join( ',' ) );
      return ( inResult == Partial || rn == Partial ) ? Partial : Complete;
    }

    case QgsExpressionNode::ntFunction:
    {
      const QgsExpressionNodeFunction *n = static_cast<const QgsExpressionNodeFunction *>( node );
      QgsExpressionFunction *fd = QgsExpression::Functions()[n->fnIndex()];

      // get sql function to compile node expression
      QString nd = sqlFunctionFromFunctionName( fd->name() );
      // if no sql function the node can't be compiled
      if ( nd.isEmpty() )
        return Fail;

      // compile arguments
      QStringList args;
      Result inResult = Complete;
      const auto constList = n->args()->list();
      for ( const QgsExpressionNode *ln : constList )
      {
        QString s;
        Result r = compileNode( ln, s );
        if ( r == Complete || r == Partial )
        {
          args << s;
          if ( r == Partial )
            inResult = Partial;
        }
        else
          return r;
      }

      // update arguments to be adapted to SQL function
      args = sqlArgumentsFromFunctionName( fd->name(), args );

      // build result
      result = QStringLiteral( "%1(%2)" ).arg( nd, args.join( ',' ) );
      return inResult == Partial ? Partial : Complete;
    }

    case QgsExpressionNode::ntCondition:
      break;

    case QgsExpressionNode::ntIndexOperator:
      break;
  }

  return Fail;
}

QString QgsSqlExpressionCompiler::sqlFunctionFromFunctionName( const QString &fnName ) const
{
  Q_UNUSED( fnName )
  return QString();
}

QStringList QgsSqlExpressionCompiler::sqlArgumentsFromFunctionName( const QString &fnName, const QStringList &fnArgs ) const
{
  Q_UNUSED( fnName )
  return QStringList( fnArgs );
}

QString QgsSqlExpressionCompiler::castToReal( const QString &value ) const
{
  Q_UNUSED( value )
  return QString();
}

QString QgsSqlExpressionCompiler::castToText( const QString &value ) const
{
  return value;
}

QString QgsSqlExpressionCompiler::castToInt( const QString &value ) const
{
  Q_UNUSED( value )
  return QString();
}

bool QgsSqlExpressionCompiler::nodeIsNullLiteral( const QgsExpressionNode *node ) const
{
  if ( node->nodeType() != QgsExpressionNode::ntLiteral )
    return false;

  const QgsExpressionNodeLiteral *nLit = static_cast<const QgsExpressionNodeLiteral *>( node );
  return nLit->value().isNull();
}
