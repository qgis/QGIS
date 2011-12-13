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
    //! Creates ogc filter xml document. Supports minimum standard filter according to the OGC filter specs (=,!=,<,>,<=,>=,AND,OR,NOT)
    //! @return true in case of success. False if string contains something that goes beyond the minimum standard filter
    bool toOGCFilter( QDomDocument& doc ) const;

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
      boILike,
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

    class Node
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

        virtual QStringList referencedColumns() const = 0;
        virtual bool needsGeometry() const = 0;
        virtual bool toOGCFilter( QDomDocument& doc, QDomElement& parent ) const { Q_UNUSED( doc ); Q_UNUSED( parent ); return false; }
    };

    class NodeList
    {
      public:
        NodeList() {}
        ~NodeList() { foreach( Node* n, mList ) delete n; }
        void append( Node* node ) { mList.append( node ); }
        int count() { return mList.count(); }
        QList<Node*> list() { return mList; }

        virtual QString dump() const;
      protected:
        QList<Node*> mList;
    };

    class NodeUnaryOperator : public Node
    {
      public:
        NodeUnaryOperator( UnaryOperator op, Node* operand ) : mOp( op ), mOperand( operand ) {}
        ~NodeUnaryOperator() { delete mOperand; }

        virtual bool prepare( QgsExpression* parent, const QgsFieldMap& fields );
        virtual QVariant eval( QgsExpression* parent, QgsFeature* f );
        virtual QString dump() const;
        virtual QStringList referencedColumns() const { return mOperand->referencedColumns(); }
        virtual bool needsGeometry() const { return mOperand->needsGeometry(); }
        virtual bool toOGCFilter( QDomDocument& doc, QDomElement& parent ) const;
      protected:
        UnaryOperator mOp;
        Node* mOperand;
    };

    class NodeBinaryOperator : public Node
    {
      public:
        NodeBinaryOperator( BinaryOperator op, Node* opLeft, Node* opRight ) : mOp( op ), mOpLeft( opLeft ), mOpRight( opRight ) {}
        ~NodeBinaryOperator() { delete mOpLeft; delete mOpRight; }

        virtual bool prepare( QgsExpression* parent, const QgsFieldMap& fields );
        virtual QVariant eval( QgsExpression* parent, QgsFeature* f );
        virtual QString dump() const;
        virtual QStringList referencedColumns() const { return mOpLeft->referencedColumns() + mOpRight->referencedColumns(); }
        virtual bool needsGeometry() const { return mOpLeft->needsGeometry() || mOpRight->needsGeometry(); }
        virtual bool toOGCFilter( QDomDocument& doc, QDomElement& parent ) const;

      protected:
        bool compare( double diff );
        int computeInt( int x, int y );
        double computeDouble( double x, double y );

        BinaryOperator mOp;
        Node* mOpLeft;
        Node* mOpRight;
    };

    class NodeInOperator : public Node
    {
      public:
        NodeInOperator( Node* node, NodeList* list, bool notin = false ) : mNode( node ), mList( list ), mNotIn( notin ) {}
        ~NodeInOperator() { delete mNode; delete mList; }

        virtual bool prepare( QgsExpression* parent, const QgsFieldMap& fields );
        virtual QVariant eval( QgsExpression* parent, QgsFeature* f );
        virtual QString dump() const;
        virtual QStringList referencedColumns() const { QStringList lst( mNode->referencedColumns() ); foreach( Node* n, mList->list() ) lst.append( n->referencedColumns() ); return lst; }
        virtual bool needsGeometry() const { bool needs = false; foreach( Node* n, mList->list() ) needs |= n->needsGeometry(); return needs; }
      protected:
        Node* mNode;
        NodeList* mList;
        bool mNotIn;
    };

    class NodeFunction : public Node
    {
      public:
        NodeFunction( int fnIndex, NodeList* args ): mFnIndex( fnIndex ), mArgs( args ) {}
        //NodeFunction( QString name, NodeList* args ) : mName(name), mArgs(args) {}
        ~NodeFunction() { delete mArgs; }

        virtual bool prepare( QgsExpression* parent, const QgsFieldMap& fields );
        virtual QVariant eval( QgsExpression* parent, QgsFeature* f );
        virtual QString dump() const;
        virtual QStringList referencedColumns() const { QStringList lst; if ( !mArgs ) return lst; foreach( Node* n, mArgs->list() ) lst.append( n->referencedColumns() ); return lst; }
        virtual bool needsGeometry() const { bool needs = BuiltinFunctions()[mFnIndex].mUsesGeometry; if ( mArgs ) { foreach( Node* n, mArgs->list() ) needs |= n->needsGeometry(); } return needs; }
      protected:
        //QString mName;
        int mFnIndex;
        NodeList* mArgs;
    };

    class NodeLiteral : public Node
    {
      public:
        NodeLiteral( QVariant value ) : mValue( value ) {}

        virtual bool prepare( QgsExpression* parent, const QgsFieldMap& fields );
        virtual QVariant eval( QgsExpression* parent, QgsFeature* f );
        virtual QString dump() const;
        virtual QStringList referencedColumns() const { return QStringList(); }
        virtual bool needsGeometry() const { return false; }
        virtual bool toOGCFilter( QDomDocument& doc, QDomElement& parent ) const;
      protected:
        QVariant mValue;
    };

    class NodeColumnRef : public Node
    {
      public:
        NodeColumnRef( QString name ) : mName( name ), mIndex( -1 ) {}

        virtual bool prepare( QgsExpression* parent, const QgsFieldMap& fields );
        virtual QVariant eval( QgsExpression* parent, QgsFeature* f );
        virtual QString dump() const;
        virtual QStringList referencedColumns() const { return QStringList( mName ); }
        virtual bool needsGeometry() const { return false; }
        virtual bool toOGCFilter( QDomDocument& doc, QDomElement& parent ) const;
      protected:
        QString mName;
        int mIndex;
    };


  protected:

    QString mExpression;
    Node* mRootNode;

    QString mParserErrorString;
    QString mEvalErrorString;

    int mRowNumber;

    void initGeomCalculator();
    QgsDistanceArea* mCalc;
};

#endif // QGSEXPRESSION_H
