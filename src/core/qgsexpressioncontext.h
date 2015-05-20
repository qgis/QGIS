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

/** \ingroup core
 * \class QgsExpressionContext
 * \brief Base class for expression contexts. Expression contexts are used to
 * encapsulate the parameters around which an expression should be evaluated.
 * Examples include the project's context, which contains information about the
 * current project such as the project file's location. Expressions can then
 * utilise the information stored in a context to vary their output result.
 * Contexts can encapsulate both variables (static values) and functions
 * (which are calculated only when the expression is evaluated).
 * \note added in QGIS 2.9
 */
class CORE_EXPORT QgsExpressionContext
{
  public:

    /** Constructor for QgsExpressionContext
     * @param name friendly display name for the context
     */
    QgsExpressionContext( const QString& name = QString() ) : mName( name ) {}

    virtual ~QgsExpressionContext();

    /** Returns the name of the context.
     */
    QString name() const { return mName; }

    /** Check whether a variable is specified in the context.
     * @param name variable name
     * @returns true if variable is set
     * @see variable
     * @see variablenames
     */
    virtual bool hasVariable( const QString& name ) const { Q_UNUSED( name ); return false; }

    /** Fetches a matching variable from the context.
     * @param name variable name
     * @returns variable value if set in the context, otherwise an invalid QVariant
     * @see hasVariable
     * @see variableNames
     */
    virtual QVariant variable( const QString& name ) const { Q_UNUSED( name ); return QVariant(); }

    /** Returns a list of variables names set in the context.
     * @returns list of unique variable names
     * @see hasVariable
     * @see variable
     */
    virtual QStringList variableNames() const { return QStringList(); }

    /** Returns whether a variable is read only, and should not be modifiable by users.
     * @param name variable name
     * @returns true if variable is read only
     */
    virtual bool isReadOnly( const QString& name ) const { Q_UNUSED( name ); return true; }

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

    /** Returns a list of reserved variable names. These variable names
     * are used internally by QGIS and must not be reused by user
     * specified variables.
     */
    static QStringList reservedVariables() { return mReservedVariableNames; }

  protected:

    QHash<QString, QgsExpression::Function*> mFunctions;
    static QStringList mReservedVariableNames;
    QString mName;

};


/** \ingroup core
 * \class QgsExpressionContextStack
 * \brief A stack of expression contexts. Essential an expression stack is
 * an ordered list of expressions contexts, where contexts later in the
 * stack override earlier contexts. Stacks can be used to combine multiple
 * QgsExpressionContexts, eg to combine the global expression context
 * with a project's context, with a particular render operation's context.
 * \note added in QGIS 2.9
 */

class CORE_EXPORT QgsExpressionContextStack : public QgsExpressionContext
{
  public:

    QgsExpressionContextStack() : QgsExpressionContext() {}

    virtual ~QgsExpressionContextStack();

    /** Appends a context to the end of the stack. This context will override
     * any matching variables or functions provided by existing members of the
     * stack. Ownership of the context is NOT transferred to the stack.
     * @param context expression context to append to stack
     */
    void appendContext( QgsExpressionContext* context );

    /** Checks whether a stack includes any contexts which specify
     * a matching variable.
     * @param name variable name
     * @returns true if stack includes the specified variable
     * @see variable
     * @see variableNames
     */
    virtual bool hasVariable( const QString& name ) const override;

    /** Fetches a matching variable from the stack. Contexts later in
     * the stack will take precedence (ie, the stack is searched from
     * the last added context through to the first and returns the
     * first matching variable found).
     * @param name variable name
     * @returns matching variable value from the stack, or an invalid
     * QVariant if variable was not found
     * @see hasVariable
     * @see variableNames
     */
    virtual QVariant variable( const QString& name ) const override;

    /** Returns the currently active context from the stack for a specified variable.
     * As contexts later in the stack override earlier contexts, this will be
     * the last matching context which contains the variable.
     * @param name variable name
     * @returns matching context containing variable, or null if none found
     */
    QgsExpressionContext* activeContextForVariable( const QString& name ) const;

    /** Returns a list of unique variable names specified by contexts
     * within the stack.
     * @see hasVariable
     * @see variable
     */
    virtual QStringList variableNames() const override;

    /** Checks whether a stack includes any contexts which specify
     * a matching function.
     * @param name function name
     * @returns true if stack includes the specified function
     * @see function
     */
    virtual bool hasFunction( const QString& name ) const override;

    /** Fetches a matching function from the stack. Contexts later in
     * the stack will take precedence (ie, the stack is searched from
     * the last added context through to the first and returns the
     * first matching function found).
     * @param name function name
     * @returns function if contained by the stack, otherwise null
     * @see hasFunction
     */
    virtual QgsExpression::Function* function( const QString& name ) const override;

    /** Returns a reference to the list of contexts stored in the stack
     */
    QList< QgsExpressionContext* >& stack() { return mStack; }

    int childCount() const;

    QgsExpressionContext* getChild( int index );

  private:

    QList< QgsExpressionContext* > mStack;

};


/** \ingroup core
 * \class QgsStaticValueExpressionContext
 * \brief Expression context for simple property/value maps.
 * \note added in QGIS 2.9
 */

class CORE_EXPORT QgsStaticValueExpressionContext : public QgsExpressionContext
{
  public:

    struct StaticVariable
    {
      StaticVariable( const QString& name = QString(), const QVariant& value = QVariant(), bool readOnly = false ) : name( name ), value( value ), readOnly( readOnly ) {}
      QString name;
      QVariant value;
      bool readOnly;
    };

    /** Constructor for QgsStaticValueExpressionContext
     * @param name friendly display name for the context
     */
    QgsStaticValueExpressionContext( const QString& name = QString() ) : QgsExpressionContext( name ) {}

    /** Convenience method for setting a variable in the context. If the variable is already set then
     * its value is overwritten, otherwise a new variable is added to the context.
     * @param name variable name
     * @param value variable value
     * @see insertVariable
     */
    void setVariable( const QString& name, const QVariant& value );

    /** Inserts a variable into the context. If the variable is already set then
     * its value is overwritten, otherwise a new variable is added to the
     * context.
     * @param variable definition of variable to insert
     * @see setVariable
     */
    virtual void insertVariable( const QgsStaticValueExpressionContext::StaticVariable& variable );

    /** Removes a variable from the context, if found.
     * @param name name of variable to remove
     */
    virtual void removeVariable( const QString& name );

    bool hasVariable( const QString& name ) const override;
    QVariant variable( const QString& name ) const override;
    QStringList variableNames() const override;
    virtual bool isReadOnly( const QString& name ) const override;

  protected:

    QMap<QString, StaticVariable> mVariables;

    void registerVariableNames( const QStringList& names )
    {
      mRegisteredNames = names;
    }

  private:

    QStringList mRegisteredNames;

};


/** \ingroup core
 * \class QgsGlobalExpressionContext
 * \brief Expression context for global, user-specified variables. These variables
 * are saved in QGIS settings and are persistent across sessions and projects.
 * \note added in QGIS 2.9
 */

class CORE_EXPORT QgsGlobalExpressionContext : public QgsStaticValueExpressionContext
{
  public:
    static QgsGlobalExpressionContext* instance();

    /** Saves the variables contained in the context to the user's QGIS settings.
     */
    void save();

    QgsGlobalExpressionContext();
    ~QgsGlobalExpressionContext();

};


/** \ingroup core
 * \class QgsProjectExpressionContext
 * \brief Expression context for variables attached to a project. The variables
 * are stored within the project file.
 * \note added in QGIS 2.9
 */

class CORE_EXPORT QgsProjectExpressionContext : public QgsStaticValueExpressionContext
{
  public:

    QgsProjectExpressionContext( QgsProject* project = 0 );

    /** Clears all variables from the project context
     */
    void clear();

    /** Loads variables from the current project to the context
     */
    void load();

    virtual void insertVariable( const QgsStaticValueExpressionContext::StaticVariable& value ) override;

    QVariant variable( const QString& name ) const override;

  private:

    QgsProject* mProject;
};

class CORE_EXPORT QgsFeatureBasedExpressionContext : public QgsExpressionContext
{
  public:

    /**
     * Creates an expression context based on a feature
     * @param feature
     * @param fields
     */
    QgsFeatureBasedExpressionContext( const QgsFeature& feature,
                                      const QgsFields& fields = QgsFields() )
        : QgsExpressionContext()
        , mFeature( feature )
        , mFields( fields )
    {
    }

    bool hasVariable( const QString& name ) const override
    {
      return ( name == "feature" ) ||
             ( name == "fields" && mFields.count() );
    }

    QVariant variable( const QString& name ) const override
    {
      if ( name == "feature" )
      {
        return QVariant::fromValue( mFeature );
      }
      else if ( name == "fields" )
      {
        return QVariant::fromValue( mFields );
      }
      return QVariant();
    }

    QStringList variableNames() const override
    {
      return QStringList() << "feature" << "fields";
    }

  private:

    const QgsFeature mFeature;
    const QgsFields mFields;

};


#endif // QGSEXPRESSIONCONTEXT_H
