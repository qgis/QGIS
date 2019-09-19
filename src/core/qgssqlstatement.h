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
#include "qgis_sip.h"
#include <QMetaType>
#include <QStringList>
#include <QVariant>
#include <QList>
#include <QSet>

#include "qgis_core.h"

/**
 * \ingroup core
Class for parsing SQL statements.
* \since QGIS 2.16
*/

class CORE_EXPORT QgsSQLStatement
{
    Q_DECLARE_TR_FUNCTIONS( QgsSQLStatement )
  public:

    /**
     * Creates a new statement based on the provided string.
     */
    QgsSQLStatement( const QString &statement );

    /**
     * Create a copy of this statement.
     */
    QgsSQLStatement( const QgsSQLStatement &other );

    /**
     * Create a copy of this statement.
     */
    QgsSQLStatement &operator=( const QgsSQLStatement &other );
    ~QgsSQLStatement();

    //! Returns TRUE if an error occurred when parsing the input statement
    bool hasParserError() const;
    //! Returns parser error
    QString parserErrorString() const;

    /**
     * Performs basic validity checks. Basically checking that columns referencing
     * a table, references a specified table. Returns TRUE if the validation is
     * successful
    */
    bool doBasicValidationChecks( QString &errorMsgOut SIP_OUT ) const;

    class Node;

    /**
     * Returns the root node of the statement.
     * The root node is NULLPTR if parsing has failed.
     */
    const QgsSQLStatement::Node *rootNode() const;

    /**
     * Returns the original, unmodified statement string.
     * If there was none supplied because it was constructed by sole
     * API calls, dump() will be used to create one instead.
     */
    QString statement() const;

    /**
     * Returns the statement string, constructed from the internal
     * abstract syntax tree. This does not contain any nice whitespace
     * formatting or comments. In general it is preferable to use
     * statement() instead.
     */
    QString dump() const;

    /**
     * Returns a quoted column reference (in double quotes)
     * \see quotedString(), quotedIdentifierIfNeeded()
     */
    static QString quotedIdentifier( QString name );

    /**
     * Returns a quoted column reference (in double quotes) if needed, or
     * otherwise the original string.
     * \see quotedString(), quotedIdentifier()
     */
    static QString quotedIdentifierIfNeeded( const QString &name );

    /**
     * Remove double quotes from an identifier.
     * \see quotedIdentifier()
     */
    static QString stripQuotedIdentifier( QString text );

    /**
     * Returns a quoted version of a string (in single quotes)
     * \see quotedIdentifier(), quotedIdentifierIfNeeded()
     */
    static QString quotedString( QString text );

    /**
     * \brief list of unary operators
     * \note if any change is made here, the definition of QgsSQLStatement::UnaryOperatorText[] must be adapted.
     */
    enum UnaryOperator
    {
      uoNot,
      uoMinus,
    };

    /**
     * \brief list of binary operators
     * \note if any change is made here, the definition of QgsSQLStatement::BinaryOperatorText[] must be adapted.
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
     * \brief list of join types
     * \note if any change is made here, the definition of QgsSQLStatement::JoinTypeText[] must be adapted.
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

    //! \note not available in Python bindings
    static const char *BINARY_OPERATOR_TEXT[] SIP_SKIP;

    //! \note not available in Python bindings
    static const char *UNARY_OPERATOR_TEXT[] SIP_SKIP;

    //! \note not available in Python bindings
    static const char *JOIN_TYPE_TEXT[] SIP_SKIP;

    //////

    class Visitor; // visitor interface is defined below

    //! Node type
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

    /**
     * \ingroup core
     * Abstract node class
    */
    class CORE_EXPORT Node
    {

#ifdef SIP_RUN
        SIP_CONVERT_TO_SUBCLASS_CODE
        switch ( sipCpp->nodeType() )
        {
          case QgsSQLStatement::ntUnaryOperator:   sipType = sipType_QgsSQLStatement_NodeUnaryOperator; break;
          case QgsSQLStatement::ntBinaryOperator:  sipType = sipType_QgsSQLStatement_NodeBinaryOperator; break;
          case QgsSQLStatement::ntInOperator:      sipType = sipType_QgsSQLStatement_NodeInOperator; break;
          case QgsSQLStatement::ntBetweenOperator: sipType = sipType_QgsSQLStatement_NodeBetweenOperator; break;
          case QgsSQLStatement::ntFunction:        sipType = sipType_QgsSQLStatement_NodeFunction; break;
          case QgsSQLStatement::ntLiteral:         sipType = sipType_QgsSQLStatement_NodeLiteral; break;
          case QgsSQLStatement::ntColumnRef:       sipType = sipType_QgsSQLStatement_NodeColumnRef; break;
          case QgsSQLStatement::ntSelectedColumn:  sipType = sipType_QgsSQLStatement_NodeSelectedColumn; break;
          case QgsSQLStatement::ntSelect:          sipType = sipType_QgsSQLStatement_NodeSelect; break;
          case QgsSQLStatement::ntTableDef:        sipType = sipType_QgsSQLStatement_NodeTableDef; break;
          case QgsSQLStatement::ntJoin:            sipType = sipType_QgsSQLStatement_NodeJoin; break;
          case QgsSQLStatement::ntColumnSorted:    sipType = sipType_QgsSQLStatement_NodeColumnSorted; break;
          case QgsSQLStatement::ntCast:            sipType = sipType_QgsSQLStatement_NodeCast; break;
          default:                               sipType = 0; break;
        }
        SIP_END
#endif

      public:
        virtual ~Node() = default;

        /**
         * Abstract virtual that returns the type of this node.
         *
         * \returns The type of this node
         */
        virtual QgsSQLStatement::NodeType nodeType() const = 0;

        /**
         * Abstract virtual dump method
         *
         * \returns A statement which represents this node as string
         */
        virtual QString dump() const = 0;

        /**
         * Generate a clone of this node.
         * Make sure that the clone does not contain any information which is
         * generated in prepare and context related.
         * Ownership is transferred to the caller.
         *
         * \returns a deep copy of this node.
         */
        virtual QgsSQLStatement::Node *clone() const = 0 SIP_FACTORY;

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
         * \param v A visitor that visits this node.
         */
        virtual void accept( QgsSQLStatement::Visitor &v ) const = 0;
    };

    /**
     * A list of nodes.
     * \ingroup core
     */
    class CORE_EXPORT NodeList
    {
      public:
        //! Constructor
        NodeList() = default;
        virtual ~NodeList() { qDeleteAll( mList ); }

        //! Takes ownership of the provided node
        void append( QgsSQLStatement::Node *node SIP_TRANSFER ) { mList.append( node ); }

        //! Returns list
        QList<QgsSQLStatement::Node *> list() { return mList; }

        /**
         * Returns the number of nodes in the list.
         */
        int count() const { return mList.count(); }

        //! Accept visitor
        void accept( QgsSQLStatement::Visitor &v ) const;

        //! Creates a deep copy of this list. Ownership is transferred to the caller
        QgsSQLStatement::NodeList *clone() const SIP_FACTORY;

        //! Dump list
        virtual QString dump() const;

      protected:
        QList<Node *> mList;
    };

    /**
     * \ingroup core
     * Unary logicial/arithmetical operator ( NOT, - )
    */
    class CORE_EXPORT NodeUnaryOperator : public QgsSQLStatement::Node
    {
      public:
        //! Constructor
        NodeUnaryOperator( QgsSQLStatement::UnaryOperator op, QgsSQLStatement::Node *operand SIP_TRANSFER ) : mOp( op ), mOperand( operand ) {}
        ~NodeUnaryOperator() override { delete mOperand; }

        //! Operator
        QgsSQLStatement::UnaryOperator op() const { return mOp; }

        //! Operand
        QgsSQLStatement::Node *operand() const { return mOperand; }

        QgsSQLStatement::NodeType nodeType() const override { return ntUnaryOperator; }
        QString dump() const override;

        void accept( QgsSQLStatement::Visitor &v ) const override { v.visit( *this ); }
        QgsSQLStatement::Node *clone() const override SIP_FACTORY;

      protected:
        UnaryOperator mOp;
        Node *mOperand = nullptr;
    };

    /**
     * \ingroup core
     * Binary logical/arithmetical operator (AND, OR, =, +, ...)
    */
    class CORE_EXPORT NodeBinaryOperator : public QgsSQLStatement::Node
    {
      public:
        //! Constructor
        NodeBinaryOperator( QgsSQLStatement::BinaryOperator op, QgsSQLStatement::Node *opLeft SIP_TRANSFER, QgsSQLStatement::Node *opRight SIP_TRANSFER )
          : mOp( op )
          , mOpLeft( opLeft )
          , mOpRight( opRight )
        {}
        ~NodeBinaryOperator() override { delete mOpLeft; delete mOpRight; }

        //! Operator
        QgsSQLStatement::BinaryOperator op() const { return mOp; }

        //! Left operand
        QgsSQLStatement::Node *opLeft() const { return mOpLeft; }

        //! Right operand
        QgsSQLStatement::Node *opRight() const { return mOpRight; }

        QgsSQLStatement::NodeType nodeType() const override { return ntBinaryOperator; }
        QString dump() const override;

        void accept( QgsSQLStatement::Visitor &v ) const override { v.visit( *this ); }
        QgsSQLStatement::Node *clone() const override SIP_FACTORY;

        //! Precedence
        int precedence() const;

        //! Is left associative ?
        bool leftAssociative() const;

      protected:

        BinaryOperator mOp;
        Node *mOpLeft = nullptr;
        Node *mOpRight = nullptr;
    };

    /**
     * \ingroup core
     * 'x IN (y, z)' operator
    */
    class CORE_EXPORT NodeInOperator : public QgsSQLStatement::Node
    {
      public:
        //! Constructor
        NodeInOperator( QgsSQLStatement::Node *node SIP_TRANSFER, QgsSQLStatement::NodeList *list SIP_TRANSFER, bool notin = false ) : mNode( node ), mList( list ), mNotIn( notin ) {}
        ~NodeInOperator() override { delete mNode; delete mList; }

        //! Variable at the left of IN
        QgsSQLStatement::Node *node() const { return mNode; }

        //! Whether this is a NOT IN operator
        bool isNotIn() const { return mNotIn; }

        //! Values list
        QgsSQLStatement::NodeList *list() const { return mList; }

        QgsSQLStatement::NodeType nodeType() const override { return ntInOperator; }
        QString dump() const override;

        void accept( QgsSQLStatement::Visitor &v ) const override { v.visit( *this ); }
        QgsSQLStatement::Node *clone() const override SIP_FACTORY;

      protected:
        Node *mNode = nullptr;
        NodeList *mList = nullptr;
        bool mNotIn;
    };

    /**
     * \ingroup core
     * 'X BETWEEN y and z' operator
    */
    class CORE_EXPORT NodeBetweenOperator : public QgsSQLStatement::Node
    {
      public:
        //! Constructor
        NodeBetweenOperator( QgsSQLStatement::Node *node SIP_TRANSFER, QgsSQLStatement::Node *minVal SIP_TRANSFER, QgsSQLStatement::Node *maxVal SIP_TRANSFER, bool notBetween = false )
          : mNode( node ), mMinVal( minVal ), mMaxVal( maxVal ), mNotBetween( notBetween ) {}
        ~NodeBetweenOperator() override { delete mNode; delete mMinVal; delete mMaxVal; }

        //! Variable at the left of BETWEEN
        QgsSQLStatement::Node *node() const { return mNode; }

        //! Whether this is a NOT BETWEEN operator
        bool isNotBetween() const { return mNotBetween; }

        //! Minimum bound
        QgsSQLStatement::Node *minVal() const { return mMinVal; }

        //! Maximum bound
        QgsSQLStatement::Node *maxVal() const { return mMaxVal; }

        QgsSQLStatement::NodeType nodeType() const override { return ntBetweenOperator; }
        QString dump() const override;

        void accept( QgsSQLStatement::Visitor &v ) const override { v.visit( *this ); }
        QgsSQLStatement::Node *clone() const override SIP_FACTORY;

      protected:
        Node *mNode = nullptr;
        Node *mMinVal = nullptr;
        Node *mMaxVal = nullptr;
        bool mNotBetween;
    };

    /**
     * \ingroup core
     * Function with a name and arguments node
    */
    class CORE_EXPORT NodeFunction : public QgsSQLStatement::Node
    {
      public:
        //! Constructor
        NodeFunction( const QString &name, QgsSQLStatement::NodeList *args  SIP_TRANSFER ) : mName( name ), mArgs( args ) {}
        ~NodeFunction() override { delete mArgs; }

        //! Returns function name
        QString name() const { return mName; }

        //! Returns arguments
        QgsSQLStatement::NodeList *args() const { return mArgs; }

        QgsSQLStatement::NodeType nodeType() const override { return ntFunction; }
        QString dump() const override;

        void accept( QgsSQLStatement::Visitor &v ) const override { v.visit( *this ); }
        QgsSQLStatement::Node *clone() const override SIP_FACTORY;

      protected:
        QString mName;
        NodeList *mArgs = nullptr;

    };

    /**
     * \ingroup core
     * Literal value (integer, integer64, double, string)
    */
    class CORE_EXPORT NodeLiteral : public QgsSQLStatement::Node
    {
      public:
        //! Constructor
        NodeLiteral( const QVariant &value ) : mValue( value ) {}

        //! The value of the literal.
        inline QVariant value() const { return mValue; }

        QgsSQLStatement::NodeType nodeType() const override { return ntLiteral; }
        QString dump() const override;

        void accept( QgsSQLStatement::Visitor &v ) const override { v.visit( *this ); }
        QgsSQLStatement::Node *clone() const override SIP_FACTORY;

      protected:
        QVariant mValue;
    };

    /**
     * \ingroup core
     * Reference to a column
    */
    class CORE_EXPORT NodeColumnRef : public QgsSQLStatement::Node
    {
      public:
        //! Constructor with column name only
        NodeColumnRef( const QString &name, bool star ) : mName( name ), mDistinct( false ), mStar( star ) {}
        //! Constructor with table and column name
        NodeColumnRef( const QString &tableName, const QString &name, bool star ) : mTableName( tableName ), mName( name ), mDistinct( false ), mStar( star ) {}

        //! Sets whether this is prefixed by DISTINCT
        void setDistinct( bool distinct = true ) { mDistinct = distinct; }

        //! The name of the table. May be empty.
        QString tableName() const { return mTableName; }

        //! The name of the column.
        QString name() const { return mName; }

        //! Whether this is the * column
        bool star() const { return mStar; }

        //! Whether this is prefixed by DISTINCT
        bool distinct() const { return mDistinct; }

        QgsSQLStatement::NodeType nodeType() const override { return ntColumnRef; }
        QString dump() const override;

        void accept( QgsSQLStatement::Visitor &v ) const override { v.visit( *this ); }
        QgsSQLStatement::Node *clone() const override SIP_FACTORY;
        //! Clone with same type return
        QgsSQLStatement::NodeColumnRef *cloneThis() const SIP_FACTORY;

      protected:
        QString mTableName;
        QString mName;
        bool mDistinct;
        bool mStar;
    };

    /**
     * \ingroup core
     * Selected column
    */
    class CORE_EXPORT NodeSelectedColumn : public QgsSQLStatement::Node
    {
      public:
        //! Constructor
        NodeSelectedColumn( QgsSQLStatement::Node *node SIP_TRANSFER ) : mColumnNode( node ) {}
        ~NodeSelectedColumn() override { delete mColumnNode; }

        //! Sets alias name
        void setAlias( const QString &alias ) { mAlias = alias; }

        //! Column that is referred to
        QgsSQLStatement::Node *column() const { return mColumnNode; }

        //! Alias name
        QString alias() const { return mAlias; }

        QgsSQLStatement::NodeType nodeType() const override { return ntSelectedColumn; }
        QString dump() const override;

        void accept( QgsSQLStatement::Visitor &v ) const override { v.visit( *this ); }
        QgsSQLStatement::Node *clone() const override SIP_FACTORY;
        //! Clone with same type return
        QgsSQLStatement::NodeSelectedColumn *cloneThis() const SIP_FACTORY;

      protected:
        Node *mColumnNode = nullptr;
        QString mAlias;
    };

    /**
     * \ingroup core
     * CAST operator
    */
    class CORE_EXPORT NodeCast : public QgsSQLStatement::Node
    {
      public:
        //! Constructor
        NodeCast( QgsSQLStatement::Node *node SIP_TRANSFER, const QString &type ) : mNode( node ), mType( type ) {}
        ~NodeCast() override { delete mNode; }

        //! Node that is referred to
        QgsSQLStatement::Node *node() const { return mNode; }

        //! Type
        QString type() const { return mType; }

        QgsSQLStatement::NodeType nodeType() const override { return ntCast; }
        QString dump() const override;

        void accept( QgsSQLStatement::Visitor &v ) const override { v.visit( *this ); }
        QgsSQLStatement::Node *clone() const override SIP_FACTORY;

      protected:
        Node *mNode = nullptr;
        QString mType;
    };

    /**
     * \ingroup core
     * Table definition
    */
    class CORE_EXPORT NodeTableDef : public QgsSQLStatement::Node
    {
      public:
        //! Constructor with table name
        NodeTableDef( const QString &name ) : mName( name ) {}
        //! Constructor with table name and alias
        NodeTableDef( const QString &name, const QString &alias ) : mName( name ), mAlias( alias ) {}

        //! Table name
        QString name() const { return mName; }

        //! Table alias
        QString alias() const { return mAlias; }

        QgsSQLStatement::NodeType nodeType() const override { return ntTableDef; }
        QString dump() const override;

        void accept( QgsSQLStatement::Visitor &v ) const override { v.visit( *this ); }
        QgsSQLStatement::Node *clone() const override SIP_FACTORY;
        //! Clone with same type return
        QgsSQLStatement::NodeTableDef *cloneThis() const SIP_FACTORY;

      protected:
        QString mName;
        QString mAlias;
    };

    /**
     * \ingroup core
     * Join definition
    */
    class CORE_EXPORT NodeJoin : public QgsSQLStatement::Node
    {
      public:
        //! Constructor with table definition, ON expression
        NodeJoin( QgsSQLStatement::NodeTableDef *tabledef SIP_TRANSFER, QgsSQLStatement::Node *onExpr SIP_TRANSFER, QgsSQLStatement::JoinType type ) : mTableDef( tabledef ), mOnExpr( onExpr ), mType( type ) {}
        //! Constructor with table definition and USING columns
        NodeJoin( QgsSQLStatement::NodeTableDef *tabledef SIP_TRANSFER, const QList<QString> &usingColumns, QgsSQLStatement::JoinType type ) : mTableDef( tabledef ), mUsingColumns( usingColumns ), mType( type ) {}
        ~NodeJoin() override { delete mTableDef; delete mOnExpr; }

        //! Table definition
        QgsSQLStatement::NodeTableDef *tableDef() const { return mTableDef; }

        //! On expression. Will be NULLPTR if usingColumns() is not empty
        QgsSQLStatement::Node *onExpr() const { return mOnExpr; }

        //! Columns referenced by USING
        QList<QString> usingColumns() const { return mUsingColumns; }

        //! Join type
        QgsSQLStatement::JoinType type() const { return mType; }

        QgsSQLStatement::NodeType nodeType() const override { return ntJoin; }
        QString dump() const override;

        void accept( QgsSQLStatement::Visitor &v ) const override { v.visit( *this ); }
        QgsSQLStatement::Node *clone() const override SIP_FACTORY;
        //! Clone with same type return
        QgsSQLStatement::NodeJoin *cloneThis() const SIP_FACTORY;

      protected:
        NodeTableDef *mTableDef = nullptr;
        Node *mOnExpr = nullptr;
        QList<QString> mUsingColumns;
        JoinType mType;
    };

    /**
     * \ingroup core
     * Column in a ORDER BY
    */
    class CORE_EXPORT NodeColumnSorted : public QgsSQLStatement::Node
    {
      public:
        //! Constructor
        NodeColumnSorted( QgsSQLStatement::NodeColumnRef *column SIP_TRANSFER, bool asc ) : mColumn( column ), mAsc( asc ) {}
        ~NodeColumnSorted() override { delete mColumn; }

        //! The name of the column.
        QgsSQLStatement::NodeColumnRef *column() const { return mColumn; }

        //! Whether the column is sorted in ascending order
        bool ascending() const { return mAsc; }

        QgsSQLStatement::NodeType nodeType() const override { return ntColumnSorted; }
        QString dump() const override;

        void accept( QgsSQLStatement::Visitor &v ) const override { v.visit( *this ); }
        QgsSQLStatement::Node *clone() const override SIP_FACTORY;
        //! Clone with same type return
        QgsSQLStatement::NodeColumnSorted *cloneThis() const SIP_FACTORY;

      protected:
        NodeColumnRef *mColumn = nullptr;
        bool mAsc;
    };

    /**
     * \ingroup core
     * SELECT node
    */
    class CORE_EXPORT NodeSelect : public QgsSQLStatement::Node
    {
      public:
        //! Constructor
        NodeSelect( const QList<QgsSQLStatement::NodeTableDef *> &tableList SIP_TRANSFER, const QList<QgsSQLStatement::NodeSelectedColumn *> &columns SIP_TRANSFER, bool distinct ) : mTableList( tableList ), mColumns( columns ), mDistinct( distinct ) {}
        ~NodeSelect() override;

        //! Sets joins
        void setJoins( const QList<QgsSQLStatement::NodeJoin *> &joins SIP_TRANSFER ) { qDeleteAll( mJoins ); mJoins = joins; }
        //! Append a join
        void appendJoin( QgsSQLStatement::NodeJoin *join SIP_TRANSFER ) { mJoins.append( join ); }
        //! Sets where clause
        void setWhere( QgsSQLStatement::Node *where SIP_TRANSFER ) { delete mWhere; mWhere = where; }
        //! Sets order by columns
        void setOrderBy( const QList<QgsSQLStatement::NodeColumnSorted *> &orderBy SIP_TRANSFER ) { qDeleteAll( mOrderBy ); mOrderBy = orderBy; }

        //! Returns the list of tables
        QList<QgsSQLStatement::NodeTableDef *> tables() const { return mTableList; }
        //! Returns the list of columns
        QList<QgsSQLStatement::NodeSelectedColumn *> columns() const { return mColumns; }
        //! Returns if the SELECT is DISTINCT
        bool distinct() const { return mDistinct; }
        //! Returns the list of joins
        QList<QgsSQLStatement::NodeJoin *> joins() const { return mJoins; }
        //! Returns the where clause
        QgsSQLStatement::Node *where() const { return mWhere; }
        //! Returns the list of order by columns
        QList<QgsSQLStatement::NodeColumnSorted *> orderBy() const { return mOrderBy; }

        QgsSQLStatement::NodeType nodeType() const override { return ntSelect; }
        QString dump() const override;

        void accept( QgsSQLStatement::Visitor &v ) const override { v.visit( *this ); }
        QgsSQLStatement::Node *clone() const override SIP_FACTORY;

      protected:
        QList<NodeTableDef *> mTableList;
        QList<NodeSelectedColumn *> mColumns;
        bool mDistinct;
        QList<NodeJoin *> mJoins;
        Node *mWhere = nullptr;
        QList<NodeColumnSorted *> mOrderBy;
    };

    //////

    /**
     * \ingroup core
     * Support for visitor pattern - algorithms dealing with the statement
        may be implemented without modifying the Node classes
    */
    class CORE_EXPORT Visitor
    {
      public:
        virtual ~Visitor() = default;
        //! Visit NodeUnaryOperator
        virtual void visit( const QgsSQLStatement::NodeUnaryOperator &n ) = 0;
        //! Visit NodeBinaryOperator
        virtual void visit( const QgsSQLStatement::NodeBinaryOperator &n ) = 0;
        //! Visit NodeInOperator
        virtual void visit( const QgsSQLStatement::NodeInOperator &n ) = 0;
        //! Visit NodeBetweenOperator
        virtual void visit( const QgsSQLStatement::NodeBetweenOperator &n ) = 0;
        //! Visit NodeFunction
        virtual void visit( const QgsSQLStatement::NodeFunction &n ) = 0;
        //! Visit NodeLiteral
        virtual void visit( const QgsSQLStatement::NodeLiteral &n ) = 0;
        //! Visit NodeColumnRef
        virtual void visit( const QgsSQLStatement::NodeColumnRef &n ) = 0;
        //! Visit NodeSelectedColumn
        virtual void visit( const QgsSQLStatement::NodeSelectedColumn &n ) = 0;
        //! Visit NodeTableDef
        virtual void visit( const QgsSQLStatement::NodeTableDef &n ) = 0;
        //! Visit NodeSelect
        virtual void visit( const QgsSQLStatement::NodeSelect &n ) = 0;
        //! Visit NodeJoin
        virtual void visit( const QgsSQLStatement::NodeJoin &n ) = 0;
        //! Visit NodeColumnSorted
        virtual void visit( const QgsSQLStatement::NodeColumnSorted &n ) = 0;
        //! Visit NodeCast
        virtual void visit( const QgsSQLStatement::NodeCast &n ) = 0;
    };

    /**
     * \ingroup core
     * A visitor that recursively explores all children
    */
    class CORE_EXPORT RecursiveVisitor: public QgsSQLStatement::Visitor
    {
      public:
        //! Constructor
        RecursiveVisitor() = default;

        void visit( const QgsSQLStatement::NodeUnaryOperator &n ) override { n.operand()->accept( *this ); }
        void visit( const QgsSQLStatement::NodeBinaryOperator &n ) override { n.opLeft()->accept( *this ); n.opRight()->accept( *this ); }
        void visit( const QgsSQLStatement::NodeInOperator &n ) override { n.node()->accept( *this ); n.list()->accept( *this ); }
        void visit( const QgsSQLStatement::NodeBetweenOperator &n ) override { n.node()->accept( *this ); n.minVal()->accept( *this ); n.maxVal()->accept( *this ); }
        void visit( const QgsSQLStatement::NodeFunction &n ) override { n.args()->accept( *this ); }
        void visit( const QgsSQLStatement::NodeLiteral & ) override {}
        void visit( const QgsSQLStatement::NodeColumnRef & ) override { }
        void visit( const QgsSQLStatement::NodeSelectedColumn &n ) override { n.column()->accept( *this ); }
        void visit( const QgsSQLStatement::NodeTableDef & ) override {}
        void visit( const QgsSQLStatement::NodeSelect &n ) override;
        void visit( const QgsSQLStatement::NodeJoin &n ) override;
        void visit( const QgsSQLStatement::NodeColumnSorted &n ) override { n.column()->accept( *this ); }
        void visit( const QgsSQLStatement::NodeCast &n ) override { n.node()->accept( *this ); }
    };

    //! Entry function for the visitor pattern
    void acceptVisitor( QgsSQLStatement::Visitor &v ) const;

  protected:
    QgsSQLStatement::Node *mRootNode = nullptr;
    QString mStatement;
    QString mParserErrorString;
};

Q_DECLARE_METATYPE( QgsSQLStatement::Node * )

#endif // QGSSQLSTATEMENT_H
