/***************************************************************************
     qgsexpressioncontext.h
     ----------------------
    Date                 : April 2015
    Copyright            : (C) 2015 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSEXPRESSIONCONTEXT_H
#define QGSEXPRESSIONCONTEXT_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include <QVariant>
#include <QHash>
#include <QString>
#include <QStringList>
#include <QSet>
#include "qgsexpressionfunction.h"
#include "qgsfeature.h"

class QgsReadWriteContext;

/**
 * \ingroup core
 * \class QgsScopedExpressionFunction
 * \brief Expression function for use within a QgsExpressionContextScope. This differs from a
 * standard QgsExpression::Function in that it requires an implemented
 * clone() method.
 * \since QGIS 2.12
 */

class CORE_EXPORT QgsScopedExpressionFunction : public QgsExpressionFunction
{
  public:

    /**
     * Create a new QgsScopedExpressionFunction
     *
     * \since QGIS 2.12
     */
    QgsScopedExpressionFunction( const QString &fnname,
                                 int params,
                                 const QString &group,
                                 const QString &helpText = QString(),
                                 bool usesGeometry = false,
                                 const QSet<QString> &referencedColumns = QSet<QString>(),
                                 bool lazyEval = false,
                                 bool handlesNull = false,
                                 bool isContextual = true )
      : QgsExpressionFunction( fnname, params, group, helpText, lazyEval, handlesNull, isContextual )
      , mUsesGeometry( usesGeometry )
      , mReferencedColumns( referencedColumns )
    {}

    /**
     * Create a new QgsScopedExpressionFunction using named parameters.
     *
     * \since QGIS 3.0
     */
    QgsScopedExpressionFunction( const QString &fnname,
                                 const QgsExpressionFunction::ParameterList &params,
                                 const QString &group,
                                 const QString &helpText = QString(),
                                 bool usesGeometry = false,
                                 const QSet<QString> &referencedColumns = QSet<QString>(),
                                 bool lazyEval = false,
                                 bool handlesNull = false,
                                 bool isContextual = true )
      : QgsExpressionFunction( fnname, params, group, helpText, lazyEval, handlesNull, isContextual )
      , mUsesGeometry( usesGeometry )
      , mReferencedColumns( referencedColumns )
    {}

    QVariant func( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction *node ) override = 0;

    /**
     * Returns a clone of the function.
     */
    virtual QgsScopedExpressionFunction *clone() const = 0 SIP_FACTORY;

    bool usesGeometry( const QgsExpressionNodeFunction *node ) const override;

    QSet<QString> referencedColumns( const QgsExpressionNodeFunction *node ) const override;

    bool isStatic( const QgsExpressionNodeFunction *node, QgsExpression *parent, const QgsExpressionContext *context ) const override;

  private:
    bool mUsesGeometry;
    QSet<QString> mReferencedColumns;
};


/**
 * \ingroup core
 * \class QgsExpressionContextScope
 * \brief Single scope for storing variables and functions for use within a QgsExpressionContext.
 * Examples include a project's scope, which could contain information about the current project such as
 * the project file's location. QgsExpressionContextScope can encapsulate both variables (static values)
 * and functions(which are calculated only when an expression is evaluated).
 *
 * See QgsExpressionContextUtils for helper methods for working with QgsExpressionContextScope objects.
 *
 * \since QGIS 2.12
 */

class CORE_EXPORT QgsExpressionContextScope
{
  public:

    /**
     * Single variable definition for use within a QgsExpressionContextScope.
     */
    struct StaticVariable
    {

      /**
       * Constructor for StaticVariable.
       * \param name variable name (should be unique within the QgsExpressionContextScope)
       * \param value initial variable value
       * \param readOnly TRUE if variable should not be editable by users
       * \param isStatic TRUE if the variable will not change during the lifteime of an iterator.
       * \param description optional translated description of variable, for use in expression builder widgets
       */
      StaticVariable( const QString &name = QString(), const QVariant &value = QVariant(), bool readOnly = false, bool isStatic = false, const QString &description = QString() )
        : name( name )
        , value( value )
        , readOnly( readOnly )
        , isStatic( isStatic )
        , description( description )
      {}

      //! Variable name
      QString name;

      //! Variable value
      QVariant value;

      //! True if variable should not be editable by users
      bool readOnly;

      //! A static variable can be cached for the lifetime of a context
      bool isStatic;

      //! Translated description of variable, for use within expression builder widgets.
      QString description;
    };

    /**
     * Constructor for QgsExpressionContextScope
     * \param name friendly display name for the context scope
     */
    QgsExpressionContextScope( const QString &name = QString() );

    /**
     * Copy constructor
     */
    QgsExpressionContextScope( const QgsExpressionContextScope &other );

    QgsExpressionContextScope &operator=( const QgsExpressionContextScope &other );

    ~QgsExpressionContextScope();

    /**
     * Returns the friendly display name of the context scope.
     */
    QString name() const { return mName; }

    /**
     * Convenience method for setting a variable in the context scope by \a name name and \a value. If a variable
     * with the same name is already set then its value is overwritten, otherwise a new variable is added to the scope.
     * If the \a isStatic parameter is set to TRUE, this variable can be cached during the execution
     * of QgsExpression::prepare().
     * \see addVariable()
     */
    void setVariable( const QString &name, const QVariant &value, bool isStatic = false );

    /**
     * Adds a variable into the context scope. If a variable with the same name is already set then its
     * value is overwritten, otherwise a new variable is added to the scope.
     * \param variable definition of variable to insert
     * \see setVariable()
     * \see addFunction()
     */
    void addVariable( const QgsExpressionContextScope::StaticVariable &variable );

    /**
     * Removes a variable from the context scope, if found.
     * \param name name of variable to remove
     * \returns TRUE if variable was removed from the scope, FALSE if matching variable was not
     * found within the scope
     */
    bool removeVariable( const QString &name );

    /**
     * Tests whether a variable with the specified name exists in the scope.
     * \param name variable name
     * \returns TRUE if matching variable was found in the scope
     * \see variable()
     * \see hasFunction()
     */
    bool hasVariable( const QString &name ) const;

    /**
     * Retrieves a variable's value from the scope.
     * \param name variable name
     * \returns variable value, or invalid QVariant if matching variable could not be found
     * \see hasVariable()
     * \see function()
     */
    QVariant variable( const QString &name ) const;

    /**
     * Returns a list of variable names contained within the scope.
     * \see functionNames()
     * \see filteredVariableNames()
     */
    QStringList variableNames() const;

    /**
     * Returns a filtered and sorted list of variable names contained within the scope.
     * Hidden variable names will be excluded, and the list will be sorted so that
     * read only variables are listed first.
     * \see variableNames()
     */
    QStringList filteredVariableNames() const;

    /**
     * Tests whether the specified variable is read only and should not be editable
     * by users.
     * \param name variable name
     * \returns TRUE if variable is read only
     */
    bool isReadOnly( const QString &name ) const;

    /**
     * Tests whether the variable with the specified \a name is static and can
     * be cached.
     *
     * \since QGIS 3.0
     */
    bool isStatic( const QString &name ) const;

    /**
     * Returns the translated description for the variable with the specified \a name
     * (if set).
     *
     * \since QGIS 3.0
     */
    QString description( const QString &name ) const;

    /**
     * Returns the count of variables contained within the scope.
     */
    int variableCount() const { return mVariables.count(); }

    /**
     * Tests whether a function with the specified name exists in the scope.
     * \param name function name
     * \returns TRUE if matching function was found in the scope
     * \see function()
     * \see hasFunction()
     */
    bool hasFunction( const QString &name ) const;

    /**
     * Retrieves a function from the scope.
     * \param name function name
     * \returns function, or NULLPTR if matching function could not be found
     * \see hasFunction()
     * \see functionNames()
     * \see variable()
     */
    QgsExpressionFunction *function( const QString &name ) const;

    /**
     * Retrieves a list of names of functions contained in the scope.
     * \see function()
     * \see variableNames()
     */
    QStringList functionNames() const;

    /**
     * Adds a function to the scope.
     * \param name function name
     * \param function function to insert. Ownership is transferred to the scope.
     * \see addVariable()
     */
    void addFunction( const QString &name, QgsScopedExpressionFunction *function SIP_TRANSFER );

    /**
     * Returns TRUE if the scope has a feature associated with it.
     * \see feature()
     * \since QGIS 3.0
     */
    bool hasFeature() const { return mHasFeature; }

    /**
     * Sets the feature associated with the scope.
     * \see setFeature()
     * \see hasFeature()
     * \since QGIS 3.0
     */
    QgsFeature feature() const { return mFeature; }

    /**
     * Convenience function for setting a feature for the scope. Any existing
     * feature set by the scope will be overwritten.
     * \param feature feature for scope
     * \see removeFeature()
     * \see feature()
     */
    void setFeature( const QgsFeature &feature ) { mHasFeature = true; mFeature = feature; }

    /**
     * Removes any feature associated with the scope.
     * \see setFeature()
     * \see hasFeature()
     * \since QGIS 3.0
     */
    void removeFeature() { mHasFeature = false; mFeature = QgsFeature(); }

    /**
     * Returns TRUE if the scope has a geometry associated with it.
     * \see geometry()
     * \since QGIS 3.24
     */
    bool hasGeometry() const { return mHasGeometry; }

    /**
     * Sets the geometry associated with the scope.
     * \see setGeometry()
     * \see hasGeometry()
     * \since QGIS 3.24
     */
    QgsGeometry geometry() const { return mGeometry; }

    /**
     * Convenience function for setting a \a geometry for the scope. Any existing
     * geometry set by the scope will be overwritten.

     * \see removeGeometry()
     * \see geometry()
     * \since QGIS 3.24
     */
    void setGeometry( const QgsGeometry &geometry ) { mHasGeometry = true; mGeometry = geometry; }

    /**
     * Removes any geometry associated with the scope.
     * \see setGeometry()
     * \see hasGeometry()
     * \since QGIS 3.24
     */
    void removeGeometry() { mHasGeometry = false; mGeometry = QgsGeometry(); }

    /**
     * Convenience function for setting a fields for the scope. Any existing
     * fields set by the scope will be overwritten.
     * \param fields fields for scope
     */
    void setFields( const QgsFields &fields );

    /**
     * Reads scope variables from an XML element.
     * \see writeXml()
     * \since QGIS 3.6
     */
    void readXml( const QDomElement &element, const QgsReadWriteContext &context );

    /**
     * Writes scope variables to an XML \a element.
     * \see readXml()
     * \since QGIS 3.6
     */
    bool writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const;

  private:
    QString mName;
    QHash<QString, StaticVariable> mVariables;
    QHash<QString, QgsScopedExpressionFunction * > mFunctions;
    bool mHasFeature = false;
    QgsFeature mFeature;
    bool mHasGeometry = false;
    QgsGeometry mGeometry;
};

/**
 * \ingroup core
 * \class QgsExpressionContext
 * \brief Expression contexts are used to encapsulate the parameters around which a QgsExpression should
 * be evaluated. QgsExpressions can then utilize the information stored within a context to contextualise
 * their evaluated result. A QgsExpressionContext consists of a stack of QgsExpressionContextScope objects,
 * where scopes added later to the stack will override conflicting variables and functions from scopes
 * lower in the stack.
 *
 * See QgsExpressionContextUtils for helper methods for working with QgsExpressionContext objects.
 *
 * \since QGIS 2.12
 */
class CORE_EXPORT QgsExpressionContext
{
  public:

    //! Constructor for QgsExpressionContext
    QgsExpressionContext() = default;

    /**
     * Initializes the context with given list of scopes.
     * Ownership of the scopes is transferred to the stack.
     * \since QGIS 3.0
     */
    explicit QgsExpressionContext( const QList<QgsExpressionContextScope *> &scopes SIP_TRANSFER );

    /**
     * Copy constructor
     */
    QgsExpressionContext( const QgsExpressionContext &other );

    QgsExpressionContext &operator=( const QgsExpressionContext &other ) SIP_SKIP;

    QgsExpressionContext &operator=( QgsExpressionContext &&other ) noexcept SIP_SKIP;

    ~QgsExpressionContext();

    /**
     * Check whether a variable is specified by any scope within the context.
     * \param name variable name
     * \returns TRUE if variable is set
     * \see variable()
     * \see variableNames()
     */
    bool hasVariable( const QString &name ) const;

    /**
     * Fetches a matching variable from the context. The variable will be fetched
     * from the last scope contained within the context which has a matching
     * variable set.
     * \param name variable name
     * \returns variable value if matching variable exists in the context, otherwise an invalid QVariant
     * \see hasVariable()
     * \see variableNames()
     */
    QVariant variable( const QString &name ) const;

    /**
     * Returns a map of variable name to value representing all the expression variables
     * contained by the context.
     * \since QGIS 3.0
     */
    QVariantMap variablesToMap() const;

    /**
     * Returns TRUE if the specified variable \a name is intended to be highlighted to the
     * user. This is used by the expression builder to more prominently display the
     * variable.
     * \see setHighlightedVariables()
     * \see isHighlightedFunction()
     */
    bool isHighlightedVariable( const QString &name ) const;

    /**
     * Returns the current list of variables highlighted within the context.
     *
     * \see setHighlightedVariables()
     * \since QGIS 3.8
     */
    QStringList highlightedVariables() const;

    /**
     * Sets the list of variable names within the context intended to be highlighted to the user. This
     * is used by the expression builder to more prominently display these variables.
     * \param variableNames variable names to highlight
     * \see isHighlightedVariable()
     * \see setHighlightedFunctions()
     */
    void setHighlightedVariables( const QStringList &variableNames );

    /**
     * Returns TRUE if the specified function \a name is intended to be highlighted to the
     * user. This is used by the expression builder to more prominently display the
     * function.
     * \see setHighlightedFunctions()
     * \see isHighlightedVariable()
     * \since QGIS 3.4
     */
    bool isHighlightedFunction( const QString &name ) const;

    /**
     * Sets the list of function \a names intended to be highlighted to the user. This
     * is used by the expression builder to more prominently display these functions.
     *
     * Note that these function names may include standard functions which are not functions
     * specific to this context, and these standard functions will also be highlighted to users.
     *
     * \see isHighlightedFunction()
     * \see setHighlightedVariables()
     * \since QGIS 3.4
     */
    void setHighlightedFunctions( const QStringList &names );

    /**
     * Returns the currently active scope from the context for a specified variable name.
     * As scopes later in the stack override earlier contexts, this will be the last matching
     * scope which contains a matching variable.
     * \param name variable name
     * \returns matching scope containing variable, or NULLPTR if none found
     */
    QgsExpressionContextScope *activeScopeForVariable( const QString &name );

    /**
     * Returns the currently active scope from the context for a specified variable name.
     * As scopes later in the stack override earlier contexts, this will be the last matching
     * scope which contains a matching variable.
     * \param name variable name
     * \returns matching scope containing variable, or NULLPTR if none found
     * \note not available in Python bindings
     */
    const QgsExpressionContextScope *activeScopeForVariable( const QString &name ) const SIP_SKIP;

    /**
     * Returns the scope at the specified index within the context.
     * \param index index of scope
     * \returns matching scope, or NULLPTR if none found
     * \see lastScope()
     */
    QgsExpressionContextScope *scope( int index );

    /**
     * Returns the last scope added to the context.
     * \see scope()
     */
    QgsExpressionContextScope *lastScope();

    /**
     * Returns a list of scopes contained within the stack.
     * \returns list of pointers to scopes
     */
    QList< QgsExpressionContextScope * > scopes() { return mStack; }

    /**
     * Returns the index of the specified scope if it exists within the context.
     * \param scope scope to find
     * \returns index of scope, or -1 if scope was not found within the context.
     */
    int indexOfScope( QgsExpressionContextScope *scope ) const;

    /**
     * Returns the index of the first scope with a matching name within the context.
     * \param scopeName name of scope to find
     * \returns index of scope, or -1 if scope was not found within the context.
     * \since QGIS 3.0
     */
    int indexOfScope( const QString &scopeName ) const;

    /**
     * Returns a list of variables names set by all scopes in the context.
     * \returns list of unique variable names
     * \see filteredVariableNames
     * \see functionNames
     * \see hasVariable
     * \see variable
     */
    QStringList variableNames() const;

    /**
     * Returns a filtered list of variables names set by all scopes in the context. The included
     * variables are those which should be seen by users.
     * \returns filtered list of unique variable names
     * \see variableNames
     */
    QStringList filteredVariableNames() const;

    /**
     * Returns whether a variable is read only, and should not be modifiable by users.
     * \param name variable name
     * \returns TRUE if variable is read only. Read only status will be taken from last
     * matching scope which contains a matching variable.
     */
    bool isReadOnly( const QString &name ) const;

    /**
     * Returns a translated description string for the variable with specified \a name.
     *
     * If no specific description has been provided for the variable, the value from
     * QgsExpression::variableHelpText() will be returned.
     *
     * \since QGIS 3.0
     */
    QString description( const QString &name ) const;

    /**
     * Checks whether a specified function is contained in the context.
     * \param name function name
     * \returns TRUE if context provides a matching function
     * \see function
     */
    bool hasFunction( const QString &name ) const;

    /**
     * Retrieves a list of function names contained in the context.
     * \see function()
     * \see variableNames()
     */
    QStringList functionNames() const;

    /**
     * Fetches a matching function from the context. The function will be fetched
     * from the last scope contained within the context which has a matching
     * function set.
     * \param name function name
     * \returns function if contained by the context, otherwise NULLPTR.
     * \see hasFunction
     */
    QgsExpressionFunction *function( const QString &name ) const;

    /**
     * Returns the number of scopes contained in the context.
     */
    int scopeCount() const;

    /**
     * Appends a scope to the end of the context. This scope will override
     * any matching variables or functions provided by existing scopes within the
     * context. Ownership of the scope is transferred to the stack.
     * \param scope expression context to append to context
     */
    void appendScope( QgsExpressionContextScope *scope SIP_TRANSFER );

    /**
     * Appends a list of scopes to the end of the context. This scopes will override
     * any matching variables or functions provided by existing scopes within the
     * context. Ownership of the scopes is transferred to the stack.
     * \param scopes scopes to append to context
     * \since QGIS 3.0
     */
    void appendScopes( const QList<QgsExpressionContextScope *> &scopes SIP_TRANSFER );

    /**
     * Removes the last scope from the expression context and return it.
     */
    QgsExpressionContextScope *popScope();

    /**
     * Returns all scopes from this context and remove them, leaving this context without
     * any context.
     * Ownership is transferred to the caller.
     *
     * \note Not available in Python
     * \since QGIS 3.0
     */
    QList<QgsExpressionContextScope *> takeScopes() SIP_SKIP;

    /**
     * Appends a scope to the end of the context. This scope will override
     * any matching variables or functions provided by existing scopes within the
     * context. Ownership of the scope is transferred to the stack.
     */
    QgsExpressionContext &operator<< ( QgsExpressionContextScope *scope SIP_TRANSFER );

    /**
     * Convenience function for setting a feature for the context. The feature
     * will be set within the last scope of the context, so will override any
     * existing features within the context.
     * \param feature feature for context
     * \see feature()
     */
    void setFeature( const QgsFeature &feature );

    /**
     * Returns TRUE if the context has a feature associated with it.
     * \see feature()
     * \since QGIS 3.0
     */
    bool hasFeature() const;

    /**
     * Convenience function for retrieving the feature for the context, if set.
     * \see setFeature
     */
    QgsFeature feature() const;

    /**
     * Convenience function for setting a \a geometry for the context. The geometry
     * will be set within the last scope of the context, so will override any
     * existing geometries within the context.

     * \see geometry()
     * \since QGIS 3.24
     */
    void setGeometry( const QgsGeometry &geometry );

    /**
     * Returns TRUE if the context has a geometry associated with it.
     * \see geometry()
     * \since QGIS 3.24
     */
    bool hasGeometry() const;

    /**
     * Convenience function for retrieving the geometry for the context, if set.
     * \see setGeometry()
     * \since QGIS 3.24
     */
    QgsGeometry geometry() const;

    /**
     * Convenience function for setting a fields for the context. The fields
     * will be set within the last scope of the context, so will override any
     * existing fields within the context.
     * \param fields fields for context
     * \see fields()
     */
    void setFields( const QgsFields &fields );

    /**
     * Convenience function for retrieving the fields for the context, if set.
     * \see setFields
     */
    QgsFields fields() const;

    /**
     * Sets the original value variable value for the context.
     * \param value value for original value variable. This usually represents an original widget
     * value before any data defined overrides have been applied.
     * \since QGIS 2.12
     */
    void setOriginalValueVariable( const QVariant &value );

    /**
     * Sets a value to cache within the expression context. This can be used to cache the results
     * of expensive expression sub-calculations, to speed up future evaluations using the same
     * expression context.
     * \param key unique key for retrieving cached value
     * \param value value to cache
     * \see hasCachedValue()
     * \see cachedValue()
     * \see clearCachedValues()
     * \since QGIS 2.16
     */
    void setCachedValue( const QString &key, const QVariant &value ) const;

    /**
     * Returns TRUE if the expression context contains a cached value with a matching key.
     * \param key unique key used to store cached value
     * \see setCachedValue()
     * \see cachedValue()
     * \see clearCachedValues()
     * \since QGIS 2.16
     */
    bool hasCachedValue( const QString &key ) const;

    /**
     * Returns the matching cached value, if set. This can be used to retrieve the previously stored results
     * of an expensive expression sub-calculation.
     * \param key unique key used to store cached value
     * \returns matching cached value, or invalid QVariant if not set
     * \see setCachedValue()
     * \see hasCachedValue()
     * \see clearCachedValues()
     * \since QGIS 2.16
     */
    QVariant cachedValue( const QString &key ) const;

    /**
     * Clears all cached values from the context.
     * \see setCachedValue()
     * \see hasCachedValue()
     * \see cachedValue()
     * \since QGIS 2.16
     */
    void clearCachedValues() const;

    /**
     * Attach a \a feedback object that can be queried regularly by the expression engine to check
     * if expression evaluation should be canceled.
     *
     * Ownership of \a feedback is NOT transferred, and the caller must take care that it exists
     * for the lifetime of the expression context.
     *
     * \see feedback()
     *
     * \since QGIS 3.20
     */
    void setFeedback( QgsFeedback *feedback );

    /**
     * Returns the feedback object that can be queried regularly by the expression to check
     * if evaluation should be canceled, if set.
     *
     * \see setFeedback()
     *
     * \since QGIS 3.20
     */
    QgsFeedback *feedback() const;

    //! Inbuilt variable name for fields storage
    static const QString EXPR_FIELDS;
    //! Inbuilt variable name for value original value variable
    static const QString EXPR_ORIGINAL_VALUE;
    //! Inbuilt variable name for symbol color variable
    static const QString EXPR_SYMBOL_COLOR;
    //! Inbuilt variable name for symbol angle variable
    static const QString EXPR_SYMBOL_ANGLE;
    //! Inbuilt variable name for geometry part count variable
    static const QString EXPR_GEOMETRY_PART_COUNT;
    //! Inbuilt variable name for geometry part number variable
    static const QString EXPR_GEOMETRY_PART_NUM;

    /**
     * Inbuilt variable name for geometry ring number variable.
     * \since QGIS 3.20
     */
    static const QString EXPR_GEOMETRY_RING_NUM;
    //! Inbuilt variable name for point count variable
    static const QString EXPR_GEOMETRY_POINT_COUNT;
    //! Inbuilt variable name for point number variable
    static const QString EXPR_GEOMETRY_POINT_NUM;
    //! Inbuilt variable name for cluster size variable
    static const QString EXPR_CLUSTER_SIZE;
    //! Inbuilt variable name for cluster color variable
    static const QString EXPR_CLUSTER_COLOR;

  private:

    QList< QgsExpressionContextScope * > mStack;
    QStringList mHighlightedVariables;
    QStringList mHighlightedFunctions;

    QgsFeedback *mFeedback = nullptr;

    // Cache is mutable because we want to be able to add cached values to const contexts
    mutable QMap< QString, QVariant > mCachedValues;

};

#endif // QGSEXPRESSIONCONTEXT_H
