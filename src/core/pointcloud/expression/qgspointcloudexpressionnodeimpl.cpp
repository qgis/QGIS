/***************************************************************************
                      qgspointcloudexpressionnodeimpl.cpp
                      -----------------------------------
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

#include "qgspointcloudexpressionnodeimpl.h"
#include "qgspointcloudexpression.h"

const char *QgsPointCloudExpressionNodeBinaryOperator::BINARY_OPERATOR_TEXT[] =
{
  // this must correspond (number and order of element) to the declaration of the enum BinaryOperator
  "OR", "AND",
  "=", "<>", "<=", ">=", "<", ">",
  "+", "-", "*", "/", "//", "%", "^"
};

const char *QgsPointCloudExpressionNodeUnaryOperator::UNARY_OPERATOR_TEXT[] =
{
  // this must correspond (number and order of element) to the declaration of the enum UnaryOperator
  "NOT", "-"
};

QgsPointCloudExpressionNode::NodeList::~NodeList()
{
  qDeleteAll( mList );
}

QgsPointCloudExpressionNode::NodeList *QgsPointCloudExpressionNode::NodeList::clone() const
{
  NodeList *nl = new NodeList;
  for ( QgsPointCloudExpressionNode *node : mList )
  {
    nl->mList.append( node->clone() );
  }
  nl->mNameList = mNameList;

  return nl;
}

QString QgsPointCloudExpressionNode::NodeList::dump() const
{
  QString msg;
  bool first = true;
  for ( QgsPointCloudExpressionNode *n : mList )
  {
    if ( !first ) msg += QLatin1String( ", " );
    else first = false;
    msg += n->dump();
  }
  return msg;
}

//

double QgsPointCloudExpressionNodeUnaryOperator::evalNode( QgsPointCloudExpression *parent, int pointIndex )
{
  double val = mOperand->eval( parent, pointIndex );
  ENSURE_NO_EVAL_ERROR

  switch ( mOp )
  {
    case uoNot:
      return val == 0. ? 1. : 0.;

    case uoMinus:
      return -val;
  }
  return std::numeric_limits<double>::quiet_NaN();
}

QgsPointCloudExpressionNode::NodeType QgsPointCloudExpressionNodeUnaryOperator::nodeType() const
{
  return ntUnaryOperator;
}

bool QgsPointCloudExpressionNodeUnaryOperator::prepareNode( QgsPointCloudExpression *parent, const QgsPointCloudBlock *block )
{
  return mOperand->prepare( parent, block );
}

QString QgsPointCloudExpressionNodeUnaryOperator::dump() const
{
  if ( dynamic_cast<QgsPointCloudExpressionNodeBinaryOperator *>( mOperand ) )
    return QStringLiteral( "%1 ( %2 )" ).arg( UNARY_OPERATOR_TEXT[mOp], mOperand->dump() );
  else
    return QStringLiteral( "%1 %2" ).arg( UNARY_OPERATOR_TEXT[mOp], mOperand->dump() );
}

QSet<QString> QgsPointCloudExpressionNodeUnaryOperator::referencedAttributes() const
{
  if ( hasCachedStaticValue() )
    return QSet< QString >();

  return mOperand->referencedAttributes();
}

QList<const QgsPointCloudExpressionNode *> QgsPointCloudExpressionNodeUnaryOperator::nodes() const
{
  QList<const QgsPointCloudExpressionNode *> lst;
  lst.append( this );
  lst += mOperand->nodes();
  return lst;
}

QgsPointCloudExpressionNode *QgsPointCloudExpressionNodeUnaryOperator::clone() const
{
  QgsPointCloudExpressionNodeUnaryOperator *copy = new QgsPointCloudExpressionNodeUnaryOperator( mOp, mOperand->clone() );
  cloneTo( copy );
  return copy;
}

bool QgsPointCloudExpressionNodeUnaryOperator::isStatic( QgsPointCloudExpression *parent, const QgsPointCloudBlock *block ) const
{
  return mOperand->isStatic( parent, block );
}

QString QgsPointCloudExpressionNodeUnaryOperator::text() const
{
  return UNARY_OPERATOR_TEXT[mOp];
}

bool QgsPointCloudExpressionNodeUnaryOperator::convert( const QgsExpressionNodeUnaryOperator::UnaryOperator source, QgsPointCloudExpressionNodeUnaryOperator::UnaryOperator &target )
{
  switch ( source )
  {
    case QgsExpressionNodeUnaryOperator::UnaryOperator::uoMinus:
      target = uoMinus;
      break;
    case QgsExpressionNodeUnaryOperator::UnaryOperator::uoNot:
      target = uoNot;
      break;
  }
  return true;
}

//

double QgsPointCloudExpressionNodeBinaryOperator::evalNode( QgsPointCloudExpression *parent, int pointIndex )
{
  double vL = mOpLeft->eval( parent, pointIndex );
  ENSURE_NO_EVAL_ERROR

  if ( mOp == boAnd || mOp == boOr )
  {
    if ( mOp == boAnd && vL == 0. )
      return 0.;  // shortcut -- no need to evaluate right-hand side
    if ( mOp == boOr && vL != 0. )
      return 1.;  // shortcut -- no need to evaluate right-hand side
  }

  double vR = mOpRight->eval( parent, pointIndex );
  ENSURE_NO_EVAL_ERROR

  switch ( mOp )
  {
    case boPlus:
      return vL + vR;
    case boMinus:
      return vL - vR;
    case boMul:
      return vL * vR;
    case boDiv:
      if ( vR == 0. )
        return std::numeric_limits<double>::quiet_NaN();
      return vL / vR;
    case boMod:
      return std::fmod( vL, vR );
    case boIntDiv:
      return std::floor( vL / vR );
    case boPow:
      return std::pow( vL, vR );
    case boAnd:
      // vL is already checked and is true (not 0.)
      return vR == 0. ? 0. : 1.;
    case boOr:
      // vL is already checked and is false (0.)
      return vR == 0. ? 0. : 1.;

    case boEQ:
    case boNE:
    case boLT:
    case boGT:
    case boLE:
    case boGE:
      return compare( vL - vR ) ? 1. : 0.;
  }
  Q_ASSERT( false );
  return std::numeric_limits<double>::quiet_NaN();
}

bool QgsPointCloudExpressionNodeBinaryOperator::compare( double diff )
{
  switch ( mOp )
  {
    case boEQ:
      return diff == 0.0;
    case boNE:
      return diff != 0.0;
    case boLT:
      return diff < 0;
    case boGT:
      return diff > 0;
    case boLE:
      return diff <= 0;
    case boGE:
      return diff >= 0;
    default:
      Q_ASSERT( false );
      return std::numeric_limits<double>::quiet_NaN();
  }
}

QgsPointCloudExpressionNode::NodeType QgsPointCloudExpressionNodeBinaryOperator::nodeType() const
{
  return ntBinaryOperator;
}

bool QgsPointCloudExpressionNodeBinaryOperator::prepareNode( QgsPointCloudExpression *parent, const QgsPointCloudBlock *block )
{
  bool resL = mOpLeft->prepare( parent, block );
  bool resR = mOpRight->prepare( parent, block );
  return resL && resR;
}

int QgsPointCloudExpressionNodeBinaryOperator::precedence() const
{
  // see left/right in qgsexpressionparser.yy
  switch ( mOp )
  {
    case boOr:
      return 1;

    case boAnd:
      return 2;

    case boEQ:
    case boNE:
    case boLE:
    case boGE:
    case boLT:
    case boGT:
      return 3;

    case boPlus:
    case boMinus:
      return 4;

    case boMul:
    case boDiv:
    case boIntDiv:
    case boMod:
      return 5;

    case boPow:
      return 6;
  }
  Q_ASSERT( false && "unexpected binary operator" );
  return -1;
}

bool QgsPointCloudExpressionNodeBinaryOperator::leftAssociative() const
{
  // see left/right in qgsexpressionparser.yy
  switch ( mOp )
  {
    case boOr:
    case boAnd:
    case boEQ:
    case boNE:
    case boLE:
    case boGE:
    case boLT:
    case boGT:
    case boPlus:
    case boMinus:
    case boMul:
    case boDiv:
    case boIntDiv:
    case boMod:
      return true;

    case boPow:
      return false;
  }
  Q_ASSERT( false && "unexpected binary operator" );
  return false;
}

QString QgsPointCloudExpressionNodeBinaryOperator::dump() const
{
  QgsPointCloudExpressionNodeBinaryOperator *lOp = dynamic_cast<QgsPointCloudExpressionNodeBinaryOperator *>( mOpLeft );
  QgsPointCloudExpressionNodeBinaryOperator *rOp = dynamic_cast<QgsPointCloudExpressionNodeBinaryOperator *>( mOpRight );

  QString rdump( mOpRight->dump() );

  QString fmt;
  if ( leftAssociative() )
  {
    fmt += lOp && ( lOp->precedence() < precedence() ) ? QStringLiteral( "(%1)" ) : QStringLiteral( "%1" );
    fmt += QLatin1String( " %2 " );
    fmt += rOp && ( rOp->precedence() <= precedence() ) ? QStringLiteral( "(%3)" ) : QStringLiteral( "%3" );
  }
  else
  {
    fmt += lOp && ( lOp->precedence() <= precedence() ) ? QStringLiteral( "(%1)" ) : QStringLiteral( "%1" );
    fmt += QLatin1String( " %2 " );
    fmt += rOp && ( rOp->precedence() < precedence() ) ? QStringLiteral( "(%3)" ) : QStringLiteral( "%3" );
  }

  return fmt.arg( mOpLeft->dump(), BINARY_OPERATOR_TEXT[mOp], rdump );
}

QSet<QString> QgsPointCloudExpressionNodeBinaryOperator::referencedAttributes() const
{
  if ( hasCachedStaticValue() )
    return QSet< QString >();

  return mOpLeft->referencedAttributes() + mOpRight->referencedAttributes();
}

QList<const QgsPointCloudExpressionNode *> QgsPointCloudExpressionNodeBinaryOperator::nodes() const
{
  QList<const QgsPointCloudExpressionNode *> lst;
  lst << this;
  lst += mOpLeft->nodes() + mOpRight->nodes();
  return lst;
}

QgsPointCloudExpressionNode *QgsPointCloudExpressionNodeBinaryOperator::clone() const
{
  QgsPointCloudExpressionNodeBinaryOperator *copy = new QgsPointCloudExpressionNodeBinaryOperator( mOp, mOpLeft->clone(), mOpRight->clone() );
  cloneTo( copy );
  return copy;
}

bool QgsPointCloudExpressionNodeBinaryOperator::isStatic( QgsPointCloudExpression *parent, const QgsPointCloudBlock *block ) const
{
  const bool leftStatic = mOpLeft->isStatic( parent, block );
  const bool rightStatic = mOpRight->isStatic( parent, block );

  if ( leftStatic && rightStatic )
    return true;

  // special logic for certain ops...
  switch ( mOp )
  {
    case QgsPointCloudExpressionNodeBinaryOperator::boOr:
    {
      // if either node is static AND evaluates to TRUE, then the result will ALWAYS be true regardless
      // of the value of the other node!
      if ( leftStatic )
      {
        mOpLeft->prepare( parent, block );
        if ( mOpLeft->hasCachedStaticValue() )
        {
          double val = mOpLeft->cachedStaticValue();
          if ( !parent->hasEvalError() && val != 0. )
          {
            mCachedStaticValue = true;
            mHasCachedValue = true;
            return true;
          }
        }
      }
      else if ( rightStatic )
      {
        mOpRight->prepare( parent, block );
        if ( mOpRight->hasCachedStaticValue() )
        {
          double val = mOpRight->cachedStaticValue();
          if ( !parent->hasEvalError() && val != 0. )
          {
            mCachedStaticValue = true;
            mHasCachedValue = true;
            return true;
          }
        }
      }

      break;
    }
    case QgsPointCloudExpressionNodeBinaryOperator::boAnd:
    {
      // if either node is static AND evaluates to FALSE, then the result will ALWAYS be false regardless
      // of the value of the other node!

      if ( leftStatic )
      {
        mOpLeft->prepare( parent, block );
        if ( mOpLeft->hasCachedStaticValue() )
        {
          double val = mOpLeft->cachedStaticValue();
          if ( !parent->hasEvalError() && val == 0. )
          {
            mCachedStaticValue = false;
            mHasCachedValue = true;
            return true;
          }
        }
      }
      else if ( rightStatic )
      {
        mOpRight->prepare( parent, block );
        if ( mOpRight->hasCachedStaticValue() )
        {
          double val = mOpRight->cachedStaticValue();
          if ( !parent->hasEvalError() && val == 0. )
          {
            mCachedStaticValue = false;
            mHasCachedValue = true;
            return true;
          }
        }
      }

      break;
    }

    case QgsPointCloudExpressionNodeBinaryOperator::boEQ:
    case QgsPointCloudExpressionNodeBinaryOperator::boNE:
    case QgsPointCloudExpressionNodeBinaryOperator::boLE:
    case QgsPointCloudExpressionNodeBinaryOperator::boGE:
    case QgsPointCloudExpressionNodeBinaryOperator::boLT:
    case QgsPointCloudExpressionNodeBinaryOperator::boGT:
    case QgsPointCloudExpressionNodeBinaryOperator::boPlus:
    case QgsPointCloudExpressionNodeBinaryOperator::boMinus:
    case QgsPointCloudExpressionNodeBinaryOperator::boMul:
    case QgsPointCloudExpressionNodeBinaryOperator::boDiv:
    case QgsPointCloudExpressionNodeBinaryOperator::boIntDiv:
    case QgsPointCloudExpressionNodeBinaryOperator::boMod:
    case QgsPointCloudExpressionNodeBinaryOperator::boPow:
      break;
  }

  return false;
}

QString QgsPointCloudExpressionNodeBinaryOperator::text() const
{
  return BINARY_OPERATOR_TEXT[mOp];
}

bool QgsPointCloudExpressionNodeBinaryOperator::convert( const QgsExpressionNodeBinaryOperator::BinaryOperator source, QgsPointCloudExpressionNodeBinaryOperator::BinaryOperator &target )
{
  switch ( source )
  {
    case QgsExpressionNodeBinaryOperator::BinaryOperator::boOr:
      target = boOr;
      break;
    case QgsExpressionNodeBinaryOperator::BinaryOperator::boAnd:
      target = boAnd;
      break;
    case QgsExpressionNodeBinaryOperator::BinaryOperator::boEQ:
      target = boEQ;
      break;
    case QgsExpressionNodeBinaryOperator::BinaryOperator::boNE:
      target = boNE;
      break;
    case QgsExpressionNodeBinaryOperator::BinaryOperator::boLE:
      target = boLE;
      break;
    case QgsExpressionNodeBinaryOperator::BinaryOperator::boGE:
      target = boGE;
      break;
    case QgsExpressionNodeBinaryOperator::BinaryOperator::boLT:
      target = boLT;
      break;
    case QgsExpressionNodeBinaryOperator::BinaryOperator::boGT:
      target = boGT;
      break;
    case QgsExpressionNodeBinaryOperator::BinaryOperator::boPlus:
      target = boPlus;
      break;
    case QgsExpressionNodeBinaryOperator::BinaryOperator::boMinus:
      target = boMinus;
      break;
    case QgsExpressionNodeBinaryOperator::BinaryOperator::boMul:
      target = boMul;
      break;
    case QgsExpressionNodeBinaryOperator::BinaryOperator::boDiv:
      target = boDiv;
      break;
    case QgsExpressionNodeBinaryOperator::BinaryOperator::boIntDiv:
      target = boIntDiv;
      break;
    case QgsExpressionNodeBinaryOperator::BinaryOperator::boMod:
      target = boMod;
      break;
    case QgsExpressionNodeBinaryOperator::BinaryOperator::boPow:
      target = boPow;
      break;

    case QgsExpressionNodeBinaryOperator::BinaryOperator::boRegexp:
    case QgsExpressionNodeBinaryOperator::BinaryOperator::boLike:
    case QgsExpressionNodeBinaryOperator::BinaryOperator::boILike:
    case QgsExpressionNodeBinaryOperator::BinaryOperator::boNotLike:
    case QgsExpressionNodeBinaryOperator::BinaryOperator::boNotILike:
    case QgsExpressionNodeBinaryOperator::BinaryOperator::boIs:
    case QgsExpressionNodeBinaryOperator::BinaryOperator::boIsNot:
    case QgsExpressionNodeBinaryOperator::BinaryOperator::boConcat:
      return false;
  }
  return true;
}

//

double QgsPointCloudExpressionNodeInOperator::evalNode( QgsPointCloudExpression *parent, int pointIndex )
{
  if ( mList->count() == 0 )
    return mNotIn ? 1. : 0.;
  double v1 = mNode->eval( parent, pointIndex );
  ENSURE_NO_EVAL_ERROR

  const QList< QgsPointCloudExpressionNode * > nodeList = mList->list();
  for ( QgsPointCloudExpressionNode *n : nodeList )
  {
    double v2 = n->eval( parent, pointIndex );
    ENSURE_NO_EVAL_ERROR
    bool equal = false;
    // check whether they are equal
    equal = v1 == v2;
    if ( equal ) // we know the result
      return mNotIn ? 0. : 1.;
  }

  return mNotIn ? 1. : 0.;
}

QgsPointCloudExpressionNodeInOperator::~QgsPointCloudExpressionNodeInOperator()
{
  delete mNode;
  delete mList;
}

QgsPointCloudExpressionNode::NodeType QgsPointCloudExpressionNodeInOperator::nodeType() const
{
  return ntInOperator;
}

bool QgsPointCloudExpressionNodeInOperator::prepareNode( QgsPointCloudExpression *parent, const QgsPointCloudBlock *block )
{
  bool res = mNode->prepare( parent, block );
  const QList< QgsPointCloudExpressionNode * > nodeList = mList->list();
  for ( QgsPointCloudExpressionNode *n : nodeList )
  {
    res = res && n->prepare( parent, block );
  }
  return res;
}

QSet<QString> QgsPointCloudExpressionNodeInOperator::referencedAttributes() const
{
  if ( hasCachedStaticValue() )
    return QSet< QString >();

  QSet<QString> lst( mNode->referencedAttributes() );
  const QList< QgsPointCloudExpressionNode * > nodeList = mList->list();
  for ( const QgsPointCloudExpressionNode *n : nodeList )
    lst.unite( n->referencedAttributes() );
  return lst;
}

QList<const QgsPointCloudExpressionNode *> QgsPointCloudExpressionNodeInOperator::nodes() const
{
  QList<const QgsPointCloudExpressionNode *> lst;
  lst << this;
  const QList< QgsPointCloudExpressionNode * > nodeList = mList->list();
  for ( const QgsPointCloudExpressionNode *n : nodeList )
    lst += n->nodes();
  return lst;
}

QString QgsPointCloudExpressionNodeInOperator::dump() const
{
  return QStringLiteral( "%1 %2 IN (%3)" ).arg( mNode->dump(), mNotIn ? "NOT" : "", mList->dump() );
}

QgsPointCloudExpressionNode *QgsPointCloudExpressionNodeInOperator::clone() const
{
  QgsPointCloudExpressionNodeInOperator *copy = new QgsPointCloudExpressionNodeInOperator( mNode->clone(), mList->clone(), mNotIn );
  cloneTo( copy );
  return copy;
}

bool QgsPointCloudExpressionNodeInOperator::isStatic( QgsPointCloudExpression *parent, const QgsPointCloudBlock *block ) const
{
  if ( !mNode->isStatic( parent, block ) )
    return false;

  const QList< QgsPointCloudExpressionNode * > nodeList = mList->list();
  for ( QgsPointCloudExpressionNode *n : nodeList )
  {
    if ( !n->isStatic( parent, block ) )
      return false;
  }

  return true;
}

//

double QgsPointCloudExpressionNodeLiteral::evalNode( QgsPointCloudExpression *parent, int pointIndex )
{
  Q_UNUSED( parent )
  Q_UNUSED( pointIndex )
  return mValue;
}

QgsPointCloudExpressionNode::NodeType QgsPointCloudExpressionNodeLiteral::nodeType() const
{
  return ntLiteral;
}

bool QgsPointCloudExpressionNodeLiteral::prepareNode( QgsPointCloudExpression *parent, const QgsPointCloudBlock *block )
{
  Q_UNUSED( parent )
  Q_UNUSED( block )
  return true;
}


QString QgsPointCloudExpressionNodeLiteral::valueAsString() const
{
  return QString( "%1" ).arg( mValue );
}

QString QgsPointCloudExpressionNodeLiteral::dump() const
{
  return valueAsString();
}

QSet<QString> QgsPointCloudExpressionNodeLiteral::referencedAttributes() const
{
  return QSet<QString>();
}

QList<const QgsPointCloudExpressionNode *> QgsPointCloudExpressionNodeLiteral::nodes() const
{
  QList<const QgsPointCloudExpressionNode *> lst;
  lst << this;
  return lst;
}

QgsPointCloudExpressionNode *QgsPointCloudExpressionNodeLiteral::clone() const
{
  QgsPointCloudExpressionNodeLiteral *copy = new QgsPointCloudExpressionNodeLiteral( mValue );
  cloneTo( copy );
  return copy;
}

bool QgsPointCloudExpressionNodeLiteral::isStatic( QgsPointCloudExpression *parent, const QgsPointCloudBlock *block ) const
{
  Q_UNUSED( parent )
  Q_UNUSED( block )
  return true;
}

//

double QgsPointCloudExpressionNodeAttributeRef::evalNode( QgsPointCloudExpression *parent, int pointIndex )
{
  Q_UNUSED( parent )

  if ( !mAttribute || mOffset < 0 )
  {
    parent->setEvalErrorString( tr( "Attribute '%1' not found" ).arg( mName ) );
    return std::numeric_limits<double>::quiet_NaN();
  }

  const char *data = mBlock->data();
  const int offset = mBlock->attributes().pointRecordSize() * pointIndex + mOffset;

  double val = mAttribute->convertValueToDouble( data + offset );

  if ( mAttribute->name().compare( QLatin1String( "X" ), Qt::CaseInsensitive ) == 0 )
    return val * mBlock->scale().x() + mBlock->offset().x();
  if ( mAttribute->name().compare( QLatin1String( "Y" ), Qt::CaseInsensitive ) == 0 )
    return val * mBlock->scale().y() + mBlock->offset().y();
  if ( mAttribute->name().compare( QLatin1String( "Z" ), Qt::CaseInsensitive ) == 0 )
    return val * mBlock->scale().z() + mBlock->offset().z();

  return val; // calculate the pointIndex's point respective attribute
}

QgsPointCloudExpressionNode::NodeType QgsPointCloudExpressionNodeAttributeRef::nodeType() const
{
  return ntAttributeRef;
}

bool QgsPointCloudExpressionNodeAttributeRef::prepareNode( QgsPointCloudExpression *parent, const QgsPointCloudBlock *block )
{
  mBlock = block;
  mAttribute = mBlock->attributes().find( mName, mOffset );

  if ( !mAttribute )
  {
    parent->setEvalErrorString( tr( "Attribute '%1' not found" ).arg( mName ) );
    return false;
  }
  Q_ASSERT( mOffset >= 0 );
  return true;
}

QString QgsPointCloudExpressionNodeAttributeRef::dump() const
{
  return mName;
}

QSet<QString> QgsPointCloudExpressionNodeAttributeRef::referencedAttributes() const
{
  return QSet<QString>() << mName;
}

QList<const QgsPointCloudExpressionNode *> QgsPointCloudExpressionNodeAttributeRef::nodes() const
{
  QList<const QgsPointCloudExpressionNode *> result;
  result << this;
  return result;
}

QgsPointCloudExpressionNode *QgsPointCloudExpressionNodeAttributeRef::clone() const
{
  QgsPointCloudExpressionNodeAttributeRef *copy = new QgsPointCloudExpressionNodeAttributeRef( mName );
  cloneTo( copy );
  return copy;
}

bool QgsPointCloudExpressionNodeAttributeRef::isStatic( QgsPointCloudExpression *parent, const QgsPointCloudBlock *block ) const
{
  Q_UNUSED( parent )
  Q_UNUSED( block )
  return false;
}
