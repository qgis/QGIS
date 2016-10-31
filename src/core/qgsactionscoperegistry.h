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

#include <QObject>
#include <QSet>
#include "qgsactionscope.h"

/** \ingroup core
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
 * @note Added in QGIS 3.0
 */
class CORE_EXPORT QgsActionScopeRegistry : public QObject
{
    Q_OBJECT
    /**
     * The action scopes which are currently registered and available.
     *
     * \read actionScopes()
     * \notify actionScopesChanged()
     *
     * \since QGIS 3.0
     */
    Q_PROPERTY( QSet<QgsActionScope> actionScopes READ actionScopes NOTIFY actionScopesChanged )

  public:
    explicit QgsActionScopeRegistry( QObject* parent = nullptr );

    /**
     * Get all registered action scopes.
     */
    QSet<QgsActionScope> actionScopes() const;

    /**
     * Register an additional action scope.
     *
     * @note Added in QGIS 3.0
     */
    void registerActionScope( const QgsActionScope& actionScope );

    /**
     * Unregister an additional action scope.
     *
     * @note Added in QGIS 3.0
     */
    void unregisterActionScope( const QgsActionScope& actionScope );

    QgsActionScope actionScope( const QString& id );

  signals:
    void actionScopesChanged();

  private:
    QSet<QgsActionScope> mActionScopes;
};

#endif // QGSACTIONSCOPEREGISTRY_H
