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
#include "qgis.h"
#include <QVariant>
#include <QHash>
#include <QString>
#include <QStringList>
#include <QSet>
#include "qgsfeature.h"
#include "qgsexpression.h"
#include "qgsexpressionfunction.h"
#include "qgspointlocator.h"

class QgsExpression;
class QgsExpressionNodeFunction;
class QgsMapLayer;
class QgsLayout;
class QgsComposition;
class QgsComposerItem;
class QgsAtlasComposition;
class QgsMapSettings;
class QgsProject;
class QgsSymbol;
class QgsProcessingAlgorithm;
class QgsProcessingContext;
class QgsLayoutAtlas;
class QgsLayoutItem;

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
       * \param readOnly true if variable should not be editable by users
       * \param isStatic true if the variable will not change during the lifteime of an iterator.
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
     * If the \a isStatic parameter is set to true, this variable can be cached during the execution
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
     * \returns true if variable was removed from the scope, false if matching variable was not
     * found within the scope
     */
    bool removeVariable( const QString &name );

    /**
     * Tests whether a variable with the specified name exists in the scope.
     * \param name variable name
     * \returns true if matching variable was found in the scope
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
     * \returns true if variable is read only
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
     * \returns true if matching function was found in the scope
     * \see function()
     * \see hasFunction()
     */
    bool hasFunction( const QString &name ) const;

    /**
     * Retrieves a function from the scope.
     * \param name function name
     * \returns function, or null if matching function could not be found
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
     * Returns true if the scope has a feature associated with it.
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

    bool variableNameSort( const QString &a, const QString &b );
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
     * \returns true if variable is set
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
     * Returns true if the specified variable \a name is intended to be highlighted to the
     * user. This is used by the expression builder to more prominently display the
     * variable.
     * \see setHighlightedVariables()
     * \see isHighlightedFunction()
     */
    bool isHighlightedVariable( const QString &name ) const;

    /**
     * Sets the list of variable names within the context intended to be highlighted to the user. This
     * is used by the expression builder to more prominently display these variables.
     * \param variableNames variable names to highlight
     * \see isHighlightedVariable()
     * \see setHighlightedFunctions()
     */
    void setHighlightedVariables( const QStringList &variableNames );

    /**
     * Returns true if the specified function \a name is intended to be highlighted to the
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
     * \returns matching scope containing variable, or null if none found
     */
    QgsExpressionContextScope *activeScopeForVariable( const QString &name );

    /**
     * Returns the currently active scope from the context for a specified variable name.
     * As scopes later in the stack override earlier contexts, this will be the last matching
     * scope which contains a matching variable.
     * \param name variable name
     * \returns matching scope containing variable, or null if none found
     * \note not available in Python bindings
     */
    const QgsExpressionContextScope *activeScopeForVariable( const QString &name ) const SIP_SKIP;

    /**
     * Returns the scope at the specified index within the context.
     * \param index index of scope
     * \returns matching scope, or null if none found
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
     * \returns true if variable is read only. Read only status will be taken from last
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
     * \returns true if context provides a matching function
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
     * \returns function if contained by the context, otherwise null.
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
     * Returns true if the context has a feature associated with it.
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
     * \param value value for original value variable. This usually represents the an original widget
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
     * Returns true if the expression context contains a cached value with a matching key.
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

    // Cache is mutable because we want to be able to add cached values to const contexts
    mutable QMap< QString, QVariant > mCachedValues;

};

/**
 * \ingroup core
 * \class QgsExpressionContextUtils
 * \brief Contains utilities for working with QgsExpressionContext objects, including methods
 * for creating scopes for specific uses (e.g., project scopes, layer scopes).
 * \since QGIS 2.12
 */

class CORE_EXPORT QgsExpressionContextUtils
{
  public:

    /**
     * Creates a new scope which contains variables and functions relating to the global QGIS context.
     * For instance, QGIS version numbers and variables specified through QGIS options.
     * \see setGlobalVariable()
     */
    static QgsExpressionContextScope *globalScope() SIP_FACTORY;

    /**
     * Creates a new scope which contains functions and variables from the current attribute form/table \a feature.
     * The variables and values in this scope will reflect the current state of the form/row being edited.
     * The \a formMode (SingleEditMode etc.) is passed as text
     * \since QGIS 3.2
     */
    static QgsExpressionContextScope *formScope( const QgsFeature &formFeature = QgsFeature( ), const QString &formMode = QString() ) SIP_FACTORY;

    /**
     * Sets a global context variable. This variable will be contained within scopes retrieved via
     * globalScope().
     * \param name variable name
     * \param value variable value
     * \see setGlobalVariable()
     * \see globalScope()
     * \see removeGlobalVariable()
     */
    static void setGlobalVariable( const QString &name, const QVariant &value );

    /**
     * Sets all global context variables. Existing global variables will be removed and replaced
     * with the variables specified.
     * \param variables new set of global variables
     * \see setGlobalVariable()
     * \see globalScope()
     * \see removeGlobalVariable()
     */
    static void setGlobalVariables( const QVariantMap &variables );

    /**
     * Remove a global context variable.
     * \param name variable name
     * \see setGlobalVariable()
     * \see setGlobalVariables()
     * \see globalScope()
     */
    static void removeGlobalVariable( const QString &name );

    /**
     * Creates a new scope which contains variables and functions relating to a QGIS project.
     * For instance, project path and title, and variables specified through the project properties.
     * \param project What project to use
     * \see setProjectVariable()
     */
    static QgsExpressionContextScope *projectScope( const QgsProject *project ) SIP_FACTORY;

    /**
     * Sets a project context variable. This variable will be contained within scopes retrieved via
     * projectScope().
     * \param project Project to apply changes to
     * \param name variable name
     * \param value variable value
     * \see setProjectVariables()
     * \see removeProjectVariable()
     * \see projectScope()
     */
    static void setProjectVariable( QgsProject *project, const QString &name, const QVariant &value );

    /**
     * Sets all project context variables. Existing project variables will be removed and replaced
     * with the variables specified.
     * \param project Project to apply changes to
     * \param variables new set of project variables
     * \see setProjectVariable()
     * \see removeProjectVariable()
     * \see projectScope()
     */
    static void setProjectVariables( QgsProject *project, const QVariantMap &variables );

    /**
     * Remove project context variable.
     * \param project Project to apply changes to
     * \param name variable name
     * \see setProjectVariable()
     * \see setProjectVariables()
     * \see projectScope()
     */
    static void removeProjectVariable( QgsProject *project, const QString &name );

    /**
     * Creates a new scope which contains variables and functions relating to a QgsMapLayer.
     * For instance, layer name, id and fields.
     */
    static QgsExpressionContextScope *layerScope( const QgsMapLayer *layer ) SIP_FACTORY;

    /**
     * Creates a list of three scopes: global, layer's project and layer.
     * \since QGIS 3.0
     */
    static QList<QgsExpressionContextScope *> globalProjectLayerScopes( const QgsMapLayer *layer ) SIP_FACTORY;

    /**
      * Sets a layer context variable. This variable will be contained within scopes retrieved via
      * layerScope().
      * \param layer map layer
      * \param name variable name
      * \param value variable value
      * \see setLayerVariables()
      * \see layerScope()
      */
    static void setLayerVariable( QgsMapLayer *layer, const QString &name, const QVariant &value );

    /**
     * Sets all layer context variables. Existing layer variables will be removed and replaced
     * with the variables specified.
     * \param layer map layer
     * \param variables new set of layer variables
     * \see setLayerVariable()
     * \see layerScope()
     */
    static void setLayerVariables( QgsMapLayer *layer, const QVariantMap &variables );

    /**
     * Creates a new scope which contains variables and functions relating to a QgsMapSettings object.
     * For instance, map scale and rotation.
     */
    static QgsExpressionContextScope *mapSettingsScope( const QgsMapSettings &mapSettings ) SIP_FACTORY;

    /**
     * Sets the expression context variables which are available for expressions triggered by
     * a map tool capture like add feature.
     *
     * \since QGIS 3.0
     */
    static QgsExpressionContextScope *mapToolCaptureScope( const QList<QgsPointLocator::Match> &matches ) SIP_FACTORY;

    /**
     * Updates a symbol scope related to a QgsSymbol to an expression context.
     * \param symbol symbol to extract properties from
     * \param symbolScope pointer to an existing scope to update
     * \since QGIS 2.14
     */
    static QgsExpressionContextScope *updateSymbolScope( const QgsSymbol *symbol, QgsExpressionContextScope *symbolScope = nullptr );

    /**
     * Creates a new scope which contains variables and functions relating to a QgsLayout \a layout.
     * For instance, number of pages and page sizes.
     * \since QGIS 3.0
     */
    static QgsExpressionContextScope *layoutScope( const QgsLayout *layout ) SIP_FACTORY;

    /**
     * Sets a layout context variable. This variable will be contained within scopes retrieved via
     * layoutScope().
     * \param layout target layout
     * \param name variable name
     * \param value variable value
     * \see setLayoutVariables()
     * \see layoutScope()
     * \since QGIS 3.0
     */
    static void setLayoutVariable( QgsLayout *layout, const QString &name, const QVariant &value );

    /**
     * Sets all layout context variables. Existing layout variables will be removed and replaced
     * with the variables specified.
     * \param layout target layout
     * \param variables new set of layer variables
     * \see setLayoutVariable()
     * \see layoutScope()
     * \since QGIS 3.0
     */
    static void setLayoutVariables( QgsLayout *layout, const QVariantMap &variables );

    /**
     * Creates a new scope which contains variables and functions relating to a QgsLayoutAtlas.
     * For instance, current page name and number.
     * \param atlas source atlas. If null, a set of default atlas variables will be added to the scope.
     */
    static QgsExpressionContextScope *atlasScope( QgsLayoutAtlas *atlas ) SIP_FACTORY;

    /**
     * Creates a new scope which contains variables and functions relating to a QgsLayoutItem.
     * For instance, item size and position.
     * \see setLayoutItemVariable()
     * \see setLayoutItemVariables()
     * \since QGIS 3.0
     */
    static QgsExpressionContextScope *layoutItemScope( const QgsLayoutItem *item ) SIP_FACTORY;

    /**
     * Sets a layout \a item context variable, with the given \a name and \a value.
     * This variable will be contained within scopes retrieved via
     * layoutItemScope().
     * \see setLayoutItemVariables()
     * \see layoutItemScope()
     * \since QGIS 3.0
     */
    static void setLayoutItemVariable( QgsLayoutItem *item, const QString &name, const QVariant &value );

    /**
     * Sets all layout item context variables for an \a item. Existing variables will be removed and replaced
     * with the \a variables specified.
     * \see setLayoutItemVariable()
     * \see layoutItemScope()
     * \since QGIS 3.0
     */
    static void setLayoutItemVariables( QgsLayoutItem *item, const QVariantMap &variables );

    /**
     * Helper function for creating an expression context which contains just a feature and fields
     * collection. Generally this method should not be used as the created context does not include
     * standard scopes such as the global and project scopes.
     */
    static QgsExpressionContext createFeatureBasedContext( const QgsFeature &feature, const QgsFields &fields );

    /**
     * Creates a new scope which contains variables and functions relating to a processing \a algorithm,
     * when used with the specified \a parameters and \a context.
     * For instance, algorithm name and parameter functions.
     */
    static QgsExpressionContextScope *processingAlgorithmScope( const QgsProcessingAlgorithm *algorithm, const QVariantMap &parameters, QgsProcessingContext &context ) SIP_FACTORY;

    /**
     * Creates a new scope which contains variables and functions relating to provider notifications
     * \param message the notification message
     */
    static QgsExpressionContextScope *notificationScope( const QString &message = QString() ) SIP_FACTORY;

    /**
     * Registers all known core functions provided by QgsExpressionContextScope objects.
     */
    static void registerContextFunctions();

  private:

    class GetLayerVisibility : public QgsScopedExpressionFunction
    {
      public:
        GetLayerVisibility( const QList<QgsMapLayer *> &layers );
        QVariant func( const QVariantList &values, const QgsExpressionContext *, QgsExpression *, const QgsExpressionNodeFunction * ) override;
        QgsScopedExpressionFunction *clone() const override;

      private:

        const QList< QPointer< QgsMapLayer > > mLayers;

    };

    friend class QgsLayoutItemMap; // needs access to GetLayerVisibility

};

#endif // QGSEXPRESSIONCONTEXT_H
