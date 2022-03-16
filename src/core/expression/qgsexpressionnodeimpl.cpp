/***************************************************************************
                               qgsexpressionnodeimpl.cpp
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

#include "qgsexpressionnodeimpl.h"
#include "qgsexpressionutils.h"
#include "qgsexpression.h"

#include "qgsgeometry.h"
#include "qgsfeaturerequest.h"
#include "qgsstringutils.h"

#include <QRegularExpression>

const char *QgsExpressionNodeBinaryOperator::BINARY_OPERATOR_TEXT[] =
{
  // this must correspond (number and order of element) to the declaration of the enum BinaryOperator
  "OR", "AND",
  "=", "<>", "<=", ">=", "<", ">", "~", "LIKE", "NOT LIKE", "ILIKE", "NOT ILIKE", "IS", "IS NOT",
  "+", "-", "*", "/", "//", "%", "^",
  "||"
};

const char *QgsExpressionNodeUnaryOperator::UNARY_OPERATOR_TEXT[] =
{
  // this must correspond (number and order of element) to the declaration of the enum UnaryOperator
  "NOT", "-"
};

bool QgsExpressionNodeInOperator::needsGeometry() const
{
  bool needs = false;
  const QList< QgsExpressionNode * > nodeList = mList->list();
  for ( QgsExpressionNode *n : nodeList )
    needs |= n->needsGeometry();
  return needs;
}

QgsExpressionNode::NodeList::~NodeList()
{
  qDeleteAll( mList );
}

void QgsExpressionNode::NodeList::append( QgsExpressionNode::NamedNode *node )
{
  mList.append( node->node );
  mNameList.append( cleanNamedNodeName( node->name ) );
  mHasNamedNodes = true;
  delete node;
}

QgsExpressionNode::NodeList *QgsExpressionNode::NodeList::clone() const
{
  NodeList *nl = new NodeList;
  for ( QgsExpressionNode *node : mList )
  {
    nl->mList.append( node->clone() );
  }
  nl->mNameList = mNameList;

  return nl;
}

QString QgsExpressionNode::NodeList::dump() const
{
  QString msg;
  bool first = true;
  for ( QgsExpressionNode *n : mList )
  {
    if ( !first ) msg += QLatin1String( ", " );
    else first = false;
    msg += n->dump();
  }
  return msg;
}

QString QgsExpressionNode::NodeList::cleanNamedNodeName( const QString &name )
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

QVariant QgsExpressionNodeUnaryOperator::evalNode( QgsExpression *parent, const QgsExpressionContext *context )
{
  QVariant val = mOperand->eval( parent, context );
  ENSURE_NO_EVAL_ERROR

  switch ( mOp )
  {
    case uoNot:
    {
      QgsExpressionUtils::TVL tvl = QgsExpressionUtils::getTVLValue( val, parent );
      ENSURE_NO_EVAL_ERROR
      return QgsExpressionUtils::tvl2variant( QgsExpressionUtils::NOT[tvl] );
    }

    case uoMinus:
      if ( QgsExpressionUtils::isIntSafe( val ) )
        return QVariant( - QgsExpressionUtils::getIntValue( val, parent ) );
      else if ( QgsExpressionUtils::isDoubleSafe( val ) )
        return QVariant( - QgsExpressionUtils::getDoubleValue( val, parent ) );
      else
        SET_EVAL_ERROR( tr( "Unary minus only for numeric values." ) )
      }
  return QVariant();
}

QgsExpressionNode::NodeType QgsExpressionNodeUnaryOperator::nodeType() const
{
  return ntUnaryOperator;
}

bool QgsExpressionNodeUnaryOperator::prepareNode( QgsExpression *parent, const QgsExpressionContext *context )
{
  return mOperand->prepare( parent, context );
}

QString QgsExpressionNodeUnaryOperator::dump() const
{
  if ( dynamic_cast<QgsExpressionNodeBinaryOperator *>( mOperand ) )
    return QStringLiteral( "%1 ( %2 )" ).arg( UNARY_OPERATOR_TEXT[mOp], mOperand->dump() );
  else
    return QStringLiteral( "%1 %2" ).arg( UNARY_OPERATOR_TEXT[mOp], mOperand->dump() );
}

QSet<QString> QgsExpressionNodeUnaryOperator::referencedColumns() const
{
  if ( hasCachedStaticValue() )
    return QSet< QString >();

  return mOperand->referencedColumns();
}

QSet<QString> QgsExpressionNodeUnaryOperator::referencedVariables() const
{
  return mOperand->referencedVariables();
}

QSet<QString> QgsExpressionNodeUnaryOperator::referencedFunctions() const
{
  return mOperand->referencedFunctions();
}

QList<const QgsExpressionNode *> QgsExpressionNodeUnaryOperator::nodes() const
{
  QList<const QgsExpressionNode *> lst;
  lst.append( this );
  lst += mOperand->nodes();
  return lst;
}

bool QgsExpressionNodeUnaryOperator::needsGeometry() const
{
  return mOperand->needsGeometry();
}

QgsExpressionNode *QgsExpressionNodeUnaryOperator::clone() const
{
  QgsExpressionNodeUnaryOperator *copy = new QgsExpressionNodeUnaryOperator( mOp, mOperand->clone() );
  cloneTo( copy );
  return copy;
}

bool QgsExpressionNodeUnaryOperator::isStatic( QgsExpression *parent, const QgsExpressionContext *context ) const
{
  return mOperand->isStatic( parent, context );
}

QString QgsExpressionNodeUnaryOperator::text() const
{
  return UNARY_OPERATOR_TEXT[mOp];
}

//

QVariant QgsExpressionNodeBinaryOperator::evalNode( QgsExpression *parent, const QgsExpressionContext *context )
{
  QVariant vL = mOpLeft->eval( parent, context );
  ENSURE_NO_EVAL_ERROR

  if ( mOp == boAnd || mOp == boOr )
  {
    QgsExpressionUtils::TVL tvlL = QgsExpressionUtils::getTVLValue( vL, parent );
    ENSURE_NO_EVAL_ERROR
    if ( mOp == boAnd && tvlL == QgsExpressionUtils::False )
      return TVL_False;  // shortcut -- no need to evaluate right-hand side
    if ( mOp == boOr && tvlL == QgsExpressionUtils::True )
      return TVL_True;  // shortcut -- no need to evaluate right-hand side
  }

  QVariant vR = mOpRight->eval( parent, context );
  ENSURE_NO_EVAL_ERROR

  switch ( mOp )
  {
    case boPlus:
      if ( vL.type() == QVariant::String && vR.type() == QVariant::String )
      {
        QString sL = QgsExpressionUtils::isNull( vL ) ? QString() : QgsExpressionUtils::getStringValue( vL, parent );
        ENSURE_NO_EVAL_ERROR
        QString sR = QgsExpressionUtils::isNull( vR ) ? QString() : QgsExpressionUtils::getStringValue( vR, parent );
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
      if ( QgsExpressionUtils::isNull( vL ) || QgsExpressionUtils::isNull( vR ) )
        return QVariant();
      else if ( mOp != boDiv && QgsExpressionUtils::isIntSafe( vL ) && QgsExpressionUtils::isIntSafe( vR ) )
      {
        // both are integers - let's use integer arithmetic
        qlonglong iL = QgsExpressionUtils::getIntValue( vL, parent );
        ENSURE_NO_EVAL_ERROR
        qlonglong iR = QgsExpressionUtils::getIntValue( vR, parent );
        ENSURE_NO_EVAL_ERROR

        if ( mOp == boMod && iR == 0 )
          return QVariant();

        return QVariant( computeInt( iL, iR ) );
      }
      else if ( QgsExpressionUtils::isDateTimeSafe( vL ) && QgsExpressionUtils::isIntervalSafe( vR ) )
      {
        QDateTime dL = QgsExpressionUtils::getDateTimeValue( vL, parent );
        ENSURE_NO_EVAL_ERROR
        QgsInterval iL = QgsExpressionUtils::getInterval( vR, parent );
        ENSURE_NO_EVAL_ERROR
        if ( mOp == boDiv || mOp == boMul || mOp == boMod )
        {
          parent->setEvalErrorString( tr( "Can't perform /, *, or % on DateTime and Interval" ) );
          return QVariant();
        }
        return QVariant( computeDateTimeFromInterval( dL, &iL ) );
      }
      else if ( mOp == boPlus && ( ( vL.type() == QVariant::Date && vR.type() == QVariant::Time ) ||
                                   ( vR.type() == QVariant::Date && vL.type() == QVariant::Time ) ) )
      {
        QDate date = QgsExpressionUtils::getDateValue( vL.type() == QVariant::Date ? vL : vR, parent );
        ENSURE_NO_EVAL_ERROR
        QTime time = QgsExpressionUtils::getTimeValue( vR.type() == QVariant::Time ? vR : vL, parent );
        ENSURE_NO_EVAL_ERROR
        QDateTime dt = QDateTime( date, time );
        return QVariant( dt );
      }
      else if ( mOp == boMinus && vL.type() == QVariant::Date && vR.type() == QVariant::Date )
      {
        QDate date1 = QgsExpressionUtils::getDateValue( vL, parent );
        ENSURE_NO_EVAL_ERROR
        QDate date2 = QgsExpressionUtils::getDateValue( vR, parent );
        ENSURE_NO_EVAL_ERROR
        return date1 - date2;
      }
      else if ( mOp == boMinus && vL.type() == QVariant::Time && vR.type() == QVariant::Time )
      {
        QTime time1 = QgsExpressionUtils::getTimeValue( vL, parent );
        ENSURE_NO_EVAL_ERROR
        QTime time2 = QgsExpressionUtils::getTimeValue( vR, parent );
        ENSURE_NO_EVAL_ERROR
        return time1 - time2;
      }
      else if ( mOp == boMinus && vL.type() == QVariant::DateTime && vR.type() == QVariant::DateTime )
      {
        QDateTime datetime1 = QgsExpressionUtils::getDateTimeValue( vL, parent );
        ENSURE_NO_EVAL_ERROR
        QDateTime datetime2 = QgsExpressionUtils::getDateTimeValue( vR, parent );
        ENSURE_NO_EVAL_ERROR
        return datetime1 - datetime2;
      }
      else
      {
        // general floating point arithmetic
        double fL = QgsExpressionUtils::getDoubleValue( vL, parent );
        ENSURE_NO_EVAL_ERROR
        double fR = QgsExpressionUtils::getDoubleValue( vR, parent );
        ENSURE_NO_EVAL_ERROR
        if ( ( mOp == boDiv || mOp == boMod ) && fR == 0. )
          return QVariant(); // silently handle division by zero and return NULL
        return QVariant( computeDouble( fL, fR ) );
      }
    }
    case boIntDiv:
    {
      //integer division
      double fL = QgsExpressionUtils::getDoubleValue( vL, parent );
      ENSURE_NO_EVAL_ERROR
      double fR = QgsExpressionUtils::getDoubleValue( vR, parent );
      ENSURE_NO_EVAL_ERROR
      if ( fR == 0. )
        return QVariant(); // silently handle division by zero and return NULL
      return QVariant( qlonglong( std::floor( fL / fR ) ) );
    }
    case boPow:
      if ( QgsExpressionUtils::isNull( vL ) || QgsExpressionUtils::isNull( vR ) )
        return QVariant();
      else
      {
        double fL = QgsExpressionUtils::getDoubleValue( vL, parent );
        ENSURE_NO_EVAL_ERROR
        double fR = QgsExpressionUtils::getDoubleValue( vR, parent );
        ENSURE_NO_EVAL_ERROR
        return QVariant( std::pow( fL, fR ) );
      }

    case boAnd:
    {
      QgsExpressionUtils::TVL tvlL = QgsExpressionUtils::getTVLValue( vL, parent ), tvlR = QgsExpressionUtils::getTVLValue( vR, parent );
      ENSURE_NO_EVAL_ERROR
      return  QgsExpressionUtils::tvl2variant( QgsExpressionUtils::AND[tvlL][tvlR] );
    }

    case boOr:
    {
      QgsExpressionUtils::TVL tvlL = QgsExpressionUtils::getTVLValue( vL, parent ), tvlR = QgsExpressionUtils::getTVLValue( vR, parent );
      ENSURE_NO_EVAL_ERROR
      return  QgsExpressionUtils::tvl2variant( QgsExpressionUtils::OR[tvlL][tvlR] );
    }

    case boEQ:
    case boNE:
    case boLT:
    case boGT:
    case boLE:
    case boGE:
      if ( QgsExpressionUtils::isNull( vL ) || QgsExpressionUtils::isNull( vR ) )
      {
        return TVL_Unknown;
      }
      else if ( QgsExpressionUtils::isList( vL ) || QgsExpressionUtils::isList( vR ) )
      {
        // verify that we have two lists
        if ( !QgsExpressionUtils::isList( vL ) || !QgsExpressionUtils::isList( vR ) )
          return TVL_Unknown;

        // and search for not equal respective items
        QVariantList lL = vL.toList();
        QVariantList lR = vR.toList();
        for ( int i = 0; i < lL.length() && i < lR.length(); i++ )
        {
          if ( QgsExpressionUtils::isNull( lL.at( i ) ) && QgsExpressionUtils::isNull( lR.at( i ) ) )
            continue;  // same behavior as PostgreSQL

          if ( QgsExpressionUtils::isNull( lL.at( i ) ) || QgsExpressionUtils::isNull( lR.at( i ) ) )
          {
            switch ( mOp )
            {
              case boEQ:
                return false;
              case boNE:
                return true;
              case boLT:
              case boLE:
                return QgsExpressionUtils::isNull( lR.at( i ) );
              case boGT:
              case boGE:
                return QgsExpressionUtils::isNull( lL.at( i ) );
              default:
                Q_ASSERT( false );
                return TVL_Unknown;
            }
          }

          QgsExpressionNodeLiteral nL( lL.at( i ) );
          QgsExpressionNodeLiteral nR( lR.at( i ) );
          QgsExpressionNodeBinaryOperator eqNode( boEQ, nL.clone(), nR.clone() );
          QVariant eq = eqNode.eval( parent, context );
          ENSURE_NO_EVAL_ERROR
          if ( eq == TVL_False )
          {
            // return the two items comparison
            QgsExpressionNodeBinaryOperator node( mOp, nL.clone(), nR.clone() );
            QVariant v = node.eval( parent, context );
            ENSURE_NO_EVAL_ERROR
            return v;
          }
        }

        // default to length comparison
        switch ( mOp )
        {
          case boEQ:
            return lL.length() == lR.length();
          case boNE:
            return lL.length() != lR.length();
          case boLT:
            return lL.length() < lR.length();
          case boGT:
            return lL.length() > lR.length();
          case boLE:
            return lL.length() <= lR.length();
          case boGE:
            return lL.length() >= lR.length();
          default:
            Q_ASSERT( false );
            return TVL_Unknown;
        }
      }
      else if ( ( vL.type() == QVariant::DateTime && vR.type() == QVariant::DateTime ) )
      {
        QDateTime dL = QgsExpressionUtils::getDateTimeValue( vL, parent );
        ENSURE_NO_EVAL_ERROR
        QDateTime dR = QgsExpressionUtils::getDateTimeValue( vR, parent );
        ENSURE_NO_EVAL_ERROR

        // while QDateTime has innate handling of timezones, we don't expose these ANYWHERE
        // in QGIS. So to avoid confusion where seemingly equal datetime values give unexpected
        // results (due to different hidden timezones), we force all datetime comparisons to treat
        // all datetime values as having the same time zone
        dL.setTimeSpec( Qt::UTC );
        dR.setTimeSpec( Qt::UTC );

        return compare( dR.msecsTo( dL ) ) ? TVL_True : TVL_False;
      }
      else if ( ( vL.type() == QVariant::Date && vR.type() == QVariant::Date ) )
      {
        const QDate dL = QgsExpressionUtils::getDateValue( vL, parent );
        ENSURE_NO_EVAL_ERROR
        const QDate dR = QgsExpressionUtils::getDateValue( vR, parent );
        ENSURE_NO_EVAL_ERROR
        return compare( dR.daysTo( dL ) ) ? TVL_True : TVL_False;
      }
      else if ( ( vL.type() == QVariant::Time && vR.type() == QVariant::Time ) )
      {
        const QTime dL = QgsExpressionUtils::getTimeValue( vL, parent );
        ENSURE_NO_EVAL_ERROR
        const QTime dR = QgsExpressionUtils::getTimeValue( vR, parent );
        ENSURE_NO_EVAL_ERROR
        return compare( dR.msecsTo( dL ) ) ? TVL_True : TVL_False;
      }
      else if ( ( vL.type() != QVariant::String || vR.type() != QVariant::String ) &&
                QgsExpressionUtils::isDoubleSafe( vL ) && QgsExpressionUtils::isDoubleSafe( vR ) )
      {
        // do numeric comparison if both operators can be converted to numbers,
        // and they aren't both string
        double fL = QgsExpressionUtils::getDoubleValue( vL, parent );
        ENSURE_NO_EVAL_ERROR
        double fR = QgsExpressionUtils::getDoubleValue( vR, parent );
        ENSURE_NO_EVAL_ERROR
        return compare( fL - fR ) ? TVL_True : TVL_False;
      }
      // warning - QgsExpression::isIntervalSafe is VERY expensive and should not be used here
      else if ( vL.canConvert< QgsInterval >() && vR.canConvert< QgsInterval >() )
      {
        double fL = QgsExpressionUtils::getInterval( vL, parent ).seconds();
        ENSURE_NO_EVAL_ERROR
        double fR = QgsExpressionUtils::getInterval( vR, parent ).seconds();
        ENSURE_NO_EVAL_ERROR
        return compare( fL - fR ) ? TVL_True : TVL_False;
      }
      else
      {
        // do string comparison otherwise
        QString sL = QgsExpressionUtils::getStringValue( vL, parent );
        ENSURE_NO_EVAL_ERROR
        QString sR = QgsExpressionUtils::getStringValue( vR, parent );
        ENSURE_NO_EVAL_ERROR
        int diff = QString::compare( sL, sR );
        return compare( diff ) ? TVL_True : TVL_False;
      }

    case boIs:
    case boIsNot:
      if ( QgsExpressionUtils::isNull( vL ) && QgsExpressionUtils::isNull( vR ) ) // both operators null
        return ( mOp == boIs ? TVL_True : TVL_False );
      else if ( QgsExpressionUtils::isNull( vL ) || QgsExpressionUtils::isNull( vR ) ) // one operator null
        return ( mOp == boIs ? TVL_False : TVL_True );
      else // both operators non-null
      {
        bool equal = false;
        if ( QgsExpressionUtils::isDoubleSafe( vL ) && QgsExpressionUtils::isDoubleSafe( vR ) &&
             ( vL.type() != QVariant::String || vR.type() != QVariant::String ) )
        {
          double fL = QgsExpressionUtils::getDoubleValue( vL, parent );
          ENSURE_NO_EVAL_ERROR
          double fR = QgsExpressionUtils::getDoubleValue( vR, parent );
          ENSURE_NO_EVAL_ERROR
          equal = qgsDoubleNear( fL, fR );
        }
        else
        {
          QString sL = QgsExpressionUtils::getStringValue( vL, parent );
          ENSURE_NO_EVAL_ERROR
          QString sR = QgsExpressionUtils::getStringValue( vR, parent );
          ENSURE_NO_EVAL_ERROR
          equal = QString::compare( sL, sR ) == 0;
        }
        if ( equal )
          return mOp == boIs ? TVL_True : TVL_False;
        else
          return mOp == boIs ? TVL_False : TVL_True;
      }

    case boRegexp:
    case boLike:
    case boNotLike:
    case boILike:
    case boNotILike:
      if ( QgsExpressionUtils::isNull( vL ) || QgsExpressionUtils::isNull( vR ) )
        return TVL_Unknown;
      else
      {
        QString str    = QgsExpressionUtils::getStringValue( vL, parent );
        ENSURE_NO_EVAL_ERROR
        QString regexp = QgsExpressionUtils::getStringValue( vR, parent );
        ENSURE_NO_EVAL_ERROR
        // TODO: cache QRegularExpression in case that regexp is a literal string (i.e. it will stay constant)
        bool matches;
        if ( mOp == boLike || mOp == boILike || mOp == boNotLike || mOp == boNotILike ) // change from LIKE syntax to regexp
        {
          QString esc_regexp = QgsStringUtils::qRegExpEscape( regexp );
          // manage escape % and _
          if ( esc_regexp.startsWith( '%' ) )
          {
            esc_regexp.replace( 0, 1, QStringLiteral( ".*" ) );
          }
          const thread_local QRegularExpression rx1( QStringLiteral( "[^\\\\](%)" ) );
          int pos = 0;
          while ( ( pos = esc_regexp.indexOf( rx1, pos ) ) != -1 )
          {
            esc_regexp.replace( pos + 1, 1, QStringLiteral( ".*" ) );
            pos += 1;
          }
          const thread_local QRegularExpression rx2( QStringLiteral( "\\\\%" ) );
          esc_regexp.replace( rx2, QStringLiteral( "%" ) );
          if ( esc_regexp.startsWith( '_' ) )
          {
            esc_regexp.replace( 0, 1, QStringLiteral( "." ) );
          }
          const thread_local QRegularExpression rx3( QStringLiteral( "[^\\\\](_)" ) );
          pos = 0;
          while ( ( pos = esc_regexp.indexOf( rx3, pos ) ) != -1 )
          {
            esc_regexp.replace( pos + 1, 1, '.' );
            pos += 1;
          }
          esc_regexp.replace( QLatin1String( "\\\\_" ), QLatin1String( "_" ) );

          matches = QRegularExpression( QRegularExpression::anchoredPattern( esc_regexp ), mOp == boLike || mOp == boNotLike ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption ).match( str ).hasMatch();
        }
        else
        {
          matches = QRegularExpression( regexp ).match( str ).hasMatch();
        }

        if ( mOp == boNotLike || mOp == boNotILike )
        {
          matches = !matches;
        }

        return matches ? TVL_True : TVL_False;
      }

    case boConcat:
      if ( QgsExpressionUtils::isNull( vL ) || QgsExpressionUtils::isNull( vR ) )
        return QVariant();
      else
      {
        QString sL = QgsExpressionUtils::getStringValue( vL, parent );
        ENSURE_NO_EVAL_ERROR
        QString sR = QgsExpressionUtils::getStringValue( vR, parent );
        ENSURE_NO_EVAL_ERROR
        return QVariant( sL + sR );
      }
  }
  Q_ASSERT( false );
  return QVariant();
}

bool QgsExpressionNodeBinaryOperator::compare( double diff )
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

qlonglong QgsExpressionNodeBinaryOperator::computeInt( qlonglong x, qlonglong y )
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

QDateTime QgsExpressionNodeBinaryOperator::computeDateTimeFromInterval( const QDateTime &d, QgsInterval *i )
{
  switch ( mOp )
  {
    case boPlus:
      return d.addSecs( i->seconds() );
    case boMinus:
      return d.addSecs( -i->seconds() );
    default:
      Q_ASSERT( false );
      return QDateTime();
  }
}

double QgsExpressionNodeBinaryOperator::computeDouble( double x, double y )
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

QgsExpressionNode::NodeType QgsExpressionNodeBinaryOperator::nodeType() const
{
  return ntBinaryOperator;
}

bool QgsExpressionNodeBinaryOperator::prepareNode( QgsExpression *parent, const QgsExpressionContext *context )
{
  bool resL = mOpLeft->prepare( parent, context );
  bool resR = mOpRight->prepare( parent, context );
  return resL && resR;
}

int QgsExpressionNodeBinaryOperator::precedence() const
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
    case boRegexp:
    case boLike:
    case boILike:
    case boNotLike:
    case boNotILike:
    case boIs:
    case boIsNot:
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

    case boConcat:
      return 7;
  }
  Q_ASSERT( false && "unexpected binary operator" );
  return -1;
}

bool QgsExpressionNodeBinaryOperator::leftAssociative() const
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
    case boRegexp:
    case boLike:
    case boILike:
    case boNotLike:
    case boNotILike:
    case boIs:
    case boIsNot:
    case boPlus:
    case boMinus:
    case boMul:
    case boDiv:
    case boIntDiv:
    case boMod:
    case boConcat:
      return true;

    case boPow:
      return false;
  }
  Q_ASSERT( false && "unexpected binary operator" );
  return false;
}

QString QgsExpressionNodeBinaryOperator::dump() const
{
  QgsExpressionNodeBinaryOperator *lOp = dynamic_cast<QgsExpressionNodeBinaryOperator *>( mOpLeft );
  QgsExpressionNodeBinaryOperator *rOp = dynamic_cast<QgsExpressionNodeBinaryOperator *>( mOpRight );
  QgsExpressionNodeUnaryOperator *ruOp = dynamic_cast<QgsExpressionNodeUnaryOperator *>( mOpRight );

  QString rdump( mOpRight->dump() );

  // avoid dumping "IS (NOT ...)" as "IS NOT ..."
  if ( mOp == boIs && ruOp && ruOp->op() == QgsExpressionNodeUnaryOperator::uoNot )
  {
    rdump.prepend( '(' ).append( ')' );
  }

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

QSet<QString> QgsExpressionNodeBinaryOperator::referencedColumns() const
{
  if ( hasCachedStaticValue() )
    return QSet< QString >();

  return mOpLeft->referencedColumns() + mOpRight->referencedColumns();
}

QSet<QString> QgsExpressionNodeBinaryOperator::referencedVariables() const
{
  return mOpLeft->referencedVariables() + mOpRight->referencedVariables();
}

QSet<QString> QgsExpressionNodeBinaryOperator::referencedFunctions() const
{
  return mOpLeft->referencedFunctions() + mOpRight->referencedFunctions();
}

QList<const QgsExpressionNode *> QgsExpressionNodeBinaryOperator::nodes() const
{
  QList<const QgsExpressionNode *> lst;
  lst << this;
  lst += mOpLeft->nodes() + mOpRight->nodes();
  return lst;
}

bool QgsExpressionNodeBinaryOperator::needsGeometry() const
{
  return mOpLeft->needsGeometry() || mOpRight->needsGeometry();
}

QgsExpressionNode *QgsExpressionNodeBinaryOperator::clone() const
{
  QgsExpressionNodeBinaryOperator *copy = new QgsExpressionNodeBinaryOperator( mOp, mOpLeft->clone(), mOpRight->clone() );
  cloneTo( copy );
  return copy;
}

bool QgsExpressionNodeBinaryOperator::isStatic( QgsExpression *parent, const QgsExpressionContext *context ) const
{
  const bool leftStatic = mOpLeft->isStatic( parent, context );
  const bool rightStatic = mOpRight->isStatic( parent, context );

  if ( leftStatic && rightStatic )
    return true;

  // special logic for certain ops...
  switch ( mOp )
  {
    case QgsExpressionNodeBinaryOperator::boOr:
    {
      // if either node is static AND evaluates to TRUE, then the result will ALWAYS be true regardless
      // of the value of the other node!
      if ( leftStatic )
      {
        mOpLeft->prepare( parent, context );
        if ( mOpLeft->hasCachedStaticValue() )
        {
          QgsExpressionUtils::TVL tvl = QgsExpressionUtils::getTVLValue( mOpLeft->cachedStaticValue(), parent );
          if ( !parent->hasEvalError() && tvl == QgsExpressionUtils::True )
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
          QgsExpressionUtils::TVL tvl = QgsExpressionUtils::getTVLValue( mOpRight->cachedStaticValue(), parent );
          if ( !parent->hasEvalError() && tvl == QgsExpressionUtils::True )
          {
            mCachedStaticValue = true;
            mHasCachedValue = true;
            return true;
          }
        }
      }

      break;
    }
    case QgsExpressionNodeBinaryOperator::boAnd:
    {
      // if either node is static AND evaluates to FALSE, then the result will ALWAYS be false regardless
      // of the value of the other node!

      if ( leftStatic )
      {
        mOpLeft->prepare( parent, context );
        if ( mOpLeft->hasCachedStaticValue() )
        {
          QgsExpressionUtils::TVL tvl = QgsExpressionUtils::getTVLValue( mOpLeft->cachedStaticValue(), parent );
          if ( !parent->hasEvalError() && tvl == QgsExpressionUtils::False )
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
          QgsExpressionUtils::TVL tvl = QgsExpressionUtils::getTVLValue( mOpRight->cachedStaticValue(), parent );
          if ( !parent->hasEvalError() && tvl == QgsExpressionUtils::False )
          {
            mCachedStaticValue = false;
            mHasCachedValue = true;
            return true;
          }
        }
      }

      break;
    }

    case QgsExpressionNodeBinaryOperator::boEQ:
    case QgsExpressionNodeBinaryOperator::boNE:
    case QgsExpressionNodeBinaryOperator::boLE:
    case QgsExpressionNodeBinaryOperator::boGE:
    case QgsExpressionNodeBinaryOperator::boLT:
    case QgsExpressionNodeBinaryOperator::boGT:
    case QgsExpressionNodeBinaryOperator::boRegexp:
    case QgsExpressionNodeBinaryOperator::boLike:
    case QgsExpressionNodeBinaryOperator::boNotLike:
    case QgsExpressionNodeBinaryOperator::boILike:
    case QgsExpressionNodeBinaryOperator::boNotILike:
    case QgsExpressionNodeBinaryOperator::boIs:
    case QgsExpressionNodeBinaryOperator::boIsNot:
    case QgsExpressionNodeBinaryOperator::boPlus:
    case QgsExpressionNodeBinaryOperator::boMinus:
    case QgsExpressionNodeBinaryOperator::boMul:
    case QgsExpressionNodeBinaryOperator::boDiv:
    case QgsExpressionNodeBinaryOperator::boIntDiv:
    case QgsExpressionNodeBinaryOperator::boMod:
    case QgsExpressionNodeBinaryOperator::boPow:
    case QgsExpressionNodeBinaryOperator::boConcat:
      break;
  }

  return false;
}

//

QVariant QgsExpressionNodeInOperator::evalNode( QgsExpression *parent, const QgsExpressionContext *context )
{
  if ( mList->count() == 0 )
    return mNotIn ? TVL_True : TVL_False;
  QVariant v1 = mNode->eval( parent, context );
  ENSURE_NO_EVAL_ERROR
  if ( QgsExpressionUtils::isNull( v1 ) )
    return TVL_Unknown;

  bool listHasNull = false;

  const QList< QgsExpressionNode * > nodeList = mList->list();
  for ( QgsExpressionNode *n : nodeList )
  {
    QVariant v2 = n->eval( parent, context );
    ENSURE_NO_EVAL_ERROR
    if ( QgsExpressionUtils::isNull( v2 ) )
      listHasNull = true;
    else
    {
      bool equal = false;
      // check whether they are equal
      if ( ( v1.type() != QVariant::String || v2.type() != QVariant::String ) &&
           QgsExpressionUtils::isDoubleSafe( v1 ) && QgsExpressionUtils::isDoubleSafe( v2 ) )
      {
        // do numeric comparison if both operators can be converted to numbers,
        // and they aren't both string
        double f1 = QgsExpressionUtils::getDoubleValue( v1, parent );
        ENSURE_NO_EVAL_ERROR
        double f2 = QgsExpressionUtils::getDoubleValue( v2, parent );
        ENSURE_NO_EVAL_ERROR
        equal = qgsDoubleNear( f1, f2 );
      }
      else
      {
        QString s1 = QgsExpressionUtils::getStringValue( v1, parent );
        ENSURE_NO_EVAL_ERROR
        QString s2 = QgsExpressionUtils::getStringValue( v2, parent );
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

QgsExpressionNodeInOperator::~QgsExpressionNodeInOperator()
{
  delete mNode;
  delete mList;
}

QgsExpressionNode::NodeType QgsExpressionNodeInOperator::nodeType() const
{
  return ntInOperator;
}

bool QgsExpressionNodeInOperator::prepareNode( QgsExpression *parent, const QgsExpressionContext *context )
{
  bool res = mNode->prepare( parent, context );
  const QList< QgsExpressionNode * > nodeList = mList->list();
  for ( QgsExpressionNode *n : nodeList )
  {
    res = res && n->prepare( parent, context );
  }
  return res;
}

QString QgsExpressionNodeInOperator::dump() const
{
  return QStringLiteral( "%1 %2 IN (%3)" ).arg( mNode->dump(), mNotIn ? "NOT" : "", mList->dump() );
}

QgsExpressionNode *QgsExpressionNodeInOperator::clone() const
{
  QgsExpressionNodeInOperator *copy = new QgsExpressionNodeInOperator( mNode->clone(), mList->clone(), mNotIn );
  cloneTo( copy );
  return copy;
}

bool QgsExpressionNodeInOperator::isStatic( QgsExpression *parent, const QgsExpressionContext *context ) const
{
  if ( !mNode->isStatic( parent, context ) )
    return false;

  const QList< QgsExpressionNode * > nodeList = mList->list();
  for ( QgsExpressionNode *n : nodeList )
  {
    if ( !n->isStatic( parent, context ) )
      return false;
  }

  return true;
}

//

QVariant QgsExpressionNodeFunction::evalNode( QgsExpression *parent, const QgsExpressionContext *context )
{
  QString name = QgsExpression::QgsExpression::Functions()[mFnIndex]->name();
  QgsExpressionFunction *fd = context && context->hasFunction( name ) ? context->function( name ) : QgsExpression::QgsExpression::Functions()[mFnIndex];

  QVariant res = fd->run( mArgs, context, parent, this );
  ENSURE_NO_EVAL_ERROR

  // everything went fine
  return res;
}

QgsExpressionNodeFunction::QgsExpressionNodeFunction( int fnIndex, QgsExpressionNode::NodeList *args )
  : mFnIndex( fnIndex )
{
  const QgsExpressionFunction::ParameterList &functionParams = QgsExpression::QgsExpression::Functions()[mFnIndex]->parameters();
  if ( functionParams.isEmpty() )
  {
    // function does not support parameters
    mArgs = args;
  }
  else if ( !args )
  {
    // no arguments specified, but function has parameters. Build a list of default parameter values for the arguments list.
    mArgs = new NodeList();
    for ( const QgsExpressionFunction::Parameter &param : functionParams )
    {
      // insert default value for QgsExpressionFunction::Parameter
      mArgs->append( new QgsExpressionNodeLiteral( param.defaultValue() ) );
    }
  }
  else
  {
    mArgs = new NodeList();

    int idx = 0;
    //first loop through unnamed arguments
    while ( idx < args->names().size() && args->names().at( idx ).isEmpty() )
    {
      mArgs->append( args->list().at( idx )->clone() );
      idx++;
    }

    //next copy named QgsExpressionFunction::Parameters in order expected by function
    for ( ; idx < functionParams.count(); ++idx )
    {
      int nodeIdx = args->names().indexOf( functionParams.at( idx ).name().toLower() );
      if ( nodeIdx < 0 )
      {
        //QgsExpressionFunction::Parameter not found - insert default value for QgsExpressionFunction::Parameter
        mArgs->append( new QgsExpressionNodeLiteral( functionParams.at( idx ).defaultValue() ) );
      }
      else
      {
        mArgs->append( args->list().at( nodeIdx )->clone() );
      }
    }

    delete args;
  }
}

QgsExpressionNodeFunction::~QgsExpressionNodeFunction()
{
  delete mArgs;
}

QgsExpressionNode::NodeType QgsExpressionNodeFunction::nodeType() const
{
  return ntFunction;
}

bool QgsExpressionNodeFunction::prepareNode( QgsExpression *parent, const QgsExpressionContext *context )
{
  QgsExpressionFunction *fd = QgsExpression::QgsExpression::Functions()[mFnIndex];

  bool res = fd->prepare( this, parent, context );
  if ( mArgs && !fd->lazyEval() )
  {
    const QList< QgsExpressionNode * > nodeList = mArgs->list();
    for ( QgsExpressionNode *n : nodeList )
    {
      res = res && n->prepare( parent, context );
    }
  }
  return res;
}

QString QgsExpressionNodeFunction::dump() const
{
  QgsExpressionFunction *fd = QgsExpression::QgsExpression::Functions()[mFnIndex];
  if ( fd->params() == 0 )
    return QStringLiteral( "%1%2" ).arg( fd->name(), fd->name().startsWith( '$' ) ? QString() : QStringLiteral( "()" ) ); // special column
  else
    return QStringLiteral( "%1(%2)" ).arg( fd->name(), mArgs ? mArgs->dump() : QString() ); // function
}

QSet<QString> QgsExpressionNodeFunction::referencedColumns() const
{
  if ( hasCachedStaticValue() )
    return QSet< QString >();

  QgsExpressionFunction *fd = QgsExpression::QgsExpression::Functions()[mFnIndex];
  QSet<QString> functionColumns = fd->referencedColumns( this );

  if ( !mArgs )
  {
    //no referenced columns in arguments, just return function's referenced columns
    return functionColumns;
  }

  int paramIndex = 0;
  const QList< QgsExpressionNode * > nodeList = mArgs->list();
  for ( QgsExpressionNode *n : nodeList )
  {
    if ( fd->parameters().count() <= paramIndex || !fd->parameters().at( paramIndex ).isSubExpression() )
      functionColumns.unite( n->referencedColumns() );
    paramIndex++;
  }

  return functionColumns;
}

QSet<QString> QgsExpressionNodeFunction::referencedVariables() const
{
  QgsExpressionFunction *fd = QgsExpression::QgsExpression::Functions()[mFnIndex];
  if ( fd->name() == QLatin1String( "var" ) )
  {
    if ( !mArgs->list().isEmpty() )
    {
      QgsExpressionNodeLiteral *var = dynamic_cast<QgsExpressionNodeLiteral *>( mArgs->list().at( 0 ) );
      if ( var )
        return QSet<QString>() << var->value().toString();
    }
    return QSet<QString>() << QString();
  }
  else
  {
    QSet<QString> functionVariables = QSet<QString>();

    if ( !mArgs )
      return functionVariables;

    const QList< QgsExpressionNode * > nodeList = mArgs->list();
    for ( QgsExpressionNode *n : nodeList )
    {
      functionVariables.unite( n->referencedVariables() );
    }

    return functionVariables;
  }
}

QSet<QString> QgsExpressionNodeFunction::referencedFunctions() const
{
  QgsExpressionFunction *fd = QgsExpression::QgsExpression::Functions()[mFnIndex];
  QSet<QString> functions = QSet<QString>();
  functions.insert( fd->name() );

  if ( !mArgs )
    return functions;

  const QList< QgsExpressionNode * > nodeList = mArgs->list();
  for ( QgsExpressionNode *n : nodeList )
  {
    functions.unite( n->referencedFunctions() );
  }
  return functions;
}

QList<const QgsExpressionNode *> QgsExpressionNodeFunction::nodes() const
{
  QList<const QgsExpressionNode *> lst;
  lst << this;
  if ( !mArgs )
    return lst;

  const QList< QgsExpressionNode * > nodeList = mArgs->list();
  for ( QgsExpressionNode *n : nodeList )
  {
    lst += n->nodes();
  }
  return lst;
}

bool QgsExpressionNodeFunction::needsGeometry() const
{
  bool needs = QgsExpression::QgsExpression::Functions()[mFnIndex]->usesGeometry( this );
  if ( mArgs )
  {
    const QList< QgsExpressionNode * > nodeList = mArgs->list();
    for ( QgsExpressionNode *n : nodeList )
      needs |= n->needsGeometry();
  }
  return needs;
}

QgsExpressionNode *QgsExpressionNodeFunction::clone() const
{
  QgsExpressionNodeFunction *copy = new QgsExpressionNodeFunction( mFnIndex, mArgs ? mArgs->clone() : nullptr );
  cloneTo( copy );
  return copy;
}

bool QgsExpressionNodeFunction::isStatic( QgsExpression *parent, const QgsExpressionContext *context ) const
{
  return QgsExpression::Functions()[mFnIndex]->isStatic( this, parent, context );
}

bool QgsExpressionNodeFunction::validateParams( int fnIndex, QgsExpressionNode::NodeList *args, QString &error )
{
  if ( !args || !args->hasNamedNodes() )
    return true;

  const QgsExpressionFunction::ParameterList &functionParams = QgsExpression::Functions()[fnIndex]->parameters();
  if ( functionParams.isEmpty() )
  {
    error = QStringLiteral( "%1 does not support named QgsExpressionFunction::Parameters" ).arg( QgsExpression::Functions()[fnIndex]->name() );
    return false;
  }
  else
  {
    QSet< int > providedArgs;
    QSet< int > handledArgs;
    int idx = 0;
    //first loop through unnamed arguments
    while ( args->names().at( idx ).isEmpty() )
    {
      providedArgs << idx;
      handledArgs << idx;
      idx++;
    }

    //next check named QgsExpressionFunction::Parameters
    for ( ; idx < functionParams.count(); ++idx )
    {
      int nodeIdx = args->names().indexOf( functionParams.at( idx ).name().toLower() );
      if ( nodeIdx < 0 )
      {
        if ( !functionParams.at( idx ).optional() )
        {
          error = QStringLiteral( "No value specified for QgsExpressionFunction::Parameter '%1' for %2" ).arg( functionParams.at( idx ).name(), QgsExpression::Functions()[fnIndex]->name() );
          return false;
        }
      }
      else
      {
        if ( providedArgs.contains( idx ) )
        {
          error = QStringLiteral( "Duplicate QgsExpressionFunction::Parameter specified for '%1' for %2" ).arg( functionParams.at( idx ).name(), QgsExpression::Functions()[fnIndex]->name() );
          return false;
        }
      }
      providedArgs << idx;
      handledArgs << nodeIdx;
    }

    //last check for bad names
    idx = 0;
    const QStringList nameList = args->names();
    for ( const QString &name : nameList )
    {
      if ( !name.isEmpty() && !functionParams.contains( name ) )
      {
        error = QStringLiteral( "Invalid QgsExpressionFunction::Parameter name '%1' for %2" ).arg( name, QgsExpression::Functions()[fnIndex]->name() );
        return false;
      }
      if ( !name.isEmpty() && !handledArgs.contains( idx ) )
      {
        int functionIdx = functionParams.indexOf( name );
        if ( providedArgs.contains( functionIdx ) )
        {
          error = QStringLiteral( "Duplicate QgsExpressionFunction::Parameter specified for '%1' for %2" ).arg( functionParams.at( functionIdx ).name(), QgsExpression::Functions()[fnIndex]->name() );
          return false;
        }
      }
      idx++;
    }

  }
  return true;
}

//

QVariant QgsExpressionNodeLiteral::evalNode( QgsExpression *parent, const QgsExpressionContext *context )
{
  Q_UNUSED( context )
  Q_UNUSED( parent )
  return mValue;
}

QgsExpressionNode::NodeType QgsExpressionNodeLiteral::nodeType() const
{
  return ntLiteral;
}

bool QgsExpressionNodeLiteral::prepareNode( QgsExpression *parent, const QgsExpressionContext *context )
{
  Q_UNUSED( parent )
  Q_UNUSED( context )
  return true;
}


QString QgsExpressionNodeLiteral::valueAsString() const
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
      return QgsExpression::quotedString( mValue.toString() );
    case QVariant::Bool:
      return mValue.toBool() ? QStringLiteral( "TRUE" ) : QStringLiteral( "FALSE" );
    default:
      return tr( "[unsupported type: %1; value: %2]" ).arg( mValue.typeName(), mValue.toString() );
  }
}

QString QgsExpressionNodeLiteral::dump() const
{
  return valueAsString();
}

QSet<QString> QgsExpressionNodeLiteral::referencedColumns() const
{
  return QSet<QString>();
}

QSet<QString> QgsExpressionNodeLiteral::referencedVariables() const
{
  return QSet<QString>();
}

QSet<QString> QgsExpressionNodeLiteral::referencedFunctions() const
{
  return QSet<QString>();
}

QList<const QgsExpressionNode *> QgsExpressionNodeLiteral::nodes() const
{
  QList<const QgsExpressionNode *> lst;
  lst << this;
  return lst;
}

bool QgsExpressionNodeLiteral::needsGeometry() const
{
  return false;
}

QgsExpressionNode *QgsExpressionNodeLiteral::clone() const
{
  QgsExpressionNodeLiteral *copy = new QgsExpressionNodeLiteral( mValue );
  cloneTo( copy );
  return copy;
}

bool QgsExpressionNodeLiteral::isStatic( QgsExpression *parent, const QgsExpressionContext *context ) const
{
  Q_UNUSED( context )
  Q_UNUSED( parent )
  return true;
}

//

QVariant QgsExpressionNodeColumnRef::evalNode( QgsExpression *parent, const QgsExpressionContext *context )
{
  Q_UNUSED( parent )
  int index = mIndex;

  if ( index < 0 )
  {
    // have not yet found field index - first check explicitly set fields collection
    if ( context && context->hasVariable( QgsExpressionContext::EXPR_FIELDS ) )
    {
      QgsFields fields = qvariant_cast<QgsFields>( context->variable( QgsExpressionContext::EXPR_FIELDS ) );
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

QgsExpressionNode::NodeType QgsExpressionNodeColumnRef::nodeType() const
{
  return ntColumnRef;
}

bool QgsExpressionNodeColumnRef::prepareNode( QgsExpression *parent, const QgsExpressionContext *context )
{
  if ( !context || !context->hasVariable( QgsExpressionContext::EXPR_FIELDS ) )
    return false;

  QgsFields fields = qvariant_cast<QgsFields>( context->variable( QgsExpressionContext::EXPR_FIELDS ) );

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

QString QgsExpressionNodeColumnRef::dump() const
{
  const thread_local QRegularExpression re( QStringLiteral( "^[A-Za-z_\x80-\xff][A-Za-z0-9_\x80-\xff]*$" ) );
  const QRegularExpressionMatch match = re.match( mName );
  return match.hasMatch() ? mName : QgsExpression::quotedColumnRef( mName );
}

QSet<QString> QgsExpressionNodeColumnRef::referencedColumns() const
{
  return QSet<QString>() << mName;
}

QSet<QString> QgsExpressionNodeColumnRef::referencedVariables() const
{
  return QSet<QString>();
}

QSet<QString> QgsExpressionNodeColumnRef::referencedFunctions() const
{
  return QSet<QString>();
}

QList<const QgsExpressionNode *> QgsExpressionNodeColumnRef::nodes() const
{
  QList<const QgsExpressionNode *> result;
  result << this;
  return result;
}

bool QgsExpressionNodeColumnRef::needsGeometry() const
{
  return false;
}

QgsExpressionNode *QgsExpressionNodeColumnRef::clone() const
{
  QgsExpressionNodeColumnRef *copy = new QgsExpressionNodeColumnRef( mName );
  cloneTo( copy );
  return copy;
}

bool QgsExpressionNodeColumnRef::isStatic( QgsExpression *parent, const QgsExpressionContext *context ) const
{
  Q_UNUSED( context )
  Q_UNUSED( parent )
  return false;
}

//

QgsExpressionNodeCondition::QgsExpressionNodeCondition( QgsExpressionNodeCondition::WhenThenList *conditions, QgsExpressionNode *elseExp )
  : mConditions( *conditions )
  , mElseExp( elseExp )
{
  delete conditions;
}

QgsExpressionNodeCondition::~QgsExpressionNodeCondition()
{
  delete mElseExp;
  qDeleteAll( mConditions );
}

QgsExpressionNode::NodeType QgsExpressionNodeCondition::nodeType() const
{
  return ntCondition;
}

QVariant QgsExpressionNodeCondition::evalNode( QgsExpression *parent, const QgsExpressionContext *context )
{
  for ( WhenThen *cond : std::as_const( mConditions ) )
  {
    QVariant vWhen = cond->mWhenExp->eval( parent, context );
    QgsExpressionUtils::TVL tvl = QgsExpressionUtils::getTVLValue( vWhen, parent );
    ENSURE_NO_EVAL_ERROR
    if ( tvl == QgsExpressionUtils::True )
    {
      QVariant vRes = cond->mThenExp->eval( parent, context );
      ENSURE_NO_EVAL_ERROR
      return vRes;
    }
  }

  if ( mElseExp )
  {
    QVariant vElse = mElseExp->eval( parent, context );
    ENSURE_NO_EVAL_ERROR
    return vElse;
  }

  // return NULL if no condition is matching
  return QVariant();
}

bool QgsExpressionNodeCondition::prepareNode( QgsExpression *parent, const QgsExpressionContext *context )
{
  bool foundAnyNonStaticConditions = false;
  for ( WhenThen *cond : std::as_const( mConditions ) )
  {
    const bool res = cond->mWhenExp->prepare( parent, context )
                     && cond->mThenExp->prepare( parent, context );
    if ( !res )
      return false;

    foundAnyNonStaticConditions |= !cond->mWhenExp->hasCachedStaticValue();
    if ( !foundAnyNonStaticConditions && QgsExpressionUtils::getTVLValue( cond->mWhenExp->cachedStaticValue(), parent ) == QgsExpressionUtils::True )
    {
      // ok, we now that we'll ALWAYS be picking the same condition, as the "WHEN" clause for this condition (and all previous conditions) is a static
      // value, and the static value for this WHEN clause is True.
      if ( cond->mThenExp->hasCachedStaticValue() )
      {
        // then "THEN" clause ALSO has a static value, so we can replace the whole node with a static value
        mCachedStaticValue = cond->mThenExp->cachedStaticValue();
        mHasCachedValue = true;
        return true;
      }
      else
      {
        // we know at least that we'll ALWAYS be picking the same condition, so even though the THEN node is non-static we can effectively replace
        // this whole QgsExpressionNodeCondition node with just the THEN node for this condition.
        mCompiledSimplifiedNode.reset( cond->mThenExp->effectiveNode()->clone() );
        return true;
      }
    }
  }

  if ( mElseExp )
  {
    const bool res = mElseExp->prepare( parent, context );
    if ( !res )
      return false;

    if ( !foundAnyNonStaticConditions )
    {
      // all condition nodes are static conditions and not TRUE, so we know we'll ALWAYS be picking the ELSE node
      if ( mElseExp->hasCachedStaticValue() )
      {
        mCachedStaticValue = mElseExp->cachedStaticValue();
        mHasCachedValue = true;
        return true;
      }
      else
      {
        // so even though the ELSE node is non-static we can effectively replace
        // this whole QgsExpressionNodeCondition node with just the ELSE node for this condition.
        mCompiledSimplifiedNode.reset( mElseExp->effectiveNode()->clone() );
        return true;
      }
    }
  }

  return true;
}

QString QgsExpressionNodeCondition::dump() const
{
  QString msg( QStringLiteral( "CASE" ) );
  for ( WhenThen *cond : mConditions )
  {
    msg += QStringLiteral( " WHEN %1 THEN %2" ).arg( cond->mWhenExp->dump(), cond->mThenExp->dump() );
  }
  if ( mElseExp )
    msg += QStringLiteral( " ELSE %1" ).arg( mElseExp->dump() );
  msg += QLatin1String( " END" );
  return msg;
}

QSet<QString> QgsExpressionNodeCondition::referencedColumns() const
{
  if ( hasCachedStaticValue() )
    return QSet< QString >();

  QSet<QString> lst;
  for ( WhenThen *cond : mConditions )
  {
    lst += cond->mWhenExp->referencedColumns() + cond->mThenExp->referencedColumns();
  }

  if ( mElseExp )
    lst += mElseExp->referencedColumns();

  return lst;
}

QSet<QString> QgsExpressionNodeCondition::referencedVariables() const
{
  QSet<QString> lst;
  for ( WhenThen *cond : mConditions )
  {
    lst += cond->mWhenExp->referencedVariables() + cond->mThenExp->referencedVariables();
  }

  if ( mElseExp )
    lst += mElseExp->referencedVariables();

  return lst;
}

QSet<QString> QgsExpressionNodeCondition::referencedFunctions() const
{
  QSet<QString> lst;
  for ( WhenThen *cond : mConditions )
  {
    lst += cond->mWhenExp->referencedFunctions() + cond->mThenExp->referencedFunctions();
  }

  if ( mElseExp )
    lst += mElseExp->referencedFunctions();

  return lst;
}

QList<const QgsExpressionNode *> QgsExpressionNodeCondition::nodes() const
{
  QList<const QgsExpressionNode *> lst;
  lst << this;
  for ( WhenThen *cond : mConditions )
  {
    lst += cond->mWhenExp->nodes() + cond->mThenExp->nodes();
  }

  if ( mElseExp )
    lst += mElseExp->nodes();

  return lst;
}

bool QgsExpressionNodeCondition::needsGeometry() const
{
  for ( WhenThen *cond : mConditions )
  {
    if ( cond->mWhenExp->needsGeometry() ||
         cond->mThenExp->needsGeometry() )
      return true;
  }

  return mElseExp && mElseExp->needsGeometry();
}

QgsExpressionNode *QgsExpressionNodeCondition::clone() const
{
  WhenThenList conditions;
  conditions.reserve( mConditions.size() );
  for ( WhenThen *wt : mConditions )
    conditions.append( wt->clone() );

  QgsExpressionNodeCondition *copy = new QgsExpressionNodeCondition( conditions, mElseExp ? mElseExp->clone() : nullptr );
  cloneTo( copy );
  return copy;
}

bool QgsExpressionNodeCondition::isStatic( QgsExpression *parent, const QgsExpressionContext *context ) const
{
  for ( WhenThen *wt : mConditions )
  {
    if ( !wt->mWhenExp->isStatic( parent, context ) || !wt->mThenExp->isStatic( parent, context ) )
      return false;
  }

  if ( mElseExp )
    return mElseExp->isStatic( parent, context );

  return true;
}

QSet<QString> QgsExpressionNodeInOperator::referencedColumns() const
{
  if ( hasCachedStaticValue() )
    return QSet< QString >();

  QSet<QString> lst( mNode->referencedColumns() );
  const QList< QgsExpressionNode * > nodeList = mList->list();
  for ( const QgsExpressionNode *n : nodeList )
    lst.unite( n->referencedColumns() );
  return lst;
}

QSet<QString> QgsExpressionNodeInOperator::referencedVariables() const
{
  QSet<QString> lst( mNode->referencedVariables() );
  const QList< QgsExpressionNode * > nodeList = mList->list();
  for ( const QgsExpressionNode *n : nodeList )
    lst.unite( n->referencedVariables() );
  return lst;
}

QSet<QString> QgsExpressionNodeInOperator::referencedFunctions() const
{
  QSet<QString> lst( mNode->referencedFunctions() );
  const QList< QgsExpressionNode * > nodeList = mList->list();
  for ( const QgsExpressionNode *n : nodeList )
    lst.unite( n->referencedFunctions() );
  return lst;
}

QList<const QgsExpressionNode *> QgsExpressionNodeInOperator::nodes() const
{
  QList<const QgsExpressionNode *> lst;
  lst << this;
  const QList< QgsExpressionNode * > nodeList = mList->list();
  for ( const QgsExpressionNode *n : nodeList )
    lst += n->nodes();
  return lst;
}

QgsExpressionNodeCondition::WhenThen::WhenThen( QgsExpressionNode *whenExp, QgsExpressionNode *thenExp )
  : mWhenExp( whenExp )
  , mThenExp( thenExp )
{
}

QgsExpressionNodeCondition::WhenThen::~WhenThen()
{
  delete mWhenExp;
  delete mThenExp;
}

QgsExpressionNodeCondition::WhenThen *QgsExpressionNodeCondition::WhenThen::clone() const
{
  return new WhenThen( mWhenExp->clone(), mThenExp->clone() );
}

QString QgsExpressionNodeBinaryOperator::text() const
{
  return BINARY_OPERATOR_TEXT[mOp];
}

//

QVariant QgsExpressionNodeIndexOperator::evalNode( QgsExpression *parent, const QgsExpressionContext *context )
{
  const QVariant container = mContainer->eval( parent, context );
  ENSURE_NO_EVAL_ERROR
  const QVariant index = mIndex->eval( parent, context );
  ENSURE_NO_EVAL_ERROR

  switch ( container.type() )
  {
    case QVariant::Map:
      return QgsExpressionUtils::getMapValue( container, parent ).value( index.toString() );

    case QVariant::List:
    case QVariant::StringList:
    {
      const QVariantList list = QgsExpressionUtils::getListValue( container, parent );
      qlonglong pos = QgsExpressionUtils::getIntValue( index, parent );
      if ( pos >= list.length() || pos < -list.length() )
      {
        return QVariant();
      }
      if ( pos < 0 )
      {
        // negative indices are from back of list
        pos += list.length();
      }

      return list.at( pos );
    }

    default:
      if ( !container.isNull() )
        parent->setEvalErrorString( tr( "[] can only be used with map or array values, not %1" ).arg( QMetaType::typeName( container.type() ) ) );
      return QVariant();
  }
}

QgsExpressionNode::NodeType QgsExpressionNodeIndexOperator::nodeType() const
{
  return ntIndexOperator;
}

bool QgsExpressionNodeIndexOperator::prepareNode( QgsExpression *parent, const QgsExpressionContext *context )
{
  bool resC = mContainer->prepare( parent, context );
  bool resV = mIndex->prepare( parent, context );
  return resC && resV;
}

QString QgsExpressionNodeIndexOperator::dump() const
{
  return QStringLiteral( "%1[%2]" ).arg( mContainer->dump(), mIndex->dump() );
}

QSet<QString> QgsExpressionNodeIndexOperator::referencedColumns() const
{
  if ( hasCachedStaticValue() )
    return QSet< QString >();

  return mContainer->referencedColumns() + mIndex->referencedColumns();
}

QSet<QString> QgsExpressionNodeIndexOperator::referencedVariables() const
{
  return mContainer->referencedVariables() + mIndex->referencedVariables();
}

QSet<QString> QgsExpressionNodeIndexOperator::referencedFunctions() const
{
  return mContainer->referencedFunctions() + mIndex->referencedFunctions();
}

QList<const QgsExpressionNode *> QgsExpressionNodeIndexOperator::nodes() const
{
  QList<const QgsExpressionNode *> lst;
  lst << this;
  lst += mContainer->nodes() + mIndex->nodes();
  return lst;
}

bool QgsExpressionNodeIndexOperator::needsGeometry() const
{
  return mContainer->needsGeometry() || mIndex->needsGeometry();
}

QgsExpressionNode *QgsExpressionNodeIndexOperator::clone() const
{
  QgsExpressionNodeIndexOperator *copy = new QgsExpressionNodeIndexOperator( mContainer->clone(), mIndex->clone() );
  cloneTo( copy );
  return copy;
}

bool QgsExpressionNodeIndexOperator::isStatic( QgsExpression *parent, const QgsExpressionContext *context ) const
{
  return mContainer->isStatic( parent, context ) && mIndex->isStatic( parent, context );
}
