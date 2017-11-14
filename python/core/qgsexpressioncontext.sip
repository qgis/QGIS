/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsexpressioncontext.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsScopedExpressionFunction : QgsExpressionFunction
{
%Docstring
 Expression function for use within a QgsExpressionContextScope. This differs from a
 standard QgsExpression.Function in that it requires an implemented
 clone() method.
.. versionadded:: 2.12
%End

%TypeHeaderCode
#include "qgsexpressioncontext.h"
%End
  public:

    QgsScopedExpressionFunction( const QString &fnname,
                                 int params,
                                 const QString &group,
                                 const QString &helpText = QString(),
                                 bool usesGeometry = false,
                                 const QSet<QString> &referencedColumns = QSet<QString>(),
                                 bool lazyEval = false,
                                 bool handlesNull = false,
                                 bool isContextual = true );
%Docstring
 Create a new QgsScopedExpressionFunction

.. versionadded:: 2.12
%End

    QgsScopedExpressionFunction( const QString &fnname,
                                 const QgsExpressionFunction::ParameterList &params,
                                 const QString &group,
                                 const QString &helpText = QString(),
                                 bool usesGeometry = false,
                                 const QSet<QString> &referencedColumns = QSet<QString>(),
                                 bool lazyEval = false,
                                 bool handlesNull = false,
                                 bool isContextual = true );
%Docstring
 Create a new QgsScopedExpressionFunction using named parameters.

.. versionadded:: 3.0
%End

    virtual QVariant func( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *parent, const QgsExpressionNodeFunction *node ) = 0;

    virtual QgsScopedExpressionFunction *clone() const = 0 /Factory/;
%Docstring
 Returns a clone of the function.
 :rtype: QgsScopedExpressionFunction
%End

    virtual bool usesGeometry( const QgsExpressionNodeFunction *node ) const;

    virtual QSet<QString> referencedColumns( const QgsExpressionNodeFunction *node ) const;

    virtual bool isStatic( const QgsExpressionNodeFunction *node, QgsExpression *parent, const QgsExpressionContext *context ) const;

};



class QgsExpressionContextScope
{
%Docstring
 Single scope for storing variables and functions for use within a QgsExpressionContext.
 Examples include a project's scope, which could contain information about the current project such as
 the project file's location. QgsExpressionContextScope can encapsulate both variables (static values)
 and functions(which are calculated only when an expression is evaluated).

 See QgsExpressionContextUtils for helper methods for working with QgsExpressionContextScope objects.

.. versionadded:: 2.12
%End

%TypeHeaderCode
#include "qgsexpressioncontext.h"
%End
  public:

    struct StaticVariable
    {

      StaticVariable( const QString &name = QString(), const QVariant &value = QVariant(), bool readOnly = false, bool isStatic = false, const QString &description = QString() );
%Docstring
 Constructor for StaticVariable.
 \param name variable name (should be unique within the QgsExpressionContextScope)
 \param value initial variable value
 \param readOnly true if variable should not be editable by users
 \param isStatic true if the variable will not change during the lifteime of an iterator.
 \param description optional translated description of variable, for use in expression builder widgets
%End

      QString name;
%Docstring
Variable name
%End

      QVariant value;
%Docstring
Variable value
%End

      bool readOnly;
%Docstring
True if variable should not be editable by users
%End

      bool isStatic;
%Docstring
A static variable can be cached for the lifetime of a context
%End

      QString description;
%Docstring
Translated description of variable, for use within expression builder widgets.
%End
    };

    QgsExpressionContextScope( const QString &name = QString() );
%Docstring
 Constructor for QgsExpressionContextScope
 \param name friendly display name for the context scope
%End

    QgsExpressionContextScope( const QgsExpressionContextScope &other );
%Docstring
 Copy constructor
%End


    ~QgsExpressionContextScope();

    QString name() const;
%Docstring
 Returns the friendly display name of the context scope.
 :rtype: str
%End

    void setVariable( const QString &name, const QVariant &value, bool isStatic = false );
%Docstring
 Convenience method for setting a variable in the context scope by ``name`` name and ``value``. If a variable
 with the same name is already set then its value is overwritten, otherwise a new variable is added to the scope.
 If the ``isStatic`` parameter is set to true, this variable can be cached during the execution
 of QgsExpression.prepare().
.. seealso:: addVariable()
%End

    void addVariable( const QgsExpressionContextScope::StaticVariable &variable );
%Docstring
 Adds a variable into the context scope. If a variable with the same name is already set then its
 value is overwritten, otherwise a new variable is added to the scope.
 \param variable definition of variable to insert
.. seealso:: setVariable()
.. seealso:: addFunction()
%End

    bool removeVariable( const QString &name );
%Docstring
 Removes a variable from the context scope, if found.
 \param name name of variable to remove
 :return: true if variable was removed from the scope, false if matching variable was not
 found within the scope
 :rtype: bool
%End

    bool hasVariable( const QString &name ) const;
%Docstring
 Tests whether a variable with the specified name exists in the scope.
 \param name variable name
 :return: true if matching variable was found in the scope
.. seealso:: variable()
.. seealso:: hasFunction()
 :rtype: bool
%End

    QVariant variable( const QString &name ) const;
%Docstring
 Retrieves a variable's value from the scope.
 \param name variable name
 :return: variable value, or invalid QVariant if matching variable could not be found
.. seealso:: hasVariable()
.. seealso:: function()
 :rtype: QVariant
%End

    QStringList variableNames() const;
%Docstring
 Returns a list of variable names contained within the scope.
.. seealso:: functionNames()
.. seealso:: filteredVariableNames()
 :rtype: list of str
%End

    QStringList filteredVariableNames() const;
%Docstring
 Returns a filtered and sorted list of variable names contained within the scope.
 Hidden variable names will be excluded, and the list will be sorted so that
 read only variables are listed first.
.. seealso:: variableNames()
 :rtype: list of str
%End

    bool isReadOnly( const QString &name ) const;
%Docstring
 Tests whether the specified variable is read only and should not be editable
 by users.
 \param name variable name
 :return: true if variable is read only
 :rtype: bool
%End

    bool isStatic( const QString &name ) const;
%Docstring
 Tests whether the variable with the specified ``name`` is static and can
 be cached.

.. versionadded:: 3.0
 :rtype: bool
%End

    QString description( const QString &name ) const;
%Docstring
 Returns the translated description for the variable with the specified ``name``
 (if set).

.. versionadded:: 3.0
 :rtype: str
%End

    int variableCount() const;
%Docstring
 Returns the count of variables contained within the scope.
 :rtype: int
%End

    bool hasFunction( const QString &name ) const;
%Docstring
 Tests whether a function with the specified name exists in the scope.
 \param name function name
 :return: true if matching function was found in the scope
.. seealso:: function()
.. seealso:: hasFunction()
 :rtype: bool
%End

    QgsExpressionFunction *function( const QString &name ) const;
%Docstring
 Retrieves a function from the scope.
 \param name function name
 :return: function, or null if matching function could not be found
.. seealso:: hasFunction()
.. seealso:: functionNames()
.. seealso:: variable()
 :rtype: QgsExpressionFunction
%End

    QStringList functionNames() const;
%Docstring
 Retrieves a list of names of functions contained in the scope.
.. seealso:: function()
.. seealso:: variableNames()
 :rtype: list of str
%End

    void addFunction( const QString &name, QgsScopedExpressionFunction *function /Transfer/ );
%Docstring
 Adds a function to the scope.
 \param name function name
 \param function function to insert. Ownership is transferred to the scope.
.. seealso:: addVariable()
%End

    bool hasFeature() const;
%Docstring
 Returns true if the scope has a feature associated with it.
.. versionadded:: 3.0
.. seealso:: feature()
 :rtype: bool
%End

    QgsFeature feature() const;
%Docstring
 Sets the feature associated with the scope.
.. seealso:: setFeature()
.. seealso:: hasFeature()
.. versionadded:: 3.0
 :rtype: QgsFeature
%End

    void setFeature( const QgsFeature &feature );
%Docstring
 Convenience function for setting a feature for the scope. Any existing
 feature set by the scope will be overwritten.
 \param feature feature for scope
.. seealso:: removeFeature()
.. seealso:: feature()
%End

    void removeFeature();
%Docstring
 Removes any feature associated with the scope.
.. versionadded:: 3.0
.. seealso:: setFeature()
.. seealso:: hasFeature()
%End

    void setFields( const QgsFields &fields );
%Docstring
 Convenience function for setting a fields for the scope. Any existing
 fields set by the scope will be overwritten.
 \param fields fields for scope
%End

};

class QgsExpressionContext
{
%Docstring
 Expression contexts are used to encapsulate the parameters around which a QgsExpression should
 be evaluated. QgsExpressions can then utilize the information stored within a context to contextualise
 their evaluated result. A QgsExpressionContext consists of a stack of QgsExpressionContextScope objects,
 where scopes added later to the stack will override conflicting variables and functions from scopes
 lower in the stack.

 See QgsExpressionContextUtils for helper methods for working with QgsExpressionContext objects.

.. versionadded:: 2.12
%End

%TypeHeaderCode
#include "qgsexpressioncontext.h"
%End
  public:

    QgsExpressionContext();
%Docstring
Constructor for QgsExpressionContext
%End

    explicit QgsExpressionContext( const QList<QgsExpressionContextScope *> &scopes /Transfer/ );
%Docstring
 Initializes the context with given list of scopes.
 Ownership of the scopes is transferred to the stack.
.. versionadded:: 3.0
%End

    QgsExpressionContext( const QgsExpressionContext &other );
%Docstring
 Copy constructor
%End



    ~QgsExpressionContext();

    bool hasVariable( const QString &name ) const;
%Docstring
 Check whether a variable is specified by any scope within the context.
 \param name variable name
 :return: true if variable is set
.. seealso:: variable()
.. seealso:: variableNames()
 :rtype: bool
%End

    QVariant variable( const QString &name ) const;
%Docstring
 Fetches a matching variable from the context. The variable will be fetched
 from the last scope contained within the context which has a matching
 variable set.
 \param name variable name
 :return: variable value if matching variable exists in the context, otherwise an invalid QVariant
.. seealso:: hasVariable()
.. seealso:: variableNames()
 :rtype: QVariant
%End

    QVariantMap variablesToMap() const;
%Docstring
 Returns a map of variable name to value representing all the expression variables
 contained by the context.
.. versionadded:: 3.0
 :rtype: QVariantMap
%End

    bool isHighlightedVariable( const QString &name ) const;
%Docstring
 Returns true if the specified variable name is intended to be highlighted to the
 user. This is used by the expression builder to more prominently display the
 variable.
 \param name variable name
.. seealso:: setHighlightedVariables()
 :rtype: bool
%End

    void setHighlightedVariables( const QStringList &variableNames );
%Docstring
 Sets the list of variable names within the context intended to be highlighted to the user. This
 is used by the expression builder to more prominently display these variables.
 \param variableNames variable names to highlight
.. seealso:: isHighlightedVariable()
%End

    QgsExpressionContextScope *activeScopeForVariable( const QString &name );
%Docstring
 Returns the currently active scope from the context for a specified variable name.
 As scopes later in the stack override earlier contexts, this will be the last matching
 scope which contains a matching variable.
 \param name variable name
 :return: matching scope containing variable, or null if none found
 :rtype: QgsExpressionContextScope
%End


    QgsExpressionContextScope *scope( int index );
%Docstring
 Returns the scope at the specified index within the context.
 \param index index of scope
 :return: matching scope, or null if none found
.. seealso:: lastScope()
 :rtype: QgsExpressionContextScope
%End

    QgsExpressionContextScope *lastScope();
%Docstring
 Returns the last scope added to the context.
.. seealso:: scope()
 :rtype: QgsExpressionContextScope
%End

    QList< QgsExpressionContextScope * > scopes();
%Docstring
 Returns a list of scopes contained within the stack.
 :return: list of pointers to scopes
 :rtype: list of QgsExpressionContextScope
%End

    int indexOfScope( QgsExpressionContextScope *scope ) const;
%Docstring
 Returns the index of the specified scope if it exists within the context.
 \param scope scope to find
 :return: index of scope, or -1 if scope was not found within the context.
 :rtype: int
%End

    int indexOfScope( const QString &scopeName ) const;
%Docstring
 Returns the index of the first scope with a matching name within the context.
 \param scopeName name of scope to find
 :return: index of scope, or -1 if scope was not found within the context.
.. versionadded:: 3.0
 :rtype: int
%End

    QStringList variableNames() const;
%Docstring
 Returns a list of variables names set by all scopes in the context.
 :return: list of unique variable names
.. seealso:: filteredVariableNames
.. seealso:: functionNames
.. seealso:: hasVariable
.. seealso:: variable
 :rtype: list of str
%End

    QStringList filteredVariableNames() const;
%Docstring
 Returns a filtered list of variables names set by all scopes in the context. The included
 variables are those which should be seen by users.
 :return: filtered list of unique variable names
.. seealso:: variableNames
 :rtype: list of str
%End

    bool isReadOnly( const QString &name ) const;
%Docstring
 Returns whether a variable is read only, and should not be modifiable by users.
 \param name variable name
 :return: true if variable is read only. Read only status will be taken from last
 matching scope which contains a matching variable.
 :rtype: bool
%End

    QString description( const QString &name ) const;
%Docstring
 Returns a translated description string for the variable with specified ``name``.

 If no specific description has been provided for the variable, the value from
 QgsExpression.variableHelpText() will be returned.

.. versionadded:: 3.0
 :rtype: str
%End

    bool hasFunction( const QString &name ) const;
%Docstring
 Checks whether a specified function is contained in the context.
 \param name function name
 :return: true if context provides a matching function
.. seealso:: function
 :rtype: bool
%End

    QStringList functionNames() const;
%Docstring
 Retrieves a list of function names contained in the context.
.. seealso:: function()
.. seealso:: variableNames()
 :rtype: list of str
%End

    QgsExpressionFunction *function( const QString &name ) const;
%Docstring
 Fetches a matching function from the context. The function will be fetched
 from the last scope contained within the context which has a matching
 function set.
 \param name function name
 :return: function if contained by the context, otherwise null.
.. seealso:: hasFunction
 :rtype: QgsExpressionFunction
%End

    int scopeCount() const;
%Docstring
 Returns the number of scopes contained in the context.
 :rtype: int
%End

    void appendScope( QgsExpressionContextScope *scope /Transfer/ );
%Docstring
 Appends a scope to the end of the context. This scope will override
 any matching variables or functions provided by existing scopes within the
 context. Ownership of the scope is transferred to the stack.
 \param scope expression context to append to context
%End

    void appendScopes( const QList<QgsExpressionContextScope *> &scopes /Transfer/ );
%Docstring
 Appends a list of scopes to the end of the context. This scopes will override
 any matching variables or functions provided by existing scopes within the
 context. Ownership of the scopes is transferred to the stack.
 \param scopes scopes to append to context
.. versionadded:: 3.0
%End

    QgsExpressionContextScope *popScope();
%Docstring
 Removes the last scope from the expression context and return it.
 :rtype: QgsExpressionContextScope
%End


    void setFeature( const QgsFeature &feature );
%Docstring
 Convenience function for setting a feature for the context. The feature
 will be set within the last scope of the context, so will override any
 existing features within the context.
 \param feature feature for context
.. seealso:: feature()
%End

    bool hasFeature() const;
%Docstring
 Returns true if the context has a feature associated with it.
.. versionadded:: 3.0
.. seealso:: feature()
 :rtype: bool
%End

    QgsFeature feature() const;
%Docstring
 Convenience function for retrieving the feature for the context, if set.
.. seealso:: setFeature
 :rtype: QgsFeature
%End

    void setFields( const QgsFields &fields );
%Docstring
 Convenience function for setting a fields for the context. The fields
 will be set within the last scope of the context, so will override any
 existing fields within the context.
 \param fields fields for context
.. seealso:: fields()
%End

    QgsFields fields() const;
%Docstring
 Convenience function for retrieving the fields for the context, if set.
.. seealso:: setFields
 :rtype: QgsFields
%End

    void setOriginalValueVariable( const QVariant &value );
%Docstring
 Sets the original value variable value for the context.
 \param value value for original value variable. This usually represents the an original widget
 value before any data defined overrides have been applied.
.. versionadded:: 2.12
%End

    void setCachedValue( const QString &key, const QVariant &value ) const;
%Docstring
 Sets a value to cache within the expression context. This can be used to cache the results
 of expensive expression sub-calculations, to speed up future evaluations using the same
 expression context.
 \param key unique key for retrieving cached value
 \param value value to cache
.. seealso:: hasCachedValue()
.. seealso:: cachedValue()
.. seealso:: clearCachedValues()
.. versionadded:: 2.16
%End

    bool hasCachedValue( const QString &key ) const;
%Docstring
 Returns true if the expression context contains a cached value with a matching key.
 \param key unique key used to store cached value
.. seealso:: setCachedValue()
.. seealso:: cachedValue()
.. seealso:: clearCachedValues()
.. versionadded:: 2.16
 :rtype: bool
%End

    QVariant cachedValue( const QString &key ) const;
%Docstring
 Returns the matching cached value, if set. This can be used to retrieve the previously stored results
 of an expensive expression sub-calculation.
 \param key unique key used to store cached value
 :return: matching cached value, or invalid QVariant if not set
.. seealso:: setCachedValue()
.. seealso:: hasCachedValue()
.. seealso:: clearCachedValues()
.. versionadded:: 2.16
 :rtype: QVariant
%End

    void clearCachedValues() const;
%Docstring
 Clears all cached values from the context.
.. seealso:: setCachedValue()
.. seealso:: hasCachedValue()
.. seealso:: cachedValue()
.. versionadded:: 2.16
%End

    static const QString EXPR_FIELDS;
%Docstring
Inbuilt variable name for fields storage
%End
    static const QString EXPR_ORIGINAL_VALUE;
%Docstring
Inbuilt variable name for value original value variable
%End
    static const QString EXPR_SYMBOL_COLOR;
%Docstring
Inbuilt variable name for symbol color variable
%End
    static const QString EXPR_SYMBOL_ANGLE;
%Docstring
Inbuilt variable name for symbol angle variable
%End
    static const QString EXPR_GEOMETRY_PART_COUNT;
%Docstring
Inbuilt variable name for geometry part count variable
%End
    static const QString EXPR_GEOMETRY_PART_NUM;
%Docstring
Inbuilt variable name for geometry part number variable
%End
    static const QString EXPR_GEOMETRY_POINT_COUNT;
%Docstring
Inbuilt variable name for point count variable
%End
    static const QString EXPR_GEOMETRY_POINT_NUM;
%Docstring
Inbuilt variable name for point number variable
%End
    static const QString EXPR_CLUSTER_SIZE;
%Docstring
Inbuilt variable name for cluster size variable
%End
    static const QString EXPR_CLUSTER_COLOR;
%Docstring
Inbuilt variable name for cluster color variable
%End

};


class QgsExpressionContextUtils
{
%Docstring
 Contains utilities for working with QgsExpressionContext objects, including methods
 for creating scopes for specific uses (e.g., project scopes, layer scopes).
.. versionadded:: 2.12
%End

%TypeHeaderCode
#include "qgsexpressioncontext.h"
%End
  public:

    static QgsExpressionContextScope *globalScope() /Factory/;
%Docstring
 Creates a new scope which contains variables and functions relating to the global QGIS context.
 For instance, QGIS version numbers and variables specified through QGIS options.
.. seealso:: setGlobalVariable()
 :rtype: QgsExpressionContextScope
%End

    static void setGlobalVariable( const QString &name, const QVariant &value );
%Docstring
 Sets a global context variable. This variable will be contained within scopes retrieved via
 globalScope().
 \param name variable name
 \param value variable value
.. seealso:: setGlobalVariable()
.. seealso:: globalScope()
.. seealso:: removeGlobalVariable()
%End

    static void setGlobalVariables( const QVariantMap &variables );
%Docstring
 Sets all global context variables. Existing global variables will be removed and replaced
 with the variables specified.
 \param variables new set of global variables
.. seealso:: setGlobalVariable()
.. seealso:: globalScope()
.. seealso:: removeGlobalVariable()
%End

    static void removeGlobalVariable( const QString &name );
%Docstring
 Remove a global context variable.
 \param name variable name
.. seealso:: setGlobalVariable()
.. seealso:: setGlobalVariables()
.. seealso:: globalScope()
%End

    static QgsExpressionContextScope *projectScope( const QgsProject *project ) /Factory/;
%Docstring
 Creates a new scope which contains variables and functions relating to a QGIS project.
 For instance, project path and title, and variables specified through the project properties.
 \param project What project to use
.. seealso:: setProjectVariable()
 :rtype: QgsExpressionContextScope
%End

    static void setProjectVariable( QgsProject *project, const QString &name, const QVariant &value );
%Docstring
 Sets a project context variable. This variable will be contained within scopes retrieved via
 projectScope().
 \param project Project to apply changes to
 \param name variable name
 \param value variable value
.. seealso:: setProjectVariables()
.. seealso:: removeProjectVariable()
.. seealso:: projectScope()
%End

    static void setProjectVariables( QgsProject *project, const QVariantMap &variables );
%Docstring
 Sets all project context variables. Existing project variables will be removed and replaced
 with the variables specified.
 \param project Project to apply changes to
 \param variables new set of project variables
.. seealso:: setProjectVariable()
.. seealso:: removeProjectVariable()
.. seealso:: projectScope()
%End

    static void removeProjectVariable( QgsProject *project, const QString &name );
%Docstring
 Remove project context variable.
 \param project Project to apply changes to
 \param name variable name
.. seealso:: setProjectVariable()
.. seealso:: setProjectVariables()
.. seealso:: projectScope()
%End

    static QgsExpressionContextScope *layerScope( const QgsMapLayer *layer ) /Factory/;
%Docstring
 Creates a new scope which contains variables and functions relating to a QgsMapLayer.
 For instance, layer name, id and fields.
 :rtype: QgsExpressionContextScope
%End

    static QList<QgsExpressionContextScope *> globalProjectLayerScopes( const QgsMapLayer *layer ) /Factory/;
%Docstring
 Creates a list of three scopes: global, layer's project and layer.
.. versionadded:: 3.0
 :rtype: list of QgsExpressionContextScope
%End

    static void setLayerVariable( QgsMapLayer *layer, const QString &name, const QVariant &value );
%Docstring
 Sets a layer context variable. This variable will be contained within scopes retrieved via
 layerScope().
 \param layer map layer
 \param name variable name
 \param value variable value
.. seealso:: setLayerVariables()
.. seealso:: layerScope()
%End

    static void setLayerVariables( QgsMapLayer *layer, const QVariantMap &variables );
%Docstring
 Sets all layer context variables. Existing layer variables will be removed and replaced
 with the variables specified.
 \param layer map layer
 \param variables new set of layer variables
.. seealso:: setLayerVariable()
.. seealso:: layerScope()
%End

    static QgsExpressionContextScope *mapSettingsScope( const QgsMapSettings &mapSettings ) /Factory/;
%Docstring
 Creates a new scope which contains variables and functions relating to a QgsMapSettings object.
 For instance, map scale and rotation.
 :rtype: QgsExpressionContextScope
%End

    static QgsExpressionContextScope *mapToolCaptureScope( const QList<QgsPointLocator::Match> &matches ) /Factory/;
%Docstring
 Sets the expression context variables which are available for expressions triggered by
 a map tool capture like add feature.

.. versionadded:: 3.0
 :rtype: QgsExpressionContextScope
%End

    static QgsExpressionContextScope *updateSymbolScope( const QgsSymbol *symbol, QgsExpressionContextScope *symbolScope = 0 );
%Docstring
 Updates a symbol scope related to a QgsSymbol to an expression context.
 \param symbol symbol to extract properties from
 \param symbolScope pointer to an existing scope to update
.. versionadded:: 2.14
 :rtype: QgsExpressionContextScope
%End

    static QgsExpressionContextScope *compositionScope( const QgsComposition *composition ) /Factory/;
%Docstring
 Creates a new scope which contains variables and functions relating to a QgsComposition.
 For instance, number of pages and page sizes.
 \param composition source composition
 :rtype: QgsExpressionContextScope
%End

    static void setCompositionVariable( QgsComposition *composition, const QString &name, const QVariant &value );
%Docstring
 Sets a composition context variable. This variable will be contained within scopes retrieved via
 compositionScope().
 \param composition target composition
 \param name variable name
 \param value variable value
.. seealso:: setCompositionVariables()
.. seealso:: compositionScope()
%End

    static void setCompositionVariables( QgsComposition *composition, const QVariantMap &variables );
%Docstring
 Sets all composition context variables. Existing composition variables will be removed and replaced
 with the variables specified.
 \param composition target composition
 \param variables new set of layer variables
.. seealso:: setCompositionVariable()
.. seealso:: compositionScope()
%End

    static QgsExpressionContextScope *layoutScope( const QgsLayout *layout ) /Factory/;
%Docstring
 Creates a new scope which contains variables and functions relating to a QgsLayout ``layout``.
 For instance, number of pages and page sizes.
.. versionadded:: 3.0
 :rtype: QgsExpressionContextScope
%End

    static void setLayoutVariable( QgsLayout *layout, const QString &name, const QVariant &value );
%Docstring
 Sets a layout context variable. This variable will be contained within scopes retrieved via
 layoutScope().
 \param layout target layout
 \param name variable name
 \param value variable value
.. seealso:: setLayoutVariables()
.. seealso:: layoutScope()
.. versionadded:: 3.0
%End

    static void setLayoutVariables( QgsLayout *layout, const QVariantMap &variables );
%Docstring
 Sets all layout context variables. Existing layout variables will be removed and replaced
 with the variables specified.
 \param layout target layout
 \param variables new set of layer variables
.. seealso:: setLayoutVariable()
.. seealso:: layoutScope()
.. versionadded:: 3.0
%End

    static QgsExpressionContextScope *atlasScope( const QgsAtlasComposition *atlas ) /Factory/;
%Docstring
 Creates a new scope which contains variables and functions relating to a QgsAtlasComposition.
 For instance, current page name and number.
 \param atlas source atlas. If null, a set of default atlas variables will be added to the scope.
 :rtype: QgsExpressionContextScope
%End

    static QgsExpressionContextScope *composerItemScope( const QgsComposerItem *composerItem ) /Factory/;
%Docstring
 Creates a new scope which contains variables and functions relating to a QgsComposerItem.
 For instance, item size and position.
 \param composerItem source composer item
 :rtype: QgsExpressionContextScope
%End

    static void setComposerItemVariable( QgsComposerItem *composerItem, const QString &name, const QVariant &value );
%Docstring
 Sets a composer item context variable. This variable will be contained within scopes retrieved via
 composerItemScope().
 \param composerItem target composer item
 \param name variable name
 \param value variable value
.. seealso:: setComposerItemVariables()
.. seealso:: composerItemScope()
%End

    static void setComposerItemVariables( QgsComposerItem *composerItem, const QVariantMap &variables );
%Docstring
 Sets all composition context variables. Existing compositoin variables will be removed and replaced
 with the variables specified.
 \param composerItem target composer item
 \param variables new set of layer variables
.. seealso:: setComposerItemVariable()
.. seealso:: composerItemScope()
%End

    static QgsExpressionContext createFeatureBasedContext( const QgsFeature &feature, const QgsFields &fields );
%Docstring
 Helper function for creating an expression context which contains just a feature and fields
 collection. Generally this method should not be used as the created context does not include
 standard scopes such as the global and project scopes.
 :rtype: QgsExpressionContext
%End

    static QgsExpressionContextScope *processingAlgorithmScope( const QgsProcessingAlgorithm *algorithm, const QVariantMap &parameters, QgsProcessingContext &context ) /Factory/;
%Docstring
 Creates a new scope which contains variables and functions relating to a processing ``algorithm``,
 when used with the specified ``parameters`` and ``context``.
 For instance, algorithm name and parameter functions.
 :rtype: QgsExpressionContextScope
%End

    static QgsExpressionContextScope *notificationScope( const QString &message = QString() ) /Factory/;
%Docstring
 Creates a new scope which contains variables and functions relating to provider notifications
 \param message the notification message
 :rtype: QgsExpressionContextScope
%End

    static void registerContextFunctions();
%Docstring
 Registers all known core functions provided by QgsExpressionContextScope objects.
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsexpressioncontext.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
