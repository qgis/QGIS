/***************************************************************************
                               qgsexpression.h
                             -------------------
    begin                : August 2011
    copyright            : (C) 2011 Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSEXPRESSION_H
#define QGSEXPRESSION_H

#include <QStringList>
#include <QVariant>
#include <QList>
#include <QDomDocument>

#include "qgsfield.h"

class QgsDistanceArea;
class QgsFeature;
class QDomElement;

/**
Class for parsing and evaluation of expressions (formerly called "search strings").
The expressions try to follow both syntax and semantics of SQL expressions.

Usage:

  QgsExpression exp("gid*2 > 10 and type not in ('D','F'));
  if (exp.hasParserError())
  {
    // show error message with parserErrorString() and exit
  }
  QVariant result = exp.evaluate(feature, fields);
  if (exp.hasEvalError())
  {
    // show error message with evalErrorString()
  }
  else
  {
    // examine the result
  }

Possible QVariant value types:
- invalid (null)
- int
- double
- string

Similarly to SQL, this class supports three-value logic: true/false/unknown.
Unknown value may be a result of operations with missing data (NULL). Please note
that NULL is different value than zero or an empty string. For example
3 > NULL returns unknown.

There is no special (three-value) 'boolean' type: true/false is represented as
1/0 integer, unknown value is represented the same way as NULL values: invalid QVariant.

For better performance with many evaluations you may first call prepare(fields) function
to find out indices of columns and then repeatedly call evaluate(feature).

Type conversion: operators and functions that expect arguments to be of particular
type automatically convert the arguments to that type, e.g. sin('2.1') will convert
the argument to a double, length(123) will first convert the number to a string.
Explicit conversion can be achieved with toint, toreal, tostring functions.
If implicit or explicit conversion is invalid, the evaluation returns an error.
Comparison operators do numeric comparison in case both operators are numeric (int/double)
or they can be converted to numeric types.

Arithmetic operators do integer arithmetics if both operands are integer. That is
2+2 yields integer 4, but 2.0+2 returns real number 4.0. There are also two versions of
division and modulo operators: 1.0/2 returns 0.5 while 1/2 returns 0.

@note added in 2.0
*/
class CORE_EXPORT QgsExpression
{
  public:
    QgsExpression( const QString& expr );
    ~QgsExpression();

    //! Returns true if an error occurred when parsing the input expression
    bool hasParserError() const { return !mParserErrorString.isNull(); }
    //! Returns parser error
    QString parserErrorString() const { return mParserErrorString; }

    //! Get the expression ready for evaluation - find out column indexes.
    bool prepare( const QgsFieldMap& fields );

    //! Get list of columns referenced by the expression
    QStringList referencedColumns();
    //! Returns true if the expression uses feature geometry for some computation
    bool needsGeometry();

    // evaluation

    //! Evaluate the feature and return the result
    //! @note prepare() should be called before calling this method
    QVariant evaluate( QgsFeature* f = NULL );

    //! Evaluate the feature and return the result
    //! @note this method does not expect that prepare() has been called on this instance
    QVariant evaluate( QgsFeature* f, const QgsFieldMap& fields );

    //! Returns true if an error occurred when evaluating last input
    bool hasEvalError() const { return !mEvalErrorString.isNull(); }
    //! Returns evaluation error
    QString evalErrorString() const { return mEvalErrorString; }
    //! Set evaluation error (used internally by evaluation functions)
    void setEvalErrorString( QString str ) { mEvalErrorString = str; }

    //! Set the number for $rownum special column
    void setCurrentRowNumber( int rowNumber ) { mRowNumber = rowNumber; }
    //! Return the number used for $rownum special column
    int currentRowNumber() { return mRowNumber; }

    //! Return the parsed expression as a string - useful for debugging
    QString dump() const;

    //! Return calculator used for distance and area calculations
    //! (used by internal functions)
    QgsDistanceArea* geomCalculator() { if ( !mCalc ) initGeomCalculator(); return mCalc; }

    //

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
      boEQ,  // =
      boNE,  // <>
      boLE,  // <=
      boGE,  // >=
      boLT,  // <
      boGT,  // >
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
      boMod,
      boPow,

      // strings
      boConcat,
    };

    static const char* BinaryOperatorText[];
    static const char* UnaryOperatorText[];

    static const char* BinaryOgcOperatorText[];
    static const char* UnaryOgcOperatorText[];

    typedef QVariant( *FcnEval )( const QVariantList& values, QgsFeature* f, QgsExpression* parent );

    struct FunctionDef
    {
      FunctionDef( QString fnname, int params, FcnEval fcn, QString group, QString helpText = QString(), bool usesGeometry = false )
          : mName( fnname ), mParams( params ), mFcn( fcn ), mUsesGeometry( usesGeometry ), mGroup( group ), mHelpText( helpText ) {}
      /** The name of the function. */
      QString mName;
      /** The number of parameters this function takes. */
      int mParams;
      /** Pointer to fucntion. */
      FcnEval mFcn;
      /** Does this function use a geometry object. */
      bool mUsesGeometry;
      /** The group the function belongs to. */
      QString mGroup;
      /** The help text for the function. */
      QString mHelpText;
    };

    static const QList<FunctionDef> &BuiltinFunctions();
    static QList<FunctionDef> gmBuiltinFunctions;

    // tells whether the identifier is a name of existing function
    static bool isFunctionName( QString name );

    // return index of the function in BuiltinFunctions array
    static int functionIndex( QString name );

    /**  Returns the number of functions defined in the parser
      *  @return The number of function defined in the parser.
      */
    static int functionCount();

    //! return quoted column reference (in double quotes)
    static QString quotedColumnRef( QString name ) { return QString( "\"%1\"" ).arg( name.replace( "\"", "\"\"" ) ); }

    //////

    class Visitor; // visitor interface is defined below

    class CORE_EXPORT Node
    {
      public:
        virtual ~Node() {}
        // abstract virtual eval function
        // errors are reported to the parent
        virtual QVariant eval( QgsExpression* parent, QgsFeature* f ) = 0;

        // abstract virtual preparation function
        // errors are reported to the parent
        virtual bool prepare( QgsExpression* parent, const QgsFieldMap& fields ) = 0;

        virtual QString dump() const = 0;

        virtual void toOgcFilter( QDomDocument &doc, QDomElement &element ) const { Q_UNUSED( doc ); Q_UNUSED( element ); }
        static QgsExpression::Node* createFromOgcFilter( QDomElement &element, QString &errorMessage );

        virtual QStringList referencedColumns() const = 0;
        virtual bool needsGeometry() const = 0;

        // support for visitor pattern
        virtual void accept( Visitor& v ) = 0;
    };

    class CORE_EXPORT NodeList
    {
      public:
        NodeList() {}
        virtual ~NodeList() { foreach( Node* n, mList ) delete n; }
        void append( Node* node ) { mList.append( node ); }
        int count() { return mList.count(); }
        QList<Node*> list() { return mList; }

        virtual QString dump() const;
        virtual void toOgcFilter( QDomDocument &doc, QDomElement &element ) const;

      protected:
        QList<Node*> mList;
    };

    class CORE_EXPORT NodeUnaryOperator : public Node
    {
      public:
        NodeUnaryOperator( UnaryOperator op, Node* operand ) : mOp( op ), mOperand( operand ) {}
        ~NodeUnaryOperator() { delete mOperand; }

        UnaryOperator op() { return mOp; }
        Node* operand() { return mOperand; }

        virtual bool prepare( QgsExpression* parent, const QgsFieldMap& fields );
        virtual QVariant eval( QgsExpression* parent, QgsFeature* f );
        virtual QString dump() const;

        virtual void toOgcFilter( QDomDocument &doc, QDomElement &element ) const;
        static QgsExpression::Node* createFromOgcFilter( QDomElement &element, QString &errorMessage );

        virtual QStringList referencedColumns() const { return mOperand->referencedColumns(); }
        virtual bool needsGeometry() const { return mOperand->needsGeometry(); }
        virtual void accept( Visitor& v ) { v.visit( this ); }

      protected:
        UnaryOperator mOp;
        Node* mOperand;
    };

    class CORE_EXPORT NodeBinaryOperator : public Node
    {
      public:
        NodeBinaryOperator( BinaryOperator op, Node* opLeft, Node* opRight ) : mOp( op ), mOpLeft( opLeft ), mOpRight( opRight ) {}
        ~NodeBinaryOperator() { delete mOpLeft; delete mOpRight; }

        BinaryOperator op() { return mOp; }
        Node* opLeft() { return mOpLeft; }
        Node* opRight() { return mOpRight; }

        virtual bool prepare( QgsExpression* parent, const QgsFieldMap& fields );
        virtual QVariant eval( QgsExpression* parent, QgsFeature* f );
        virtual QString dump() const;

        virtual void toOgcFilter( QDomDocument &doc, QDomElement &element ) const;
        static QgsExpression::Node* createFromOgcFilter( QDomElement &element, QString &errorMessage );

        virtual QStringList referencedColumns() const { return mOpLeft->referencedColumns() + mOpRight->referencedColumns(); }
        virtual bool needsGeometry() const { return mOpLeft->needsGeometry() || mOpRight->needsGeometry(); }
        virtual void accept( Visitor& v ) { v.visit( this ); }

      protected:
        bool compare( double diff );
        int computeInt( int x, int y );
        double computeDouble( double x, double y );

        BinaryOperator mOp;
        Node* mOpLeft;
        Node* mOpRight;
    };

    class CORE_EXPORT NodeInOperator : public Node
    {
      public:
        NodeInOperator( Node* node, NodeList* list, bool notin = false ) : mNode( node ), mList( list ), mNotIn( notin ) {}
        virtual ~NodeInOperator() { delete mNode; delete mList; }

        Node* node() { return mNode; }
        bool isNotIn() { return mNotIn; }
        NodeList* list() { return mList; }

        virtual bool prepare( QgsExpression* parent, const QgsFieldMap& fields );
        virtual QVariant eval( QgsExpression* parent, QgsFeature* f );
        virtual QString dump() const;

        virtual void toOgcFilter( QDomDocument &doc, QDomElement &element ) const;

        virtual QStringList referencedColumns() const { QStringList lst( mNode->referencedColumns() ); foreach( Node* n, mList->list() ) lst.append( n->referencedColumns() ); return lst; }
        virtual bool needsGeometry() const { bool needs = false; foreach( Node* n, mList->list() ) needs |= n->needsGeometry(); return needs; }
        virtual void accept( Visitor& v ) { v.visit( this ); }

      protected:
        Node* mNode;
        NodeList* mList;
        bool mNotIn;
    };

    class CORE_EXPORT NodeFunction : public Node
    {
      public:
        NodeFunction( int fnIndex, NodeList* args ): mFnIndex( fnIndex ), mArgs( args ) {}
        //NodeFunction( QString name, NodeList* args ) : mName(name), mArgs(args) {}
        virtual ~NodeFunction() { delete mArgs; }

        int fnIndex() { return mFnIndex; }
        NodeList* args() { return mArgs; }

        virtual bool prepare( QgsExpression* parent, const QgsFieldMap& fields );
        virtual QVariant eval( QgsExpression* parent, QgsFeature* f );
        virtual QString dump() const;

        virtual void toOgcFilter( QDomDocument &doc, QDomElement &element ) const;
        static QgsExpression::Node* createFromOgcFilter( QDomElement &element, QString &errorMessage );

        virtual QStringList referencedColumns() const { QStringList lst; if ( !mArgs ) return lst; foreach( Node* n, mArgs->list() ) lst.append( n->referencedColumns() ); return lst; }
        virtual bool needsGeometry() const { bool needs = BuiltinFunctions()[mFnIndex].mUsesGeometry; if ( mArgs ) { foreach( Node* n, mArgs->list() ) needs |= n->needsGeometry(); } return needs; }
        virtual void accept( Visitor& v ) { v.visit( this ); }

      protected:
        //QString mName;
        int mFnIndex;
        NodeList* mArgs;
    };

    class CORE_EXPORT NodeLiteral : public Node
    {
      public:
        NodeLiteral( QVariant value ) : mValue( value ) {}

        QVariant value() { return mValue; }

        virtual bool prepare( QgsExpression* parent, const QgsFieldMap& fields );
        virtual QVariant eval( QgsExpression* parent, QgsFeature* f );
        virtual QString dump() const;

        virtual void toOgcFilter( QDomDocument &doc, QDomElement &element ) const;
        static QgsExpression::Node* createFromOgcFilter( QDomElement &element, QString &errorMessage );

        virtual QStringList referencedColumns() const { return QStringList(); }
        virtual bool needsGeometry() const { return false; }
        virtual void accept( Visitor& v ) { v.visit( this ); }

      protected:
        QVariant mValue;
    };

    class CORE_EXPORT NodeColumnRef : public Node
    {
      public:
        NodeColumnRef( QString name ) : mName( name ), mIndex( -1 ) {}

        QString name() { return mName; }

        virtual bool prepare( QgsExpression* parent, const QgsFieldMap& fields );
        virtual QVariant eval( QgsExpression* parent, QgsFeature* f );
        virtual QString dump() const;

        virtual void toOgcFilter( QDomDocument &doc, QDomElement &element ) const;
        static QgsExpression::Node* createFromOgcFilter( QDomElement &element, QString &errorMessage );

        virtual QStringList referencedColumns() const { return QStringList( mName ); }
        virtual bool needsGeometry() const { return false; }
        virtual void accept( Visitor& v ) { v.visit( this ); }

      protected:
        QString mName;
        int mIndex;
    };

    class CORE_EXPORT WhenThen
    {
      public:
        WhenThen( Node* whenExp, Node* thenExp ) : mWhenExp( whenExp ), mThenExp( thenExp ) {}
        ~WhenThen() { delete mWhenExp; delete mThenExp; }

        //protected:
        Node* mWhenExp;
        Node* mThenExp;
    };
    typedef QList<WhenThen*> WhenThenList;

    class CORE_EXPORT NodeCondition : public Node
    {
      public:
        NodeCondition( WhenThenList* conditions, Node* elseExp = NULL ) : mConditions( *conditions ), mElseExp( elseExp ) { delete conditions; }
        ~NodeCondition() { delete mElseExp; foreach( WhenThen* cond, mConditions ) delete cond; }

        virtual QVariant eval( QgsExpression* parent, QgsFeature* f );
        virtual bool prepare( QgsExpression* parent, const QgsFieldMap& fields );
        virtual QString dump() const;

        virtual void toOgcFilter( QDomDocument &doc, QDomElement &element ) const;

        virtual QStringList referencedColumns() const;
        virtual bool needsGeometry() const;
        virtual void accept( Visitor& v ) { v.visit( this ); }

      protected:
        WhenThenList mConditions;
        Node* mElseExp;
    };

    //////

    /** support for visitor pattern - algorithms dealing with the expressions
        may be implemented without modifying the Node classes */
    class CORE_EXPORT Visitor
    {
      public:
        virtual ~Visitor() {}
        virtual void visit( NodeUnaryOperator* n ) = 0;
        virtual void visit( NodeBinaryOperator* n ) = 0;
        virtual void visit( NodeInOperator* n ) = 0;
        virtual void visit( NodeFunction* n ) = 0;
        virtual void visit( NodeLiteral* n ) = 0;
        virtual void visit( NodeColumnRef* n ) = 0;
        virtual void visit( NodeCondition* n ) = 0;
    };

    /** entry function for the visitor pattern */
    void acceptVisitor( Visitor& v );

    // convert from/to OGC Filter
    void toOgcFilter( QDomDocument &doc, QDomElement &element ) const;
    static QgsExpression* createFromOgcFilter( QDomElement &element );

  protected:
    // internally used to create an empty expression
    QgsExpression() : mRootNode( NULL ), mRowNumber( 0 ), mCalc( NULL ) {}

    QString mExpression;
    Node* mRootNode;

    QString mParserErrorString;
    QString mEvalErrorString;

    int mRowNumber;

    void initGeomCalculator();
    QgsDistanceArea* mCalc;
};

#endif // QGSEXPRESSION_H
