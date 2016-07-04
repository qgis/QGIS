/***************************************************************************
                              qgssqlstatement.cpp
                             -------------------
    begin                : April 2016
    copyright            : (C) 2011 by Martin Dobias
    copyright            : (C) 2016 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssqlstatement.h"

#include <math.h>
#include <limits>

static const QRegExp identifierRE( "^[A-Za-z_\x80-\xff][A-Za-z0-9_\x80-\xff]*$" );

// from parser
extern QgsSQLStatement::Node* parse( const QString& str, QString& parserErrorMsg );

///////////////////////////////////////////////
// operators

const char* QgsSQLStatement::BinaryOperatorText[] =
{
  // this must correspond (number and order of element) to the declaration of the enum BinaryOperator
  "OR", "AND",
  "=", "<>", "<=", ">=", "<", ">", "LIKE", "NOT LIKE", "ILIKE", "NOT ILIKE", "IS", "IS NOT",
  "+", "-", "*", "/", "//", "%", "^",
  "||"
};

const char* QgsSQLStatement::UnaryOperatorText[] =
{
  // this must correspond (number and order of element) to the declaration of the enum UnaryOperator
  "NOT", "-"
};

const char* QgsSQLStatement::JoinTypeText[] =
{
  // this must correspond (number and order of element) to the declaration of the enum JoinType
  "", "LEFT", "LEFT OUTER", "RIGHT", "RIGHT OUTER", "CROSS", "INNER", "FULL"
};

//////

QString QgsSQLStatement::statement() const
{
  if ( !mStatement.isNull() )
    return mStatement;
  else
    return dump();
}

QString QgsSQLStatement::dump() const
{
  if ( !mRootNode )
    return tr( "(no root)" );

  return mRootNode->dump();
}

QString QgsSQLStatement::quotedIdentifier( QString name )
{
  return QString( "\"%1\"" ).arg( name.replace( '\"', "\"\"" ) );
}

QString QgsSQLStatement::quotedIdentifierIfNeeded( QString name )
{
  // This might not be complete, but it must be at least what we recognize
  static const char* const reservedKeyWords[] =
  {
    "AND", "OR", "NOT", "LIKE", "IN", "IS", "BETWEEN", "NULL", "SELECT", "ALL", "DISTINCT", "CAST", "AS",
    "FROM", "JOIN", "ON", "USING", "WHERE", "ORDER", "BY", "ASC", "DESC",
    "LEFT", "RIGHT", "INNER", "OUTER", "CROSS", "FULL", "NATURAL", "UNION",
    "OFFSET", "LIMIT", "GROUP", "HAVING"
  };

  for ( size_t i = 0; i < sizeof( reservedKeyWords ) / sizeof( reservedKeyWords[0] ); ++i )
  {
    if ( name.compare( QString( reservedKeyWords[i] ), Qt::CaseInsensitive ) == 0 )
    {
      return quotedIdentifier( name );
    }
  }
  return identifierRE.exactMatch( name ) ? name : quotedIdentifier( name );
}

QString QgsSQLStatement::stripQuotedIdentifier( QString text )
{
  if ( text.length() >= 2 && text[0] == '"' && text[text.length()-1] == '"' )
  {
    // strip double quotes on start,end
    text = text.mid( 1, text.length() - 2 );

    // make single "double quotes" from double "double quotes"
    text.replace( "\"\"", "\"" );
  }
  return text;
}

QString QgsSQLStatement::quotedString( QString text )
{
  text.replace( '\'', "''" );
  text.replace( '\\', "\\\\" );
  text.replace( '\n', "\\n" );
  text.replace( '\t', "\\t" );
  return QString( "'%1'" ).arg( text );
}

QgsSQLStatement::QgsSQLStatement( const QString& expr )
{
  mRootNode = ::parse( expr, mParserErrorString );
  mStatement = expr;
}

QgsSQLStatement::QgsSQLStatement( const QgsSQLStatement& other )
{
  mRootNode = ::parse( other.mStatement, mParserErrorString );
  mStatement = other.mStatement;
}

QgsSQLStatement& QgsSQLStatement::operator=( const QgsSQLStatement & other )
{
  if ( &other != this )
  {
    delete mRootNode;
    mParserErrorString.clear();
    mRootNode = ::parse( other.mStatement, mParserErrorString );
    mStatement = other.mStatement;
  }
  return *this;
}

QgsSQLStatement::~QgsSQLStatement()
{
  delete mRootNode;
}

bool QgsSQLStatement::hasParserError() const { return !mParserErrorString.isNull(); }

QString QgsSQLStatement::parserErrorString() const { return mParserErrorString; }

void QgsSQLStatement::acceptVisitor( QgsSQLStatement::Visitor& v ) const
{
  if ( mRootNode )
    mRootNode->accept( v );
}

const QgsSQLStatement::Node* QgsSQLStatement::rootNode() const
{
  return mRootNode;
}

void QgsSQLStatement::RecursiveVisitor::visit( const QgsSQLStatement::NodeSelect& n )
{
  Q_FOREACH ( QgsSQLStatement::NodeTableDef* table, n.tables() )
  {
    table->accept( *this );
  }
  Q_FOREACH ( QgsSQLStatement::NodeSelectedColumn* column, n.columns() )
  {
    column->accept( *this );
  }
  Q_FOREACH ( QgsSQLStatement::NodeJoin* join, n.joins() )
  {
    join->accept( *this );
  }
  QgsSQLStatement::Node* where = n.where();
  if ( where )
    where->accept( *this );
  Q_FOREACH ( QgsSQLStatement::NodeColumnSorted* column, n.orderBy() )
  {
    column->accept( *this );
  }
}

void QgsSQLStatement::RecursiveVisitor::visit( const QgsSQLStatement::NodeJoin& n )
{
  n.tableDef()->accept( *this );
  QgsSQLStatement::Node* expr = n.onExpr();
  if ( expr )
    expr->accept( *this );
}

/** \ingroup core
 * Internal use.
 *  @note not available in Python bindings
 */
class QgsSQLStatementCollectTableNames: public QgsSQLStatement::RecursiveVisitor
{
  public:
    typedef QPair<QString, QString> TableColumnPair;

    QgsSQLStatementCollectTableNames() {}

    void visit( const QgsSQLStatement::NodeColumnRef& n ) override;
    void visit( const QgsSQLStatement::NodeTableDef& n ) override;

    QSet<QString> tableNamesDeclared;
    QSet<TableColumnPair> tableNamesReferenced;
};

void QgsSQLStatementCollectTableNames::visit( const QgsSQLStatement::NodeColumnRef& n )
{
  if ( !n.tableName().isEmpty() )
    tableNamesReferenced.insert( TableColumnPair( n.tableName(), n.name() ) );
  QgsSQLStatement::RecursiveVisitor::visit( n );
}

void QgsSQLStatementCollectTableNames::visit( const QgsSQLStatement::NodeTableDef& n )
{
  tableNamesDeclared.insert( n.alias().isEmpty() ? n.name() : n.alias() );
  QgsSQLStatement::RecursiveVisitor::visit( n );
}

bool QgsSQLStatement::doBasicValidationChecks( QString& errorMsgOut ) const
{
  errorMsgOut.clear();
  if ( mRootNode == nullptr )
  {
    errorMsgOut = tr( "No root node" );
    return false;
  }
  QgsSQLStatementCollectTableNames v;
  mRootNode->accept( v );

  Q_FOREACH ( QgsSQLStatementCollectTableNames::TableColumnPair pair, v.tableNamesReferenced )
  {
    if ( !v.tableNamesDeclared.contains( pair.first ) )
    {
      if ( !errorMsgOut.isEmpty() )
        errorMsgOut += " ";
      errorMsgOut += QString( tr( "Table %1 is referenced by column %2, but not selected in FROM / JOIN." ) ).arg( pair.first ).arg( pair.second );
    }
  }

  return errorMsgOut.isEmpty();
}

///////////////////////////////////////////////
// nodes

QgsSQLStatement::NodeList* QgsSQLStatement::NodeList::clone() const
{
  NodeList* nl = new NodeList;
  Q_FOREACH ( Node* node, mList )
  {
    nl->mList.append( node->clone() );
  }

  return nl;
}

QString QgsSQLStatement::NodeList::dump() const
{
  QString msg;
  bool first = true;
  Q_FOREACH ( Node* n, mList )
  {
    if ( !first ) msg += ", ";
    else first = false;
    msg += n->dump();
  }
  return msg;
}


//

QString QgsSQLStatement::NodeUnaryOperator::dump() const
{
  return QString( "%1 %2" ).arg( UnaryOperatorText[mOp], mOperand->dump() );
}

QgsSQLStatement::Node*QgsSQLStatement::NodeUnaryOperator::clone() const
{
  return new NodeUnaryOperator( mOp, mOperand->clone() );
}

//

int QgsSQLStatement::NodeBinaryOperator::precedence() const
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
  Q_ASSERT( 0 && "unexpected binary operator" );
  return -1;
}

bool QgsSQLStatement::NodeBinaryOperator::leftAssociative() const
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
  Q_ASSERT( 0 && "unexpected binary operator" );
  return false;
}

QString QgsSQLStatement::NodeBinaryOperator::dump() const
{
  QgsSQLStatement::NodeBinaryOperator *lOp = dynamic_cast<QgsSQLStatement::NodeBinaryOperator *>( mOpLeft );
  QgsSQLStatement::NodeBinaryOperator *rOp = dynamic_cast<QgsSQLStatement::NodeBinaryOperator *>( mOpRight );
  QgsSQLStatement::NodeUnaryOperator *ruOp = dynamic_cast<QgsSQLStatement::NodeUnaryOperator *>( mOpRight );

  QString rdump( mOpRight->dump() );

  // avoid dumping "IS (NOT ...)" as "IS NOT ..."
  if ( mOp == boIs && ruOp && ruOp->op() == uoNot )
  {
    rdump.prepend( '(' ).append( ')' );
  }

  QString fmt;
  if ( leftAssociative() )
  {
    fmt += lOp && ( lOp->precedence() < precedence() ) ? "(%1)" : "%1";
    fmt += " %2 ";
    fmt += rOp && ( rOp->precedence() <= precedence() ) ? "(%3)" : "%3";
  }
  else
  {
    fmt += lOp && ( lOp->precedence() <= precedence() ) ? "(%1)" : "%1";
    fmt += " %2 ";
    fmt += rOp && ( rOp->precedence() < precedence() ) ? "(%3)" : "%3";
  }

  return fmt.arg( mOpLeft->dump(), BinaryOperatorText[mOp], rdump );
}

QgsSQLStatement::Node* QgsSQLStatement::NodeBinaryOperator::clone() const
{
  return new NodeBinaryOperator( mOp, mOpLeft->clone(), mOpRight->clone() );
}

//

QString QgsSQLStatement::NodeInOperator::dump() const
{
  return QString( "%1 %2IN (%3)" ).arg( mNode->dump(), mNotIn ? "NOT " : "", mList->dump() );
}

QgsSQLStatement::Node* QgsSQLStatement::NodeInOperator::clone() const
{
  return new NodeInOperator( mNode->clone(), mList->clone(), mNotIn );
}

//

QString QgsSQLStatement::NodeBetweenOperator::dump() const
{
  return QString( "%1 %2BETWEEN %3 AND %4" ).arg( mNode->dump(), mNotBetween ? "NOT " : "", mMinVal->dump(), mMaxVal->dump() );
}

QgsSQLStatement::Node* QgsSQLStatement::NodeBetweenOperator::clone() const
{
  return new NodeBetweenOperator( mNode->clone(), mMinVal->clone(), mMaxVal->clone(), mNotBetween );
}

//

QString QgsSQLStatement::NodeFunction::dump() const
{
  return QString( "%1(%2)" ).arg( mName, mArgs ? mArgs->dump() : QString() ); // function
}

QgsSQLStatement::Node* QgsSQLStatement::NodeFunction::clone() const
{
  return new NodeFunction( mName, mArgs ? mArgs->clone() : nullptr );
}

//

QString QgsSQLStatement::NodeLiteral::dump() const
{
  if ( mValue.isNull() )
    return "NULL";

  switch ( mValue.type() )
  {
    case QVariant::Int:
      return QString::number( mValue.toInt() );
    case QVariant::LongLong:
      return QString::number( mValue.toLongLong() );
    case QVariant::Double:
      return QString::number( mValue.toDouble() );
    case QVariant::String:
      return quotedString( mValue.toString() );
    case QVariant::Bool:
      return mValue.toBool() ? "TRUE" : "FALSE";
    default:
      return tr( "[unsupported type;%1; value:%2]" ).arg( mValue.typeName(), mValue.toString() );
  }
}

QgsSQLStatement::Node* QgsSQLStatement::NodeLiteral::clone() const
{
  return new NodeLiteral( mValue );
}

//

QString QgsSQLStatement::NodeColumnRef::dump() const
{
  QString ret;
  if ( mDistinct )
    ret += "DISTINCT ";
  if ( !mTableName.isEmpty() )
  {
    ret += quotedIdentifierIfNeeded( mTableName );
    ret += '.';
  }
  ret += ( mStar ) ? mName : quotedIdentifierIfNeeded( mName );
  return ret;
}

QgsSQLStatement::Node* QgsSQLStatement::NodeColumnRef::clone() const
{
  return cloneThis();
}

QgsSQLStatement::NodeColumnRef* QgsSQLStatement::NodeColumnRef::cloneThis() const
{
  NodeColumnRef* newColumnRef = new NodeColumnRef( mTableName, mName, mStar );
  newColumnRef->setDistinct( mDistinct );
  return newColumnRef;
}

//

QString QgsSQLStatement::NodeSelectedColumn::dump() const
{
  QString ret;
  ret += mColumnNode->dump();
  if ( !mAlias.isEmpty() )
  {
    ret += " AS ";
    ret += quotedIdentifierIfNeeded( mAlias );
  }
  return ret;
}

QgsSQLStatement::NodeSelectedColumn* QgsSQLStatement::NodeSelectedColumn::cloneThis() const
{
  NodeSelectedColumn* newObj = new NodeSelectedColumn( mColumnNode->clone() );
  newObj->setAlias( mAlias );
  return newObj;
}

QgsSQLStatement::Node* QgsSQLStatement::NodeSelectedColumn::clone() const
{
  return cloneThis();
}
//

QString QgsSQLStatement::NodeTableDef::dump() const
{
  QString ret;
  ret = quotedIdentifierIfNeeded( mName );
  if ( !mAlias.isEmpty() )
  {
    ret += " AS ";
    ret += quotedIdentifierIfNeeded( mAlias );
  }
  return ret;
}

QgsSQLStatement::NodeTableDef* QgsSQLStatement::NodeTableDef::cloneThis() const
{
  return new NodeTableDef( mName, mAlias );
}

QgsSQLStatement::Node* QgsSQLStatement::NodeTableDef::clone() const
{
  return cloneThis();
}

//

QString QgsSQLStatement::NodeSelect::dump() const
{
  QString ret = "SELECT ";
  if ( mDistinct )
    ret += "DISTINCT ";
  bool bFirstColumn = true;
  Q_FOREACH ( QgsSQLStatement::NodeSelectedColumn* column, mColumns )
  {
    if ( !bFirstColumn )
      ret += ", ";
    bFirstColumn = false;
    ret += column->dump();
  }
  ret += " FROM ";
  bool bFirstTable = true;
  Q_FOREACH ( QgsSQLStatement::NodeTableDef* table, mTableList )
  {
    if ( !bFirstTable )
      ret += ", ";
    bFirstTable = false;
    ret += table->dump();
  }
  Q_FOREACH ( QgsSQLStatement::NodeJoin* join, mJoins )
  {
    ret += ' ';
    ret += join->dump();
  }
  if ( mWhere != nullptr )
  {
    ret += " WHERE ";
    ret += mWhere->dump();
  }
  if ( !mOrderBy.isEmpty() )
  {
    ret += " ORDER BY ";
    bool bFirst = true;
    Q_FOREACH ( QgsSQLStatement::NodeColumnSorted* orderBy, mOrderBy )
    {
      if ( !bFirst )
        ret += ", ";
      bFirst = false;
      ret += orderBy->dump();
    }
  }
  return ret;
}

QgsSQLStatement::Node* QgsSQLStatement::NodeSelect::clone() const
{
  QList<QgsSQLStatement::NodeSelectedColumn*> newColumnList;
  Q_FOREACH ( QgsSQLStatement::NodeSelectedColumn* column, mColumns )
  {
    newColumnList.push_back( column->cloneThis() );
  }
  QList<QgsSQLStatement::NodeTableDef*> newTableList;
  Q_FOREACH ( QgsSQLStatement::NodeTableDef* table, mTableList )
  {
    newTableList.push_back( table->cloneThis() );
  }
  QgsSQLStatement::NodeSelect* newSelect = new NodeSelect( newTableList, newColumnList, mDistinct );
  Q_FOREACH ( QgsSQLStatement::NodeJoin* join, mJoins )
  {
    newSelect->appendJoin( join->cloneThis() );
  }
  if ( mWhere != nullptr )
  {
    newSelect->setWhere( mWhere->clone() );
  }
  QList<QgsSQLStatement::NodeColumnSorted*> newOrderByList;
  Q_FOREACH ( QgsSQLStatement::NodeColumnSorted* columnSorted, mOrderBy )
  {
    newOrderByList.push_back( columnSorted->cloneThis() );
  }
  newSelect->setOrderBy( newOrderByList );
  return newSelect;
}

//

QString QgsSQLStatement::NodeJoin::dump() const
{
  QString ret;
  if ( mType != jtDefault )
  {
    ret += JoinTypeText[mType];
    ret += " ";
  }
  ret += "JOIN ";
  ret += mTableDef->dump();
  if ( mOnExpr != nullptr )
  {
    ret += " ON ";
    ret += mOnExpr->dump();
  }
  else
  {
    ret += " USING (";
    bool first = true;
    Q_FOREACH ( QString column, mUsingColumns )
    {
      if ( !first )
        ret += ", ";
      first = false;
      ret += quotedIdentifierIfNeeded( column );
    }
    ret += ")";
  }
  return ret;
}

QgsSQLStatement::Node* QgsSQLStatement::NodeJoin::clone() const
{
  return cloneThis();
}

QgsSQLStatement::NodeJoin* QgsSQLStatement::NodeJoin::cloneThis() const
{
  if ( mOnExpr != nullptr )
    return new NodeJoin( mTableDef->cloneThis(), mOnExpr->clone(), mType );
  else
    return new NodeJoin( mTableDef->cloneThis(), mUsingColumns, mType );
}

//

QString QgsSQLStatement::NodeColumnSorted::dump() const
{
  QString ret;
  ret = mColumn->dump();
  if ( !mAsc )
    ret += " DESC";
  return ret;
}

QgsSQLStatement::Node* QgsSQLStatement::NodeColumnSorted::clone() const
{
  return cloneThis();
}

QgsSQLStatement::NodeColumnSorted* QgsSQLStatement::NodeColumnSorted::cloneThis() const
{
  return new NodeColumnSorted( mColumn->cloneThis(), mAsc );
}

//

QString QgsSQLStatement::NodeCast::dump() const
{
  QString ret( "CAST(" );
  ret += mNode->dump();
  ret += " AS ";
  ret += mType;
  ret += ')';
  return ret;
}

QgsSQLStatement::Node* QgsSQLStatement::NodeCast::clone() const
{
  return new NodeCast( mNode->clone(), mType );
}
