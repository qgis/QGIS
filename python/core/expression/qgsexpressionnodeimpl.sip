/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/expression/qgsexpressionnodeimpl.h                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsExpressionNodeUnaryOperator : QgsExpressionNode
{
%Docstring
A unary node is either negative as in boolean (not) or as in numbers (minus).
%End

%TypeHeaderCode
#include "qgsexpressionnodeimpl.h"
%End
  public:

    enum UnaryOperator
    {
      uoNot,
      uoMinus,
    };

    QgsExpressionNodeUnaryOperator( QgsExpressionNodeUnaryOperator::UnaryOperator op, QgsExpressionNode *operand /Transfer/ );
%Docstring
A node unary operator is modifying the value of ``operand`` by negating it with ``op``.
%End
    ~QgsExpressionNodeUnaryOperator();

    QgsExpressionNodeUnaryOperator::UnaryOperator op() const;
    QgsExpressionNode *operand() const;

    virtual QgsExpressionNode::NodeType nodeType() const;

    virtual bool prepareNode( QgsExpression *parent, const QgsExpressionContext *context );

    virtual QVariant evalNode( QgsExpression *parent, const QgsExpressionContext *context );

    virtual QString dump() const;


    virtual QSet<QString> referencedColumns() const;

    virtual QSet<QString> referencedVariables() const;

    virtual bool needsGeometry() const;

    virtual QgsExpressionNode *clone() const /Factory/;


    virtual bool isStatic( QgsExpression *parent, const QgsExpressionContext *context ) const;


    QString text() const;
%Docstring
Returns a the name of this operator without the operands.
I.e. "NOT" or "-"
%End

};

class QgsExpressionNodeBinaryOperator : QgsExpressionNode
{

%TypeHeaderCode
#include "qgsexpressionnodeimpl.h"
%End
  public:

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

    QgsExpressionNodeBinaryOperator( QgsExpressionNodeBinaryOperator::BinaryOperator op, QgsExpressionNode *opLeft /Transfer/, QgsExpressionNode *opRight /Transfer/ );
%Docstring
Binary combination of the left and the right with op.
%End
    ~QgsExpressionNodeBinaryOperator();

    QgsExpressionNodeBinaryOperator::BinaryOperator op() const;
    QgsExpressionNode *opLeft() const;
    QgsExpressionNode *opRight() const;

    virtual QgsExpressionNode::NodeType nodeType() const;

    virtual bool prepareNode( QgsExpression *parent, const QgsExpressionContext *context );

    virtual QVariant evalNode( QgsExpression *parent, const QgsExpressionContext *context );

    virtual QString dump() const;


    virtual QSet<QString> referencedColumns() const;

    virtual QSet<QString> referencedVariables() const;

    virtual bool needsGeometry() const;

    virtual QgsExpressionNode *clone() const /Factory/;

    virtual bool isStatic( QgsExpression *parent, const QgsExpressionContext *context ) const;


    int precedence() const;
    bool leftAssociative() const;

    QString text() const;
%Docstring
Returns a the name of this operator without the operands.
I.e. "AND", "OR", ...
%End

};

class QgsExpressionNodeInOperator : QgsExpressionNode
{

%TypeHeaderCode
#include "qgsexpressionnodeimpl.h"
%End
  public:

    QgsExpressionNodeInOperator( QgsExpressionNode *node /Transfer/, QgsExpressionNode::NodeList *list /Transfer/, bool notin = false );
%Docstring
This node tests if the result of ``node`` is in the result of ``list``. Optionally it can be inverted with ``notin`` which by default is false.
%End
    ~QgsExpressionNodeInOperator();

    QgsExpressionNode *node() const;
    bool isNotIn() const;
    QgsExpressionNode::NodeList *list() const;

    virtual QgsExpressionNode::NodeType nodeType() const;

    virtual bool prepareNode( QgsExpression *parent, const QgsExpressionContext *context );

    virtual QVariant evalNode( QgsExpression *parent, const QgsExpressionContext *context );

    virtual QString dump() const;


    virtual QSet<QString> referencedColumns() const;

    virtual QSet<QString> referencedVariables() const;

    virtual bool needsGeometry() const;

    virtual QgsExpressionNode *clone() const /Factory/;

    virtual bool isStatic( QgsExpression *parent, const QgsExpressionContext *context ) const;


};

class QgsExpressionNodeFunction : QgsExpressionNode
{

%TypeHeaderCode
#include "qgsexpressionnodeimpl.h"
%End
  public:

    QgsExpressionNodeFunction( int fnIndex, QgsExpressionNode::NodeList *args /Transfer/ );
%Docstring
A function node consists of an index of the function in the global function array and
a list of arguments that will be passed to it.
%End

    ~QgsExpressionNodeFunction();

    int fnIndex() const;
    QgsExpressionNode::NodeList *args() const;

    virtual QgsExpressionNode::NodeType nodeType() const;

    virtual bool prepareNode( QgsExpression *parent, const QgsExpressionContext *context );

    virtual QVariant evalNode( QgsExpression *parent, const QgsExpressionContext *context );

    virtual QString dump() const;


    virtual QSet<QString> referencedColumns() const;

    virtual QSet<QString> referencedVariables() const;

    virtual bool needsGeometry() const;

    virtual QgsExpressionNode *clone() const /Factory/;

    virtual bool isStatic( QgsExpression *parent, const QgsExpressionContext *context ) const;


    static bool validateParams( int fnIndex, QgsExpressionNode::NodeList *args, QString &error );
%Docstring
Tests whether the provided argument list is valid for the matching function
%End

};

class QgsExpressionNodeLiteral : QgsExpressionNode
{

%TypeHeaderCode
#include "qgsexpressionnodeimpl.h"
%End
  public:
    QgsExpressionNodeLiteral( const QVariant &value );

    QVariant value() const;
%Docstring
The value of the literal.
%End

    virtual QgsExpressionNode::NodeType nodeType() const;

    virtual bool prepareNode( QgsExpression *parent, const QgsExpressionContext *context );

    virtual QVariant evalNode( QgsExpression *parent, const QgsExpressionContext *context );

    virtual QString dump() const;


    virtual QSet<QString> referencedColumns() const;

    virtual QSet<QString> referencedVariables() const;

    virtual bool needsGeometry() const;

    virtual QgsExpressionNode *clone() const /Factory/;

    virtual bool isStatic( QgsExpression *parent, const QgsExpressionContext *context ) const;


};

class QgsExpressionNodeColumnRef : QgsExpressionNode
{

%TypeHeaderCode
#include "qgsexpressionnodeimpl.h"
%End
  public:
    QgsExpressionNodeColumnRef( const QString &name );

    QString name() const;
%Docstring
The name of the column.
%End

    virtual QgsExpressionNode::NodeType nodeType() const;

    virtual bool prepareNode( QgsExpression *parent, const QgsExpressionContext *context );

    virtual QVariant evalNode( QgsExpression *parent, const QgsExpressionContext *context );

    virtual QString dump() const;


    virtual QSet<QString> referencedColumns() const;

    virtual QSet<QString> referencedVariables() const;

    virtual bool needsGeometry() const;


    virtual QgsExpressionNode *clone() const /Factory/;

    virtual bool isStatic( QgsExpression *parent, const QgsExpressionContext *context ) const;


};

class QgsExpressionNodeCondition : QgsExpressionNode
{

%TypeHeaderCode
#include "qgsexpressionnodeimpl.h"
%End
  public:
    class WhenThen
{

%TypeHeaderCode
#include "qgsexpressionnodeimpl.h"
%End
      public:

        WhenThen( QgsExpressionNode *whenExp, QgsExpressionNode *thenExp );
%Docstring
A combination of when and then. Simple as that.
%End
        ~WhenThen();


        QgsExpressionNodeCondition::WhenThen *clone() const /Factory/;
%Docstring
Get a deep copy of this WhenThen combination.
%End

      private:
        WhenThen( const QgsExpressionNodeCondition::WhenThen &rh );
    };
    typedef QList<QgsExpressionNodeCondition::WhenThen *> WhenThenList;

    QgsExpressionNodeCondition( QgsExpressionNodeCondition::WhenThenList *conditions, QgsExpressionNode *elseExp = 0 );
%Docstring
Create a new node with the given list of ``conditions`` and an optional ``elseExp`` expression.
%End


    ~QgsExpressionNodeCondition();

    virtual QgsExpressionNode::NodeType nodeType() const;

    virtual QVariant evalNode( QgsExpression *parent, const QgsExpressionContext *context );

    virtual bool prepareNode( QgsExpression *parent, const QgsExpressionContext *context );

    virtual QString dump() const;


    virtual QSet<QString> referencedColumns() const;

    virtual QSet<QString> referencedVariables() const;

    virtual bool needsGeometry() const;

    virtual QgsExpressionNode *clone() const /Factory/;

    virtual bool isStatic( QgsExpression *parent, const QgsExpressionContext *context ) const;


};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/expression/qgsexpressionnodeimpl.h                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
