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

#include <QMetaType>
#include <QStringList>
#include <QVariant>
#include <QList>
#include <QDomDocument>
#include <QCoreApplication>
#include <QSet>

#include "qgis.h"
#include "qgsunittypes.h"
#include "qgsinterval.h"

class QgsFeature;
class QgsGeometry;
class QgsOgcUtils;
class QgsVectorLayer;
class QgsVectorDataProvider;
class QgsField;
class QgsFields;
class QgsDistanceArea;
class QDomElement;
class QgsExpressionContext;
class QgsExpressionPrivate;

/** \ingroup core
Class for parsing and evaluation of expressions (formerly called "search strings").
The expressions try to follow both syntax and semantics of SQL expressions.

Usage:
\code{.cpp}
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
\endcode

Three Value Logic
=================

Similarly to SQL, this class supports three-value logic: true/false/unknown.
Unknown value may be a result of operations with missing data (NULL). Please note
that NULL is different value than zero or an empty string. For example
3 > NULL returns unknown.

There is no special (three-value) 'boolean' type: true/false is represented as
1/0 integer, unknown value is represented the same way as NULL values: invalid QVariant.

Performance
===========

For better performance with many evaluations you may first call prepare(fields) function
to find out indices of columns and then repeatedly call evaluate(feature).

Type conversion
===============

Operators and functions that expect arguments to be of a particular
type automatically convert the arguments to that type, e.g. sin('2.1') will convert
the argument to a double, length(123) will first convert the number to a string.
Explicit conversion can be achieved with to_int, to_real, to_string functions.
If implicit or explicit conversion is invalid, the evaluation returns an error.
Comparison operators do numeric comparison in case both operators are numeric (int/double)
or they can be converted to numeric types.

Implicit sharing
================

This class is implicitly shared, copying has a very low overhead.
It is normally preferable to call `QgsExpression( otherExpression )` instead of
`QgsExpression( otherExpression.expression() )`. A deep copy will only be made
when {@link prepare()} is called. For usage this means mainly, that you should
normally keep an unprepared master copy of a QgsExpression and whenever using it
with a particular QgsFeatureIterator copy it just before and prepare it using the
same context as the iterator.

Implicit sharing was added in 2.14

*/

Q_NOWARN_DEPRECATED_PUSH
class CORE_EXPORT QgsExpression
{
    Q_DECLARE_TR_FUNCTIONS( QgsExpression )
  public:
    /**
     * Creates a new expression based on the provided string.
     * The string will immediately be parsed. For optimization
     * {@link prepare()} should alwys be called before every
     * loop in which this expression is used.
     */
    QgsExpression( const QString& expr );

    /**
     * Create a copy of this expression. This is preferred
     * over recreating an expression from a string since
     * it does not need to be re-parsed.
     */
    QgsExpression( const QgsExpression& other );
    /**
     * Create a copy of this expression. This is preferred
     * over recreating an expression from a string since
     * it does not need to be re-parsed.
     */
    QgsExpression& operator=( const QgsExpression& other );
    ~QgsExpression();

    //! Returns true if an error occurred when parsing the input expression
    bool hasParserError() const;
    //! Returns parser error
    QString parserErrorString() const;

    class Node;

    //! Returns root node of the expression. Root node is null is parsing has failed
    const Node* rootNode() const;

    //! Get the expression ready for evaluation - find out column indexes.
    //! @deprecated use prepare( const QgsExpressionContext *context ) instead
    Q_DECL_DEPRECATED bool prepare( const QgsFields &fields );

    /** Get the expression ready for evaluation - find out column indexes.
     * @param context context for preparing expression
     * @note added in QGIS 2.12
     */
    bool prepare( const QgsExpressionContext *context );

    /**
     * Get list of columns referenced by the expression.
     * @note if the returned list contains the QgsFeatureRequest::AllAttributes constant then
     * all attributes from the layer are required for evaluation of the expression.
     * QgsFeatureRequest::setSubsetOfAttributes automatically handles this case.
     *
     * TODO QGIS3: Return QSet<QString>
     */
    QStringList referencedColumns() const;

    //! Returns true if the expression uses feature geometry for some computation
    bool needsGeometry() const;

    // evaluation

    //! Evaluate the feature and return the result
    //! @note prepare() should be called before calling this method
    //! @deprecated use evaluate( const QgsExpressionContext* context ) instead
    Q_DECL_DEPRECATED QVariant evaluate( const QgsFeature* f );

    //! Evaluate the feature and return the result
    //! @note prepare() should be called before calling this method
    //! @note available in python bindings as evaluatePrepared
    //! @deprecated use evaluate( const QgsExpressionContext* context ) instead
    Q_DECL_DEPRECATED QVariant evaluate( const QgsFeature& f );

    //! Evaluate the feature and return the result
    //! @note this method does not expect that prepare() has been called on this instance
    //! @deprecated use evaluate( const QgsExpressionContext* context ) instead
    Q_DECL_DEPRECATED QVariant evaluate( const QgsFeature* f, const QgsFields& fields );

    //! Evaluate the feature and return the result
    //! @note this method does not expect that prepare() has been called on this instance
    //! @note not available in python bindings
    //! @deprecated use evaluate( const QgsExpressionContext* context ) instead
    Q_DECL_DEPRECATED QVariant evaluate( const QgsFeature& f, const QgsFields& fields );

    /** Evaluate the feature and return the result.
     * @note this method does not expect that prepare() has been called on this instance
     * @note added in QGIS 2.12
     */
    QVariant evaluate();

    /** Evaluate the expression against the specified context and return the result.
     * @param context context for evaluating expression
     * @note prepare() should be called before calling this method.
     * @note added in QGIS 2.12
     */
    QVariant evaluate( const QgsExpressionContext* context );

    //! Returns true if an error occurred when evaluating last input
    bool hasEvalError() const;
    //! Returns evaluation error
    QString evalErrorString() const;
    //! Set evaluation error (used internally by evaluation functions)
    void setEvalErrorString( const QString& str );

    //! Set the number for $rownum special column
    //! @deprecated use QgsExpressionContext to set row number instead
    Q_DECL_DEPRECATED void setCurrentRowNumber( int rowNumber );
    //! Return the number used for $rownum special column
    //! @deprecated use QgsExpressionContext to retrieve row number instead
    Q_DECL_DEPRECATED int currentRowNumber();    //TODO QGIS 3.0: make the following methods private. They are still required for replaceExpressionText
    //but should not be publicly used
    /** Assign a special column
     * @deprecated use global or project QgsExpressionContext variables instead
     */
    Q_DECL_DEPRECATED static void setSpecialColumn( const QString& name, const QVariant& value );
    /** Unset a special column
     * @deprecated use global or project QgsExpressionContext variables instead
     */
    Q_DECL_DEPRECATED static void unsetSpecialColumn( const QString& name );
    /** Return the value of the given special column or a null QVariant if undefined
     * @deprecated use global or project QgsExpressionContext variables instead
     */
    Q_DECL_DEPRECATED static QVariant specialColumn( const QString& name );

    /** Check whether a special column exists
     * @note added in 2.2
     */
    static bool hasSpecialColumn( const QString& name );

    /** Checks whether an expression consists only of a single field reference
     * @note added in 2.9
     */
    bool isField() const { return rootNode() && dynamic_cast<const NodeColumnRef*>( rootNode() ) ;}

    //! @deprecated use QgsExpressionContext variant instead
    Q_DECL_DEPRECATED static bool isValid( const QString& text, const QgsFields& fields, QString &errorMessage );

    /** Tests whether a string is a valid expression.
     * @param text string to test
     * @param context optional expression context
     * @param errorMessage will be filled with any error message from the validation
     * @returns true if string is a valid expression
     * @note added in QGIS 2.12
     */
    static bool isValid( const QString& text, const QgsExpressionContext* context, QString &errorMessage );

    void setScale( double scale );

    double scale();

    //! Return the original, unmodified expression string.
    //! If there was none supplied because it was constructed by sole
    //! API calls, dump() will be used to create one instead.
    QString expression() const;

    //! Return an expression string, constructed from the internal
    //! abstract syntax tree. This does not contain any nice whitespace
    //! formatting or comments. In general it is preferrable to use
    //! expression() instead.
    QString dump() const;

    /** Return calculator used for distance and area calculations
     * (used by $length, $area and $perimeter functions only)
     * @see setGeomCalculator()
     * @see distanceUnits()
     * @see areaUnits()
     */
    QgsDistanceArea *geomCalculator();

    /** Sets the geometry calculator used for distance and area calculations in expressions.
     * (used by $length, $area and $perimeter functions only). By default, no geometry
     * calculator is set and all distance and area calculations are performed using simple
     * cartesian methods (ie no ellipsoidal calculations).
     * @see geomCalculator()
     */
    //TODO QGIS 3.0 change calc to a pointer, so that calculator can be cleared by passing nullptr
    void setGeomCalculator( const QgsDistanceArea &calc );

    /** Returns the desired distance units for calculations involving geomCalculator(), eg "$length" and "$perimeter".
     * @note distances are only converted when a geomCalculator() has been set
     * @note added in QGIS 2.14
     * @see setDistanceUnits()
     * @see areaUnits()
     */
    QGis::UnitType distanceUnits() const;

    /** Sets the desired distance units for calculations involving geomCalculator(), eg "$length" and "$perimeter".
     * @note distances are only converted when a geomCalculator() has been set
     * @note added in QGIS 2.14
     * @see distanceUnits()
     * @see setAreaUnits()
     */
    void setDistanceUnits( QGis::UnitType unit );

    /** Returns the desired areal units for calculations involving geomCalculator(), eg "$area".
     * @note areas are only converted when a geomCalculator() has been set
     * @note added in QGIS 2.14
     * @see setAreaUnits()
     * @see distanceUnits()
     */
    QgsUnitTypes::AreaUnit areaUnits() const;

    /** Sets the desired areal units for calculations involving geomCalculator(), eg "$area".
     * @note areas are only converted when a geomCalculator() has been set
     * @note added in QGIS 2.14
     * @see areaUnits()
     * @see setDistanceUnits()
     */
    void setAreaUnits( QgsUnitTypes::AreaUnit unit );

    /** This function currently replaces each expression between [% and %]
     * in the string with the result of its evaluation on the feature
     * passed as argument.
     *
     * Additional substitutions can be passed through the substitutionMap
     * parameter
     * @param action
     * @param feat
     * @param layer
     * @param substitutionMap
     * @param distanceArea optional QgsDistanceArea. If specified, the QgsDistanceArea is used for distance and area conversion
     * @deprecated use QgsExpressionContext variant instead
     */
    Q_DECL_DEPRECATED static QString replaceExpressionText( const QString &action, const QgsFeature *feat,
        QgsVectorLayer *layer,
        const QMap<QString, QVariant> *substitutionMap = nullptr,
        const QgsDistanceArea* distanceArea = nullptr );

    /** This function replaces each expression between [% and %]
     * in the string with the result of its evaluation with the specified context
     *
     * Additional substitutions can be passed through the substitutionMap parameter
     * @param action
     * @param context expression context
     * @param substitutionMap
     * @param distanceArea optional QgsDistanceArea. If specified, the QgsDistanceArea is used for distance
     * and area conversion
     * @note added in QGIS 2.12
     */
    static QString replaceExpressionText( const QString &action, const QgsExpressionContext* context,
                                          const QMap<QString, QVariant> *substitutionMap = nullptr,
                                          const QgsDistanceArea* distanceArea = nullptr );

    /** Attempts to evaluate a text string as an expression to a resultant double
     * value.
     * @param text text to evaluate as expression
     * @param fallbackValue value to return if text can not be evaluated as a double
     * @returns evaluated double value, or fallback value
     * @note added in QGIS 2.7
     * @note this method is inefficient for bulk evaluation of expressions, it is intended
     * for one-off evaluations only.
     */
    static double evaluateToDouble( const QString& text, const double fallbackValue );

    /**
     * @brief list of unary operators
     * @note if any change is made here, the definition of QgsExpression::UnaryOperatorText[] must be adapted.
     */
    enum UnaryOperator
    {
      uoNot,
      uoMinus,
    };

    /**
     * @brief list of binary operators
     * @note if any change is made here, the definition of QgsExpression::BinaryOperatorText[] must be adapted.
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

    enum SpatialOperator
    {
      soBbox,
      soIntersects,
      soContains,
      soCrosses,
      soEquals,
      soDisjoint,
      soOverlaps,
      soTouches,
      soWithin,
    };

    //! @note not available in Python bindings
    static const char* BinaryOperatorText[];

    //! @note not available in Python bindings
    static const char* UnaryOperatorText[];

    /** \ingroup core
      * Represents a single parameter passed to a function.
      * \note added in QGIS 2.16
      */
    class CORE_EXPORT Parameter
    {
      public:

        /** Constructor for Parameter.
         * @param name parameter name, used when named parameter are specified in an expression
         * @param optional set to true if parameter should be optional
         * @param defaultValue default value to use for optional parameters
         */
        Parameter( const QString& name,
                   bool optional = false,
                   const QVariant& defaultValue = QVariant() )
            : mName( name )
            , mOptional( optional )
            , mDefaultValue( defaultValue )
        {}

        //! Returns the name of the parameter.
        QString name() const { return mName; }

        //! Returns true if the parameter is optional.
        bool optional() const { return mOptional; }

        //! Returns the default value for the parameter.
        QVariant defaultValue() const { return mDefaultValue; }

        bool operator==( const Parameter& other ) const
        {
          return ( QString::compare( mName, other.mName, Qt::CaseInsensitive ) == 0 );
        }

      private:
        QString mName;
        bool mOptional;
        QVariant mDefaultValue;
    };

    //! List of parameters, used for function definition
    typedef QList< Parameter > ParameterList;

    //! @deprecated will be removed in QGIS 3.0
    typedef QVariant( *FcnEval )( const QVariantList& values, const QgsFeature* f, QgsExpression* parent );

    /** Function definition for evaluation against an expression context, using a list of values as parameters to the function.
     */
    typedef QVariant( *FcnEvalContext )( const QVariantList& values, const QgsExpressionContext* context, QgsExpression* parent );

    /** \ingroup core
      * A abstract base class for defining QgsExpression functions.
      */
    class CORE_EXPORT Function
    {
      public:

        //! Constructor for function which uses unnamed parameters
        Function( const QString& fnname,
                  int params,
                  const QString& group,
                  const QString& helpText = QString(),
                  bool usesGeometry = false,
                  const QStringList& referencedColumns = QStringList(),
                  bool lazyEval = false,
                  bool handlesNull = false,
                  bool isContextual = false )
            : mName( fnname )
            , mParams( params )
            , mUsesGeometry( usesGeometry )
            , mGroup( group )
            , mHelpText( helpText )
            , mReferencedColumns( referencedColumns )
            , mLazyEval( lazyEval )
            , mHandlesNull( handlesNull )
            , mIsContextual( isContextual )
        {
        }

        /** Constructor for function which uses named parameter list.
         * @note added in QGIS 2.16
         */
        Function( const QString& fnname,
                  const ParameterList& params,
                  const QString& group,
                  const QString& helpText = QString(),
                  bool usesGeometry = false,
                  const QStringList& referencedColumns = QStringList(),
                  bool lazyEval = false,
                  bool handlesNull = false,
                  bool isContextual = false )
            : mName( fnname )
            , mParams( 0 )
            , mParameterList( params )
            , mUsesGeometry( usesGeometry )
            , mGroup( group )
            , mHelpText( helpText )
            , mReferencedColumns( referencedColumns )
            , mLazyEval( lazyEval )
            , mHandlesNull( handlesNull )
            , mIsContextual( isContextual )
        {}

        virtual ~Function() {}

        /** The name of the function. */
        QString name() const { return mName; }

        /** The number of parameters this function takes. */
        int params() const { return mParameterList.isEmpty() ? mParams : mParameterList.count(); }

        /** The mininum number of parameters this function takes. */
        int minParams() const
        {
          if ( mParameterList.isEmpty() )
            return mParams;

          int min = 0;
          Q_FOREACH ( const Parameter& param, mParameterList )
          {
            if ( !param.optional() )
              min++;
          }
          return min;
        }

        /** Returns the list of named parameters for the function, if set.
         * @note added in QGIS 2.16
        */
        const ParameterList& parameters() const { return mParameterList; }

        /** Does this function use a geometry object. */
        //TODO QGIS 3.0 - rename to usesGeometry()
        bool usesgeometry() const { return mUsesGeometry; }

        /** Returns a list of possible aliases for the function. These include
         * other permissible names for the function, eg deprecated names.
         * @return list of known aliases
         * @note added in QGIS 2.9
         */
        virtual QStringList aliases() const { return QStringList(); }

        /** True if this function should use lazy evaluation.  Lazy evaluation functions take QgsExpression::Node objects
         * rather than the node results when called.  You can use node->eval(parent, feature) to evaluate the node and return the result
         * Functions are non lazy default and will be given the node return value when called **/
        bool lazyEval() const { return mLazyEval; }

        virtual QStringList referencedColumns() const { return mReferencedColumns; }

        /** Returns whether the function is only available if provided by a QgsExpressionContext object.
         * @note added in QGIS 2.12
         */
        bool isContextual() const { return mIsContextual; }

        /** The group the function belongs to. */
        QString group() const { return mGroup; }
        /** The help text for the function. */
        //TODO QGIS 3.0 - rename to helpText()
        const QString helptext() const { return mHelpText.isEmpty() ? QgsExpression::helptext( mName ) : mHelpText; }

        //! @deprecated Use QgsExpressionContext variant instead
        Q_DECL_DEPRECATED virtual QVariant func( const QVariantList&, const QgsFeature*, QgsExpression* );

        /** Returns result of evaluating the function.
         * @param values list of values passed to the function
         * @param context context expression is being evaluated against
         * @param parent parent expression
         * @returns result of function
         * @note named funcV2 in Python bindings. Will be renamed to func to replace deprecated method in QGIS 3.0.
         */
        //TODO QGIS 3.0 - rename python method
        virtual QVariant func( const QVariantList& values, const QgsExpressionContext* context, QgsExpression* parent );

        bool operator==( const Function& other ) const
        {
          if ( QString::compare( mName, other.mName, Qt::CaseInsensitive ) == 0 )
            return true;

          return false;
        }

        virtual bool handlesNull() const { return mHandlesNull; }

      private:
        QString mName;
        int mParams;
        ParameterList mParameterList;
        bool mUsesGeometry;
        QString mGroup;
        QString mHelpText;
        QStringList mReferencedColumns;
        bool mLazyEval;
        bool mHandlesNull;
        bool mIsContextual; //if true function is only available through an expression context
    };

    /** \ingroup core
      * c++ helper class for defining QgsExpression functions.
      * \note not available in Python bindings
      */
    class StaticFunction : public Function
    {
      public:
        //! @deprecated use QgsExpressionContext variant instead
        Q_DECL_DEPRECATED StaticFunction( const QString& fnname,
                                          int params,
                                          FcnEval fcn,
                                          const QString& group,
                                          const QString& helpText = QString(),
                                          bool usesGeometry = false,
                                          const QStringList& referencedColumns = QStringList(),
                                          bool lazyEval = false,
                                          const QStringList& aliases = QStringList(),
                                          bool handlesNull = false )
            : Function( fnname, params, group, helpText, usesGeometry, referencedColumns, lazyEval, handlesNull )
            , mFnc( fcn )
            , mContextFnc( nullptr )
            , mAliases( aliases )
        {}

        virtual ~StaticFunction() {}

        /** Static function for evaluation against a QgsExpressionContext, using an unnamed list of parameter values.
         */
        StaticFunction( const QString& fnname,
                        int params,
                        FcnEvalContext fcn,
                        const QString& group,
                        const QString& helpText = QString(),
                        bool usesGeometry = false,
                        const QStringList& referencedColumns = QStringList(),
                        bool lazyEval = false,
                        const QStringList& aliases = QStringList(),
                        bool handlesNull = false )
            : Function( fnname, params, group, helpText, usesGeometry, referencedColumns, lazyEval, handlesNull )
            , mFnc( nullptr )
            , mContextFnc( fcn )
            , mAliases( aliases )
        {}

        /** Static function for evaluation against a QgsExpressionContext, using a named list of parameter values.
         */
        StaticFunction( const QString& fnname,
                        const ParameterList& params,
                        FcnEvalContext fcn,
                        const QString& group,
                        const QString& helpText = QString(),
                        bool usesGeometry = false,
                        const QStringList& referencedColumns = QStringList(),
                        bool lazyEval = false,
                        const QStringList& aliases = QStringList(),
                        bool handlesNull = false )
            : Function( fnname, params, group, helpText, usesGeometry, referencedColumns, lazyEval, handlesNull )
            , mFnc( nullptr )
            , mContextFnc( fcn )
            , mAliases( aliases )
        {}

        //! @deprecated use QgsExpressionContext variant instead
        Q_DECL_DEPRECATED virtual QVariant func( const QVariantList& values, const QgsFeature* f, QgsExpression* parent ) override;

        /** Returns result of evaluating the function.
         * @param values list of values passed to the function
         * @param context context expression is being evaluated against
         * @param parent parent expression
         * @returns result of function
         */
        virtual QVariant func( const QVariantList& values, const QgsExpressionContext* context, QgsExpression* parent ) override
        {
          return mContextFnc ? mContextFnc( values, context, parent ) : QVariant();
        }

        virtual QStringList aliases() const override { return mAliases; }

      private:
        FcnEval mFnc;
        FcnEvalContext mContextFnc;
        QStringList mAliases;
    };

    //! @note not available in Python bindings
    static QList<Function*> gmFunctions;
    static const QList<Function*>& Functions();

    //! @note not available in Python bindings
    static QStringList gmBuiltinFunctions;
    static const QStringList& BuiltinFunctions();

    /** Registers a function to the expression engine. This is required to allow expressions to utilise the function.
     * @param function function to register
     * @param transferOwnership set to true to transfer ownership of function to expression engine
     * @returns true on successful registration
     * @see unregisterFunction
     */
    static bool registerFunction( Function* function, bool transferOwnership = false );

    /** Unregisters a function from the expression engine. The function will no longer be usable in expressions.
     * @param name function name
     * @see registerFunction
     */
    static bool unregisterFunction( const QString& name );

    //! List of functions owned by the expression engine
    //! @note not available in Python bindings
    static QList<Function*> gmOwnedFunctions;

    /** Deletes all registered functions whose ownership have been transferred to the expression engine.
     * @note added in QGIS 2.12
     */
    static void cleanRegisteredFunctions();

    //! tells whether the identifier is a name of existing function
    static bool isFunctionName( const QString& name );

    //! return index of the function in Functions array
    static int functionIndex( const QString& name );

    /** Returns the number of functions defined in the parser
     *  @return The number of function defined in the parser.
     */
    static int functionCount();

    /**
     * Returns a list of special Column definitions
     */
    static QList<Function*> specialColumns();

    /** Returns a quoted column reference (in double quotes)
     * @see quotedString()
     * @see quotedValue()
     */
    static QString quotedColumnRef( QString name );

    /** Returns a quoted version of a string (in single quotes)
     * @see quotedValue()
     * @see quotedColumnRef()
     */
    static QString quotedString( QString text );

    /** Returns a string representation of a literal value, including appropriate
     * quotations where required.
     * @param value value to convert to a string representation
     * @note added in QGIS 2.14
     * @see quotedString()
     * @see quotedColumnRef()
     */
    static QString quotedValue( const QVariant& value );

    /** Returns a string representation of a literal value, including appropriate
     * quotations where required.
     * @param value value to convert to a string representation
     * @param type value type
     * @note added in QGIS 2.14
     * @see quotedString()
     * @see quotedColumnRef()
     */
    static QString quotedValue( const QVariant& value, QVariant::Type type );

    //////

    class Visitor; // visitor interface is defined below

    enum NodeType
    {
      ntUnaryOperator,
      ntBinaryOperator,
      ntInOperator,
      ntFunction,
      ntLiteral,
      ntColumnRef,
      ntCondition
    };

    /** \ingroup core
     */
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
          * Abstract virtual eval method
          * Errors are reported to the parent
          * @deprecated use QgsExpressionContext variant instead
          */
        Q_DECL_DEPRECATED virtual QVariant eval( QgsExpression* parent, const QgsFeature* f );

        /**
         * Abstract virtual eval method
         * Errors are reported to the parent
         * @note added in QGIS 2.12
         */
        virtual QVariant eval( QgsExpression* parent, const QgsExpressionContext* context );

        /**
         * Abstract virtual preparation method
         * Errors are reported to the parent
         * @deprecated use QgsExpressionContext variant instead
         */
        Q_DECL_DEPRECATED virtual bool prepare( QgsExpression* parent, const QgsFields &fields );

        /**
         * Abstract virtual preparation method
         * Errors are reported to the parent
         * @note added in QGIS 2.12
         */
        virtual bool prepare( QgsExpression* parent, const QgsExpressionContext* context );

        /**
         * Abstract virtual dump method
         *
         * @return An expression which represents this node as string
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
         * Abstract virtual method which returns a list of columns required to
         * evaluate this node.
         *
         * When reimplementing this, you need to return any column that is required to
         * evaluate this node and in addition recursively collect all the columns required
         * to evaluate child nodes.
         *
         * @return A list of columns required to evaluate this expression
         */
        virtual QStringList referencedColumns() const = 0;

        /**
         * Abstract virtual method which returns if the geometry is required to evaluate
         * this expression.
         *
         * This needs to call `needsGeometry()` recursively on any child nodes.
         *
         * @return true if a geometry is required to evaluate this expression
         */
        virtual bool needsGeometry() const = 0;

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

    //! Named node
    //! @note added in QGIS 2.16
    //! \ingroup core
    class CORE_EXPORT NamedNode
    {
      public:

        /** Constructor for NamedNode
         * @param name node name
         * @param node node
         */
        NamedNode( const QString& name, Node* node )
            : name( name )
            , node( node )
        {}

        //! Node name
        QString name;

        //! Node
        Node* node;
    };

    /** \ingroup core
     */
    class CORE_EXPORT NodeList
    {
      public:
        NodeList() : mHasNamedNodes( false ) {}
        virtual ~NodeList() { qDeleteAll( mList ); }
        /** Takes ownership of the provided node */
        void append( Node* node ) { mList.append( node ); mNameList.append( QString() ); }

        /** Adds a named node. Takes ownership of the provided node.
         * @note added in QGIS 2.16
        */
        void append( NamedNode* node ) { mList.append( node->node ); mNameList.append( node->name.toLower() ); mHasNamedNodes = true; }

        /** Returns the number of nodes in the list.
         */
        int count() const { return mList.count(); }

        //! Returns true if list contains any named nodes
        //! @note added in QGIS 2.16
        bool hasNamedNodes() const { return mHasNamedNodes; }

        QList<Node*> list() { return mList; }

        //! Returns a list of names for nodes. Unnamed nodes will be indicated by an empty string in the list.
        //! @note added in QGIS 2.16
        QStringList names() const { return mNameList; }

        /** Creates a deep copy of this list. Ownership is transferred to the caller */
        NodeList* clone() const;

        virtual QString dump() const;

      protected:
        QList<Node*> mList;
        QStringList mNameList;

      private:

        bool mHasNamedNodes;
    };

    //TODO QGIS 3.0 - remove
    //! @deprecated use QgsInterval instead
    //! \ingroup core
    typedef QgsInterval Interval;

    /** \ingroup core
     */
    class CORE_EXPORT NodeUnaryOperator : public Node
    {
      public:
        NodeUnaryOperator( UnaryOperator op, Node* operand )
            : mOp( op )
            , mOperand( operand )
        {}
        ~NodeUnaryOperator() { delete mOperand; }

        UnaryOperator op() const { return mOp; }
        Node* operand() const { return mOperand; }

        virtual NodeType nodeType() const override { return ntUnaryOperator; }
        virtual bool prepare( QgsExpression* parent, const QgsExpressionContext* context ) override;
        virtual QVariant eval( QgsExpression* parent, const QgsExpressionContext* context ) override;
        virtual QString dump() const override;

        virtual QStringList referencedColumns() const override { return mOperand->referencedColumns(); }
        virtual bool needsGeometry() const override { return mOperand->needsGeometry(); }
        virtual void accept( Visitor& v ) const override { v.visit( *this ); }
        virtual Node* clone() const override;

      protected:
        UnaryOperator mOp;
        Node* mOperand;
    };

    /** \ingroup core
     */
    class CORE_EXPORT NodeBinaryOperator : public Node
    {
      public:
        NodeBinaryOperator( BinaryOperator op, Node* opLeft, Node* opRight )
            : mOp( op )
            , mOpLeft( opLeft )
            , mOpRight( opRight )
        {}
        ~NodeBinaryOperator() { delete mOpLeft; delete mOpRight; }

        BinaryOperator op() const { return mOp; }
        Node* opLeft() const { return mOpLeft; }
        Node* opRight() const { return mOpRight; }

        virtual NodeType nodeType() const override { return ntBinaryOperator; }
        virtual bool prepare( QgsExpression* parent, const QgsExpressionContext* context ) override;
        virtual QVariant eval( QgsExpression* parent, const QgsExpressionContext* context ) override;
        virtual QString dump() const override;

        virtual QStringList referencedColumns() const override { return mOpLeft->referencedColumns() + mOpRight->referencedColumns(); }
        virtual bool needsGeometry() const override { return mOpLeft->needsGeometry() || mOpRight->needsGeometry(); }
        virtual void accept( Visitor& v ) const override { v.visit( *this ); }
        virtual Node* clone() const override;

        int precedence() const;
        bool leftAssociative() const;

      protected:
        bool compare( double diff );
        int computeInt( int x, int y );
        double computeDouble( double x, double y );

        /** Computes the result date time calculation from a start datetime and an interval
         * @param d start datetime
         * @param i interval to add or subtract (depending on mOp)
         */
        QDateTime computeDateTimeFromInterval( const QDateTime& d, QgsInterval* i );

        BinaryOperator mOp;
        Node* mOpLeft;
        Node* mOpRight;
    };

    /** \ingroup core
     */
    class CORE_EXPORT NodeInOperator : public Node
    {
      public:
        NodeInOperator( Node* node, NodeList* list, bool notin = false )
            : mNode( node )
            , mList( list )
            , mNotIn( notin )
        {}
        virtual ~NodeInOperator() { delete mNode; delete mList; }

        Node* node() const { return mNode; }
        bool isNotIn() const { return mNotIn; }
        NodeList* list() const { return mList; }

        virtual NodeType nodeType() const override { return ntInOperator; }
        virtual bool prepare( QgsExpression* parent, const QgsExpressionContext* context ) override;
        virtual QVariant eval( QgsExpression* parent, const QgsExpressionContext* context ) override;
        virtual QString dump() const override;

        virtual QStringList referencedColumns() const override { QStringList lst( mNode->referencedColumns() ); Q_FOREACH ( const Node* n, mList->list() ) lst.append( n->referencedColumns() ); return lst; }
        virtual bool needsGeometry() const override { bool needs = false; Q_FOREACH ( Node* n, mList->list() ) needs |= n->needsGeometry(); return needs; }
        virtual void accept( Visitor& v ) const override { v.visit( *this ); }
        virtual Node* clone() const override;

      protected:
        Node* mNode;
        NodeList* mList;
        bool mNotIn;
    };

    /** \ingroup core
     */
    class CORE_EXPORT NodeFunction : public Node
    {
      public:
        NodeFunction( int fnIndex, NodeList* args ) : mFnIndex( fnIndex )
        {
          const ParameterList& functionParams = Functions()[mFnIndex]->parameters();
          if ( !args || !args->hasNamedNodes() || functionParams.isEmpty() )
          {
            // no named parameters, or function does not support them
            mArgs = args;
          }
          else
          {
            mArgs = new NodeList();

            int idx = 0;
            //first loop through unnamed arguments
            while ( args->names().at( idx ).isEmpty() )
            {
              mArgs->append( args->list().at( idx )->clone() );
              idx++;
            }

            //next copy named parameters in order expected by function
            for ( ; idx < functionParams.count(); ++idx )
            {
              int nodeIdx = args->names().indexOf( functionParams.at( idx ).name().toLower() );
              if ( nodeIdx < 0 )
              {
                //parameter not found - insert default value for parameter
                mArgs->append( new NodeLiteral( functionParams.at( idx ).defaultValue() ) );
              }
              else
              {
                mArgs->append( args->list().at( nodeIdx )->clone() );
              }
            }

            delete args;
          }
        }

        virtual ~NodeFunction() { delete mArgs; }

        int fnIndex() const { return mFnIndex; }
        NodeList* args() const { return mArgs; }

        virtual NodeType nodeType() const override { return ntFunction; }
        virtual bool prepare( QgsExpression* parent, const QgsExpressionContext* context ) override;
        virtual QVariant eval( QgsExpression* parent, const QgsExpressionContext* context ) override;
        virtual QString dump() const override;

        virtual QStringList referencedColumns() const override;
        virtual bool needsGeometry() const override { bool needs = Functions()[mFnIndex]->usesgeometry(); if ( mArgs ) { Q_FOREACH ( Node* n, mArgs->list() ) needs |= n->needsGeometry(); } return needs; }
        virtual void accept( Visitor& v ) const override { v.visit( *this ); }
        virtual Node* clone() const override;

        //! Tests whether the provided argument list is valid for the matching function
        static bool validateParams( int fnIndex, NodeList* args, QString& error )
        {
          if ( !args || !args->hasNamedNodes() )
            return true;

          const ParameterList& functionParams = Functions()[fnIndex]->parameters();
          if ( functionParams.isEmpty() )
          {
            error = QString( "%1 does not supported named parameters" ).arg( Functions()[fnIndex]->name() );
            return false;
          }
          else
          {
            QSet< int > providedArgs;
            QSet< int > handledArgs;
            int idx = 0;
            //first loop through unnamed arguments
            while ( args->names().at( idx ).isEmpty() )
            {
              providedArgs << idx;
              handledArgs << idx;
              idx++;
            }

            //next check named parameters
            for ( ; idx < functionParams.count(); ++idx )
            {
              int nodeIdx = args->names().indexOf( functionParams.at( idx ).name().toLower() );
              if ( nodeIdx < 0 )
              {
                if ( !functionParams.at( idx ).optional() )
                {
                  error = QString( "No value specified for parameter '%1' for %2" ).arg( functionParams.at( idx ).name(), Functions()[fnIndex]->name() );
                  return false;
                }
              }
              else
              {
                if ( providedArgs.contains( idx ) )
                {
                  error = QString( "Duplicate parameter specified for '%1' for %2" ).arg( functionParams.at( idx ).name(), Functions()[fnIndex]->name() );
                  return false;
                }
              }
              providedArgs << idx;
              handledArgs << nodeIdx;
            }

            //last check for bad names
            idx = 0;
            Q_FOREACH ( const QString& name, args->names() )
            {
              if ( !name.isEmpty() && !functionParams.contains( name ) )
              {
                error = QString( "Invalid parameter name '%1' for %2" ).arg( name, Functions()[fnIndex]->name() );
                return false;
              }
              if ( !name.isEmpty() && !handledArgs.contains( idx ) )
              {
                int functionIdx = functionParams.indexOf( name );
                if ( providedArgs.contains( functionIdx ) )
                {
                  error = QString( "Duplicate parameter specified for '%1' for %2" ).arg( functionParams.at( functionIdx ).name(), Functions()[fnIndex]->name() );
                  return false;
                }
              }
              idx++;
            }

          }
          return true;
        }

      protected:
        int mFnIndex;
        NodeList* mArgs;

    };

    /** \ingroup core
     */
    class CORE_EXPORT NodeLiteral : public Node
    {
      public:
        NodeLiteral( const QVariant& value )
            : mValue( value )
        {}

        /** The value of the literal. */
        inline QVariant value() const { return mValue; }

        virtual NodeType nodeType() const override { return ntLiteral; }
        virtual bool prepare( QgsExpression* parent, const QgsExpressionContext* context ) override;
        virtual QVariant eval( QgsExpression* parent, const QgsExpressionContext* context ) override;
        virtual QString dump() const override;

        virtual QStringList referencedColumns() const override { return QStringList(); }
        virtual bool needsGeometry() const override { return false; }
        virtual void accept( Visitor& v ) const override { v.visit( *this ); }
        virtual Node* clone() const override;

      protected:
        QVariant mValue;
    };

    /** \ingroup core
     */
    class CORE_EXPORT NodeColumnRef : public Node
    {
      public:
        NodeColumnRef( const QString& name )
            : mName( name )
            , mIndex( -1 )
        {}

        /** The name of the column. */
        QString name() const { return mName; }

        virtual NodeType nodeType() const override { return ntColumnRef; }
        virtual bool prepare( QgsExpression* parent, const QgsExpressionContext* context ) override;
        virtual QVariant eval( QgsExpression* parent, const QgsExpressionContext* context ) override;
        virtual QString dump() const override;

        virtual QStringList referencedColumns() const override { return QStringList( mName ); }
        virtual bool needsGeometry() const override { return false; }

        virtual void accept( Visitor& v ) const override { v.visit( *this ); }
        virtual Node* clone() const override;

      protected:
        QString mName;
        int mIndex;
    };

    /** \ingroup core
     */
    class CORE_EXPORT WhenThen
    {
      public:
        WhenThen( Node* whenExp, Node* thenExp )
            : mWhenExp( whenExp )
            , mThenExp( thenExp )
        {}
        ~WhenThen() { delete mWhenExp; delete mThenExp; }

        // protected:
        Node* mWhenExp;
        Node* mThenExp;

      private:
        WhenThen( const WhenThen& rh );
        WhenThen& operator=( const WhenThen& rh );
    };
    typedef QList<WhenThen*> WhenThenList;

    /** \ingroup core
     */
    class CORE_EXPORT NodeCondition : public Node
    {
      public:
        NodeCondition( WhenThenList* conditions, Node* elseExp = nullptr )
            : mConditions( *conditions )
            , mElseExp( elseExp )
        { delete conditions; }
        NodeCondition( const WhenThenList& conditions, Node* elseExp = nullptr )
            : mConditions( conditions )
            , mElseExp( elseExp )
        {}
        ~NodeCondition() { delete mElseExp; qDeleteAll( mConditions ); }

        virtual NodeType nodeType() const override { return ntCondition; }
        virtual QVariant eval( QgsExpression* parent, const QgsExpressionContext* context ) override;
        virtual bool prepare( QgsExpression* parent, const QgsExpressionContext* context ) override;
        virtual QString dump() const override;

        virtual QStringList referencedColumns() const override;
        virtual bool needsGeometry() const override;
        virtual void accept( Visitor& v ) const override { v.visit( *this ); }
        virtual Node* clone() const override;

      protected:
        WhenThenList mConditions;
        Node* mElseExp;
    };

    //////

    /** \ingroup core
     * Support for visitor pattern - algorithms dealing with the expressions
        may be implemented without modifying the Node classes */
    class CORE_EXPORT Visitor
    {
      public:
        virtual ~Visitor() {}
        virtual void visit( const NodeUnaryOperator& n ) = 0;
        virtual void visit( const NodeBinaryOperator& n ) = 0;
        virtual void visit( const NodeInOperator& n ) = 0;
        virtual void visit( const NodeFunction& n ) = 0;
        virtual void visit( const NodeLiteral& n ) = 0;
        virtual void visit( const NodeColumnRef& n ) = 0;
        virtual void visit( const NodeCondition& n ) = 0;
    };

    /** Entry function for the visitor pattern */
    void acceptVisitor( Visitor& v ) const;

    /** Returns the help text for a specified function.
     * @param name function name
     * @see variableHelpText()
     */
    static QString helptext( QString name );

    /** Returns the help text for a specified variable.
     * @param variableName name of variable
     * @param showValue set to true to include current value of variable in help text
     * @param value current value of variable to show in help text
     * @see helptext()
     * @note added in QGIS 2.12
     */
    static QString variableHelpText( const QString& variableName, bool showValue = true, const QVariant& value = QVariant() );

    /** Returns the translated name for a function group.
     * @param group untranslated group name
     */
    static QString group( const QString& group );

    /** Formats an expression result for friendly display to the user. Truncates the result to a sensible
     * length, and presents text representations of non numeric/text types (eg geometries and features).
     * @param value expression result to format
     * @returns formatted string, may contain HTML formatting characters
     * @note added in QGIS 2.14
     */
    static QString formatPreviewString( const QVariant& value );

  protected:
    /**
     * Used by QgsOgcUtils to create an empty
     */
    QgsExpression();

    void initGeomCalculator();

    static QMap<QString, QVariant> gmSpecialColumns;
    static QMap<QString, QString> gmSpecialColumnGroups;

    struct HelpArg
    {
      HelpArg( const QString& arg, const QString& desc, bool descOnly = false, bool syntaxOnly = false,
               bool optional = false, const QString& defaultVal = QString() )
          : mArg( arg )
          , mDescription( desc )
          , mDescOnly( descOnly )
          , mSyntaxOnly( syntaxOnly )
          , mOptional( optional )
          , mDefaultVal( defaultVal )
      {}

      QString mArg;
      QString mDescription;
      bool mDescOnly;
      bool mSyntaxOnly;
      bool mOptional;
      QString mDefaultVal;
    };

    struct HelpExample
    {
      HelpExample( const QString& expression, const QString& returns, const QString& note = QString::null )
          : mExpression( expression )
          , mReturns( returns )
          , mNote( note )
      {}

      QString mExpression;
      QString mReturns;
      QString mNote;
    };

    struct HelpVariant
    {
      HelpVariant( const QString& name, const QString& description,
                   const QList<HelpArg>& arguments = QList<HelpArg>(),
                   bool variableLenArguments = false,
                   const QList<HelpExample>& examples = QList<HelpExample>(),
                   const QString& notes = QString::null )
          : mName( name )
          , mDescription( description )
          , mArguments( arguments )
          , mVariableLenArguments( variableLenArguments )
          , mExamples( examples )
          , mNotes( notes )
      {}

      QString mName;
      QString mDescription;
      QList<HelpArg> mArguments;
      bool mVariableLenArguments;
      QList<HelpExample> mExamples;
      QString mNotes;
    };

    struct Help
    {
      Help() {}

      Help( const QString& name, const QString& type, const QString& description, const QList<HelpVariant>& variants )
          : mName( name )
          , mType( type )
          , mDescription( description )
          , mVariants( variants )
      {}

      QString mName;
      QString mType;
      QString mDescription;
      QList<HelpVariant> mVariants;
    };

    /**
     * Helper for implicit sharing. When called will create
     * a new deep copy of this expression.
     *
     * @note not available in Python bindings
     */
    void detach();

    QgsExpressionPrivate* d;

    static QHash<QString, Help> gFunctionHelpTexts;
    static QHash<QString, QString> gVariableHelpTexts;
    static QHash<QString, QString> gGroups;

    //! @note not available in Python bindings
    static void initFunctionHelp();
    //! @note not available in Python bindings
    static void initVariableHelp();

    friend class QgsOgcUtils;
};


Q_NOWARN_DEPRECATED_POP

Q_DECLARE_METATYPE( QgsExpression::Node* )

#endif // QGSEXPRESSION_H
