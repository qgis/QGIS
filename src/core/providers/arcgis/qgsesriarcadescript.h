/***************************************************************************
    qgsesriarcadescript.h
    --------------------
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

#ifndef QGSESRIARCADESCRIPT_H
#define QGSESRIARCADESCRIPT_H

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

class QgsEsriArcadeVisitor;
class QgsEsriArcadeStatement;
class QgsEsriArcadeBlockStatement;
class QgsEsriArcadeAssignmentStatement;
class QgsEsriArcadeArrayLiteralExpression;
class QgsEsriArcadeDictionaryElement;
class QgsEsriArcadeDictionaryLiteralExpression;
class QgsEsriArcadeIfStatement;
class QgsEsriArcadeForStatement;
class QgsEsriArcadeForInStatement;
class QgsEsriArcadeWhileStatement;
class QgsEsriArcadeBreakStatement;
class QgsEsriArcadeContinueStatement;

/**
 * \ingroup core
 * \brief Represents an abstract node for an Esri Arcade script.
 *
 * Arcade node subclasses are either:
 *
 * - statements (inheriting QgsEsriArcadeStatement), such as a variable declaration, if statement, for loop.
 * - or expressions (inheriting QgsEsriArcadeExpression), which are a portion of a statement representing
 *   an expression. For example ``1+2``, ``Upper('A string')``, or literals like ``255`` or ``'a string'``.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsEsriArcadeNode
{
#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    switch ( sipCpp->nodeType() )
    {
      case QgsEsriArcadeNode::NodeType::BlockStatement:
        sipType = sipType_QgsEsriArcadeBlockStatement;
        break;
      case QgsEsriArcadeNode::NodeType::VariableDeclarationStatement:
        sipType = sipType_QgsEsriArcadeVariableDeclarationStatement;
        break;
      case QgsEsriArcadeNode::NodeType::AssignmentStatement:
        sipType = sipType_QgsEsriArcadeAssignmentStatement;
        break;
      case QgsEsriArcadeNode::NodeType::ReturnStatement:
        sipType = sipType_QgsEsriArcadeReturnStatement;
        break;
      case QgsEsriArcadeNode::NodeType::FunctionDeclarationStatement:
        sipType = sipType_QgsEsriArcadeFunctionDeclarationStatement;
        break;
      case QgsEsriArcadeNode::NodeType::ExpressionStatement:
        sipType = sipType_QgsEsriArcadeExpressionStatement;
        break;
      case QgsEsriArcadeNode::NodeType::ArrayLiteralExpression:
        sipType = sipType_QgsEsriArcadeArrayLiteralExpression;
        break;
      case QgsEsriArcadeNode::NodeType::LiteralExpression:
        sipType = sipType_QgsEsriArcadeLiteralExpression;
        break;
      case QgsEsriArcadeNode::NodeType::VariableReferenceExpression:
        sipType = sipType_QgsEsriArcadeVariableReferenceExpression;
        break;
      case QgsEsriArcadeNode::NodeType::ProfileVariableReferenceExpression:
        sipType = sipType_QgsEsriArcadeProfileVariableReferenceExpression;
        break;
      case QgsEsriArcadeNode::NodeType::MemberAccessExpression:
        sipType = sipType_QgsEsriArcadeMemberAccessExpression;
        break;
      case QgsEsriArcadeNode::NodeType::BinaryOperatorExpression:
        sipType = sipType_QgsEsriArcadeBinaryOperatorExpression;
        break;
      case QgsEsriArcadeNode::NodeType::UnaryOperatorExpression:
        sipType = sipType_QgsEsriArcadeUnaryOperatorExpression;
        break;
      case QgsEsriArcadeNode::NodeType::FunctionCallExpression:
        sipType = sipType_QgsEsriArcadeFunctionCallExpression;
        break;
      case QgsEsriArcadeNode::NodeType::IndexExpression:
        sipType = sipType_QgsEsriArcadeIndexExpression;
        break;
      case QgsEsriArcadeNode::NodeType::DictionaryLiteralExpression:
        sipType = sipType_QgsEsriArcadeDictionaryLiteralExpression;
        break;
      case QgsEsriArcadeNode::NodeType::IfStatement:
        sipType = sipType_QgsEsriArcadeIfStatement;
        break;
      case QgsEsriArcadeNode::NodeType::ForStatement:
        sipType = sipType_QgsEsriArcadeForStatement;
        break;
      case QgsEsriArcadeNode::NodeType::ForInStatement:
        sipType = sipType_QgsEsriArcadeForInStatement;
        break;
      case QgsEsriArcadeNode::NodeType::WhileStatement:
        sipType = sipType_QgsEsriArcadeWhileStatement;
        break;
      case QgsEsriArcadeNode::NodeType::BreakStatement:
        sipType = sipType_QgsEsriArcadeBreakStatement;
        break;
      case QgsEsriArcadeNode::NodeType::ContinueStatement:
        sipType = sipType_QgsEsriArcadeContinueStatement;
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
      BlockStatement,                     //!< Block statement (a list of statements grouped together in a block)
      VariableDeclarationStatement,       //!< Variable declaration statement
      ReturnStatement,                    //!< Return statement
      AssignmentStatement,                //!< Variable assignment statement
      FunctionDeclarationStatement,       //!< Function declaration
      ExpressionStatement,                //!< Expression
      LiteralExpression,                  //!< Literal
      VariableReferenceExpression,        //!< Reference to a standard variable
      ProfileVariableReferenceExpression, //!< Reference to a profile variable (eg ``$feature``)
      MemberAccessExpression,             //!< Member access via bracket or notation (eg ``$feature.ATTRIBUTE`` or ``$feature["ATTRIBUTE"]``)
      BinaryOperatorExpression,           //!< Binary Operator
      UnaryOperatorExpression,            //!< Operator
      FunctionCallExpression,             //!< Function call
      IndexExpression,                    //!< Index operator
      ArrayLiteralExpression,             //!< A literal array, where the element values may be expressions themselves (eg ``[1, 2, 3/4]``)
      DictionaryLiteralExpression,        //!< A literal dictionary, eg ``{key: 1, other_key: 2}``
      IfStatement,                        //!< If statement
      ForStatement,                       //!< For loop
      ForInStatement,                     //!< For x in y loop
      WhileStatement,                     //!< While loop
      BreakStatement,                     //!< Break statement
      ContinueStatement,                  //!< Continue statement
    };

    virtual ~QgsEsriArcadeNode() = default;

    /**
     * Returns the node type.
     */
    virtual QgsEsriArcadeNode::NodeType nodeType() const = 0;

    /**
     * Returns the formatted script representation of the node.
     *
     * This can be used to export a script node back to a valid Esri arcade script string.
     */
    virtual QString dump() const = 0;

    /**
     * Creates a copy of the node.
     *
     * Caller takes ownership of the returned object.
     */
    virtual QgsEsriArcadeNode *clone() const = 0 SIP_FACTORY;

    /**
     * Accepts a \a visitor to traverse the node.
     */
    virtual void accept( QgsEsriArcadeVisitor &visitor ) const = 0;
};

/**
 * \ingroup core
 * \brief Base class for statement nodes in an Esri Arcade script.
 *
 * Subclasses represent standalone statements, such as a variable declaration, if
 * statement, for loop, etc.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsEsriArcadeStatement : public QgsEsriArcadeNode
{
  public:
    ~QgsEsriArcadeStatement() override = default;
};

/**
 * \ingroup core
 * \brief Base class for expression nodes in an Esri Arcade script.
 *
 * Subclasses represent expressions, which are a portion of a statement
 * representing an expression.
 *
 * For example ``1+2``, ``Upper('A string')``, or literals like
 * ``255`` or ``'a string'``.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsEsriArcadeExpression : public QgsEsriArcadeNode
{
  public:
    ~QgsEsriArcadeExpression() override = default;
};

/**
 * \ingroup core
 * \brief Represents a sequence or block of statements enclosed in braces.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsEsriArcadeBlockStatement : public QgsEsriArcadeStatement
{
  public:
    QgsEsriArcadeBlockStatement() = default;

    /**
     * Constructor for QgsEsriArcadeBlockStatement with an existing statement list.
     *
     * Ownership of all \a statements is transferred to this block.
     */
    explicit QgsEsriArcadeBlockStatement( const QList<QgsEsriArcadeStatement *> &statements SIP_TRANSFER );

    ~QgsEsriArcadeBlockStatement() override;

    QgsEsriArcadeNode::NodeType nodeType() const override { return NodeType::BlockStatement; }
    QString dump() const override;
    QgsEsriArcadeNode *clone() const override SIP_FACTORY;
    void accept( QgsEsriArcadeVisitor &visitor ) const override;

    /**
     * Appends a statement to the block.
     *
     * Ownership of \a statement is transferred to this block.
     */
    void appendStatement( QgsEsriArcadeStatement *statement SIP_TRANSFER );

    /**
     * Returns the list of statements contained within this block.
     */
    QList<QgsEsriArcadeStatement *> statements() const { return mStatements; }

    /**
     * Returns the number of statements in this block.
     */
    int count() const { return mStatements.count(); }

  private:
    QgsEsriArcadeBlockStatement( const QgsEsriArcadeBlockStatement &other ) = delete;
    QgsEsriArcadeBlockStatement &operator=( const QgsEsriArcadeBlockStatement &other ) = delete;

#ifdef SIP_RUN
    QgsEsriArcadeBlockStatement( const QgsEsriArcadeBlockStatement &other );
#endif

    QList<QgsEsriArcadeStatement *> mStatements;
};

/**
 * \ingroup core
 * \brief Represents a variable declaration statement.
 *
 * For example ``var x = 3;``.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsEsriArcadeVariableDeclarationStatement : public QgsEsriArcadeStatement
{
  public:
    /**
     * Constructor for QgsEsriArcadeVariableDeclarationStatement.
     *
     * Ownership of \a initializer is transferred to this statement.
     */
    explicit QgsEsriArcadeVariableDeclarationStatement( const QString &variableName, QgsEsriArcadeExpression *initializer SIP_TRANSFER = nullptr );

    QgsEsriArcadeNode::NodeType nodeType() const override { return NodeType::VariableDeclarationStatement; }
    QString dump() const override;
    QgsEsriArcadeNode *clone() const override SIP_FACTORY;
    void accept( QgsEsriArcadeVisitor &visitor ) const override;

    /**
     * Returns the declared variable name.
     *
     * For the statement ``var my_var = 3;`` this would return "my_var".
     */
    QString variableName() const { return mVariableName; }

    /**
     * Returns the optional initializer expression, or NULLPTR if uninitialized.
     *
     * For the statement ``var my_var = 3 * 2;`` this would return the ``3 * 2``
     * expression node.
     */
    QgsEsriArcadeExpression *initializer() const { return mInitializer.get(); }

  private:
    QgsEsriArcadeVariableDeclarationStatement( const QgsEsriArcadeVariableDeclarationStatement &other ) = delete;
    QgsEsriArcadeVariableDeclarationStatement &operator=( const QgsEsriArcadeVariableDeclarationStatement &other ) = delete;

#ifdef SIP_RUN
    QgsEsriArcadeVariableDeclarationStatement( const QgsEsriArcadeVariableDeclarationStatement &other );
#endif

    QString mVariableName;
    std::unique_ptr<QgsEsriArcadeExpression> mInitializer;
};

/**
 * \ingroup core
 * \brief Represents a variable reassignment statement.
 *
 * For example, ``age = "35 years";``.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsEsriArcadeAssignmentStatement : public QgsEsriArcadeStatement
{
  public:
    /**
     * Assignment operator types.
     */
    enum class AssignmentOperator
    {
      Assign,         //!< Standard assignment ``a = 10``
      PlusAssign,     //!< Add and assign ``a += 10``
      MinusAssign,    //!< Subtract and assign ``a -= 10``
      MultiplyAssign, //!< Multiply and assign ``a *= 10``
      DivideAssign,   //!< Divide and assign ``a /= 10``
    };

    //! \note not available in Python bindings
    static const char *ASSIGNMENT_OPERATOR_TEXT[] SIP_SKIP;

    /**
     * Constructor for QgsEsriArcadeAssignmentStatement.
     *
     * Ownership of \a target and \a value is transferred to this statement.
     */
    QgsEsriArcadeAssignmentStatement( AssignmentOperator assignmentOperator, QgsEsriArcadeExpression *target SIP_TRANSFER, QgsEsriArcadeExpression *value SIP_TRANSFER );

    QgsEsriArcadeNode::NodeType nodeType() const override { return NodeType::AssignmentStatement; }
    QString dump() const override;
    QgsEsriArcadeNode *clone() const override SIP_FACTORY;
    void accept( QgsEsriArcadeVisitor &visitor ) const override;

    /**
     * Returns the target expression being assigned to.
     */
    QgsEsriArcadeExpression *target() const { return mTarget.get(); }

    /**
     * Returns the assigned value expression.
     */
    QgsEsriArcadeExpression *value() const { return mValue.get(); }

    /**
     * Returns the assignment operator.
     */
    AssignmentOperator assignmentOperator() const { return mAssignmentOperator; }

  private:
    QgsEsriArcadeAssignmentStatement( const QgsEsriArcadeAssignmentStatement &other ) = delete;
    QgsEsriArcadeAssignmentStatement &operator=( const QgsEsriArcadeAssignmentStatement &other ) = delete;

#ifdef SIP_RUN
    QgsEsriArcadeAssignmentStatement( const QgsEsriArcadeAssignmentStatement &other );
#endif

    std::unique_ptr<QgsEsriArcadeExpression> mTarget;
    std::unique_ptr<QgsEsriArcadeExpression> mValue;
    AssignmentOperator mAssignmentOperator = AssignmentOperator::Assign;
};

/**
 * \ingroup core
 * \brief Represents a return statement.
 *
 * For example ``return x + y;``.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsEsriArcadeReturnStatement : public QgsEsriArcadeStatement
{
  public:
    /**
     * Constructor for QgsEsriArcadeReturnStatement.
     *
     * Ownership of \a expression is transferred to this statement.
     */
    explicit QgsEsriArcadeReturnStatement( QgsEsriArcadeExpression *expression SIP_TRANSFER = nullptr );

    QgsEsriArcadeNode::NodeType nodeType() const override { return NodeType::ReturnStatement; }
    QString dump() const override;
    QgsEsriArcadeNode *clone() const override SIP_FACTORY;
    void accept( QgsEsriArcadeVisitor &visitor ) const override;

    /**
     * Returns the expression returned by the statement, or NULLPTR if void return.
     */
    QgsEsriArcadeExpression *expression() const { return mExpression.get(); }

  private:
    QgsEsriArcadeReturnStatement( const QgsEsriArcadeReturnStatement &other ) = delete;
    QgsEsriArcadeReturnStatement &operator=( const QgsEsriArcadeReturnStatement &other ) = delete;

#ifdef SIP_RUN
    QgsEsriArcadeReturnStatement( const QgsEsriArcadeReturnStatement &other );
#endif

    std::unique_ptr<QgsEsriArcadeExpression> mExpression;
};

/**
 * \ingroup core
 * \brief Represents a function declaration statement.
 *
 * For example ``function foo(a, b) { ... })``.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsEsriArcadeFunctionDeclarationStatement : public QgsEsriArcadeStatement
{
  public:
    /**
     * Constructor for QgsEsriArcadeFunctionDeclarationStatement.
     *
     * Ownership of \a body is transferred to this statement.
     */
    QgsEsriArcadeFunctionDeclarationStatement( const QString &functionName, const QStringList &parameters, QgsEsriArcadeBlockStatement *body SIP_TRANSFER );

    QgsEsriArcadeNode::NodeType nodeType() const override { return NodeType::FunctionDeclarationStatement; }
    QString dump() const override;
    QgsEsriArcadeNode *clone() const override SIP_FACTORY;
    void accept( QgsEsriArcadeVisitor &visitor ) const override;

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
    QgsEsriArcadeBlockStatement *body() const { return mBody.get(); }

  private:
    QgsEsriArcadeFunctionDeclarationStatement( const QgsEsriArcadeFunctionDeclarationStatement &other ) = delete;
    QgsEsriArcadeFunctionDeclarationStatement &operator=( const QgsEsriArcadeFunctionDeclarationStatement &other ) = delete;

#ifdef SIP_RUN
    QgsEsriArcadeFunctionDeclarationStatement( const QgsEsriArcadeFunctionDeclarationStatement &other );
#endif

    QString mFunctionName;
    QStringList mParameters;
    std::unique_ptr<QgsEsriArcadeBlockStatement> mBody;
};

/**
 * \ingroup core
 * \brief Represents an expression evaluated as a statement.
 *
 * For example, a standalone ``variable * 2 + 3`` expression. This is usually
 * only used for simple single-line Arcade scripts, where the ``return``
 * and trailing ``;`` are implied.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsEsriArcadeExpressionStatement : public QgsEsriArcadeStatement
{
  public:
    /**
     * Constructor for QgsEsriArcadeExpressionStatement.
     *
     * Ownership of \a expression is transferred to this statement.
     */
    explicit QgsEsriArcadeExpressionStatement( QgsEsriArcadeExpression *expression SIP_TRANSFER );

    QgsEsriArcadeNode::NodeType nodeType() const override { return NodeType::ExpressionStatement; }
    QString dump() const override;
    QgsEsriArcadeNode *clone() const override SIP_FACTORY;
    void accept( QgsEsriArcadeVisitor &visitor ) const override;

    /**
     * Returns the underlying expression.
     */
    QgsEsriArcadeExpression *expression() const { return mExpression.get(); }

  private:
    QgsEsriArcadeExpressionStatement( const QgsEsriArcadeExpressionStatement &other ) = delete;
    QgsEsriArcadeExpressionStatement &operator=( const QgsEsriArcadeExpressionStatement &other ) = delete;

#ifdef SIP_RUN
    QgsEsriArcadeExpressionStatement( const QgsEsriArcadeExpressionStatement &other );
#endif

    std::unique_ptr<QgsEsriArcadeExpression> mExpression;
};

/**
 * \ingroup core
 * \brief Represents a literal value (number, text string, boolean, or null).
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsEsriArcadeLiteralExpression : public QgsEsriArcadeExpression
{
  public:
    /**
     * Constructor for QgsEsriArcadeLiteralExpression.
     */
    explicit QgsEsriArcadeLiteralExpression( const QVariant &value );

    QgsEsriArcadeNode::NodeType nodeType() const override { return NodeType::LiteralExpression; }
    QString dump() const override;
    QgsEsriArcadeNode *clone() const override SIP_FACTORY;
    void accept( QgsEsriArcadeVisitor &visitor ) const override;

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
class CORE_EXPORT QgsEsriArcadeVariableReferenceExpression : public QgsEsriArcadeExpression
{
  public:
    /**
     * Constructor for QgsEsriArcadeVariableReferenceExpression.
     */
    explicit QgsEsriArcadeVariableReferenceExpression( const QString &name );

    QgsEsriArcadeNode::NodeType nodeType() const override { return NodeType::VariableReferenceExpression; }
    QString dump() const override;
    QgsEsriArcadeNode *clone() const override SIP_FACTORY;
    void accept( QgsEsriArcadeVisitor &visitor ) const override;

    /**
     * Returns the variable identifier name.
     */
    QString name() const { return mName; }

  private:
    QString mName;
};

/**
 * \ingroup core
 * \brief Represents a reference to a profile variable.
 *
 * For example ``$feature``.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsEsriArcadeProfileVariableReferenceExpression : public QgsEsriArcadeExpression
{
  public:
    /**
     * Constructor for QgsEsriArcadeProfileVariableReferenceExpression.
     */
    explicit QgsEsriArcadeProfileVariableReferenceExpression( const QString &name );

    QgsEsriArcadeNode::NodeType nodeType() const override { return NodeType::ProfileVariableReferenceExpression; }
    QString dump() const override;
    QgsEsriArcadeNode *clone() const override SIP_FACTORY;
    void accept( QgsEsriArcadeVisitor &visitor ) const override;

    /**
     * Returns the profile variable name, including the leading $ symbol.
     */
    QString name() const { return mName; }

  private:
    QString mName;
};

/**
 * \ingroup core
 * \brief Represents a member access expression using dot or bracket notation.
 *
 * For example ``$feature.ATTRIBUTE.`` or ``my_dict["ATTRIBUTE"]`` for accessing
 * members of a dictionary.
 *
 * Arcade also supports a ``?.`` accessor which performs a "safe access", where null
 * values are returned when the container itself is null. For these ``?.`` forms
 * isSafeAccess() will return TRUE.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsEsriArcadeMemberAccessExpression : public QgsEsriArcadeExpression
{
  public:
    /**
     * Constructor for QgsEsriArcadeMemberAccessExpression.
     *
     * Ownership of \a containerExpression is transferred to this expression.
     */
    QgsEsriArcadeMemberAccessExpression( QgsEsriArcadeExpression *containerExpression SIP_TRANSFER, const QString &memberName, bool isSafeAccess = false );

    QgsEsriArcadeNode::NodeType nodeType() const override { return NodeType::MemberAccessExpression; }
    QString dump() const override;
    QgsEsriArcadeNode *clone() const override SIP_FACTORY;
    void accept( QgsEsriArcadeVisitor &visitor ) const override;

    /**
     * Returns the container expression being accessed.
     */
    QgsEsriArcadeExpression *containerExpression() const { return mContainerExpression.get(); }

    /**
     * Returns the member attribute name.
     */
    QString memberName() const { return mMemberName; }

    /**
     * Returns TRUE if member access uses the safe access operator "?.".
     */
    bool isSafeAccess() const { return mIsSafeAccess; }

    /**
     * Sets whether member access uses the safe access operator ``?.``.
     */
    void setIsSafeAccess( bool isSafeAccess ) { mIsSafeAccess = isSafeAccess; }

  private:
    QgsEsriArcadeMemberAccessExpression( const QgsEsriArcadeMemberAccessExpression &other ) = delete;
    QgsEsriArcadeMemberAccessExpression &operator=( const QgsEsriArcadeMemberAccessExpression &other ) = delete;

#ifdef SIP_RUN
    QgsEsriArcadeMemberAccessExpression( const QgsEsriArcadeMemberAccessExpression &other );
#endif

    std::unique_ptr<QgsEsriArcadeExpression> mContainerExpression;
    QString mMemberName;
    bool mIsSafeAccess = false;
};

/**
 * \ingroup core
 * \brief Represents a binary operation expression.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsEsriArcadeBinaryOperatorExpression : public QgsEsriArcadeExpression
{
  public:
    /**
     * Binary operator enumeration.
     */
    enum class BinaryOperator
    {
      Plus,                      //!< Addition +
      Minus,                     //!< Subtraction -
      Multiply,                  //!< Multiplication *
      Divide,                    //!< Division /
      Modulo,                    //!< Modulo %
      Equal,                     //!< Equality test ==
      NotEqual,                  //!< Not equal test !=
      LessThan,                  //!< Less than <
      GreaterThan,               //!< Greater than >
      LessThanOrEqual,           //!< Less than or equal to <=
      GreaterThanOrEqual,        //!< Greater than or equal to >=
      LogicalAnd,                //!< Logical and &&
      LogicalOr,                 //!< Logical or ||
      BitwiseAnd,                //!< Bitwise and, eg "7 & 14"
      BitwiseOr,                 //!< Bitwise or, eg "7 | 14"
      BitwiseXor,                //!< Bitwise xor, eg "7 ^ 14"
      BitwiseLeftShift,          //!< Bitwise left shift, eg "7 << 14"
      BitwiseSignedRightShift,   //!< Bitwise right shift, eg "7 >> 14"
      BitwiseUnsignedRightShift, //!< Bitwise unsigned right shift, eg "7 >>> 14"
    };

    //! \note not available in Python bindings
    static const char *BINARY_OPERATOR_TEXT[] SIP_SKIP;

    /**
     * Constructor for QgsEsriArcadeBinaryOperatorExpression.
     *
     * Ownership of \a leftOperand and \a rightOperand is transferred to this expression.
     */
    QgsEsriArcadeBinaryOperatorExpression( BinaryOperator binaryOperator, QgsEsriArcadeExpression *leftOperand SIP_TRANSFER, QgsEsriArcadeExpression *rightOperand SIP_TRANSFER );

    QgsEsriArcadeNode::NodeType nodeType() const override { return NodeType::BinaryOperatorExpression; }
    QString dump() const override;
    QgsEsriArcadeNode *clone() const override SIP_FACTORY;
    void accept( QgsEsriArcadeVisitor &visitor ) const override;

    /**
     * Returns the binary operator.
     */
    BinaryOperator binaryOperator() const { return mBinaryOperator; }

    /**
     * Returns the left-hand operand expression.
     */
    QgsEsriArcadeExpression *leftOperand() const { return mLeftOperand.get(); }

    /**
     * Returns the right-hand operand expression.
     */
    QgsEsriArcadeExpression *rightOperand() const { return mRightOperand.get(); }

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
    QgsEsriArcadeBinaryOperatorExpression( const QgsEsriArcadeBinaryOperatorExpression &other ) = delete;
    QgsEsriArcadeBinaryOperatorExpression &operator=( const QgsEsriArcadeBinaryOperatorExpression &other ) = delete;

#ifdef SIP_RUN
    QgsEsriArcadeBinaryOperatorExpression( const QgsEsriArcadeBinaryOperatorExpression &other );
#endif

    BinaryOperator mBinaryOperator;
    std::unique_ptr<QgsEsriArcadeExpression> mLeftOperand;
    std::unique_ptr<QgsEsriArcadeExpression> mRightOperand;
};

/**
 * \ingroup core
 * \brief Represents a unary operation expression.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsEsriArcadeUnaryOperatorExpression : public QgsEsriArcadeExpression
{
  public:
    /**
     * Unary operator enumeration.
     */
    enum class UnaryOperator
    {
      Not,           //!< Not operator, !
      Minus,         //!< Minus operator, -
      Plus,          //!< Plus operator, +
      BitwiseNot,    //!< Bitwise not, eg "~7"
      PreIncrement,  //!< Preincrement operator, eg "++x"
      PreDecrement,  //!< Prederement operator, eg "--x"
      PostIncrement, //!< Postincrement operator, eg "x++"
      PostDecrement, //!< Postdecrement operator, eg "x--"
    };

    //! \note not available in Python bindings
    static const char *UNARY_OPERATOR_TEXT[] SIP_SKIP;

    /**
     * Constructor for QgsEsriArcadeUnaryOperatorExpression.
     *
     * Ownership of \a operand is transferred to this expression.
     */
    QgsEsriArcadeUnaryOperatorExpression( UnaryOperator unaryOperator, QgsEsriArcadeExpression *operand SIP_TRANSFER );

    QgsEsriArcadeNode::NodeType nodeType() const override { return NodeType::UnaryOperatorExpression; }
    QString dump() const override;
    QgsEsriArcadeNode *clone() const override SIP_FACTORY;
    void accept( QgsEsriArcadeVisitor &visitor ) const override;

    /**
     * Returns the unary operator.
     */
    UnaryOperator unaryOperator() const { return mUnaryOperator; }

    /**
     * Returns the operand expression.
     */
    QgsEsriArcadeExpression *operand() const { return mOperand.get(); }

    /**
     * Returns text representation of the operator.
     */
    QString text() const;

  private:
    QgsEsriArcadeUnaryOperatorExpression( const QgsEsriArcadeUnaryOperatorExpression &other ) = delete;
    QgsEsriArcadeUnaryOperatorExpression &operator=( const QgsEsriArcadeUnaryOperatorExpression &other ) = delete;

#ifdef SIP_RUN
    QgsEsriArcadeUnaryOperatorExpression( const QgsEsriArcadeUnaryOperatorExpression &other );
#endif

    UnaryOperator mUnaryOperator;
    std::unique_ptr<QgsEsriArcadeExpression> mOperand;
};

/**
 * \ingroup core
 * \brief Represents a function call expression.
 *
 * For example ``Sum(values)``.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsEsriArcadeFunctionCallExpression : public QgsEsriArcadeExpression
{
  public:
    /**
     * Constructor for QgsEsriArcadeFunctionCallExpression.
     *
     * Ownership of all expressions in \a arguments is transferred to this node.
     */
    QgsEsriArcadeFunctionCallExpression( const QString &functionName, const QList<QgsEsriArcadeExpression *> &arguments SIP_TRANSFER );

    ~QgsEsriArcadeFunctionCallExpression() override;

    QgsEsriArcadeNode::NodeType nodeType() const override { return NodeType::FunctionCallExpression; }
    QString dump() const override;
    QgsEsriArcadeNode *clone() const override SIP_FACTORY;
    void accept( QgsEsriArcadeVisitor &visitor ) const override;

    /**
     * Returns the function name.
     */
    QString functionName() const { return mFunctionName; }

    /**
     * Returns the list of argument expressions.
     */
    QList<QgsEsriArcadeExpression *> arguments() const { return mArguments; }

  private:
    QgsEsriArcadeFunctionCallExpression( const QgsEsriArcadeFunctionCallExpression &other ) = delete;
    QgsEsriArcadeFunctionCallExpression &operator=( const QgsEsriArcadeFunctionCallExpression &other ) = delete;

#ifdef SIP_RUN
    QgsEsriArcadeFunctionCallExpression( const QgsEsriArcadeFunctionCallExpression &other );
#endif

    QString mFunctionName;
    QList<QgsEsriArcadeExpression *> mArguments;
};

/**
 * \ingroup core
 * \brief Represents an array or dictionary index subscript expression.
 *
 * For example ``values[index]``.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsEsriArcadeIndexExpression : public QgsEsriArcadeExpression
{
  public:
    /**
     * Constructor for QgsEsriArcadeIndexExpression.
     *
     * Ownership of \a containerExpression and \a indexExpression is transferred to this expression.
     */
    QgsEsriArcadeIndexExpression( QgsEsriArcadeExpression *containerExpression SIP_TRANSFER, QgsEsriArcadeExpression *indexExpression SIP_TRANSFER );

    QgsEsriArcadeNode::NodeType nodeType() const override { return NodeType::IndexExpression; }
    QString dump() const override;
    QgsEsriArcadeNode *clone() const override SIP_FACTORY;
    void accept( QgsEsriArcadeVisitor &visitor ) const override;

    /**
     * Returns the container expression being indexed.
     */
    QgsEsriArcadeExpression *containerExpression() const { return mContainerExpression.get(); }

    /**
     * Returns the index expression.
     */
    QgsEsriArcadeExpression *indexExpression() const { return mIndexExpression.get(); }

  private:
    QgsEsriArcadeIndexExpression( const QgsEsriArcadeIndexExpression &other ) = delete;
    QgsEsriArcadeIndexExpression &operator=( const QgsEsriArcadeIndexExpression &other ) = delete;

#ifdef SIP_RUN
    QgsEsriArcadeIndexExpression( const QgsEsriArcadeIndexExpression &other );
#endif

    std::unique_ptr<QgsEsriArcadeExpression> mContainerExpression;
    std::unique_ptr<QgsEsriArcadeExpression> mIndexExpression;
};

/**
 * \ingroup core
 * \brief Represents an array literal expression.
 *
 * For example ``[1, 2, 3]``.
 *
 * Array elements may be expressions themselves.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsEsriArcadeArrayLiteralExpression : public QgsEsriArcadeExpression
{
  public:
    /**
      * Constructor for QgsEsriArcadeArrayLiteralExpression.
      *
      * Ownership of all expressions in \a elements is transferred to this node.
      */
    explicit QgsEsriArcadeArrayLiteralExpression( const QList<QgsEsriArcadeExpression *> &elements SIP_TRANSFER );

    ~QgsEsriArcadeArrayLiteralExpression() override;

    QgsEsriArcadeNode::NodeType nodeType() const override { return NodeType::ArrayLiteralExpression; }
    QString dump() const override;
    QgsEsriArcadeNode *clone() const override SIP_FACTORY;
    void accept( QgsEsriArcadeVisitor &visitor ) const override;

    /**
      * Returns the list of element expressions.
     */
    QList<QgsEsriArcadeExpression *> elements() const { return mElements; }

  private:
    QgsEsriArcadeArrayLiteralExpression( const QgsEsriArcadeArrayLiteralExpression &other ) = delete;
    QgsEsriArcadeArrayLiteralExpression &operator=( const QgsEsriArcadeArrayLiteralExpression &other ) = delete;

#ifdef SIP_RUN
    QgsEsriArcadeArrayLiteralExpression( const QgsEsriArcadeArrayLiteralExpression &other );
#endif

    QList<QgsEsriArcadeExpression *> mElements;
};

/**
 * \ingroup core
 * \brief Represents a key-value entry in an Esri Arcade dictionary literal.
 *
 * Eg the ``a: 5`` part of the ``{a: 5, b: 6}`` dictionary literal.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsEsriArcadeDictionaryElement
{
  public:
    /**
   * Constructor for QgsEsriArcadeDictionaryElement.
   *
   * Ownership of \a value is transferred to this element.
   */
    QgsEsriArcadeDictionaryElement( const QString &key, QgsEsriArcadeExpression *value SIP_TRANSFER, bool isAutoKey = false );

    ~QgsEsriArcadeDictionaryElement();

    /**
   * Returns the entry key.
   */
    QString key() const { return mKey; }

    /**
   * Returns the entry value expression.
   */
    QgsEsriArcadeExpression *value() const { return mValue.get(); }

    /**
   * Returns TRUE if the entry was defined using auto-keys.
   */
    bool isAutoKey() const { return mIsAutoKey; }

    /**
   * Creates a deep copy of the element.
   *
   * Caller takes ownership of the returned object.
   */
    QgsEsriArcadeDictionaryElement *clone() const SIP_FACTORY;

    /**
   * Formats the element as string.
   */
    QString dump() const;

  private:
    QgsEsriArcadeDictionaryElement( const QgsEsriArcadeDictionaryElement &other ) = delete;
    QgsEsriArcadeDictionaryElement &operator=( const QgsEsriArcadeDictionaryElement &other ) = delete;

#ifdef SIP_RUN
    QgsEsriArcadeDictionaryElement( const QgsEsriArcadeDictionaryElement &other );
#endif

    QString mKey;
    std::unique_ptr<QgsEsriArcadeExpression> mValue;
    bool mIsAutoKey = false;
};

/**
 * \ingroup core
 * \brief Represents a dictionary literal expression.
 *
 * For example ``{ a: 1, "b": 2 }``.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsEsriArcadeDictionaryLiteralExpression : public QgsEsriArcadeExpression
{
  public:
    /**
   * Constructor for QgsEsriArcadeDictionaryLiteralExpression.
   *
   * Ownership of the \a elements is transferred to the expression.
   */
    explicit QgsEsriArcadeDictionaryLiteralExpression( const QList<QgsEsriArcadeDictionaryElement *> &elements SIP_TRANSFER );
    ~QgsEsriArcadeDictionaryLiteralExpression() override;

    QgsEsriArcadeNode::NodeType nodeType() const override { return NodeType::DictionaryLiteralExpression; }
    QString dump() const override;
    QgsEsriArcadeNode *clone() const override SIP_FACTORY;
    void accept( QgsEsriArcadeVisitor &visitor ) const override;

    /**
   * Returns the dictionary elements.
   */
    QList<QgsEsriArcadeDictionaryElement *> elements() const { return mElements; }

  private:
    QgsEsriArcadeDictionaryLiteralExpression( const QgsEsriArcadeDictionaryLiteralExpression &other ) = delete;
    QgsEsriArcadeDictionaryLiteralExpression &operator=( const QgsEsriArcadeDictionaryLiteralExpression &other ) = delete;

#ifdef SIP_RUN
    QgsEsriArcadeDictionaryLiteralExpression( const QgsEsriArcadeDictionaryLiteralExpression &other );
#endif

    QList<QgsEsriArcadeDictionaryElement *> mElements;
};

/**
 * \ingroup core
 * \brief Represents an if statement.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsEsriArcadeIfStatement : public QgsEsriArcadeStatement
{
  public:
    /**
     * Constructor for QgsEsriArcadeIfStatement.
     *
     * Ownership of \a condition, \a trueBlock, and \a falseBlock is transferred to this statement.
     */
    QgsEsriArcadeIfStatement( QgsEsriArcadeExpression *condition SIP_TRANSFER, QgsEsriArcadeBlockStatement *trueBlock SIP_TRANSFER, QgsEsriArcadeBlockStatement *falseBlock SIP_TRANSFER = nullptr );

    QgsEsriArcadeNode::NodeType nodeType() const override { return NodeType::IfStatement; }
    QString dump() const override;
    QgsEsriArcadeNode *clone() const override SIP_FACTORY;
    void accept( QgsEsriArcadeVisitor &visitor ) const override;

    /**
     * Returns the condition expression.
     */
    QgsEsriArcadeExpression *condition() const { return mCondition.get(); }

    /**
     * Returns the statement block executed when the condition evaluates to TRUE.
     */
    QgsEsriArcadeBlockStatement *trueBlock() const { return mTrueBlock.get(); }

    /**
     * Returns the statement block executed when the condition evaluates to FALSE, or NULLPTR if none.
     */
    QgsEsriArcadeBlockStatement *falseBlock() const { return mFalseBlock.get(); }

  private:
    QgsEsriArcadeIfStatement( const QgsEsriArcadeIfStatement &other ) = delete;
    QgsEsriArcadeIfStatement &operator=( const QgsEsriArcadeIfStatement &other ) = delete;

#ifdef SIP_RUN
    QgsEsriArcadeIfStatement( const QgsEsriArcadeIfStatement &other );
#endif

    std::unique_ptr<QgsEsriArcadeExpression> mCondition;
    std::unique_ptr<QgsEsriArcadeBlockStatement> mTrueBlock;
    std::unique_ptr<QgsEsriArcadeBlockStatement> mFalseBlock;
};

/**
 * \ingroup core
 * \brief Represents a standard counter-based for loop.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsEsriArcadeForStatement : public QgsEsriArcadeStatement
{
  public:
    /**
     * Constructor for QgsEsriArcadeForStatement.
     *
     * Ownership of \a initialization, \a condition, \a step, and \a body is transferred to this statement.
     */
    QgsEsriArcadeForStatement(
      QgsEsriArcadeStatement *initialization SIP_TRANSFER, QgsEsriArcadeExpression *condition SIP_TRANSFER, QgsEsriArcadeNode *step SIP_TRANSFER, QgsEsriArcadeBlockStatement *body SIP_TRANSFER
    );

    QgsEsriArcadeNode::NodeType nodeType() const override { return NodeType::ForStatement; }
    QString dump() const override;
    QgsEsriArcadeNode *clone() const override SIP_FACTORY;
    void accept( QgsEsriArcadeVisitor &visitor ) const override;

    /**
     * Returns the initialization statement, or NULLPTR if none.
     */
    QgsEsriArcadeStatement *initialization() const { return mInitialization.get(); }

    /**
     * Returns the loop condition expression, or NULLPTR if none.
     */
    QgsEsriArcadeExpression *condition() const { return mCondition.get(); }

    /**
     * Returns the step expression or statement, or NULLPTR if none.
     */
    QgsEsriArcadeNode *step() const { return mStep.get(); }

    /**
     * Returns the loop body block statement.
     */
    QgsEsriArcadeBlockStatement *body() const { return mBody.get(); }

  private:
    QgsEsriArcadeForStatement( const QgsEsriArcadeForStatement &other ) = delete;
    QgsEsriArcadeForStatement &operator=( const QgsEsriArcadeForStatement &other ) = delete;

#ifdef SIP_RUN
    QgsEsriArcadeForStatement( const QgsEsriArcadeForStatement &other );
#endif

    std::unique_ptr<QgsEsriArcadeStatement> mInitialization;
    std::unique_ptr<QgsEsriArcadeExpression> mCondition;
    std::unique_ptr<QgsEsriArcadeNode> mStep;
    std::unique_ptr<QgsEsriArcadeBlockStatement> mBody;
};

/**
 * \ingroup core
 * \brief Represents a range based for...in or for...of iteration statement.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsEsriArcadeForInStatement : public QgsEsriArcadeStatement
{
  public:
    /**
     * Type of iteration loop.
     */
    enum class IterationType
    {
      ForIn, //!< For x in y loop
      ForOf, //!< For x of y loop
    };

    /**
     * Constructor for QgsEsriArcadeForInStatement.
     *
     * Ownership of \a collection and \a body is transferred to this statement.
     */
    QgsEsriArcadeForInStatement(
      IterationType iterationType, const QString &variableName, bool isVariableDeclaration, QgsEsriArcadeExpression *collection SIP_TRANSFER, QgsEsriArcadeBlockStatement *body SIP_TRANSFER
    );

    QgsEsriArcadeNode::NodeType nodeType() const override { return NodeType::ForInStatement; }
    QString dump() const override;
    QgsEsriArcadeNode *clone() const override SIP_FACTORY;
    void accept( QgsEsriArcadeVisitor &visitor ) const override;

    /**
     * Returns the iteration type (for...in or for...of).
     */
    IterationType iterationType() const { return mIterationType; }

    /**
     * Returns the loop variable identifier name.
     */
    QString variableName() const { return mVariableName; }

    /**
     * Returns TRUE if the loop variable was declared with the var keyword.
     */
    bool isVariableDeclaration() const { return mIsVariableDeclaration; }

    /**
     * Returns the collection expression being iterated over.
     */
    QgsEsriArcadeExpression *collection() const { return mCollection.get(); }

    /**
     * Returns the loop body block statement.
     */
    QgsEsriArcadeBlockStatement *body() const { return mBody.get(); }

  private:
    QgsEsriArcadeForInStatement( const QgsEsriArcadeForInStatement &other ) = delete;
    QgsEsriArcadeForInStatement &operator=( const QgsEsriArcadeForInStatement &other ) = delete;

#ifdef SIP_RUN
    QgsEsriArcadeForInStatement( const QgsEsriArcadeForInStatement &other );
#endif

    IterationType mIterationType = IterationType::ForIn;
    QString mVariableName;
    bool mIsVariableDeclaration = false;
    std::unique_ptr<QgsEsriArcadeExpression> mCollection;
    std::unique_ptr<QgsEsriArcadeBlockStatement> mBody;
};

/**
 * \ingroup core
 * \brief Represents a while loop statement.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsEsriArcadeWhileStatement : public QgsEsriArcadeStatement
{
  public:
    /**
     * Constructor for QgsEsriArcadeWhileStatement.
     *
     * Ownership of \a condition and \a body is transferred to this statement.
     */
    QgsEsriArcadeWhileStatement( QgsEsriArcadeExpression *condition SIP_TRANSFER, QgsEsriArcadeBlockStatement *body SIP_TRANSFER );

    QgsEsriArcadeNode::NodeType nodeType() const override { return NodeType::WhileStatement; }
    QString dump() const override;
    QgsEsriArcadeNode *clone() const override SIP_FACTORY;
    void accept( QgsEsriArcadeVisitor &visitor ) const override;

    /**
     * Returns the loop condition expression.
     */
    QgsEsriArcadeExpression *condition() const { return mCondition.get(); }

    /**
     * Returns the loop body block statement.
     */
    QgsEsriArcadeBlockStatement *body() const { return mBody.get(); }

  private:
    QgsEsriArcadeWhileStatement( const QgsEsriArcadeWhileStatement &other ) = delete;
    QgsEsriArcadeWhileStatement &operator=( const QgsEsriArcadeWhileStatement &other ) = delete;

#ifdef SIP_RUN
    QgsEsriArcadeWhileStatement( const QgsEsriArcadeWhileStatement &other );
#endif

    std::unique_ptr<QgsEsriArcadeExpression> mCondition;
    std::unique_ptr<QgsEsriArcadeBlockStatement> mBody;
};

/**
 * \ingroup core
 * \brief Represents a break loop control statement.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsEsriArcadeBreakStatement : public QgsEsriArcadeStatement
{
  public:
    /**
     * Constructor for QgsEsriArcadeBreakStatement.
     */
    QgsEsriArcadeBreakStatement() = default;

    QgsEsriArcadeNode::NodeType nodeType() const override { return NodeType::BreakStatement; }
    QString dump() const override;
    QgsEsriArcadeNode *clone() const override SIP_FACTORY;
    void accept( QgsEsriArcadeVisitor &visitor ) const override;
};

/**
 * \ingroup core
 * \brief Represents a continue loop control statement.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsEsriArcadeContinueStatement : public QgsEsriArcadeStatement
{
  public:
    /**
     * Constructor for QgsEsriArcadeContinueStatement.
     */
    QgsEsriArcadeContinueStatement() = default;

    QgsEsriArcadeNode::NodeType nodeType() const override { return NodeType::ContinueStatement; }
    QString dump() const override;
    QgsEsriArcadeNode *clone() const override SIP_FACTORY;
    void accept( QgsEsriArcadeVisitor &visitor ) const override;
};


/**
 * \ingroup core
 * \brief Abstract visitor interface for traversing Esri Arcade abstract syntax trees.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsEsriArcadeVisitor
{
  public:
    virtual ~QgsEsriArcadeVisitor() = default;

    //! Visit a QgsEsriArcadeBlockStatement \a node
    virtual void visit( const QgsEsriArcadeBlockStatement &node ) = 0;
    //! Visit a QgsEsriArcadeVariableDeclarationStatement \a node
    virtual void visit( const QgsEsriArcadeVariableDeclarationStatement &node ) = 0;
    //! Visit a QgsEsriArcadeAssignmentStatement \a node
    virtual void visit( const QgsEsriArcadeAssignmentStatement &node ) = 0;
    //! Visit a QgsEsriArcadeReturnStatement \a node
    virtual void visit( const QgsEsriArcadeReturnStatement &node ) = 0;
    //! Visit a QgsEsriArcadeFunctionDeclarationStatement \a node
    virtual void visit( const QgsEsriArcadeFunctionDeclarationStatement &node ) = 0;
    //! Visit a QgsEsriArcadeExpressionStatement \a node
    virtual void visit( const QgsEsriArcadeExpressionStatement &node ) = 0;
    //! Visit a QgsEsriArcadeLiteralExpression \a node
    virtual void visit( const QgsEsriArcadeLiteralExpression &node ) = 0;
    //! Visit a QgsEsriArcadeVariableReferenceExpression \a node
    virtual void visit( const QgsEsriArcadeVariableReferenceExpression &node ) = 0;
    //! Visit a QgsEsriArcadeProfileVariableReferenceExpression \a node
    virtual void visit( const QgsEsriArcadeProfileVariableReferenceExpression &node ) = 0;
    //! Visit a QgsEsriArcadeMemberAccessExpression \a node
    virtual void visit( const QgsEsriArcadeMemberAccessExpression &node ) = 0;
    //! Visit a QgsEsriArcadeBinaryOperatorExpression \a node
    virtual void visit( const QgsEsriArcadeBinaryOperatorExpression &node ) = 0;
    //! Visit a QgsEsriArcadeUnaryOperatorExpression \a node
    virtual void visit( const QgsEsriArcadeUnaryOperatorExpression &node ) = 0;
    //! Visit a QgsEsriArcadeFunctionCallExpression \a node
    virtual void visit( const QgsEsriArcadeFunctionCallExpression &node ) = 0;
    //! Visit a QgsEsriArcadeIndexExpression \a node
    virtual void visit( const QgsEsriArcadeIndexExpression &node ) = 0;
    //! Visit a QgsEsriArcadeDictionaryLiteralExpression \a node
    virtual void visit( const QgsEsriArcadeDictionaryLiteralExpression &node ) = 0;
    //! Visit a QgsEsriArcadeArrayLiteralExpression \a node
    virtual void visit( const QgsEsriArcadeArrayLiteralExpression &node ) = 0;
    //! Visit a QgsEsriArcadeIfStatement \a node
    virtual void visit( const QgsEsriArcadeIfStatement &node ) = 0;
    //! Visit a QgsEsriArcadeForStatement \a node
    virtual void visit( const QgsEsriArcadeForStatement &node ) = 0;
    //! Visit a QgsEsriArcadeForInStatement \a node
    virtual void visit( const QgsEsriArcadeForInStatement &node ) = 0;
    //! Visit a QgsEsriArcadeWhileStatement \a node
    virtual void visit( const QgsEsriArcadeWhileStatement &node ) = 0;
    //! Visit a QgsEsriArcadeBreakStatement \a node
    virtual void visit( const QgsEsriArcadeBreakStatement &node ) = 0;
    //! Visit a QgsEsriArcadeContinueStatement \a node
    virtual void visit( const QgsEsriArcadeContinueStatement &node ) = 0;
};

/**
 * \ingroup core
 * \brief Visitor implementation that recursively traverses all child nodes in an Esri Arcade script.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsEsriArcadeRecursiveVisitor : public QgsEsriArcadeVisitor
{
  public:
    QgsEsriArcadeRecursiveVisitor() = default;

    void visit( const QgsEsriArcadeBlockStatement &node ) override;
    void visit( const QgsEsriArcadeVariableDeclarationStatement &node ) override;
    void visit( const QgsEsriArcadeAssignmentStatement &node ) override;
    void visit( const QgsEsriArcadeReturnStatement &node ) override;
    void visit( const QgsEsriArcadeFunctionDeclarationStatement &node ) override;
    void visit( const QgsEsriArcadeExpressionStatement &node ) override;
    void visit( const QgsEsriArcadeLiteralExpression &node ) override;
    void visit( const QgsEsriArcadeVariableReferenceExpression &node ) override;
    void visit( const QgsEsriArcadeProfileVariableReferenceExpression &node ) override;
    void visit( const QgsEsriArcadeMemberAccessExpression &node ) override;
    void visit( const QgsEsriArcadeBinaryOperatorExpression &node ) override;
    void visit( const QgsEsriArcadeUnaryOperatorExpression &node ) override;
    void visit( const QgsEsriArcadeFunctionCallExpression &node ) override;
    void visit( const QgsEsriArcadeIndexExpression &node ) override;
    void visit( const QgsEsriArcadeArrayLiteralExpression &node ) override;
    void visit( const QgsEsriArcadeDictionaryLiteralExpression &node ) override;
    void visit( const QgsEsriArcadeIfStatement &node ) override;
    void visit( const QgsEsriArcadeForStatement &node ) override;
    void visit( const QgsEsriArcadeForInStatement &node ) override;
    void visit( const QgsEsriArcadeWhileStatement &node ) override;
    void visit( const QgsEsriArcadeBreakStatement &node ) override;
    void visit( const QgsEsriArcadeContinueStatement &node ) override;
};

/**
 * \ingroup core
 * \brief Parses and encapsulates an entire Esri Arcade script.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsEsriArcadeScript
{
  public:
    /**
     * Creates a new script from \a scriptText.
     */
    explicit QgsEsriArcadeScript( const QString &scriptText = QString() );

    QgsEsriArcadeScript( const QgsEsriArcadeScript &other );
    QgsEsriArcadeScript &operator=( const QgsEsriArcadeScript &other );

    ~QgsEsriArcadeScript();

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
    const QgsEsriArcadeBlockStatement *rootBlock() const;

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
    void acceptVisitor( QgsEsriArcadeVisitor &visitor ) const;

    /**
     * Returns a properly quoted string literal (enclosed in double quotes with escape sequences).
     */
    static QString quotedString( const QString &text );

  protected:
    std::unique_ptr<QgsEsriArcadeBlockStatement> mRootBlock;
    QString mScript;
    QString mParserErrorString;
};

#endif // QGSESRIARCADESCRIPT_H
