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
#include "qgspointcloudexpression.h"


QVariant QgsPointcloudExpressionNode::eval( QgsPointcloudExpression *parent, QVariantMap &p )
{
  if ( mHasCachedValue )
  {
    return mCachedStaticValue;
  }
  else if ( mCompiledSimplifiedNode )
  {
    return mCompiledSimplifiedNode->eval( parent, p );
  }
  else
  {
    QVariant res = evalNode( parent, p );
    return res;
  }
}

bool QgsPointcloudExpressionNode::prepare( QgsPointcloudExpression *parent, const QgsPointCloudAttributeCollection &attributes )
{
  mHasCachedValue = false;
  mCompiledSimplifiedNode.reset();
  if ( isStatic( parent, attributes ) )
  {
    // some calls to isStatic already evaluate the node to a cached value, so if that's
    // happened then don't re-evaluate again
    if ( !mHasCachedValue )
    {
      QVariantMap map;
      mCachedStaticValue = evalNode( parent, map );
      if ( !parent->hasEvalError() )
        mHasCachedValue = true;
    }
    return true;
  }
  else
  {
    return prepareNode( parent, attributes );
  }
}

QgsPointcloudExpressionNode::QgsPointcloudExpressionNode( const QgsPointcloudExpressionNode &other )
  : parserFirstLine( other.parserFirstLine )
  , parserFirstColumn( other.parserFirstColumn )
  , parserLastLine( other.parserLastLine )
  , parserLastColumn( other.parserLastColumn )
  , mHasCachedValue( other.mHasCachedValue )
  , mCachedStaticValue( other.mCachedStaticValue )
  , mCompiledSimplifiedNode( other.mCompiledSimplifiedNode ? other.mCompiledSimplifiedNode->clone() : nullptr )
{

}

QgsPointcloudExpressionNode &QgsPointcloudExpressionNode::operator=( const QgsPointcloudExpressionNode &other )
{
  parserFirstLine = other.parserFirstLine;
  parserFirstColumn = other.parserFirstColumn;
  parserLastLine = other.parserLastLine;
  parserLastColumn = other.parserLastColumn;
  mHasCachedValue = other.mHasCachedValue;
  mCachedStaticValue = other.mCachedStaticValue;
  mCompiledSimplifiedNode.reset( other.mCompiledSimplifiedNode ? other.mCompiledSimplifiedNode->clone() : nullptr );
  return *this;
}

void QgsPointcloudExpressionNode::cloneTo( QgsPointcloudExpressionNode *target ) const
{
  target->mHasCachedValue = mHasCachedValue;
  target->mCachedStaticValue = mCachedStaticValue;
  if ( mCompiledSimplifiedNode )
    target->mCompiledSimplifiedNode.reset( mCompiledSimplifiedNode->clone() );
  target->parserLastColumn = parserLastColumn;
  target->parserLastLine = parserLastLine;
  target->parserFirstColumn = parserFirstColumn;
  target->parserFirstLine = parserFirstLine;
}

