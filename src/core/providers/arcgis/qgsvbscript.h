/***************************************************************************
    qgsvbscript.h
    --------------------
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

#ifndef QGSVBSCRIPT_H
#define QGSVBSCRIPT_H

#include <memory>

#include "qgis_core.h"
#include "qgis_sip.h"

#include <QCoreApplication>
#include <QList>
#include <QMetaType>
#include <QString>
#include <QStringList>
#include <QVariant>

using namespace Qt::StringLiterals;

class QgsVbScriptVisitor;
class QgsVbScriptStatement;
class QgsVbScriptBlockStatement;
class QgsVbScriptAssignmentStatement;
class QgsVbScriptArrayLiteralExpression;
class QgsVbScriptIfStatement;
class QgsVbScriptForStatement;
class QgsVbScriptForEachStatement;
class QgsVbScriptWhileStatement;
class QgsVbScriptExitStatement;

/**
 * \ingroup core
 * \brief Represents an abstract node for a VBScript script.
 *
 * VBScript node subclasses are either:
 *
 * - statements (inheriting QgsVbSriptStatement), such as a variable declaration, if statement, for loop.
 * - or expressions (inheriting QgsVbScriptExpression), which are a portion of a statement representing
 *   an expression. For example ``1+2``, ``Upper('A string')``, or literals like ``255`` or ``'a string'``.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsVbScriptNode
{
#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    switch ( sipCpp->nodeType() )
    {
      case QgsVbScriptNode::NodeType::BlockStatement:
        sipType = sipType_QgsVbScriptBlockStatement;
        break;
      case QgsVbScriptNode::NodeType::VariableDeclarationStatement:
        sipType = sipType_QgsVbScriptVariableDeclarationStatement;
        break;
      case QgsVbScriptNode::NodeType::AssignmentStatement:
        sipType = sipType_QgsVbScriptAssignmentStatement;
        break;
      case QgsVbScriptNode::NodeType::IfStatement:
        sipType = sipType_QgsVbScriptIfStatement;
        break;
      case QgsVbScriptNode::NodeType::ForStatement:
        sipType = sipType_QgsVbScriptForStatement;
        break;
      case QgsVbScriptNode::NodeType::ForEachStatement:
        sipType = sipType_QgsVbScriptForEachStatement;
        break;
      case QgsVbScriptNode::NodeType::WhileStatement:
        sipType = sipType_QgsVbScriptWhileStatement;
        break;
      case QgsVbScriptNode::NodeType::ExitStatement:
        sipType = sipType_QgsVbScriptExitStatement;
        break;
      case QgsVbScriptNode::NodeType::FunctionDeclarationStatement:
        sipType = sipType_QgsVbScriptFunctionDeclarationStatement;
        break;
      case QgsVbScriptNode::NodeType::ExpressionStatement:
        sipType = sipType_QgsVbScriptExpressionStatement;
        break;
      case QgsVbScriptNode::NodeType::LiteralExpression:
        sipType = sipType_QgsVbScriptLiteralExpression;
        break;
      case QgsVbScriptNode::NodeType::VariableReferenceExpression:
        sipType = sipType_QgsVbScriptVariableReferenceExpression;
        break;
      case QgsVbScriptNode::NodeType::MemberAccessExpression:
        sipType = sipType_QgsVbScriptMemberAccessExpression;
        break;
      case QgsVbScriptNode::NodeType::BinaryOperatorExpression:
        sipType = sipType_QgsVbScriptBinaryOperatorExpression;
        break;
      case QgsVbScriptNode::NodeType::UnaryOperatorExpression:
        sipType = sipType_QgsVbScriptUnaryOperatorExpression;
        break;
      case QgsVbScriptNode::NodeType::FunctionCallExpression:
        sipType = sipType_QgsVbScriptFunctionCallExpression;
        break;
      case QgsVbScriptNode::NodeType::ArrayLiteralExpression:
        sipType = sipType_QgsVbScriptArrayLiteralExpression;
        break;
      default:
        sipType = nullptr;
        break;
    }
  SIP_END
#endif

  public:
    /**
     * Node type identifier.
     */
    enum class NodeType
    {
      BlockStatement,               //!< Block statement (a list of statements grouped together in a block)
      VariableDeclarationStatement, //!< Variable declaration statement
      AssignmentStatement,          //!< Variable assignment statement
      FunctionDeclarationStatement, //!< Function declaration
      ExpressionStatement,          //!< Expression
      LiteralExpression,            //!< Literal
      VariableReferenceExpression,  //!< Reference to a standard variable
      MemberAccessExpression,       //!< Member access via bracket or notation (eg ``$feature.ATTRIBUTE`` or ``$feature["ATTRIBUTE"]``)
      BinaryOperatorExpression,     //!< Binary Operator
      UnaryOperatorExpression,      //!< Operator
      FunctionCallExpression,       //!< Function call
      IndexExpression,              //!< Index operator
      ArrayLiteralExpression,       //!< A literal array, where the element values may be expressions themselves (eg ``[1, 2, 3/4]``)
      IfStatement,                  //!< If statement
      ForStatement,                 //!< For loop
      ForEachStatement,             //!< For each loop
      WhileStatement,               //!< While loop
      ExitStatement,                //!< Exit loop statement
    };

    virtual ~QgsVbScriptNode() = default;

    /**
     * Returns the node type.
     */
    virtual QgsVbScriptNode::NodeType nodeType() const = 0;

    /**
     * Returns the formatted script representation of the node.
     *
     * This can be used to export a script node back to a valid VBScript string.
     */
    virtual QString dump() const = 0;

    /**
     * Creates a copy of the node.
     *
     * Caller takes ownership of the returned object.
     */
    virtual QgsVbScriptNode *clone() const = 0 SIP_FACTORY;

    /**
     * Accepts a \a visitor to traverse the node.
     */
    virtual void accept( QgsVbScriptVisitor &visitor ) const = 0;
};

/**
 * \ingroup core
 * \brief Base class for statement nodes in a VBScript script.
 *
 * Subclasses represent standalone statements, such as a variable declaration, if
 * statement, for loop, etc.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsVbScriptStatement : public QgsVbScriptNode
{
  public:
    ~QgsVbScriptStatement() override = default;
};

/**
 * \ingroup core
 * \brief Base class for expression nodes in a VBScript script.
 *
 * Subclasses represent expressions, which are a portion of a statement
 * representing an expression.
 *
 * For example ``1+2``, ``UCase('A string')``, or literals like
 * ``255`` or ``"a string"``.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsVbScriptExpression : public QgsVbScriptNode
{
  public:
    ~QgsVbScriptExpression() override = default;
};

/**
 * \ingroup core
 * \brief Represents a sequence of statements.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsVbScriptBlockStatement : public QgsVbScriptStatement
{
  public:
    QgsVbScriptBlockStatement() = default;

    /**
     * Constructor for QgsVbScriptBlockStatement with an existing statement list.
     *
     * Ownership of all \a statements is transferred to this block.
     */
    explicit QgsVbScriptBlockStatement( const QList<QgsVbScriptStatement *> &statements SIP_TRANSFER );

    ~QgsVbScriptBlockStatement() override;

    QgsVbScriptNode::NodeType nodeType() const override { return NodeType::BlockStatement; }
    QString dump() const override;
    QgsVbScriptNode *clone() const override SIP_FACTORY;
    void accept( QgsVbScriptVisitor &visitor ) const override;

    /**
     * Appends a statement to the block.
     *
     * Ownership of \a statement is transferred to this block.
     */
    void appendStatement( QgsVbScriptStatement *statement SIP_TRANSFER );

    /**
     * Returns the list of statements contained within this block.
     */
    QList<QgsVbScriptStatement *> statements() const { return mStatements; }

    /**
     * Returns the number of statements in this block.
     */
    int count() const { return mStatements.count(); }

  private:
    QgsVbScriptBlockStatement( const QgsVbScriptBlockStatement &other ) = delete;
    QgsVbScriptBlockStatement &operator=( const QgsVbScriptBlockStatement &other ) = delete;

#ifdef SIP_RUN
    QgsVbScriptBlockStatement( const QgsVbScriptBlockStatement &other );
#endif

    QList<QgsVbScriptStatement *> mStatements;
};

/**
 * \ingroup core
 * \brief Represents a variable declaration statement.
 *
 * For example ``var x = 3;``.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsVbScriptVariableDeclarationStatement : public QgsVbScriptStatement
{
  public:
    /**
     * Constructor for QgsVbScriptVariableDeclarationStatement.
     */
    explicit QgsVbScriptVariableDeclarationStatement( const QString &variableName );

    /**
     * Constructor for QgsVbScriptVariableDeclarationStatement.
     */
    explicit QgsVbScriptVariableDeclarationStatement( const QStringList &variableNames );

    QgsVbScriptNode::NodeType nodeType() const override { return NodeType::VariableDeclarationStatement; }
    QString dump() const override;
    QgsVbScriptNode *clone() const override SIP_FACTORY;
    void accept( QgsVbScriptVisitor &visitor ) const override;

    /**
     * Returns the declared variable names.
     */
    QStringList variableNames() const { return mVariableNames; }

  private:
    QgsVbScriptVariableDeclarationStatement( const QgsVbScriptVariableDeclarationStatement &other ) = delete;
    QgsVbScriptVariableDeclarationStatement &operator=( const QgsVbScriptVariableDeclarationStatement &other ) = delete;

#ifdef SIP_RUN
    QgsVbScriptVariableDeclarationStatement( const QgsVbScriptVariableDeclarationStatement &other );
#endif

    QStringList mVariableNames;
};

/**
 * \ingroup core
 * \brief Represents a variable reassignment statement.
 *
 * For example, ``age = "35 years";``.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsVbScriptAssignmentStatement : public QgsVbScriptStatement
{
  public:
    /**
     * Constructor for QgsVbScriptAssignmentStatement.
     *
     * Ownership of \a target and \a value is transferred to this statement.
     */
    QgsVbScriptAssignmentStatement( QgsVbScriptExpression *target SIP_TRANSFER, QgsVbScriptExpression *value SIP_TRANSFER, bool isSet = false );

    QgsVbScriptNode::NodeType nodeType() const override { return NodeType::AssignmentStatement; }
    QString dump() const override;
    QgsVbScriptNode *clone() const override SIP_FACTORY;
    void accept( QgsVbScriptVisitor &visitor ) const override;

    /**
     * Returns the target expression being assigned to.
     */
    QgsVbScriptExpression *target() const { return mTarget.get(); }

    /**
     * Returns the assigned value expression.
     */
    QgsVbScriptExpression *value() const { return mValue.get(); }

    /**
     * Returns TRUE if the assignment uses the Set keyword.
     */
    bool isSet() const { return mIsSet; }

  private:
    QgsVbScriptAssignmentStatement( const QgsVbScriptAssignmentStatement &other ) = delete;
    QgsVbScriptAssignmentStatement &operator=( const QgsVbScriptAssignmentStatement &other ) = delete;

#ifdef SIP_RUN
    QgsVbScriptAssignmentStatement( const QgsVbScriptAssignmentStatement &other );
#endif

    std::unique_ptr<QgsVbScriptExpression> mTarget;
    std::unique_ptr<QgsVbScriptExpression> mValue;
    bool mIsSet = false;
};

/**
 * \ingroup core
 * \brief Represents a function declaration statement.
 *
 * For example ``function foo(a, b)  ...  end function``.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsVbScriptFunctionDeclarationStatement : public QgsVbScriptStatement
{
  public:
    /**
     * Constructor for QgsVbScriptFunctionDeclarationStatement.
     *
     * Ownership of \a body is transferred to this statement.
     */
    QgsVbScriptFunctionDeclarationStatement( const QString &functionName, const QStringList &parameters, QgsVbScriptBlockStatement *body SIP_TRANSFER, bool isSub = false );

    QgsVbScriptNode::NodeType nodeType() const override { return NodeType::FunctionDeclarationStatement; }
    QString dump() const override;
    QgsVbScriptNode *clone() const override SIP_FACTORY;
    void accept( QgsVbScriptVisitor &visitor ) const override;

    /**
     * Returns the function name.
     */
    QString functionName() const { return mFunctionName; }

    /**
     * Returns the parameter names.
     */
    QStringList parameters() const { return mParameters; }

    /**
     * Returns the function body block.
     */
    QgsVbScriptBlockStatement *body() const { return mBody.get(); }

    /**
     * Returns TRUE if this is a Sub procedure rather than a Function.
     */
    bool isSub() const { return mIsSub; }

  private:
    QgsVbScriptFunctionDeclarationStatement( const QgsVbScriptFunctionDeclarationStatement &other ) = delete;
    QgsVbScriptFunctionDeclarationStatement &operator=( const QgsVbScriptFunctionDeclarationStatement &other ) = delete;

#ifdef SIP_RUN
    QgsVbScriptFunctionDeclarationStatement( const QgsVbScriptFunctionDeclarationStatement &other );
#endif

    QString mFunctionName;
    QStringList mParameters;
    std::unique_ptr<QgsVbScriptBlockStatement> mBody;
    bool mIsSub = false;
};

/**
 * \ingroup core
 * \brief Represents an expression evaluated as a statement.
 *
 * For example, a standalone ``variable * 2 + 3`` expression.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsVbScriptExpressionStatement : public QgsVbScriptStatement
{
  public:
    /**
     * Constructor for QgsVbScriptExpressionStatement.
     *
     * Ownership of \a expression is transferred to this statement.
     */
    explicit QgsVbScriptExpressionStatement( QgsVbScriptExpression *expression SIP_TRANSFER );

    QgsVbScriptNode::NodeType nodeType() const override { return NodeType::ExpressionStatement; }
    QString dump() const override;
    QgsVbScriptNode *clone() const override SIP_FACTORY;
    void accept( QgsVbScriptVisitor &visitor ) const override;

    /**
     * Returns the underlying expression.
     */
    QgsVbScriptExpression *expression() const { return mExpression.get(); }

  private:
    QgsVbScriptExpressionStatement( const QgsVbScriptExpressionStatement &other ) = delete;
    QgsVbScriptExpressionStatement &operator=( const QgsVbScriptExpressionStatement &other ) = delete;

#ifdef SIP_RUN
    QgsVbScriptExpressionStatement( const QgsVbScriptExpressionStatement &other );
#endif

    std::unique_ptr<QgsVbScriptExpression> mExpression;
};

/**
 * \ingroup core
 * \brief Represents a literal value (number, text string, boolean, or null).
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsVbScriptLiteralExpression : public QgsVbScriptExpression
{
  public:
    /**
     * Constructor for QgsVbScriptLiteralExpression.
     */
    explicit QgsVbScriptLiteralExpression( const QVariant &value );

    QgsVbScriptNode::NodeType nodeType() const override { return NodeType::LiteralExpression; }
    QString dump() const override;
    QgsVbScriptNode *clone() const override SIP_FACTORY;
    void accept( QgsVbScriptVisitor &visitor ) const override;

    /**
     * Returns the literal value.
     */
    QVariant value() const { return mValue; }

    /**
     * Returns a string representation of the literal value.
     */
    QString valueAsString() const;

  private:
    QVariant mValue;
};

/**
 * \ingroup core
 * \brief Represents a reference to a variable identifier.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsVbScriptVariableReferenceExpression : public QgsVbScriptExpression
{
  public:
    /**
     * Constructor for QgsVbScriptVariableReferenceExpression.
     */
    explicit QgsVbScriptVariableReferenceExpression( const QString &name );

    QgsVbScriptNode::NodeType nodeType() const override { return NodeType::VariableReferenceExpression; }
    QString dump() const override;
    QgsVbScriptNode *clone() const override SIP_FACTORY;
    void accept( QgsVbScriptVisitor &visitor ) const override;

    /**
     * Returns the variable identifier name.
     */
    QString name() const { return mName; }

  private:
    QString mName;
};

/**
 * \ingroup core
 * \brief Represents a member access expression using bracket notation.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsVbScriptMemberAccessExpression : public QgsVbScriptExpression
{
  public:
    /**
     * Constructor for QgsVbScriptMemberAccessExpression.
     *
     * Ownership of \a containerExpression is transferred to this expression.
     */
    QgsVbScriptMemberAccessExpression( QgsVbScriptExpression *containerExpression SIP_TRANSFER, const QString &memberName );

    QgsVbScriptNode::NodeType nodeType() const override { return NodeType::MemberAccessExpression; }
    QString dump() const override;
    QgsVbScriptNode *clone() const override SIP_FACTORY;
    void accept( QgsVbScriptVisitor &visitor ) const override;

    /**
     * Returns the container expression being accessed.
     */
    QgsVbScriptExpression *containerExpression() const { return mContainerExpression.get(); }

    /**
     * Returns the member attribute name.
     */
    QString memberName() const { return mMemberName; }

  private:
    QgsVbScriptMemberAccessExpression( const QgsVbScriptMemberAccessExpression &other ) = delete;
    QgsVbScriptMemberAccessExpression &operator=( const QgsVbScriptMemberAccessExpression &other ) = delete;

#ifdef SIP_RUN
    QgsVbScriptMemberAccessExpression( const QgsVbScriptMemberAccessExpression &other );
#endif

    std::unique_ptr<QgsVbScriptExpression> mContainerExpression;
    QString mMemberName;
};

/**
 * \ingroup core
 * \brief Represents a binary operation expression.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsVbScriptBinaryOperatorExpression : public QgsVbScriptExpression
{
  public:
    /**
     * Binary operator enumeration.
     */
    enum class BinaryOperator
    {
      Plus,               //!< Addition +
      Minus,              //!< Subtraction -
      Multiply,           //!< Multiplication *
      Divide,             //!< Division /
      IntegerDivide,      //!< Integer division
      Modulo,             //!< Modulo %
      Exponent,           //!< Exponential ^
      Concat,             //!< String concatenation &
      Equal,              //!< Equality test ==
      NotEqual,           //!< Not equal test !=
      LessThan,           //!< Less than <
      GreaterThan,        //!< Greater than >
      LessThanOrEqual,    //!< Less than or equal to <=
      GreaterThanOrEqual, //!< Greater than or equal to >=
      Is,                 //!< Is comparison
      LogicalAnd,         //!< Logical and
      LogicalOr,          //!< Logical or
      LogicalXor,         //!< Logical xor
      LogicalImp,         //!< Logical imp
      LogicalEqv,         //!< Logical eqv
    };

    //! \note not available in Python bindings
    static const char *BINARY_OPERATOR_TEXT[] SIP_SKIP;

    /**
     * Constructor for QgsVbScriptBinaryOperatorExpression.
     *
     * Ownership of \a leftOperand and \a rightOperand is transferred to this expression.
     */
    QgsVbScriptBinaryOperatorExpression( BinaryOperator binaryOperator, QgsVbScriptExpression *leftOperand SIP_TRANSFER, QgsVbScriptExpression *rightOperand SIP_TRANSFER );

    QgsVbScriptNode::NodeType nodeType() const override { return NodeType::BinaryOperatorExpression; }
    QString dump() const override;
    QgsVbScriptNode *clone() const override SIP_FACTORY;
    void accept( QgsVbScriptVisitor &visitor ) const override;

    /**
     * Returns the binary operator.
     */
    BinaryOperator binaryOperator() const { return mBinaryOperator; }

    /**
     * Returns the left-hand operand expression.
     */
    QgsVbScriptExpression *leftOperand() const { return mLeftOperand.get(); }

    /**
     * Returns the right-hand operand expression.
     */
    QgsVbScriptExpression *rightOperand() const { return mRightOperand.get(); }

    /**
     * Returns operator precedence value.
     */
    int precedence() const;

    /**
     * Returns TRUE if operator is left-associative.
     */
    bool leftAssociative() const;

    /**
     * Returns text representation of the operator.
     */
    QString text() const;

  private:
    QgsVbScriptBinaryOperatorExpression( const QgsVbScriptBinaryOperatorExpression &other ) = delete;
    QgsVbScriptBinaryOperatorExpression &operator=( const QgsVbScriptBinaryOperatorExpression &other ) = delete;

#ifdef SIP_RUN
    QgsVbScriptBinaryOperatorExpression( const QgsVbScriptBinaryOperatorExpression &other );
#endif

    BinaryOperator mBinaryOperator;
    std::unique_ptr<QgsVbScriptExpression> mLeftOperand;
    std::unique_ptr<QgsVbScriptExpression> mRightOperand;
};

/**
 * \ingroup core
 * \brief Represents a unary operation expression.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsVbScriptUnaryOperatorExpression : public QgsVbScriptExpression
{
  public:
    /**
     * Unary operator enumeration.
     */
    enum class UnaryOperator
    {
      Not,   //!< Logical Not
      Minus, //!< Minus operator, -
      Plus,  //!< Plus operator, +
    };

    //! \note not available in Python bindings
    static const char *UNARY_OPERATOR_TEXT[] SIP_SKIP;

    /**
     * Constructor for QgsVbScriptUnaryOperatorExpression.
     *
     * Ownership of \a operand is transferred to this expression.
     */
    QgsVbScriptUnaryOperatorExpression( UnaryOperator unaryOperator, QgsVbScriptExpression *operand SIP_TRANSFER );

    QgsVbScriptNode::NodeType nodeType() const override { return NodeType::UnaryOperatorExpression; }
    QString dump() const override;
    QgsVbScriptNode *clone() const override SIP_FACTORY;
    void accept( QgsVbScriptVisitor &visitor ) const override;

    /**
     * Returns the unary operator.
     */
    UnaryOperator unaryOperator() const { return mUnaryOperator; }

    /**
     * Returns the operand expression.
     */
    QgsVbScriptExpression *operand() const { return mOperand.get(); }

    /**
     * Returns text representation of the operator.
     */
    QString text() const;

  private:
    QgsVbScriptUnaryOperatorExpression( const QgsVbScriptUnaryOperatorExpression &other ) = delete;
    QgsVbScriptUnaryOperatorExpression &operator=( const QgsVbScriptUnaryOperatorExpression &other ) = delete;

#ifdef SIP_RUN
    QgsVbScriptUnaryOperatorExpression( const QgsVbScriptUnaryOperatorExpression &other );
#endif

    UnaryOperator mUnaryOperator;
    std::unique_ptr<QgsVbScriptExpression> mOperand;
};

/**
 * \ingroup core
 * \brief Represents a function call expression.
 *
 * For example ``UCase(value)``.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsVbScriptFunctionCallExpression : public QgsVbScriptExpression
{
  public:
    /**
     * Constructor for QgsVbScriptFunctionCallExpression.
     *
     * Ownership of \a targetExpression and all expressions in \a arguments is transferred to this node.
     */
    QgsVbScriptFunctionCallExpression( QgsVbScriptExpression *targetExpression SIP_TRANSFER, const QList<QgsVbScriptExpression *> &arguments SIP_TRANSFER );

    ~QgsVbScriptFunctionCallExpression() override;

    QgsVbScriptNode::NodeType nodeType() const override { return NodeType::FunctionCallExpression; }
    QString dump() const override;
    QgsVbScriptNode *clone() const override SIP_FACTORY;
    void accept( QgsVbScriptVisitor &visitor ) const override;

    /**
     * Returns the callee expression.
     */
    QgsVbScriptExpression *targetExpression() const { return mTargetExpression.get(); }

    /**
     * Returns the list of argument expressions.
     */
    QList<QgsVbScriptExpression *> arguments() const { return mArguments; }

  private:
    QgsVbScriptFunctionCallExpression( const QgsVbScriptFunctionCallExpression &other ) = delete;
    QgsVbScriptFunctionCallExpression &operator=( const QgsVbScriptFunctionCallExpression &other ) = delete;

#ifdef SIP_RUN
    QgsVbScriptFunctionCallExpression( const QgsVbScriptFunctionCallExpression &other );
#endif

    std::unique_ptr<QgsVbScriptExpression> mTargetExpression;
    QList<QgsVbScriptExpression *> mArguments;
};

/**
 * \ingroup core
 * \brief Represents an array or dictionary index subscript expression.
 *
 * For example ``values[index]``.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsVbScriptIndexExpression : public QgsVbScriptExpression
{
  public:
    /**
     * Constructor for QgsVbScriptIndexExpression.
     *
     * Ownership of \a containerExpression and \a indexExpression is transferred to this expression.
     */
    QgsVbScriptIndexExpression( QgsVbScriptExpression *containerExpression SIP_TRANSFER, QgsVbScriptExpression *indexExpression SIP_TRANSFER );

    QgsVbScriptNode::NodeType nodeType() const override { return NodeType::IndexExpression; }
    QString dump() const override;
    QgsVbScriptNode *clone() const override SIP_FACTORY;
    void accept( QgsVbScriptVisitor &visitor ) const override;

    /**
     * Returns the container expression being indexed.
     */
    QgsVbScriptExpression *containerExpression() const { return mContainerExpression.get(); }

    /**
     * Returns the index expression.
     */
    QgsVbScriptExpression *indexExpression() const { return mIndexExpression.get(); }

  private:
    QgsVbScriptIndexExpression( const QgsVbScriptIndexExpression &other ) = delete;
    QgsVbScriptIndexExpression &operator=( const QgsVbScriptIndexExpression &other ) = delete;

#ifdef SIP_RUN
    QgsVbScriptIndexExpression( const QgsVbScriptIndexExpression &other );
#endif

    std::unique_ptr<QgsVbScriptExpression> mContainerExpression;
    std::unique_ptr<QgsVbScriptExpression> mIndexExpression;
};

/**
 * \ingroup core
 * \brief Represents an array literal expression.
 *
 * Array elements may be expressions themselves.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsVbScriptArrayLiteralExpression : public QgsVbScriptExpression
{
  public:
    /**
      * Constructor for QgsVbScriptArrayLiteralExpression.
      *
      * Ownership of all expressions in \a elements is transferred to this node.
      */
    explicit QgsVbScriptArrayLiteralExpression( const QList<QgsVbScriptExpression *> &elements SIP_TRANSFER );

    ~QgsVbScriptArrayLiteralExpression() override;

    QgsVbScriptNode::NodeType nodeType() const override { return NodeType::ArrayLiteralExpression; }
    QString dump() const override;
    QgsVbScriptNode *clone() const override SIP_FACTORY;
    void accept( QgsVbScriptVisitor &visitor ) const override;

    /**
      * Returns the list of element expressions.
     */
    QList<QgsVbScriptExpression *> elements() const { return mElements; }

  private:
    QgsVbScriptArrayLiteralExpression( const QgsVbScriptArrayLiteralExpression &other ) = delete;
    QgsVbScriptArrayLiteralExpression &operator=( const QgsVbScriptArrayLiteralExpression &other ) = delete;

#ifdef SIP_RUN
    QgsVbScriptArrayLiteralExpression( const QgsVbScriptArrayLiteralExpression &other );
#endif

    QList<QgsVbScriptExpression *> mElements;
};

/**
 * \ingroup core
 * \brief Represents an if statement.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsVbScriptIfStatement : public QgsVbScriptStatement
{
  public:
    /**
     * Constructor for QgsVbScriptIfStatement.
     *
     * Ownership of \a condition, \a trueBlock, and \a falseBlock is transferred to this statement.
     */
    QgsVbScriptIfStatement(
      QgsVbScriptExpression *condition SIP_TRANSFER, QgsVbScriptBlockStatement *trueBlock SIP_TRANSFER, QgsVbScriptBlockStatement *falseBlock SIP_TRANSFER = nullptr, bool isInline = false
    );

    QgsVbScriptNode::NodeType nodeType() const override { return NodeType::IfStatement; }
    QString dump() const override;
    QgsVbScriptNode *clone() const override SIP_FACTORY;
    void accept( QgsVbScriptVisitor &visitor ) const override;

    /**
     * Returns the condition expression.
     */
    QgsVbScriptExpression *condition() const { return mCondition.get(); }

    /**
     * Returns the statement block executed when the condition evaluates to TRUE.
     */
    QgsVbScriptBlockStatement *trueBlock() const { return mTrueBlock.get(); }

    /**
     * Returns the statement block executed when the condition evaluates to FALSE, or NULLPTR if none.
     */
    QgsVbScriptBlockStatement *falseBlock() const { return mFalseBlock.get(); }

    /**
     * Returns TRUE if this statement was declared as a single-line inline If.
     */
    bool isInline() const { return mIsInline; }

  private:
    QgsVbScriptIfStatement( const QgsVbScriptIfStatement &other ) = delete;
    QgsVbScriptIfStatement &operator=( const QgsVbScriptIfStatement &other ) = delete;

#ifdef SIP_RUN
    QgsVbScriptIfStatement( const QgsVbScriptIfStatement &other );
#endif

    std::unique_ptr<QgsVbScriptExpression> mCondition;
    std::unique_ptr<QgsVbScriptBlockStatement> mTrueBlock;
    std::unique_ptr<QgsVbScriptBlockStatement> mFalseBlock;
    bool mIsInline = false;
};

/**
 * \ingroup core
 * \brief Represents a standard counter-based for loop.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsVbScriptForStatement : public QgsVbScriptStatement
{
  public:
    /**
     * Constructor for QgsVbScriptForStatement.
     *
     * Ownership of \a startExpression, \a endExpression, \a stepExpression, and \a body is transferred to this statement.
     */
    QgsVbScriptForStatement(
      const QString &variableName,
      QgsVbScriptExpression *startExpression SIP_TRANSFER,
      QgsVbScriptExpression *endExpression SIP_TRANSFER,
      QgsVbScriptExpression *stepExpression SIP_TRANSFER,
      QgsVbScriptBlockStatement *body SIP_TRANSFER
    );

    QgsVbScriptNode::NodeType nodeType() const override { return NodeType::ForStatement; }
    QString dump() const override;
    QgsVbScriptNode *clone() const override SIP_FACTORY;
    void accept( QgsVbScriptVisitor &visitor ) const override;

    /**
     * Returns the loop variable identifier name.
     */
    QString variableName() const { return mVariableName; }

    /**
     * Returns the initial counter expression.
     */
    QgsVbScriptExpression *startExpression() const { return mStartExpression.get(); }

    /**
     * Returns the terminal counter expression.
     */
    QgsVbScriptExpression *endExpression() const { return mEndExpression.get(); }

    /**
     * Returns the step increment expression, or NULLPTR if default.
     */
    QgsVbScriptExpression *stepExpression() const { return mStepExpression.get(); }

    /**
     * Returns the loop body block statement.
     */
    QgsVbScriptBlockStatement *body() const { return mBody.get(); }

  private:
    QgsVbScriptForStatement( const QgsVbScriptForStatement &other ) = delete;
    QgsVbScriptForStatement &operator=( const QgsVbScriptForStatement &other ) = delete;

#ifdef SIP_RUN
    QgsVbScriptForStatement( const QgsVbScriptForStatement &other );
#endif

    QString mVariableName;
    std::unique_ptr<QgsVbScriptExpression> mStartExpression;
    std::unique_ptr<QgsVbScriptExpression> mEndExpression;
    std::unique_ptr<QgsVbScriptExpression> mStepExpression;
    std::unique_ptr<QgsVbScriptBlockStatement> mBody;
};

/**
 * \ingroup core
 * \brief Represents a for ... each loop.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsVbScriptForEachStatement : public QgsVbScriptStatement
{
  public:
    /**
     * Constructor for QgsVbScriptForEachStatement.
     *
     * Ownership of \a collection and \a body is transferred to this statement.
     */
    QgsVbScriptForEachStatement( const QString &variableName, QgsVbScriptExpression *collection SIP_TRANSFER, QgsVbScriptBlockStatement *body SIP_TRANSFER );

    QgsVbScriptNode::NodeType nodeType() const override { return NodeType::ForEachStatement; }
    QString dump() const override;
    QgsVbScriptNode *clone() const override SIP_FACTORY;
    void accept( QgsVbScriptVisitor &visitor ) const override;

    /**
     * Returns the loop variable identifier name.
     */
    QString variableName() const { return mVariableName; }

    /**
     * Returns the collection expression being iterated over.
     */
    QgsVbScriptExpression *collection() const { return mCollection.get(); }

    /**
     * Returns the loop body block statement.
     */
    QgsVbScriptBlockStatement *body() const { return mBody.get(); }

  private:
    QgsVbScriptForEachStatement( const QgsVbScriptForEachStatement &other ) = delete;
    QgsVbScriptForEachStatement &operator=( const QgsVbScriptForEachStatement &other ) = delete;

#ifdef SIP_RUN
    QgsVbScriptForEachStatement( const QgsVbScriptForEachStatement &other );
#endif

    QString mVariableName;
    std::unique_ptr<QgsVbScriptExpression> mCollection;
    std::unique_ptr<QgsVbScriptBlockStatement> mBody;
};

/**
 * \ingroup core
 * \brief Represents a while loop statement.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsVbScriptWhileStatement : public QgsVbScriptStatement
{
  public:
    /**
     * Type of while or do loop.
     */
    enum class LoopType
    {
      WhileWend,   //!< While ... Wend loop
      DoWhile,     //!< Do While ... Loop
      DoUntil,     //!< Do Until ... Loop
      DoLoopWhile, //!< Do ... Loop While
      DoLoopUntil, //!< Do ... Loop Until
    };

    /**
     * Constructor for QgsVbScriptWhileStatement.
     *
     * Ownership of \a condition and \a body is transferred to this statement.
     */
    QgsVbScriptWhileStatement( LoopType loopType, QgsVbScriptExpression *condition SIP_TRANSFER, QgsVbScriptBlockStatement *body SIP_TRANSFER );

    QgsVbScriptNode::NodeType nodeType() const override { return NodeType::WhileStatement; }
    QString dump() const override;
    QgsVbScriptNode *clone() const override SIP_FACTORY;
    void accept( QgsVbScriptVisitor &visitor ) const override;

    /**
     * Returns the loop variant type.
     */
    LoopType loopType() const { return mLoopType; }

    /**
     * Returns the loop condition expression.
     */
    QgsVbScriptExpression *condition() const { return mCondition.get(); }

    /**
     * Returns the loop body block statement.
     */
    QgsVbScriptBlockStatement *body() const { return mBody.get(); }

  private:
    QgsVbScriptWhileStatement( const QgsVbScriptWhileStatement &other ) = delete;
    QgsVbScriptWhileStatement &operator=( const QgsVbScriptWhileStatement &other ) = delete;

#ifdef SIP_RUN
    QgsVbScriptWhileStatement( const QgsVbScriptWhileStatement &other );
#endif
    LoopType mLoopType = LoopType::WhileWend;
    std::unique_ptr<QgsVbScriptExpression> mCondition;
    std::unique_ptr<QgsVbScriptBlockStatement> mBody;
};

/**
 * \ingroup core
 * \brief Represents an Exit loop or Exit Sub/Function statement.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsVbScriptExitStatement : public QgsVbScriptStatement
{
  public:
    /**
   * Object to exit from.
   */
    enum class ExitType
    {
      ExitDo,       //!< Exit Do
      ExitFor,      //!< Exit For
      ExitFunction, //!< Exit Function
      ExitSub,      //!< Exit Sub
    };

    /**
   * Constructor for QgsVbScriptExitStatement.
   */
    explicit QgsVbScriptExitStatement( ExitType exitType );

    QgsVbScriptNode::NodeType nodeType() const override { return NodeType::ExitStatement; }
    QString dump() const override;
    QgsVbScriptNode *clone() const override SIP_FACTORY;
    void accept( QgsVbScriptVisitor &visitor ) const override;

    /**
   * Returns the exit target scope.
   */
    ExitType exitType() const { return mExitType; }

  private:
    ExitType mExitType = ExitType::ExitFunction;
};

/**
 * \ingroup core
 * \brief Abstract visitor interface for traversing VBScript abstract syntax trees.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsVbScriptVisitor
{
  public:
    virtual ~QgsVbScriptVisitor() = default;

    //! Visit a QgsVbScriptBlockStatement \a node
    virtual void visit( const QgsVbScriptBlockStatement &node ) = 0;
    //! Visit a QgsVbScriptVariableDeclarationStatement \a node
    virtual void visit( const QgsVbScriptVariableDeclarationStatement &node ) = 0;
    //! Visit a QgsVbScriptAssignmentStatement \a node
    virtual void visit( const QgsVbScriptAssignmentStatement &node ) = 0;
    //! Visit a QgsVbScriptFunctionDeclarationStatement \a node
    virtual void visit( const QgsVbScriptFunctionDeclarationStatement &node ) = 0;
    //! Visit a QgsVbScriptExpressionStatement \a node
    virtual void visit( const QgsVbScriptExpressionStatement &node ) = 0;
    //! Visit a QgsVbScriptLiteralExpression \a node
    virtual void visit( const QgsVbScriptLiteralExpression &node ) = 0;
    //! Visit a QgsVbScriptVariableReferenceExpression \a node
    virtual void visit( const QgsVbScriptVariableReferenceExpression &node ) = 0;
    //! Visit a QgsVbScriptMemberAccessExpression \a node
    virtual void visit( const QgsVbScriptMemberAccessExpression &node ) = 0;
    //! Visit a QgsVbScriptBinaryOperatorExpression \a node
    virtual void visit( const QgsVbScriptBinaryOperatorExpression &node ) = 0;
    //! Visit a QgsVbScriptUnaryOperatorExpression \a node
    virtual void visit( const QgsVbScriptUnaryOperatorExpression &node ) = 0;
    //! Visit a QgsVbScriptFunctionCallExpression \a node
    virtual void visit( const QgsVbScriptFunctionCallExpression &node ) = 0;
    //! Visit a QgsVbScriptIndexExpression \a node
    virtual void visit( const QgsVbScriptIndexExpression &node ) = 0;
    //! Visit a QgsVbScriptArrayLiteralExpression \a node
    virtual void visit( const QgsVbScriptArrayLiteralExpression &node ) = 0;
    //! Visit a QgsVbScriptIfStatement \a node
    virtual void visit( const QgsVbScriptIfStatement &node ) = 0;
    //! Visit a QgsVbScriptForStatement \a node
    virtual void visit( const QgsVbScriptForStatement &node ) = 0;
    //! Visit a QgsVbScriptForEachStatement \a node
    virtual void visit( const QgsVbScriptForEachStatement &node ) = 0;
    //! Visit a QgsVbScriptWhileStatement \a node
    virtual void visit( const QgsVbScriptWhileStatement &node ) = 0;
    //! Visit a QgsVbScriptExitStatement \a node
    virtual void visit( const QgsVbScriptExitStatement &node ) = 0;
};

/**
 * \ingroup core
 * \brief Visitor implementation that recursively traverses all child nodes in a VBScript script.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsVbScriptRecursiveVisitor : public QgsVbScriptVisitor
{
  public:
    QgsVbScriptRecursiveVisitor() = default;

    void visit( const QgsVbScriptBlockStatement &node ) override;
    void visit( const QgsVbScriptVariableDeclarationStatement &node ) override;
    void visit( const QgsVbScriptAssignmentStatement &node ) override;
    void visit( const QgsVbScriptFunctionDeclarationStatement &node ) override;
    void visit( const QgsVbScriptExpressionStatement &node ) override;
    void visit( const QgsVbScriptLiteralExpression &node ) override;
    void visit( const QgsVbScriptVariableReferenceExpression &node ) override;
    void visit( const QgsVbScriptMemberAccessExpression &node ) override;
    void visit( const QgsVbScriptBinaryOperatorExpression &node ) override;
    void visit( const QgsVbScriptUnaryOperatorExpression &node ) override;
    void visit( const QgsVbScriptFunctionCallExpression &node ) override;
    void visit( const QgsVbScriptIndexExpression &node ) override;
    void visit( const QgsVbScriptArrayLiteralExpression &node ) override;
    void visit( const QgsVbScriptIfStatement &node ) override;
    void visit( const QgsVbScriptForStatement &node ) override;
    void visit( const QgsVbScriptForEachStatement &node ) override;
    void visit( const QgsVbScriptWhileStatement &node ) override;
    void visit( const QgsVbScriptExitStatement &node ) override;
};

/**
 * \ingroup core
 * \brief Parses and encapsulates an entire VBScript script.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsVbScript
{
  public:
    /**
     * Creates a new script from \a scriptText.
     */
    explicit QgsVbScript( const QString &scriptText = QString() );

    QgsVbScript( const QgsVbScript &other );
    QgsVbScript &operator=( const QgsVbScript &other );

    ~QgsVbScript();

    /**
     * Returns TRUE if a parsing error occurred.
     */
    bool hasParserError() const;

    /**
     * Returns parser error description.
     */
    QString parserErrorString() const;

    /**
     * Returns the root block statement node, or NULLPTR if parsing failed.
     */
    const QgsVbScriptBlockStatement *rootBlock() const;

    /**
     * Returns the original script text.
     */
    QString script() const;

    /**
     * Dumps the abstract syntax tree back into an Arcade script string.
     */
    QString dump() const;

    /**
     * Accepts a visitor to traverse the script abstract syntax tree.
     */
    void acceptVisitor( QgsVbScriptVisitor &visitor ) const;

    /**
     * Returns a properly quoted string literal (enclosed in double quotes with escape sequences).
     */
    static QString quotedString( const QString &text );

  protected:
    std::unique_ptr<QgsVbScriptBlockStatement> mRootBlock;
    QString mScript;
    QString mParserErrorString;
};

#endif // QGSVBSCRIPT_H
