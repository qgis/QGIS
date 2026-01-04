/***************************************************************************
                               qgsexpressionnodeimpl.h
                             -------------------
    begin                : May 2017
    copyright            : (C) 2017 Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSEXPRESSIONNODEIMPL_H
#define QGSEXPRESSIONNODEIMPL_H

#include "qgsexpressionnode.h"
#include "qgsinterval.h"

/**
 * \ingroup core
 * \brief A unary node is either negative as in boolean (not) or as in numbers (minus).
 */
class CORE_EXPORT QgsExpressionNodeUnaryOperator : public QgsExpressionNode
{
  public:

    /**
     * \brief list of unary operators
     * \note if any change is made here, the definition of QgsExpression::UnaryOperatorText[] must be adapted.
     */
    enum UnaryOperator
    {
      uoNot,
      uoMinus,
    };

    /**
     * A node unary operator is modifying the value of \a operand by negating it with \a op.
     */
    QgsExpressionNodeUnaryOperator( QgsExpressionNodeUnaryOperator::UnaryOperator op, QgsExpressionNode *operand SIP_TRANSFER )
      : mOp( op )
      , mOperand( operand )
    {}

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = u"<QgsExpressionNodeUnaryOperator: %1>"_s.arg( sipCpp->text() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

    /**
     * Returns the unary operator.
     */
    QgsExpressionNodeUnaryOperator::UnaryOperator op() const { return mOp; }

    /**
     * Returns the node the operator will operate upon.
     */
    QgsExpressionNode *operand() const { return mOperand.get(); }

    QgsExpressionNode::NodeType nodeType() const override;
    bool prepareNode( QgsExpression *parent, const QgsExpressionContext *context ) override;
    QVariant evalNode( QgsExpression *parent, const QgsExpressionContext *context ) override;
    QString dump() const override;

    QSet<QString> referencedColumns() const override;
    QSet<QString> referencedVariables() const override;
    QSet<QString> referencedFunctions() const override;
    QList<const QgsExpressionNode *> nodes() const override; SIP_SKIP
    bool needsGeometry() const override;
    QgsExpressionNode *clone() const override SIP_FACTORY;

    bool isStatic( QgsExpression *parent, const QgsExpressionContext *context ) const override;

    /**
     * Returns a the name of this operator without the operands.
     * I.e. "NOT" or "-"
     */
    QString text() const;

  private:

    QgsExpressionNodeUnaryOperator( const QgsExpressionNodeUnaryOperator &other ) = delete;
    QgsExpressionNodeUnaryOperator &operator=( const QgsExpressionNodeUnaryOperator &other ) = delete;

#ifdef SIP_RUN
    QgsExpressionNodeUnaryOperator( const QgsExpressionNodeUnaryOperator &other );
#endif

    UnaryOperator mOp;
    std::unique_ptr<QgsExpressionNode> mOperand;

    static const char *UNARY_OPERATOR_TEXT[];
};

/**
 * \brief A binary expression operator, which operates on two values.
 * \ingroup core
 */
class CORE_EXPORT QgsExpressionNodeBinaryOperator : public QgsExpressionNode
{
  public:

    /**
     * \brief list of binary operators
     * \note if any change is made here, the definition of QgsExpression::BinaryOperatorText[] must be adapted.
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
      boRegexp,
      boLike,
      boNotLike,
      boILike,
      boNotILike,
      boIs,
      boIsNot,

      // math
      boPlus,
      boMinus,
      boMul,
      boDiv,
      boIntDiv,
      boMod,
      boPow,

      // strings
      boConcat,
    };

    /**
     * Binary combination of the left and the right with op.
     */
    QgsExpressionNodeBinaryOperator( QgsExpressionNodeBinaryOperator::BinaryOperator op, QgsExpressionNode *opLeft SIP_TRANSFER, QgsExpressionNode *opRight SIP_TRANSFER )
      : mOp( op )
      , mOpLeft( opLeft )
      , mOpRight( opRight )
    {}

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = u"<QgsExpressionNodeBinaryOperator: %1>"_s.arg( sipCpp->text() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

    /**
     * Returns the binary operator.
     */
    QgsExpressionNodeBinaryOperator::BinaryOperator op() const { return mOp; }

    /**
     * Returns the node to the left of the operator.
     * \see opRight()
     */
    QgsExpressionNode *opLeft() const { return mOpLeft.get(); }

    /**
     * Returns the node to the right of the operator.
     * \see opLeft()
     */
    QgsExpressionNode *opRight() const { return mOpRight.get(); }

    QgsExpressionNode::NodeType nodeType() const override;
    bool prepareNode( QgsExpression *parent, const QgsExpressionContext *context ) override;
    QVariant evalNode( QgsExpression *parent, const QgsExpressionContext *context ) override;
    QString dump() const override;

    QSet<QString> referencedColumns() const override;
    QSet<QString> referencedVariables() const override;
    QSet<QString> referencedFunctions() const override;
    QList<const QgsExpressionNode *> nodes( ) const override; SIP_SKIP

    bool needsGeometry() const override;
    QgsExpressionNode *clone() const override SIP_FACTORY;
    bool isStatic( QgsExpression *parent, const QgsExpressionContext *context ) const override;

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

    QgsExpressionNodeBinaryOperator( const QgsExpressionNodeBinaryOperator &other ) = delete;
    QgsExpressionNodeBinaryOperator &operator=( const QgsExpressionNodeBinaryOperator &other ) = delete;

#ifdef SIP_RUN
    QgsExpressionNodeBinaryOperator( const QgsExpressionNodeBinaryOperator &other );
#endif
    qlonglong computeInt( qlonglong x, qlonglong y );
    double computeDouble( double x, double y );

    /**
     * Computes the result date time calculation from a start datetime and an interval
     * \param d start datetime
     * \param i interval to add or subtract (depending on mOp)
     */
    QDateTime computeDateTimeFromInterval( const QDateTime &d, QgsInterval *i );

    /**
     * Compares two values, which are guaranteed to be non-null, using the specified operator.
     */
    static QVariant compareNonNullValues( QgsExpression *parent, const QgsExpressionContext *context, const QVariant &vL, const QVariant &vR, BinaryOperator op );

    BinaryOperator mOp;
    std::unique_ptr<QgsExpressionNode> mOpLeft;
    std::unique_ptr<QgsExpressionNode> mOpRight;

    static const char *BINARY_OPERATOR_TEXT[];
};

/**
 * \brief An indexing expression operator, which allows use of square brackets [] to reference map and array items.
 * \ingroup core
 * \since QGIS 3.6
 */
class CORE_EXPORT QgsExpressionNodeIndexOperator : public QgsExpressionNode
{
  public:

    /**
     * Constructor for QgsExpressionNodeIndexOperator.
     */
    QgsExpressionNodeIndexOperator( QgsExpressionNode *container SIP_TRANSFER, QgsExpressionNode *index SIP_TRANSFER )
      : mContainer( container )
      , mIndex( index )
    {}

    /**
     * Returns the container node, representing an array or map value.
     * \see index()
     */
    QgsExpressionNode *container() const { return mContainer.get(); }

    /**
     * Returns the index node, representing an array element index or map key.
     * \see container()
     */
    QgsExpressionNode *index() const { return mIndex.get(); }

    QgsExpressionNode::NodeType nodeType() const override;
    bool prepareNode( QgsExpression *parent, const QgsExpressionContext *context ) override;
    QVariant evalNode( QgsExpression *parent, const QgsExpressionContext *context ) override;
    QString dump() const override;

    QSet<QString> referencedColumns() const override;
    QSet<QString> referencedVariables() const override;
    QSet<QString> referencedFunctions() const override;
    QList<const QgsExpressionNode *> nodes( ) const override; SIP_SKIP

    bool needsGeometry() const override;
    QgsExpressionNode *clone() const override SIP_FACTORY;
    bool isStatic( QgsExpression *parent, const QgsExpressionContext *context ) const override;

  private:

    QgsExpressionNodeIndexOperator( const QgsExpressionNodeIndexOperator &other ) = delete;
    QgsExpressionNodeIndexOperator &operator=( const QgsExpressionNodeIndexOperator &other ) = delete;

#ifdef SIP_RUN
    QgsExpressionNodeIndexOperator( const QgsExpressionNodeIndexOperator &other );
#endif

    std::unique_ptr<QgsExpressionNode> mContainer;
    std::unique_ptr<QgsExpressionNode> mIndex;

};

/**
 * \brief SQL-like BETWEEN and NOT BETWEEN predicates.
 * \ingroup core
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsExpressionNodeBetweenOperator: public QgsExpressionNode
{
  public:

    /**
     * This node tests if the result of \a node is between the result of \a nodeLowerBound and \a nodeHigherBound nodes. Optionally it can be inverted with \a negate which by default is FALSE.
     */
    QgsExpressionNodeBetweenOperator( QgsExpressionNode *node SIP_TRANSFER, QgsExpressionNode *nodeLowerBound SIP_TRANSFER, QgsExpressionNode *nodeHigherBound SIP_TRANSFER, bool negate = false )
      : mNode( node )
      , mLowerBound( nodeLowerBound )
      , mHigherBound( nodeHigherBound )
      , mNegate( negate )
    {}

    ~QgsExpressionNodeBetweenOperator() override;

    /**
     * Returns the expression node.
     */
    QgsExpressionNode *node() const { return mNode.get(); }


    QgsExpressionNode::NodeType nodeType() const override;
    bool prepareNode( QgsExpression *parent, const QgsExpressionContext *context ) override;
    QVariant evalNode( QgsExpression *parent, const QgsExpressionContext *context ) override;
    QString dump() const override;

    QSet<QString> referencedColumns() const override;
    QSet<QString> referencedVariables() const override;
    QSet<QString> referencedFunctions() const override;
    QList<const QgsExpressionNode *> nodes() const override; SIP_SKIP
    bool needsGeometry() const override;
    QgsExpressionNode *clone() const override SIP_FACTORY;
    bool isStatic( QgsExpression *parent, const QgsExpressionContext *context ) const override;

    /**
     * Returns the lower bound expression node of the range.
     */
    QgsExpressionNode *lowerBound() const;

    /**
     * Returns the higher bound expression node of the range.
     */
    QgsExpressionNode *higherBound() const;

    /**
     * Returns TRUE if the predicate is an exclusion test (NOT BETWEEN).
     */
    bool negate() const;

  private:

    QgsExpressionNodeBetweenOperator( const QgsExpressionNodeBetweenOperator &other ) = delete;
    QgsExpressionNodeBetweenOperator &operator=( const QgsExpressionNodeBetweenOperator &other ) = delete;

#ifdef SIP_RUN
    QgsExpressionNodeBetweenOperator( const QgsExpressionNodeBetweenOperator &other );
#endif


    std::unique_ptr<QgsExpressionNode> mNode;
    std::unique_ptr<QgsExpressionNode> mLowerBound;
    std::unique_ptr<QgsExpressionNode> mHigherBound;
    bool mNegate = false;

};

/**
 * \brief An expression node for value IN or NOT IN clauses.
 * \ingroup core
 */
class CORE_EXPORT QgsExpressionNodeInOperator : public QgsExpressionNode
{
  public:

    /**
     * This node tests if the result of \a node is in the result of \a list. Optionally it can be inverted with \a notin which by default is FALSE.
     */
    QgsExpressionNodeInOperator( QgsExpressionNode *node SIP_TRANSFER, QgsExpressionNode::NodeList *list SIP_TRANSFER, bool notin = false )
      : mNode( node )
      , mList( list )
      , mNotIn( notin )
    {}
    ~QgsExpressionNodeInOperator() override;

    /**
     * Returns the expression node.
     */
    QgsExpressionNode *node() const { return mNode.get(); }

    /**
     * Returns TRUE if this node is a "NOT IN" operator, or FALSE if the node is a normal "IN" operator.
     */
    bool isNotIn() const { return mNotIn; }

    /**
     * Returns the list of nodes to search for matching values within.
     */
    QgsExpressionNode::NodeList *list() const { return mList.get(); }

    QgsExpressionNode::NodeType nodeType() const override;
    bool prepareNode( QgsExpression *parent, const QgsExpressionContext *context ) override;
    QVariant evalNode( QgsExpression *parent, const QgsExpressionContext *context ) override;
    QString dump() const override;

    QSet<QString> referencedColumns() const override;
    QSet<QString> referencedVariables() const override;
    QSet<QString> referencedFunctions() const override;
    QList<const QgsExpressionNode *> nodes() const override; SIP_SKIP
    bool needsGeometry() const override;
    QgsExpressionNode *clone() const override SIP_FACTORY;
    bool isStatic( QgsExpression *parent, const QgsExpressionContext *context ) const override;

  private:

    QgsExpressionNodeInOperator( const QgsExpressionNodeInOperator &other ) = delete;
    QgsExpressionNodeInOperator &operator=( const QgsExpressionNodeInOperator &other ) = delete;

#ifdef SIP_RUN
    QgsExpressionNodeInOperator( const QgsExpressionNodeInOperator &other );
#endif

    std::unique_ptr<QgsExpressionNode> mNode;
    std::unique_ptr<QgsExpressionNodeInOperator::NodeList> mList;
    bool mNotIn;
};

/**
 * \brief An expression node for expression functions.
 * \ingroup core
 */
class CORE_EXPORT QgsExpressionNodeFunction : public QgsExpressionNode
{
  public:

    /**
     * A function node consists of an index of the function in the global function array and
     * a list of arguments that will be passed to it.
     */
    QgsExpressionNodeFunction( int fnIndex, QgsExpressionNode::NodeList *args SIP_TRANSFER );

    ~QgsExpressionNodeFunction() override;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString function;
    if ( QgsExpressionFunction *fd = QgsExpression::QgsExpression::Functions()[sipCpp->fnIndex()] )
    {
      function = fd->name();
    }
    else
    {
      function = QString::number( sipCpp->fnIndex() );
    }

    QString str = u"<QgsExpressionNodeFunction: %1>"_s.arg( function );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

    /**
     * Returns the index of the node's function.
     */
    int fnIndex() const { return mFnIndex; }

    /**
     * Returns a list of arguments specified for the function.
     */
    QgsExpressionNode::NodeList *args() const { return mArgs.get(); }

    QgsExpressionNode::NodeType nodeType() const override;
    bool prepareNode( QgsExpression *parent, const QgsExpressionContext *context ) override;
    QVariant evalNode( QgsExpression *parent, const QgsExpressionContext *context ) override;
    QString dump() const override;

    QSet<QString> referencedColumns() const override;
    QSet<QString> referencedVariables() const override;
    QSet<QString> referencedFunctions() const override;

    QList<const QgsExpressionNode *> nodes() const override; SIP_SKIP
    bool needsGeometry() const override;
    QgsExpressionNode *clone() const override SIP_FACTORY;
    bool isStatic( QgsExpression *parent, const QgsExpressionContext *context ) const override;

    //! Tests whether the provided argument list is valid for the matching function
    static bool validateParams( int fnIndex, QgsExpressionNode::NodeList *args, QString &error );

  private:

    QgsExpressionNodeFunction( const QgsExpressionNodeFunction &other ) = delete;
    QgsExpressionNodeFunction &operator=( const QgsExpressionNodeFunction &other ) = delete;

#ifdef SIP_RUN
    QgsExpressionNodeFunction( const QgsExpressionNodeFunction &other );
#endif



    int mFnIndex;
    std::unique_ptr<NodeList> mArgs;
};

/**
 * \brief An expression node for literal values.
 * \ingroup core
 */
class CORE_EXPORT QgsExpressionNodeLiteral : public QgsExpressionNode
{
  public:

    /**
     * Constructor for QgsExpressionNodeLiteral, with the specified literal \a value.
     */
    QgsExpressionNodeLiteral( const QVariant &value )
      : mValue( value )
    {}

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = u"<QgsExpressionNodeLiteral: %1>"_s.arg( sipCpp->valueAsString() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

    //! The value of the literal.
    inline QVariant value() const { return mValue; }

    QgsExpressionNode::NodeType nodeType() const override;
    bool prepareNode( QgsExpression *parent, const QgsExpressionContext *context ) override;
    QVariant evalNode( QgsExpression *parent, const QgsExpressionContext *context ) override;
    QString dump() const override;

    QSet<QString> referencedColumns() const override;
    QSet<QString> referencedVariables() const override;
    QSet<QString> referencedFunctions() const override;

    QList<const QgsExpressionNode *> nodes() const override; SIP_SKIP
    bool needsGeometry() const override;
    QgsExpressionNode *clone() const override SIP_FACTORY;
    bool isStatic( QgsExpression *parent, const QgsExpressionContext *context ) const override;

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
 * \brief An expression node which takes its value from a feature's field.
 * \ingroup core
 */
class CORE_EXPORT QgsExpressionNodeColumnRef : public QgsExpressionNode
{
  public:

    /**
     * Constructor for QgsExpressionNodeColumnRef, referencing the column
     * with the specified \a name.
     */
    QgsExpressionNodeColumnRef( const QString &name )
      : mName( name )
    {}

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = u"<QgsExpressionNodeColumnRef: \"%1\">"_s.arg( sipCpp->name() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

    //! The name of the column.
    QString name() const { return mName; }

    QgsExpressionNode::NodeType nodeType() const override;
    bool prepareNode( QgsExpression *parent, const QgsExpressionContext *context ) override;
    QVariant evalNode( QgsExpression *parent, const QgsExpressionContext *context ) override;
    QString dump() const override;

    QSet<QString> referencedColumns() const override;
    QSet<QString> referencedVariables() const override;
    QSet<QString> referencedFunctions() const override;
    QList<const QgsExpressionNode *> nodes( ) const override; SIP_SKIP

    bool needsGeometry() const override;

    QgsExpressionNode *clone() const override SIP_FACTORY;
    bool isStatic( QgsExpression *parent, const QgsExpressionContext *context ) const override;

  private:
    QString mName;
    int mIndex = -1;
};

/**
 * \brief An expression node for CASE WHEN clauses.
 * \ingroup core
 */
class CORE_EXPORT QgsExpressionNodeCondition : public QgsExpressionNode
{
  public:

    /**
     * \brief Represents a "WHEN... THEN..." portation of a CASE WHEN clause in an expression.
     * \ingroup core
     */
    class CORE_EXPORT WhenThen
    {
      public:

        /**
         * A combination of when and then. Simple as that.
         */
        WhenThen( QgsExpressionNode *whenExp, QgsExpressionNode *thenExp );
        ~WhenThen();

        WhenThen( const WhenThen &rh ) = delete;
        WhenThen &operator=( const WhenThen &rh ) = delete;

        /**
         * Gets a deep copy of this WhenThen combination.
         */
        QgsExpressionNodeCondition::WhenThen *clone() const SIP_FACTORY;

        /**
         * The expression that makes the WHEN part of the condition.
         * \return The expression node that makes the WHEN part of the condition check.
         */
        QgsExpressionNode *whenExp() const { return mWhenExp.get(); }

        /**
         * The expression node that makes the THEN result part of the condition.
         * \return The expression node that makes the THEN result part of the condition.
         */

        QgsExpressionNode *thenExp() const { return mThenExp.get(); }

      private:
#ifdef SIP_RUN
        WhenThen( const QgsExpressionNodeCondition::WhenThen &rh );
#endif
        std::unique_ptr<QgsExpressionNode> mWhenExp;
        std::unique_ptr<QgsExpressionNode> mThenExp;

        friend class QgsExpressionNodeCondition;
    };
    typedef QList<QgsExpressionNodeCondition::WhenThen *> WhenThenList;

    /**
     * Create a new node with the given list of \a conditions and an optional \a elseExp expression.
     */
    QgsExpressionNodeCondition( QgsExpressionNodeCondition::WhenThenList *conditions, QgsExpressionNode *elseExp = nullptr );

    /**
     * Create a new node with the given list of \a conditions and an optional \a elseExp expression.
     */
    QgsExpressionNodeCondition( const QgsExpressionNodeCondition::WhenThenList &conditions, QgsExpressionNode *elseExp = nullptr ) SIP_SKIP
  : mConditions( conditions )
    , mElseExp( elseExp )
    {}

    ~QgsExpressionNodeCondition() override;

    QgsExpressionNode::NodeType nodeType() const override;
    QVariant evalNode( QgsExpression *parent, const QgsExpressionContext *context ) override;
    bool prepareNode( QgsExpression *parent, const QgsExpressionContext *context ) override;
    QString dump() const override;

    /**
     * The list of WHEN THEN expression parts of the expression.
     * \return The list of WHEN THEN expression parts of the expression.
     */
    WhenThenList conditions() const { return mConditions; }

    /**
     * The ELSE expression used for the condition.
     * \return The ELSE expression used for the condition.
     */
    QgsExpressionNode *elseExp() const { return mElseExp.get(); }

    QSet<QString> referencedColumns() const override;
    QSet<QString> referencedVariables() const override;
    QSet<QString> referencedFunctions() const override;

    QList<const QgsExpressionNode *> nodes() const override; SIP_SKIP

    bool needsGeometry() const override;
    QgsExpressionNode *clone() const override SIP_FACTORY;
    bool isStatic( QgsExpression *parent, const QgsExpressionContext *context ) const override;

  private:

    QgsExpressionNodeCondition( const QgsExpressionNodeCondition &other ) = delete;
    QgsExpressionNodeCondition &operator=( const QgsExpressionNodeCondition &other ) = delete;

#ifdef SIP_RUN
    QgsExpressionNodeCondition( const QgsExpressionNodeCondition &other );
#endif


    WhenThenList mConditions;
    std::unique_ptr<QgsExpressionNode> mElseExp;
};


#endif // QGSEXPRESSIONNODEIMPL_H
