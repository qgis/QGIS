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

#include "qgis_core.h"
#include <QMetaType>
#include <QStringList>
#include <QVariant>
#include <QList>
#include <QDomDocument>
#include <QCoreApplication>
#include <QSet>
#include <functional>

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
when prepare() is called. For usage this means mainly, that you should
normally keep an unprepared master copy of a QgsExpression and whenever using it
with a particular QgsFeatureIterator copy it just before and prepare it using the
same context as the iterator.

Implicit sharing was added in 2.14

*/

class CORE_EXPORT QgsExpression
{
    Q_DECLARE_TR_FUNCTIONS( QgsExpression )
  public:

    /**
     * Creates a new expression based on the provided string.
     * The string will immediately be parsed. For optimization
     * prepare() should always be called before every
     * loop in which this expression is used.
     */
    QgsExpression( const QString &expr );

    /**
     * Create a copy of this expression. This is preferred
     * over recreating an expression from a string since
     * it does not need to be re-parsed.
     */
    QgsExpression( const QgsExpression &other );

    /**
     * Create a copy of this expression. This is preferred
     * over recreating an expression from a string since
     * it does not need to be re-parsed.
     */
    QgsExpression &operator=( const QgsExpression &other );

    /**
     * Create an empty expression.
     *
     * \since QGIS 3.0
     */
    QgsExpression();

    ~QgsExpression();

    /**
     * Compares two expressions. The operator returns true
     * if the expression string is equal.
     *
     * \since QGIS 3.0
     */
    bool operator==( const QgsExpression &other ) const;

    /**
     * Checks if this expression is valid.
     * A valid expression could be parsed but does not necessarily evaluate properly.
     *
     * \since QGIS 3.0
     */
    bool isValid() const;

    //! Returns true if an error occurred when parsing the input expression
    bool hasParserError() const;
    //! Returns parser error
    QString parserErrorString() const;

    class Node;

    //! Returns root node of the expression. Root node is null is parsing has failed
    const QgsExpression::Node *rootNode() const;

    /** Get the expression ready for evaluation - find out column indexes.
     * \param context context for preparing expression
     * \since QGIS 2.12
     */
    bool prepare( const QgsExpressionContext *context );

    /**
     * Get list of columns referenced by the expression.
     *
     * \note If the returned list contains the QgsFeatureRequest::AllAttributes constant then
     * all attributes from the layer are required for evaluation of the expression.
     * QgsFeatureRequest::setSubsetOfAttributes automatically handles this case.
     *
     * \see referencedAttributeIndexes()
     */
    QSet<QString> referencedColumns() const;

    /**
     * Return a list of all variables which are used in this expression.
     * If the list contains a NULL QString, there is a variable name used
     * which is determined at runtime.
     *
     * \since QGIS 3.0
     */
    QSet<QString> referencedVariables() const;

    /**
     * Return a list of field name indexes obtained from the provided fields.
     *
     * \since QGIS 3.0
     */
    QSet<int> referencedAttributeIndexes( const QgsFields &fields ) const;

    //! Returns true if the expression uses feature geometry for some computation
    bool needsGeometry() const;

    // evaluation

    /** Evaluate the feature and return the result.
     * \note this method does not expect that prepare() has been called on this instance
     * \since QGIS 2.12
     */
    QVariant evaluate();

    /** Evaluate the expression against the specified context and return the result.
     * \param context context for evaluating expression
     * \note prepare() should be called before calling this method.
     * \since QGIS 2.12
     */
    QVariant evaluate( const QgsExpressionContext *context );

    //! Returns true if an error occurred when evaluating last input
    bool hasEvalError() const;
    //! Returns evaluation error
    QString evalErrorString() const;
    //! Set evaluation error (used internally by evaluation functions)
    void setEvalErrorString( const QString &str );

    /** Checks whether an expression consists only of a single field reference
     * \since QGIS 2.9
     */
    bool isField() const { return rootNode() && dynamic_cast<const NodeColumnRef *>( rootNode() ) ;}

    /** Tests whether a string is a valid expression.
     * \param text string to test
     * \param context optional expression context
     * \param errorMessage will be filled with any error message from the validation
     * \returns true if string is a valid expression
     * \since QGIS 2.12
     */
    static bool checkExpression( const QString &text, const QgsExpressionContext *context, QString &errorMessage SIP_OUT );

    /**
     * Set the expression string, will reset the whole internal structure.
     *
     * \since QGIS 3.0
     */
    void setExpression( const QString &expression );

    //! Return the original, unmodified expression string.
    //! If there was none supplied because it was constructed by sole
    //! API calls, dump() will be used to create one instead.
    QString expression() const;

    //! Return an expression string, constructed from the internal
    //! abstract syntax tree. This does not contain any nice whitespace
    //! formatting or comments. In general it is preferable to use
    //! expression() instead.
    QString dump() const;

    /** Return calculator used for distance and area calculations
     * (used by $length, $area and $perimeter functions only)
     * \see setGeomCalculator()
     * \see distanceUnits()
     * \see areaUnits()
     */
    QgsDistanceArea *geomCalculator();

    /** Sets the geometry calculator used for distance and area calculations in expressions.
     * (used by $length, $area and $perimeter functions only). By default, no geometry
     * calculator is set and all distance and area calculations are performed using simple
     * cartesian methods (ie no ellipsoidal calculations).
     * \param calc geometry calculator. Ownership is not transferred. Set to a nullptr to force
     * cartesian calculations.
     * \see geomCalculator()
     */
    void setGeomCalculator( const QgsDistanceArea *calc );

    /** Returns the desired distance units for calculations involving geomCalculator(), e.g., "$length" and "$perimeter".
     * \note distances are only converted when a geomCalculator() has been set
     * \since QGIS 2.14
     * \see setDistanceUnits()
     * \see areaUnits()
     */
    QgsUnitTypes::DistanceUnit distanceUnits() const;

    /** Sets the desired distance units for calculations involving geomCalculator(), e.g., "$length" and "$perimeter".
     * \note distances are only converted when a geomCalculator() has been set
     * \since QGIS 2.14
     * \see distanceUnits()
     * \see setAreaUnits()
     */
    void setDistanceUnits( QgsUnitTypes::DistanceUnit unit );

    /** Returns the desired areal units for calculations involving geomCalculator(), e.g., "$area".
     * \note areas are only converted when a geomCalculator() has been set
     * \since QGIS 2.14
     * \see setAreaUnits()
     * \see distanceUnits()
     */
    QgsUnitTypes::AreaUnit areaUnits() const;

    /** Sets the desired areal units for calculations involving geomCalculator(), e.g., "$area".
     * \note areas are only converted when a geomCalculator() has been set
     * \since QGIS 2.14
     * \see areaUnits()
     * \see setDistanceUnits()
     */
    void setAreaUnits( QgsUnitTypes::AreaUnit unit );

    /** This function replaces each expression between [% and %]
     * in the string with the result of its evaluation with the specified context
     *
     * Additional substitutions can be passed through the substitutionMap parameter
     * \param action The source string in which placeholders should be replaced.
     * \param context Expression context
     * \param distanceArea Optional QgsDistanceArea. If specified, the QgsDistanceArea is used for distance
     * and area conversion
     * \since QGIS 2.12
     */
    static QString replaceExpressionText( const QString &action, const QgsExpressionContext *context,
                                          const QgsDistanceArea *distanceArea = nullptr );

    /** Attempts to evaluate a text string as an expression to a resultant double
     * value.
     * \param text text to evaluate as expression
     * \param fallbackValue value to return if text can not be evaluated as a double
     * \returns evaluated double value, or fallback value
     * \since QGIS 2.7
     * \note this method is inefficient for bulk evaluation of expressions, it is intended
     * for one-off evaluations only.
     */
    static double evaluateToDouble( const QString &text, const double fallbackValue );

    /**
     * \brief list of unary operators
     * \note if any change is made here, the definition of QgsExpression::UnaryOperatorText[] must be adapted.
     */
    enum UnaryOperator
    {
      uoNot,
      uoMinus,
    };

    /**
     * \brief list of binary operators
     * \note if any change is made here, the definition of QgsExpression::BinaryOperatorText[] must be adapted.
     */
    enum BinaryOperator
    {
      // logical
      boOr,
      boAnd,

      // comparison
      boEQ,  //!< =
      boNE,  //!< <>
      boLE,  //!< <=
      boGE,  //!< >=
      boLT,  //!< <
      boGT,  //!< >
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

    //! \note not available in Python bindings
    static const char *BINARY_OPERATOR_TEXT[] SIP_SKIP;

    //! \note not available in Python bindings
    static const char *UNARY_OPERATOR_TEXT[] SIP_SKIP;

    /** \ingroup core
      * Represents a single parameter passed to a function.
      * \since QGIS 2.16
      */
    class CORE_EXPORT Parameter
    {
      public:

        /** Constructor for Parameter.
         * \param name parameter name, used when named parameter are specified in an expression
         * \param optional set to true if parameter should be optional
         * \param defaultValue default value to use for optional parameters
         */
        Parameter( const QString &name,
                   bool optional = false,
                   const QVariant &defaultValue = QVariant() )
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

        bool operator==( const QgsExpression::Parameter &other ) const
        {
          return ( QString::compare( mName, other.mName, Qt::CaseInsensitive ) == 0 );
        }

      private:
        QString mName;
        bool mOptional;
        QVariant mDefaultValue;
    };

    //! List of parameters, used for function definition
    typedef QList< QgsExpression::Parameter > ParameterList;

    /** Function definition for evaluation against an expression context, using a list of values as parameters to the function.
     */
    typedef QVariant( *FcnEval )( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent ) SIP_SKIP;

    class NodeFunction;

    /** \ingroup core
      * A abstract base class for defining QgsExpression functions.
      */
    class CORE_EXPORT Function
    {
      public:

        //! Constructor for function which uses unnamed parameters
        Function( const QString &fnname,
                  int params,
                  const QString &group,
                  const QString &helpText = QString(),
                  bool lazyEval = false,
                  bool handlesNull = false,
                  bool isContextual = false )
          : mName( fnname )
          , mParams( params )
          , mGroups( group.isEmpty() ? QStringList() : QStringList() << group )
          , mHelpText( helpText )
          , mLazyEval( lazyEval )
          , mHandlesNull( handlesNull )
          , mIsContextual( isContextual )
        {
        }

        /** Constructor for function which uses unnamed parameters and group list
         * \since QGIS 3.0
         */
        Function( const QString &fnname,
                  int params,
                  const QStringList &groups,
                  const QString &helpText = QString(),
                  bool lazyEval = false,
                  bool handlesNull = false,
                  bool isContextual = false )
          : mName( fnname )
          , mParams( params )
          , mGroups( groups )
          , mHelpText( helpText )
          , mLazyEval( lazyEval )
          , mHandlesNull( handlesNull )
          , mIsContextual( isContextual )
        {
        }

        /** Constructor for function which uses named parameter list.
         * \since QGIS 2.16
         */
        Function( const QString &fnname,
                  const QgsExpression::ParameterList &params,
                  const QString &group,
                  const QString &helpText = QString(),
                  bool lazyEval = false,
                  bool handlesNull = false,
                  bool isContextual = false )
          : mName( fnname )
          , mParams( 0 )
          , mParameterList( params )
          , mGroups( group.isEmpty() ? QStringList() : QStringList() << group )
          , mHelpText( helpText )
          , mLazyEval( lazyEval )
          , mHandlesNull( handlesNull )
          , mIsContextual( isContextual )
        {}

        /** Constructor for function which uses named parameter list and group list.
         * \since QGIS 3.0
         */
        Function( const QString &fnname,
                  const QgsExpression::ParameterList &params,
                  const QStringList &groups,
                  const QString &helpText = QString(),
                  bool lazyEval = false,
                  bool handlesNull = false,
                  bool isContextual = false )
          : mName( fnname )
          , mParams( 0 )
          , mParameterList( params )
          , mGroups( groups )
          , mHelpText( helpText )
          , mLazyEval( lazyEval )
          , mHandlesNull( handlesNull )
          , mIsContextual( isContextual )
        {}

        virtual ~Function() = default;

        //! The name of the function.
        QString name() const { return mName; }

        //! The number of parameters this function takes.
        int params() const { return mParameterList.isEmpty() ? mParams : mParameterList.count(); }

        //! The minimum number of parameters this function takes.
        int minParams() const
        {
          if ( mParameterList.isEmpty() )
            return mParams;

          int min = 0;
          Q_FOREACH ( const Parameter &param, mParameterList )
          {
            if ( !param.optional() )
              min++;
          }
          return min;
        }

        /** Returns the list of named parameters for the function, if set.
         * \since QGIS 2.16
        */
        const QgsExpression::ParameterList &parameters() const { return mParameterList; }

        //! Does this function use a geometry object.
        virtual bool usesGeometry( const QgsExpression::NodeFunction *node ) const;

        /**
         * Returns a list of possible aliases for the function. These include
         * other permissible names for the function, e.g., deprecated names.
         * \returns list of known aliases
         * \since QGIS 2.9
         */
        virtual QStringList aliases() const { return QStringList(); }

        /**
         * True if this function should use lazy evaluation.  Lazy evaluation functions take QgsExpression::Node objects
         * rather than the node results when called.  You can use node->eval(parent, feature) to evaluate the node and return the result
         * Functions are non lazy default and will be given the node return value when called.
         */
        bool lazyEval() const { return mLazyEval; }

        /**
         * Will be called during prepare to determine if the function is static.
         * A function is static if it will return the same value for every feature with different
         * attributes and/or geometry.
         *
         * By default this will return true, if all arguments that have been passed to the function
         * are also static.
         *
         * \since QGIS 3.0
         */
        virtual bool isStatic( const QgsExpression::NodeFunction *node, QgsExpression *parent, const QgsExpressionContext *context ) const;

        /**
         * This will be called during the prepare step() of an expression if it is not static.
         *
         * This can be used by functions to do any preparation steps that might help to speedup the upcoming
         * evaluation.
         *
         * \since QGIS 3.0
         */
        virtual bool prepare( const QgsExpression::NodeFunction *node, QgsExpression *parent, const QgsExpressionContext *context ) const;

        /**
         * Returns a set of field names which are required for this function.
         * May contain QgsFeatureRequest::AllAttributes to signal that all
         * attributes are required.
         * If in doubt this will return more fields than strictly required.
         *
         * \since QGIS 3.0
         */
        virtual QSet<QString> referencedColumns( const QgsExpression::NodeFunction *node ) const;

        /** Returns whether the function is only available if provided by a QgsExpressionContext object.
         * \since QGIS 2.12
         */
        bool isContextual() const { return mIsContextual; }

        /** Returns true if the function is deprecated and should not be presented as a valid option
         * to users in expression builders.
         * \since QGIS 3.0
         */
        virtual bool isDeprecated() const { return mGroups.isEmpty() ? false : mGroups.contains( QStringLiteral( "deprecated" ) ); }

        /** Returns the first group which the function belongs to.
         * \note consider using groups() instead, as some functions naturally belong in multiple groups
        */
        QString group() const { return mGroups.isEmpty() ? QString() : mGroups.at( 0 ); }

        /** Returns a list of the groups the function belongs to.
         * \since QGIS 3.0
         * \see group()
        */
        QStringList groups() const { return mGroups; }

        //! The help text for the function.
        const QString helpText() const { return mHelpText.isEmpty() ? QgsExpression::helpText( mName ) : mHelpText; }

        /** Returns result of evaluating the function.
         * \param values list of values passed to the function
         * \param context context expression is being evaluated against
         * \param parent parent expression
         * \returns result of function
         */
        virtual QVariant func( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent ) = 0;

        bool operator==( const QgsExpression::Function &other ) const;

        virtual bool handlesNull() const { return mHandlesNull; }

      protected:

        /**
         * This will return true if all the params for the provided function \a node are static within the
         * constraints imposed by the \a context within the given \a parent.
         *
         * This can be used as callback for custom implementations of subclasses. It is the default for implementation
         * for StaticFunction::isStatic.
         *
         * \note Added in QGIS 3.0
         */
        static bool allParamsStatic( const QgsExpression::NodeFunction *node, QgsExpression *parent, const QgsExpressionContext *context );

      private:
        QString mName;
        int mParams;
        QgsExpression::ParameterList mParameterList;
        QStringList mGroups;
        QString mHelpText;
        bool mLazyEval;
        bool mHandlesNull;
        bool mIsContextual; //if true function is only available through an expression context
    };

    /** \ingroup core
      * c++ helper class for defining QgsExpression functions.
      * \note not available in Python bindings
      */
#ifndef SIP_RUN
    class StaticFunction : public QgsExpression::Function
    {
      public:

        /** Static function for evaluation against a QgsExpressionContext, using an unnamed list of parameter values.
         */
        StaticFunction( const QString &fnname,
                        int params,
                        FcnEval fcn,
                        const QString &group,
                        const QString &helpText = QString(),
                        bool usesGeometry = false,
                        const QSet<QString> &referencedColumns = QSet<QString>(),
                        bool lazyEval = false,
                        const QStringList &aliases = QStringList(),
                        bool handlesNull = false )
          : Function( fnname, params, group, helpText, lazyEval, handlesNull )
          , mFnc( fcn )
          , mAliases( aliases )
          , mUsesGeometry( usesGeometry )
          , mReferencedColumns( referencedColumns )
        {
        }

        /** Static function for evaluation against a QgsExpressionContext, using a named list of parameter values.
         */
        StaticFunction( const QString &fnname,
                        const QgsExpression::ParameterList &params,
                        FcnEval fcn,
                        const QString &group,
                        const QString &helpText = QString(),
                        bool usesGeometry = false,
                        const QSet<QString> &referencedColumns = QSet<QString>(),
                        bool lazyEval = false,
                        const QStringList &aliases = QStringList(),
                        bool handlesNull = false )
          : Function( fnname, params, group, helpText, lazyEval, handlesNull )
          , mFnc( fcn )
          , mAliases( aliases )
          , mUsesGeometry( usesGeometry )
          , mReferencedColumns( referencedColumns )
        {}

        /**
         * Static function for evaluation against a QgsExpressionContext, using a named list of parameter values.
         *
         * Lambda functions can be provided that will be called to determine if a geometry is used an which
         * columns are referenced.
         * This is only required if this cannot be determined by calling each parameter node's usesGeometry() or
         * referencedColumns() method. For example, an aggregate expression requires the geometry and all columns
         * if the parent variable is used.
         * If a nullptr is passed as a node to these functions, they should stay on the safe side and return if they
         * could potentially require a geometry or columns.
         */
        StaticFunction( const QString &fnname,
                        const QgsExpression::ParameterList &params,
                        FcnEval fcn,
                        const QString &group,
                        const QString &helpText,
                        std::function < bool ( const QgsExpression::NodeFunction *node ) > usesGeometry,
                        std::function < QSet<QString>( const QgsExpression::NodeFunction *node ) > referencedColumns,
                        bool lazyEval = false,
                        const QStringList &aliases = QStringList(),
                        bool handlesNull = false );


        /** Static function for evaluation against a QgsExpressionContext, using a named list of parameter values and list
         * of groups.
         */
        StaticFunction( const QString &fnname,
                        const QgsExpression::ParameterList &params,
                        FcnEval fcn,
                        const QStringList &groups,
                        const QString &helpText = QString(),
                        bool usesGeometry = false,
                        const QSet<QString> &referencedColumns = QSet<QString>(),
                        bool lazyEval = false,
                        const QStringList &aliases = QStringList(),
                        bool handlesNull = false )
          : Function( fnname, params, groups, helpText, lazyEval, handlesNull )
          , mFnc( fcn )
          , mAliases( aliases )
          , mUsesGeometry( usesGeometry )
          , mReferencedColumns( referencedColumns )
        {}

        /** Returns result of evaluating the function.
         * \param values list of values passed to the function
         * \param context context expression is being evaluated against
         * \param parent parent expression
         * \returns result of function
         */
        virtual QVariant func( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent ) override
        {
          return mFnc ? mFnc( values, context, parent ) : QVariant();
        }

        virtual QStringList aliases() const override { return mAliases; }

        virtual bool usesGeometry( const QgsExpression::NodeFunction *node ) const override;

        virtual QSet<QString> referencedColumns( const QgsExpression::NodeFunction *node ) const override;

        virtual bool isStatic( const QgsExpression::NodeFunction *node, QgsExpression *parent, const QgsExpressionContext *context ) const override;

        virtual bool prepare( const QgsExpression::NodeFunction *node, QgsExpression *parent, const QgsExpressionContext *context ) const override;

        /**
         * Set a function that will be called in the prepare step to determine if the function is
         * static or not.
         * By default this is set to a function that checks all arguments that have been passed to the variable
         * and if all of them are static, it will be assumed that the function is static as well.
         */
        void setIsStaticFunction( std::function < bool( const NodeFunction *node, QgsExpression *parent, const QgsExpressionContext *context ) > isStatic );

        /**
         * Tag this function as either static or not static.
         * This will indicate that the function is always expected to return the same value for
         * an iteration (or explicitly request that it's going to be called for every feature, if false).
         *
         * \see setIsStaticFunction
         */
        void setIsStatic( bool isStatic );

        /**
         * Set a function that will be called in the prepare step to determine if the function is
         * static or not.
         * By default this is set to a function that checks all arguments that have been passed to the variable
         * and if all of them are static, it will be assumed that the function is static as well.
         */
        void setPrepareFunction( std::function < bool( const NodeFunction *node, QgsExpression *parent, const QgsExpressionContext *context ) > prepareFunc );


      private:
        FcnEval mFnc;
        QStringList mAliases;
        bool mUsesGeometry;
        std::function < bool( const QgsExpression::NodeFunction *node ) > mUsesGeometryFunc;
        std::function < QSet<QString>( const QgsExpression::NodeFunction *node ) > mReferencedColumnsFunc;
        std::function < bool( const NodeFunction *node,  QgsExpression *parent, const QgsExpressionContext *context ) > mIsStaticFunc = allParamsStatic;
        std::function < bool( const NodeFunction *node,  QgsExpression *parent, const QgsExpressionContext *context ) > mPrepareFunc;
        QSet<QString> mReferencedColumns;
        bool mIsStatic = false;
    };
#endif

    //! \note not available in Python bindings
    static QList<QgsExpression::Function *> sFunctions SIP_SKIP;
    static const QList<QgsExpression::Function *> &Functions();

    //! \note not available in Python bindings
    static QStringList sBuiltinFunctions SIP_SKIP;
    static const QStringList &BuiltinFunctions();

    /** Registers a function to the expression engine. This is required to allow expressions to utilize the function.
     * \param function function to register
     * \param transferOwnership set to true to transfer ownership of function to expression engine
     * \returns true on successful registration
     * \see unregisterFunction
     */
    static bool registerFunction( QgsExpression::Function *function, bool transferOwnership = false );

    /** Unregisters a function from the expression engine. The function will no longer be usable in expressions.
     * \param name function name
     * \see registerFunction
     */
    static bool unregisterFunction( const QString &name );

    //! List of functions owned by the expression engine
    //! \note not available in Python bindings
    static QList<QgsExpression::Function *> sOwnedFunctions SIP_SKIP;

    /** Deletes all registered functions whose ownership have been transferred to the expression engine.
     * \since QGIS 2.12
     */
    static void cleanRegisteredFunctions();

    //! tells whether the identifier is a name of existing function
    static bool isFunctionName( const QString &name );

    //! return index of the function in Functions array
    static int functionIndex( const QString &name );

    /** Returns the number of functions defined in the parser
     *  \returns The number of function defined in the parser.
     */
    static int functionCount();

    /** Returns a quoted column reference (in double quotes)
     * \see quotedString()
     * \see quotedValue()
     */
    static QString quotedColumnRef( QString name );

    /** Returns a quoted version of a string (in single quotes)
     * \see quotedValue()
     * \see quotedColumnRef()
     */
    static QString quotedString( QString text );

    /** Returns a string representation of a literal value, including appropriate
     * quotations where required.
     * \param value value to convert to a string representation
     * \since QGIS 2.14
     * \see quotedString()
     * \see quotedColumnRef()
     */
    static QString quotedValue( const QVariant &value );

    /** Returns a string representation of a literal value, including appropriate
     * quotations where required.
     * \param value value to convert to a string representation
     * \param type value type
     * \since QGIS 2.14
     * \see quotedString()
     * \see quotedColumnRef()
     */
    static QString quotedValue( const QVariant &value, QVariant::Type type );

    //////

    enum NodeType
    {
      ntUnaryOperator, //!< \see QgsExpression::Node::NodeUnaryOperator
      ntBinaryOperator, //!< \see QgsExpression::Node::NodeBinaryOperator
      ntInOperator, //!< \see QgsExpression::Node::NodeInOperator
      ntFunction,  //!< \see QgsExpression::Node::NodeFunction
      ntLiteral, //!< \see QgsExpression::Node::NodeLiteral
      ntColumnRef, //!< \see QgsExpression::Node::NodeColumnRef
      ntCondition //!< \see QgsExpression::Node::NodeCondition
    };

    /**
     * \ingroup core
     *
     * Abstract base class for all nodes that can appear in an expression.
     */
    class CORE_EXPORT Node
    {

#ifdef SIP_RUN
        SIP_CONVERT_TO_SUBCLASS_CODE
        switch ( sipCpp->nodeType() )
        {
          case QgsExpression::ntUnaryOperator:
            sipType = sipType_QgsExpression_NodeUnaryOperator;
            break;
          case QgsExpression::ntBinaryOperator:
            sipType = sipType_QgsExpression_NodeBinaryOperator;
            break;
          case QgsExpression::ntInOperator:
            sipType = sipType_QgsExpression_NodeInOperator;
            break;
          case QgsExpression::ntFunction:
            sipType = sipType_QgsExpression_NodeFunction;
            break;
          case QgsExpression::ntLiteral:
            sipType = sipType_QgsExpression_NodeLiteral;
            break;
          case QgsExpression::ntColumnRef:
            sipType = sipType_QgsExpression_NodeColumnRef;
            break;
          case QgsExpression::ntCondition:
            sipType = sipType_QgsExpression_NodeCondition;
            break;
          default:
            sipType = 0;
            break;
        }
        SIP_END
#endif

      public:
        virtual ~Node() = default;

        /**
         * Get the type of this node.
         *
         * \returns The type of this node
         */
        virtual QgsExpression::NodeType nodeType() const = 0;

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
        virtual QgsExpression::Node *clone() const = 0;

        /**
         * Abstract virtual method which returns a list of columns required to
         * evaluate this node.
         *
         * When reimplementing this, you need to return any column that is required to
         * evaluate this node and in addition recursively collect all the columns required
         * to evaluate child nodes.
         *
         * \returns A list of columns required to evaluate this expression
         */
        virtual QSet<QString> referencedColumns() const = 0;

        /**
         * Return a set of all variables which are used in this expression.
         */
        virtual QSet<QString> referencedVariables() const = 0;

        /**
         * Abstract virtual method which returns if the geometry is required to evaluate
         * this expression.
         *
         * This needs to call `needsGeometry()` recursively on any child nodes.
         *
         * \returns true if a geometry is required to evaluate this expression
         */
        virtual bool needsGeometry() const = 0;

        /**
         * Returns true if this node can be evaluated for a static value. This is used during
         * the prepare() step and in case it returns true, the value of this node will already
         * be evaluated and the result cached (and therefore not re-evaluated in subsequent calls
         * to eval()). In case this returns true, prepareNode() will never be called.
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


      protected:

        /**
         * Needs to be called by all subclasses as part of their clone() implementation.
         *
         * \note Not available in python bindings
         * \since QGIS 3.0
         */
        void cloneTo( QgsExpression::Node *target ) const;

      private:

        /**
         * Abstract virtual preparation method
         * Errors are reported to the parent
         * \since QGIS 3.0
         */
        virtual bool prepareNode( QgsExpression *parent, const QgsExpressionContext *context ) = 0 SIP_FORCE;

        /**
         * Abstract virtual eval method
         * Errors are reported to the parent
         * \since QGIS 3.0
         */
        virtual QVariant evalNode( QgsExpression *parent, const QgsExpressionContext *context ) = 0 SIP_FORCE;

        bool mHasCachedValue = false;
        QVariant mCachedStaticValue;
    };

    //! Named node
    //! \since QGIS 2.16
    //! \ingroup core
    class CORE_EXPORT NamedNode
    {
      public:

        /** Constructor for NamedNode
         * \param name node name
         * \param node node
         */
        NamedNode( const QString &name, QgsExpression::Node *node )
          : name( name )
          , node( node )
        {}

        //! Node name
        QString name;

        //! Node
        QgsExpression::Node *node = nullptr;
    };

    /** \ingroup core
     */
    class CORE_EXPORT NodeList
    {
      public:
        NodeList() : mHasNamedNodes( false ) {}
        virtual ~NodeList() { qDeleteAll( mList ); }
        //! Takes ownership of the provided node
        void append( QgsExpression::Node *node SIP_TRANSFER ) { mList.append( node ); mNameList.append( QString() ); }

        /** Adds a named node. Takes ownership of the provided node.
         * \since QGIS 2.16
        */
        void append( QgsExpression::NamedNode *node SIP_TRANSFER );

        /** Returns the number of nodes in the list.
         */
        int count() const { return mList.count(); }

        //! Returns true if list contains any named nodes
        //! \since QGIS 2.16
        bool hasNamedNodes() const { return mHasNamedNodes; }

        /**
         * Get a list of all the nodes.
         */
        QList<QgsExpression::Node *> list() { return mList; }

        /**
         * Get the node at position i in the list.
         *
         * \since QGIS 3.0
         */
        QgsExpression::Node *at( int i ) { return mList.at( i ); }

        //! Returns a list of names for nodes. Unnamed nodes will be indicated by an empty string in the list.
        //! \since QGIS 2.16
        QStringList names() const { return mNameList; }

        //! Creates a deep copy of this list. Ownership is transferred to the caller
        QgsExpression::NodeList *clone() const;

        virtual QString dump() const;

      private:
        QList<Node *> mList;
        QStringList mNameList;

        bool mHasNamedNodes;
    };

    /** \ingroup core
     * A unary node is either negative as in boolean (not) or as in numbers (minus).
     */
    class CORE_EXPORT NodeUnaryOperator : public QgsExpression::Node
    {
      public:

        /**
         * A node unary operator is modifying the value of \a operand by negating it with \a op.
         */
        NodeUnaryOperator( QgsExpression::UnaryOperator op, QgsExpression::Node *operand SIP_TRANSFER )
          : mOp( op )
          , mOperand( operand )
        {}
        ~NodeUnaryOperator() { delete mOperand; }

        QgsExpression::UnaryOperator op() const { return mOp; }
        QgsExpression::Node *operand() const { return mOperand; }

        virtual QgsExpression::NodeType nodeType() const override { return ntUnaryOperator; }
        virtual bool prepareNode( QgsExpression *parent, const QgsExpressionContext *context ) override;
        virtual QVariant evalNode( QgsExpression *parent, const QgsExpressionContext *context ) override;
        virtual QString dump() const override;

        virtual QSet<QString> referencedColumns() const override;
        virtual QSet<QString> referencedVariables() const override;
        virtual bool needsGeometry() const override { return mOperand->needsGeometry(); }
        virtual QgsExpression::Node *clone() const override;

        virtual bool isStatic( QgsExpression *parent, const QgsExpressionContext *context ) const override;

      private:
        UnaryOperator mOp;
        Node *mOperand = nullptr;
    };

    /** \ingroup core
     */
    class CORE_EXPORT NodeBinaryOperator : public QgsExpression::Node
    {
      public:

        /**
         * Binary combination of the left and the right with op.
         */
        NodeBinaryOperator( QgsExpression::BinaryOperator op, QgsExpression::Node *opLeft SIP_TRANSFER, QgsExpression::Node *opRight SIP_TRANSFER )
          : mOp( op )
          , mOpLeft( opLeft )
          , mOpRight( opRight )
        {}
        ~NodeBinaryOperator() { delete mOpLeft; delete mOpRight; }

        QgsExpression::BinaryOperator op() const { return mOp; }
        QgsExpression::Node *opLeft() const { return mOpLeft; }
        QgsExpression::Node *opRight() const { return mOpRight; }

        virtual QgsExpression::NodeType nodeType() const override { return ntBinaryOperator; }
        virtual bool prepareNode( QgsExpression *parent, const QgsExpressionContext *context ) override;
        virtual QVariant evalNode( QgsExpression *parent, const QgsExpressionContext *context ) override;
        virtual QString dump() const override;

        virtual QSet<QString> referencedColumns() const override;
        virtual QSet<QString> referencedVariables() const override;
        virtual bool needsGeometry() const override;
        virtual QgsExpression::Node *clone() const override;
        virtual bool isStatic( QgsExpression *parent, const QgsExpressionContext *context ) const override;

        int precedence() const;
        bool leftAssociative() const;

      private:
        bool compare( double diff );
        qlonglong computeInt( qlonglong x, qlonglong y );
        double computeDouble( double x, double y );

        /** Computes the result date time calculation from a start datetime and an interval
         * \param d start datetime
         * \param i interval to add or subtract (depending on mOp)
         */
        QDateTime computeDateTimeFromInterval( const QDateTime &d, QgsInterval *i );

        BinaryOperator mOp;
        Node *mOpLeft = nullptr;
        Node *mOpRight = nullptr;
    };

    /** \ingroup core
     */
    class CORE_EXPORT NodeInOperator : public QgsExpression::Node
    {
      public:

        /**
         * This node tests if the result of \a node is in the result of \a list. Optionally it can be inverted with \a notin which by default is false.
         */
        NodeInOperator( QgsExpression::Node *node SIP_TRANSFER, QgsExpression::NodeList *list SIP_TRANSFER, bool notin = false )
          : mNode( node )
          , mList( list )
          , mNotIn( notin )
        {}
        virtual ~NodeInOperator() { delete mNode; delete mList; }

        QgsExpression::Node *node() const { return mNode; }
        bool isNotIn() const { return mNotIn; }
        QgsExpression::NodeList *list() const { return mList; }

        virtual QgsExpression::NodeType nodeType() const override { return ntInOperator; }
        virtual bool prepareNode( QgsExpression *parent, const QgsExpressionContext *context ) override;
        virtual QVariant evalNode( QgsExpression *parent, const QgsExpressionContext *context ) override;
        virtual QString dump() const override;

        virtual QSet<QString> referencedColumns() const override;
        virtual QSet<QString> referencedVariables() const override;
        virtual bool needsGeometry() const override;
        virtual QgsExpression::Node *clone() const override;
        virtual bool isStatic( QgsExpression *parent, const QgsExpressionContext *context ) const override;

      private:
        Node *mNode = nullptr;
        NodeList *mList = nullptr;
        bool mNotIn;
    };

    /** \ingroup core
     */
    class CORE_EXPORT NodeFunction : public QgsExpression::Node
    {
      public:

        /**
         * A function node consists of an index of the function in the global function array and
         * a list of arguments that will be passed to it.
         */
        NodeFunction( int fnIndex, QgsExpression::NodeList *args SIP_TRANSFER );

        virtual ~NodeFunction() { delete mArgs; }

        int fnIndex() const { return mFnIndex; }
        QgsExpression::NodeList *args() const { return mArgs; }

        virtual QgsExpression::NodeType nodeType() const override { return ntFunction; }
        virtual bool prepareNode( QgsExpression *parent, const QgsExpressionContext *context ) override;
        virtual QVariant evalNode( QgsExpression *parent, const QgsExpressionContext *context ) override;
        virtual QString dump() const override;

        virtual QSet<QString> referencedColumns() const override;
        virtual QSet<QString> referencedVariables() const override;
        virtual bool needsGeometry() const override;
        virtual QgsExpression::Node *clone() const override;
        virtual bool isStatic( QgsExpression *parent, const QgsExpressionContext *context ) const override;

        //! Tests whether the provided argument list is valid for the matching function
        static bool validateParams( int fnIndex, QgsExpression::NodeList *args, QString &error );

      private:
        int mFnIndex;
        NodeList *mArgs = nullptr;

    };

    /** \ingroup core
     */
    class CORE_EXPORT NodeLiteral : public QgsExpression::Node
    {
      public:
        NodeLiteral( const QVariant &value )
          : mValue( value )
        {}

        //! The value of the literal.
        inline QVariant value() const { return mValue; }

        virtual QgsExpression::NodeType nodeType() const override { return ntLiteral; }
        virtual bool prepareNode( QgsExpression *parent, const QgsExpressionContext *context ) override;
        virtual QVariant evalNode( QgsExpression *parent, const QgsExpressionContext *context ) override;
        virtual QString dump() const override;

        virtual QSet<QString> referencedColumns() const override;
        virtual QSet<QString> referencedVariables() const override;
        virtual bool needsGeometry() const override { return false; }
        virtual QgsExpression::Node *clone() const override;
        virtual bool isStatic( QgsExpression *parent, const QgsExpressionContext *context ) const override;

      private:
        QVariant mValue;
    };

    /** \ingroup core
     */
    class CORE_EXPORT NodeColumnRef : public QgsExpression::Node
    {
      public:
        NodeColumnRef( const QString &name )
          : mName( name )
          , mIndex( -1 )
        {}

        //! The name of the column.
        QString name() const { return mName; }

        virtual QgsExpression::NodeType nodeType() const override { return ntColumnRef; }
        virtual bool prepareNode( QgsExpression *parent, const QgsExpressionContext *context ) override;
        virtual QVariant evalNode( QgsExpression *parent, const QgsExpressionContext *context ) override;
        virtual QString dump() const override;

        virtual QSet<QString> referencedColumns() const override;
        virtual QSet<QString> referencedVariables() const override;
        virtual bool needsGeometry() const override { return false; }

        virtual QgsExpression::Node *clone() const override;
        virtual bool isStatic( QgsExpression *parent, const QgsExpressionContext *context ) const override;

      private:
        QString mName;
        int mIndex;
    };

    class NodeCondition;

    /** \ingroup core
     */
    class CORE_EXPORT WhenThen
    {
      public:

        /**
         * A combination of when and then. Simple as that.
         */
        WhenThen( QgsExpression::Node *whenExp, QgsExpression::Node *thenExp );
        ~WhenThen();

        //! WhenThen nodes cannot be copied.
        WhenThen( const WhenThen &rh ) = delete;
        //! WhenThen nodes cannot be copied.
        WhenThen &operator=( const WhenThen &rh ) = delete;

        /**
         * Get a deep copy of this WhenThen combination.
         */
        QgsExpression::WhenThen *clone() const;

      private:
#ifdef SIP_RUN
        WhenThen( const QgsExpression::WhenThen &rh );
#endif
        Node *mWhenExp = nullptr;
        Node *mThenExp = nullptr;

        friend class NodeCondition;

    };
    typedef QList<QgsExpression::WhenThen *> WhenThenList;

    /** \ingroup core
     */
    class CORE_EXPORT NodeCondition : public QgsExpression::Node
    {
      public:

        /**
         * Create a new node with the given list of \a conditions and an optional \a elseExp expression.
         */
        NodeCondition( QgsExpression::WhenThenList *conditions, QgsExpression::Node *elseExp = nullptr );

        /**
         * Create a new node with the given list of \a conditions and an optional \a elseExp expression.
         */
        NodeCondition( const QgsExpression::WhenThenList &conditions, QgsExpression::Node *elseExp = nullptr ) SIP_SKIP
      : mConditions( conditions )
        , mElseExp( elseExp )
        {}

        ~NodeCondition() { delete mElseExp; qDeleteAll( mConditions ); }

        virtual QgsExpression::NodeType nodeType() const override { return ntCondition; }
        virtual QVariant evalNode( QgsExpression *parent, const QgsExpressionContext *context ) override;
        virtual bool prepareNode( QgsExpression *parent, const QgsExpressionContext *context ) override;
        virtual QString dump() const override;

        virtual QSet<QString> referencedColumns() const override;
        virtual QSet<QString> referencedVariables() const override;
        virtual bool needsGeometry() const override;
        virtual QgsExpression::Node *clone() const override;
        virtual bool isStatic( QgsExpression *parent, const QgsExpressionContext *context ) const override;

      private:
        WhenThenList mConditions;
        Node *mElseExp = nullptr;
    };

    /** Returns the help text for a specified function.
     * \param name function name
     * \see variableHelpText()
     */
    static QString helpText( QString name );

    /** Returns the help text for a specified variable.
     * \param variableName name of variable
     * \param showValue set to true to include current value of variable in help text
     * \param value current value of variable to show in help text
     * \see helpText()
     * \since QGIS 2.12
     */
    static QString variableHelpText( const QString &variableName, bool showValue = true, const QVariant &value = QVariant() );

    /** Returns the translated name for a function group.
     * \param group untranslated group name
     */
    static QString group( const QString &group );

    /** Formats an expression result for friendly display to the user. Truncates the result to a sensible
     * length, and presents text representations of non numeric/text types (e.g., geometries and features).
     * \param value expression result to format
     * \returns formatted string, may contain HTML formatting characters
     * \since QGIS 2.14
     */
    static QString formatPreviewString( const QVariant &value );

  protected:
    void initGeomCalculator();

    struct HelpArg SIP_SKIP
    {
      HelpArg( const QString &arg, const QString &desc, bool descOnly = false, bool syntaxOnly = false,
               bool optional = false, const QString &defaultVal = QString() )
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

    struct HelpExample SIP_SKIP
    {
      HelpExample( const QString &expression, const QString &returns, const QString &note = QString::null )
        : mExpression( expression )
        , mReturns( returns )
        , mNote( note )
      {}

      QString mExpression;
      QString mReturns;
      QString mNote;
    };

    struct HelpVariant SIP_SKIP
    {
      HelpVariant( const QString &name, const QString &description,
                   const QList<QgsExpression::HelpArg> &arguments = QList<QgsExpression::HelpArg>(),
                   bool variableLenArguments = false,
                   const QList<QgsExpression::HelpExample> &examples = QList<QgsExpression::HelpExample>(),
                   const QString &notes = QString::null )
        : mName( name )
        , mDescription( description )
        , mArguments( arguments )
        , mVariableLenArguments( variableLenArguments )
        , mExamples( examples )
        , mNotes( notes )
      {}

      QString mName;
      QString mDescription;
      QList<QgsExpression::HelpArg> mArguments;
      bool mVariableLenArguments;
      QList<QgsExpression::HelpExample> mExamples;
      QString mNotes;
    };

    struct Help SIP_SKIP
    {
      Help() {}

      Help( const QString &name, const QString &type, const QString &description, const QList<QgsExpression::HelpVariant> &variants )
        : mName( name )
        , mType( type )
        , mDescription( description )
        , mVariants( variants )
      {}

      QString mName;
      QString mType;
      QString mDescription;
      QList<QgsExpression::HelpVariant> mVariants;
    };

    /**
     * Helper for implicit sharing. When called will create
     * a new deep copy of this expression.
     *
     * \note not available in Python bindings
     */
    void detach() SIP_SKIP;

    QgsExpressionPrivate *d = nullptr;

    static QHash<QString, Help> sFunctionHelpTexts;
    static QHash<QString, QString> sVariableHelpTexts;
    static QHash<QString, QString> sGroups;

    //! \note not available in Python bindings
    static void initFunctionHelp() SIP_SKIP;
    //! \note not available in Python bindings
    static void initVariableHelp() SIP_SKIP;

    friend class QgsOgcUtils;
};



Q_DECLARE_METATYPE( QgsExpression::Node * )
Q_DECLARE_METATYPE( QgsExpression )

#endif // QGSEXPRESSION_H
