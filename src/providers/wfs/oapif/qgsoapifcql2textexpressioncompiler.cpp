/***************************************************************************
    qgsoapifcql2textexpressioncompiler.cpp
    --------------------------------------
    begin                : April 2023
    copyright            : (C) 2023 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsoapifcql2textexpressioncompiler.h"

#include "qgsexpressionnodeimpl.h"
#include "qgsexpressionfunction.h"
#include "qgsexpression.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsvariantutils.h"

QgsOapifCql2TextExpressionCompiler::QgsOapifCql2TextExpressionCompiler(
  const QMap<QString, QgsOapifQueryablesRequest::Queryable> &queryables,
  bool supportsLikeBetweenIn,
  bool supportsCaseI,
  bool supportsBasicSpatialOperators,
  bool invertAxisOrientation
)
  : mQueryables( queryables ), mSupportsLikeBetweenIn( supportsLikeBetweenIn ), mSupportsCaseI( supportsCaseI ), mSupportsBasicSpatialOperators( supportsBasicSpatialOperators ), mInvertAxisOrientation( invertAxisOrientation )
{
}

QgsOapifCql2TextExpressionCompiler::Result QgsOapifCql2TextExpressionCompiler::compile( const QgsExpression *exp )
{
  if ( exp->rootNode() )
    return compileNode( exp->rootNode(), mResult );
  else
    return Fail;
}

QString QgsOapifCql2TextExpressionCompiler::literalValue( const QVariant &value ) const
{
  if ( QgsVariantUtils::isNull( value ) )
    return QStringLiteral( "NULL" );

  switch ( value.userType() )
  {
    case QMetaType::Type::Int:
    case QMetaType::Type::LongLong:
    case QMetaType::Type::Double:
      return value.toString();

    case QMetaType::Type::Bool:
      return value.toBool() ? QStringLiteral( "TRUE" ) : QStringLiteral( "FALSE" );

    case QMetaType::Type::QDateTime:
      return value.toDateTime().toOffsetFromUtc( 0 ).toString( Qt::ISODateWithMs ).prepend( "TIMESTAMP('" ).append( "')" );

    case QMetaType::Type::QDate:
      return value.toDate().toString( Qt::ISODate ).prepend( "DATE('" ).append( "')" );

    case QMetaType::Type::QString:
    default:
      QString v = value.toString();
      v.replace( '\'', QLatin1String( "''" ) );
      return v.replace( '\\', QLatin1String( "\\\\" ) ).prepend( '\'' ).append( '\'' );
  }
}

QString QgsOapifCql2TextExpressionCompiler::quotedIdentifier( const QString &identifier ) const
{
  bool isAlphaNumOnly = true;
  for ( QChar ch : identifier )
  {
    if ( !ch.isDigit() && !ch.isLetter() )
    {
      isAlphaNumOnly = false;
      break;
    }
  }
  if ( isAlphaNumOnly )
  {
    return identifier;
  }
  QString quoted = identifier;
  quoted.replace( '"', QLatin1String( "\"\"" ) );
  quoted = quoted.prepend( '\"' ).append( '\"' );
  return quoted;
}

static bool isGeometryColumn( const QgsExpressionNode *node )
{
  if ( node->nodeType() != QgsExpressionNode::ntFunction )
    return false;

  const QgsExpressionNodeFunction *fn = static_cast<const QgsExpressionNodeFunction *>( node );
  QgsExpressionFunction *fd = QgsExpression::Functions()[fn->fnIndex()];
  return fd->name() == QLatin1String( "$geometry" );
}

static QgsGeometry geometryFromConstExpr( const QgsExpressionNode *node )
{
  // Right now we support only geomFromWKT(' ..... ')
  // Ideally we should support any constant sub-expression (not dependent on feature's geometry or attributes)

  if ( node->nodeType() == QgsExpressionNode::ntFunction )
  {
    const QgsExpressionNodeFunction *fnNode = static_cast<const QgsExpressionNodeFunction *>( node );
    QgsExpressionFunction *fnDef = QgsExpression::Functions()[fnNode->fnIndex()];
    if ( fnDef->name() == QLatin1String( "geom_from_wkt" ) )
    {
      const QList<QgsExpressionNode *> &args = fnNode->args()->list();
      if ( args[0]->nodeType() == QgsExpressionNode::ntLiteral )
      {
        const QString wkt = static_cast<const QgsExpressionNodeLiteral *>( args[0] )->value().toString();
        return QgsGeometry::fromWkt( wkt );
      }
    }
  }
  return QgsGeometry();
}

QgsOapifCql2TextExpressionCompiler::Result QgsOapifCql2TextExpressionCompiler::compileNodeFunction( const QgsExpressionNodeFunction *node, QString &result )
{
  QgsExpressionFunction *fd = QgsExpression::Functions()[node->fnIndex()];

  if ( fd->name() == QLatin1String( "intersects_bbox" ) && mSupportsBasicSpatialOperators )
  {
    QList<QgsExpressionNode *> argNodes = node->args()->list();
    Q_ASSERT( argNodes.count() == 2 ); // binary spatial ops must have two args

    const QgsGeometry geom = geometryFromConstExpr( argNodes[1] );
    if ( !geom.isNull() && isGeometryColumn( argNodes[0] ) )
    {
      QString geometryColumn;
      for ( const auto &kv : mQueryables.toStdMap() )
      {
        if ( kv.second.mIsGeometry )
        {
          geometryColumn = kv.first;
          break;
        }
      }
      if ( geometryColumn.isEmpty() )
      {
        return Fail;
      }

      QgsRectangle rect = geom.boundingBox();
      if ( mInvertAxisOrientation )
        rect.invert();
      QString coordString;
      coordString += qgsDoubleToString( rect.xMinimum() );
      coordString += ',';
      coordString += qgsDoubleToString( rect.yMinimum() );
      coordString += ',';
      coordString += qgsDoubleToString( rect.xMaximum() );
      coordString += ',';
      coordString += qgsDoubleToString( rect.yMaximum() );
      result = QStringLiteral( "S_INTERSECTS(" );
      result += quotedIdentifier( geometryColumn );
      result += QLatin1String( ",BBOX(" );
      result += coordString;
      result += QLatin1String( "))" );
      mGeometryLiteralUsed = true;
      return Complete;
    }
  }

  if ( fd->name() == QLatin1String( "intersects" ) && mSupportsBasicSpatialOperators )
  {
    QList<QgsExpressionNode *> argNodes = node->args()->list();
    Q_ASSERT( argNodes.count() == 2 ); // binary spatial ops must have two args

    const QgsGeometry geom = geometryFromConstExpr( argNodes[1] );
    if ( !geom.isNull() && geom.wkbType() == Qgis::WkbType::Point && isGeometryColumn( argNodes[0] ) )
    {
      QString geometryColumn;
      for ( const auto &kv : mQueryables.toStdMap() )
      {
        if ( kv.second.mIsGeometry )
        {
          geometryColumn = kv.first;
          break;
        }
      }
      if ( geometryColumn.isEmpty() )
      {
        return Fail;
      }

      const QgsPoint *point = static_cast<const QgsPoint *>( geom.constGet() );
      QString coordString;
      coordString += qgsDoubleToString( mInvertAxisOrientation ? point->y() : point->x() );
      coordString += ' ';
      coordString += qgsDoubleToString( mInvertAxisOrientation ? point->x() : point->y() );
      result = QStringLiteral( "S_INTERSECTS(" );
      result += quotedIdentifier( geometryColumn );
      result += QLatin1String( ",POINT(" );
      result += coordString;
      result += QLatin1String( "))" );
      mGeometryLiteralUsed = true;
      return Complete;
    }
  }

  if ( fd->name() == QLatin1String( "make_datetime" ) )
  {
    QList<QgsExpressionNode *> argNodes = node->args()->list();
    if ( argNodes.count() == 6 )
    {
      std::vector<int> values;
      for ( int i = 0; i < 6; ++i )
      {
        if ( argNodes[i]->nodeType() != QgsExpressionNode::ntLiteral )
          return Fail;
        const QgsExpressionNodeLiteral *n = static_cast<const QgsExpressionNodeLiteral *>( argNodes[i] );
        if ( n->value().userType() != QMetaType::Type::Int )
          return Fail;
        values.push_back( n->value().toInt() );
      }
      result = QDateTime( QDate( values[0], values[1], values[2] ), QTime( values[3], values[4], values[5] ), Qt::UTC ).toString( Qt::ISODateWithMs ).prepend( "TIMESTAMP('" ).append( "')" );
      return Complete;
    }
  }

  if ( fd->name() == QLatin1String( "make_date" ) )
  {
    QList<QgsExpressionNode *> argNodes = node->args()->list();
    if ( argNodes.count() == 3 )
    {
      std::vector<int> values;
      for ( int i = 0; i < 3; ++i )
      {
        if ( argNodes[i]->nodeType() != QgsExpressionNode::ntLiteral )
          return Fail;
        const QgsExpressionNodeLiteral *n = static_cast<const QgsExpressionNodeLiteral *>( argNodes[i] );
        if ( n->value().userType() != QMetaType::Type::Int )
          return Fail;
        values.push_back( n->value().toInt() );
      }
      result = QDate( values[0], values[1], values[2] ).toString( Qt::ISODate ).prepend( "DATE('" ).append( "')" );
      return Complete;
    }
  }
  return Fail;
}

QgsOapifCql2TextExpressionCompiler::Result QgsOapifCql2TextExpressionCompiler::compileNode( const QgsExpressionNode *node, QString &result )
{
  if ( node->hasCachedStaticValue() )
  {
    result = literalValue( node->cachedStaticValue() );
    return Complete;
  }

  switch ( node->nodeType() )
  {
    case QgsExpressionNode::ntUnaryOperator:
    {
      const QgsExpressionNodeUnaryOperator *n = static_cast<const QgsExpressionNodeUnaryOperator *>( node );
      switch ( n->op() )
      {
        case QgsExpressionNodeUnaryOperator::uoNot:
        {
          QString right;
          if ( compileNode( n->operand(), right ) == Complete )
          {
            result = "(NOT (" + right + "))";
            return Complete;
          }

          return Fail;
        }

        case QgsExpressionNodeUnaryOperator::uoMinus:
        {
          QString right;
          if ( compileNode( n->operand(), right ) == Complete )
          {
            result = "( - (" + right + "))";
            return Complete;
          }

          return Fail;
        }
      }

      break;
    }

    case QgsExpressionNodeBinaryOperator::ntBinaryOperator:
    {
      const QgsExpressionNodeBinaryOperator *n = static_cast<const QgsExpressionNodeBinaryOperator *>( node );

      QString op;
      bool isCaseI = false;
      switch ( n->op() )
      {
        case QgsExpressionNodeBinaryOperator::boEQ:
          op = QStringLiteral( "=" );
          break;

        case QgsExpressionNodeBinaryOperator::boGE:
          op = QStringLiteral( ">=" );
          break;

        case QgsExpressionNodeBinaryOperator::boGT:
          op = QStringLiteral( ">" );
          break;

        case QgsExpressionNodeBinaryOperator::boLE:
          op = QStringLiteral( "<=" );
          break;

        case QgsExpressionNodeBinaryOperator::boLT:
          op = QStringLiteral( "<" );
          break;

        case QgsExpressionNodeBinaryOperator::boIs:
          op = QStringLiteral( "IS" );
          break;

        case QgsExpressionNodeBinaryOperator::boIsNot:
          op = QStringLiteral( "IS NOT" );
          break;

        case QgsExpressionNodeBinaryOperator::boLike:
          if ( !mSupportsLikeBetweenIn )
            return Fail;
          op = QStringLiteral( "LIKE" );
          break;

        case QgsExpressionNodeBinaryOperator::boILike:
          if ( !mSupportsLikeBetweenIn )
            return Fail;
          if ( !mSupportsCaseI )
            return Fail;
          isCaseI = true;
          op = QStringLiteral( "LIKE" );
          break;

        case QgsExpressionNodeBinaryOperator::boNotLike:
          if ( !mSupportsLikeBetweenIn )
            return Fail;
          op = QStringLiteral( "NOT LIKE" );
          break;

        case QgsExpressionNodeBinaryOperator::boNotILike:
          if ( !mSupportsLikeBetweenIn )
            return Fail;
          if ( !mSupportsCaseI )
            return Fail;
          isCaseI = true;
          op = QStringLiteral( "NOT LIKE" );
          break;

        case QgsExpressionNodeBinaryOperator::boOr:
          op = QStringLiteral( "OR" );
          break;

        case QgsExpressionNodeBinaryOperator::boAnd:
          op = QStringLiteral( "AND" );
          break;

        case QgsExpressionNodeBinaryOperator::boNE:
          op = QStringLiteral( "<>" );
          break;

        case QgsExpressionNodeBinaryOperator::boMul:
          op = QStringLiteral( "*" );
          break;

        case QgsExpressionNodeBinaryOperator::boPlus:
          op = QStringLiteral( "+" );
          break;

        case QgsExpressionNodeBinaryOperator::boMinus:
          op = QStringLiteral( "-" );
          break;

        case QgsExpressionNodeBinaryOperator::boDiv:
          op = QStringLiteral( "/" );
          break;

        case QgsExpressionNodeBinaryOperator::boMod:
          op = QStringLiteral( "%" );
          break;

        case QgsExpressionNodeBinaryOperator::boConcat:
          return Fail;

        case QgsExpressionNodeBinaryOperator::boIntDiv:
          op = QStringLiteral( "/" );
          break;

        case QgsExpressionNodeBinaryOperator::boPow:
          op = QStringLiteral( "^" );
          break;

        case QgsExpressionNodeBinaryOperator::boRegexp:
          return Fail;
      }

      QString left;
      const Result lr( compileNode( n->opLeft(), left ) );

      QString right;
      // Special case to handle "datetimefield OP 'YYYY-MM-DDTHH:MM:SS'"
      // or "datefield OP 'YYYY-MM-DD'"
      // that can be suggested by the query builder
      if ( n->opLeft()->nodeType() == QgsExpressionNode::ntColumnRef && n->opRight()->nodeType() == QgsExpressionNode::ntLiteral )
      {
        const QgsExpressionNodeColumnRef *nLeft = static_cast<const QgsExpressionNodeColumnRef *>( n->opLeft() );
        const QgsExpressionNodeLiteral *nRight = static_cast<const QgsExpressionNodeLiteral *>( n->opRight() );
        if ( nRight->value().userType() == QMetaType::Type::QString )
        {
          QString columnFormat;
          for ( const auto &kv : mQueryables.toStdMap() )
          {
            if ( kv.first.compare( nLeft->name(), Qt::CaseInsensitive ) == 0 )
            {
              columnFormat = kv.second.mFormat;
              break;
            }
          }
          if ( columnFormat == "date-time" )
          {
            QDateTime dt = QDateTime::fromString( nRight->value().toString(), Qt::ISODateWithMs );
            if ( dt.isValid() )
            {
              right = literalValue( QVariant( dt ) );
            }
          }
          else if ( columnFormat == "date" )
          {
            QDate date = QDate::fromString( nRight->value().toString(), Qt::ISODate );
            if ( date.isValid() )
            {
              right = literalValue( QVariant( date ) );
            }
          }
        }
      }

      const Result rr( right.isEmpty() ? compileNode( n->opRight(), right ) : Complete );

      if ( lr != Complete )
      {
        if ( n->op() == QgsExpressionNodeBinaryOperator::boAnd )
        {
          result = right;
          return rr != Fail ? Partial : Fail;
        }
        return Fail;
      }

      if ( rr != Complete )
      {
        if ( n->op() == QgsExpressionNodeBinaryOperator::boAnd )
        {
          result = left;
          return lr != Fail ? Partial : Fail;
        }
        return Fail;
      }

      if ( isCaseI )
        result = "(CASEI(" + left + ") " + op + " CASEI(" + right + "))";
      else
        result = '(' + left + ' ' + op + ' ' + right + ')';
      return Complete;
    }

    case QgsExpressionNode::ntBetweenOperator:
    {
      if ( !mSupportsLikeBetweenIn )
        return Fail;
      const QgsExpressionNodeBetweenOperator *n = static_cast<const QgsExpressionNodeBetweenOperator *>( node );
      QString res;

      if ( compileNode( n->node(), res ) != Complete )
        return Fail;

      QString s;
      if ( compileNode( n->lowerBound(), s ) != Complete )
        return Fail;

      res.append( n->negate() ? QStringLiteral( " NOT BETWEEN %1" ).arg( s ) : QStringLiteral( " BETWEEN %1" ).arg( s ) );

      if ( compileNode( n->higherBound(), s ) != Complete )
        return Fail;

      res.append( QStringLiteral( " AND %1" ).arg( s ) );
      result = res;
      return Complete;
    }

    case QgsExpressionNode::ntLiteral:
    {
      const QgsExpressionNodeLiteral *n = static_cast<const QgsExpressionNodeLiteral *>( node );
      result = literalValue( n->value() );
      return Complete;
    }

    case QgsExpressionNode::ntColumnRef:
    {
      const QgsExpressionNodeColumnRef *n = static_cast<const QgsExpressionNodeColumnRef *>( node );

      // QGIS expressions don't care about case sensitive field naming, so we match case insensitively here to the
      // layer's fields and then retrieve the actual case of the field name for use in the compilation
      QString columnName;
      for ( const auto &kv : mQueryables.toStdMap() )
      {
        if ( kv.first.compare( n->name(), Qt::CaseInsensitive ) == 0 )
        {
          columnName = kv.first;
          break;
        }
      }
      if ( columnName.isEmpty() )
      {
        QgsDebugMsgLevel( QStringLiteral( "%1 is not a queryable" ).arg( n->name() ), 4 );
        return Fail;
      }

      result = quotedIdentifier( columnName );

      return Complete;
    }

    case QgsExpressionNode::ntInOperator:
    {
      if ( !mSupportsLikeBetweenIn )
        return Fail;
      const QgsExpressionNodeInOperator *n = static_cast<const QgsExpressionNodeInOperator *>( node );
      QStringList list;

      const auto constList = n->list()->list();
      for ( const QgsExpressionNode *ln : constList )
      {
        QString s;
        if ( compileNode( ln, s ) != Complete )
          return Fail;
        list << s;
      }

      QString nd;
      if ( compileNode( n->node(), nd ) != Complete )
        return Fail;

      result = QStringLiteral( "%1 %2IN (%3)" ).arg( nd, n->isNotIn() ? QStringLiteral( "NOT " ) : QString(), list.join( ',' ) );
      return Complete;
    }

    case QgsExpressionNode::ntFunction:
    {
      const QgsExpressionNodeFunction *n = static_cast<const QgsExpressionNodeFunction *>( node );
      return compileNodeFunction( n, result );
    }

    case QgsExpressionNode::ntCondition:
    case QgsExpressionNode::ntIndexOperator:
      break;
  }

  return Fail;
}
