/***************************************************************************
                        qgspointcloudexpressionnode.h
                        -----------------------------
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

#ifndef QGSPOINTCLOUDEXPRESSIONNODE_H
#define QGSPOINTCLOUDEXPRESSIONNODE_H

#include <QSet>
#include <QVariant>
#include <QCoreApplication>

#include "qgis.h"
#include "qgspointcloudattribute.h"
#include "qgspointcloudblock.h"

class QgsPointCloudExpression;
class QgsExpressionNode;

/**
 * \ingroup core
 *
 * \brief Abstract base class for all nodes that can appear in an expression.
 *
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsPointCloudExpressionNode
{

    Q_DECLARE_TR_FUNCTIONS( QgsPointCloudExpressionNode )

  public:

    //! Known node types.
    enum NodeType
    {
      ntUnaryOperator, //!< \see QgsPointCloudExpression::Node::NodeUnaryOperator
      ntBinaryOperator, //!< \see QgsPointCloudExpression::Node::NodeBinaryOperator
      ntInOperator, //!< \see QgsExpression::Node::NodeInOperator
      ntLiteral, //!< \see QgsPointCloudExpression::Node::NodeLiteral
      ntAttributeRef, //!< \see QgsPointCloudExpression::Node::NodeAttributeRef
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
        void append( QgsPointCloudExpressionNode *node ) { mList.append( node ); mNameList.append( QString() ); }

        /**
         * Returns the number of nodes in the list.
         */
        int count() const { return mList.count(); }

        /**
         * Gets a list of all the nodes.
         */
        QList<QgsPointCloudExpressionNode *> list() { return mList; }

        /**
         * Gets the node at position i in the list.
         */
        QgsPointCloudExpressionNode *at( int i ) { return mList.at( i ); }

        /**
         * Returns a list of names for nodes. Unnamed nodes will be indicated by an empty string in the list.
         */
        QStringList names() const { return mNameList; }

        //! Creates a deep copy of this list. Ownership is transferred to the caller
        QgsPointCloudExpressionNode::NodeList *clone() const;

        /**
         * Returns a string dump of the expression node.
         */
        virtual QString dump() const;

      private:
        QList<QgsPointCloudExpressionNode *> mList;
        QStringList mNameList;


      public:
    };

    virtual ~QgsPointCloudExpressionNode() = default;

    /**
     * Gets the type of this node.
     *
     * \returns The type of this node
     */
    virtual QgsPointCloudExpressionNode::NodeType nodeType() const = 0;

    /**
     * Dump this node into a serialized (part) of an expression.
     * The returned expression does not necessarily literally match
     * the original expression, it's just guaranteed to behave the same way.
     */
    virtual QString dump() const = 0;

    /**
     * Evaluate this node for the given point and parent.
     * This will return a cached value if it has been determined to be static
     * during the prepare() execution.
     */
    double eval( QgsPointCloudExpression *parent, int pointIndex );

    /**
     * Generate a clone of this node.
     * Ownership is transferred to the caller.
     *
     * \returns a deep copy of this node.
     */
    virtual QgsPointCloudExpressionNode *clone() const = 0;

    /**
     * Abstract virtual method which returns a list of columns required to
     * evaluate this node.
     *
     * When reimplementing this, you need to return any attribute that is required to
     * evaluate this node and in addition recursively collect all the attributes required
     * to evaluate child nodes.
     *
     * \returns A list of attributes required to evaluate this expression
     */
    virtual QSet<QString> referencedAttributes() const = 0;

    /**
     * Returns a list of all nodes which are used in this expression.
     */
    virtual QList<const QgsPointCloudExpressionNode *> nodes( ) const = 0;

    /**
     * Returns TRUE if this node can be evaluated for a static value. This is used during
     * the prepare() step and in case it returns TRUE, the value of this node will already
     * be evaluated and the result cached (and therefore not re-evaluated in subsequent calls
     * to eval()). In case this returns TRUE, prepareNode() will never be called.
     */
    virtual bool isStatic( QgsPointCloudExpression *parent, const QgsPointCloudBlock *block ) const = 0;

    /**
     * Prepare this node for evaluation.
     * This will check if the node content is static and in this case cache the value.
     * If it's not static it will call prepareNode() to allow the node to do initialization
     * work like for example resolving an attribute name to an attribute index.
     */
    bool prepare( QgsPointCloudExpression *parent, const QgsPointCloudBlock *block );

    /**
     * First line in the parser this node was found.
     * \note This might not be complete for all nodes. Currently
     * only \see QgsPointCloudExpressionNode has this complete
     */
    int parserFirstLine = 0;

    /**
     * First column in the parser this node was found.
     * \note This might not be complete for all nodes. Currently
     * only \see QgsPointCloudExpressionNode has this complete
     */
    int parserFirstColumn = 0;

    /**
     * Last line in the parser this node was found.
     * \note This might not be complete for all nodes. Currently
     * only \see QgsPointCloudExpressionNode has this complete
     */
    int parserLastLine = 0;

    /**
     * Last column in the parser this node was found.
     * \note This might not be complete for all nodes. Currently
     * only \see QgsPointCloudExpressionNode has this complete
     */
    int parserLastColumn = 0;

    /**
     * Returns TRUE if the node can be replaced by a static cached value.
     *
     * \see cachedStaticValue()
     */
    bool hasCachedStaticValue() const { return mHasCachedValue; }

    /**
     * Returns the node's static cached value. Only valid if hasCachedStaticValue() is TRUE.
     *
     * \see hasCachedStaticValue()
     */
    double cachedStaticValue() const { return mCachedStaticValue; }

    static QgsPointCloudExpressionNode *convert( const QgsExpressionNode *node, QString &error );

  protected:

    /**
     * Constructor.
     */
    QgsPointCloudExpressionNode() = default;

    //! Copy constructor
    QgsPointCloudExpressionNode( const QgsPointCloudExpressionNode &other );
    //! Assignment operator
    QgsPointCloudExpressionNode &operator=( const QgsPointCloudExpressionNode &other );

    /**
     * Copies the members of this node to the node provided in \a target.
     * Needs to be called by all subclasses as part of their clone() implementation.
     */
    void cloneTo( QgsPointCloudExpressionNode *target ) const;

    /**
     * TRUE if the node has a static, precalculated value.
     */
    mutable bool mHasCachedValue = false;

    /**
     * Contains the static, precalculated value for the node if mHasCachedValue is TRUE.
     */
    mutable double mCachedStaticValue;

  private:

    /**
     * Abstract virtual preparation method
     * Errors are reported to the parent
     */
    virtual bool prepareNode( QgsPointCloudExpression *parent, const QgsPointCloudBlock *block ) = 0;

    /**
     * Abstract virtual eval method
     * Errors are reported to the parent
     */
    virtual double evalNode( QgsPointCloudExpression *parent, int pointIndex ) = 0;

};

Q_DECLARE_METATYPE( QgsPointCloudExpressionNode * )

#endif // QGSPOINTCLOUDEXPRESSIONNODE_H
