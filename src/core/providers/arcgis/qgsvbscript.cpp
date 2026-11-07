/***************************************************************************
    qgsvbscript.cpp
    -----------------------
    begin                : November 2026
    copyright            : (C) 2026 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvbscript.h"

#include <cmath>

#include "qgis.h"
#include "qgsvariantutils.h"

#include <QRegularExpression>
#include <QString>

using namespace Qt::StringLiterals;

// external parser entry point
extern QgsVbScriptBlockStatement *parseVbScript( const QString &scriptText, QString &parserErrorMessage );

const char *QgsVbScriptBinaryOperatorExpression::BINARY_OPERATOR_TEXT[] = { "+", "-", "*", "/", "\\", "Mod", "^", "&", "=", "<>", "<", ">", "<=", ">=", "Is", "And", "Or", "Xor", "Eqv", "Imp" };

const char *QgsVbScriptUnaryOperatorExpression::UNARY_OPERATOR_TEXT[] = { "Not", "-", "+" };

//
// QgsVbScriptBlockStatement
//

QgsVbScriptBlockStatement::QgsVbScriptBlockStatement( const QList<QgsVbScriptStatement *> &statements )
  : mStatements( statements )
{}

QgsVbScriptBlockStatement::~QgsVbScriptBlockStatement()
{
  qDeleteAll( mStatements );
}

void QgsVbScriptBlockStatement::appendStatement( QgsVbScriptStatement *statement )
{
  mStatements.append( statement );
}

QString QgsVbScriptBlockStatement::dump() const
{
  QStringList parts;
  parts.reserve( mStatements.size() );
  for ( QgsVbScriptStatement *statement : std::as_const( mStatements ) )
  {
    parts << statement->dump();
  }
  return parts.join( '\n' );
}

QgsVbScriptNode *QgsVbScriptBlockStatement::clone() const
{
  QList<QgsVbScriptStatement *> clonedList;
  clonedList.reserve( mStatements.size() );
  for ( QgsVbScriptStatement *statement : std::as_const( mStatements ) )
  {
    clonedList.append( dynamic_cast<QgsVbScriptStatement *>( statement->clone() ) );
  }
  return new QgsVbScriptBlockStatement( clonedList );
}

void QgsVbScriptBlockStatement::accept( QgsVbScriptVisitor &visitor ) const
{
  visitor.visit( *this );
}

//
// QgsVbScriptVariableDeclarationStatement
//

QgsVbScriptVariableDeclarationStatement::QgsVbScriptVariableDeclarationStatement( const QString &variableName )
  : mVariableNames( { variableName } )
{}

QgsVbScriptVariableDeclarationStatement::QgsVbScriptVariableDeclarationStatement( const QStringList &variableNames )
  : mVariableNames( variableNames )
{}

QString QgsVbScriptVariableDeclarationStatement::dump() const
{
  return u"Dim %1"_s.arg( mVariableNames.join( ", "_L1 ) );
}

QgsVbScriptNode *QgsVbScriptVariableDeclarationStatement::clone() const
{
  return new QgsVbScriptVariableDeclarationStatement( mVariableNames );
}

void QgsVbScriptVariableDeclarationStatement::accept( QgsVbScriptVisitor &visitor ) const
{
  visitor.visit( *this );
}

//
// QgsVbScriptAssignmentStatement
//

QgsVbScriptAssignmentStatement::QgsVbScriptAssignmentStatement( QgsVbScriptExpression *target, QgsVbScriptExpression *value, bool isSet )
  : mTarget( target )
  , mValue( value )
  , mIsSet( isSet )
{}

QString QgsVbScriptAssignmentStatement::dump() const
{
  return u"%1%2 = %3"_s.arg( mIsSet ? "Set "_L1 : ""_L1, mTarget ? mTarget->dump() : QString(), mValue ? mValue->dump() : QString() );
}

QgsVbScriptNode *QgsVbScriptAssignmentStatement::clone() const
{
  QgsVbScriptExpression *clonedTarget = mTarget ? dynamic_cast<QgsVbScriptExpression *>( mTarget->clone() ) : nullptr;
  QgsVbScriptExpression *clonedValue = mValue ? dynamic_cast<QgsVbScriptExpression *>( mValue->clone() ) : nullptr;
  return new QgsVbScriptAssignmentStatement( clonedTarget, clonedValue, mIsSet );
}

void QgsVbScriptAssignmentStatement::accept( QgsVbScriptVisitor &visitor ) const
{
  visitor.visit( *this );
}

//
// QgsVbScriptFunctionDeclarationStatement
//

QgsVbScriptFunctionDeclarationStatement::QgsVbScriptFunctionDeclarationStatement( const QString &functionName, const QStringList &parameters, QgsVbScriptBlockStatement *body, bool isSub )
  : mFunctionName( functionName )
  , mParameters( parameters )
  , mBody( body )
  , mIsSub( isSub )
{}

QString QgsVbScriptFunctionDeclarationStatement::dump() const
{
  const QString bodyDump = mBody ? mBody->dump() : QString();
  const QString keyword = mIsSub ? u"Sub"_s : u"Function"_s;

  return u"%1 %2(%3)\n%4\nEnd %1"_s.arg( keyword, mFunctionName, mParameters.join( ", "_L1 ), bodyDump );
}

QgsVbScriptNode *QgsVbScriptFunctionDeclarationStatement::clone() const
{
  return new QgsVbScriptFunctionDeclarationStatement( mFunctionName, mParameters, mBody ? dynamic_cast<QgsVbScriptBlockStatement *>( mBody->clone() ) : nullptr, mIsSub );
}

void QgsVbScriptFunctionDeclarationStatement::accept( QgsVbScriptVisitor &visitor ) const
{
  visitor.visit( *this );
}

//
// QgsVbScriptExpressionStatement
//

QgsVbScriptExpressionStatement::QgsVbScriptExpressionStatement( QgsVbScriptExpression *expression )
  : mExpression( expression )
{}

QString QgsVbScriptExpressionStatement::dump() const
{
  return mExpression ? mExpression->dump() : QString();
}

QgsVbScriptNode *QgsVbScriptExpressionStatement::clone() const
{
  return new QgsVbScriptExpressionStatement( mExpression ? dynamic_cast<QgsVbScriptExpression *>( mExpression->clone() ) : nullptr );
}

void QgsVbScriptExpressionStatement::accept( QgsVbScriptVisitor &visitor ) const
{
  visitor.visit( *this );
}

//
// QgsVbScriptLiteralExpression
//

QgsVbScriptLiteralExpression::QgsVbScriptLiteralExpression( const QVariant &value )
  : mValue( value )
{}

QString QgsVbScriptLiteralExpression::valueAsString() const
{
  if ( QgsVariantUtils::isNull( mValue ) )
    return u"Null"_s;

  switch ( mValue.userType() )
  {
    case QMetaType::Type::Int:
      return QString::number( mValue.toInt() );
    case QMetaType::Type::LongLong:
      return QString::number( mValue.toLongLong() );
    case QMetaType::Type::Double:
      return qgsDoubleToString( mValue.toDouble() );
    case QMetaType::Type::QString:
      return mValue.toString();
    case QMetaType::Type::Bool:
      return mValue.toBool() ? u"True"_s : u"False"_s;
    default:
      return mValue.toString();
  }
}

QString QgsVbScriptLiteralExpression::dump() const
{
  if ( QgsVariantUtils::isNull( mValue ) )
    return u"Null"_s;

  if ( mValue.userType() == QMetaType::Type::QString )
    return QgsVbScript::quotedString( mValue.toString() );

  return valueAsString();
}

QgsVbScriptNode *QgsVbScriptLiteralExpression::clone() const
{
  return new QgsVbScriptLiteralExpression( mValue );
}

void QgsVbScriptLiteralExpression::accept( QgsVbScriptVisitor &visitor ) const
{
  visitor.visit( *this );
}

//
// QgsVbScriptVariableReferenceExpression
//

QgsVbScriptVariableReferenceExpression::QgsVbScriptVariableReferenceExpression( const QString &name )
  : mName( name )
{}

QString QgsVbScriptVariableReferenceExpression::dump() const
{
  return mName;
}

QgsVbScriptNode *QgsVbScriptVariableReferenceExpression::clone() const
{
  return new QgsVbScriptVariableReferenceExpression( mName );
}

void QgsVbScriptVariableReferenceExpression::accept( QgsVbScriptVisitor &visitor ) const
{
  visitor.visit( *this );
}

//
// QgsVbScriptMemberAccessExpression
//

QgsVbScriptMemberAccessExpression::QgsVbScriptMemberAccessExpression( QgsVbScriptExpression *containerExpression, const QString &memberName )
  : mContainerExpression( containerExpression )
  , mMemberName( memberName )
{}

QString QgsVbScriptMemberAccessExpression::dump() const
{
  return u"%1.%2"_s.arg( mContainerExpression ? mContainerExpression->dump() : QString(), mMemberName );
}

QgsVbScriptNode *QgsVbScriptMemberAccessExpression::clone() const
{
  QgsVbScriptExpression *clonedContainer = mContainerExpression ? dynamic_cast<QgsVbScriptExpression *>( mContainerExpression->clone() ) : nullptr;
  return new QgsVbScriptMemberAccessExpression( clonedContainer, mMemberName );
}

void QgsVbScriptMemberAccessExpression::accept( QgsVbScriptVisitor &visitor ) const
{
  visitor.visit( *this );
}

//
// QgsVbScriptBinaryOperatorExpression
//

QgsVbScriptBinaryOperatorExpression::QgsVbScriptBinaryOperatorExpression( BinaryOperator binaryOperator, QgsVbScriptExpression *leftOperand, QgsVbScriptExpression *rightOperand )
  : mBinaryOperator( binaryOperator )
  , mLeftOperand( leftOperand )
  , mRightOperand( rightOperand )
{}

QString QgsVbScriptBinaryOperatorExpression::text() const
{
  return BINARY_OPERATOR_TEXT[static_cast<int>( mBinaryOperator )];
}

int QgsVbScriptBinaryOperatorExpression::precedence() const
{
  switch ( mBinaryOperator )
  {
    case BinaryOperator::LogicalImp:
      return 1;
    case BinaryOperator::LogicalEqv:
      return 2;
    case BinaryOperator::LogicalXor:
      return 3;
    case BinaryOperator::LogicalOr:
      return 4;
    case BinaryOperator::LogicalAnd:
      return 5;
    case BinaryOperator::Equal:
    case BinaryOperator::NotEqual:
    case BinaryOperator::LessThan:
    case BinaryOperator::GreaterThan:
    case BinaryOperator::LessThanOrEqual:
    case BinaryOperator::GreaterThanOrEqual:
    case BinaryOperator::Is:
      return 6;
    case BinaryOperator::Concat:
      return 7;
    case BinaryOperator::Plus:
    case BinaryOperator::Minus:
      return 8;
    case BinaryOperator::Modulo:
      return 9;
    case BinaryOperator::IntegerDivide:
      return 10;
    case BinaryOperator::Multiply:
    case BinaryOperator::Divide:
      return 11;
    case BinaryOperator::Exponent:
      return 12;
  }
  return -1;
}

bool QgsVbScriptBinaryOperatorExpression::leftAssociative() const
{
  return true;
}

QString QgsVbScriptBinaryOperatorExpression::dump() const
{
  QgsVbScriptBinaryOperatorExpression *leftBinary = dynamic_cast<QgsVbScriptBinaryOperatorExpression *>( mLeftOperand.get() );
  QgsVbScriptBinaryOperatorExpression *rightBinary = dynamic_cast<QgsVbScriptBinaryOperatorExpression *>( mRightOperand.get() );

  const QString leftDump = leftBinary && ( leftBinary->precedence() < precedence() ) ? u"(%1)"_s.arg( mLeftOperand->dump() ) : mLeftOperand->dump();

  const QString rightDump = rightBinary && ( rightBinary->precedence() <= precedence() ) ? u"(%1)"_s.arg( mRightOperand->dump() ) : mRightOperand->dump();

  return u"%1 %2 %3"_s.arg( leftDump, text(), rightDump );
}

QgsVbScriptNode *QgsVbScriptBinaryOperatorExpression::clone() const
{
  QgsVbScriptExpression *leftClone = mLeftOperand ? dynamic_cast<QgsVbScriptExpression *>( mLeftOperand->clone() ) : nullptr;
  QgsVbScriptExpression *rightClone = mRightOperand ? dynamic_cast<QgsVbScriptExpression *>( mRightOperand->clone() ) : nullptr;
  return new QgsVbScriptBinaryOperatorExpression( mBinaryOperator, leftClone, rightClone );
}

void QgsVbScriptBinaryOperatorExpression::accept( QgsVbScriptVisitor &visitor ) const
{
  visitor.visit( *this );
}

//
// QgsVbScriptUnaryOperatorExpression
//

QgsVbScriptUnaryOperatorExpression::QgsVbScriptUnaryOperatorExpression( UnaryOperator unaryOperator, QgsVbScriptExpression *operand )
  : mUnaryOperator( unaryOperator )
  , mOperand( operand )
{}

QString QgsVbScriptUnaryOperatorExpression::text() const
{
  return UNARY_OPERATOR_TEXT[static_cast<int>( mUnaryOperator )];
}

QString QgsVbScriptUnaryOperatorExpression::dump() const
{
  if ( mUnaryOperator == UnaryOperator::Not )
    return u"Not %1"_s.arg( mOperand ? mOperand->dump() : QString() );
  else
    return u"%1%2"_s.arg( text(), mOperand ? mOperand->dump() : QString() );
}

QgsVbScriptNode *QgsVbScriptUnaryOperatorExpression::clone() const
{
  return new QgsVbScriptUnaryOperatorExpression( mUnaryOperator, mOperand ? dynamic_cast<QgsVbScriptExpression *>( mOperand->clone() ) : nullptr );
}

void QgsVbScriptUnaryOperatorExpression::accept( QgsVbScriptVisitor &visitor ) const
{
  visitor.visit( *this );
}

//
// QgsVbScriptFunctionCallExpression
//

QgsVbScriptFunctionCallExpression::QgsVbScriptFunctionCallExpression( QgsVbScriptExpression *targetExpression, const QList<QgsVbScriptExpression *> &arguments )
  : mTargetExpression( targetExpression )
  , mArguments( arguments )
{}

QgsVbScriptFunctionCallExpression::~QgsVbScriptFunctionCallExpression()
{
  qDeleteAll( mArguments );
}

QString QgsVbScriptFunctionCallExpression::dump() const
{
  QStringList argumentStrings;
  argumentStrings.reserve( mArguments.size() );
  for ( QgsVbScriptExpression *argument : std::as_const( mArguments ) )
  {
    argumentStrings.append( argument->dump() );
  }
  return u"%1(%2)"_s.arg( mTargetExpression ? mTargetExpression->dump() : QString(), argumentStrings.join( ", "_L1 ) );
}

QgsVbScriptNode *QgsVbScriptFunctionCallExpression::clone() const
{
  QList<QgsVbScriptExpression *> clonedArguments;
  clonedArguments.reserve( mArguments.size() );
  for ( QgsVbScriptExpression *argument : std::as_const( mArguments ) )
  {
    clonedArguments.append( dynamic_cast<QgsVbScriptExpression *>( argument->clone() ) );
  }
  return new QgsVbScriptFunctionCallExpression( mTargetExpression ? dynamic_cast<QgsVbScriptExpression *>( mTargetExpression->clone() ) : nullptr, clonedArguments );
}

void QgsVbScriptFunctionCallExpression::accept( QgsVbScriptVisitor &visitor ) const
{
  visitor.visit( *this );
}

//
// QgsVbScriptIndexExpression
//

QgsVbScriptIndexExpression::QgsVbScriptIndexExpression( QgsVbScriptExpression *containerExpression, QgsVbScriptExpression *indexExpression )
  : mContainerExpression( containerExpression )
  , mIndexExpression( indexExpression )
{}

QString QgsVbScriptIndexExpression::dump() const
{
  return u"%1[%2]"_s.arg( mContainerExpression ? mContainerExpression->dump() : QString(), mIndexExpression ? mIndexExpression->dump() : QString() );
}

QgsVbScriptNode *QgsVbScriptIndexExpression::clone() const
{
  QgsVbScriptExpression *clonedContainer = mContainerExpression ? dynamic_cast<QgsVbScriptExpression *>( mContainerExpression->clone() ) : nullptr;
  QgsVbScriptExpression *clonedIndex = mIndexExpression ? dynamic_cast<QgsVbScriptExpression *>( mIndexExpression->clone() ) : nullptr;
  return new QgsVbScriptIndexExpression( clonedContainer, clonedIndex );
}

void QgsVbScriptIndexExpression::accept( QgsVbScriptVisitor &visitor ) const
{
  visitor.visit( *this );
}

//
// QgsVbScriptArrayLiteralExpression
//

QgsVbScriptArrayLiteralExpression::QgsVbScriptArrayLiteralExpression( const QList<QgsVbScriptExpression *> &elements )
  : mElements( elements )
{}

QgsVbScriptArrayLiteralExpression::~QgsVbScriptArrayLiteralExpression()
{
  qDeleteAll( mElements );
}

QString QgsVbScriptArrayLiteralExpression::dump() const
{
  QStringList elementStrings;
  elementStrings.reserve( mElements.size() );
  for ( QgsVbScriptExpression *element : std::as_const( mElements ) )
  {
    elementStrings.append( element->dump() );
  }
  return u"Array(%1)"_s.arg( elementStrings.join( ", "_L1 ) );
}

QgsVbScriptNode *QgsVbScriptArrayLiteralExpression::clone() const
{
  QList<QgsVbScriptExpression *> clonedElements;
  clonedElements.reserve( mElements.size() );
  for ( QgsVbScriptExpression *element : std::as_const( mElements ) )
  {
    clonedElements.append( dynamic_cast<QgsVbScriptExpression *>( element->clone() ) );
  }
  return new QgsVbScriptArrayLiteralExpression( clonedElements );
}

void QgsVbScriptArrayLiteralExpression::accept( QgsVbScriptVisitor &visitor ) const
{
  visitor.visit( *this );
}


//
// QgsVbScriptIfStatement
//

QgsVbScriptIfStatement::QgsVbScriptIfStatement( QgsVbScriptExpression *condition, QgsVbScriptBlockStatement *trueBlock, QgsVbScriptBlockStatement *falseBlock, bool isInline )
  : mCondition( condition )
  , mTrueBlock( trueBlock )
  , mFalseBlock( falseBlock )
  , mIsInline( isInline )
{}

QString QgsVbScriptIfStatement::dump() const
{
  if ( mIsInline )
  {
    QString result = u"If %1 Then %2"_s.arg( mCondition ? mCondition->dump() : QString(), mTrueBlock ? mTrueBlock->dump() : QString() );
    if ( mFalseBlock )
      result += u" Else %1"_s.arg( mFalseBlock->dump() );
    return result;
  }

  QString result = u"If %1 Then\n%2"_s.arg( mCondition ? mCondition->dump() : QString(), mTrueBlock ? mTrueBlock->dump() : QString() );
  if ( mFalseBlock )
  {
    if ( mFalseBlock->count() == 1 && mFalseBlock->statements().at( 0 )->nodeType() == NodeType::IfStatement )
    {
      const QgsVbScriptIfStatement *elseIfStatement = dynamic_cast<const QgsVbScriptIfStatement *>( mFalseBlock->statements().at( 0 ) );
      if ( elseIfStatement && !elseIfStatement->isInline() )
      {
        result += u"\nElse%1"_s.arg( elseIfStatement->dump() );
        return result;
      }
    }
    result += u"\nElse\n%1"_s.arg( mFalseBlock->dump() );
  }
  result += "\nEnd If"_L1;
  return result;
}

QgsVbScriptNode *QgsVbScriptIfStatement::clone() const
{
  QgsVbScriptExpression *clonedCondition = mCondition ? dynamic_cast<QgsVbScriptExpression *>( mCondition->clone() ) : nullptr;
  QgsVbScriptBlockStatement *clonedTrueBlock = mTrueBlock ? dynamic_cast<QgsVbScriptBlockStatement *>( mTrueBlock->clone() ) : nullptr;
  QgsVbScriptBlockStatement *clonedFalseBlock = mFalseBlock ? dynamic_cast<QgsVbScriptBlockStatement *>( mFalseBlock->clone() ) : nullptr;
  return new QgsVbScriptIfStatement( clonedCondition, clonedTrueBlock, clonedFalseBlock, mIsInline );
}

void QgsVbScriptIfStatement::accept( QgsVbScriptVisitor &visitor ) const
{
  visitor.visit( *this );
}

//
// QgsVbScriptForStatement
//

QgsVbScriptForStatement::QgsVbScriptForStatement(
  const QString &variableName, QgsVbScriptExpression *startExpression, QgsVbScriptExpression *endExpression, QgsVbScriptExpression *stepExpression, QgsVbScriptBlockStatement *body
)
  : mVariableName( variableName )
  , mStartExpression( startExpression )
  , mEndExpression( endExpression )
  , mStepExpression( stepExpression )
  , mBody( body )
{}

QString QgsVbScriptForStatement::dump() const
{
  QString stepString;
  if ( mStepExpression )
    stepString = u" Step %1"_s.arg( mStepExpression->dump() );

  return u"For %1 = %2 To %3%4\n%5\nNext"_s
    .arg( mVariableName, mStartExpression ? mStartExpression->dump() : QString(), mEndExpression ? mEndExpression->dump() : QString(), stepString, mBody ? mBody->dump() : QString() );
}

QgsVbScriptNode *QgsVbScriptForStatement::clone() const
{
  QgsVbScriptExpression *clonedStart = mStartExpression ? dynamic_cast<QgsVbScriptExpression *>( mStartExpression->clone() ) : nullptr;
  QgsVbScriptExpression *clonedEnd = mEndExpression ? dynamic_cast<QgsVbScriptExpression *>( mEndExpression->clone() ) : nullptr;
  QgsVbScriptExpression *clonedStep = mStepExpression ? dynamic_cast<QgsVbScriptExpression *>( mStepExpression->clone() ) : nullptr;
  QgsVbScriptBlockStatement *clonedBody = mBody ? dynamic_cast<QgsVbScriptBlockStatement *>( mBody->clone() ) : nullptr;
  return new QgsVbScriptForStatement( mVariableName, clonedStart, clonedEnd, clonedStep, clonedBody );
}

void QgsVbScriptForStatement::accept( QgsVbScriptVisitor &visitor ) const
{
  visitor.visit( *this );
}

//
// QgsVbScriptForEachStatement
//

QgsVbScriptForEachStatement::QgsVbScriptForEachStatement( const QString &variableName, QgsVbScriptExpression *collection, QgsVbScriptBlockStatement *body )
  : mVariableName( variableName )
  , mCollection( collection )
  , mBody( body )
{}

QString QgsVbScriptForEachStatement::dump() const
{
  return u"For Each %1 In %2\n%3\nNext"_s.arg( mVariableName, mCollection ? mCollection->dump() : QString(), mBody ? mBody->dump() : QString() );
}

QgsVbScriptNode *QgsVbScriptForEachStatement::clone() const
{
  QgsVbScriptExpression *clonedCollection = mCollection ? dynamic_cast<QgsVbScriptExpression *>( mCollection->clone() ) : nullptr;
  QgsVbScriptBlockStatement *clonedBody = mBody ? dynamic_cast<QgsVbScriptBlockStatement *>( mBody->clone() ) : nullptr;
  return new QgsVbScriptForEachStatement( mVariableName, clonedCollection, clonedBody );
}

void QgsVbScriptForEachStatement::accept( QgsVbScriptVisitor &visitor ) const
{
  visitor.visit( *this );
}

//
// QgsVbScriptWhileStatement
//

QgsVbScriptWhileStatement::QgsVbScriptWhileStatement( LoopType loopType, QgsVbScriptExpression *condition, QgsVbScriptBlockStatement *body )
  : mLoopType( loopType )
  , mCondition( condition )
  , mBody( body )
{}

QString QgsVbScriptWhileStatement::dump() const
{
  const QString bodyDump = mBody ? mBody->dump() : QString();
  const QString conditionDump = mCondition ? mCondition->dump() : QString();

  switch ( mLoopType )
  {
    case LoopType::WhileWend:
      return u"While %1\n%2\nWend"_s.arg( conditionDump, bodyDump );
    case LoopType::DoWhile:
      return u"Do While %1\n%2\nLoop"_s.arg( conditionDump, bodyDump );
    case LoopType::DoUntil:
      return u"Do Until %1\n%2\nLoop"_s.arg( conditionDump, bodyDump );
    case LoopType::DoLoopWhile:
      return u"Do\n%1\nLoop While %2"_s.arg( bodyDump, conditionDump );
    case LoopType::DoLoopUntil:
      return u"Do\n%1\nLoop Until %2"_s.arg( bodyDump, conditionDump );
  }
  return QString();
}

QgsVbScriptNode *QgsVbScriptWhileStatement::clone() const
{
  QgsVbScriptExpression *clonedCondition = mCondition ? dynamic_cast<QgsVbScriptExpression *>( mCondition->clone() ) : nullptr;
  QgsVbScriptBlockStatement *clonedBody = mBody ? dynamic_cast<QgsVbScriptBlockStatement *>( mBody->clone() ) : nullptr;
  return new QgsVbScriptWhileStatement( mLoopType, clonedCondition, clonedBody );
}

void QgsVbScriptWhileStatement::accept( QgsVbScriptVisitor &visitor ) const
{
  visitor.visit( *this );
}


//
// QgsVbScriptExitStatement
//

QgsVbScriptExitStatement::QgsVbScriptExitStatement( ExitType exitType )
  : mExitType( exitType )
{}

QString QgsVbScriptExitStatement::dump() const
{
  switch ( mExitType )
  {
    case ExitType::ExitDo:
      return u"Exit Do"_s;
    case ExitType::ExitFor:
      return u"Exit For"_s;
    case ExitType::ExitFunction:
      return u"Exit Function"_s;
    case ExitType::ExitSub:
      return u"Exit Sub"_s;
  }
  return QString();
}

QgsVbScriptNode *QgsVbScriptExitStatement::clone() const
{
  return new QgsVbScriptExitStatement( mExitType );
}

void QgsVbScriptExitStatement::accept( QgsVbScriptVisitor &visitor ) const
{
  visitor.visit( *this );
}

//
// QgsVbScriptRecursiveVisitor
//

void QgsVbScriptRecursiveVisitor::visit( const QgsVbScriptBlockStatement &node )
{
  const QList<QgsVbScriptStatement *> statements = node.statements();
  for ( QgsVbScriptStatement *statement : statements )
  {
    statement->accept( *this );
  }
}

void QgsVbScriptRecursiveVisitor::visit( const QgsVbScriptVariableDeclarationStatement & )
{}

void QgsVbScriptRecursiveVisitor::visit( const QgsVbScriptAssignmentStatement &node )
{
  if ( node.target() )
    node.target()->accept( *this );
  if ( node.value() )
    node.value()->accept( *this );
}

void QgsVbScriptRecursiveVisitor::visit( const QgsVbScriptFunctionDeclarationStatement &node )
{
  if ( node.body() )
    node.body()->accept( *this );
}

void QgsVbScriptRecursiveVisitor::visit( const QgsVbScriptExpressionStatement &node )
{
  if ( node.expression() )
    node.expression()->accept( *this );
}

void QgsVbScriptRecursiveVisitor::visit( const QgsVbScriptLiteralExpression & )
{}

void QgsVbScriptRecursiveVisitor::visit( const QgsVbScriptVariableReferenceExpression & )
{}

void QgsVbScriptRecursiveVisitor::visit( const QgsVbScriptMemberAccessExpression &node )
{
  if ( node.containerExpression() )
    node.containerExpression()->accept( *this );
}

void QgsVbScriptRecursiveVisitor::visit( const QgsVbScriptBinaryOperatorExpression &node )
{
  if ( node.leftOperand() )
    node.leftOperand()->accept( *this );
  if ( node.rightOperand() )
    node.rightOperand()->accept( *this );
}

void QgsVbScriptRecursiveVisitor::visit( const QgsVbScriptUnaryOperatorExpression &node )
{
  if ( node.operand() )
    node.operand()->accept( *this );
}

void QgsVbScriptRecursiveVisitor::visit( const QgsVbScriptFunctionCallExpression &node )
{
  if ( node.targetExpression() )
    node.targetExpression()->accept( *this );
  const QList<QgsVbScriptExpression *> arguments = node.arguments();
  for ( QgsVbScriptExpression *argument : arguments )
  {
    argument->accept( *this );
  }
}

void QgsVbScriptRecursiveVisitor::visit( const QgsVbScriptIndexExpression &node )
{
  if ( node.containerExpression() )
    node.containerExpression()->accept( *this );
  if ( node.indexExpression() )
    node.indexExpression()->accept( *this );
}

void QgsVbScriptRecursiveVisitor::visit( const QgsVbScriptArrayLiteralExpression &node )
{
  const QList<QgsVbScriptExpression *> elements = node.elements();
  for ( QgsVbScriptExpression *element : elements )
  {
    element->accept( *this );
  }
}

void QgsVbScriptRecursiveVisitor::visit( const QgsVbScriptIfStatement &node )
{
  if ( node.condition() )
    node.condition()->accept( *this );
  if ( node.trueBlock() )
    node.trueBlock()->accept( *this );
  if ( node.falseBlock() )
    node.falseBlock()->accept( *this );
}

void QgsVbScriptRecursiveVisitor::visit( const QgsVbScriptForStatement &node )
{
  if ( node.startExpression() )
    node.startExpression()->accept( *this );
  if ( node.endExpression() )
    node.endExpression()->accept( *this );
  if ( node.stepExpression() )
    node.stepExpression()->accept( *this );
  if ( node.body() )
    node.body()->accept( *this );
}

void QgsVbScriptRecursiveVisitor::visit( const QgsVbScriptForEachStatement &node )
{
  if ( node.collection() )
    node.collection()->accept( *this );
  if ( node.body() )
    node.body()->accept( *this );
}

void QgsVbScriptRecursiveVisitor::visit( const QgsVbScriptWhileStatement &node )
{
  if ( node.condition() )
    node.condition()->accept( *this );
  if ( node.body() )
    node.body()->accept( *this );
}

void QgsVbScriptRecursiveVisitor::visit( const QgsVbScriptExitStatement & )
{}

//
// QgsVbScriptScript
//

QgsVbScript::QgsVbScript( const QString &scriptText )
  : mScript( scriptText )
{
  if ( !scriptText.isEmpty() )
  {
    mRootBlock.reset( parseVbScript( scriptText, mParserErrorString ) );
  }
}

QgsVbScript::QgsVbScript( const QgsVbScript &other )
  : mScript( other.mScript )
  , mParserErrorString( other.mParserErrorString )
{
  if ( other.mRootBlock )
  {
    mRootBlock.reset( dynamic_cast<QgsVbScriptBlockStatement *>( other.mRootBlock->clone() ) );
  }
}

QgsVbScript &QgsVbScript::operator=( const QgsVbScript &other )
{
  if ( this != &other )
  {
    mScript = other.mScript;
    mParserErrorString = other.mParserErrorString;
    if ( other.mRootBlock )
    {
      mRootBlock.reset( dynamic_cast<QgsVbScriptBlockStatement *>( other.mRootBlock->clone() ) );
    }
    else
    {
      mRootBlock.reset();
    }
  }
  return *this;
}

QgsVbScript::~QgsVbScript() = default;

bool QgsVbScript::hasParserError() const
{
  return !mParserErrorString.isEmpty() || ( !mScript.trimmed().isEmpty() && !mRootBlock );
}

QString QgsVbScript::parserErrorString() const
{
  return mParserErrorString;
}

const QgsVbScriptBlockStatement *QgsVbScript::rootBlock() const
{
  return mRootBlock.get();
}

QString QgsVbScript::script() const
{
  if ( !mScript.isEmpty() )
    return mScript;

  return dump();
}

QString QgsVbScript::dump() const
{
  if ( !mRootBlock )
    return QString();

  return mRootBlock->dump();
}

void QgsVbScript::acceptVisitor( QgsVbScriptVisitor &visitor ) const
{
  if ( mRootBlock )
    mRootBlock->accept( visitor );
}

QString QgsVbScript::quotedString( const QString &text )
{
  QString escaped = text;
  escaped.replace( '\"'_L1, "\"\""_L1 );
  return u"\"%1\""_s.arg( escaped );
}
