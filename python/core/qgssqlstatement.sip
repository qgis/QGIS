class QgsSQLStatement
{
%TypeHeaderCode
#include "qgssqlstatement.h"
%End

  public:
    /**
     * Creates a new SQL statement based on the provided string.
     */
    QgsSQLStatement( const QString& statement );
    ~QgsSQLStatement();

    //! Returns true if an error occurred when parsing the input statement
    bool hasParserError() const;
    //! Returns parser error
    QString parserErrorString() const;

    /** Performs basic validity checks. Basically checking that columns referencing
     * a table, references a specified table. Returns true if the validation is
     * successful */
    bool doBasicValidationChecks( QString& errorMsgOut /Out/ ) const;

    //! Returns root node of the statement. Root node is null is parsing has failed
    const QgsSQLStatement::Node* rootNode() const;

    //! Return the original, unmodified statement string.
    //! If there was none supplied because it was constructed by sole
    //! API calls, dump() will be used to create one instead.
    QString statement() const;

    //! Return a statement string, constructed from the internal
    //! abstract syntax tree. This does not contain any nice whitespace
    //! formatting or comments. In general it is preferrable to use
    //! statement() instead.
    QString dump() const;

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
    // static const char* BinaryOperatorText[];

    //! @note not available in Python bindings
    // static const char* UnaryOperatorText[];

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
    static QString stripQuotedIdentifier(QString text);

    /** Returns a quoted version of a string (in single quotes)
     * @see quotedIdentifier(), quotedIdentifierIfNeeded()
     */
    static QString quotedString( QString text );


    //////

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

    /** Abstract node class */
    class Node
    {
      %ConvertToSubClassCode
        switch (sipCpp->nodeType())
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
      %End

      public:
        virtual ~Node();

        /**
         * Abstract virtual that returns the type of this node.
         *
         * @return The type of this node
         */
        virtual QgsSQLStatement::NodeType nodeType() const = 0;

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
        virtual QgsSQLStatement::Node* clone() const = 0 /Factory/;

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
        virtual void accept( QgsSQLStatement::Visitor& v ) const = 0;
    };

    /** List of nodes */
    class NodeList
    {
      public:
        NodeList();
        ~NodeList();
        /** Takes ownership of the provided node */
        void append( QgsSQLStatement::Node* node /Transfer/ );

        /** Returns the number of nodes in the list.
         */
        int count() const;

        /** Accept visitor */
        void accept( QgsSQLStatement::Visitor& v ) const;

        const QList<QgsSQLStatement::Node*>& list();

        /** Creates a deep copy of this list. Ownership is transferred to the caller */
        QgsSQLStatement::NodeList* clone() const /Factory/;

        virtual QString dump() const;
    };

    /** Unary logicial/arithmetical operator ( NOT, - ) */
    class NodeUnaryOperator : QgsSQLStatement::Node
    {
      public:
        NodeUnaryOperator( QgsSQLStatement::UnaryOperator op, QgsSQLStatement::Node* operand /Transfer/ );
        ~NodeUnaryOperator();

        /** Operator */
        QgsSQLStatement::UnaryOperator op() const;

        /** Operand */
        QgsSQLStatement::Node* operand() const;

        virtual QgsSQLStatement::NodeType nodeType() const;
        virtual QString dump() const;

        virtual void accept( QgsSQLStatement::Visitor& v ) const;
        virtual QgsSQLStatement::Node* clone() const /Factory/;
    };

    /** Binary logical/arithmetical operator (AND, OR, =, +, ...) */
    class NodeBinaryOperator : QgsSQLStatement::Node
    {
      public:
        NodeBinaryOperator( QgsSQLStatement::BinaryOperator op, QgsSQLStatement::Node* opLeft /Transfer/, QgsSQLStatement::Node* opRight /Transfer/ );
        ~NodeBinaryOperator();

        /** Operator */
        QgsSQLStatement::BinaryOperator op() const;

        /** Left operand */
        QgsSQLStatement::Node* opLeft() const;

        /** Right operand */
        QgsSQLStatement::Node* opRight() const;

        virtual QgsSQLStatement::NodeType nodeType() const;
        virtual QString dump() const;

        virtual void accept( QgsSQLStatement::Visitor& v ) const;
        virtual QgsSQLStatement::Node* clone() const /Factory/;

        /** Precedence */
        int precedence() const;

        /** Is left associative ? */
        bool leftAssociative() const;
    };

    /** 'x IN (y, z)' operator */
    class NodeInOperator : QgsSQLStatement::Node
    {
      public:
        NodeInOperator( QgsSQLStatement::Node* node /Transfer/, QgsSQLStatement::NodeList* list /Transfer/, bool notin = false );
        ~NodeInOperator();

        /** Variable at the left of IN */
        QgsSQLStatement::Node* node() const;

        /** Whether this is a NOT IN operator */
        bool isNotIn() const;

        /** Values list */
        QgsSQLStatement::NodeList* list() const;

        virtual QgsSQLStatement::NodeType nodeType() const;
        virtual QString dump() const;

        virtual void accept( QgsSQLStatement::Visitor& v ) const;
        virtual QgsSQLStatement::Node* clone() const /Factory/;
    };

    /** 'X BETWEEN y and z' operator */
    class NodeBetweenOperator : QgsSQLStatement::Node
    {
      public:
        NodeBetweenOperator( QgsSQLStatement::Node* node /Transfer/, QgsSQLStatement::Node* minVal /Transfer/, QgsSQLStatement::Node* maxVal /Transfer/, bool notbetween = false );
        ~NodeBetweenOperator();

        /** Variable at the left of BETWEEN */
        QgsSQLStatement::Node* node() const;

        /** Whether this is a NOT BETWEEN operator */
        bool isNotBetween() const;

        /** Minimum bound */
        QgsSQLStatement::Node* minVal() const;

        /** Maximum bound */
        QgsSQLStatement::Node* maxVal() const;

        virtual QgsSQLStatement::NodeType nodeType() const;
        virtual QString dump() const;

        virtual void accept( QgsSQLStatement::Visitor& v ) const;
        virtual QgsSQLStatement::Node* clone() const /Factory/;
    };

    /** Function with a name and arguments node */
    class NodeFunction : QgsSQLStatement::Node
    {
      public:
        NodeFunction( QString name, QgsSQLStatement::NodeList* args /Transfer/ );
        ~NodeFunction();

        /** Return function name */
        QString name() const;

        /** Return arguments */
        QgsSQLStatement::NodeList* args() const;

        virtual QgsSQLStatement::NodeType nodeType() const;
        virtual QString dump() const;

        virtual void accept( QgsSQLStatement::Visitor& v ) const;
        virtual QgsSQLStatement::Node* clone() const /Factory/;
    };

    /** Literal value (integer, integer64, double, string) */
    class NodeLiteral : QgsSQLStatement::Node
    {
      public:
        NodeLiteral( const QVariant& value );

        /** The value of the literal. */
        const QVariant& value() const;

        virtual QgsSQLStatement::NodeType nodeType() const;
        virtual QString dump() const;
        virtual QgsSQLStatement::Node* clone() const /Factory/;

        virtual void accept( QgsSQLStatement::Visitor& v ) const;
    };

    /** Reference to a column */
    class NodeColumnRef : QgsSQLStatement::Node
    {
      public:
        NodeColumnRef( const QString& tableName, const QString& name, bool star );
        NodeColumnRef( const QString& name, bool star );

        /** Set whether this is prefixed by DISTINCT */
        void setDistinct( bool distinct = true );

        /** The name of the table. May be empty. */
        QString tableName() const;

        /** The name of the column. */
        QString name() const;

        /** Whether this is prefixed by DISTINCT */
        bool distinct() const;

        /** Whether this is the * column */
        bool star() const;

        virtual QgsSQLStatement::NodeType nodeType() const;
        virtual QString dump() const;

        virtual void accept( QgsSQLStatement::Visitor& v ) const;
        virtual QgsSQLStatement::Node* clone() const /Factory/;
        /** Clone with same type return */
        QgsSQLStatement::NodeColumnRef* cloneThis() const /Factory/;
    };

    /** Selected column */
    class NodeSelectedColumn : QgsSQLStatement::Node
    {
      public:
        NodeSelectedColumn( QgsSQLStatement::Node* node /Transfer/  );

        /** Set alias name */
        void setAlias( const QString& alias );

        /** Column that is refered to */
        QgsSQLStatement::Node* column() const;

        /** Alias name */
        QString alias() const;

        virtual QgsSQLStatement::NodeType nodeType() const;
        virtual QString dump() const;

        virtual void accept( QgsSQLStatement::Visitor& v ) const;
        virtual QgsSQLStatement::Node* clone() const /Factory/;

        /** Clone with same type return */
        QgsSQLStatement::NodeSelectedColumn* cloneThis() const /Factory/;
    };

    /** CAST operator */
    class NodeCast : QgsSQLStatement::Node
    {
      public:
        NodeCast( QgsSQLStatement::Node* node /Transfer/, const QString& type );

        /** Node that is refered to */
        QgsSQLStatement::Node* node() const;

        /** Type */
        QString type() const;

        virtual QgsSQLStatement::NodeType nodeType() const;
        virtual QString dump() const;

        virtual void accept( QgsSQLStatement::Visitor& v ) const;
        virtual QgsSQLStatement::Node* clone() const /Factory/;
    };

    /** Table definition */
    class NodeTableDef : QgsSQLStatement::Node
    {
      public:
        NodeTableDef( const QString& name, const QString& alias );
        NodeTableDef( const QString& name );

        /** Table name */
        QString name() const;

        /** Table alias */
        QString alias() const;

        virtual QgsSQLStatement::NodeType nodeType() const;
        virtual QString dump() const;

        virtual void accept( QgsSQLStatement::Visitor& v ) const;
        virtual QgsSQLStatement::Node* clone() const /Factory/;
        /** Clone with same type return */
        QgsSQLStatement::NodeTableDef* cloneThis() const /Factory/;
    };

    /** Join definition */
    class NodeJoin : QgsSQLStatement::Node
    {
      public:
        NodeJoin( QgsSQLStatement::NodeTableDef* tabledef /Transfer/, QgsSQLStatement::Node* onExpr /Transfer/, QgsSQLStatement::JoinType type );
        NodeJoin( QgsSQLStatement::NodeTableDef* tabledef /Transfer/, QList<QString> usingColumns, QgsSQLStatement::JoinType type );

        /** Table definition */
        QgsSQLStatement::NodeTableDef* tableDef() const;

        /** On expression. Will be nullptr if usingColumns() is not empty */
        QgsSQLStatement::Node* onExpr() const;

        /** Columns referenced by USING */
        QList<QString> usingColumns() const;

        /** Join type */
        QgsSQLStatement::JoinType type() const;

        virtual QgsSQLStatement::NodeType nodeType() const;
        virtual QString dump() const;

        virtual void accept( QgsSQLStatement::Visitor& v ) const;
        virtual QgsSQLStatement::Node* clone() const /Factory/;
        /** Clone with same type return */
        QgsSQLStatement::NodeJoin* cloneThis() const /Factory/;
    };

    /** Column in a ORDER BY */
    class NodeColumnSorted : QgsSQLStatement::Node
    {
      public:
        NodeColumnSorted( QgsSQLStatement::NodeColumnRef* column /Transfer/, bool asc );

        /** The name of the column. */
        QgsSQLStatement::NodeColumnRef* column() const;

        /** Whether the column is sorted in ascending order */
        bool ascending() const;

        virtual QgsSQLStatement::NodeType nodeType() const;
        virtual QString dump() const;

        virtual void accept( QgsSQLStatement::Visitor& v ) const;
        virtual QgsSQLStatement::Node* clone() const /Factory/;
        /** Clone with same type return */
        QgsSQLStatement::NodeColumnSorted* cloneThis() const /Factory/;
    };

    /** SELECT node */
    class NodeSelect : QgsSQLStatement::Node
    {
      public:
        NodeSelect( QList<QgsSQLStatement::NodeTableDef*> tableList /Transfer/, QList<QgsSQLStatement::NodeSelectedColumn*> columns /Transfer/, bool distinct );

        /** Set joins */
        void setJoins( QList<QgsSQLStatement::NodeJoin*> joins /Transfer/ );
        /** Append a join */
        void appendJoin( QgsSQLStatement::NodeJoin* join /Transfer/ );
        /** Set where clause */
        void setWhere( QgsSQLStatement::Node* where /Transfer/ );
        /** Set order by columns */
        void setOrderBy( QList<QgsSQLStatement::NodeColumnSorted*> orderBy /Transfer/ );

        /** Return the list of tables */
        QList<QgsSQLStatement::NodeTableDef*> tables() const;
        /** Return the list of columns */
        QList<QgsSQLStatement::NodeSelectedColumn*> columns() const;
        /** Return if the SELECT is DISTINCT */
        bool distinct() const;
        /** Return the list of joins */
        QList<QgsSQLStatement::NodeJoin*> joins() const;
        /** Return the where clause */
        QgsSQLStatement::Node* where() const;
        /** Return the list of order by columns */
        QList<QgsSQLStatement::NodeColumnSorted*> orderBy() const;

        virtual QgsSQLStatement::NodeType nodeType() const;
        virtual QString dump() const;

        virtual void accept( QgsSQLStatement::Visitor& v ) const;
        virtual QgsSQLStatement::Node* clone() const /Factory/;
    };

    //////

    /** Support for visitor pattern - algorithms dealing with the statement
        may be implemented without modifying the Node classes */
    class Visitor
    {
      public:
        virtual ~Visitor();
        virtual void visit( const QgsSQLStatement::NodeUnaryOperator& n ) = 0;
        virtual void visit( const QgsSQLStatement::NodeBinaryOperator& n ) = 0;
        virtual void visit( const QgsSQLStatement::NodeInOperator& n ) = 0;
        virtual void visit( const QgsSQLStatement::NodeBetweenOperator& n ) = 0;
        virtual void visit( const QgsSQLStatement::NodeFunction& n ) = 0;
        virtual void visit( const QgsSQLStatement::NodeLiteral& n ) = 0;
        virtual void visit( const QgsSQLStatement::NodeColumnRef& n ) = 0;
        virtual void visit( const QgsSQLStatement::NodeSelectedColumn& n ) = 0;
        virtual void visit( const QgsSQLStatement::NodeTableDef& n ) = 0;
        virtual void visit( const QgsSQLStatement::NodeJoin& n ) = 0;
        virtual void visit( const QgsSQLStatement::NodeColumnSorted& n ) = 0;
        virtual void visit( const QgsSQLStatement::NodeSelect& n ) = 0;
        virtual void visit( const QgsSQLStatement::NodeCast& n ) = 0;
    };

    /** A visitor that recursively explores all children */
    class RecursiveVisitor: public QgsSQLStatement::Visitor
    {
      public:
        RecursiveVisitor();

        virtual void visit( const QgsSQLStatement::NodeUnaryOperator& n );
        virtual void visit( const QgsSQLStatement::NodeBinaryOperator& n );
        virtual void visit( const QgsSQLStatement::NodeInOperator& n );
        virtual void visit( const QgsSQLStatement::NodeBetweenOperator& n) ;
        virtual void visit( const QgsSQLStatement::NodeFunction& n );
        virtual void visit( const QgsSQLStatement::NodeLiteral& n );
        virtual void visit( const QgsSQLStatement::NodeColumnRef& n );
        virtual void visit( const QgsSQLStatement::NodeSelectedColumn& n );
        virtual void visit( const QgsSQLStatement::NodeTableDef& n );
        virtual void visit( const QgsSQLStatement::NodeJoin& n );
        virtual void visit( const QgsSQLStatement::NodeColumnSorted& n );
        virtual void visit( const QgsSQLStatement::NodeSelect& n );
        virtual void visit( const QgsSQLStatement::NodeCast& n );
    };

    /** Entry function for the visitor pattern */
    void acceptVisitor( QgsSQLStatement::Visitor& v ) const;
};
