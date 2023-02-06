/***************************************************************************
                               qgsexpressionnode.cpp
                             -------------------
    begin                : May 2017
    copyright            : (C) 2017 Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsexpressionnode.h"
#include "qgsexpression.h"


QVariant QgsExpressionNode::eval( QgsExpression *parent, const QgsExpressionContext *context )
{
  if ( mHasCachedValue )
  {
    return mCachedStaticValue;
  }
  else if ( mCompiledSimplifiedNode )
  {
    return mCompiledSimplifiedNode->eval( parent, context );
  }
  else
  {
    QVariant res = evalNode( parent, context );
    return res;
  }
}

bool QgsExpressionNode::prepare( QgsExpression *parent, const QgsExpressionContext *context )
{
  mHasCachedValue = false;
  mCompiledSimplifiedNode.reset();
  if ( isStatic( parent, context ) )
  {
    // some calls to isStatic already evaluate the node to a cached value, so if that's
    // happened then don't re-evaluate again
    if ( !mHasCachedValue )
    {
      mCachedStaticValue = evalNode( parent, context );
      if ( !parent->hasEvalError() )
        mHasCachedValue = true;
    }
    return true;
  }
  else
  {
    return prepareNode( parent, context );
  }
}

void QgsExpressionNode::setCachedStaticValue( const QVariant &value ) const
{
  mHasCachedValue = true;
  mCachedStaticValue = value;
}

QgsExpressionNode::QgsExpressionNode( const QgsExpressionNode &other )
  : parserFirstLine( other.parserFirstLine )
  , parserFirstColumn( other.parserFirstColumn )
  , parserLastLine( other.parserLastLine )
  , parserLastColumn( other.parserLastColumn )
  , mHasCachedValue( other.mHasCachedValue )
  , mCachedStaticValue( other.mCachedStaticValue )
  , mCompiledSimplifiedNode( other.mCompiledSimplifiedNode ? other.mCompiledSimplifiedNode->clone() : nullptr )
{

}

QgsExpressionNode &QgsExpressionNode::operator=( const QgsExpressionNode &other )
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

void QgsExpressionNode::cloneTo( QgsExpressionNode *target ) const
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

