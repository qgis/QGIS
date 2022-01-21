/***************************************************************************
                       qgspointcloudexpressionnodeimpl.h
                       ---------------------------------
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


#ifndef QGSPOINTCLOUDEXPRESSIONNODEIMPL_H
#define QGSPOINTCLOUDEXPRESSIONNODEIMPL_H

#include "qgspointcloudexpressionnode.h"
#include "qgsinterval.h"

/**
 * \ingroup core
 * \brief A unary node is either negative as in boolean (not) or as in numbers (minus).
 */
class CORE_EXPORT QgsPointcloudExpressionNodeUnaryOperator : public QgsPointcloudExpressionNode
{
  public:

    /**
     * \brief list of unary operators
     * \note if any change is made here, the definition of QgsPointcloudExpression::UnaryOperatorText[] must be adapted.
     */
    enum UnaryOperator
    {
      uoNot,
      uoMinus,
    };

    /**
     * A node unary operator is modifying the value of \a operand by negating it with \a op.
     */
    QgsPointcloudExpressionNodeUnaryOperator( QgsPointcloudExpressionNodeUnaryOperator::UnaryOperator op, QgsPointcloudExpressionNode *operand SIP_TRANSFER )
      : mOp( op )
      , mOperand( operand )
    {}
    ~QgsPointcloudExpressionNodeUnaryOperator() override { delete mOperand; }

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsPointcloudExpressionNodeUnaryOperator: %1>" ).arg( sipCpp->text() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

    /**
     * Returns the unary operator.
     */
    QgsPointcloudExpressionNodeUnaryOperator::UnaryOperator op() const { return mOp; }

    /**
     * Returns the node the operator will operate upon.
     */
    QgsPointcloudExpressionNode *operand() const { return mOperand; }

    QgsPointcloudExpressionNode::NodeType nodeType() const override;
    bool prepareNode( QgsPointcloudExpression *parent, const QgsPointCloudAttributeCollection &attributes ) override;
    QVariant evalNode( QgsPointcloudExpression *parent, QVariantMap &p ) override;
    QString dump() const override;

    QSet<QString> referencedAttributes() const override;
    QList<const QgsPointcloudExpressionNode *> nodes() const override; SIP_SKIP
    QgsPointcloudExpressionNode *clone() const override SIP_FACTORY;

    bool isStatic( QgsPointcloudExpression *parent, const QgsPointCloudAttributeCollection &attributes ) const override;

    /**
     * Returns a the name of this operator without the operands.
     * I.e. "NOT" or "-"
     */
    QString text() const;

  private:
    UnaryOperator mOp;
    QgsPointcloudExpressionNode *mOperand = nullptr;

    static const char *UNARY_OPERATOR_TEXT[];
};

/**
 * \brief A binary expression operator, which operates on two values.
 * \ingroup core
 */
class CORE_EXPORT QgsPointcloudExpressionNodeBinaryOperator : public QgsPointcloudExpressionNode
{
  public:

    /**
     * \brief list of binary operators
     * \note if any change is made here, the definition of QgsPointcloudExpression::BinaryOperatorText[] must be adapted.
     */
    enum BinaryOperator
    {
      // logical
      boOr,
      boAnd,

      // comparison
      boEQ,  //!< =
      boNE,  //!< <>
      boLE,  //!< <=
      boGE,  //!< >=
      boLT,  //!< <
      boGT,  //!< >

      // math
      boPlus,
      boMinus,
      boMul,
      boDiv,
      boIntDiv,
      boMod,
      boPow,
    };

    /**
     * Binary combination of the left and the right with op.
     */
    QgsPointcloudExpressionNodeBinaryOperator( QgsPointcloudExpressionNodeBinaryOperator::BinaryOperator op, QgsPointcloudExpressionNode *opLeft SIP_TRANSFER, QgsPointcloudExpressionNode *opRight SIP_TRANSFER )
      : mOp( op )
      , mOpLeft( opLeft )
      , mOpRight( opRight )
    {}
    ~QgsPointcloudExpressionNodeBinaryOperator() override { delete mOpLeft; delete mOpRight; }

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsPointcloudExpressionNodeBinaryOperator: %1>" ).arg( sipCpp->text() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

    /**
     * Returns the binary operator.
     */
    QgsPointcloudExpressionNodeBinaryOperator::BinaryOperator op() const { return mOp; }

    /**
     * Returns the node to the left of the operator.
     * \see opRight()
     */
    QgsPointcloudExpressionNode *opLeft() const { return mOpLeft; }

    /**
     * Returns the node to the right of the operator.
     * \see opLeft()
     */
    QgsPointcloudExpressionNode *opRight() const { return mOpRight; }

    QgsPointcloudExpressionNode::NodeType nodeType() const override;
    bool prepareNode( QgsPointcloudExpression *parent, const QgsPointCloudAttributeCollection &attributes ) override;
    QVariant evalNode( QgsPointcloudExpression *parent, QVariantMap &p ) override;
    QString dump() const override;

    QSet<QString> referencedAttributes() const override;
    QList<const QgsPointcloudExpressionNode *> nodes( ) const override; SIP_SKIP

    QgsPointcloudExpressionNode *clone() const override SIP_FACTORY;
    bool isStatic( QgsPointcloudExpression *parent, const QgsPointCloudAttributeCollection &attributes ) const override;

    /**
     * Returns the precedence index for the operator. Higher values have higher precedence.
     */
    int precedence() const;

    /**
     * Returns TRUE if the operator is left-associative.
     */
    bool leftAssociative() const;

    /**
     * Returns a the name of this operator without the operands.
     * I.e. "AND", "OR", ...
     */
    QString text() const;

  private:
    bool compare( double diff );
    qlonglong computeInt( qlonglong x, qlonglong y );
    double computeDouble( double x, double y );

    BinaryOperator mOp;
    QgsPointcloudExpressionNode *mOpLeft = nullptr;
    QgsPointcloudExpressionNode *mOpRight = nullptr;

    static const char *BINARY_OPERATOR_TEXT[];
};

/**
 * \brief An expression node for value IN or NOT IN clauses.
 * \ingroup core
 */
class CORE_EXPORT QgsPointcloudExpressionNodeInOperator : public QgsPointcloudExpressionNode
{
  public:

    /**
     * This node tests if the result of \a node is in the result of \a list. Optionally it can be inverted with \a notin which by default is FALSE.
     */
    QgsPointcloudExpressionNodeInOperator( QgsPointcloudExpressionNode *node SIP_TRANSFER, QgsPointcloudExpressionNode::NodeList *list SIP_TRANSFER, bool notin = false )
      : mNode( node )
      , mList( list )
      , mNotIn( notin )
    {}
    ~QgsPointcloudExpressionNodeInOperator() override;

    /**
     * Returns the expression node.
     */
    QgsPointcloudExpressionNode *node() const { return mNode; }

    /**
     * Returns TRUE if this node is a "NOT IN" operator, or FALSE if the node is a normal "IN" operator.
     */
    bool isNotIn() const { return mNotIn; }

    /**
     * Returns the list of nodes to search for matching values within.
     */
    QgsPointcloudExpressionNode::NodeList *list() const { return mList; }

    QgsPointcloudExpressionNode::NodeType nodeType() const override;
    bool prepareNode( QgsPointcloudExpression *parent, const QgsPointCloudAttributeCollection &attributes ) override;
    QVariant evalNode( QgsPointcloudExpression *parent, QVariantMap &p ) override;
    QString dump() const override;

    QSet<QString> referencedAttributes() const override;
    QList<const QgsPointcloudExpressionNode *> nodes() const override; SIP_SKIP
    QgsPointcloudExpressionNode *clone() const override SIP_FACTORY;
    bool isStatic( QgsPointcloudExpression *parent, const QgsPointCloudAttributeCollection &attributes ) const override;

  private:
    QgsPointcloudExpressionNode *mNode = nullptr;
    QgsPointcloudExpressionNodeInOperator::NodeList *mList = nullptr;
    bool mNotIn;
};

/**
 * \brief An expression node for literal values.
 * \ingroup core
 */
class CORE_EXPORT QgsPointcloudExpressionNodeLiteral : public QgsPointcloudExpressionNode
{
  public:

    /**
     * Constructor for QgsPointcloudExpressionNodeLiteral, with the specified literal \a value.
     */
    QgsPointcloudExpressionNodeLiteral( const QVariant &value )
      : mValue( value )
    {}

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsPointcloudExpressionNodeLiteral: %1>" ).arg( sipCpp->valueAsString() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

    //! The value of the literal.
    inline QVariant value() const { return mValue; }

    QgsPointcloudExpressionNode::NodeType nodeType() const override;
    bool prepareNode( QgsPointcloudExpression *parent, const QgsPointCloudAttributeCollection &attributes ) override;
    QVariant evalNode( QgsPointcloudExpression *parent, QVariantMap &p ) override;
    QString dump() const override;

    QSet<QString> referencedAttributes() const override;

    QList<const QgsPointcloudExpressionNode *> nodes() const override; SIP_SKIP
    QgsPointcloudExpressionNode *clone() const override SIP_FACTORY;
    bool isStatic( QgsPointcloudExpression *parent, const QgsPointCloudAttributeCollection &attributes ) const override;

    /**
     * Returns a string representation of the node's literal value.
     *
     * \since QGIS 3.20
     */
    QString valueAsString() const;

  private:
    QVariant mValue;
};

/**
 * \brief An expression node which takes it value from a feature's field.
 * \ingroup core
 */
class CORE_EXPORT QgsPointcloudExpressionNodeAttributeRef : public QgsPointcloudExpressionNode
{
  public:

    /**
     * Constructor for QgsPointcloudExpressionNodeColumnRef, referencing the column
     * with the specified \a name.
     */
    QgsPointcloudExpressionNodeAttributeRef( const QString &name )
      : mName( name )
      , mIndex( -1 )
    {}

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsPointcloudExpressionNodeAttributeRef: \"%1\">" ).arg( sipCpp->name() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

    //! The name of the column.
    QString name() const { return mName; }

    QgsPointcloudExpressionNode::NodeType nodeType() const override;
    bool prepareNode( QgsPointcloudExpression *parent, const QgsPointCloudAttributeCollection &attributes ) override;
    QVariant evalNode( QgsPointcloudExpression *parent, QVariantMap &p ) override;
    QString dump() const override;

    QSet<QString> referencedAttributes() const override;
    QList<const QgsPointcloudExpressionNode *> nodes( ) const override; SIP_SKIP

    QgsPointcloudExpressionNode *clone() const override SIP_FACTORY;
    bool isStatic( QgsPointcloudExpression *parent, const QgsPointCloudAttributeCollection &attributes ) const override;

  private:
    QString mName;
    int mIndex;
};


#endif // QGSPOINTCLOUDEXPRESSIONNODEIMPL_H
