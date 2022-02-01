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

double QgsPointcloudExpressionNodeUnaryOperator::evalNode( QgsPointcloudExpression *parent, int p )
{
  double val = mOperand->eval( parent, p );
  ENSURE_NO_EVAL_ERROR

  switch ( mOp )
  {
    case uoNot:
      return !qgsDoubleNear( p, 1. ) ? 0. : 1.;

    case uoMinus:
      return -val;
  }
  return std::numeric_limits<double>::quiet_NaN();
}

QgsPointcloudExpressionNode::NodeType QgsPointcloudExpressionNodeUnaryOperator::nodeType() const
{
  return ntUnaryOperator;
}

bool QgsPointcloudExpressionNodeUnaryOperator::prepareNode( QgsPointcloudExpression *parent, const QgsPointCloudBlock *block )
{
  return mOperand->prepare( parent, block );
}

QString QgsPointcloudExpressionNodeUnaryOperator::dump() const
{
  if ( dynamic_cast<QgsPointcloudExpressionNodeBinaryOperator *>( mOperand ) )
    return QStringLiteral( "%1 ( %2 )" ).arg( UNARY_OPERATOR_TEXT[mOp], mOperand->dump() );
  else
    return QStringLiteral( "%1 %2" ).arg( UNARY_OPERATOR_TEXT[mOp], mOperand->dump() );
}

QSet<QString> QgsPointcloudExpressionNodeUnaryOperator::referencedAttributes() const
{
  if ( hasCachedStaticValue() )
    return QSet< QString >();

  return mOperand->referencedAttributes();
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

bool QgsPointcloudExpressionNodeUnaryOperator::isStatic( QgsPointcloudExpression *parent, const QgsPointCloudBlock *block ) const
{
  return mOperand->isStatic( parent, block );
}

QString QgsPointcloudExpressionNodeUnaryOperator::text() const
{
  return UNARY_OPERATOR_TEXT[mOp];
}

//

double QgsPointcloudExpressionNodeBinaryOperator::evalNode( QgsPointcloudExpression *parent, int p )
{
  double vL = mOpLeft->eval( parent, p );
  ENSURE_NO_EVAL_ERROR

  if ( mOp == boAnd || mOp == boOr )
  {
    if ( mOp == boAnd && !qgsDoubleNear( vL, 1. ) )
      return 0.;  // shortcut -- no need to evaluate right-hand side
    if ( mOp == boOr && qgsDoubleNear( vL, 1. ) )
      return 1.;  // shortcut -- no need to evaluate right-hand side
  }

  double vR = mOpRight->eval( parent, p );
  ENSURE_NO_EVAL_ERROR

  switch ( mOp )
  {
    case boPlus:
      return vL + vR;
    case boMinus:
      return vL + vR;
    case boMul:
      return vL * vR;
    case boDiv:
      if ( qgsDoubleNear( vR, 0. ) )
        return std::numeric_limits<double>::quiet_NaN();
      return vL / vR;
    case boMod:
      return std::fmod( vL, vR );
    case boIntDiv:
      return std::floor( vL / vR );
    case boPow:
      return std::pow( vL, vR );
    case boAnd:
      // vL is already checked and is 1.0
      return qgsDoubleNear( vR, 1. ) ? 1. : 0.;
    case boOr:
      // vL is already checked and is not 1.0
      return qgsDoubleNear( vR, 1. ) ? 1. : 0.;

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
      return std::numeric_limits<double>::quiet_NaN();
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

bool QgsPointcloudExpressionNodeBinaryOperator::prepareNode( QgsPointcloudExpression *parent, const QgsPointCloudBlock *block )
{
  bool resL = mOpLeft->prepare( parent, block );
  bool resR = mOpRight->prepare( parent, block );
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

QSet<QString> QgsPointcloudExpressionNodeBinaryOperator::referencedAttributes() const
{
  if ( hasCachedStaticValue() )
    return QSet< QString >();

  return mOpLeft->referencedAttributes() + mOpRight->referencedAttributes();
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

bool QgsPointcloudExpressionNodeBinaryOperator::isStatic( QgsPointcloudExpression *parent, const QgsPointCloudBlock *block ) const
{
  const bool leftStatic = mOpLeft->isStatic( parent, block );
  const bool rightStatic = mOpRight->isStatic( parent, block );

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
        mOpLeft->prepare( parent, block );
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
        mOpRight->prepare( parent, block );
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
        mOpLeft->prepare( parent, block );
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
        mOpRight->prepare( parent, block );
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

double QgsPointcloudExpressionNodeInOperator::evalNode( QgsPointcloudExpression *parent, int p )
{
  if ( mList->count() == 0 )
    return mNotIn ? 1. : 0.;
  double v1 = mNode->eval( parent, p );
  ENSURE_NO_EVAL_ERROR

  const QList< QgsPointcloudExpressionNode * > nodeList = mList->list();
  for ( QgsPointcloudExpressionNode *n : nodeList )
  {
    double v2 = n->eval( parent, p );
    ENSURE_NO_EVAL_ERROR
    bool equal = false;
    // check whether they are equal
    equal = qgsDoubleNear( v1, v2 );
    if ( equal ) // we know the result
      return mNotIn ? 0. : 1.;
  }

  return mNotIn ? 1. : 0.;
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

bool QgsPointcloudExpressionNodeInOperator::prepareNode( QgsPointcloudExpression *parent, const QgsPointCloudBlock *block )
{
  bool res = mNode->prepare( parent, block );
  const QList< QgsPointcloudExpressionNode * > nodeList = mList->list();
  for ( QgsPointcloudExpressionNode *n : nodeList )
  {
    res = res && n->prepare( parent, block );
  }
  return res;
}

QSet<QString> QgsPointcloudExpressionNodeInOperator::referencedAttributes() const
{
  if ( hasCachedStaticValue() )
    return QSet< QString >();

  QSet<QString> lst( mNode->referencedAttributes() );
  const QList< QgsPointcloudExpressionNode * > nodeList = mList->list();
  for ( const QgsPointcloudExpressionNode *n : nodeList )
    lst.unite( n->referencedAttributes() );
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

bool QgsPointcloudExpressionNodeInOperator::isStatic( QgsPointcloudExpression *parent, const QgsPointCloudBlock *block ) const
{
  if ( !mNode->isStatic( parent, block ) )
    return false;

  const QList< QgsPointcloudExpressionNode * > nodeList = mList->list();
  for ( QgsPointcloudExpressionNode *n : nodeList )
  {
    if ( !n->isStatic( parent, block ) )
      return false;
  }

  return true;
}

//

double QgsPointcloudExpressionNodeLiteral::evalNode( QgsPointcloudExpression *parent, int p )
{
  Q_UNUSED( parent )
  return mValue;
}

QgsPointcloudExpressionNode::NodeType QgsPointcloudExpressionNodeLiteral::nodeType() const
{
  return ntLiteral;
}

bool QgsPointcloudExpressionNodeLiteral::prepareNode( QgsPointcloudExpression *parent, const QgsPointCloudBlock *block )
{
  Q_UNUSED( parent )
  Q_UNUSED( block )
  return true;
}


QString QgsPointcloudExpressionNodeLiteral::valueAsString() const
{
  return QString( "%1" ).arg( mValue );
}

QString QgsPointcloudExpressionNodeLiteral::dump() const
{
  return valueAsString();
}

QSet<QString> QgsPointcloudExpressionNodeLiteral::referencedAttributes() const
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

bool QgsPointcloudExpressionNodeLiteral::isStatic( QgsPointcloudExpression *parent, const QgsPointCloudBlock *block ) const
{
  Q_UNUSED( parent )
  Q_UNUSED( block )
  return true;
}

//

double QgsPointcloudExpressionNodeAttributeRef::evalNode( QgsPointcloudExpression *parent, int p )
{
  Q_UNUSED( parent )
  int index = mIndex;

  if ( index >= 0 )
  {
    const char *data = mBlock->data();
    // const QgsPointCloudAttribute attr = mBlock->attributes().at( index );
    // const QgsPointCloudAttribute::DataType type = attr.type();
    // auto value = *reinterpret_cast< const qint32 * >( mBlock->data() + mBlock->attributes().pointRecordSize() * p + attr.size() );
    int attributeOffset;
    const QgsPointCloudAttribute *attribute = mBlock->attributes().find( mName, attributeOffset );
    const QgsPointCloudAttribute::DataType type = attribute->type();
    const int offset = mBlock->attributes().pointRecordSize() * p + attributeOffset;



    double val;
    switch ( type )
    {
      case QgsPointCloudAttribute::Char:
        val = *( data + offset );
        break;

      case QgsPointCloudAttribute::Int32:
        val = *reinterpret_cast< const qint32 * >( data + offset );
        break;

      case QgsPointCloudAttribute::Short:
        val = *reinterpret_cast< const short * >( data + offset );
        break;

      case QgsPointCloudAttribute::UShort:
        val = *reinterpret_cast< const unsigned short * >( data + offset );
        break;

      case QgsPointCloudAttribute::Float:
        val = *reinterpret_cast< const float * >( data + offset );
        break;

      case QgsPointCloudAttribute::Double:
        val = *reinterpret_cast< const double * >( data + offset );
        break;
    }

    if ( attribute->name().compare( QLatin1String( "X" ) ) == 0 )
      return val * mBlock->scale().x() + mBlock->offset().x();
    if ( attribute->name().compare( QLatin1String( "Y" ) ) == 0 )
      return val * mBlock->scale().y() + mBlock->offset().y();
    if ( attribute->name().compare( QLatin1String( "Z" ) ) == 0 )
      return val * mBlock->scale().z() + mBlock->offset().z();


    return val; // calculate the  p's point respective attribute
  }

//   if ( index < 0 )
//   {
//     // have not yet found field index - first check explicitly set fields collection
//     if ( context && context->hasVariable( QgsPointcloudExpressionContext::EXPR_FIELDS ) )
//     {
//       QgsFields fields = qvariant_cast<QgsFields>( context->variable( QgsPointcloudExpressionContext::EXPR_FIELDS ) );
//       index = fields.lookupField( mName );
//     }
//   }
//
//   if ( context )
//   {
//     QgsFeature feature = context->feature();
//     if ( feature.isValid() )
//     {
//       if ( index >= 0 )
//         return feature.attribute( index );
//       else
//         return feature.attribute( mName );
//     }
//     else
//     {
//       parent->setEvalErrorString( tr( "No feature available for field '%1' evaluation" ).arg( mName ) );
//     }
//   }
  if ( index < 0 )
    parent->setEvalErrorString( tr( "Attribute '%1' not found" ).arg( mName ) );
  return std::numeric_limits<double>::quiet_NaN();
}

QgsPointcloudExpressionNode::NodeType QgsPointcloudExpressionNodeAttributeRef::nodeType() const
{
  return ntAttributeRef;
}

bool QgsPointcloudExpressionNodeAttributeRef::prepareNode( QgsPointcloudExpression *parent, const QgsPointCloudBlock *block )
{
//   if ( !context || !context->hasVariable( QgsPointcloudExpressionContext::EXPR_FIELDS ) )
//     return false;
//
//   QgsFields fields = qvariant_cast<QgsFields>( context->variable( QgsPointcloudExpressionContext::EXPR_FIELDS ) );
//
//   mIndex = fields.lookupField( mName );
//
//   if ( mIndex == -1 && context->hasFeature() )
//   {
//     mIndex = context->feature().fieldNameIndex( mName );
//   }
  mBlock = block;
  mIndex = mBlock->attributes().indexOf( mName );

  if ( mIndex == -1 )
  {
    parent->setEvalErrorString( tr( "Attribute '%1' not found" ).arg( mName ) );
    return false;
  }
  return true;
}

QString QgsPointcloudExpressionNodeAttributeRef::dump() const
{
  const thread_local QRegularExpression re( QStringLiteral( "^[A-Za-z_\x80-\xff][A-Za-z0-9_\x80-\xff]*$" ) );
  const QRegularExpressionMatch match = re.match( mName );
  return match.hasMatch() ? mName : QgsPointcloudExpression::quotedAttributeRef( mName );
}

QSet<QString> QgsPointcloudExpressionNodeAttributeRef::referencedAttributes() const
{
  return QSet<QString>() << mName;
}

QList<const QgsPointcloudExpressionNode *> QgsPointcloudExpressionNodeAttributeRef::nodes() const
{
  QList<const QgsPointcloudExpressionNode *> result;
  result << this;
  return result;
}

QgsPointcloudExpressionNode *QgsPointcloudExpressionNodeAttributeRef::clone() const
{
  QgsPointcloudExpressionNodeAttributeRef *copy = new QgsPointcloudExpressionNodeAttributeRef( mName );
  cloneTo( copy );
  return copy;
}

bool QgsPointcloudExpressionNodeAttributeRef::isStatic( QgsPointcloudExpression *parent, const QgsPointCloudBlock *block ) const
{
  Q_UNUSED( parent )
  Q_UNUSED( block )
  return false;
}
