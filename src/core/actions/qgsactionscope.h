/***************************************************************************
  qgsactionscope.h - QgsActionScope

 ---------------------
 begin                : 1.11.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSACTIONSCOPE_H
#define QGSACTIONSCOPE_H

#include "qgis_core.h"
#include <QString>
#include "qgsexpressioncontext.h"

/**
 * \ingroup core
 * \brief An action scope defines a "place" for an action to be shown and may add
 * additional expression variables.
 *
 * Each QgsAction can be available in one or several action scopes.
 *
 * ## Examples
 *
 * ### Canvas
 *
 * Show for canvas tools. Adds `@click_x` and `@click_y` in map coordinates.
 *
 * ### Feature
 *
 * Show in feature specific places like the attribute table or feature form.
 *
 * ### Field
 *
 * Show in context menus for individual fields (e.g. attribute table). Adds `@field_index`, `@field_name` and `@field_value`.
 *
 * ### Layer
 *
 * Show in attribute table and work on the layer or selection.
 */
class CORE_EXPORT QgsActionScope
{
  public:
#ifdef SIP_RUN
    % TypeCode
#include <QHash>
    % End
#endif

    /**
     * Creates a new invalid action scope.
     *
     */
    explicit QgsActionScope();

    /**
     * Creates a new action scope.
     * For details concerning the parameters check the documentation
     * of the corresponding properties.
     */
    explicit QgsActionScope( const QString &id, const QString &title, const QString &description, const QgsExpressionContextScope &expressionContextScope = QgsExpressionContextScope() );

    bool operator==( const QgsActionScope &other ) const;

    /**
     * Returns the expression context scope for the action scope.
     *
     * An expression scope may offer additional variables for an action scope.
     * This can be an `field_name` for the attribute which was clicked or
     * `click_x` and `click_y` for actions which are available as map canvas clicks.
     *
     * \see setExpressionContextScope()
     */
    QgsExpressionContextScope expressionContextScope() const;

    /**
     * Sets the expression context scope for the action scope.
     *
     * An expression scope may offer additional variables for an action scope.
     * This can be an `field_name` for the attribute which was clicked or
     * `click_x` and `click_y` for actions which are available as map canvas clicks.
     *
     * \see expressionContextScope()
     */
    void setExpressionContextScope( const QgsExpressionContextScope &expressionContextScope );

    /**
     * Returns the unique identifier for this action scope.
     *
     * \see setId()
     */
    QString id() const;

    /**
     * Sets the unique \a id for this action scope.
     *
     * \see id()
     */
    void setId( const QString &id );

    /**
     * Returns the action scope's title.
     *
     * The title is a human readable and translated string that will be
     * presented to the user in the properties dialog.
     *
     * \see setTitle()
     */
    QString title() const;

    /**
     * Sets the action scope's \a title.
     *
     * The title should be a human readable and translated string that will be
     * presented to the user in the properties dialog.
     *
     * \see title()
     */
    void setTitle( const QString &title );

    /**
     * Returns the action scope's description.
     *
     * The description should be a longer description of where actions in this scope
     * are available. It is not necessary to list the available expression variables
     * in here, they are extracted automatically from the expressionContextScope().
     *
     * \see setDescription()
     */
    QString description() const;

    /**
     * Sets the action scope's \a description.
     *
     * The description should be a longer description of where actions in this scope
     * are available. It is not necessary to list the available expression variables
     * in here, they are extracted automatically from the expressionContextScope().
     *
     * \see description()
     */
    void setDescription( const QString &description );

    /**
     * Returns TRUE if this scope is valid.
     */
    bool isValid() const;
#ifdef SIP_RUN
    long __hash__();
    % MethodCode
    sipRes = qHash( *sipCpp );
    % End
#endif

  private:
    QString mId;
    QString mTitle;
    QString mDescription;
    QgsExpressionContextScope mExpressionContextScope;
};

CORE_EXPORT uint qHash( const QgsActionScope &key, uint seed = 0 ) SIP_SKIP;

#endif // QGSACTIONSCOPE_H
