/***************************************************************************
                               qgsexpressionfunction.h
                             -------------------
    begin                : May 2017
    copyright            : (C) 2017 Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSEXPRESSIONFUNCTION_H
#define QGSEXPRESSIONFUNCTION_H

#include <functional>

#include <QString>
#include <QVariant>
#include <QSet>
#include <QJsonDocument>
#include <QJsonObject>

#include "qgis.h"
#include "qgis_core.h"
#include "qgsexpressionnode.h"

class QgsExpressionNodeFunction;
class QgsExpression;
class QgsExpressionContext;
class QgsExpressionContextScope;

/**
 * \ingroup core
  * A abstract base class for defining QgsExpression functions.
  */
class CORE_EXPORT QgsExpressionFunction
{
  public:

    /**
     * Function definition for evaluation against an expression context, using a list of values as parameters to the function.
     */
    typedef QVariant( *FcnEval )( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction *node ) SIP_SKIP;

    /**
     * \ingroup core
      * Represents a single parameter passed to a function.
      * \since QGIS 2.16
      */
    class CORE_EXPORT Parameter
    {
      public:

        /**
         * Constructor for Parameter.
         * \param name parameter name, used when named parameter are specified in an expression
         * \param optional set to TRUE if parameter should be optional
         * \param defaultValue default value to use for optional parameters
         * \param isSubExpression set to TRUE if this parameter is a sub-expression
         */
        Parameter( const QString &name,
                   bool optional = false,
                   const QVariant &defaultValue = QVariant(),
                   bool isSubExpression = false )
          : mName( name )
          , mOptional( optional )
          , mDefaultValue( defaultValue )
          , mIsSubExpression( isSubExpression )
        {}

        //! Returns the name of the parameter.
        QString name() const { return mName; }

        //! Returns TRUE if the parameter is optional.
        bool optional() const { return mOptional; }

        //! Returns the default value for the parameter.
        QVariant defaultValue() const { return mDefaultValue; }

        /**
         * Returns TRUE if parameter argument is a separate sub-expression, and
         * should not be checked while determining referenced columns for the expression.
         * \since QGIS 3.2
         */
        bool isSubExpression() const { return mIsSubExpression; }

        bool operator==( const QgsExpressionFunction::Parameter &other ) const
        {
          return ( QString::compare( mName, other.mName, Qt::CaseInsensitive ) == 0 );
        }

      private:
        QString mName;
        bool mOptional = false;
        QVariant mDefaultValue;
        bool mIsSubExpression = false;
    };

    //! List of parameters, used for function definition
    typedef QList< QgsExpressionFunction::Parameter > ParameterList;

    //! Constructor for function which uses unnamed parameters
    QgsExpressionFunction( const QString &fnname,
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

    /**
     * Constructor for function which uses unnamed parameters and group list
     * \since QGIS 3.0
     */
    QgsExpressionFunction( const QString &fnname,
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

    /**
     * Constructor for function which uses named parameter list.
     * \since QGIS 2.16
     */
    QgsExpressionFunction( const QString &fnname,
                           const QgsExpressionFunction::ParameterList &params,
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

    /**
     * Constructor for function which uses named parameter list and group list.
     * \since QGIS 3.0
     */
    QgsExpressionFunction( const QString &fnname,
                           const QgsExpressionFunction::ParameterList &params,
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

    virtual ~QgsExpressionFunction() = default;

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
      for ( const Parameter &param : mParameterList )
      {
        if ( !param.optional() )
          min++;
      }
      return min;
    }

    /**
     * Returns the list of named parameters for the function, if set.
     * \since QGIS 2.16
    */
    const QgsExpressionFunction::ParameterList &parameters() const { return mParameterList; }

    //! Does this function use a geometry object.
    virtual bool usesGeometry( const QgsExpressionNodeFunction *node ) const;

    /**
     * Returns a list of possible aliases for the function. These include
     * other permissible names for the function, e.g., deprecated names.
     * \returns list of known aliases
     * \since QGIS 2.9
     */
    virtual QStringList aliases() const;

    /**
     * TRUE if this function should use lazy evaluation.  Lazy evaluation functions take QgsExpression::Node objects
     * rather than the node results when called.  You can use node->eval(parent, feature) to evaluate the node and return the result
     * Functions are non lazy default and will be given the node return value when called.
     */
    bool lazyEval() const { return mLazyEval; }

    /**
     * Will be called during prepare to determine if the function is static.
     * A function is static if it will return the same value for every feature with different
     * attributes and/or geometry.
     *
     * By default this will return TRUE, if all arguments that have been passed to the function
     * are also static.
     *
     * \since QGIS 3.0
     */
    virtual bool isStatic( const QgsExpressionNodeFunction *node, QgsExpression *parent, const QgsExpressionContext *context ) const;

    /**
     * This will be called during the prepare step() of an expression if it is not static.
     *
     * This can be used by functions to do any preparation steps that might help to speedup the upcoming
     * evaluation.
     *
     * \since QGIS 3.0
     */
    virtual bool prepare( const QgsExpressionNodeFunction *node, QgsExpression *parent, const QgsExpressionContext *context ) const;

    /**
     * Returns a set of field names which are required for this function.
     * May contain QgsFeatureRequest::AllAttributes to signal that all
     * attributes are required.
     * If in doubt this will return more fields than strictly required.
     *
     * \since QGIS 3.0
     */
    virtual QSet<QString> referencedColumns( const QgsExpressionNodeFunction *node ) const;

    /**
     * Returns whether the function is only available if provided by a QgsExpressionContext object.
     * \since QGIS 2.12
     */
    bool isContextual() const { return mIsContextual; }

    /**
     * Returns TRUE if the function is deprecated and should not be presented as a valid option
     * to users in expression builders.
     * \since QGIS 3.0
     */
    virtual bool isDeprecated() const;

    /**
     * Returns the first group which the function belongs to.
     * \note consider using groups() instead, as some functions naturally belong in multiple groups
    */
    QString group() const { return mGroups.isEmpty() ? QString() : mGroups.at( 0 ); }

    /**
     * Returns a list of the groups the function belongs to.
     * \see group()
     * \since QGIS 3.0
    */
    QStringList groups() const { return mGroups; }

    //! The help text for the function.
    const QString helpText() const;

    /**
     * Returns result of evaluating the function.
     * \param values list of values passed to the function
     * \param context context expression is being evaluated against
     * \param parent parent expression
     * \param node expression node
     * \returns result of function
     */
    virtual QVariant func( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction *node ) = 0;

    /**
     * Evaluates the function, first evaluating all required arguments before passing them to the
     * function's func() method.
     */
    virtual QVariant run( QgsExpressionNode::NodeList *args, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction *node );

    bool operator==( const QgsExpressionFunction &other ) const;

    /**
     * Returns TRUE if the function handles NULL values in arguments by itself, and the default
     * NULL value handling should be skipped.
     */
    virtual bool handlesNull() const;

  protected:

    /**
     * This will return TRUE if all the params for the provided function \a node are static within the
     * constraints imposed by the \a context within the given \a parent.
     *
     * This can be used as callback for custom implementations of subclasses. It is the default for implementation
     * for StaticFunction::isStatic.
     *
     * \since QGIS 3.0
     */
    static bool allParamsStatic( const QgsExpressionNodeFunction *node, QgsExpression *parent, const QgsExpressionContext *context );

  private:
    QString mName;
    int mParams;
    QgsExpressionFunction::ParameterList mParameterList;
    QStringList mGroups;
    QString mHelpText;
    bool mLazyEval;
    bool mHandlesNull;
    bool mIsContextual; //if true function is only available through an expression context
};

/**
 * \ingroup core
  * c++ helper class for defining QgsExpression functions.
  * \note not available in Python bindings
  */
#ifndef SIP_RUN
class QgsStaticExpressionFunction : public QgsExpressionFunction
{
  public:

    /**
     * Static function for evaluation against a QgsExpressionContext, using an unnamed list of parameter values.
     */
    QgsStaticExpressionFunction( const QString &fnname,
                                 int params,
                                 FcnEval fcn,
                                 const QString &group,
                                 const QString &helpText = QString(),
                                 bool usesGeometry = false,
                                 const QSet<QString> &referencedColumns = QSet<QString>(),
                                 bool lazyEval = false,
                                 const QStringList &aliases = QStringList(),
                                 bool handlesNull = false )
      : QgsExpressionFunction( fnname, params, group, helpText, lazyEval, handlesNull )
      , mFnc( fcn )
      , mAliases( aliases )
      , mUsesGeometry( usesGeometry )
      , mReferencedColumns( referencedColumns )
    {
    }

    /**
     * Static function for evaluation against a QgsExpressionContext, using a named list of parameter values.
     */
    QgsStaticExpressionFunction( const QString &fnname,
                                 const QgsExpressionFunction::ParameterList &params,
                                 FcnEval fcn,
                                 const QString &group,
                                 const QString &helpText = QString(),
                                 bool usesGeometry = false,
                                 const QSet<QString> &referencedColumns = QSet<QString>(),
                                 bool lazyEval = false,
                                 const QStringList &aliases = QStringList(),
                                 bool handlesNull = false )
      : QgsExpressionFunction( fnname, params, group, helpText, lazyEval, handlesNull )
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
     * If NULLPTR is passed as a node to these functions, they should stay on the safe side and return if they
     * could potentially require a geometry or columns.
     */
    QgsStaticExpressionFunction( const QString &fnname,
                                 const QgsExpressionFunction::ParameterList &params,
                                 FcnEval fcn,
                                 const QString &group,
                                 const QString &helpText,
                                 const std::function< bool( const QgsExpressionNodeFunction *node )> &usesGeometry,
                                 const std::function< QSet<QString>( const QgsExpressionNodeFunction *node )> &referencedColumns,
                                 bool lazyEval = false,
                                 const QStringList &aliases = QStringList(),
                                 bool handlesNull = false );


    /**
     * Static function for evaluation against a QgsExpressionContext, using a named list of parameter values and list
     * of groups.
     */
    QgsStaticExpressionFunction( const QString &fnname,
                                 const QgsExpressionFunction::ParameterList &params,
                                 FcnEval fcn,
                                 const QStringList &groups,
                                 const QString &helpText = QString(),
                                 bool usesGeometry = false,
                                 const QSet<QString> &referencedColumns = QSet<QString>(),
                                 bool lazyEval = false,
                                 const QStringList &aliases = QStringList(),
                                 bool handlesNull = false )
      : QgsExpressionFunction( fnname, params, groups, helpText, lazyEval, handlesNull )
      , mFnc( fcn )
      , mAliases( aliases )
      , mUsesGeometry( usesGeometry )
      , mReferencedColumns( referencedColumns )
    {}

    /**
     * Returns result of evaluating the function.
     * \param values list of values passed to the function
     * \param context context expression is being evaluated against
     * \param parent parent expression
     * \param node function node
     * \returns result of function
     */
    QVariant func( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction *node ) override
    {
      return mFnc ? mFnc( values, context, parent, node ) : QVariant();
    }

    QStringList aliases() const override;

    bool usesGeometry( const QgsExpressionNodeFunction *node ) const override;

    QSet<QString> referencedColumns( const QgsExpressionNodeFunction *node ) const override;

    bool isStatic( const QgsExpressionNodeFunction *node, QgsExpression *parent, const QgsExpressionContext *context ) const override;

    bool prepare( const QgsExpressionNodeFunction *node, QgsExpression *parent, const QgsExpressionContext *context ) const override;

    /**
     * Set a function that will be called in the prepare step to determine if the function is
     * static or not.
     * By default this is set to a function that checks all arguments that have been passed to the variable
     * and if all of them are static, it will be assumed that the function is static as well.
     */
    void setIsStaticFunction( const std::function< bool ( const QgsExpressionNodeFunction *, QgsExpression *, const QgsExpressionContext * ) > &isStatic );

    /**
     * Tag this function as either static or not static.
     * This will indicate that the function is always expected to return the same value for
     * an iteration (or explicitly request that it's going to be called for every feature, if FALSE).
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
    void setPrepareFunction( const std::function< bool( const QgsExpressionNodeFunction *, QgsExpression *, const QgsExpressionContext * )> &prepareFunc );

    /**
     * Returns a list of all registered expression functions.
     */
    static const QList<QgsExpressionFunction *> &functions();

  private:
    FcnEval mFnc;
    QStringList mAliases;
    bool mUsesGeometry;
    std::function < bool( const QgsExpressionNodeFunction *node ) > mUsesGeometryFunc;
    std::function < QSet<QString>( const QgsExpressionNodeFunction *node ) > mReferencedColumnsFunc;
    std::function < bool( const QgsExpressionNodeFunction *node,  QgsExpression *parent, const QgsExpressionContext *context ) > mIsStaticFunc = allParamsStatic;
    std::function < bool( const QgsExpressionNodeFunction *node,  QgsExpression *parent, const QgsExpressionContext *context ) > mPrepareFunc;
    QSet<QString> mReferencedColumns;
    bool mIsStatic = false;
};

/**
 * Handles the ``array_foreach(array, expression)`` expression function.
 * It temporarily appends a new scope to the expression context.
 *
 * \ingroup core
 * \note Not available in Python bindings
 * \since QGIS 3.4
 */
class QgsArrayForeachExpressionFunction : public QgsExpressionFunction
{
  public:
    QgsArrayForeachExpressionFunction();

    bool isStatic( const QgsExpressionNodeFunction *node, QgsExpression *parent, const QgsExpressionContext *context ) const override;

    QVariant run( QgsExpressionNode::NodeList *args, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction *node ) override;

    QVariant func( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction *node ) override;

    bool prepare( const QgsExpressionNodeFunction *node, QgsExpression *parent, const QgsExpressionContext *context ) const override;

};

/**
 * Handles the ``array_filter(array, expression)`` expression function.
 * It temporarily appends a new scope to the expression context.
 *
 * \ingroup core
 * \note Not available in Python bindings
 * \since QGIS 3.4
 */
class QgsArrayFilterExpressionFunction : public QgsExpressionFunction
{
  public:
    QgsArrayFilterExpressionFunction();

    bool isStatic( const QgsExpressionNodeFunction *node, QgsExpression *parent, const QgsExpressionContext *context ) const override;

    QVariant run( QgsExpressionNode::NodeList *args, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction *node ) override;

    QVariant func( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction *node ) override;

    bool prepare( const QgsExpressionNodeFunction *node, QgsExpression *parent, const QgsExpressionContext *context ) const override;

};

/**
 * Handles the ``with_variable(name, value, node)`` expression function.
 * It temporarily appends a new scope to the expression context for all nested
 * nodes.
 *
 * \ingroup core
 * \note Not available in Python bindings
 * \since QGIS 3.0
 */
class QgsWithVariableExpressionFunction : public QgsExpressionFunction
{
  public:
    QgsWithVariableExpressionFunction();

    bool isStatic( const QgsExpressionNodeFunction *node, QgsExpression *parent, const QgsExpressionContext *context ) const override;

    QVariant run( QgsExpressionNode::NodeList *args, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction *node ) override;

    QVariant func( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction *node ) override;

    bool prepare( const QgsExpressionNodeFunction *node, QgsExpression *parent, const QgsExpressionContext *context ) const override;

  private:

    /**
     * Append a scope with a single variable definition (``name``=``value``)
     */
    void appendTemporaryVariable( const QgsExpressionContext *context, const QString &name, const QVariant &value ) const;

    /**
     * Pop the temporary scope again
     */
    void popTemporaryVariable( const QgsExpressionContext *context ) const;
};

#endif

#endif // QGSEXPRESSIONFUNCTION_H
