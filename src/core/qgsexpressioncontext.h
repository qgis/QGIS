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
#include "qgsfield.h"
#include "qgsfeature.h"
#include "qgsexpression.h"
#include "qgsrendercontext.h"

class QgsExpression;
class QgsVectorLayer;
class QgsFeature;
class QgsRenderContext;
class QgsProject;
class QgsMapLayer;

class QgsScopedExpressionFunction : public QgsExpression::Function
{
  public:
    QgsScopedExpressionFunction( QString fnname,
                                 int params,
                                 QString group,
                                 QString helpText = QString(),
                                 bool usesGeometry = false,
                                 QStringList referencedColumns = QStringList(),
                                 bool lazyEval = false,
                                 bool handlesNull = false )
        : QgsExpression::Function( fnname, params, group, helpText, usesGeometry, referencedColumns, lazyEval, handlesNull )
    {}

    virtual ~QgsScopedExpressionFunction() {}

    virtual QVariant func( const QVariantList& values, const QgsExpressionContext* context, QgsExpression* parent ) override = 0;

    virtual QgsScopedExpressionFunction* clone() const = 0;

};


/** \ingroup core
 * \class QgsExpressionContextScope
 * \brief Expression context
 * \note added in QGIS 2.12
 */

class CORE_EXPORT QgsExpressionContextScope
{
  public:

    struct StaticVariable
    {
      StaticVariable( const QString& name = QString(), const QVariant& value = QVariant(), bool readOnly = false ) : name( name ), value( value ), readOnly( readOnly ) {}
      QString name;
      QVariant value;
      bool readOnly;
    };

    /** Constructor for QgsExpressionContextScope
     * @param name friendly display name for the context scope
     */
    QgsExpressionContextScope( const QString& name = QString() );

    QgsExpressionContextScope( const QgsExpressionContextScope& other );

    virtual ~QgsExpressionContextScope();

    /** Returns the name of the context scope.
     */
    QString name() const { return mName; }

    /** Convenience method for setting a variable in the context scope. If the variable is already set then
     * its value is overwritten, otherwise a new variable is added to the scope.
     * @param name variable name
     * @param value variable value
     * @see insertVariable
     */
    void setVariable( const QString& name, const QVariant& value );

    /** Adds a variable into the context scope. If the variable is already set then
     * its value is overwritten, otherwise a new variable is added to the
     * scope.
     * @param variable definition of variable to insert
     * @see setVariable
     */
    void addVariable( const QgsExpressionContextScope::StaticVariable& variable );

    /** Removes a variable from the context scope, if found.
     * @param name name of variable to remove
     */
    void removeVariable( const QString& name );

    bool hasVariable( const QString& name ) const;
    QVariant variable( const QString& name ) const;
    QStringList variableNames() const;
    bool isReadOnly( const QString& name ) const;
    int variableCount() const { return mVariables.count(); }

    bool hasFunction( const QString &name ) const;
    QgsExpression::Function* function( const QString &name ) const;
    void addFunction( const QString& name, QgsScopedExpressionFunction* function );

  private:
    QString mName;
    QHash<QString, StaticVariable> mVariables;
    QHash<QString, QgsScopedExpressionFunction* > mFunctions;

};



/** \ingroup core
 * \class QgsExpressionContext
 * \brief Base class for expression contexts. Expression contexts are used to
 * encapsulate the parameters around which an expression should be evaluated.
 * Examples include the project's context, which contains information about the
 * current project such as the project file's location. Expressions can then
 * utilise the information stored in a context to vary their output result.
 * Contexts can encapsulate both variables (static values) and functions
 * (which are calculated only when the expression is evaluated).
 * \note added in QGIS 2.12
 */
class CORE_EXPORT QgsExpressionContext
{
  public:

    QgsExpressionContext( ) {}

    virtual ~QgsExpressionContext();

    /** Check whether a variable is specified in the context.
     * @param name variable name
     * @returns true if variable is set
     * @see variable
     * @see variablenames
     */
    virtual bool hasVariable( const QString& name ) const;

    /** Fetches a matching variable from the context.
     * @param name variable name
     * @returns variable value if set in the context, otherwise an invalid QVariant
     * @see hasVariable
     * @see variableNames
     */
    virtual QVariant variable( const QString& name ) const;

    /** Returns the currently active context from the stack for a specified variable.
     * As contexts later in the stack override earlier contexts, this will be
     * the last matching context which contains the variable.
     * @param name variable name
     * @returns matching context containing variable, or null if none found
     */
    QgsExpressionContextScope* activeScopeForVariable( const QString& name );

    const QgsExpressionContextScope* activeScopeForVariable( const QString& name ) const;

    QgsExpressionContextScope* scope( int index );

    /** Returns a list of variables names set in the context.
     * @returns list of unique variable names
     * @see hasVariable
     * @see variable
     */
    virtual QStringList variableNames() const;

    /** Returns whether a variable is read only, and should not be modifiable by users.
     * @param name variable name
     * @returns true if variable is read only
     */
    virtual bool isReadOnly( const QString& name ) const;

    /** Checks whether a specified function is contained in the context.
     * @param name function name
     * @returns true if context provides a matching function
     * @see function
     */
    virtual bool hasFunction( const QString& name ) const;

    /** Fetches a matching function from the context.
     * @param name function name
     * @returns function if contained by the context, otherwise null
     * @see hasFunction
     */
    virtual QgsExpression::Function* function( const QString& name ) const;

    int childCount() const;

    /** Appends a context to the end of the stack. This context will override
     * any matching variables or functions provided by existing members of the
     * stack. Ownership of the context is NOT transferred to the stack.
     * @param context expression context to append to stack
     */
    void appendScope( const QgsExpressionContextScope& scope );

    QgsExpressionContext& operator<< ( const QgsExpressionContextScope& scope );

  private:

    QList< QgsExpressionContextScope > mStack;

};

class CORE_EXPORT QgsExpressionContextUtils
{
  public:

    static QgsExpressionContextScope globalScope();

    static void setGlobalVariable( const QString& name, const QVariant& value );

    static QgsExpressionContextScope projectScope();

    static void setProjectVariable( const QString& name, const QVariant& value );

    static QgsExpressionContextScope layerScope( QgsMapLayer* layer );

    static void registerContextFunctions();

};

#endif // QGSEXPRESSIONCONTEXT_H
