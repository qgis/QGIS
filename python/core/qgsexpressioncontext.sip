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
%End

    virtual bool usesGeometry( const QgsExpressionNodeFunction *node ) const;


    virtual QSet<QString> referencedColumns( const QgsExpressionNodeFunction *node ) const;


    virtual bool isStatic( const QgsExpressionNodeFunction *node, QgsExpression *parent, const QgsExpressionContext *context ) const;


};



class QgsExpressionContextScope
{
%Docstring
 Single scope for storing variables and functions for use within a :py:class:`QgsExpressionContext`.
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

:param name: variable name (should be unique within the QgsExpressionContextScope)
:param value: initial variable value
:param readOnly: true if variable should not be editable by users
:param isStatic: true if the variable will not change during the lifteime of an iterator.
:param description: optional translated description of variable, for use in expression builder widgets
%End

      QString name;

      QVariant value;

      bool readOnly;

      bool isStatic;

      QString description;
    };

    QgsExpressionContextScope( const QString &name = QString() );
%Docstring
Constructor for QgsExpressionContextScope

:param name: friendly display name for the context scope
%End

    QgsExpressionContextScope( const QgsExpressionContextScope &other );
%Docstring
Copy constructor
%End


    ~QgsExpressionContextScope();

    QString name() const;
%Docstring
Returns the friendly display name of the context scope.
%End

    void setVariable( const QString &name, const QVariant &value, bool isStatic = false );
%Docstring
Convenience method for setting a variable in the context scope by ``name`` name and ``value``. If a variable
with the same name is already set then its value is overwritten, otherwise a new variable is added to the scope.
If the ``isStatic`` parameter is set to true, this variable can be cached during the execution
of :py:func:`QgsExpression.prepare()`

.. seealso:: :py:func:`addVariable`
%End

    void addVariable( const QgsExpressionContextScope::StaticVariable &variable );
%Docstring
Adds a variable into the context scope. If a variable with the same name is already set then its
value is overwritten, otherwise a new variable is added to the scope.

:param variable: definition of variable to insert

.. seealso:: :py:func:`setVariable`

.. seealso:: :py:func:`addFunction`
%End

    bool removeVariable( const QString &name );
%Docstring
Removes a variable from the context scope, if found.

:param name: name of variable to remove

:return: true if variable was removed from the scope, false if matching variable was not
found within the scope
%End

    bool hasVariable( const QString &name ) const;
%Docstring
Tests whether a variable with the specified name exists in the scope.

:param name: variable name

:return: true if matching variable was found in the scope

.. seealso:: :py:func:`variable`

.. seealso:: :py:func:`hasFunction`
%End

    QVariant variable( const QString &name ) const;
%Docstring
Retrieves a variable's value from the scope.

:param name: variable name

:return: variable value, or invalid QVariant if matching variable could not be found

.. seealso:: :py:func:`hasVariable`

.. seealso:: :py:func:`function`
%End

    QStringList variableNames() const;
%Docstring
Returns a list of variable names contained within the scope.

.. seealso:: :py:func:`functionNames`

.. seealso:: :py:func:`filteredVariableNames`
%End

    QStringList filteredVariableNames() const;
%Docstring
Returns a filtered and sorted list of variable names contained within the scope.
Hidden variable names will be excluded, and the list will be sorted so that
read only variables are listed first.

.. seealso:: :py:func:`variableNames`
%End

    bool isReadOnly( const QString &name ) const;
%Docstring
Tests whether the specified variable is read only and should not be editable
by users.

:param name: variable name

:return: true if variable is read only
%End

    bool isStatic( const QString &name ) const;
%Docstring
Tests whether the variable with the specified ``name`` is static and can
be cached.

.. versionadded:: 3.0
%End

    QString description( const QString &name ) const;
%Docstring
Returns the translated description for the variable with the specified ``name``
(if set).

.. versionadded:: 3.0
%End

    int variableCount() const;
%Docstring
Returns the count of variables contained within the scope.
%End

    bool hasFunction( const QString &name ) const;
%Docstring
Tests whether a function with the specified name exists in the scope.

:param name: function name

:return: true if matching function was found in the scope

.. seealso:: :py:func:`function`

.. seealso:: :py:func:`hasFunction`
%End

    QgsExpressionFunction *function( const QString &name ) const;
%Docstring
Retrieves a function from the scope.

:param name: function name

:return: function, or null if matching function could not be found

.. seealso:: :py:func:`hasFunction`

.. seealso:: :py:func:`functionNames`

.. seealso:: :py:func:`variable`
%End

    QStringList functionNames() const;
%Docstring
Retrieves a list of names of functions contained in the scope.

.. seealso:: :py:func:`function`

.. seealso:: :py:func:`variableNames`
%End

    void addFunction( const QString &name, QgsScopedExpressionFunction *function /Transfer/ );
%Docstring
Adds a function to the scope.

:param name: function name
:param function: function to insert. Ownership is transferred to the scope.

.. seealso:: :py:func:`addVariable`
%End

    bool hasFeature() const;
%Docstring
Returns true if the scope has a feature associated with it.

.. versionadded:: 3.0

.. seealso:: :py:func:`feature`
%End

    QgsFeature feature() const;
%Docstring
Sets the feature associated with the scope.

.. seealso:: :py:func:`setFeature`

.. seealso:: :py:func:`hasFeature`

.. versionadded:: 3.0
%End

    void setFeature( const QgsFeature &feature );
%Docstring
Convenience function for setting a feature for the scope. Any existing
feature set by the scope will be overwritten.

:param feature: feature for scope

.. seealso:: :py:func:`removeFeature`

.. seealso:: :py:func:`feature`
%End

    void removeFeature();
%Docstring
Removes any feature associated with the scope.

.. versionadded:: 3.0

.. seealso:: :py:func:`setFeature`

.. seealso:: :py:func:`hasFeature`
%End

    void setFields( const QgsFields &fields );
%Docstring
Convenience function for setting a fields for the scope. Any existing
fields set by the scope will be overwritten.

:param fields: fields for scope
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

:param name: variable name

:return: true if variable is set

.. seealso:: :py:func:`variable`

.. seealso:: :py:func:`variableNames`
%End

    QVariant variable( const QString &name ) const;
%Docstring
Fetches a matching variable from the context. The variable will be fetched
from the last scope contained within the context which has a matching
variable set.

:param name: variable name

:return: variable value if matching variable exists in the context, otherwise an invalid QVariant

.. seealso:: :py:func:`hasVariable`

.. seealso:: :py:func:`variableNames`
%End

    QVariantMap variablesToMap() const;
%Docstring
Returns a map of variable name to value representing all the expression variables
contained by the context.

.. versionadded:: 3.0
%End

    bool isHighlightedVariable( const QString &name ) const;
%Docstring
Returns true if the specified variable name is intended to be highlighted to the
user. This is used by the expression builder to more prominently display the
variable.

:param name: variable name

.. seealso:: :py:func:`setHighlightedVariables`
%End

    void setHighlightedVariables( const QStringList &variableNames );
%Docstring
Sets the list of variable names within the context intended to be highlighted to the user. This
is used by the expression builder to more prominently display these variables.

:param variableNames: variable names to highlight

.. seealso:: :py:func:`isHighlightedVariable`
%End

    QgsExpressionContextScope *activeScopeForVariable( const QString &name );
%Docstring
Returns the currently active scope from the context for a specified variable name.
As scopes later in the stack override earlier contexts, this will be the last matching
scope which contains a matching variable.

:param name: variable name

:return: matching scope containing variable, or null if none found
%End


    QgsExpressionContextScope *scope( int index );
%Docstring
Returns the scope at the specified index within the context.

:param index: index of scope

:return: matching scope, or null if none found

.. seealso:: :py:func:`lastScope`
%End

    QgsExpressionContextScope *lastScope();
%Docstring
Returns the last scope added to the context.

.. seealso:: :py:func:`scope`
%End

    QList< QgsExpressionContextScope * > scopes();
%Docstring
Returns a list of scopes contained within the stack.

:return: list of pointers to scopes
%End

    int indexOfScope( QgsExpressionContextScope *scope ) const;
%Docstring
Returns the index of the specified scope if it exists within the context.

:param scope: scope to find

:return: index of scope, or -1 if scope was not found within the context.
%End

    int indexOfScope( const QString &scopeName ) const;
%Docstring
Returns the index of the first scope with a matching name within the context.

:param scopeName: name of scope to find

:return: index of scope, or -1 if scope was not found within the context.

.. versionadded:: 3.0
%End

    QStringList variableNames() const;
%Docstring
Returns a list of variables names set by all scopes in the context.

:return: list of unique variable names

.. seealso:: :py:func:`filteredVariableNames`

.. seealso:: :py:func:`functionNames`

.. seealso:: :py:func:`hasVariable`

.. seealso:: :py:func:`variable`
%End

    QStringList filteredVariableNames() const;
%Docstring
Returns a filtered list of variables names set by all scopes in the context. The included
variables are those which should be seen by users.

:return: filtered list of unique variable names

.. seealso:: :py:func:`variableNames`
%End

    bool isReadOnly( const QString &name ) const;
%Docstring
Returns whether a variable is read only, and should not be modifiable by users.

:param name: variable name

:return: true if variable is read only. Read only status will be taken from last
matching scope which contains a matching variable.
%End

    QString description( const QString &name ) const;
%Docstring
Returns a translated description string for the variable with specified ``name``.

If no specific description has been provided for the variable, the value from
QgsExpression.variableHelpText() will be returned.

.. versionadded:: 3.0
%End

    bool hasFunction( const QString &name ) const;
%Docstring
Checks whether a specified function is contained in the context.

:param name: function name

:return: true if context provides a matching function

.. seealso:: :py:func:`function`
%End

    QStringList functionNames() const;
%Docstring
Retrieves a list of function names contained in the context.

.. seealso:: :py:func:`function`

.. seealso:: :py:func:`variableNames`
%End

    QgsExpressionFunction *function( const QString &name ) const;
%Docstring
Fetches a matching function from the context. The function will be fetched
from the last scope contained within the context which has a matching
function set.

:param name: function name

:return: function if contained by the context, otherwise null.

.. seealso:: :py:func:`hasFunction`
%End

    int scopeCount() const;
%Docstring
Returns the number of scopes contained in the context.
%End

    void appendScope( QgsExpressionContextScope *scope /Transfer/ );
%Docstring
Appends a scope to the end of the context. This scope will override
any matching variables or functions provided by existing scopes within the
context. Ownership of the scope is transferred to the stack.

:param scope: expression context to append to context
%End

    void appendScopes( const QList<QgsExpressionContextScope *> &scopes /Transfer/ );
%Docstring
Appends a list of scopes to the end of the context. This scopes will override
any matching variables or functions provided by existing scopes within the
context. Ownership of the scopes is transferred to the stack.

:param scopes: scopes to append to context

.. versionadded:: 3.0
%End

    QgsExpressionContextScope *popScope();
%Docstring
Removes the last scope from the expression context and return it.
%End



    void setFeature( const QgsFeature &feature );
%Docstring
Convenience function for setting a feature for the context. The feature
will be set within the last scope of the context, so will override any
existing features within the context.

:param feature: feature for context

.. seealso:: :py:func:`feature`
%End

    bool hasFeature() const;
%Docstring
Returns true if the context has a feature associated with it.

.. versionadded:: 3.0

.. seealso:: :py:func:`feature`
%End

    QgsFeature feature() const;
%Docstring
Convenience function for retrieving the feature for the context, if set.

.. seealso:: :py:func:`setFeature`
%End

    void setFields( const QgsFields &fields );
%Docstring
Convenience function for setting a fields for the context. The fields
will be set within the last scope of the context, so will override any
existing fields within the context.

:param fields: fields for context

.. seealso:: :py:func:`fields`
%End

    QgsFields fields() const;
%Docstring
Convenience function for retrieving the fields for the context, if set.

.. seealso:: :py:func:`setFields`
%End

    void setOriginalValueVariable( const QVariant &value );
%Docstring
Sets the original value variable value for the context.

:param value: value for original value variable. This usually represents the an original widget
value before any data defined overrides have been applied.

.. versionadded:: 2.12
%End

    void setCachedValue( const QString &key, const QVariant &value ) const;
%Docstring
Sets a value to cache within the expression context. This can be used to cache the results
of expensive expression sub-calculations, to speed up future evaluations using the same
expression context.

:param key: unique key for retrieving cached value
:param value: value to cache

.. seealso:: :py:func:`hasCachedValue`

.. seealso:: :py:func:`cachedValue`

.. seealso:: :py:func:`clearCachedValues`

.. versionadded:: 2.16
%End

    bool hasCachedValue( const QString &key ) const;
%Docstring
Returns true if the expression context contains a cached value with a matching key.

:param key: unique key used to store cached value

.. seealso:: :py:func:`setCachedValue`

.. seealso:: :py:func:`cachedValue`

.. seealso:: :py:func:`clearCachedValues`

.. versionadded:: 2.16
%End

    QVariant cachedValue( const QString &key ) const;
%Docstring
Returns the matching cached value, if set. This can be used to retrieve the previously stored results
of an expensive expression sub-calculation.

:param key: unique key used to store cached value

:return: matching cached value, or invalid QVariant if not set

.. seealso:: :py:func:`setCachedValue`

.. seealso:: :py:func:`hasCachedValue`

.. seealso:: :py:func:`clearCachedValues`

.. versionadded:: 2.16
%End

    void clearCachedValues() const;
%Docstring
Clears all cached values from the context.

.. seealso:: :py:func:`setCachedValue`

.. seealso:: :py:func:`hasCachedValue`

.. seealso:: :py:func:`cachedValue`

.. versionadded:: 2.16
%End

    static const QString EXPR_FIELDS;
    static const QString EXPR_ORIGINAL_VALUE;
    static const QString EXPR_SYMBOL_COLOR;
    static const QString EXPR_SYMBOL_ANGLE;
    static const QString EXPR_GEOMETRY_PART_COUNT;
    static const QString EXPR_GEOMETRY_PART_NUM;
    static const QString EXPR_GEOMETRY_POINT_COUNT;
    static const QString EXPR_GEOMETRY_POINT_NUM;
    static const QString EXPR_CLUSTER_SIZE;
    static const QString EXPR_CLUSTER_COLOR;

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

.. seealso:: :py:func:`setGlobalVariable`
%End

    static void setGlobalVariable( const QString &name, const QVariant &value );
%Docstring
Sets a global context variable. This variable will be contained within scopes retrieved via
globalScope().

:param name: variable name
:param value: variable value

.. seealso:: :py:func:`setGlobalVariable`

.. seealso:: :py:func:`globalScope`

.. seealso:: :py:func:`removeGlobalVariable`
%End

    static void setGlobalVariables( const QVariantMap &variables );
%Docstring
Sets all global context variables. Existing global variables will be removed and replaced
with the variables specified.

:param variables: new set of global variables

.. seealso:: :py:func:`setGlobalVariable`

.. seealso:: :py:func:`globalScope`

.. seealso:: :py:func:`removeGlobalVariable`
%End

    static void removeGlobalVariable( const QString &name );
%Docstring
Remove a global context variable.

:param name: variable name

.. seealso:: :py:func:`setGlobalVariable`

.. seealso:: :py:func:`setGlobalVariables`

.. seealso:: :py:func:`globalScope`
%End

    static QgsExpressionContextScope *projectScope( const QgsProject *project ) /Factory/;
%Docstring
Creates a new scope which contains variables and functions relating to a QGIS project.
For instance, project path and title, and variables specified through the project properties.

:param project: What project to use

.. seealso:: :py:func:`setProjectVariable`
%End

    static void setProjectVariable( QgsProject *project, const QString &name, const QVariant &value );
%Docstring
Sets a project context variable. This variable will be contained within scopes retrieved via
projectScope().

:param project: Project to apply changes to
:param name: variable name
:param value: variable value

.. seealso:: :py:func:`setProjectVariables`

.. seealso:: :py:func:`removeProjectVariable`

.. seealso:: :py:func:`projectScope`
%End

    static void setProjectVariables( QgsProject *project, const QVariantMap &variables );
%Docstring
Sets all project context variables. Existing project variables will be removed and replaced
with the variables specified.

:param project: Project to apply changes to
:param variables: new set of project variables

.. seealso:: :py:func:`setProjectVariable`

.. seealso:: :py:func:`removeProjectVariable`

.. seealso:: :py:func:`projectScope`
%End

    static void removeProjectVariable( QgsProject *project, const QString &name );
%Docstring
Remove project context variable.

:param project: Project to apply changes to
:param name: variable name

.. seealso:: :py:func:`setProjectVariable`

.. seealso:: :py:func:`setProjectVariables`

.. seealso:: :py:func:`projectScope`
%End

    static QgsExpressionContextScope *layerScope( const QgsMapLayer *layer ) /Factory/;
%Docstring
Creates a new scope which contains variables and functions relating to a :py:class:`QgsMapLayer`.
For instance, layer name, id and fields.
%End

    static QList<QgsExpressionContextScope *> globalProjectLayerScopes( const QgsMapLayer *layer ) /Factory/;
%Docstring
Creates a list of three scopes: global, layer's project and layer.

.. versionadded:: 3.0
%End

    static void setLayerVariable( QgsMapLayer *layer, const QString &name, const QVariant &value );
%Docstring
Sets a layer context variable. This variable will be contained within scopes retrieved via
layerScope().

:param layer: map layer
:param name: variable name
:param value: variable value

.. seealso:: :py:func:`setLayerVariables`

.. seealso:: :py:func:`layerScope`
%End

    static void setLayerVariables( QgsMapLayer *layer, const QVariantMap &variables );
%Docstring
Sets all layer context variables. Existing layer variables will be removed and replaced
with the variables specified.

:param layer: map layer
:param variables: new set of layer variables

.. seealso:: :py:func:`setLayerVariable`

.. seealso:: :py:func:`layerScope`
%End

    static QgsExpressionContextScope *mapSettingsScope( const QgsMapSettings &mapSettings ) /Factory/;
%Docstring
Creates a new scope which contains variables and functions relating to a QgsMapSettings object.
For instance, map scale and rotation.
%End

    static QgsExpressionContextScope *mapToolCaptureScope( const QList<QgsPointLocator::Match> &matches ) /Factory/;
%Docstring
Sets the expression context variables which are available for expressions triggered by
a map tool capture like add feature.

.. versionadded:: 3.0
%End

    static QgsExpressionContextScope *updateSymbolScope( const QgsSymbol *symbol, QgsExpressionContextScope *symbolScope = 0 );
%Docstring
Updates a symbol scope related to a QgsSymbol to an expression context.

:param symbol: symbol to extract properties from
:param symbolScope: pointer to an existing scope to update

.. versionadded:: 2.14
%End


    static QgsExpressionContextScope *layoutScope( const QgsLayout *layout ) /Factory/;
%Docstring
Creates a new scope which contains variables and functions relating to a :py:class:`QgsLayout` ``layout``.
For instance, number of pages and page sizes.

.. versionadded:: 3.0
%End

    static void setLayoutVariable( QgsLayout *layout, const QString &name, const QVariant &value );
%Docstring
Sets a layout context variable. This variable will be contained within scopes retrieved via
layoutScope().

:param layout: target layout
:param name: variable name
:param value: variable value

.. seealso:: :py:func:`setLayoutVariables`

.. seealso:: :py:func:`layoutScope`

.. versionadded:: 3.0
%End

    static void setLayoutVariables( QgsLayout *layout, const QVariantMap &variables );
%Docstring
Sets all layout context variables. Existing layout variables will be removed and replaced
with the variables specified.

:param layout: target layout
:param variables: new set of layer variables

.. seealso:: :py:func:`setLayoutVariable`

.. seealso:: :py:func:`layoutScope`

.. versionadded:: 3.0
%End


    static QgsExpressionContextScope *atlasScope( QgsLayoutAtlas *atlas ) /Factory/;
%Docstring
Creates a new scope which contains variables and functions relating to a :py:class:`QgsLayoutAtlas`.
For instance, current page name and number.

:param atlas: source atlas. If null, a set of default atlas variables will be added to the scope.
%End


    static QgsExpressionContextScope *layoutItemScope( const QgsLayoutItem *item ) /Factory/;
%Docstring
Creates a new scope which contains variables and functions relating to a :py:class:`QgsLayoutItem`.
For instance, item size and position.

.. versionadded:: 3.0

.. seealso:: :py:func:`setLayoutItemVariable`

.. seealso:: :py:func:`setLayoutItemVariables`
%End

    static void setLayoutItemVariable( QgsLayoutItem *item, const QString &name, const QVariant &value );
%Docstring
Sets a layout ``item`` context variable, with the given ``name`` and ``value``.
This variable will be contained within scopes retrieved via
layoutItemScope().

.. seealso:: :py:func:`setLayoutItemVariables`

.. seealso:: :py:func:`layoutItemScope`

.. versionadded:: 3.0
%End

    static void setLayoutItemVariables( QgsLayoutItem *item, const QVariantMap &variables );
%Docstring
Sets all layout item context variables for an ``item``. Existing variables will be removed and replaced
with the ``variables`` specified.

.. seealso:: :py:func:`setLayoutItemVariable`

.. seealso:: :py:func:`layoutItemScope`

.. versionadded:: 3.0
%End



    static QgsExpressionContext createFeatureBasedContext( const QgsFeature &feature, const QgsFields &fields );
%Docstring
Helper function for creating an expression context which contains just a feature and fields
collection. Generally this method should not be used as the created context does not include
standard scopes such as the global and project scopes.
%End

    static QgsExpressionContextScope *processingAlgorithmScope( const QgsProcessingAlgorithm *algorithm, const QVariantMap &parameters, QgsProcessingContext &context ) /Factory/;
%Docstring
Creates a new scope which contains variables and functions relating to a processing ``algorithm``,
when used with the specified ``parameters`` and ``context``.
For instance, algorithm name and parameter functions.
%End

    static QgsExpressionContextScope *notificationScope( const QString &message = QString() ) /Factory/;
%Docstring
Creates a new scope which contains variables and functions relating to provider notifications

:param message: the notification message
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
