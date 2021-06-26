/***************************************************************************
                               qgsexpressionnode.cpp
                             -------------------
    begin                : May 2017
    copyright            : (C) 2017 Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
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
  if ( isStatic( parent, context ) )
  {
    mCachedStaticValue = evalNode( parent, context );
    if ( !parent->hasEvalError() )
      mHasCachedValue = true;
    else
      mHasCachedValue = false;
    return true;
  }
  else
  {
    mHasCachedValue = false;
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

