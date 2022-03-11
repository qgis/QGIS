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

#define SIP_NO_FILE

#ifndef QGSPOINTCLOUDEXPRESSIONNODEIMPL_H
#define QGSPOINTCLOUDEXPRESSIONNODEIMPL_H

#include "qgspointcloudexpressionnode.h"
#include "qgspointcloudblock.h"
#include "qgsexpressionnodeimpl.h"

#define ENSURE_NO_EVAL_ERROR   {  if ( parent->hasEvalError() ) return std::numeric_limits<double>::quiet_NaN(); }

/**
 * \ingroup core
 * \brief A unary node is either negative as in boolean (not) or as in numbers (minus).
 *
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsPointCloudExpressionNodeUnaryOperator : public QgsPointCloudExpressionNode
{
  public:

    /**
     * \brief list of unary operators
     * \note if any change is made here, the definition of QgsPointCloudExpression::UnaryOperatorText[] must be adapted.
     */
    enum UnaryOperator
    {
      uoNot,
      uoMinus,
    };

    /**
     * A node unary operator is modifying the value of \a operand by negating it with \a op.
     */
    QgsPointCloudExpressionNodeUnaryOperator( QgsPointCloudExpressionNodeUnaryOperator::UnaryOperator op, QgsPointCloudExpressionNode *operand )
      : mOp( op )
      , mOperand( operand )
    {}
    ~QgsPointCloudExpressionNodeUnaryOperator() override { delete mOperand; }

    /**
     * Returns the unary operator.
     */
    QgsPointCloudExpressionNodeUnaryOperator::UnaryOperator op() const { return mOp; }

    /**
     * Returns the node the operator will operate upon.
     */
    QgsPointCloudExpressionNode *operand() const { return mOperand; }

    QgsPointCloudExpressionNode::NodeType nodeType() const override;
    bool prepareNode( QgsPointCloudExpression *parent, const QgsPointCloudBlock *block ) override;
    double evalNode( QgsPointCloudExpression *parent, int pointIndex ) override;
    QString dump() const override;

    QSet<QString> referencedAttributes() const override;
    QList<const QgsPointCloudExpressionNode *> nodes() const override;
    QgsPointCloudExpressionNode *clone() const override;

    bool isStatic( QgsPointCloudExpression *parent, const QgsPointCloudBlock *block ) const override;

    /**
     * Returns a the name of this operator without the operands.
     * I.e. "NOT" or "-"
     */
    QString text() const;

    static bool convert( const QgsExpressionNodeUnaryOperator::UnaryOperator source, QgsPointCloudExpressionNodeUnaryOperator::UnaryOperator &target );

  private:
    UnaryOperator mOp;
    QgsPointCloudExpressionNode *mOperand = nullptr;

    static const char *UNARY_OPERATOR_TEXT[];
};

/**
 * \brief A binary expression operator, which operates on two values.
 * \ingroup core
 *
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsPointCloudExpressionNodeBinaryOperator : public QgsPointCloudExpressionNode
{
  public:

    /**
     * \brief list of binary operators
     * \note if any change is made here, the definition of QgsPointCloudExpression::BinaryOperatorText[] must be adapted.
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
    QgsPointCloudExpressionNodeBinaryOperator( QgsPointCloudExpressionNodeBinaryOperator::BinaryOperator op, QgsPointCloudExpressionNode *opLeft, QgsPointCloudExpressionNode *opRight )
      : mOp( op )
      , mOpLeft( opLeft )
      , mOpRight( opRight )
    {}
    ~QgsPointCloudExpressionNodeBinaryOperator() override { delete mOpLeft; delete mOpRight; }

    /**
     * Returns the binary operator.
     */
    QgsPointCloudExpressionNodeBinaryOperator::BinaryOperator op() const { return mOp; }

    /**
     * Returns the node to the left of the operator.
     * \see opRight()
     */
    QgsPointCloudExpressionNode *opLeft() const { return mOpLeft; }

    /**
     * Returns the node to the right of the operator.
     * \see opLeft()
     */
    QgsPointCloudExpressionNode *opRight() const { return mOpRight; }

    QgsPointCloudExpressionNode::NodeType nodeType() const override;
    bool prepareNode( QgsPointCloudExpression *parent, const QgsPointCloudBlock *block ) override;
    double evalNode( QgsPointCloudExpression *parent, int pointIndex ) override;
    QString dump() const override;

    QSet<QString> referencedAttributes() const override;
    QList<const QgsPointCloudExpressionNode *> nodes( ) const override;

    QgsPointCloudExpressionNode *clone() const override;
    bool isStatic( QgsPointCloudExpression *parent, const QgsPointCloudBlock *block ) const override;

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

    static bool convert( const QgsExpressionNodeBinaryOperator::BinaryOperator source, QgsPointCloudExpressionNodeBinaryOperator::BinaryOperator &target );

  private:
    bool compare( double diff );

    BinaryOperator mOp;
    QgsPointCloudExpressionNode *mOpLeft = nullptr;
    QgsPointCloudExpressionNode *mOpRight = nullptr;

    static const char *BINARY_OPERATOR_TEXT[];
};

/**
 * \brief An expression node for value IN or NOT IN clauses.
 * \ingroup core
 *
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsPointCloudExpressionNodeInOperator : public QgsPointCloudExpressionNode
{
  public:

    /**
     * This node tests if the result of \a node is in the result of \a list. Optionally it can be inverted with \a notin which by default is FALSE.
     */
    QgsPointCloudExpressionNodeInOperator( QgsPointCloudExpressionNode *node, QgsPointCloudExpressionNode::NodeList *list, bool notin = false )
      : mNode( node )
      , mList( list )
      , mNotIn( notin )
    {}
    ~QgsPointCloudExpressionNodeInOperator() override;

    /**
     * Returns the expression node.
     */
    QgsPointCloudExpressionNode *node() const { return mNode; }

    /**
     * Returns TRUE if this node is a "NOT IN" operator, or FALSE if the node is a normal "IN" operator.
     */
    bool isNotIn() const { return mNotIn; }

    /**
     * Returns the list of nodes to search for matching values within.
     */
    QgsPointCloudExpressionNode::NodeList *list() const { return mList; }

    QgsPointCloudExpressionNode::NodeType nodeType() const override;
    bool prepareNode( QgsPointCloudExpression *parent, const QgsPointCloudBlock *block ) override;
    double evalNode( QgsPointCloudExpression *parent, int pointIndex ) override;
    QString dump() const override;

    QSet<QString> referencedAttributes() const override;
    QList<const QgsPointCloudExpressionNode *> nodes() const override;
    QgsPointCloudExpressionNode *clone() const override;
    bool isStatic( QgsPointCloudExpression *parent, const QgsPointCloudBlock *block ) const override;

  private:
    QgsPointCloudExpressionNode *mNode = nullptr;
    QgsPointCloudExpressionNodeInOperator::NodeList *mList = nullptr;
    bool mNotIn;
};

/**
 * \brief An expression node for literal values.
 * \ingroup core
 *
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsPointCloudExpressionNodeLiteral : public QgsPointCloudExpressionNode
{
  public:

    /**
     * Constructor for QgsPointCloudExpressionNodeLiteral, with the specified literal \a value.
     */
    QgsPointCloudExpressionNodeLiteral( const double &value )
      : mValue( value )
    {}

    //! The value of the literal.
    inline double value() const { return mValue; }

    QgsPointCloudExpressionNode::NodeType nodeType() const override;
    bool prepareNode( QgsPointCloudExpression *parent, const QgsPointCloudBlock *block ) override;
    double evalNode( QgsPointCloudExpression *parent, int pointIndex ) override;
    QString dump() const override;

    QSet<QString> referencedAttributes() const override;

    QList<const QgsPointCloudExpressionNode *> nodes() const override;
    QgsPointCloudExpressionNode *clone() const override;
    bool isStatic( QgsPointCloudExpression *parent, const QgsPointCloudBlock *block ) const override;

    /**
     * Returns a string representation of the node's literal value.
     */
    QString valueAsString() const;

  private:
    double mValue;
};

/**
 * \brief An expression node which takes it value from a feature's field.
 * \ingroup core
 *
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsPointCloudExpressionNodeAttributeRef : public QgsPointCloudExpressionNode
{
  public:

    /**
     * Constructor for QgsPointCloudExpressionNodeColumnRef, referencing the column
     * with the specified \a name.
     */
    QgsPointCloudExpressionNodeAttributeRef( const QString &name )
      : mName( name )
    {}

    //! The name of the column.
    QString name() const { return mName; }

    QgsPointCloudExpressionNode::NodeType nodeType() const override;
    bool prepareNode( QgsPointCloudExpression *parent, const QgsPointCloudBlock *block ) override;
    double evalNode( QgsPointCloudExpression *parent, int pointIndex ) override;
    QString dump() const override;

    QSet<QString> referencedAttributes() const override;
    QList<const QgsPointCloudExpressionNode *> nodes( ) const override;

    QgsPointCloudExpressionNode *clone() const override;
    bool isStatic( QgsPointCloudExpression *parent, const QgsPointCloudBlock *block ) const override;

  private:
    QString mName;
    const QgsPointCloudAttribute *mAttribute = nullptr;
    int mOffset = -1;
    const QgsPointCloudBlock *mBlock = nullptr;
};


#endif // QGSPOINTCLOUDEXPRESSIONNODEIMPL_H
