/***************************************************************************
                               qgssqlstatement.h
                             ---------------------
    begin                : April 2016
    copyright            : (C) 2011 by Martin Dobias
    copyright            : (C) 2016 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSQLSTATEMENT_H
#define QGSSQLSTATEMENT_H

#include <QCoreApplication>
#include <QMetaType>
#include <QStringList>
#include <QVariant>
#include <QList>
#include <QSet>

/** \ingroup core
Class for parsing SQL statements.
* @note Added in QGIS 2.16
*/

class CORE_EXPORT QgsSQLStatement
{
    Q_DECLARE_TR_FUNCTIONS( QgsSQLStatement )
  public:
    /**
     * Creates a new statement based on the provided string.
     */
    QgsSQLStatement( const QString& statement );

    /**
     * Create a copy of this statement.
     */
    QgsSQLStatement( const QgsSQLStatement& other );
    /**
     * Create a copy of this statement.
     */
    QgsSQLStatement& operator=( const QgsSQLStatement& other );
    ~QgsSQLStatement();

    //! Returns true if an error occurred when parsing the input statement
    bool hasParserError() const;
    //! Returns parser error
    QString parserErrorString() const;

    /** Performs basic validity checks. Basically checking that columns referencing
     * a table, references a specified table. Returns true if the validation is
     * successful */
    bool doBasicValidationChecks( QString& errorMsgOut ) const;

    class Node;

    //! Returns root node of the statement. Root node is null is parsing has failed
    const Node* rootNode() const;

    //! Return the original, unmodified statement string.
    //! If there was none supplied because it was constructed by sole
    //! API calls, dump() will be used to create one instead.
    QString statement() const;

    //! Return  statement string, constructed from the internal
    //! abstract syntax tree. This does not contain any nice whitespace
    //! formatting or comments. In general it is preferrable to use
    //! statement() instead.
    QString dump() const;

    /** Returns a quoted column reference (in double quotes)
     * @see quotedString(), quotedIdentifierIfNeeded()
     */
    static QString quotedIdentifier( QString name );

    /** Returns a quoted column reference (in double quotes) if needed, or
     * otherwise the original string.
     * @see quotedString(), quotedIdentifier()
     */
    static QString quotedIdentifierIfNeeded( QString name );

    /** Remove double quotes from an identifier.
     * @see quotedIdentifier()
     */
    static QString stripQuotedIdentifier( QString text );

    /** Returns a quoted version of a string (in single quotes)
     * @see quotedIdentifier(), quotedIdentifierIfNeeded()
     */
    static QString quotedString( QString text );

    /**
     * @brief list of unary operators
     * @note if any change is made here, the definition of QgsSQLStatement::UnaryOperatorText[] must be adapted.
     */
    enum UnaryOperator
    {
      uoNot,
      uoMinus,
    };

    /**
     * @brief list of binary operators
     * @note if any change is made here, the definition of QgsSQLStatement::BinaryOperatorText[] must be adapted.
     */
    enum BinaryOperator
    {
      // logical
      boOr,
      boAnd,

      // comparison
      boEQ,  // =
      boNE,  // <>
      boLE,  // <=
      boGE,  // >=
      boLT,  // <
      boGT,  // >
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
     * @brief list of join types
     * @note if any change is made here, the definition of QgsSQLStatement::JoinTypeText[] must be adapted.
     */
    enum JoinType
    {
      jtDefault,
      jtLeft,
      jtLeftOuter,
      jtRight,
      jtRightOuter,
      jtCross,
      jtInner,
      jtFull
    };

    //! @note not available in Python bindings
    static const char* BinaryOperatorText[];

    //! @note not available in Python bindings
    static const char* UnaryOperatorText[];

    //! @note not available in Python bindings
    static const char* JoinTypeText[];

    //////

    class Visitor; // visitor interface is defined below

    /** Node type */
    enum NodeType
    {
      ntUnaryOperator,
      ntBinaryOperator,
      ntInOperator,
      ntBetweenOperator,
      ntFunction,
      ntLiteral,
      ntColumnRef,
      ntSelectedColumn,
      ntSelect,
      ntTableDef,
      ntJoin,
      ntColumnSorted,
      ntCast
    };

    /** \ingroup core
     * Abstract node class */
    class CORE_EXPORT Node
    {
      public:
        virtual ~Node() {}

        /**
         * Abstract virtual that returns the type of this node.
         *
         * @return The type of this node
         */
        virtual NodeType nodeType() const = 0;

        /**
         * Abstract virtual dump method
         *
         * @return A statement which represents this node as string
         */
        virtual QString dump() const = 0;

        /**
         * Generate a clone of this node.
         * Make sure that the clone does not contain any information which is
         * generated in prepare and context related.
         * Ownership is transferred to the caller.
         *
         * @return a deep copy of this node.
         */
        virtual Node* clone() const = 0;

        /**
         * Support the visitor pattern.
         *
         * For any implementation this should look like
         *
         * C++:
         *
         *     v.visit( *this );
         *
         * Python:
         *
         *     v.visit( self)
         *
         * @param v A visitor that visits this node.
         */
        virtual void accept( Visitor& v ) const = 0;
    };

    /** \ingroup core
     * List of nodes */
    class CORE_EXPORT NodeList
    {
      public:
        /** Constructor */
        NodeList()  {}
        virtual ~NodeList() { qDeleteAll( mList ); }

        /** Takes ownership of the provided node */
        void append( Node* node ) { mList.append( node ); }

        /** Return list */
        QList<Node*> list() { return mList; }

        /** Returns the number of nodes in the list.
         */
        int count() const { return mList.count(); }

        /** Accept visitor */
        void accept( Visitor& v ) const { Q_FOREACH ( Node* node, mList ) { node->accept( v ); } }

        /** Creates a deep copy of this list. Ownership is transferred to the caller */
        NodeList* clone() const;

        /** Dump list */
        virtual QString dump() const;

      protected:
        QList<Node*> mList;
    };

    /** \ingroup core
     * Unary logicial/arithmetical operator ( NOT, - ) */
    class CORE_EXPORT NodeUnaryOperator : public Node
    {
      public:
        /** Constructor */
        NodeUnaryOperator( UnaryOperator op, Node* operand ) : mOp( op ), mOperand( operand ) {}
        ~NodeUnaryOperator() { delete mOperand; }

        /** Operator */
        UnaryOperator op() const { return mOp; }

        /** Operand */
        Node* operand() const { return mOperand; }

        virtual NodeType nodeType() const override { return ntUnaryOperator; }
        virtual QString dump() const override;

        virtual void accept( Visitor& v ) const override { v.visit( *this ); }
        virtual Node* clone() const override;

      protected:
        UnaryOperator mOp;
        Node* mOperand;
    };

    /** \ingroup core
     * Binary logical/arithmetical operator (AND, OR, =, +, ...) */
    class CORE_EXPORT NodeBinaryOperator : public Node
    {
      public:
        /** Constructor */
        NodeBinaryOperator( BinaryOperator op, Node* opLeft, Node* opRight ) : mOp( op ), mOpLeft( opLeft ), mOpRight( opRight ) {}
        ~NodeBinaryOperator() { delete mOpLeft; delete mOpRight; }

        /** Operator */
        BinaryOperator op() const { return mOp; }

        /** Left operand */
        Node* opLeft() const { return mOpLeft; }

        /** Right operand */
        Node* opRight() const { return mOpRight; }

        virtual NodeType nodeType() const override { return ntBinaryOperator; }
        virtual QString dump() const override;

        virtual void accept( Visitor& v ) const override { v.visit( *this ); }
        virtual Node* clone() const override;

        /** Precedence */
        int precedence() const;

        /** Is left associative ? */
        bool leftAssociative() const;

      protected:

        BinaryOperator mOp;
        Node* mOpLeft;
        Node* mOpRight;
    };

    /** \ingroup core
     * 'x IN (y, z)' operator */
    class CORE_EXPORT NodeInOperator : public Node
    {
      public:
        /** Constructor */
        NodeInOperator( Node* node, NodeList* list, bool notin = false ) : mNode( node ), mList( list ), mNotIn( notin ) {}
        virtual ~NodeInOperator() { delete mNode; delete mList; }

        /** Variable at the left of IN */
        Node* node() const { return mNode; }

        /** Whether this is a NOT IN operator */
        bool isNotIn() const { return mNotIn; }

        /** Values list */
        NodeList* list() const { return mList; }

        virtual NodeType nodeType() const override { return ntInOperator; }
        virtual QString dump() const override;

        virtual void accept( Visitor& v ) const override { v.visit( *this ); }
        virtual Node* clone() const override;

      protected:
        Node* mNode;
        NodeList* mList;
        bool mNotIn;
    };

    /** \ingroup core
     * 'X BETWEEN y and z' operator */
    class CORE_EXPORT NodeBetweenOperator : public Node
    {
      public:
        /** Constructor */
        NodeBetweenOperator( Node* node, Node* minVal, Node* maxVal, bool notBetween = false ) : mNode( node ), mMinVal( minVal ), mMaxVal( maxVal ), mNotBetween( notBetween ) {}
        virtual ~NodeBetweenOperator() { delete mNode; delete mMinVal; delete mMaxVal; }

        /** Variable at the left of BETWEEN */
        Node* node() const { return mNode; }

        /** Whether this is a NOT BETWEEN operator */
        bool isNotBetween() const { return mNotBetween; }

        /** Minimum bound */
        Node* minVal() const { return mMinVal; }

        /** Maximum bound */
        Node* maxVal() const { return mMaxVal; }

        virtual NodeType nodeType() const override { return ntBetweenOperator; }
        virtual QString dump() const override;

        virtual void accept( Visitor& v ) const override { v.visit( *this ); }
        virtual Node* clone() const override;

      protected:
        Node* mNode;
        Node* mMinVal;
        Node* mMaxVal;
        bool mNotBetween;
    };

    /** \ingroup core
     * Function with a name and arguments node */
    class CORE_EXPORT NodeFunction : public Node
    {
      public:
        /** Constructor */
        NodeFunction( QString name, NodeList* args ) : mName( name ), mArgs( args ) {}
        virtual ~NodeFunction() { delete mArgs; }

        /** Return function name */
        QString name() const { return mName; }

        /** Return arguments */
        NodeList* args() const { return mArgs; }

        virtual NodeType nodeType() const override { return ntFunction; }
        virtual QString dump() const override;

        virtual void accept( Visitor& v ) const override { v.visit( *this ); }
        virtual Node* clone() const override;

      protected:
        QString mName;
        NodeList* mArgs;

    };

    /** \ingroup core
     * Literal value (integer, integer64, double, string) */
    class CORE_EXPORT NodeLiteral : public Node
    {
      public:
        /** Constructor */
        NodeLiteral( const QVariant& value ) : mValue( value ) {}

        /** The value of the literal. */
        inline QVariant value() const { return mValue; }

        virtual NodeType nodeType() const override { return ntLiteral; }
        virtual QString dump() const override;

        virtual void accept( Visitor& v ) const override { v.visit( *this ); }
        virtual Node* clone() const override;

      protected:
        QVariant mValue;
    };

    /** \ingroup core
     * Reference to a column */
    class CORE_EXPORT NodeColumnRef : public Node
    {
      public:
        /** Constructor with colum name only */
        NodeColumnRef( const QString& name, bool star ) : mName( name ), mDistinct( false ), mStar( star ) {}
        /** Constructor with table and column name */
        NodeColumnRef( const QString& tableName, const QString& name, bool star ) : mTableName( tableName ), mName( name ), mDistinct( false ), mStar( star ) {}

        /** Set whether this is prefixed by DISTINCT */
        void setDistinct( bool distinct = true ) { mDistinct = distinct; }

        /** The name of the table. May be empty. */
        QString tableName() const { return mTableName; }

        /** The name of the column. */
        QString name() const { return mName; }

        /** Whether this is the * column */
        bool star() const { return mStar; }

        /** Whether this is prefixed by DISTINCT */
        bool distinct() const { return mDistinct; }

        virtual NodeType nodeType() const override { return ntColumnRef; }
        virtual QString dump() const override;

        virtual void accept( Visitor& v ) const override { v.visit( *this ); }
        virtual Node* clone() const override;
        /** Clone with same type return */
        NodeColumnRef* cloneThis() const;

      protected:
        QString mTableName;
        QString mName;
        bool mDistinct;
        bool mStar;
    };

    /** \ingroup core
     * Selected column */
    class CORE_EXPORT NodeSelectedColumn : public Node
    {
      public:
        /** Constructor */
        NodeSelectedColumn( Node* node ) : mColumnNode( node ) {}
        virtual ~NodeSelectedColumn() { delete mColumnNode; }

        /** Set alias name */
        void setAlias( const QString& alias ) { mAlias = alias; }

        /** Column that is refered to */
        Node* column() const { return mColumnNode; }

        /** Alias name */
        QString alias() const { return mAlias; }

        virtual NodeType nodeType() const override { return ntSelectedColumn; }
        virtual QString dump() const override;

        virtual void accept( Visitor& v ) const override { v.visit( *this ); }
        virtual Node* clone() const override;
        /** Clone with same type return */
        NodeSelectedColumn* cloneThis() const;

      protected:
        Node *mColumnNode;
        QString mAlias;
    };

    /** \ingroup core
     * CAST operator */
    class CORE_EXPORT NodeCast : public Node
    {
      public:
        /** Constructor */
        NodeCast( Node* node, const QString& type ) : mNode( node ), mType( type ) {}
        virtual ~NodeCast() { delete mNode; }

        /** Node that is refered to */
        Node* node() const { return mNode; }

        /** Type */
        QString type() const { return mType; }

        virtual NodeType nodeType() const override { return ntCast; }
        virtual QString dump() const override;

        virtual void accept( Visitor& v ) const override { v.visit( *this ); }
        virtual Node* clone() const override;

      protected:
        Node *mNode;
        QString mType;
    };

    /** \ingroup core
     * Table definition */
    class CORE_EXPORT NodeTableDef : public Node
    {
      public:
        /** Constructor with table name */
        NodeTableDef( const QString& name ) : mName( name ) {}
        /** Constructor with table name and alias */
        NodeTableDef( const QString& name, const QString& alias ) : mName( name ), mAlias( alias ) {}

        /** Table name */
        QString name() const { return mName; }

        /** Table alias */
        QString alias() const { return mAlias; }

        virtual NodeType nodeType() const override { return ntTableDef; }
        virtual QString dump() const override;

        virtual void accept( Visitor& v ) const override { v.visit( *this ); }
        virtual Node* clone() const override;
        /** Clone with same type return */
        NodeTableDef* cloneThis() const;

      protected:
        QString mName;
        QString mAlias;
    };

    /** \ingroup core
     * Join definition */
    class CORE_EXPORT NodeJoin : public Node
    {
      public:
        /** Constructor with table definition, ON expression */
        NodeJoin( NodeTableDef* tabledef, Node* onExpr, JoinType type ) : mTableDef( tabledef ), mOnExpr( onExpr ), mType( type ) {}
        /** Constructor with table definition and USING columns */
        NodeJoin( NodeTableDef* tabledef, QList<QString> usingColumns, JoinType type ) : mTableDef( tabledef ), mOnExpr( nullptr ), mUsingColumns( usingColumns ), mType( type ) {}
        virtual ~NodeJoin() { delete mTableDef; delete mOnExpr; }

        /** Table definition */
        NodeTableDef* tableDef() const { return mTableDef; }

        /** On expression. Will be nullptr if usingColumns() is not empty */
        Node* onExpr() const { return mOnExpr; }

        /** Columns referenced by USING */
        QList<QString> usingColumns() const { return mUsingColumns; }

        /** Join type */
        JoinType type() const { return mType; }

        virtual NodeType nodeType() const override { return ntJoin; }
        virtual QString dump() const override;

        virtual void accept( Visitor& v ) const override { v.visit( *this ); }
        virtual Node* clone() const override;
        /** Clone with same type return */
        NodeJoin* cloneThis() const;

      protected:
        NodeTableDef* mTableDef;
        Node* mOnExpr;
        QList<QString> mUsingColumns;
        JoinType mType;
    };

    /** \ingroup core
     * Column in a ORDER BY */
    class CORE_EXPORT NodeColumnSorted : public Node
    {
      public:
        /** Constructor */
        NodeColumnSorted( NodeColumnRef* column, bool asc ) : mColumn( column ), mAsc( asc ) {}
        ~NodeColumnSorted() { delete mColumn; }

        /** The name of the column. */
        NodeColumnRef* column() const { return mColumn; }

        /** Whether the column is sorted in ascending order */
        bool ascending() const { return mAsc; }

        virtual NodeType nodeType() const override { return ntColumnSorted; }
        virtual QString dump() const override;

        virtual void accept( Visitor& v ) const override { v.visit( *this ); }
        virtual Node* clone() const override;
        /** Clone with same type return */
        NodeColumnSorted* cloneThis() const;

      protected:
        NodeColumnRef* mColumn;
        bool mAsc;
    };

    /** \ingroup core
     * SELECT node */
    class CORE_EXPORT NodeSelect : public Node
    {
      public:
        /** Constructor */
        NodeSelect( QList<NodeTableDef*> tableList, QList<NodeSelectedColumn*> columns, bool distinct ) : mTableList( tableList ), mColumns( columns ), mDistinct( distinct ), mWhere( nullptr ) {}
        virtual ~NodeSelect() { qDeleteAll( mTableList ); qDeleteAll( mColumns ); qDeleteAll( mJoins ); delete mWhere; qDeleteAll( mOrderBy ); }

        /** Set joins */
        void setJoins( QList<NodeJoin*> joins ) { qDeleteAll( mJoins ); mJoins = joins; }
        /** Append a join */
        void appendJoin( NodeJoin* join ) { mJoins.append( join ); }
        /** Set where clause */
        void setWhere( Node* where ) { delete mWhere; mWhere = where; }
        /** Set order by columns */
        void setOrderBy( QList<NodeColumnSorted*> orderBy ) { qDeleteAll( mOrderBy ); mOrderBy = orderBy; }

        /** Return the list of tables */
        QList<NodeTableDef*> tables() const { return mTableList; }
        /** Return the list of columns */
        QList<NodeSelectedColumn*> columns() const { return mColumns; }
        /** Return if the SELECT is DISTINCT */
        bool distinct() const { return mDistinct; }
        /** Return the list of joins */
        QList<NodeJoin*> joins() const { return mJoins; }
        /** Return the where clause */
        Node* where() const { return mWhere; }
        /** Return the list of order by columns */
        QList<NodeColumnSorted*> orderBy() const { return mOrderBy; }

        virtual NodeType nodeType() const override { return ntSelect; }
        virtual QString dump() const override;

        virtual void accept( Visitor& v ) const override { v.visit( *this ); }
        virtual Node* clone() const override;

      protected:
        QList<NodeTableDef*> mTableList;
        QList<NodeSelectedColumn*> mColumns;
        bool mDistinct;
        QList<NodeJoin*> mJoins;
        Node* mWhere;
        QList<NodeColumnSorted*> mOrderBy;
    };

    //////

    /** \ingroup core
     * Support for visitor pattern - algorithms dealing with the statement
        may be implemented without modifying the Node classes */
    class CORE_EXPORT Visitor
    {
      public:
        virtual ~Visitor() {}
        /** Visit NodeUnaryOperator */
        virtual void visit( const NodeUnaryOperator& n ) = 0;
        /** Visit NodeBinaryOperator */
        virtual void visit( const NodeBinaryOperator& n ) = 0;
        /** Visit NodeInOperator */
        virtual void visit( const NodeInOperator& n ) = 0;
        /** Visit NodeBetweenOperator */
        virtual void visit( const NodeBetweenOperator& n ) = 0;
        /** Visit NodeFunction */
        virtual void visit( const NodeFunction& n ) = 0;
        /** Visit NodeLiteral */
        virtual void visit( const NodeLiteral& n ) = 0;
        /** Visit NodeColumnRef */
        virtual void visit( const NodeColumnRef& n ) = 0;
        /** Visit NodeSelectedColumn */
        virtual void visit( const NodeSelectedColumn& n ) = 0;
        /** Visit NodeTableDef */
        virtual void visit( const NodeTableDef& n ) = 0;
        /** Visit NodeSelect */
        virtual void visit( const NodeSelect& n ) = 0;
        /** Visit NodeJoin */
        virtual void visit( const NodeJoin& n ) = 0;
        /** Visit NodeColumnSorted */
        virtual void visit( const NodeColumnSorted& n ) = 0;
        /** Visit NodeCast */
        virtual void visit( const NodeCast& n ) = 0;
    };

    /** \ingroup core
     * A visitor that recursively explores all children */
    class CORE_EXPORT RecursiveVisitor: public QgsSQLStatement::Visitor
    {
      public:
        /** Constructor */
        RecursiveVisitor() {}

        void visit( const QgsSQLStatement::NodeUnaryOperator& n ) override { n.operand()->accept( *this ); }
        void visit( const QgsSQLStatement::NodeBinaryOperator& n ) override { n.opLeft()->accept( *this ); n.opRight()->accept( *this ); }
        void visit( const QgsSQLStatement::NodeInOperator& n ) override { n.node()->accept( *this ); n.list()->accept( *this ); }
        void visit( const QgsSQLStatement::NodeBetweenOperator& n ) override { n.node()->accept( *this ); n.minVal()->accept( *this ); n.maxVal()->accept( *this ); }
        void visit( const QgsSQLStatement::NodeFunction& n ) override { n.args()->accept( *this ); }
        void visit( const QgsSQLStatement::NodeLiteral& ) override {}
        void visit( const QgsSQLStatement::NodeColumnRef& ) override { }
        void visit( const QgsSQLStatement::NodeSelectedColumn& n ) override { n.column()->accept( *this ); }
        void visit( const QgsSQLStatement::NodeTableDef& ) override {}
        void visit( const QgsSQLStatement::NodeSelect& n ) override;
        void visit( const QgsSQLStatement::NodeJoin& n ) override;
        void visit( const QgsSQLStatement::NodeColumnSorted& n ) override { n.column()->accept( *this ); }
        void visit( const QgsSQLStatement::NodeCast& n ) override { n.node()->accept( *this ); }
    };

    /** Entry function for the visitor pattern */
    void acceptVisitor( Visitor& v ) const;

  protected:
    QgsSQLStatement::Node* mRootNode;
    QString mStatement;
    QString mParserErrorString;
};

Q_DECLARE_METATYPE( QgsSQLStatement::Node* )

#endif // QGSSQLSTATEMENT_H
