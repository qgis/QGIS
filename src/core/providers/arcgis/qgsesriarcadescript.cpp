/***************************************************************************
    qgsesriarcadescript.cpp
    -----------------------
    begin                : October 2026
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

#include "qgsesriarcadescript.h"

#include <cmath>

#include "qgis.h"
#include "qgsvariantutils.h"

#include <QRegularExpression>
#include <QString>

using namespace Qt::StringLiterals;

// external parser entry point
extern QgsEsriArcadeBlockStatement *parseArcadeScript( const QString &scriptText, QString &parserErrorMessage );

const char *QgsEsriArcadeBinaryOperatorExpression::BINARY_OPERATOR_TEXT[] = { "+", "-", "*", "/", "%", "==", "!=", "<", ">", "<=", ">=", "&&", "||", "&", "|", "^", "<<", ">>", ">>>" };

const char *QgsEsriArcadeUnaryOperatorExpression::UNARY_OPERATOR_TEXT[] = { "!", "-", "+", "~", "++", "--", "++", "--" };

const char *QgsEsriArcadeAssignmentStatement::ASSIGNMENT_OPERATOR_TEXT[] = { "=", "+=", "-=", "*=", "/=" };

//
// QgsEsriArcadeBlockStatement
//

QgsEsriArcadeBlockStatement::QgsEsriArcadeBlockStatement( const QList<QgsEsriArcadeStatement *> &statements )
  : mStatements( statements )
{}

QgsEsriArcadeBlockStatement::~QgsEsriArcadeBlockStatement()
{
  qDeleteAll( mStatements );
}

void QgsEsriArcadeBlockStatement::appendStatement( QgsEsriArcadeStatement *statement )
{
  mStatements.append( statement );
}

QString QgsEsriArcadeBlockStatement::dump() const
{
  QStringList parts;
  parts.reserve( mStatements.size() );
  for ( QgsEsriArcadeStatement *statement : std::as_const( mStatements ) )
  {
    parts << statement->dump();
  }
  return parts.join( '\n' );
}

QgsEsriArcadeNode *QgsEsriArcadeBlockStatement::clone() const
{
  QList<QgsEsriArcadeStatement *> clonedList;
  clonedList.reserve( mStatements.size() );
  for ( QgsEsriArcadeStatement *statement : std::as_const( mStatements ) )
  {
    clonedList.append( dynamic_cast<QgsEsriArcadeStatement *>( statement->clone() ) );
  }
  return new QgsEsriArcadeBlockStatement( clonedList );
}

void QgsEsriArcadeBlockStatement::accept( QgsEsriArcadeVisitor &visitor ) const
{
  visitor.visit( *this );
}

//
// QgsEsriArcadeVariableDeclarationStatement
//

QgsEsriArcadeVariableDeclarationStatement::QgsEsriArcadeVariableDeclarationStatement( const QString &variableName, QgsEsriArcadeExpression *initializer )
  : mVariableName( variableName )
  , mInitializer( initializer )
{}

QString QgsEsriArcadeVariableDeclarationStatement::dump() const
{
  if ( mInitializer )
    return u"var %1 = %2;"_s.arg( mVariableName, mInitializer->dump() );
  else
    return u"var %1;"_s.arg( mVariableName );
}

QgsEsriArcadeNode *QgsEsriArcadeVariableDeclarationStatement::clone() const
{
  return new QgsEsriArcadeVariableDeclarationStatement( mVariableName, mInitializer ? dynamic_cast<QgsEsriArcadeExpression *>( mInitializer->clone() ) : nullptr );
}

void QgsEsriArcadeVariableDeclarationStatement::accept( QgsEsriArcadeVisitor &visitor ) const
{
  visitor.visit( *this );
}

//
// QgsEsriArcadeAssignmentStatement
//

QgsEsriArcadeAssignmentStatement::QgsEsriArcadeAssignmentStatement( AssignmentOperator assignmentOperator, QgsEsriArcadeExpression *target, QgsEsriArcadeExpression *value )
  : mTarget( target )
  , mValue( value )
  , mAssignmentOperator( assignmentOperator )
{}

QString QgsEsriArcadeAssignmentStatement::dump() const
{
  return u"%1 %2 %3;"_s.arg( mTarget ? mTarget->dump() : QString(), ASSIGNMENT_OPERATOR_TEXT[static_cast<int>( mAssignmentOperator )], mValue ? mValue->dump() : QString() );
}

QgsEsriArcadeNode *QgsEsriArcadeAssignmentStatement::clone() const
{
  QgsEsriArcadeExpression *clonedTarget = mTarget ? dynamic_cast<QgsEsriArcadeExpression *>( mTarget->clone() ) : nullptr;
  QgsEsriArcadeExpression *clonedValue = mValue ? dynamic_cast<QgsEsriArcadeExpression *>( mValue->clone() ) : nullptr;
  return new QgsEsriArcadeAssignmentStatement( mAssignmentOperator, clonedTarget, clonedValue );
}

void QgsEsriArcadeAssignmentStatement::accept( QgsEsriArcadeVisitor &visitor ) const
{
  visitor.visit( *this );
}

//
// QgsEsriArcadeReturnStatement
//

QgsEsriArcadeReturnStatement::QgsEsriArcadeReturnStatement( QgsEsriArcadeExpression *expression )
  : mExpression( expression )
{}

QString QgsEsriArcadeReturnStatement::dump() const
{
  if ( mExpression )
    return u"return %1;"_s.arg( mExpression->dump() );
  else
    return u"return;"_s;
}

QgsEsriArcadeNode *QgsEsriArcadeReturnStatement::clone() const
{
  return new QgsEsriArcadeReturnStatement( mExpression ? dynamic_cast<QgsEsriArcadeExpression *>( mExpression->clone() ) : nullptr );
}

void QgsEsriArcadeReturnStatement::accept( QgsEsriArcadeVisitor &visitor ) const
{
  visitor.visit( *this );
}

//
// QgsEsriArcadeFunctionDeclarationStatement
//

QgsEsriArcadeFunctionDeclarationStatement::QgsEsriArcadeFunctionDeclarationStatement( const QString &functionName, const QStringList &parameters, QgsEsriArcadeBlockStatement *body )
  : mFunctionName( functionName )
  , mParameters( parameters )
  , mBody( body )
{}

QString QgsEsriArcadeFunctionDeclarationStatement::dump() const
{
  QString body;
  if ( mBody )
    body = mBody->dump();

  return u"function %1(%2) {\n%3\n}"_s.arg( mFunctionName, mParameters.join( ", "_L1 ), body );
}

QgsEsriArcadeNode *QgsEsriArcadeFunctionDeclarationStatement::clone() const
{
  return new QgsEsriArcadeFunctionDeclarationStatement( mFunctionName, mParameters, mBody ? dynamic_cast<QgsEsriArcadeBlockStatement *>( mBody->clone() ) : nullptr );
}

void QgsEsriArcadeFunctionDeclarationStatement::accept( QgsEsriArcadeVisitor &visitor ) const
{
  visitor.visit( *this );
}

//
// QgsEsriArcadeExpressionStatement
//

QgsEsriArcadeExpressionStatement::QgsEsriArcadeExpressionStatement( QgsEsriArcadeExpression *expression )
  : mExpression( expression )
{}

QString QgsEsriArcadeExpressionStatement::dump() const
{
  if ( mExpression )
    return u"%1;"_s.arg( mExpression->dump() );
  else
    return u";"_s;
}

QgsEsriArcadeNode *QgsEsriArcadeExpressionStatement::clone() const
{
  return new QgsEsriArcadeExpressionStatement( mExpression ? dynamic_cast<QgsEsriArcadeExpression *>( mExpression->clone() ) : nullptr );
}

void QgsEsriArcadeExpressionStatement::accept( QgsEsriArcadeVisitor &visitor ) const
{
  visitor.visit( *this );
}

//
// QgsEsriArcadeLiteralExpression
//

QgsEsriArcadeLiteralExpression::QgsEsriArcadeLiteralExpression( const QVariant &value )
  : mValue( value )
{}

QString QgsEsriArcadeLiteralExpression::valueAsString() const
{
  if ( QgsVariantUtils::isNull( mValue ) )
    return u"null"_s;

  switch ( mValue.userType() )
  {
    case QMetaType::Type::Int:
      return QString::number( mValue.toInt() );
    case QMetaType::Type::LongLong:
      return QString::number( mValue.toLongLong() );
    case QMetaType::Type::Double:
    {
      const double value = mValue.toDouble();
      if ( std::isnan( value ) )
        return u"NaN"_s;
      if ( std::isinf( value ) )
        return value < 0 ? u"-Infinity"_s : u"Infinity"_s;
      return qgsDoubleToString( value );
    }
    case QMetaType::Type::QString:
      return mValue.toString();
    case QMetaType::Type::Bool:
      return mValue.toBool() ? u"true"_s : u"false"_s;
    default:
      return mValue.toString();
  }
}

QString QgsEsriArcadeLiteralExpression::dump() const
{
  if ( QgsVariantUtils::isNull( mValue ) )
    return u"null"_s;

  if ( mValue.userType() == QMetaType::Type::QString )
    return QgsEsriArcadeScript::quotedString( mValue.toString() );

  return valueAsString();
}

QgsEsriArcadeNode *QgsEsriArcadeLiteralExpression::clone() const
{
  return new QgsEsriArcadeLiteralExpression( mValue );
}

void QgsEsriArcadeLiteralExpression::accept( QgsEsriArcadeVisitor &visitor ) const
{
  visitor.visit( *this );
}

//
// QgsEsriArcadeVariableReferenceExpression
//

QgsEsriArcadeVariableReferenceExpression::QgsEsriArcadeVariableReferenceExpression( const QString &name )
  : mName( name )
{}

QString QgsEsriArcadeVariableReferenceExpression::dump() const
{
  return mName;
}

QgsEsriArcadeNode *QgsEsriArcadeVariableReferenceExpression::clone() const
{
  return new QgsEsriArcadeVariableReferenceExpression( mName );
}

void QgsEsriArcadeVariableReferenceExpression::accept( QgsEsriArcadeVisitor &visitor ) const
{
  visitor.visit( *this );
}

//
// QgsEsriArcadeProfileVariableReferenceExpression
//

QgsEsriArcadeProfileVariableReferenceExpression::QgsEsriArcadeProfileVariableReferenceExpression( const QString &name )
  : mName( name )
{}

QString QgsEsriArcadeProfileVariableReferenceExpression::dump() const
{
  return mName;
}

QgsEsriArcadeNode *QgsEsriArcadeProfileVariableReferenceExpression::clone() const
{
  return new QgsEsriArcadeProfileVariableReferenceExpression( mName );
}

void QgsEsriArcadeProfileVariableReferenceExpression::accept( QgsEsriArcadeVisitor &visitor ) const
{
  visitor.visit( *this );
}

//
// QgsEsriArcadeMemberAccessExpression
//

QgsEsriArcadeMemberAccessExpression::QgsEsriArcadeMemberAccessExpression( QgsEsriArcadeExpression *containerExpression, const QString &memberName, bool isSafeAccess )
  : mContainerExpression( containerExpression )
  , mMemberName( memberName )
  , mIsSafeAccess( isSafeAccess )
{}

QString QgsEsriArcadeMemberAccessExpression::dump() const
{
  return u"%1%2%3"_s.arg( mContainerExpression ? mContainerExpression->dump() : QString(), mIsSafeAccess ? "?."_L1 : "."_L1, mMemberName );
}

QgsEsriArcadeNode *QgsEsriArcadeMemberAccessExpression::clone() const
{
  QgsEsriArcadeExpression *clonedContainer = mContainerExpression ? dynamic_cast<QgsEsriArcadeExpression *>( mContainerExpression->clone() ) : nullptr;
  return new QgsEsriArcadeMemberAccessExpression( clonedContainer, mMemberName, mIsSafeAccess );
}

void QgsEsriArcadeMemberAccessExpression::accept( QgsEsriArcadeVisitor &visitor ) const
{
  visitor.visit( *this );
}

//
// QgsEsriArcadeBinaryOperatorExpression
//

QgsEsriArcadeBinaryOperatorExpression::QgsEsriArcadeBinaryOperatorExpression( BinaryOperator binaryOperator, QgsEsriArcadeExpression *leftOperand, QgsEsriArcadeExpression *rightOperand )
  : mBinaryOperator( binaryOperator )
  , mLeftOperand( leftOperand )
  , mRightOperand( rightOperand )
{}

QString QgsEsriArcadeBinaryOperatorExpression::text() const
{
  return BINARY_OPERATOR_TEXT[static_cast<int>( mBinaryOperator )];
}

int QgsEsriArcadeBinaryOperatorExpression::precedence() const
{
  switch ( mBinaryOperator )
  {
    case BinaryOperator::LogicalOr:
      return 1;
    case BinaryOperator::LogicalAnd:
      return 2;
    case BinaryOperator::BitwiseOr:
      return 3;
    case BinaryOperator::BitwiseXor:
      return 4;
    case BinaryOperator::BitwiseAnd:
      return 5;
    case BinaryOperator::Equal:
    case BinaryOperator::NotEqual:
      return 6;
    case BinaryOperator::LessThan:
    case BinaryOperator::GreaterThan:
    case BinaryOperator::LessThanOrEqual:
    case BinaryOperator::GreaterThanOrEqual:
      return 7;
    case BinaryOperator::BitwiseLeftShift:
    case BinaryOperator::BitwiseSignedRightShift:
    case BinaryOperator::BitwiseUnsignedRightShift:
      return 8;
    case BinaryOperator::Plus:
    case BinaryOperator::Minus:
      return 9;
    case BinaryOperator::Multiply:
    case BinaryOperator::Divide:
    case BinaryOperator::Modulo:
      return 10;
  }
  return -1;
}

bool QgsEsriArcadeBinaryOperatorExpression::leftAssociative() const
{
  return true;
}

QString QgsEsriArcadeBinaryOperatorExpression::dump() const
{
  QgsEsriArcadeBinaryOperatorExpression *leftBinary = dynamic_cast<QgsEsriArcadeBinaryOperatorExpression *>( mLeftOperand.get() );
  QgsEsriArcadeBinaryOperatorExpression *rightBinary = dynamic_cast<QgsEsriArcadeBinaryOperatorExpression *>( mRightOperand.get() );

  const QString leftDump = leftBinary && ( leftBinary->precedence() < precedence() ) ? u"(%1)"_s.arg( mLeftOperand->dump() ) : mLeftOperand->dump();

  const QString rightDump = rightBinary && ( rightBinary->precedence() <= precedence() ) ? u"(%1)"_s.arg( mRightOperand->dump() ) : mRightOperand->dump();

  return u"%1 %2 %3"_s.arg( leftDump, text(), rightDump );
}

QgsEsriArcadeNode *QgsEsriArcadeBinaryOperatorExpression::clone() const
{
  QgsEsriArcadeExpression *leftClone = mLeftOperand ? dynamic_cast<QgsEsriArcadeExpression *>( mLeftOperand->clone() ) : nullptr;
  QgsEsriArcadeExpression *rightClone = mRightOperand ? dynamic_cast<QgsEsriArcadeExpression *>( mRightOperand->clone() ) : nullptr;
  return new QgsEsriArcadeBinaryOperatorExpression( mBinaryOperator, leftClone, rightClone );
}

void QgsEsriArcadeBinaryOperatorExpression::accept( QgsEsriArcadeVisitor &visitor ) const
{
  visitor.visit( *this );
}

//
// QgsEsriArcadeUnaryOperatorExpression
//

QgsEsriArcadeUnaryOperatorExpression::QgsEsriArcadeUnaryOperatorExpression( UnaryOperator unaryOperator, QgsEsriArcadeExpression *operand )
  : mUnaryOperator( unaryOperator )
  , mOperand( operand )
{}

QString QgsEsriArcadeUnaryOperatorExpression::text() const
{
  return UNARY_OPERATOR_TEXT[static_cast<int>( mUnaryOperator )];
}

QString QgsEsriArcadeUnaryOperatorExpression::dump() const
{
  if ( mUnaryOperator == UnaryOperator::PostIncrement || mUnaryOperator == UnaryOperator::PostDecrement )
    return u"%1%2"_s.arg( mOperand ? mOperand->dump() : QString(), text() );
  else
    return u"%1%2"_s.arg( text(), mOperand ? mOperand->dump() : QString() );
}

QgsEsriArcadeNode *QgsEsriArcadeUnaryOperatorExpression::clone() const
{
  return new QgsEsriArcadeUnaryOperatorExpression( mUnaryOperator, mOperand ? dynamic_cast<QgsEsriArcadeExpression *>( mOperand->clone() ) : nullptr );
}

void QgsEsriArcadeUnaryOperatorExpression::accept( QgsEsriArcadeVisitor &visitor ) const
{
  visitor.visit( *this );
}

//
// QgsEsriArcadeFunctionCallExpression
//

QgsEsriArcadeFunctionCallExpression::QgsEsriArcadeFunctionCallExpression( const QString &functionName, const QList<QgsEsriArcadeExpression *> &arguments )
  : mFunctionName( functionName )
  , mArguments( arguments )
{}

QgsEsriArcadeFunctionCallExpression::~QgsEsriArcadeFunctionCallExpression()
{
  qDeleteAll( mArguments );
}

QString QgsEsriArcadeFunctionCallExpression::dump() const
{
  QStringList argumentStrings;
  argumentStrings.reserve( mArguments.size() );
  for ( QgsEsriArcadeExpression *argument : std::as_const( mArguments ) )
  {
    argumentStrings.append( argument->dump() );
  }
  return u"%1(%2)"_s.arg( mFunctionName, argumentStrings.join( ", "_L1 ) );
}

QgsEsriArcadeNode *QgsEsriArcadeFunctionCallExpression::clone() const
{
  QList<QgsEsriArcadeExpression *> clonedArguments;
  clonedArguments.reserve( mArguments.size() );
  for ( QgsEsriArcadeExpression *argument : std::as_const( mArguments ) )
  {
    clonedArguments.append( dynamic_cast<QgsEsriArcadeExpression *>( argument->clone() ) );
  }
  return new QgsEsriArcadeFunctionCallExpression( mFunctionName, clonedArguments );
}

void QgsEsriArcadeFunctionCallExpression::accept( QgsEsriArcadeVisitor &visitor ) const
{
  visitor.visit( *this );
}

//
// QgsEsriArcadeIndexExpression
//

QgsEsriArcadeIndexExpression::QgsEsriArcadeIndexExpression( QgsEsriArcadeExpression *containerExpression, QgsEsriArcadeExpression *indexExpression )
  : mContainerExpression( containerExpression )
  , mIndexExpression( indexExpression )
{}

QString QgsEsriArcadeIndexExpression::dump() const
{
  return u"%1[%2]"_s.arg( mContainerExpression ? mContainerExpression->dump() : QString(), mIndexExpression ? mIndexExpression->dump() : QString() );
}

QgsEsriArcadeNode *QgsEsriArcadeIndexExpression::clone() const
{
  QgsEsriArcadeExpression *clonedContainer = mContainerExpression ? dynamic_cast<QgsEsriArcadeExpression *>( mContainerExpression->clone() ) : nullptr;
  QgsEsriArcadeExpression *clonedIndex = mIndexExpression ? dynamic_cast<QgsEsriArcadeExpression *>( mIndexExpression->clone() ) : nullptr;
  return new QgsEsriArcadeIndexExpression( clonedContainer, clonedIndex );
}

void QgsEsriArcadeIndexExpression::accept( QgsEsriArcadeVisitor &visitor ) const
{
  visitor.visit( *this );
}

//
// QgsEsriArcadeArrayLiteralExpression
//

QgsEsriArcadeArrayLiteralExpression::QgsEsriArcadeArrayLiteralExpression( const QList<QgsEsriArcadeExpression *> &elements )
  : mElements( elements )
{}

QgsEsriArcadeArrayLiteralExpression::~QgsEsriArcadeArrayLiteralExpression()
{
  qDeleteAll( mElements );
}

QString QgsEsriArcadeArrayLiteralExpression::dump() const
{
  QStringList elementStrings;
  elementStrings.reserve( mElements.size() );
  for ( QgsEsriArcadeExpression *element : std::as_const( mElements ) )
  {
    elementStrings.append( element->dump() );
  }
  return u"[%1]"_s.arg( elementStrings.join( ", "_L1 ) );
}

QgsEsriArcadeNode *QgsEsriArcadeArrayLiteralExpression::clone() const
{
  QList<QgsEsriArcadeExpression *> clonedElements;
  clonedElements.reserve( mElements.size() );
  for ( QgsEsriArcadeExpression *element : std::as_const( mElements ) )
  {
    clonedElements.append( dynamic_cast<QgsEsriArcadeExpression *>( element->clone() ) );
  }
  return new QgsEsriArcadeArrayLiteralExpression( clonedElements );
}

void QgsEsriArcadeArrayLiteralExpression::accept( QgsEsriArcadeVisitor &visitor ) const
{
  visitor.visit( *this );
}

//
// QgsEsriArcadeDictionaryElement
//

QgsEsriArcadeDictionaryElement::QgsEsriArcadeDictionaryElement( const QString &key, QgsEsriArcadeExpression *value, bool isAutoKey )
  : mKey( key )
  , mValue( value )
  , mIsAutoKey( isAutoKey )
{}

QgsEsriArcadeDictionaryElement::~QgsEsriArcadeDictionaryElement() = default;

QString QgsEsriArcadeDictionaryElement::dump() const
{
  if ( mIsAutoKey )
    return mKey;

  return u"%1: %2"_s.arg( QgsEsriArcadeScript::quotedString( mKey ), mValue ? mValue->dump() : QString() );
}

QgsEsriArcadeDictionaryElement *QgsEsriArcadeDictionaryElement::clone() const
{
  QgsEsriArcadeExpression *clonedValue = mValue ? dynamic_cast<QgsEsriArcadeExpression *>( mValue->clone() ) : nullptr;
  return new QgsEsriArcadeDictionaryElement( mKey, clonedValue, mIsAutoKey );
}

//
// QgsEsriArcadeDictionaryLiteralExpression
//

QgsEsriArcadeDictionaryLiteralExpression::QgsEsriArcadeDictionaryLiteralExpression( const QList<QgsEsriArcadeDictionaryElement *> &elements )
  : mElements( elements )
{}

QgsEsriArcadeDictionaryLiteralExpression::~QgsEsriArcadeDictionaryLiteralExpression()
{
  qDeleteAll( mElements );
}

QString QgsEsriArcadeDictionaryLiteralExpression::dump() const
{
  if ( mElements.isEmpty() )
    return u"{}"_s;

  QStringList elementStrings;
  elementStrings.reserve( mElements.size() );
  for ( QgsEsriArcadeDictionaryElement *element : std::as_const( mElements ) )
  {
    elementStrings.append( element->dump() );
  }
  return u"{ %1 }"_s.arg( elementStrings.join( ", "_L1 ) );
}

QgsEsriArcadeNode *QgsEsriArcadeDictionaryLiteralExpression::clone() const
{
  QList<QgsEsriArcadeDictionaryElement *> clonedElements;
  clonedElements.reserve( mElements.size() );
  for ( QgsEsriArcadeDictionaryElement *element : std::as_const( mElements ) )
  {
    clonedElements.append( element->clone() );
  }
  return new QgsEsriArcadeDictionaryLiteralExpression( clonedElements );
}

void QgsEsriArcadeDictionaryLiteralExpression::accept( QgsEsriArcadeVisitor &visitor ) const
{
  visitor.visit( *this );
}


//
// QgsEsriArcadeIfStatement
//

QgsEsriArcadeIfStatement::QgsEsriArcadeIfStatement( QgsEsriArcadeExpression *condition, QgsEsriArcadeBlockStatement *trueBlock, QgsEsriArcadeBlockStatement *falseBlock )
  : mCondition( condition )
  , mTrueBlock( trueBlock )
  , mFalseBlock( falseBlock )
{}

QString QgsEsriArcadeIfStatement::dump() const
{
  QString result = u"if (%1) {\n%2}"_s.arg( mCondition ? mCondition->dump() : QString(), mTrueBlock ? mTrueBlock->dump() : QString() );
  if ( mFalseBlock )
  {
    if ( mFalseBlock->nodeType() == NodeType::IfStatement )
      result += u" else %1"_s.arg( mFalseBlock->dump() );
    else
      result += u" else {\n%1}"_s.arg( mFalseBlock->dump() );
  }
  return result;
}

QgsEsriArcadeNode *QgsEsriArcadeIfStatement::clone() const
{
  QgsEsriArcadeExpression *clonedCondition = mCondition ? dynamic_cast<QgsEsriArcadeExpression *>( mCondition->clone() ) : nullptr;
  QgsEsriArcadeBlockStatement *clonedTrueBlock = mTrueBlock ? dynamic_cast<QgsEsriArcadeBlockStatement *>( mTrueBlock->clone() ) : nullptr;
  QgsEsriArcadeBlockStatement *clonedFalseBlock = mFalseBlock ? dynamic_cast<QgsEsriArcadeBlockStatement *>( mFalseBlock->clone() ) : nullptr;
  return new QgsEsriArcadeIfStatement( clonedCondition, clonedTrueBlock, clonedFalseBlock );
}

void QgsEsriArcadeIfStatement::accept( QgsEsriArcadeVisitor &visitor ) const
{
  visitor.visit( *this );
}

//
// QgsEsriArcadeForStatement
//

QgsEsriArcadeForStatement::QgsEsriArcadeForStatement( QgsEsriArcadeStatement *initialization, QgsEsriArcadeExpression *condition, QgsEsriArcadeNode *step, QgsEsriArcadeBlockStatement *body )
  : mInitialization( initialization )
  , mCondition( condition )
  , mStep( step )
  , mBody( body )
{}

QString QgsEsriArcadeForStatement::dump() const
{
  QString initDump;
  if ( mInitialization )
  {
    initDump = mInitialization->dump().trimmed();
    if ( initDump.endsWith( ';'_L1 ) )
      initDump.chop( 1 );
  }

  return u"for (%1; %2; %3) {\n%4}"_s.arg( initDump, mCondition ? mCondition->dump() : QString(), mStep ? mStep->dump() : QString(), mBody ? mBody->dump() : QString() );
}

QgsEsriArcadeNode *QgsEsriArcadeForStatement::clone() const
{
  QgsEsriArcadeStatement *clonedInit = mInitialization ? dynamic_cast<QgsEsriArcadeStatement *>( mInitialization->clone() ) : nullptr;
  QgsEsriArcadeExpression *clonedCondition = mCondition ? dynamic_cast<QgsEsriArcadeExpression *>( mCondition->clone() ) : nullptr;
  QgsEsriArcadeNode *clonedStep = mStep ? mStep->clone() : nullptr;
  QgsEsriArcadeBlockStatement *clonedBody = mBody ? dynamic_cast<QgsEsriArcadeBlockStatement *>( mBody->clone() ) : nullptr;
  return new QgsEsriArcadeForStatement( clonedInit, clonedCondition, clonedStep, clonedBody );
}

void QgsEsriArcadeForStatement::accept( QgsEsriArcadeVisitor &visitor ) const
{
  visitor.visit( *this );
}

//
// QgsEsriArcadeForInStatement
//

QgsEsriArcadeForInStatement::QgsEsriArcadeForInStatement(
  IterationType iterationType, const QString &variableName, bool isVariableDeclaration, QgsEsriArcadeExpression *collection, QgsEsriArcadeBlockStatement *body
)
  : mIterationType( iterationType )
  , mVariableName( variableName )
  , mIsVariableDeclaration( isVariableDeclaration )
  , mCollection( collection )
  , mBody( body )
{}

QString QgsEsriArcadeForInStatement::dump() const
{
  const QString keyword = mIterationType == IterationType::ForIn ? u"in"_s : u"of"_s;
  return u"for (%1%2 %3 %4) {\n%5}"_s.arg( mIsVariableDeclaration ? "var "_L1 : ""_L1, mVariableName, keyword, mCollection ? mCollection->dump() : QString(), mBody ? mBody->dump() : QString() );
}

QgsEsriArcadeNode *QgsEsriArcadeForInStatement::clone() const
{
  QgsEsriArcadeExpression *clonedCollection = mCollection ? dynamic_cast<QgsEsriArcadeExpression *>( mCollection->clone() ) : nullptr;
  QgsEsriArcadeBlockStatement *clonedBody = mBody ? dynamic_cast<QgsEsriArcadeBlockStatement *>( mBody->clone() ) : nullptr;
  return new QgsEsriArcadeForInStatement( mIterationType, mVariableName, mIsVariableDeclaration, clonedCollection, clonedBody );
}

void QgsEsriArcadeForInStatement::accept( QgsEsriArcadeVisitor &visitor ) const
{
  visitor.visit( *this );
}

//
// QgsEsriArcadeWhileStatement
//

QgsEsriArcadeWhileStatement::QgsEsriArcadeWhileStatement( QgsEsriArcadeExpression *condition, QgsEsriArcadeBlockStatement *body )
  : mCondition( condition )
  , mBody( body )
{}

QString QgsEsriArcadeWhileStatement::dump() const
{
  return u"while (%1) {\n%2}"_s.arg( mCondition ? mCondition->dump() : QString(), mBody ? mBody->dump() : QString() );
}

QgsEsriArcadeNode *QgsEsriArcadeWhileStatement::clone() const
{
  QgsEsriArcadeExpression *clonedCondition = mCondition ? dynamic_cast<QgsEsriArcadeExpression *>( mCondition->clone() ) : nullptr;
  QgsEsriArcadeBlockStatement *clonedBody = mBody ? dynamic_cast<QgsEsriArcadeBlockStatement *>( mBody->clone() ) : nullptr;
  return new QgsEsriArcadeWhileStatement( clonedCondition, clonedBody );
}

void QgsEsriArcadeWhileStatement::accept( QgsEsriArcadeVisitor &visitor ) const
{
  visitor.visit( *this );
}

//
// QgsEsriArcadeBreakStatement
//

QString QgsEsriArcadeBreakStatement::dump() const
{
  return u"break;"_s;
}

QgsEsriArcadeNode *QgsEsriArcadeBreakStatement::clone() const
{
  return new QgsEsriArcadeBreakStatement();
}

void QgsEsriArcadeBreakStatement::accept( QgsEsriArcadeVisitor &visitor ) const
{
  visitor.visit( *this );
}

//
// QgsEsriArcadeContinueStatement
//

QString QgsEsriArcadeContinueStatement::dump() const
{
  return u"continue;"_s;
}

QgsEsriArcadeNode *QgsEsriArcadeContinueStatement::clone() const
{
  return new QgsEsriArcadeContinueStatement();
}

void QgsEsriArcadeContinueStatement::accept( QgsEsriArcadeVisitor &visitor ) const
{
  visitor.visit( *this );
}


//
// QgsEsriArcadeRecursiveVisitor
//

void QgsEsriArcadeRecursiveVisitor::visit( const QgsEsriArcadeBlockStatement &node )
{
  const QList<QgsEsriArcadeStatement *> statements = node.statements();
  for ( QgsEsriArcadeStatement *statement : statements )
  {
    statement->accept( *this );
  }
}

void QgsEsriArcadeRecursiveVisitor::visit( const QgsEsriArcadeVariableDeclarationStatement &node )
{
  if ( node.initializer() )
    node.initializer()->accept( *this );
}

void QgsEsriArcadeRecursiveVisitor::visit( const QgsEsriArcadeAssignmentStatement &node )
{
  if ( node.target() )
    node.target()->accept( *this );
  if ( node.value() )
    node.value()->accept( *this );
}

void QgsEsriArcadeRecursiveVisitor::visit( const QgsEsriArcadeReturnStatement &node )
{
  if ( node.expression() )
    node.expression()->accept( *this );
}

void QgsEsriArcadeRecursiveVisitor::visit( const QgsEsriArcadeFunctionDeclarationStatement &node )
{
  if ( node.body() )
    node.body()->accept( *this );
}

void QgsEsriArcadeRecursiveVisitor::visit( const QgsEsriArcadeExpressionStatement &node )
{
  if ( node.expression() )
    node.expression()->accept( *this );
}

void QgsEsriArcadeRecursiveVisitor::visit( const QgsEsriArcadeLiteralExpression & )
{}

void QgsEsriArcadeRecursiveVisitor::visit( const QgsEsriArcadeVariableReferenceExpression & )
{}

void QgsEsriArcadeRecursiveVisitor::visit( const QgsEsriArcadeProfileVariableReferenceExpression & )
{}

void QgsEsriArcadeRecursiveVisitor::visit( const QgsEsriArcadeMemberAccessExpression &node )
{
  if ( node.containerExpression() )
    node.containerExpression()->accept( *this );
}

void QgsEsriArcadeRecursiveVisitor::visit( const QgsEsriArcadeBinaryOperatorExpression &node )
{
  if ( node.leftOperand() )
    node.leftOperand()->accept( *this );
  if ( node.rightOperand() )
    node.rightOperand()->accept( *this );
}

void QgsEsriArcadeRecursiveVisitor::visit( const QgsEsriArcadeUnaryOperatorExpression &node )
{
  if ( node.operand() )
    node.operand()->accept( *this );
}

void QgsEsriArcadeRecursiveVisitor::visit( const QgsEsriArcadeFunctionCallExpression &node )
{
  const QList<QgsEsriArcadeExpression *> arguments = node.arguments();
  for ( QgsEsriArcadeExpression *argument : arguments )
  {
    argument->accept( *this );
  }
}

void QgsEsriArcadeRecursiveVisitor::visit( const QgsEsriArcadeIndexExpression &node )
{
  if ( node.containerExpression() )
    node.containerExpression()->accept( *this );
  if ( node.indexExpression() )
    node.indexExpression()->accept( *this );
}

void QgsEsriArcadeRecursiveVisitor::visit( const QgsEsriArcadeArrayLiteralExpression &node )
{
  const QList<QgsEsriArcadeExpression *> elements = node.elements();
  for ( QgsEsriArcadeExpression *element : elements )
  {
    element->accept( *this );
  }
}

void QgsEsriArcadeRecursiveVisitor::visit( const QgsEsriArcadeDictionaryLiteralExpression &node )
{
  const QList<QgsEsriArcadeDictionaryElement *> elements = node.elements();
  for ( QgsEsriArcadeDictionaryElement *element : elements )
  {
    if ( element->value() )
      element->value()->accept( *this );
  }
}

void QgsEsriArcadeRecursiveVisitor::visit( const QgsEsriArcadeIfStatement &node )
{
  if ( node.condition() )
    node.condition()->accept( *this );
  if ( node.trueBlock() )
    node.trueBlock()->accept( *this );
  if ( node.falseBlock() )
    node.falseBlock()->accept( *this );
}

void QgsEsriArcadeRecursiveVisitor::visit( const QgsEsriArcadeForStatement &node )
{
  if ( node.initialization() )
    node.initialization()->accept( *this );
  if ( node.condition() )
    node.condition()->accept( *this );
  if ( node.step() )
    node.step()->accept( *this );
  if ( node.body() )
    node.body()->accept( *this );
}

void QgsEsriArcadeRecursiveVisitor::visit( const QgsEsriArcadeForInStatement &node )
{
  if ( node.collection() )
    node.collection()->accept( *this );
  if ( node.body() )
    node.body()->accept( *this );
}

void QgsEsriArcadeRecursiveVisitor::visit( const QgsEsriArcadeWhileStatement &node )
{
  if ( node.condition() )
    node.condition()->accept( *this );
  if ( node.body() )
    node.body()->accept( *this );
}

void QgsEsriArcadeRecursiveVisitor::visit( const QgsEsriArcadeBreakStatement & )
{}

void QgsEsriArcadeRecursiveVisitor::visit( const QgsEsriArcadeContinueStatement & )
{}

//
// QgsEsriArcadeScript
//

QgsEsriArcadeScript::QgsEsriArcadeScript( const QString &scriptText )
  : mScript( scriptText )
{
  if ( !scriptText.isEmpty() )
  {
    mRootBlock.reset( parseArcadeScript( scriptText, mParserErrorString ) );
  }
}

QgsEsriArcadeScript::QgsEsriArcadeScript( const QgsEsriArcadeScript &other )
  : mScript( other.mScript )
  , mParserErrorString( other.mParserErrorString )
{
  if ( other.mRootBlock )
  {
    mRootBlock.reset( dynamic_cast<QgsEsriArcadeBlockStatement *>( other.mRootBlock->clone() ) );
  }
}

QgsEsriArcadeScript &QgsEsriArcadeScript::operator=( const QgsEsriArcadeScript &other )
{
  if ( this != &other )
  {
    mScript = other.mScript;
    mParserErrorString = other.mParserErrorString;
    if ( other.mRootBlock )
    {
      mRootBlock.reset( dynamic_cast<QgsEsriArcadeBlockStatement *>( other.mRootBlock->clone() ) );
    }
    else
    {
      mRootBlock.reset();
    }
  }
  return *this;
}

QgsEsriArcadeScript::~QgsEsriArcadeScript() = default;

bool QgsEsriArcadeScript::hasParserError() const
{
  return !mParserErrorString.isEmpty() || ( !mScript.trimmed().isEmpty() && !mRootBlock );
}

QString QgsEsriArcadeScript::parserErrorString() const
{
  return mParserErrorString;
}

const QgsEsriArcadeBlockStatement *QgsEsriArcadeScript::rootBlock() const
{
  return mRootBlock.get();
}

QString QgsEsriArcadeScript::script() const
{
  if ( !mScript.isEmpty() )
    return mScript;

  return dump();
}

QString QgsEsriArcadeScript::dump() const
{
  if ( !mRootBlock )
    return QString();

  return mRootBlock->dump();
}

void QgsEsriArcadeScript::acceptVisitor( QgsEsriArcadeVisitor &visitor ) const
{
  if ( mRootBlock )
    mRootBlock->accept( visitor );
}

QString QgsEsriArcadeScript::quotedString( const QString &text )
{
  QString escaped = text;
  escaped.replace( '\\'_L1, "\\\\"_L1 );
  escaped.replace( '\"'_L1, "\\\""_L1 );
  escaped.replace( '\n'_L1, "\\n"_L1 );
  escaped.replace( '\t'_L1, "\\t"_L1 );
  escaped.replace( '\r'_L1, "\\r"_L1 );
  return u"\"%1\""_s.arg( escaped );
}
