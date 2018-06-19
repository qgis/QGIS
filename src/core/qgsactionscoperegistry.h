/***************************************************************************
  qgsactionscoperegistry.h - QgsActionScopeRegistry

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
#ifndef QGSACTIONSCOPEREGISTRY_H
#define QGSACTIONSCOPEREGISTRY_H

#include "qgis_core.h"
#include <QObject>
#include <QSet>
#include "qgsactionscope.h"

/**
 * \ingroup core
 * The action scope registry is an application wide registry that
 * contains a list of available action scopes.
 * Some scopes are available by default, additional ones can be registered
 * at runtime by plugins or custom applications.
 *
 * To get access use the QgsApplication:
 *
 * ```
 * QgsApplication::actionScopeRegistry()
 * ```
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsActionScopeRegistry : public QObject
{
    Q_OBJECT
    Q_PROPERTY( QSet<QgsActionScope> actionScopes READ actionScopes NOTIFY actionScopesChanged )

  public:

    /**
     * Create a new QgsActionScopeRegistry.
     * QGIS already creates a central registry. You will normally
     * want to use QgsApplication::actionScopeRegistry() to get access
     * to that one instead.
     *
     * \since QGIS 3.0
     */
    explicit QgsActionScopeRegistry( QObject *parent = nullptr );

    /**
     * Gets all registered action scopes.
     *
     * \since QGIS 3.0
     */
    QSet<QgsActionScope> actionScopes() const;

    /**
     * Register an additional action scope.
     *
     * \since QGIS 3.0
     */
    void registerActionScope( const QgsActionScope &actionScope );

    /**
     * Unregister an additional action scope.
     *
     * \since QGIS 3.0
     */
    void unregisterActionScope( const QgsActionScope &actionScope );

    /**
     * Gets an action scope by its id.
     *
     * \since QGIS 3.0
     */
    QgsActionScope actionScope( const QString &id );

  signals:

    /**
     * Emitted whenever a new action scope is registered or an action scope
     * is unregistered.
     *
     * \since QGIS 3.0
     */
    void actionScopesChanged();

  private:
    QSet<QgsActionScope> mActionScopes;
};

#endif // QGSACTIONSCOPEREGISTRY_H
