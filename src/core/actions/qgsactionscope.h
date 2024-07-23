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
 * Examples:
 * ---------
 *
 * <dl>
 *   <dt>Canvas</dt>
 *   <dd>Show for canvas tools. Adds `@click_x` and `@click_y` in map coordinates.</dd>
 *   <dt>Feature</dt>
 *   <dd>Show in feature specific places like the attribute table or feature
 *   form.</dd>
 *   <dt>Field</dt>
 *   <dd>Show in context menus for individual fields (e.g. attribute table). Adds `@field_index`, `@field_name` and `@field_value`.</dd>
 *   <dt>Layer</dt>
 *   <dd>Show in attribute table and work on the layer or selection.</dd>
 * </dl>
 *
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
     * An expression scope may offer additional variables for an action scope.
     * This can be an `field_name` for the attribute which was clicked or
     * `click_x` and `click_y` for actions which are available as map canvas clicks.
     *
     */
    QgsExpressionContextScope expressionContextScope() const;

    /**
     * \copydoc expressionContextScope()
     */
    void setExpressionContextScope( const QgsExpressionContextScope &expressionContextScope );

    /**
     * A unique identifier for this action scope.
     *
     */
    QString id() const;

    //! \copydoc id()
    void setId( const QString &id );

    /**
     * The title is a human readable and translated string that will be
     * presented to the user in the properties dialog.
     *
     */
    QString title() const;
    //! \copydoc title()
    void setTitle( const QString &title );

    /**
     * The description should be a longer description of where actions in this scope
     * are available. It is not necessary to list the available expression variables
     * in here, they are extracted automatically from the expressionContextScope().
     *
     */
    QString description() const;
    //! \copydoc description()
    void setDescription( const QString &description );

    /**
     * Returns if this scope is valid.
     *
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
