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
  else
  {
    QVariant res = evalNode( parent, context );
    return res;
  }
}

bool QgsExpressionNode::prepare( QgsExpression *parent, const QgsExpressionContext *context )
{
  mHasCachedValue = false;
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

void QgsExpressionNode::cloneTo( QgsExpressionNode *target ) const
{
  target->mHasCachedValue = mHasCachedValue;
  target->mCachedStaticValue = mCachedStaticValue;
  target->parserLastColumn = parserLastColumn;
  target->parserLastLine = parserLastLine;
  target->parserFirstColumn = parserFirstColumn;
  target->parserFirstLine = parserFirstLine;
}

