/***************************************************************************
                               qgsexpressionnode.h
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


#ifndef QGSEXPRESSIONNODE_H
#define QGSEXPRESSIONNODE_H

#include <QSet>
#include <QVariant>
#include <QCoreApplication>

#include "qgis.h"

class QgsExpression;
class QgsExpressionContext;

/**
 * \ingroup core
 *
 * \brief Abstract base class for all nodes that can appear in an expression.
 */
class CORE_EXPORT QgsExpressionNode SIP_ABSTRACT
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    switch ( sipCpp->nodeType() )
    {
      case QgsExpressionNode::ntUnaryOperator:
        sipType = sipType_QgsExpressionNodeUnaryOperator;
        break;
      case QgsExpressionNode::ntBinaryOperator:
        sipType = sipType_QgsExpressionNodeBinaryOperator;
        break;
      case QgsExpressionNode::ntInOperator:
        sipType = sipType_QgsExpressionNodeInOperator;
        break;
      case QgsExpressionNode::ntFunction:
        sipType = sipType_QgsExpressionNodeFunction;
        break;
      case QgsExpressionNode::ntLiteral:
        sipType = sipType_QgsExpressionNodeLiteral;
        break;
      case QgsExpressionNode::ntColumnRef:
        sipType = sipType_QgsExpressionNodeColumnRef;
        break;
      case QgsExpressionNode::ntCondition:
        sipType = sipType_QgsExpressionNodeCondition;
        break;
      case QgsExpressionNode::ntBetweenOperator:
        sipType = sipType_QgsExpressionNodeBetweenOperator;
        break;
      default:
        sipType = 0;
        break;
    }
    SIP_END
#endif

    Q_DECLARE_TR_FUNCTIONS( QgsExpressionNode )

  public:

    //! Known node types.
    enum NodeType
    {
      ntUnaryOperator, //!< \see QgsExpression::Node::NodeUnaryOperator
      ntBinaryOperator, //!< \see QgsExpression::Node::NodeBinaryOperator
      ntInOperator, //!< \see QgsExpression::Node::NodeInOperator
      ntFunction,  //!< \see QgsExpression::Node::NodeFunction
      ntLiteral, //!< \see QgsExpression::Node::NodeLiteral
      ntColumnRef, //!< \see QgsExpression::Node::NodeColumnRef
      ntCondition, //!< \see QgsExpression::Node::NodeCondition
      ntIndexOperator, //!< Index operator
      ntBetweenOperator, //!< Between operator \since QGIS 3.26
    };


    /**
     * \brief Named node
     * \ingroup core
     * \since QGIS 2.16
     */
    struct NamedNode
    {
      public:

        /**
         * Constructor for NamedNode
         * \param name node name
         * \param node node
         */
        NamedNode( const QString &name, QgsExpressionNode *node )
          : name( name )
          , node( node )
        {}

        //! Node name
        QString name;

        //! Node
        QgsExpressionNode *node = nullptr;
    };

    /**
     * \brief A list of expression nodes.
     * \ingroup core
     */
    class CORE_EXPORT NodeList
    {
      public:
        virtual ~NodeList();
        //! Takes ownership of the provided node
        void append( QgsExpressionNode *node SIP_TRANSFER ) { mList.append( node ); mNameList.append( QString() ); }

        /**
         * Adds a named node. Takes ownership of the provided node.
         * \since QGIS 2.16
        */
        void append( QgsExpressionNode::NamedNode *node SIP_TRANSFER );

        /**
         * Returns the number of nodes in the list.
         */
        int count() const { return mList.count(); }

        /**
         * Returns TRUE if list contains any named nodes
         * \since QGIS 2.16
         */
        bool hasNamedNodes() const { return mHasNamedNodes; }

        /**
         * Gets a list of all the nodes.
         */
        QList<QgsExpressionNode *> list() { return mList; }

        /**
         * Gets the node at position i in the list.
         *
         * \since QGIS 3.0
         */
        QgsExpressionNode *at( int i ) { return mList.at( i ); }

        /**
         * Returns a list of names for nodes. Unnamed nodes will be indicated by an empty string in the list.
         * \since QGIS 2.16
         */
        QStringList names() const { return mNameList; }

        //! Creates a deep copy of this list. Ownership is transferred to the caller
        QgsExpressionNode::NodeList *clone() const SIP_FACTORY;

        /**
         * Returns a string dump of the expression node.
         */
        virtual QString dump() const;

      private:
        QList<QgsExpressionNode *> mList;
        QStringList mNameList;

        bool mHasNamedNodes = false;

        /**
         * Cleans up and standardises the name of a named node.
         */
        static QString cleanNamedNodeName( const QString &name );

      public:
    };

    virtual ~QgsExpressionNode() = default;

    /**
     * Gets the type of this node.
     *
     * \returns The type of this node
     */
    virtual QgsExpressionNode::NodeType nodeType() const = 0;

    /**
     * Dump this node into a serialized (part) of an expression.
     * The returned expression does not necessarily literally match
     * the original expression, it's just guaranteed to behave the same way.
     */
    virtual QString dump() const = 0;

    /**
     * Evaluate this node with the given context and parent.
     * This will return a cached value if it has been determined to be static
     * during the prepare() execution.
     *
     * \since QGIS 2.12
     */
    QVariant eval( QgsExpression *parent, const QgsExpressionContext *context );

    /**
     * Generate a clone of this node.
     * Ownership is transferred to the caller.
     *
     * \returns a deep copy of this node.
     */
    virtual QgsExpressionNode *clone() const = 0;

    /**
     * Abstract virtual method which returns a list of columns required to
     * evaluate this node.
     *
     * When reimplementing this, you need to return any column that is required to
     * evaluate this node and in addition recursively collect all the columns required
     * to evaluate child nodes.
     *
     * \warning If the expression has been prepared via a call to QgsExpression::prepare(),
     * or a call to QgsExpressionNode::prepare() for a node has been made, then some nodes in
     * the expression may have been determined to evaluate to a static pre-calculatable value.
     * In this case the results will omit attribute indices which are used by these
     * pre-calculated nodes, regardless of their actual referenced columns.
     * If you are seeking to use these functions to introspect an expression you must
     * take care to do this with an unprepared expression node.
     *
     * \returns A list of columns required to evaluate this expression
     */
    virtual QSet<QString> referencedColumns() const = 0;

    /**
     * Returns a set of all variables which are used in this expression.
     *
     * \note In contrast to the referencedColumns() function this method
     * is not affected by any previous calls to QgsExpressionNode::prepare().
     */
    virtual QSet<QString> referencedVariables() const = 0;

    /**
     * Returns a set of all functions which are used in this expression.
     *
     * \note In contrast to the referencedColumns() function this method
     * is not affected by any previous calls to QgsExpressionNode::prepare().
     */
    virtual QSet<QString> referencedFunctions() const = 0;

    /**
     * Returns a list of all nodes which are used in this expression.
     *
     * \note not available in Python bindings
     * \since QGIS 3.2
     */
    virtual QList<const QgsExpressionNode *> nodes( ) const = 0; SIP_SKIP

    /**
     * Abstract virtual method which returns if the geometry is required to evaluate
     * this expression.
     *
     * This needs to call `needsGeometry()` recursively on any child nodes.
     *
     * \returns TRUE if a geometry is required to evaluate this expression
     */
    virtual bool needsGeometry() const = 0;

    /**
     * Returns TRUE if this node can be evaluated for a static value. This is used during
     * the prepare() step and in case it returns TRUE, the value of this node will already
     * be evaluated and the result cached (and therefore not re-evaluated in subsequent calls
     * to eval()). In case this returns TRUE, prepareNode() will never be called.
     *
     * \since QGIS 3.0
     */
    virtual bool isStatic( QgsExpression *parent, const QgsExpressionContext *context ) const = 0;

    /**
     * Prepare this node for evaluation.
     * This will check if the node content is static and in this case cache the value.
     * If it's not static it will call prepareNode() to allow the node to do initialization
     * work like for example resolving a column name to an attribute index.
     *
     * \since QGIS 2.12
     */
    bool prepare( QgsExpression *parent, const QgsExpressionContext *context );

    /**
     * First line in the parser this node was found.
     * \note This might not be complete for all nodes. Currently
     * only \see QgsExpressionNode has this complete
     */
    int parserFirstLine = 0;

    /**
     * First column in the parser this node was found.
     * \note This might not be complete for all nodes. Currently
     * only \see QgsExpressionNode has this complete
     */
    int parserFirstColumn = 0;

    /**
     * Last line in the parser this node was found.
     * \note This might not be complete for all nodes. Currently
     * only \see QgsExpressionNode has this complete
     */
    int parserLastLine = 0;

    /**
     * Last column in the parser this node was found.
     * \note This might not be complete for all nodes. Currently
     * only \see QgsExpressionNode has this complete
     */
    int parserLastColumn = 0;

    /**
     * Returns TRUE if the node can be replaced by a static cached value.
     *
     * \see cachedStaticValue()
     * \since QGIS 3.18
     */
    bool hasCachedStaticValue() const { return mHasCachedValue; }

    /**
     * Returns the node's static cached value. Only valid if hasCachedStaticValue() is TRUE.
     *
     * \see hasCachedStaticValue()
     * \since QGIS 3.18
     */
    QVariant cachedStaticValue() const { return mCachedStaticValue; }

    /**
     * Sets the cached static \a value for the node.
     *
     * \note Not available from Python bindings.
     *
     * \since QGIS 3.30
     */
    void setCachedStaticValue( const QVariant &value ) const SIP_SKIP;

    /**
     * Returns a reference to the simplest node which represents this node,
     * after any compilation optimizations have been applied.
     *
     * Eg. a node like "CASE WHEN true THEN "some_field" WHEN other condition THEN ... END" can effectively
     * be replaced entirely by a QgsExpressionNodeColumnRef referencing the "some_field" field, as the
     * CASE WHEN ... will ALWAYS evaluate to "some_field".
     *
     * Returns a reference to the current object if no optimizations were applied.
     *
     * \since QGIS 3.20
     */
    const QgsExpressionNode *effectiveNode() const { return mCompiledSimplifiedNode ? mCompiledSimplifiedNode.get() : this; }

  protected:

    /**
     * Constructor.
     */
    QgsExpressionNode() = default;

    //! Copy constructor
    QgsExpressionNode( const QgsExpressionNode &other );
    //! Assignment operator
    QgsExpressionNode &operator=( const QgsExpressionNode &other );

    /**
     * Copies the members of this node to the node provided in \a target.
     * Needs to be called by all subclasses as part of their clone() implementation.
     *
     * \note Not available in python bindings, QgsExpression::Node is not
     * going to be subclassed from python. If that's what you are looking
     * for, look into writing a custom python expression function.
     * \since QGIS 3.0
     */
    void cloneTo( QgsExpressionNode *target ) const SIP_SKIP;

#ifndef SIP_RUN

    /**
     * TRUE if the node has a static, precalculated value.
     *
     * \since QGIS 3.20
     */
    mutable bool mHasCachedValue = false;

    /**
     * Contains the static, precalculated value for the node if mHasCachedValue is TRUE.
     *
     * \since QGIS 3.20
     */
    mutable QVariant mCachedStaticValue;

    /**
     * Contains a compiled node which represents a simplified version of this node
     * as a result of compilation optimizations.
     *
     * Eg. a node like "CASE WHEN true THEN "some_field" WHEN other condition THEN ... END" can effectively
     * be replaced entirely by a QgsExpressionNodeColumnRef referencing the "some_field" field, as the
     * CASE WHEN ... will ALWAYS evaluate to "some_field".
     *
     * \since QGIS 3.20
     */
    mutable std::unique_ptr< QgsExpressionNode > mCompiledSimplifiedNode;
#endif

  private:

    /**
     * Abstract virtual preparation method
     * Errors are reported to the parent
     * \since QGIS 3.0
     */
    virtual bool prepareNode( QgsExpression *parent, const QgsExpressionContext *context ) = 0;

    /**
     * Abstract virtual eval method
     * Errors are reported to the parent
     * \since QGIS 3.0
     */
    virtual QVariant evalNode( QgsExpression *parent, const QgsExpressionContext *context ) = 0;

};

Q_DECLARE_METATYPE( QgsExpressionNode * )

#endif // QGSEXPRESSIONNODE_H
