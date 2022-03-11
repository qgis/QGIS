/***************************************************************************
                       qgspointcloudexpressionnode.cpp
                       -------------------------------
    begin                : January 2022
    copyright            : (C) 2022 Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointcloudexpressionnode.h"
#include "qgspointcloudexpressionnodeimpl.h"
#include "qgspointcloudexpression.h"
#include "qgsexpressionnode.h"
#include "qgsexpressionnodeimpl.h"


double QgsPointCloudExpressionNode::eval( QgsPointCloudExpression *parent, int pointIndex )
{
  if ( mHasCachedValue )
  {
    return mCachedStaticValue;
  }
  else
  {
    double res = evalNode( parent, pointIndex );
    return res;
  }
}

bool QgsPointCloudExpressionNode::prepare( QgsPointCloudExpression *parent, const QgsPointCloudBlock *block )
{
  mHasCachedValue = false;
  if ( isStatic( parent, block ) )
  {
    // some calls to isStatic already evaluate the node to a cached value, so if that's
    // happened then don't re-evaluate again
    if ( !mHasCachedValue )
    {
      int map = 0;
      mCachedStaticValue = evalNode( parent, map );
      if ( !parent->hasEvalError() )
        mHasCachedValue = true;
    }
    return true;
  }
  else
  {
    return prepareNode( parent, block );
  }
}

QgsPointCloudExpressionNode::QgsPointCloudExpressionNode( const QgsPointCloudExpressionNode &other )
  : parserFirstLine( other.parserFirstLine )
  , parserFirstColumn( other.parserFirstColumn )
  , parserLastLine( other.parserLastLine )
  , parserLastColumn( other.parserLastColumn )
  , mHasCachedValue( other.mHasCachedValue )
  , mCachedStaticValue( other.mCachedStaticValue )
{

}

QgsPointCloudExpressionNode &QgsPointCloudExpressionNode::operator=( const QgsPointCloudExpressionNode &other )
{
  parserFirstLine = other.parserFirstLine;
  parserFirstColumn = other.parserFirstColumn;
  parserLastLine = other.parserLastLine;
  parserLastColumn = other.parserLastColumn;
  mHasCachedValue = other.mHasCachedValue;
  mCachedStaticValue = other.mCachedStaticValue;
  return *this;
}

void QgsPointCloudExpressionNode::cloneTo( QgsPointCloudExpressionNode *target ) const
{
  target->mHasCachedValue = mHasCachedValue;
  target->mCachedStaticValue = mCachedStaticValue;
  target->parserLastColumn = parserLastColumn;
  target->parserLastLine = parserLastLine;
  target->parserFirstColumn = parserFirstColumn;
  target->parserFirstLine = parserFirstLine;
}

QgsPointCloudExpressionNode *QgsPointCloudExpressionNode::convert( const QgsExpressionNode *expressionNode, QString &error )
{
  error.clear();
  if ( !expressionNode )
    return nullptr;
  switch ( expressionNode->nodeType() )
  {
    case QgsExpressionNode::NodeType::ntFunction:
    {
      error = QStringLiteral( "Functions are not supported in point cloud expressions" );
      return nullptr;
    }
    case QgsExpressionNode::NodeType::ntCondition:
    {
      error = QStringLiteral( "Conditional Statements are not supported in point cloud expressions" );
      return nullptr;
    }
    case QgsExpressionNode::NodeType::ntIndexOperator:
    {
      error = QStringLiteral( "Index operators are not supported in point cloud expressions" );
      return nullptr;
    }
    case QgsExpressionNode::NodeType::ntLiteral:
    {
      const QgsExpressionNodeLiteral *n = static_cast<const QgsExpressionNodeLiteral *>( expressionNode );
      bool ok;
      const double value = n->value().toDouble( &ok );
      if ( !ok )
      {
        error = QStringLiteral( "Literal %1 cannot be converted to double" ).arg( n->value().toString() );
        return nullptr;
      }
      return new QgsPointCloudExpressionNodeLiteral( value );
    }
    case QgsExpressionNode::NodeType::ntBinaryOperator:
    {
      const QgsExpressionNodeBinaryOperator *n = static_cast<const QgsExpressionNodeBinaryOperator *>( expressionNode );
      QgsPointCloudExpressionNodeBinaryOperator::BinaryOperator op;
      if ( !QgsPointCloudExpressionNodeBinaryOperator::convert( n->op(), op ) )
      {
        error = QStringLiteral( "Unsupported binary operator %1" ).arg( n->text() );
        return nullptr;
      }
      QgsPointCloudExpressionNode *opLeft = convert( n->opLeft(), error );
      if ( !opLeft )
      {
        return nullptr;
      }
      QgsPointCloudExpressionNode *opRight = convert( n->opRight(), error );
      if ( !opRight )
      {
        delete opLeft;
        return nullptr;
      }
      return new QgsPointCloudExpressionNodeBinaryOperator( op, opLeft, opRight );
    }
    case QgsExpressionNode::NodeType::ntColumnRef:
    {
      const QgsExpressionNodeColumnRef *n = static_cast<const QgsExpressionNodeColumnRef *>( expressionNode );
      return new QgsPointCloudExpressionNodeAttributeRef( n->name() );
    }
    case QgsExpressionNode::NodeType::ntInOperator:
    {
      const QgsExpressionNodeInOperator *n = static_cast<const QgsExpressionNodeInOperator *>( expressionNode );
      QgsPointCloudExpressionNode *node = convert( n->node(), error );
      if ( !node )
      {
        return nullptr;
      }
      const bool notIn = n->isNotIn();
      QgsPointCloudExpressionNode::NodeList *nodeList = new QgsPointCloudExpressionNode::NodeList; \
      const QList<QgsExpressionNode *> nNodeList = n->list()->list();
      for ( const QgsExpressionNode *nd : nNodeList )
      {
        QgsPointCloudExpressionNode *convertedNode = convert( nd, error );
        if ( !convertedNode )
        {
          delete node;
          qDeleteAll( nodeList->list() );
          return nullptr;
        }
        nodeList->append( convertedNode );
      }
      return new QgsPointCloudExpressionNodeInOperator( node, nodeList, notIn );
    }
    case QgsExpressionNode::NodeType::ntUnaryOperator:
    {
      const QgsExpressionNodeUnaryOperator *n = static_cast<const QgsExpressionNodeUnaryOperator *>( expressionNode );
      QgsPointCloudExpressionNodeUnaryOperator::UnaryOperator op;
      if ( !QgsPointCloudExpressionNodeUnaryOperator::convert( n->op(), op ) )
      {
        error = QStringLiteral( "Unsupported unary operator %1" ).arg( n->text() );
        return nullptr;
      }
      QgsPointCloudExpressionNode *operand = convert( n->operand(), error );
      if ( !operand )
      {
        return nullptr;
      }
      return new QgsPointCloudExpressionNodeUnaryOperator( op, operand );
    }
  }
  Q_ASSERT( false );
  return nullptr;
}
