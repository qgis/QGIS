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

#include <QVariant>
#include <QHash>
#include <QString>
#include <QStringList>
#include <QSet>
#include "qgsexpression.h"

class QgsExpression;
class QgsMapLayer;
class QgsComposition;
class QgsComposerItem;
class QgsAtlasComposition;
class QgsMapSettings;
class QgsSymbolV2;

/** \ingroup core
 * \class QgsScopedExpressionFunction
 * \brief Expression function for use within a QgsExpressionContextScope. This differs from a
 * standard QgsExpression::Function in that it requires an implemented
 * clone() method.
 * \note added in QGIS 2.12
 */

class CORE_EXPORT QgsScopedExpressionFunction : public QgsExpression::Function
{
  public:
    QgsScopedExpressionFunction( const QString& fnname,
                                 int params,
                                 const QString& group,
                                 const QString& helpText = QString(),
                                 bool usesGeometry = false,
                                 const QStringList& referencedColumns = QStringList(),
                                 bool lazyEval = false,
                                 bool handlesNull = false,
                                 bool isContextual = true )
        : QgsExpression::Function( fnname, params, group, helpText, usesGeometry, referencedColumns, lazyEval, handlesNull, isContextual )
    {}

    virtual ~QgsScopedExpressionFunction() {}

    virtual QVariant func( const QVariantList& values, const QgsExpressionContext* context, QgsExpression* parent ) override = 0;

    /** Returns a clone of the function.
     */
    virtual QgsScopedExpressionFunction* clone() const = 0;

};


/** \ingroup core
 * \class QgsExpressionContextScope
 * \brief Single scope for storing variables and functions for use within a QgsExpressionContext.
 * Examples include a project's scope, which could contain information about the current project such as
 * the project file's location. QgsExpressionContextScope can encapsulate both variables (static values)
 * and functions(which are calculated only when an expression is evaluated).
 *
 * See QgsExpressionContextUtils for helper methods for working with QgsExpressionContextScope objects.
 *
 * \note added in QGIS 2.12
 */

class CORE_EXPORT QgsExpressionContextScope
{
  public:

    /** Single variable definition for use within a QgsExpressionContextScope.
     */
    struct StaticVariable
    {
      /** Constructor for StaticVariable.
       * @param name variable name (should be unique within the QgsExpressionContextScope)
       * @param value intial variable value
       * @param readOnly true if variable should not be editable by users
       */
      StaticVariable( const QString& name = QString(), const QVariant& value = QVariant(), bool readOnly = false )
          : name( name )
          , value( value )
          , readOnly( readOnly )
      {}

      /** Variable name */
      QString name;

      /** Variable value */
      QVariant value;

      /** True if variable should not be editable by users */
      bool readOnly;
    };

    /** Constructor for QgsExpressionContextScope
     * @param name friendly display name for the context scope
     */
    QgsExpressionContextScope( const QString& name = QString() );

    /** Copy constructor
     */
    QgsExpressionContextScope( const QgsExpressionContextScope& other );

    QgsExpressionContextScope& operator=( const QgsExpressionContextScope& other );

    ~QgsExpressionContextScope();

    /** Returns the friendly display name of the context scope.
     */
    QString name() const { return mName; }

    /** Convenience method for setting a variable in the context scope by name and value. If a variable
     * with the same name is already set then its value is overwritten, otherwise a new variable is added to the scope.
     * @param name variable name
     * @param value variable value
     * @see addVariable()
     */
    void setVariable( const QString& name, const QVariant& value );

    /** Adds a variable into the context scope. If a variable with the same name is already set then its
     * value is overwritten, otherwise a new variable is added to the scope.
     * @param variable definition of variable to insert
     * @see setVariable()
     * @see addFunction()
     */
    void addVariable( const QgsExpressionContextScope::StaticVariable& variable );

    /** Removes a variable from the context scope, if found.
     * @param name name of variable to remove
     * @returns true if variable was removed from the scope, false if matching variable was not
     * found within the scope
     */
    bool removeVariable( const QString& name );

    /** Tests whether a variable with the specified name exists in the scope.
     * @param name variable name
     * @returns true if matching variable was found in the scope
     * @see variable()
     * @see hasFunction()
     */
    bool hasVariable( const QString& name ) const;

    /** Retrieves a variable's value from the scope.
     * @param name variable name
     * @returns variable value, or invalid QVariant if matching variable could not be found
     * @see hasVariable()
     * @see function()
     */
    QVariant variable( const QString& name ) const;

    /** Returns a list of variable names contained within the scope.
     * @see functionNames()
     * @see filteredVariableNames()
     */
    QStringList variableNames() const;

    /** Returns a fitlered and sorted list of variable names contained within the scope.
     * Hidden variable names will be excluded, and the list will be sorted so that
     * read only variables are listed first.
     * @see variableNames()
     */
    QStringList filteredVariableNames() const;

    /** Tests whether the specified variable is read only and should not be editable
     * by users.
     * @param name variable name
     * @returns true if variable is read only
     */
    bool isReadOnly( const QString& name ) const;

    /** Returns the count of variables contained within the scope.
     */
    int variableCount() const { return mVariables.count(); }

    /** Tests whether a function with the specified name exists in the scope.
     * @param name function name
     * @returns true if matching function was found in the scope
     * @see function()
     * @see hasFunction()
     */
    bool hasFunction( const QString &name ) const;

    /** Retrieves a function from the scope.
     * @param name function name
     * @returns function, or null if matching function could not be found
     * @see hasFunction()
     * @see functionNames()
     * @see variable()
     */
    QgsExpression::Function* function( const QString &name ) const;

    /** Retrieves a list of names of functions contained in the scope.
     * @see function()
     * @see variableNames()
     */
    QStringList functionNames() const;

    /** Adds a function to the scope.
     * @param name function name
     * @param function function to insert. Ownership is transferred to the scope.
     * @see addVariable()
     */
    void addFunction( const QString& name, QgsScopedExpressionFunction* function );

    /** Convenience function for setting a feature for the scope. Any existing
     * feature set by the scope will be overwritten.
     * @param feature feature for scope
     */
    void setFeature( const QgsFeature& feature );

    /** Convenience function for setting a fields for the scope. Any existing
     * fields set by the scope will be overwritten.
     * @param fields fields for scope
     */
    void setFields( const QgsFields& fields );

  private:
    QString mName;
    QHash<QString, StaticVariable> mVariables;
    QHash<QString, QgsScopedExpressionFunction* > mFunctions;

    bool variableNameSort( const QString &a, const QString &b );
};

/** \ingroup core
 * \class QgsExpressionContext
 * \brief Expression contexts are used to encapsulate the parameters around which a QgsExpression should
 * be evaluated. QgsExpressions can then utilise the information stored within a context to contextualise
 * their evaluated result. A QgsExpressionContext consists of a stack of QgsExpressionContextScope objects,
 * where scopes added later to the stack will override conflicting variables and functions from scopes
 * lower in the stack.
 *
 * See QgsExpressionContextUtils for helper methods for working with QgsExpressionContext objects.
 *
 * \note added in QGIS 2.12
 */
class CORE_EXPORT QgsExpressionContext
{
  public:

    QgsExpressionContext() {}

    /** Copy constructor
     */
    QgsExpressionContext( const QgsExpressionContext& other );

    QgsExpressionContext& operator=( const QgsExpressionContext& other );

    ~QgsExpressionContext();

    /** Check whether a variable is specified by any scope within the context.
     * @param name variable name
     * @returns true if variable is set
     * @see variable()
     * @see variableNames()
     */
    bool hasVariable( const QString& name ) const;

    /** Fetches a matching variable from the context. The variable will be fetched
     * from the last scope contained within the context which has a matching
     * variable set.
     * @param name variable name
     * @returns variable value if matching variable exists in the context, otherwise an invalid QVariant
     * @see hasVariable()
     * @see variableNames()
     */
    QVariant variable( const QString& name ) const;

    /** Returns true if the specified variable name is intended to be highlighted to the
     * user. This is used by the expression builder to more prominently display the
     * variable.
     * @param name variable name
     * @see setHighlightedVariables()
     */
    bool isHighlightedVariable( const QString& name ) const;

    /** Sets the list of variable names within the context intended to be highlighted to the user. This
     * is used by the expression builder to more prominently display these variables.
     * @param variableNames variable names to highlight
     * @see isHighlightedVariable()
     */
    void setHighlightedVariables( const QStringList& variableNames );

    /** Returns the currently active scope from the context for a specified variable name.
     * As scopes later in the stack override earlier contexts, this will be the last matching
     * scope which contains a matching variable.
     * @param name variable name
     * @returns matching scope containing variable, or null if none found
     */
    QgsExpressionContextScope* activeScopeForVariable( const QString& name );

    /** Returns the currently active scope from the context for a specified variable name.
     * As scopes later in the stack override earlier contexts, this will be the last matching
     * scope which contains a matching variable.
     * @param name variable name
     * @returns matching scope containing variable, or null if none found
     * @note not available in python bindings
     */
    const QgsExpressionContextScope* activeScopeForVariable( const QString& name ) const;

    /** Returns the scope at the specified index within the context.
     * @param index index of scope
     * @returns matching scope, or null if none found
     * @see lastScope()
     */
    QgsExpressionContextScope* scope( int index );

    /** Returns the last scope added to the context.
     * @see scope()
     */
    QgsExpressionContextScope* lastScope();

    /** Returns a list of scopes contained within the stack.
     * @returns list of pointers to scopes
     */
    QList< QgsExpressionContextScope* > scopes() { return mStack; }

    /** Returns the index of the specified scope if it exists within the context.
     * @param scope scope to find
     * @returns index of scope, or -1 if scope was not found within the context.
     */
    int indexOfScope( QgsExpressionContextScope* scope ) const;

    /** Returns a list of variables names set by all scopes in the context.
     * @returns list of unique variable names
     * @see filteredVariableNames
     * @see functionNames
     * @see hasVariable
     * @see variable
     */
    QStringList variableNames() const;

    /** Returns a filtered list of variables names set by all scopes in the context. The included
     * variables are those which should be seen by users.
     * @returns filtered list of unique variable names
     * @see variableNames
     */
    QStringList filteredVariableNames() const;

    /** Returns whether a variable is read only, and should not be modifiable by users.
     * @param name variable name
     * @returns true if variable is read only. Read only status will be taken from last
     * matching scope which contains a matching variable.
     */
    bool isReadOnly( const QString& name ) const;

    /** Checks whether a specified function is contained in the context.
     * @param name function name
     * @returns true if context provides a matching function
     * @see function
     */
    bool hasFunction( const QString& name ) const;

    /** Retrieves a list of function names contained in the context.
     * @see function()
     * @see variableNames()
     */
    QStringList functionNames() const;

    /** Fetches a matching function from the context. The function will be fetched
     * from the last scope contained within the context which has a matching
     * function set.
     * @param name function name
     * @returns function if contained by the context, otherwise null.
     * @see hasFunction
     */
    QgsExpression::Function* function( const QString& name ) const;

    /** Returns the number of scopes contained in the context.
     */
    int scopeCount() const;

    /** Appends a scope to the end of the context. This scope will override
     * any matching variables or functions provided by existing scopes within the
     * context. Ownership of the scope is transferred to the stack.
     * @param scope expression context to append to context
     */
    void appendScope( QgsExpressionContextScope* scope );

    /**
     * Removes the last scope from the expression context and return it.
     */
    QgsExpressionContextScope* popScope();

    /** Appends a scope to the end of the context. This scope will override
     * any matching variables or functions provided by existing scopes within the
     * context. Ownership of the scope is transferred to the stack.
     */
    QgsExpressionContext& operator<< ( QgsExpressionContextScope* scope );

    /** Convenience function for setting a feature for the context. The feature
     * will be set within the last scope of the context, so will override any
     * existing features within the context.
     * @param feature feature for context
     * @see feature()
     */
    void setFeature( const QgsFeature& feature );

    /** Convenience function for retrieving the feature for the context, if set.
     * @see setFeature
     */
    QgsFeature feature() const;

    /** Convenience function for setting a fields for the context. The fields
     * will be set within the last scope of the context, so will override any
     * existing fields within the context.
     * @param fields fields for context
     * @see fields()
     */
    void setFields( const QgsFields& fields );

    /** Convenience function for retrieving the fields for the context, if set.
     * @see setFields
     */
    QgsFields fields() const;

    /** Sets the original value variable value for the context.
     * @param value value for original value variable. This usually represents the an original widget
     * value before any data defined overrides have been applied.
     * @note added in QGIS 2.12
     */
    void setOriginalValueVariable( const QVariant& value );

    /** Sets a value to cache within the expression context. This can be used to cache the results
     * of expensive expression sub-calculations, to speed up future evaluations using the same
     * expression context.
     * @param key unique key for retrieving cached value
     * @param value value to cache
     * @see hasCachedValue()
     * @see cachedValue()
     * @see clearCachedValues()
     * @note added in QGIS 2.16
     */
    void setCachedValue( const QString& key, const QVariant& value ) const;

    /** Returns true if the expression context contains a cached value with a matching key.
     * @param key unique key used to store cached value
     * @see setCachedValue()
     * @see cachedValue()
     * @see clearCachedValues()
     * @note added in QGIS 2.16
     */
    bool hasCachedValue( const QString& key ) const;

    /** Returns the matching cached value, if set. This can be used to retrieve the previously stored results
     * of an expensive expression sub-calculation.
     * @param key unique key used to store cached value
     * @returns matching cached value, or invalid QVariant if not set
     * @see setCachedValue()
     * @see hasCachedValue()
     * @see clearCachedValues()
     * @note added in QGIS 2.16
     */
    QVariant cachedValue( const QString& key ) const;

    /** Clears all cached values from the context.
     * @see setCachedValue()
     * @see hasCachedValue()
     * @see cachedValue()
     * @note added in QGIS 2.16
     */
    void clearCachedValues() const;

    //! Inbuilt variable name for fields storage
    static const QString EXPR_FIELDS;
    //! Inbuilt variable name for feature storage
    static const QString EXPR_FEATURE;
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

  private:

    QList< QgsExpressionContextScope* > mStack;
    QStringList mHighlightedVariables;

    // Cache is mutable because we want to be able to add cached values to const contexts
    mutable QMap< QString, QVariant > mCachedValues;

};

/** \ingroup core
 * \class QgsExpressionContextUtils
 * \brief Contains utilities for working with QgsExpressionContext objects, including methods
 * for creating scopes for specific uses (eg project scopes, layer scopes).
 * \note added in QGIS 2.12
 */

class CORE_EXPORT QgsExpressionContextUtils
{
  public:

    /** Creates a new scope which contains variables and functions relating to the global QGIS context.
     * For instance, QGIS version numbers and variables specified through QGIS options.
     * @see setGlobalVariable()
     */
    static QgsExpressionContextScope* globalScope();

    /** Sets a global context variable. This variable will be contained within scopes retrieved via
     * globalScope().
     * @param name variable name
     * @param value variable value
     * @see setGlobalVariable()
     * @see globalScope()
     */
    static void setGlobalVariable( const QString& name, const QVariant& value );

    /** Sets all global context variables. Existing global variables will be removed and replaced
     * with the variables specified.
     * @param variables new set of global variables
     * @see setGlobalVariable()
     * @see globalScope()
     */
    static void setGlobalVariables( const QgsStringMap& variables );

    /** Creates a new scope which contains variables and functions relating to the current QGIS project.
     * For instance, project path and title, and variables specified through the project properties.
     * @see setProjectVariable()
     */
    static QgsExpressionContextScope* projectScope();

    /** Sets a project context variable. This variable will be contained within scopes retrieved via
     * projectScope().
     * @param name variable name
     * @param value variable value
     * @see setProjectVariables()
     * @see projectScope()
     */
    static void setProjectVariable( const QString& name, const QVariant& value );

    /** Sets all project context variables. Existing project variables will be removed and replaced
     * with the variables specified.
     * @param variables new set of project variables
     * @see setProjectVariable()
     * @see projectScope()
     */
    static void setProjectVariables( const QgsStringMap& variables );

    /** Creates a new scope which contains variables and functions relating to a QgsMapLayer.
     * For instance, layer name, id and fields.
     */
    static QgsExpressionContextScope* layerScope( const QgsMapLayer *layer );

    /** Sets a layer context variable. This variable will be contained within scopes retrieved via
     * layerScope().
     * @param layer map layer
     * @param name variable name
     * @param value variable value
     * @see setLayerVariables()
     * @see layerScope()
     */
    static void setLayerVariable( QgsMapLayer* layer, const QString& name, const QVariant& value );

    /** Sets all layer context variables. Existing layer variables will be removed and replaced
     * with the variables specified.
     * @param layer map layer
     * @param variables new set of layer variables
     * @see setLayerVariable()
     * @see layerScope()
     */
    static void setLayerVariables( QgsMapLayer* layer, const QgsStringMap& variables );

    /** Creates a new scope which contains variables and functions relating to a QgsMapSettings object.
     * For instance, map scale and rotation.
     */
    static QgsExpressionContextScope* mapSettingsScope( const QgsMapSettings &mapSettings );

    /**
     * Updates a symbol scope related to a QgsSymbolV2 to an expression context.
     * @param symbol symbol to extract properties from
     * @param symbolScope pointer to an existing scope to update
     * @note added in QGIS 2.14
     */
    static QgsExpressionContextScope* updateSymbolScope( const QgsSymbolV2* symbol, QgsExpressionContextScope* symbolScope = nullptr );

    /** Creates a new scope which contains variables and functions relating to a QgsComposition.
     * For instance, number of pages and page sizes.
     * @param composition source composition
     */
    static QgsExpressionContextScope* compositionScope( const QgsComposition *composition );

    /** Sets a composition context variable. This variable will be contained within scopes retrieved via
     * compositionScope().
     * @param composition target composition
     * @param name variable name
     * @param value variable value
     * @see setCompositionVariables()
     * @see compositionScope()
     */
    static void setCompositionVariable( QgsComposition* composition, const QString& name, const QVariant& value );

    /** Sets all composition context variables. Existing composition variables will be removed and replaced
     * with the variables specified.
     * @param composition target composition
     * @param variables new set of layer variables
     * @see setCompositionVariable()
     * @see compositionScope()
     */
    static void setCompositionVariables( QgsComposition* composition, const QgsStringMap& variables );

    /** Creates a new scope which contains variables and functions relating to a QgsAtlasComposition.
     * For instance, current page name and number.
     * @param atlas source atlas. If null, a set of default atlas variables will be added to the scope.
     */
    static QgsExpressionContextScope* atlasScope( const QgsAtlasComposition* atlas );

    /** Creates a new scope which contains variables and functions relating to a QgsComposerItem.
     * For instance, item size and position.
     * @param composerItem source composer item
     */
    static QgsExpressionContextScope* composerItemScope( const QgsComposerItem *composerItem );

    /** Sets a composer item context variable. This variable will be contained within scopes retrieved via
     * composerItemScope().
     * @param composerItem target composer item
     * @param name variable name
     * @param value variable value
     * @see setComposerItemVariables()
     * @see composerItemScope()
     */
    static void setComposerItemVariable( QgsComposerItem* composerItem, const QString& name, const QVariant& value );

    /** Sets all composition context variables. Existing compositoin variables will be removed and replaced
     * with the variables specified.
     * @param composerItem target composer item
     * @param variables new set of layer variables
     * @see setComposerItemVariable()
     * @see composerItemScope()
     */
    static void setComposerItemVariables( QgsComposerItem* composerItem, const QgsStringMap& variables );

    /** Helper function for creating an expression context which contains just a feature and fields
     * collection. Generally this method should not be used as the created context does not include
     * standard scopes such as the global and project scopes.
     */
    static QgsExpressionContext createFeatureBasedContext( const QgsFeature& feature, const QgsFields& fields );

    /** Registers all known core functions provided by QgsExpressionContextScope objects.
     */
    static void registerContextFunctions();

};

#endif // QGSEXPRESSIONCONTEXT_H
