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
#include "qgspointcloudexpressionutils.h"
#include "qgspointcloudexpression.h"

#include "qgsgeometry.h"
#include "qgsfeaturerequest.h"
#include "qgsstringutils.h"

const char *QgsPointcloudExpressionNodeBinaryOperator::BINARY_OPERATOR_TEXT[] =
{
  // this must correspond (number and order of element) to the declaration of the enum BinaryOperator
  "OR", "AND",
  "=", "<>", "<=", ">=", "<", ">",
  "+", "-", "*", "/", "//", "%", "^"
};

const char *QgsPointcloudExpressionNodeUnaryOperator::UNARY_OPERATOR_TEXT[] =
{
  // this must correspond (number and order of element) to the declaration of the enum UnaryOperator
  "NOT", "-"
};

QgsPointcloudExpressionNode::NodeList::~NodeList()
{
  qDeleteAll( mList );
}

void QgsPointcloudExpressionNode::NodeList::append( QgsPointcloudExpressionNode::NamedNode *node )
{
  mList.append( node->node );
  mNameList.append( cleanNamedNodeName( node->name ) );
  mHasNamedNodes = true;
  delete node;
}

QgsPointcloudExpressionNode::NodeList *QgsPointcloudExpressionNode::NodeList::clone() const
{
  NodeList *nl = new NodeList;
  for ( QgsPointcloudExpressionNode *node : mList )
  {
    nl->mList.append( node->clone() );
  }
  nl->mNameList = mNameList;

  return nl;
}

QString QgsPointcloudExpressionNode::NodeList::dump() const
{
  QString msg;
  bool first = true;
  for ( QgsPointcloudExpressionNode *n : mList )
  {
    if ( !first ) msg += QLatin1String( ", " );
    else first = false;
    msg += n->dump();
  }
  return msg;
}

QString QgsPointcloudExpressionNode::NodeList::cleanNamedNodeName( const QString &name )
{
  QString cleaned = name.toLower();

  // upgrade older argument names to standard versions
  if ( cleaned == QLatin1String( "geom" ) )
    cleaned = QStringLiteral( "geometry" );
  else if ( cleaned == QLatin1String( "val" ) )
    cleaned = QStringLiteral( "value" );
  else if ( cleaned == QLatin1String( "geometry a" ) )
    cleaned = QStringLiteral( "geometry1" );
  else if ( cleaned == QLatin1String( "geometry b" ) )
    cleaned = QStringLiteral( "geometry2" );

  return cleaned;
}


//

QVariant QgsPointcloudExpressionNodeUnaryOperator::evalNode( QgsPointcloudExpression *parent, const QgsPointcloudExpressionContext *context )
{
  QVariant val = mOperand->eval( parent, context );
  ENSURE_NO_EVAL_ERROR

  switch ( mOp )
  {
    case uoNot:
    {
      QgsPointcloudExpressionUtils::TVL tvl = QgsPointcloudExpressionUtils::getTVLValue( val, parent );
      ENSURE_NO_EVAL_ERROR
      return QgsPointcloudExpressionUtils::tvl2variant( QgsPointcloudExpressionUtils::NOT[tvl] );
    }

    case uoMinus:
      if ( QgsPointcloudExpressionUtils::isIntSafe( val ) )
        return QVariant( - QgsPointcloudExpressionUtils::getIntValue( val, parent ) );
      else if ( QgsPointcloudExpressionUtils::isDoubleSafe( val ) )
        return QVariant( - QgsPointcloudExpressionUtils::getDoubleValue( val, parent ) );
      else
        SET_EVAL_ERROR( tr( "Unary minus only for numeric values." ) )
      }
  return QVariant();
}

QgsPointcloudExpressionNode::NodeType QgsPointcloudExpressionNodeUnaryOperator::nodeType() const
{
  return ntUnaryOperator;
}

bool QgsPointcloudExpressionNodeUnaryOperator::prepareNode( QgsPointcloudExpression *parent, const QgsPointcloudExpressionContext *context )
{
  return mOperand->prepare( parent, context );
}

QString QgsPointcloudExpressionNodeUnaryOperator::dump() const
{
  if ( dynamic_cast<QgsPointcloudExpressionNodeBinaryOperator *>( mOperand ) )
    return QStringLiteral( "%1 ( %2 )" ).arg( UNARY_OPERATOR_TEXT[mOp], mOperand->dump() );
  else
    return QStringLiteral( "%1 %2" ).arg( UNARY_OPERATOR_TEXT[mOp], mOperand->dump() );
}

QSet<QString> QgsPointcloudExpressionNodeUnaryOperator::referencedColumns() const
{
  if ( hasCachedStaticValue() )
    return QSet< QString >();

  return mOperand->referencedColumns();
}

QList<const QgsPointcloudExpressionNode *> QgsPointcloudExpressionNodeUnaryOperator::nodes() const
{
  QList<const QgsPointcloudExpressionNode *> lst;
  lst.append( this );
  lst += mOperand->nodes();
  return lst;
}

QgsPointcloudExpressionNode *QgsPointcloudExpressionNodeUnaryOperator::clone() const
{
  QgsPointcloudExpressionNodeUnaryOperator *copy = new QgsPointcloudExpressionNodeUnaryOperator( mOp, mOperand->clone() );
  cloneTo( copy );
  return copy;
}

bool QgsPointcloudExpressionNodeUnaryOperator::isStatic( QgsPointcloudExpression *parent, const QgsPointcloudExpressionContext *context ) const
{
  return mOperand->isStatic( parent, context );
}

QString QgsPointcloudExpressionNodeUnaryOperator::text() const
{
  return UNARY_OPERATOR_TEXT[mOp];
}

//

QVariant QgsPointcloudExpressionNodeBinaryOperator::evalNode( QgsPointcloudExpression *parent, const QgsPointcloudExpressionContext *context )
{
  QVariant vL = mOpLeft->eval( parent, context );
  ENSURE_NO_EVAL_ERROR

  if ( mOp == boAnd || mOp == boOr )
  {
    QgsPointcloudExpressionUtils::TVL tvlL = QgsPointcloudExpressionUtils::getTVLValue( vL, parent );
    ENSURE_NO_EVAL_ERROR
    if ( mOp == boAnd && tvlL == QgsPointcloudExpressionUtils::False )
      return TVL_False;  // shortcut -- no need to evaluate right-hand side
    if ( mOp == boOr && tvlL == QgsPointcloudExpressionUtils::True )
      return TVL_True;  // shortcut -- no need to evaluate right-hand side
  }

  QVariant vR = mOpRight->eval( parent, context );
  ENSURE_NO_EVAL_ERROR

  switch ( mOp )
  {
    case boPlus:
      if ( vL.type() == QVariant::String && vR.type() == QVariant::String )
      {
        QString sL = QgsPointcloudExpressionUtils::isNull( vL ) ? QString() : QgsPointcloudExpressionUtils::getStringValue( vL, parent );
        ENSURE_NO_EVAL_ERROR
        QString sR = QgsPointcloudExpressionUtils::isNull( vR ) ? QString() : QgsPointcloudExpressionUtils::getStringValue( vR, parent );
        ENSURE_NO_EVAL_ERROR
        return QVariant( sL + sR );
      }
      //intentional fall-through
      FALLTHROUGH
    case boMinus:
    case boMul:
    case boDiv:
    case boMod:
    {
      if ( QgsPointcloudExpressionUtils::isNull( vL ) || QgsPointcloudExpressionUtils::isNull( vR ) )
        return QVariant();
      else if ( mOp != boDiv && QgsPointcloudExpressionUtils::isIntSafe( vL ) && QgsPointcloudExpressionUtils::isIntSafe( vR ) )
      {
        // both are integers - let's use integer arithmetic
        qlonglong iL = QgsPointcloudExpressionUtils::getIntValue( vL, parent );
        ENSURE_NO_EVAL_ERROR
        qlonglong iR = QgsPointcloudExpressionUtils::getIntValue( vR, parent );
        ENSURE_NO_EVAL_ERROR

        if ( mOp == boMod && iR == 0 )
          return QVariant();

        return QVariant( computeInt( iL, iR ) );
      }
      else
      {
        // general floating point arithmetic
        double fL = QgsPointcloudExpressionUtils::getDoubleValue( vL, parent );
        ENSURE_NO_EVAL_ERROR
        double fR = QgsPointcloudExpressionUtils::getDoubleValue( vR, parent );
        ENSURE_NO_EVAL_ERROR
        if ( ( mOp == boDiv || mOp == boMod ) && fR == 0. )
          return QVariant(); // silently handle division by zero and return NULL
        return QVariant( computeDouble( fL, fR ) );
      }
    }
    case boIntDiv:
    {
      //integer division
      double fL = QgsPointcloudExpressionUtils::getDoubleValue( vL, parent );
      ENSURE_NO_EVAL_ERROR
      double fR = QgsPointcloudExpressionUtils::getDoubleValue( vR, parent );
      ENSURE_NO_EVAL_ERROR
      if ( fR == 0. )
        return QVariant(); // silently handle division by zero and return NULL
      return QVariant( qlonglong( std::floor( fL / fR ) ) );
    }
    case boPow:
      if ( QgsPointcloudExpressionUtils::isNull( vL ) || QgsPointcloudExpressionUtils::isNull( vR ) )
        return QVariant();
      else
      {
        double fL = QgsPointcloudExpressionUtils::getDoubleValue( vL, parent );
        ENSURE_NO_EVAL_ERROR
        double fR = QgsPointcloudExpressionUtils::getDoubleValue( vR, parent );
        ENSURE_NO_EVAL_ERROR
        return QVariant( std::pow( fL, fR ) );
      }

    case boAnd:
    {
      QgsPointcloudExpressionUtils::TVL tvlL = QgsPointcloudExpressionUtils::getTVLValue( vL, parent ), tvlR = QgsPointcloudExpressionUtils::getTVLValue( vR, parent );
      ENSURE_NO_EVAL_ERROR
      return  QgsPointcloudExpressionUtils::tvl2variant( QgsPointcloudExpressionUtils::AND[tvlL][tvlR] );
    }

    case boOr:
    {
      QgsPointcloudExpressionUtils::TVL tvlL = QgsPointcloudExpressionUtils::getTVLValue( vL, parent ), tvlR = QgsPointcloudExpressionUtils::getTVLValue( vR, parent );
      ENSURE_NO_EVAL_ERROR
      return  QgsPointcloudExpressionUtils::tvl2variant( QgsPointcloudExpressionUtils::OR[tvlL][tvlR] );
    }

    case boEQ:
    case boNE:
    case boLT:
    case boGT:
    case boLE:
    case boGE:
      if ( QgsPointcloudExpressionUtils::isNull( vL ) || QgsPointcloudExpressionUtils::isNull( vR ) )
      {
        return TVL_Unknown;
      }
      else if ( ( vL.type() != QVariant::String || vR.type() != QVariant::String ) &&
                QgsPointcloudExpressionUtils::isDoubleSafe( vL ) && QgsPointcloudExpressionUtils::isDoubleSafe( vR ) )
      {
        // do numeric comparison if both operators can be converted to numbers,
        // and they aren't both string
        double fL = QgsPointcloudExpressionUtils::getDoubleValue( vL, parent );
        ENSURE_NO_EVAL_ERROR
        double fR = QgsPointcloudExpressionUtils::getDoubleValue( vR, parent );
        ENSURE_NO_EVAL_ERROR
        return compare( fL - fR ) ? TVL_True : TVL_False;
      }
      else
      {
        // do string comparison otherwise
        QString sL = QgsPointcloudExpressionUtils::getStringValue( vL, parent );
        ENSURE_NO_EVAL_ERROR
        QString sR = QgsPointcloudExpressionUtils::getStringValue( vR, parent );
        ENSURE_NO_EVAL_ERROR
        int diff = QString::compare( sL, sR );
        return compare( diff ) ? TVL_True : TVL_False;
      }
  }
  Q_ASSERT( false );
  return QVariant();
}

bool QgsPointcloudExpressionNodeBinaryOperator::compare( double diff )
{
  switch ( mOp )
  {
    case boEQ:
      return qgsDoubleNear( diff, 0.0 );
    case boNE:
      return !qgsDoubleNear( diff, 0.0 );
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
      return false;
  }
}

qlonglong QgsPointcloudExpressionNodeBinaryOperator::computeInt( qlonglong x, qlonglong y )
{
  switch ( mOp )
  {
    case boPlus:
      return x + y;
    case boMinus:
      return x - y;
    case boMul:
      return x * y;
    case boDiv:
      return x / y;
    case boMod:
      return x % y;
    default:
      Q_ASSERT( false );
      return 0;
  }
}

double QgsPointcloudExpressionNodeBinaryOperator::computeDouble( double x, double y )
{
  switch ( mOp )
  {
    case boPlus:
      return x + y;
    case boMinus:
      return x - y;
    case boMul:
      return x * y;
    case boDiv:
      return x / y;
    case boMod:
      return std::fmod( x, y );
    default:
      Q_ASSERT( false );
      return 0;
  }
}

QgsPointcloudExpressionNode::NodeType QgsPointcloudExpressionNodeBinaryOperator::nodeType() const
{
  return ntBinaryOperator;
}

bool QgsPointcloudExpressionNodeBinaryOperator::prepareNode( QgsPointcloudExpression *parent, const QgsPointcloudExpressionContext *context )
{
  bool resL = mOpLeft->prepare( parent, context );
  bool resR = mOpRight->prepare( parent, context );
  return resL && resR;
}

int QgsPointcloudExpressionNodeBinaryOperator::precedence() const
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

bool QgsPointcloudExpressionNodeBinaryOperator::leftAssociative() const
{
  // see left/right in qgspointcloudexpressionparser.yy
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

QString QgsPointcloudExpressionNodeBinaryOperator::dump() const
{
  QgsPointcloudExpressionNodeBinaryOperator *lOp = dynamic_cast<QgsPointcloudExpressionNodeBinaryOperator *>( mOpLeft );
  QgsPointcloudExpressionNodeBinaryOperator *rOp = dynamic_cast<QgsPointcloudExpressionNodeBinaryOperator *>( mOpRight );
  QgsPointcloudExpressionNodeUnaryOperator *ruOp = dynamic_cast<QgsPointcloudExpressionNodeUnaryOperator *>( mOpRight );

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

QSet<QString> QgsPointcloudExpressionNodeBinaryOperator::referencedColumns() const
{
  if ( hasCachedStaticValue() )
    return QSet< QString >();

  return mOpLeft->referencedColumns() + mOpRight->referencedColumns();
}

QList<const QgsPointcloudExpressionNode *> QgsPointcloudExpressionNodeBinaryOperator::nodes() const
{
  QList<const QgsPointcloudExpressionNode *> lst;
  lst << this;
  lst += mOpLeft->nodes() + mOpRight->nodes();
  return lst;
}

QgsPointcloudExpressionNode *QgsPointcloudExpressionNodeBinaryOperator::clone() const
{
  QgsPointcloudExpressionNodeBinaryOperator *copy = new QgsPointcloudExpressionNodeBinaryOperator( mOp, mOpLeft->clone(), mOpRight->clone() );
  cloneTo( copy );
  return copy;
}

bool QgsPointcloudExpressionNodeBinaryOperator::isStatic( QgsPointcloudExpression *parent, const QgsPointcloudExpressionContext *context ) const
{
  const bool leftStatic = mOpLeft->isStatic( parent, context );
  const bool rightStatic = mOpRight->isStatic( parent, context );

  if ( leftStatic && rightStatic )
    return true;

  // special logic for certain ops...
  switch ( mOp )
  {
    case QgsPointcloudExpressionNodeBinaryOperator::boOr:
    {
      // if either node is static AND evaluates to TRUE, then the result will ALWAYS be true regardless
      // of the value of the other node!
      if ( leftStatic )
      {
        mOpLeft->prepare( parent, context );
        if ( mOpLeft->hasCachedStaticValue() )
        {
          QgsPointcloudExpressionUtils::TVL tvl = QgsPointcloudExpressionUtils::getTVLValue( mOpLeft->cachedStaticValue(), parent );
          if ( !parent->hasEvalError() && tvl == QgsPointcloudExpressionUtils::True )
          {
            mCachedStaticValue = true;
            mHasCachedValue = true;
            return true;
          }
        }
      }
      else if ( rightStatic )
      {
        mOpRight->prepare( parent, context );
        if ( mOpRight->hasCachedStaticValue() )
        {
          QgsPointcloudExpressionUtils::TVL tvl = QgsPointcloudExpressionUtils::getTVLValue( mOpRight->cachedStaticValue(), parent );
          if ( !parent->hasEvalError() && tvl == QgsPointcloudExpressionUtils::True )
          {
            mCachedStaticValue = true;
            mHasCachedValue = true;
            return true;
          }
        }
      }

      break;
    }
    case QgsPointcloudExpressionNodeBinaryOperator::boAnd:
    {
      // if either node is static AND evaluates to FALSE, then the result will ALWAYS be false regardless
      // of the value of the other node!

      if ( leftStatic )
      {
        mOpLeft->prepare( parent, context );
        if ( mOpLeft->hasCachedStaticValue() )
        {
          QgsPointcloudExpressionUtils::TVL tvl = QgsPointcloudExpressionUtils::getTVLValue( mOpLeft->cachedStaticValue(), parent );
          if ( !parent->hasEvalError() && tvl == QgsPointcloudExpressionUtils::False )
          {
            mCachedStaticValue = false;
            mHasCachedValue = true;
            return true;
          }
        }
      }
      else if ( rightStatic )
      {
        mOpRight->prepare( parent, context );
        if ( mOpRight->hasCachedStaticValue() )
        {
          QgsPointcloudExpressionUtils::TVL tvl = QgsPointcloudExpressionUtils::getTVLValue( mOpRight->cachedStaticValue(), parent );
          if ( !parent->hasEvalError() && tvl == QgsPointcloudExpressionUtils::False )
          {
            mCachedStaticValue = false;
            mHasCachedValue = true;
            return true;
          }
        }
      }

      break;
    }

    case QgsPointcloudExpressionNodeBinaryOperator::boEQ:
    case QgsPointcloudExpressionNodeBinaryOperator::boNE:
    case QgsPointcloudExpressionNodeBinaryOperator::boLE:
    case QgsPointcloudExpressionNodeBinaryOperator::boGE:
    case QgsPointcloudExpressionNodeBinaryOperator::boLT:
    case QgsPointcloudExpressionNodeBinaryOperator::boGT:
    case QgsPointcloudExpressionNodeBinaryOperator::boPlus:
    case QgsPointcloudExpressionNodeBinaryOperator::boMinus:
    case QgsPointcloudExpressionNodeBinaryOperator::boMul:
    case QgsPointcloudExpressionNodeBinaryOperator::boDiv:
    case QgsPointcloudExpressionNodeBinaryOperator::boIntDiv:
    case QgsPointcloudExpressionNodeBinaryOperator::boMod:
    case QgsPointcloudExpressionNodeBinaryOperator::boPow:
      break;
  }

  return false;
}

QString QgsPointcloudExpressionNodeBinaryOperator::text() const
{
  return BINARY_OPERATOR_TEXT[mOp];
}

//

QVariant QgsPointcloudExpressionNodeInOperator::evalNode( QgsPointcloudExpression *parent, const QgsPointcloudExpressionContext *context )
{
  if ( mList->count() == 0 )
    return mNotIn ? TVL_True : TVL_False;
  QVariant v1 = mNode->eval( parent, context );
  ENSURE_NO_EVAL_ERROR
  if ( QgsPointcloudExpressionUtils::isNull( v1 ) )
    return TVL_Unknown;

  bool listHasNull = false;

  const QList< QgsPointcloudExpressionNode * > nodeList = mList->list();
  for ( QgsPointcloudExpressionNode *n : nodeList )
  {
    QVariant v2 = n->eval( parent, context );
    ENSURE_NO_EVAL_ERROR
    if ( QgsPointcloudExpressionUtils::isNull( v2 ) )
      listHasNull = true;
    else
    {
      bool equal = false;
      // check whether they are equal
      if ( ( v1.type() != QVariant::String || v2.type() != QVariant::String ) &&
           QgsPointcloudExpressionUtils::isDoubleSafe( v1 ) && QgsPointcloudExpressionUtils::isDoubleSafe( v2 ) )
      {
        // do numeric comparison if both operators can be converted to numbers,
        // and they aren't both string
        double f1 = QgsPointcloudExpressionUtils::getDoubleValue( v1, parent );
        ENSURE_NO_EVAL_ERROR
        double f2 = QgsPointcloudExpressionUtils::getDoubleValue( v2, parent );
        ENSURE_NO_EVAL_ERROR
        equal = qgsDoubleNear( f1, f2 );
      }
      else
      {
        QString s1 = QgsPointcloudExpressionUtils::getStringValue( v1, parent );
        ENSURE_NO_EVAL_ERROR
        QString s2 = QgsPointcloudExpressionUtils::getStringValue( v2, parent );
        ENSURE_NO_EVAL_ERROR
        equal = QString::compare( s1, s2 ) == 0;
      }

      if ( equal ) // we know the result
        return mNotIn ? TVL_False : TVL_True;
    }
  }

  // item not found
  if ( listHasNull )
    return TVL_Unknown;
  else
    return mNotIn ? TVL_True : TVL_False;
}

QgsPointcloudExpressionNodeInOperator::~QgsPointcloudExpressionNodeInOperator()
{
  delete mNode;
  delete mList;
}

QgsPointcloudExpressionNode::NodeType QgsPointcloudExpressionNodeInOperator::nodeType() const
{
  return ntInOperator;
}

bool QgsPointcloudExpressionNodeInOperator::prepareNode( QgsPointcloudExpression *parent, const QgsPointcloudExpressionContext *context )
{
  bool res = mNode->prepare( parent, context );
  const QList< QgsPointcloudExpressionNode * > nodeList = mList->list();
  for ( QgsPointcloudExpressionNode *n : nodeList )
  {
    res = res && n->prepare( parent, context );
  }
  return res;
}

QSet<QString> QgsPointcloudExpressionNodeInOperator::referencedColumns() const
{
  if ( hasCachedStaticValue() )
    return QSet< QString >();

  QSet<QString> lst( mNode->referencedColumns() );
  const QList< QgsPointcloudExpressionNode * > nodeList = mList->list();
  for ( const QgsPointcloudExpressionNode *n : nodeList )
    lst.unite( n->referencedColumns() );
  return lst;
}

QList<const QgsPointcloudExpressionNode *> QgsPointcloudExpressionNodeInOperator::nodes() const
{
  QList<const QgsPointcloudExpressionNode *> lst;
  lst << this;
  const QList< QgsPointcloudExpressionNode * > nodeList = mList->list();
  for ( const QgsPointcloudExpressionNode *n : nodeList )
    lst += n->nodes();
  return lst;
}

QString QgsPointcloudExpressionNodeInOperator::dump() const
{
  return QStringLiteral( "%1 %2 IN (%3)" ).arg( mNode->dump(), mNotIn ? "NOT" : "", mList->dump() );
}

QgsPointcloudExpressionNode *QgsPointcloudExpressionNodeInOperator::clone() const
{
  QgsPointcloudExpressionNodeInOperator *copy = new QgsPointcloudExpressionNodeInOperator( mNode->clone(), mList->clone(), mNotIn );
  cloneTo( copy );
  return copy;
}

bool QgsPointcloudExpressionNodeInOperator::isStatic( QgsPointcloudExpression *parent, const QgsPointcloudExpressionContext *context ) const
{
  if ( !mNode->isStatic( parent, context ) )
    return false;

  const QList< QgsPointcloudExpressionNode * > nodeList = mList->list();
  for ( QgsPointcloudExpressionNode *n : nodeList )
  {
    if ( !n->isStatic( parent, context ) )
      return false;
  }

  return true;
}

//

QVariant QgsPointcloudExpressionNodeLiteral::evalNode( QgsPointcloudExpression *parent, const QgsPointcloudExpressionContext *context )
{
  Q_UNUSED( context )
  Q_UNUSED( parent )
  return mValue;
}

QgsPointcloudExpressionNode::NodeType QgsPointcloudExpressionNodeLiteral::nodeType() const
{
  return ntLiteral;
}

bool QgsPointcloudExpressionNodeLiteral::prepareNode( QgsPointcloudExpression *parent, const QgsPointcloudExpressionContext *context )
{
  Q_UNUSED( parent )
  Q_UNUSED( context )
  return true;
}


QString QgsPointcloudExpressionNodeLiteral::valueAsString() const
{
  if ( mValue.isNull() )
    return QStringLiteral( "NULL" );

  switch ( mValue.type() )
  {
    case QVariant::Int:
      return QString::number( mValue.toInt() );
    case QVariant::Double:
      return QString::number( mValue.toDouble() );
    case QVariant::LongLong:
      return QString::number( mValue.toLongLong() );
    case QVariant::String:
      return QgsPointcloudExpression::quotedString( mValue.toString() );
    case QVariant::Bool:
      return mValue.toBool() ? QStringLiteral( "TRUE" ) : QStringLiteral( "FALSE" );
    default:
      return tr( "[unsupported type: %1; value: %2]" ).arg( mValue.typeName(), mValue.toString() );
  }
}

QString QgsPointcloudExpressionNodeLiteral::dump() const
{
  return valueAsString();
}

QSet<QString> QgsPointcloudExpressionNodeLiteral::referencedColumns() const
{
  return QSet<QString>();
}

QList<const QgsPointcloudExpressionNode *> QgsPointcloudExpressionNodeLiteral::nodes() const
{
  QList<const QgsPointcloudExpressionNode *> lst;
  lst << this;
  return lst;
}

QgsPointcloudExpressionNode *QgsPointcloudExpressionNodeLiteral::clone() const
{
  QgsPointcloudExpressionNodeLiteral *copy = new QgsPointcloudExpressionNodeLiteral( mValue );
  cloneTo( copy );
  return copy;
}

bool QgsPointcloudExpressionNodeLiteral::isStatic( QgsPointcloudExpression *parent, const QgsPointcloudExpressionContext *context ) const
{
  Q_UNUSED( context )
  Q_UNUSED( parent )
  return true;
}

//

QVariant QgsPointcloudExpressionNodeColumnRef::evalNode( QgsPointcloudExpression *parent, const QgsPointcloudExpressionContext *context )
{
  Q_UNUSED( parent )
  int index = mIndex;

  if ( index < 0 )
  {
    // have not yet found field index - first check explicitly set fields collection
    if ( context && context->hasVariable( QgsPointcloudExpressionContext::EXPR_FIELDS ) )
    {
      QgsFields fields = qvariant_cast<QgsFields>( context->variable( QgsPointcloudExpressionContext::EXPR_FIELDS ) );
      index = fields.lookupField( mName );
    }
  }

  if ( context )
  {
    QgsFeature feature = context->feature();
    if ( feature.isValid() )
    {
      if ( index >= 0 )
        return feature.attribute( index );
      else
        return feature.attribute( mName );
    }
    else
    {
      parent->setEvalErrorString( tr( "No feature available for field '%1' evaluation" ).arg( mName ) );
    }
  }
  if ( index < 0 )
    parent->setEvalErrorString( tr( "Field '%1' not found" ).arg( mName ) );
  return QVariant();
}

QgsPointcloudExpressionNode::NodeType QgsPointcloudExpressionNodeColumnRef::nodeType() const
{
  return ntColumnRef;
}

bool QgsPointcloudExpressionNodeColumnRef::prepareNode( QgsPointcloudExpression *parent, const QgsPointcloudExpressionContext *context )
{
  if ( !context || !context->hasVariable( QgsPointcloudExpressionContext::EXPR_FIELDS ) )
    return false;

  QgsFields fields = qvariant_cast<QgsFields>( context->variable( QgsPointcloudExpressionContext::EXPR_FIELDS ) );

  mIndex = fields.lookupField( mName );

  if ( mIndex == -1 && context->hasFeature() )
  {
    mIndex = context->feature().fieldNameIndex( mName );
  }

  if ( mIndex == -1 )
  {
    parent->setEvalErrorString( tr( "Field '%1' not found" ).arg( mName ) );
    return false;
  }
  return true;
}

QString QgsPointcloudExpressionNodeColumnRef::dump() const
{
  const thread_local QRegularExpression re( QStringLiteral( "^[A-Za-z_\x80-\xff][A-Za-z0-9_\x80-\xff]*$" ) );
  const QRegularExpressionMatch match = re.match( mName );
  return match.hasMatch() ? mName : QgsPointcloudExpression::quotedColumnRef( mName );
}

QSet<QString> QgsPointcloudExpressionNodeColumnRef::referencedColumns() const
{
  return QSet<QString>() << mName;
}

QList<const QgsPointcloudExpressionNode *> QgsPointcloudExpressionNodeColumnRef::nodes() const
{
  QList<const QgsPointcloudExpressionNode *> result;
  result << this;
  return result;
}

QgsPointcloudExpressionNode *QgsPointcloudExpressionNodeColumnRef::clone() const
{
  QgsPointcloudExpressionNodeColumnRef *copy = new QgsPointcloudExpressionNodeColumnRef( mName );
  cloneTo( copy );
  return copy;
}

bool QgsPointcloudExpressionNodeColumnRef::isStatic( QgsPointcloudExpression *parent, const QgsPointcloudExpressionContext *context ) const
{
  Q_UNUSED( context )
  Q_UNUSED( parent )
  return false;
}
