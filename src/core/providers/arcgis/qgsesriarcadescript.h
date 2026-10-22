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

/**
 * \ingroup core
 * \brief Represents an abstract node for an Esri Arcade script.
 *
 * \since QGIS 4.6
 */
class CORE_EXPORT QgsEsriArcadeNode
{
#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    switch ( sipCpp->nodeType() )
    {
      case QgsEsriArcadeNode::NodeType::ntBlockStatement:
        sipType = sipType_QgsEsriArcadeBlockStatement;
        break;
      case QgsEsriArcadeNode::NodeType::ntVariableDeclarationStatement:
        sipType = sipType_QgsEsriArcadeVariableDeclarationStatement;
        break;
      case QgsEsriArcadeNode::NodeType::ntAssignmentStatement:
        sipType = sipType_QgsEsriArcadeAssignmentStatement;
        break;
      case QgsEsriArcadeNode::NodeType::ntReturnStatement:
        sipType = sipType_QgsEsriArcadeReturnStatement;
        break;
      case QgsEsriArcadeNode::NodeType::ntFunctionDeclarationStatement:
        sipType = sipType_QgsEsriArcadeFunctionDeclarationStatement;
        break;
      case QgsEsriArcadeNode::NodeType::ntExpressionStatement:
        sipType = sipType_QgsEsriArcadeExpressionStatement;
        break;
      case QgsEsriArcadeNode::NodeType::ntArrayLiteralExpression:
        sipType = sipType_QgsEsriArcadeArrayLiteralExpression;
        break;
      case QgsEsriArcadeNode::NodeType::ntLiteralExpression:
        sipType = sipType_QgsEsriArcadeLiteralExpression;
        break;
      case QgsEsriArcadeNode::NodeType::ntVariableReferenceExpression:
        sipType = sipType_QgsEsriArcadeVariableReferenceExpression;
        break;
      case QgsEsriArcadeNode::NodeType::ntProfileVariableReferenceExpression:
        sipType = sipType_QgsEsriArcadeProfileVariableReferenceExpression;
        break;
      case QgsEsriArcadeNode::NodeType::ntMemberAccessExpression:
        sipType = sipType_QgsEsriArcadeMemberAccessExpression;
        break;
      case QgsEsriArcadeNode::NodeType::ntBinaryOperatorExpression:
        sipType = sipType_QgsEsriArcadeBinaryOperatorExpression;
        break;
      case QgsEsriArcadeNode::NodeType::ntUnaryOperatorExpression:
        sipType = sipType_QgsEsriArcadeUnaryOperatorExpression;
        break;
      case QgsEsriArcadeNode::NodeType::ntFunctionCallExpression:
        sipType = sipType_QgsEsriArcadeFunctionCallExpression;
        break;
      case QgsEsriArcadeNode::NodeType::ntIndexExpression:
        sipType = sipType_QgsEsriArcadeIndexExpression;
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
      ntBlockStatement,                     //!< Block statement (a list of statements grouped together in a block)
      ntVariableDeclarationStatement,       //!< Variable declaration statement
      ntReturnStatement,                    //!< Return statement
      ntAssignmentStatement,                //!< Variable assignment statement
      ntFunctionDeclarationStatement,       //!< Function declaration
      ntExpressionStatement,                //!< Expression
      ntLiteralExpression,                  //!< Literal
      ntVariableReferenceExpression,        //!< Reference to a standard variable
      ntProfileVariableReferenceExpression, //!< Reference to a profile variable (eg $feature.ATTRIBUTE)
      ntMemberAccessExpression,             //!< Member access via bracket notation (eg $feature["ATTRIBUTE"])
      ntBinaryOperatorExpression,           //!< Binary Operator
      ntUnaryOperatorExpression,            //!< Operator
      ntFunctionCallExpression,             //!< Function call
      ntIndexExpression,                    //!< Index operator
      ntArrayLiteralExpression,             //!< A literal array, where the element values may be expressions themselves (eg [1, 2, 3/4])
    };

    virtual ~QgsEsriArcadeNode() = default;

    /**
     * Returns the node type.
     */
    virtual QgsEsriArcadeNode::NodeType nodeType() const = 0;

    /**
     * Returns the formatted script representation of the node.
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
 * \since QGIS 4.6
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
 * \since QGIS 4.6
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
 * \since QGIS 4.6
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

    QgsEsriArcadeNode::NodeType nodeType() const override { return NodeType::ntBlockStatement; }
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
 * \brief Represents a variable declaration statement (e.g. var x = 3;).
 *
 * \since QGIS 4.6
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

    QgsEsriArcadeNode::NodeType nodeType() const override { return NodeType::ntVariableDeclarationStatement; }
    QString dump() const override;
    QgsEsriArcadeNode *clone() const override SIP_FACTORY;
    void accept( QgsEsriArcadeVisitor &visitor ) const override;

    /**
     * Returns the declared variable name.
     */
    QString variableName() const { return mVariableName; }

    /**
     * Returns the optional initializer expression, or NULLPTR if uninitialized.
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
 * \brief Represents a variable reassignment statement (e.g. age = "35 years";).
 *
 * \since QGIS 4.6
 */
class CORE_EXPORT QgsEsriArcadeAssignmentStatement : public QgsEsriArcadeStatement
{
  public:
    /**
     * Constructor for QgsEsriArcadeAssignmentStatement.
     *
     * Ownership of \a target and \a value is transferred to this statement.
     */
    QgsEsriArcadeAssignmentStatement( QgsEsriArcadeExpression *target SIP_TRANSFER, QgsEsriArcadeExpression *value SIP_TRANSFER );

    QgsEsriArcadeNode::NodeType nodeType() const override { return NodeType::ntAssignmentStatement; }
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

  private:
    QgsEsriArcadeAssignmentStatement( const QgsEsriArcadeAssignmentStatement &other ) = delete;
    QgsEsriArcadeAssignmentStatement &operator=( const QgsEsriArcadeAssignmentStatement &other ) = delete;

#ifdef SIP_RUN
    QgsEsriArcadeAssignmentStatement( const QgsEsriArcadeAssignmentStatement &other );
#endif

    std::unique_ptr<QgsEsriArcadeExpression> mTarget;
    std::unique_ptr<QgsEsriArcadeExpression> mValue;
};

/**
 * \ingroup core
 * \brief Represents a return statement (e.g. return x + y;).
 *
 * \since QGIS 4.6
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

    QgsEsriArcadeNode::NodeType nodeType() const override { return NodeType::ntReturnStatement; }
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
 * \brief Represents a function declaration statement (e.g. function foo(a, b) { ... }).
 *
 * \since QGIS 4.6
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

    QgsEsriArcadeNode::NodeType nodeType() const override { return NodeType::ntFunctionDeclarationStatement; }
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
 * \since QGIS 4.6
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

    QgsEsriArcadeNode::NodeType nodeType() const override { return NodeType::ntExpressionStatement; }
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
 * \since QGIS 4.6
 */
class CORE_EXPORT QgsEsriArcadeLiteralExpression : public QgsEsriArcadeExpression
{
  public:
    /**
     * Constructor for QgsEsriArcadeLiteralExpression.
     */
    explicit QgsEsriArcadeLiteralExpression( const QVariant &value );

    QgsEsriArcadeNode::NodeType nodeType() const override { return NodeType::ntLiteralExpression; }
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
 * \since QGIS 4.6
 */
class CORE_EXPORT QgsEsriArcadeVariableReferenceExpression : public QgsEsriArcadeExpression
{
  public:
    /**
     * Constructor for QgsEsriArcadeVariableReferenceExpression.
     */
    explicit QgsEsriArcadeVariableReferenceExpression( const QString &name );

    QgsEsriArcadeNode::NodeType nodeType() const override { return NodeType::ntVariableReferenceExpression; }
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
 * \brief Represents a reference to a profile variable (e.g. "$feature").
 *
 * \since QGIS 4.6
 */
class CORE_EXPORT QgsEsriArcadeProfileVariableReferenceExpression : public QgsEsriArcadeExpression
{
  public:
    /**
     * Constructor for QgsEsriArcadeProfileVariableReferenceExpression.
     */
    explicit QgsEsriArcadeProfileVariableReferenceExpression( const QString &name );

    QgsEsriArcadeNode::NodeType nodeType() const override { return NodeType::ntProfileVariableReferenceExpression; }
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
 * \brief Represents a member access expression using dot notation (e.g. "$feature.ATTRIBUTE").
 *
 * \since QGIS 4.6
 */
class CORE_EXPORT QgsEsriArcadeMemberAccessExpression : public QgsEsriArcadeExpression
{
  public:
    /**
     * Constructor for QgsEsriArcadeMemberAccessExpression.
     *
     * Ownership of \a containerExpression is transferred to this expression.
     */
    QgsEsriArcadeMemberAccessExpression( QgsEsriArcadeExpression *containerExpression SIP_TRANSFER, const QString &memberName );

    QgsEsriArcadeNode::NodeType nodeType() const override { return NodeType::ntMemberAccessExpression; }
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

  private:
    QgsEsriArcadeMemberAccessExpression( const QgsEsriArcadeMemberAccessExpression &other ) = delete;
    QgsEsriArcadeMemberAccessExpression &operator=( const QgsEsriArcadeMemberAccessExpression &other ) = delete;

#ifdef SIP_RUN
    QgsEsriArcadeMemberAccessExpression( const QgsEsriArcadeMemberAccessExpression &other );
#endif

    std::unique_ptr<QgsEsriArcadeExpression> mContainerExpression;
    QString mMemberName;
};

/**
 * \ingroup core
 * \brief Represents a binary operation expression.
 *
 * \since QGIS 4.6
 */
class CORE_EXPORT QgsEsriArcadeBinaryOperatorExpression : public QgsEsriArcadeExpression
{
  public:
    /**
     * Binary operator enumeration.
     */
    enum class BinaryOperator
    {
      boPlus,
      boMinus,
      boMultiply,
      boDivide,
      boModulo,
      boEqual,
      boNotEqual,
      boLessThan,
      boGreaterThan,
      boLessThanOrEqual,
      boGreaterThanOrEqual,
      boLogicalAnd,
      boLogicalOr,
    };

    //! \note not available in Python bindings
    static const char *BINARY_OPERATOR_TEXT[] SIP_SKIP;

    /**
     * Constructor for QgsEsriArcadeBinaryOperatorExpression.
     *
     * Ownership of \a leftOperand and \a rightOperand is transferred to this expression.
     */
    QgsEsriArcadeBinaryOperatorExpression( BinaryOperator binaryOperator, QgsEsriArcadeExpression *leftOperand SIP_TRANSFER, QgsEsriArcadeExpression *rightOperand SIP_TRANSFER );

    QgsEsriArcadeNode::NodeType nodeType() const override { return NodeType::ntBinaryOperatorExpression; }
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
 * \since QGIS 4.6
 */
class CORE_EXPORT QgsEsriArcadeUnaryOperatorExpression : public QgsEsriArcadeExpression
{
  public:
    /**
     * Unary operator enumeration.
     */
    enum class UnaryOperator
    {
      uoNot,
      uoMinus,
      uoPlus,
    };

    //! \note not available in Python bindings
    static const char *UNARY_OPERATOR_TEXT[] SIP_SKIP;

    /**
     * Constructor for QgsEsriArcadeUnaryOperatorExpression.
     *
     * Ownership of \a operand is transferred to this expression.
     */
    QgsEsriArcadeUnaryOperatorExpression( UnaryOperator unaryOperator, QgsEsriArcadeExpression *operand SIP_TRANSFER );

    QgsEsriArcadeNode::NodeType nodeType() const override { return NodeType::ntUnaryOperatorExpression; }
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
 * \brief Represents a function call expression (e.g. Sum(values)).
 *
 * \since QGIS 4.6
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

    QgsEsriArcadeNode::NodeType nodeType() const override { return NodeType::ntFunctionCallExpression; }
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
 * \brief Represents an array or dictionary index subscript expression (e.g. values[index]).
 *
 * \since QGIS 4.6
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

    QgsEsriArcadeNode::NodeType nodeType() const override { return NodeType::ntIndexExpression; }
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
 * \brief Represents an array literal expression (e.g. [1, 2, 3]).
 *
 * Array elements may be expressions themselves.
 *
 * \since QGIS 4.6
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

    QgsEsriArcadeNode::NodeType nodeType() const override { return NodeType::ntArrayLiteralExpression; }
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
 * \brief Abstract visitor interface for traversing Esri Arcade abstract syntax trees.
 *
 * \since QGIS 4.6
 */
class CORE_EXPORT QgsEsriArcadeVisitor
{
  public:
    virtual ~QgsEsriArcadeVisitor() = default;

    virtual void visit( const QgsEsriArcadeBlockStatement &node ) = 0;
    virtual void visit( const QgsEsriArcadeVariableDeclarationStatement &node ) = 0;
    virtual void visit( const QgsEsriArcadeAssignmentStatement &node ) = 0;
    virtual void visit( const QgsEsriArcadeReturnStatement &node ) = 0;
    virtual void visit( const QgsEsriArcadeFunctionDeclarationStatement &node ) = 0;
    virtual void visit( const QgsEsriArcadeExpressionStatement &node ) = 0;
    virtual void visit( const QgsEsriArcadeLiteralExpression &node ) = 0;
    virtual void visit( const QgsEsriArcadeVariableReferenceExpression &node ) = 0;
    virtual void visit( const QgsEsriArcadeProfileVariableReferenceExpression &node ) = 0;
    virtual void visit( const QgsEsriArcadeMemberAccessExpression &node ) = 0;
    virtual void visit( const QgsEsriArcadeBinaryOperatorExpression &node ) = 0;
    virtual void visit( const QgsEsriArcadeUnaryOperatorExpression &node ) = 0;
    virtual void visit( const QgsEsriArcadeFunctionCallExpression &node ) = 0;
    virtual void visit( const QgsEsriArcadeIndexExpression &node ) = 0;
    virtual void visit( const QgsEsriArcadeArrayLiteralExpression &node ) = 0;
};

/**
 * \ingroup core
 * \brief Visitor implementation that recursively traverses all child nodes in an Esri Arcade script.
 *
 * \since QGIS 4.6
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
};

/**
 * \ingroup core
 * \brief Parses and encapsulates an entire Esri Arcade script.
 *
 * \since QGIS 4.6
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
