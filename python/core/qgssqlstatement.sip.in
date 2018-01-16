/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgssqlstatement.h                                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsSQLStatement
{
%Docstring
Class for parsing SQL statements.

.. versionadded:: 2.16
%End

%TypeHeaderCode
#include "qgssqlstatement.h"
%End
  public:

    QgsSQLStatement( const QString &statement );
%Docstring
Creates a new statement based on the provided string.
%End

    QgsSQLStatement( const QgsSQLStatement &other );
%Docstring
Create a copy of this statement.
%End

    ~QgsSQLStatement();

    bool hasParserError() const;
%Docstring
Returns true if an error occurred when parsing the input statement
%End
    QString parserErrorString() const;
%Docstring
Returns parser error
%End

    bool doBasicValidationChecks( QString &errorMsgOut /Out/ ) const;
%Docstring
Performs basic validity checks. Basically checking that columns referencing
a table, references a specified table. Returns true if the validation is
successful *
%End


    const QgsSQLStatement::Node *rootNode() const;
%Docstring
Returns root node of the statement. Root node is null is parsing has failed
%End

    QString statement() const;
%Docstring
Return the original, unmodified statement string.
If there was none supplied because it was constructed by sole
API calls, dump() will be used to create one instead.
%End

    QString dump() const;
%Docstring
Return  statement string, constructed from the internal
abstract syntax tree. This does not contain any nice whitespace
formatting or comments. In general it is preferable to use
statement() instead.
%End

    static QString quotedIdentifier( QString name );
%Docstring
Returns a quoted column reference (in double quotes)

.. seealso:: :py:func:`quotedString`
%End

    static QString quotedIdentifierIfNeeded( const QString &name );
%Docstring
Returns a quoted column reference (in double quotes) if needed, or
otherwise the original string.

.. seealso:: :py:func:`quotedString`
%End

    static QString stripQuotedIdentifier( QString text );
%Docstring
Remove double quotes from an identifier.

.. seealso:: :py:func:`quotedIdentifier`
%End

    static QString quotedString( QString text );
%Docstring
Returns a quoted version of a string (in single quotes)

.. seealso:: :py:func:`quotedIdentifier`
%End

    enum UnaryOperator
    {
      uoNot,
      uoMinus,
    };

    enum BinaryOperator
    {
      // logical
      boOr,
      boAnd,

      // comparison
      boEQ,
      boNE,
      boLE,
      boGE,
      boLT,
      boGT,
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

    class Node
{
%Docstring
Abstract node class *
%End

%TypeHeaderCode
#include "qgssqlstatement.h"
%End
%ConvertToSubClassCode
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
%End
      public:
        virtual ~Node();

        virtual QgsSQLStatement::NodeType nodeType() const = 0;
%Docstring
Abstract virtual that returns the type of this node.

:return: The type of this node
%End

        virtual QString dump() const = 0;
%Docstring
Abstract virtual dump method

:return: A statement which represents this node as string
%End

        virtual QgsSQLStatement::Node *clone() const = 0 /Factory/;
%Docstring
Generate a clone of this node.
Make sure that the clone does not contain any information which is
generated in prepare and context related.
Ownership is transferred to the caller.

:return: a deep copy of this node.
%End

        virtual void accept( QgsSQLStatement::Visitor &v ) const = 0;
%Docstring
Support the visitor pattern.

For any implementation this should look like

C++:

v.visit( *this );

Python:

v.visit( self)

:param v: A visitor that visits this node.
%End
    };

    class NodeList
{
%Docstring
List of nodes *
%End

%TypeHeaderCode
#include "qgssqlstatement.h"
%End
      public:
        NodeList();
%Docstring
Constructor
%End
        virtual ~NodeList();

        void append( QgsSQLStatement::Node *node /Transfer/ );
%Docstring
Takes ownership of the provided node
%End

        QList<QgsSQLStatement::Node *> list();
%Docstring
Return list
%End

        int count() const;
%Docstring
Returns the number of nodes in the list.
%End

        void accept( QgsSQLStatement::Visitor &v ) const;
%Docstring
Accept visitor
%End

        QgsSQLStatement::NodeList *clone() const /Factory/;
%Docstring
Creates a deep copy of this list. Ownership is transferred to the caller
%End

        virtual QString dump() const;
%Docstring
Dump list
%End

      protected:
    };

    class NodeUnaryOperator : QgsSQLStatement::Node
{
%Docstring
Unary logicial/arithmetical operator ( NOT, - ) *
%End

%TypeHeaderCode
#include "qgssqlstatement.h"
%End
      public:
        NodeUnaryOperator( QgsSQLStatement::UnaryOperator op, QgsSQLStatement::Node *operand /Transfer/ );
%Docstring
Constructor
%End
        ~NodeUnaryOperator();

        QgsSQLStatement::UnaryOperator op() const;
%Docstring
Operator
%End

        QgsSQLStatement::Node *operand() const;
%Docstring
Operand
%End

        virtual QgsSQLStatement::NodeType nodeType() const;
        virtual QString dump() const;


        virtual void accept( QgsSQLStatement::Visitor &v ) const;
        virtual QgsSQLStatement::Node *clone() const /Factory/;


      protected:
    };

    class NodeBinaryOperator : QgsSQLStatement::Node
{
%Docstring
Binary logical/arithmetical operator (AND, OR, =, +, ...) *
%End

%TypeHeaderCode
#include "qgssqlstatement.h"
%End
      public:
        NodeBinaryOperator( QgsSQLStatement::BinaryOperator op, QgsSQLStatement::Node *opLeft /Transfer/, QgsSQLStatement::Node *opRight /Transfer/ );
%Docstring
Constructor
%End
        ~NodeBinaryOperator();

        QgsSQLStatement::BinaryOperator op() const;
%Docstring
Operator
%End

        QgsSQLStatement::Node *opLeft() const;
%Docstring
Left operand
%End

        QgsSQLStatement::Node *opRight() const;
%Docstring
Right operand
%End

        virtual QgsSQLStatement::NodeType nodeType() const;
        virtual QString dump() const;


        virtual void accept( QgsSQLStatement::Visitor &v ) const;
        virtual QgsSQLStatement::Node *clone() const /Factory/;


        int precedence() const;
%Docstring
Precedence
%End

        bool leftAssociative() const;
%Docstring
Is left associative ?
%End

      protected:

    };

    class NodeInOperator : QgsSQLStatement::Node
{
%Docstring
'x IN (y, z)' operator *
%End

%TypeHeaderCode
#include "qgssqlstatement.h"
%End
      public:
        NodeInOperator( QgsSQLStatement::Node *node /Transfer/, QgsSQLStatement::NodeList *list /Transfer/, bool notin = false );
%Docstring
Constructor
%End
        ~NodeInOperator();

        QgsSQLStatement::Node *node() const;
%Docstring
Variable at the left of IN
%End

        bool isNotIn() const;
%Docstring
Whether this is a NOT IN operator
%End

        QgsSQLStatement::NodeList *list() const;
%Docstring
Values list
%End

        virtual QgsSQLStatement::NodeType nodeType() const;
        virtual QString dump() const;


        virtual void accept( QgsSQLStatement::Visitor &v ) const;
        virtual QgsSQLStatement::Node *clone() const /Factory/;


      protected:
    };

    class NodeBetweenOperator : QgsSQLStatement::Node
{
%Docstring
'X BETWEEN y and z' operator *
%End

%TypeHeaderCode
#include "qgssqlstatement.h"
%End
      public:
        NodeBetweenOperator( QgsSQLStatement::Node *node /Transfer/, QgsSQLStatement::Node *minVal /Transfer/, QgsSQLStatement::Node *maxVal /Transfer/, bool notBetween = false );
%Docstring
Constructor
%End

        QgsSQLStatement::Node *node() const;
%Docstring
Variable at the left of BETWEEN
%End

        bool isNotBetween() const;
%Docstring
Whether this is a NOT BETWEEN operator
%End

        QgsSQLStatement::Node *minVal() const;
%Docstring
Minimum bound
%End

        QgsSQLStatement::Node *maxVal() const;
%Docstring
Maximum bound
%End

        virtual QgsSQLStatement::NodeType nodeType() const;
        virtual QString dump() const;


        virtual void accept( QgsSQLStatement::Visitor &v ) const;
        virtual QgsSQLStatement::Node *clone() const /Factory/;


      protected:
    };

    class NodeFunction : QgsSQLStatement::Node
{
%Docstring
Function with a name and arguments node *
%End

%TypeHeaderCode
#include "qgssqlstatement.h"
%End
      public:
        NodeFunction( const QString &name, QgsSQLStatement::NodeList *args  /Transfer/ );
%Docstring
Constructor
%End
        ~NodeFunction();

        QString name() const;
%Docstring
Return function name
%End

        QgsSQLStatement::NodeList *args() const;
%Docstring
Return arguments
%End

        virtual QgsSQLStatement::NodeType nodeType() const;
        virtual QString dump() const;


        virtual void accept( QgsSQLStatement::Visitor &v ) const;
        virtual QgsSQLStatement::Node *clone() const /Factory/;


      protected:

    };

    class NodeLiteral : QgsSQLStatement::Node
{
%Docstring
Literal value (integer, integer64, double, string) *
%End

%TypeHeaderCode
#include "qgssqlstatement.h"
%End
      public:
        NodeLiteral( const QVariant &value );
%Docstring
Constructor
%End

        QVariant value() const;
%Docstring
The value of the literal.
%End

        virtual QgsSQLStatement::NodeType nodeType() const;
        virtual QString dump() const;


        virtual void accept( QgsSQLStatement::Visitor &v ) const;
        virtual QgsSQLStatement::Node *clone() const /Factory/;


      protected:
    };

    class NodeColumnRef : QgsSQLStatement::Node
{
%Docstring
Reference to a column *
%End

%TypeHeaderCode
#include "qgssqlstatement.h"
%End
      public:
        NodeColumnRef( const QString &name, bool star );
%Docstring
Constructor with column name only
%End
        NodeColumnRef( const QString &tableName, const QString &name, bool star );
%Docstring
Constructor with table and column name
%End

        void setDistinct( bool distinct = true );
%Docstring
Set whether this is prefixed by DISTINCT
%End

        QString tableName() const;
%Docstring
The name of the table. May be empty.
%End

        QString name() const;
%Docstring
The name of the column.
%End

        bool star() const;
%Docstring
Whether this is the * column
%End

        bool distinct() const;
%Docstring
Whether this is prefixed by DISTINCT
%End

        virtual QgsSQLStatement::NodeType nodeType() const;
        virtual QString dump() const;


        virtual void accept( QgsSQLStatement::Visitor &v ) const;
        virtual QgsSQLStatement::Node *clone() const /Factory/;

        QgsSQLStatement::NodeColumnRef *cloneThis() const /Factory/;
%Docstring
Clone with same type return
%End

      protected:
    };

    class NodeSelectedColumn : QgsSQLStatement::Node
{
%Docstring
Selected column *
%End

%TypeHeaderCode
#include "qgssqlstatement.h"
%End
      public:
        NodeSelectedColumn( QgsSQLStatement::Node *node /Transfer/ );
%Docstring
Constructor
%End
        ~NodeSelectedColumn();

        void setAlias( const QString &alias );
%Docstring
Set alias name
%End

        QgsSQLStatement::Node *column() const;
%Docstring
Column that is referred to
%End

        QString alias() const;
%Docstring
Alias name
%End

        virtual QgsSQLStatement::NodeType nodeType() const;
        virtual QString dump() const;


        virtual void accept( QgsSQLStatement::Visitor &v ) const;
        virtual QgsSQLStatement::Node *clone() const /Factory/;

        QgsSQLStatement::NodeSelectedColumn *cloneThis() const /Factory/;
%Docstring
Clone with same type return
%End

      protected:
    };

    class NodeCast : QgsSQLStatement::Node
{
%Docstring
CAST operator *
%End

%TypeHeaderCode
#include "qgssqlstatement.h"
%End
      public:
        NodeCast( QgsSQLStatement::Node *node /Transfer/, const QString &type );
%Docstring
Constructor
%End
        ~NodeCast();

        QgsSQLStatement::Node *node() const;
%Docstring
Node that is referred to
%End

        QString type() const;
%Docstring
Type
%End

        virtual QgsSQLStatement::NodeType nodeType() const;
        virtual QString dump() const;


        virtual void accept( QgsSQLStatement::Visitor &v ) const;
        virtual QgsSQLStatement::Node *clone() const /Factory/;


      protected:
    };

    class NodeTableDef : QgsSQLStatement::Node
{
%Docstring
Table definition *
%End

%TypeHeaderCode
#include "qgssqlstatement.h"
%End
      public:
        NodeTableDef( const QString &name );
%Docstring
Constructor with table name
%End
        NodeTableDef( const QString &name, const QString &alias );
%Docstring
Constructor with table name and alias
%End

        QString name() const;
%Docstring
Table name
%End

        QString alias() const;
%Docstring
Table alias
%End

        virtual QgsSQLStatement::NodeType nodeType() const;
        virtual QString dump() const;


        virtual void accept( QgsSQLStatement::Visitor &v ) const;
        virtual QgsSQLStatement::Node *clone() const /Factory/;

        QgsSQLStatement::NodeTableDef *cloneThis() const /Factory/;
%Docstring
Clone with same type return
%End

      protected:
    };

    class NodeJoin : QgsSQLStatement::Node
{
%Docstring
Join definition *
%End

%TypeHeaderCode
#include "qgssqlstatement.h"
%End
      public:
        NodeJoin( QgsSQLStatement::NodeTableDef *tabledef /Transfer/, QgsSQLStatement::Node *onExpr /Transfer/, QgsSQLStatement::JoinType type );
%Docstring
Constructor with table definition, ON expression
%End
        NodeJoin( QgsSQLStatement::NodeTableDef *tabledef /Transfer/, const QList<QString> &usingColumns, QgsSQLStatement::JoinType type );
%Docstring
Constructor with table definition and USING columns
%End
        ~NodeJoin();

        QgsSQLStatement::NodeTableDef *tableDef() const;
%Docstring
Table definition
%End

        QgsSQLStatement::Node *onExpr() const;
%Docstring
On expression. Will be None if usingColumns() is not empty
%End

        QList<QString> usingColumns() const;
%Docstring
Columns referenced by USING
%End

        QgsSQLStatement::JoinType type() const;
%Docstring
Join type
%End

        virtual QgsSQLStatement::NodeType nodeType() const;
        virtual QString dump() const;


        virtual void accept( QgsSQLStatement::Visitor &v ) const;
        virtual QgsSQLStatement::Node *clone() const /Factory/;

        QgsSQLStatement::NodeJoin *cloneThis() const /Factory/;
%Docstring
Clone with same type return
%End

      protected:
    };

    class NodeColumnSorted : QgsSQLStatement::Node
{
%Docstring
Column in a ORDER BY *
%End

%TypeHeaderCode
#include "qgssqlstatement.h"
%End
      public:
        NodeColumnSorted( QgsSQLStatement::NodeColumnRef *column /Transfer/, bool asc );
%Docstring
Constructor
%End
        ~NodeColumnSorted();

        QgsSQLStatement::NodeColumnRef *column() const;
%Docstring
The name of the column.
%End

        bool ascending() const;
%Docstring
Whether the column is sorted in ascending order
%End

        virtual QgsSQLStatement::NodeType nodeType() const;
        virtual QString dump() const;


        virtual void accept( QgsSQLStatement::Visitor &v ) const;
        virtual QgsSQLStatement::Node *clone() const /Factory/;

        QgsSQLStatement::NodeColumnSorted *cloneThis() const /Factory/;
%Docstring
Clone with same type return
%End

      protected:
    };

    class NodeSelect : QgsSQLStatement::Node
{
%Docstring
SELECT node *
%End

%TypeHeaderCode
#include "qgssqlstatement.h"
%End
      public:
        NodeSelect( const QList<QgsSQLStatement::NodeTableDef *> &tableList /Transfer/, const QList<QgsSQLStatement::NodeSelectedColumn *> &columns /Transfer/, bool distinct );
%Docstring
Constructor
%End
        ~NodeSelect();

        void setJoins( const QList<QgsSQLStatement::NodeJoin *> &joins /Transfer/ );
%Docstring
Set joins
%End
        void appendJoin( QgsSQLStatement::NodeJoin *join /Transfer/ );
%Docstring
Append a join
%End
        void setWhere( QgsSQLStatement::Node *where /Transfer/ );
%Docstring
Set where clause
%End
        void setOrderBy( const QList<QgsSQLStatement::NodeColumnSorted *> &orderBy /Transfer/ );
%Docstring
Set order by columns
%End

        QList<QgsSQLStatement::NodeTableDef *> tables() const;
%Docstring
Return the list of tables
%End
        QList<QgsSQLStatement::NodeSelectedColumn *> columns() const;
%Docstring
Return the list of columns
%End
        bool distinct() const;
%Docstring
Return if the SELECT is DISTINCT
%End
        QList<QgsSQLStatement::NodeJoin *> joins() const;
%Docstring
Return the list of joins
%End
        QgsSQLStatement::Node *where() const;
%Docstring
Return the where clause
%End
        QList<QgsSQLStatement::NodeColumnSorted *> orderBy() const;
%Docstring
Return the list of order by columns
%End

        virtual QgsSQLStatement::NodeType nodeType() const;
        virtual QString dump() const;


        virtual void accept( QgsSQLStatement::Visitor &v ) const;
        virtual QgsSQLStatement::Node *clone() const /Factory/;


      protected:
    };


    class Visitor
{
%Docstring
Support for visitor pattern - algorithms dealing with the statement
may be implemented without modifying the Node classes *
%End

%TypeHeaderCode
#include "qgssqlstatement.h"
%End
      public:
        virtual ~Visitor();
        virtual void visit( const QgsSQLStatement::NodeUnaryOperator &n ) = 0;
%Docstring
Visit NodeUnaryOperator
%End
        virtual void visit( const QgsSQLStatement::NodeBinaryOperator &n ) = 0;
%Docstring
Visit NodeBinaryOperator
%End
        virtual void visit( const QgsSQLStatement::NodeInOperator &n ) = 0;
%Docstring
Visit NodeInOperator
%End
        virtual void visit( const QgsSQLStatement::NodeBetweenOperator &n ) = 0;
%Docstring
Visit NodeBetweenOperator
%End
        virtual void visit( const QgsSQLStatement::NodeFunction &n ) = 0;
%Docstring
Visit NodeFunction
%End
        virtual void visit( const QgsSQLStatement::NodeLiteral &n ) = 0;
%Docstring
Visit NodeLiteral
%End
        virtual void visit( const QgsSQLStatement::NodeColumnRef &n ) = 0;
%Docstring
Visit NodeColumnRef
%End
        virtual void visit( const QgsSQLStatement::NodeSelectedColumn &n ) = 0;
%Docstring
Visit NodeSelectedColumn
%End
        virtual void visit( const QgsSQLStatement::NodeTableDef &n ) = 0;
%Docstring
Visit NodeTableDef
%End
        virtual void visit( const QgsSQLStatement::NodeSelect &n ) = 0;
%Docstring
Visit NodeSelect
%End
        virtual void visit( const QgsSQLStatement::NodeJoin &n ) = 0;
%Docstring
Visit NodeJoin
%End
        virtual void visit( const QgsSQLStatement::NodeColumnSorted &n ) = 0;
%Docstring
Visit NodeColumnSorted
%End
        virtual void visit( const QgsSQLStatement::NodeCast &n ) = 0;
%Docstring
Visit NodeCast
%End
    };

    class RecursiveVisitor: QgsSQLStatement::Visitor
{
%Docstring
A visitor that recursively explores all children *
%End

%TypeHeaderCode
#include "qgssqlstatement.h"
%End
      public:
        RecursiveVisitor();
%Docstring
Constructor
%End

        virtual void visit( const QgsSQLStatement::NodeUnaryOperator &n );
        virtual void visit( const QgsSQLStatement::NodeBinaryOperator &n );
        virtual void visit( const QgsSQLStatement::NodeInOperator &n );
        virtual void visit( const QgsSQLStatement::NodeBetweenOperator &n );
        virtual void visit( const QgsSQLStatement::NodeFunction &n );
        virtual void visit( const QgsSQLStatement::NodeLiteral & );
        virtual void visit( const QgsSQLStatement::NodeColumnRef & );
        virtual void visit( const QgsSQLStatement::NodeSelectedColumn &n );
        virtual void visit( const QgsSQLStatement::NodeTableDef & );
        virtual void visit( const QgsSQLStatement::NodeSelect &n );

        virtual void visit( const QgsSQLStatement::NodeJoin &n );

        virtual void visit( const QgsSQLStatement::NodeColumnSorted &n );
        virtual void visit( const QgsSQLStatement::NodeCast &n );
    };

    void acceptVisitor( QgsSQLStatement::Visitor &v ) const;
%Docstring
Entry function for the visitor pattern
%End

  protected:
};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgssqlstatement.h                                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
