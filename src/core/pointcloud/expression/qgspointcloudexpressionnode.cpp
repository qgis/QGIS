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

#include "qgsexpressionnode.h"
#include "qgsexpressionnodeimpl.h"
#include "qgspointcloudexpression.h"
#include "qgspointcloudexpressionnodeimpl.h"

#include <QString>

using namespace Qt::StringLiterals;

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

QgsPointCloudExpressionNode::QgsPointCloudExpressionNode( const QgsPointCloudExpressionNode & ) = default;

QgsPointCloudExpressionNode &QgsPointCloudExpressionNode::operator=( const QgsPointCloudExpressionNode & ) = default;

void QgsPointCloudExpressionNode::cloneTo( QgsPointCloudExpressionNode *target ) const
{
  target->mHasCachedValue = mHasCachedValue;
  target->mCachedStaticValue = mCachedStaticValue;
  target->parserLastColumn = parserLastColumn;
  target->parserLastLine = parserLastLine;
  target->parserFirstColumn = parserFirstColumn;
  target->parserFirstLine = parserFirstLine;
}

std::unique_ptr<QgsPointCloudExpressionNode> QgsPointCloudExpressionNode::convert( const QgsExpressionNode *expressionNode, QString &error )
{
  error.clear();
  if ( !expressionNode )
    return nullptr;
  switch ( expressionNode->nodeType() )
  {
    case QgsExpressionNode::NodeType::ntFunction:
    {
      error = u"Functions are not supported in point cloud expressions"_s;
      return nullptr;
    }
    case QgsExpressionNode::NodeType::ntBetweenOperator:
    {
      error = u"Between predicate is not supported in point cloud expressions"_s;
      return nullptr;
    }
    case QgsExpressionNode::NodeType::ntCondition:
    {
      error = u"Conditional Statements are not supported in point cloud expressions"_s;
      return nullptr;
    }
    case QgsExpressionNode::NodeType::ntIndexOperator:
    {
      error = u"Index operators are not supported in point cloud expressions"_s;
      return nullptr;
    }
    case QgsExpressionNode::NodeType::ntLiteral:
    {
      const QgsExpressionNodeLiteral *n = qgis::down_cast<const QgsExpressionNodeLiteral *>( expressionNode );
      bool ok;
      const double value = n->value().toDouble( &ok );
      if ( !ok )
      {
        error = u"Literal %1 cannot be converted to double"_s.arg( n->value().toString() );
        return nullptr;
      }
      return std::make_unique<QgsPointCloudExpressionNodeLiteral>( value );
    }
    case QgsExpressionNode::NodeType::ntBinaryOperator:
    {
      const QgsExpressionNodeBinaryOperator *n = qgis::down_cast<const QgsExpressionNodeBinaryOperator *>( expressionNode );
      QgsPointCloudExpressionNodeBinaryOperator::BinaryOperator op;
      if ( !QgsPointCloudExpressionNodeBinaryOperator::convert( n->op(), op ) )
      {
        error = u"Unsupported binary operator %1"_s.arg( n->text() );
        return nullptr;
      }
      std::unique_ptr<QgsPointCloudExpressionNode> opLeft( convert( n->opLeft(), error ) );
      if ( !opLeft )
      {
        return nullptr;
      }
      std::unique_ptr<QgsPointCloudExpressionNode> opRight( convert( n->opRight(), error ) );
      if ( !opRight )
      {
        return nullptr;
      }
      return std::make_unique<QgsPointCloudExpressionNodeBinaryOperator>( op, std::move( opLeft ), std::move( opRight ) );
    }
    case QgsExpressionNode::NodeType::ntColumnRef:
    {
      const QgsExpressionNodeColumnRef *n = qgis::down_cast<const QgsExpressionNodeColumnRef *>( expressionNode );
      return std::make_unique<QgsPointCloudExpressionNodeAttributeRef>( n->name() );
    }
    case QgsExpressionNode::NodeType::ntInOperator:
    {
      const QgsExpressionNodeInOperator *n = qgis::down_cast<const QgsExpressionNodeInOperator *>( expressionNode );
      std::unique_ptr<QgsPointCloudExpressionNode> node( convert( n->node(), error ) );
      if ( !node )
      {
        return nullptr;
      }
      const bool notIn = n->isNotIn();
      auto nodeList = std::make_unique< QgsPointCloudExpressionNode::NodeList >();
      const QList<QgsExpressionNode *> nNodeList = n->list()->list();
      for ( const QgsExpressionNode *nd : nNodeList )
      {
        std::unique_ptr<QgsPointCloudExpressionNode> convertedNode( convert( nd, error ) );
        if ( !convertedNode )
        {
          return nullptr;
        }
        nodeList->append( std::move( convertedNode ) );
      }
      return std::make_unique<QgsPointCloudExpressionNodeInOperator>( std::move( node ), std::move( nodeList ), notIn );
    }
    case QgsExpressionNode::NodeType::ntUnaryOperator:
    {
      const QgsExpressionNodeUnaryOperator *n = qgis::down_cast<const QgsExpressionNodeUnaryOperator *>( expressionNode );
      QgsPointCloudExpressionNodeUnaryOperator::UnaryOperator op;
      if ( !QgsPointCloudExpressionNodeUnaryOperator::convert( n->op(), op ) )
      {
        error = u"Unsupported unary operator %1"_s.arg( n->text() );
        return nullptr;
      }
      std::unique_ptr<QgsPointCloudExpressionNode> operand( convert( n->operand(), error ) );
      if ( !operand )
      {
        return nullptr;
      }
      return std::make_unique<QgsPointCloudExpressionNodeUnaryOperator>( op, std::move( operand ) );
    }
  }
  Q_ASSERT( false );
  return nullptr;
}
