/***************************************************************************
                         qgspointcouldexpression.cpp
                         ---------------------------
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

#include "qgspointcloudexpression.h"
#include "qgspointcloudexpressionnodeimpl.h"
#include "qgsfeaturerequest.h"
#include "qgscolorrampimpl.h"
#include "qgslogger.h"
#include "qgsgeometry.h"
#include "qgsproject.h"
#include "qgspointcloudexpressionutils.h"
#include "qgspointcloudexpression_p.h"

#include <QRegularExpression>

// from parser
extern QgsPointcloudExpressionNode *parseExpression( const QString &str, QString &parserErrorMsg, QList<QgsPointcloudExpression::ParserError> &parserErrors );

bool QgsPointcloudExpression::checkExpression( const QString &text, QString &errorMessage )
{
  QgsPointcloudExpression exp( text );
  exp.prepare();
  errorMessage = exp.parserErrorString();
  return !exp.hasParserError();
}

void QgsPointcloudExpression::setExpression( const QString &expression )
{
  detach();
  d->mRootNode = ::parseExpression( expression, d->mParserErrorString, d->mParserErrors );
  d->mEvalErrorString = QString();
  d->mExp = expression;
  d->mIsPrepared = false;
}

QString QgsPointcloudExpression::expression() const
{
  if ( !d->mExp.isNull() )
    return d->mExp;
  else
    return dump();
}

QString QgsPointcloudExpression::quotedAttributeRef( QString name )
{
  return QStringLiteral( "\"%1\"" ).arg( name.replace( '\"', QLatin1String( "\"\"" ) ) );
}

QString QgsPointcloudExpression::quotedString( QString text )
{
  text.replace( '\'', QLatin1String( "''" ) );
  text.replace( '\\', QLatin1String( "\\\\" ) );
  text.replace( '\n', QLatin1String( "\\n" ) );
  text.replace( '\t', QLatin1String( "\\t" ) );
  return QStringLiteral( "'%1'" ).arg( text );
}

QString QgsPointcloudExpression::quotedValue( const QVariant &value )
{
  return quotedValue( value, value.type() );
}

QString QgsPointcloudExpression::quotedValue( const QVariant &value, QVariant::Type type )
{
  if ( value.isNull() )
    return QStringLiteral( "NULL" );

  switch ( type )
  {
    case QVariant::Int:
    case QVariant::LongLong:
    case QVariant::Double:
      return value.toString();

    case QVariant::Bool:
      return value.toBool() ? QStringLiteral( "TRUE" ) : QStringLiteral( "FALSE" );

    case QVariant::List:
    {
      QStringList quotedValues;
      const QVariantList values = value.toList();
      quotedValues.reserve( values.count() );
      for ( const QVariant &v : values )
      {
        quotedValues += quotedValue( v );
      }
      return QStringLiteral( "array( %1 )" ).arg( quotedValues.join( QLatin1String( ", " ) ) );
    }

    default:
    case QVariant::String:
      return quotedString( value.toString() );
  }

}

QgsPointcloudExpression::QgsPointcloudExpression( const QString &expr )
  : d( new QgsPointcloudExpressionPrivate )
{
  d->mRootNode = ::parseExpression( expr, d->mParserErrorString, d->mParserErrors );
  d->mExp = expr;
  Q_ASSERT( !d->mParserErrorString.isNull() || d->mRootNode );
}

QgsPointcloudExpression::QgsPointcloudExpression( const QgsPointcloudExpression &other )
  : d( other.d )
{
  d->ref.ref();
}

QgsPointcloudExpression &QgsPointcloudExpression::operator=( const QgsPointcloudExpression &other )
{
  if ( this != &other )
  {
    if ( !d->ref.deref() )
    {
      delete d;
    }

    d = other.d;
    d->ref.ref();
  }
  return *this;
}

QgsPointcloudExpression::operator QString() const
{
  return d->mExp;
}

QgsPointcloudExpression::QgsPointcloudExpression()
  : d( new QgsPointcloudExpressionPrivate )
{
}

QgsPointcloudExpression::~QgsPointcloudExpression()
{
  Q_ASSERT( d );
  if ( !d->ref.deref() )
    delete d;
}

bool QgsPointcloudExpression::operator==( const QgsPointcloudExpression &other ) const
{
  return ( d == other.d || d->mExp == other.d->mExp );
}

bool QgsPointcloudExpression::isValid() const
{
  return d->mRootNode;
}

bool QgsPointcloudExpression::hasParserError() const
{
  return d->mParserErrors.count() > 0;
}

QString QgsPointcloudExpression::parserErrorString() const
{
  return d->mParserErrorString;
}

QList<QgsPointcloudExpression::ParserError> QgsPointcloudExpression::parserErrors() const
{
  return d->mParserErrors;
}

QSet<QString> QgsPointcloudExpression::referencedAttributes() const
{
  if ( !d->mRootNode )
    return QSet<QString>();

  return d->mRootNode->referencedAttributes();
}

QSet<int> QgsPointcloudExpression::referencedAttributeIndexes( const QgsFields &fields ) const
{
  if ( !d->mRootNode )
    return QSet<int>();

  const QSet<QString> referencedFields = d->mRootNode->referencedAttributes();
  QSet<int> referencedIndexes;

  for ( const QString &fieldName : referencedFields )
  {
    if ( fieldName == QgsFeatureRequest::ALL_ATTRIBUTES )
    {
      referencedIndexes = qgis::listToSet( fields.allAttributesList() );
      break;
    }
    const int idx = fields.lookupField( fieldName );
    if ( idx >= 0 )
    {
      referencedIndexes << idx;
    }
  }

  return referencedIndexes;
}

void QgsPointcloudExpression::detach()
{
  Q_ASSERT( d );

  if ( d->ref > 1 )
  {
    ( void )d->ref.deref();

    d = new QgsPointcloudExpressionPrivate( *d );
  }
}

bool QgsPointcloudExpression::prepare()
{
  detach();
  d->mEvalErrorString = QString();
  if ( !d->mRootNode )
  {
    //re-parse expression. Creation of QgsPointcloudExpressionContexts may have added extra
    //known functions since this expression was created, so we have another try
    //at re-parsing it now that the context must have been created
    d->mRootNode = ::parseExpression( d->mExp, d->mParserErrorString, d->mParserErrors );
  }

  if ( !d->mRootNode )
  {
    d->mEvalErrorString = tr( "No root node! Parsing failed?" );
    return false;
  }

  d->mIsPrepared = true;
  return d->mRootNode->prepare( this );
}

QVariant QgsPointcloudExpression::evaluate()
{
  d->mEvalErrorString = QString();
  if ( !d->mRootNode )
  {
    d->mEvalErrorString = tr( "No root node! Parsing failed?" );
    return QVariant();
  }

  return d->mRootNode->eval( this );
}

// QVariant QgsPointcloudExpression::evaluate()
// {
//   d->mEvalErrorString = QString();
//   if ( !d->mRootNode )
//   {
//     d->mEvalErrorString = tr( "No root node! Parsing failed?" );
//     return QVariant();
//   }
//
//   if ( ! d->mIsPrepared )
//   {
//     prepare();
//   }
//   return d->mRootNode->eval( this );
// }

bool QgsPointcloudExpression::hasEvalError() const
{
  return !d->mEvalErrorString.isNull();
}

QString QgsPointcloudExpression::evalErrorString() const
{
  return d->mEvalErrorString;
}

void QgsPointcloudExpression::setEvalErrorString( const QString &str )
{
  d->mEvalErrorString = str;
}

QString QgsPointcloudExpression::dump() const
{
  if ( !d->mRootNode )
    return QString();

  return d->mRootNode->dump();
}

QString QgsPointcloudExpression::createFieldEqualityExpression( const QString &fieldName, const QVariant &value, QVariant::Type fieldType )
{
  QString expr;

  if ( value.isNull() )
    expr = QStringLiteral( "%1 IS NULL" ).arg( quotedAttributeRef( fieldName ) );
  else if ( fieldType == QVariant::Type::Invalid )
    expr = QStringLiteral( "%1 = %2" ).arg( quotedAttributeRef( fieldName ), quotedValue( value ) );
  else
    expr = QStringLiteral( "%1 = %2" ).arg( quotedAttributeRef( fieldName ), quotedValue( value, fieldType ) );

  return expr;
}

bool QgsPointcloudExpression::isFieldEqualityExpression( const QString &expression, QString &field, QVariant &value )
{
  QgsPointcloudExpression e( expression );

  if ( !e.rootNode() )
    return false;

  if ( const QgsPointcloudExpressionNodeBinaryOperator *binOp = dynamic_cast<const QgsPointcloudExpressionNodeBinaryOperator *>( e.rootNode() ) )
  {
    if ( binOp->op() == QgsPointcloudExpressionNodeBinaryOperator::boEQ )
    {
      const QgsPointcloudExpressionNodeAttributeRef *attributeRef = dynamic_cast<const QgsPointcloudExpressionNodeAttributeRef *>( binOp->opLeft() );
      const QgsPointcloudExpressionNodeLiteral *literal = dynamic_cast<const QgsPointcloudExpressionNodeLiteral *>( binOp->opRight() );
      if ( attributeRef && literal )
      {
        field = attributeRef->name();
        value = literal->value();
        return true;
      }
    }
  }
  return false;
}

bool QgsPointcloudExpression::attemptReduceToInClause( const QStringList &expressions, QString &result )
{
  if ( expressions.empty() )
    return false;

  QString inField;
  bool first = true;
  QStringList values;
  for ( const QString &expression : expressions )
  {
    QString field;
    QVariant value;
    if ( QgsPointcloudExpression::isFieldEqualityExpression( expression, field, value ) )
    {
      if ( first )
      {
        inField = field;
        first = false;
      }
      else if ( field != inField )
      {
        return false;
      }
      values << QgsPointcloudExpression::quotedValue( value );
    }
    else
    {
      // we also allow reducing similar 'field IN (...)' expressions!
      QgsPointcloudExpression e( expression );

      if ( !e.rootNode() )
        return false;

      if ( const QgsPointcloudExpressionNodeInOperator *inOp = dynamic_cast<const QgsPointcloudExpressionNodeInOperator *>( e.rootNode() ) )
      {
        if ( inOp->isNotIn() )
          return false;

        const QgsPointcloudExpressionNodeAttributeRef *attributeRef = dynamic_cast<const QgsPointcloudExpressionNodeAttributeRef *>( inOp->node() );
        if ( !attributeRef )
          return false;

        if ( first )
        {
          inField = attributeRef->name();
          first = false;
        }
        else if ( attributeRef->name() != inField )
        {
          return false;
        }

        if ( QgsPointcloudExpressionNode::NodeList *nodeList = inOp->list() )
        {
          const QList<QgsPointcloudExpressionNode *> nodes = nodeList->list();
          for ( const QgsPointcloudExpressionNode *node : nodes )
          {
            const QgsPointcloudExpressionNodeLiteral *literal = dynamic_cast<const QgsPointcloudExpressionNodeLiteral *>( node );
            if ( !literal )
              return false;

            values << QgsPointcloudExpression::quotedValue( literal->value() );
          }
        }
      }
      // Collect ORs
      else if ( const QgsPointcloudExpressionNodeBinaryOperator *orOp = dynamic_cast<const QgsPointcloudExpressionNodeBinaryOperator *>( e.rootNode() ) )
      {

        // OR Collector function: returns a possibly empty list of the left and right operands of an OR expression
        std::function<QStringList( QgsPointcloudExpressionNode *, QgsPointcloudExpressionNode * )> collectOrs = [ &collectOrs ]( QgsPointcloudExpressionNode * opLeft,  QgsPointcloudExpressionNode * opRight ) -> QStringList
        {
          QStringList orParts;
          if ( const QgsPointcloudExpressionNodeBinaryOperator *leftOrOp = dynamic_cast<const QgsPointcloudExpressionNodeBinaryOperator *>( opLeft ) )
          {
            if ( leftOrOp->op( ) == QgsPointcloudExpressionNodeBinaryOperator::BinaryOperator::boOr )
            {
              orParts.append( collectOrs( leftOrOp->opLeft(), leftOrOp->opRight() ) );
            }
            else
            {
              orParts.append( leftOrOp->dump() );
            }
          }
          else if ( const QgsPointcloudExpressionNodeInOperator *leftInOp = dynamic_cast<const QgsPointcloudExpressionNodeInOperator *>( opLeft ) )
          {
            orParts.append( leftInOp->dump() );
          }
          else
          {
            return {};
          }

          if ( const QgsPointcloudExpressionNodeBinaryOperator *rightOrOp = dynamic_cast<const QgsPointcloudExpressionNodeBinaryOperator *>( opRight ) )
          {
            if ( rightOrOp->op( ) == QgsPointcloudExpressionNodeBinaryOperator::BinaryOperator::boOr )
            {
              orParts.append( collectOrs( rightOrOp->opLeft(), rightOrOp->opRight() ) );
            }
            else
            {
              orParts.append( rightOrOp->dump() );
            }
          }
          else if ( const QgsPointcloudExpressionNodeInOperator *rightInOp = dynamic_cast<const QgsPointcloudExpressionNodeInOperator *>( opRight ) )
          {
            orParts.append( rightInOp->dump() );
          }
          else
          {
            return {};
          }

          return orParts;
        };

        if ( orOp->op( ) == QgsPointcloudExpressionNodeBinaryOperator::BinaryOperator::boOr )
        {
          // Try to collect all OR conditions
          const QStringList orParts = collectOrs( orOp->opLeft(), orOp->opRight() );
          if ( orParts.isEmpty() )
          {
            return false;
          }
          else
          {
            QString orPartsResult;
            if ( attemptReduceToInClause( orParts, orPartsResult ) )
            {
              // Need to check if the IN field is correct,
              QgsPointcloudExpression inExp { orPartsResult };
              if ( ! inExp.rootNode() )
              {
                return false;
              }

              if ( const QgsPointcloudExpressionNodeInOperator *inOpInner = dynamic_cast<const QgsPointcloudExpressionNodeInOperator *>( inExp.rootNode() ) )
              {
                if ( inOpInner->node()->nodeType() != QgsPointcloudExpressionNode::NodeType::ntAttributeRef || inOpInner->node()->referencedAttributes().size() < 1 )
                {
                  return false;
                }

                const QString innerInfield { inOpInner->node()->referencedAttributes().values().first() };

                if ( first )
                {
                  inField = innerInfield;
                  first = false;
                }

                if ( innerInfield != inField )
                {
                  return false;
                }
                else
                {
                  const auto constInnerValuesList { inOpInner->list()->list() };
                  for ( const auto &innerInValueNode : std::as_const( constInnerValuesList ) )
                  {
                    values.append( innerInValueNode->dump() );
                  }
                }

              }
              else
              {
                return false;
              }
            }
            else
            {
              return false;
            }
          }
        }
        else
        {
          return false;
        }
      }
      else
      {
        return false;
      }
    }
  }
  result = QStringLiteral( "%1 IN (%2)" ).arg( quotedAttributeRef( inField ), values.join( ',' ) );
  return true;
}

const QgsPointcloudExpressionNode *QgsPointcloudExpression::rootNode() const
{
  return d->mRootNode;
}

QList<const QgsPointcloudExpressionNode *> QgsPointcloudExpression::nodes() const
{
  if ( !d->mRootNode )
    return QList<const QgsPointcloudExpressionNode *>();

  return d->mRootNode->nodes();
}



