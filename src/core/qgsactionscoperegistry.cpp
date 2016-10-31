/***************************************************************************
  qgsactionscoperegistry.cpp - QgsActionScopeRegistry

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
#include "qgsactionscoperegistry.h"

#include "qgsexpressioncontext.h"

QgsActionScopeRegistry::QgsActionScopeRegistry( QObject* parent )
    : QObject( parent )
{
  // Register some default action scopes:

  QgsExpressionContextScope canvasScope;
  canvasScope.addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "clicked_x" ), 25, true ) );
  canvasScope.addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "clicked_y" ), 30, true ) );
  mActionScopes.insert( QgsActionScope( QStringLiteral( "Canvas" ), tr( "Canvas" ), tr( "Available for the action map tool on the canvas." ), canvasScope ) );

  QgsExpressionContextScope fieldScope;
  fieldScope.addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "current_field" ), QStringLiteral( "[field_value]" ), true ) );
  fieldScope.addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "field_index" ), 0, true ) );
  fieldScope.addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "field_name" ), "[field_name]", true ) );
  fieldScope.addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "field_value" ), "[field_value]", true ) );
  mActionScopes.insert( QgsActionScope( QStringLiteral( "FieldSpecific" ), tr( "Field Specific" ), tr( "Available for individual fields in the attribute table." ), fieldScope ) );

  mActionScopes.insert( QgsActionScope( QStringLiteral( "AttributeTableRow" ), tr( "Attribute Table Row" ), tr( "Available for individual features (rows) in the attribute table." ) ) );
  mActionScopes.insert( QgsActionScope( QStringLiteral( "FeatureForm" ), tr( "Feature Form" ), tr( "Available for individual features in their form." ) ) );

  mActionScopes.insert( QgsActionScope( QStringLiteral( "AttributeTable" ), tr( "Attribute Table" ), tr( "Available as layer global action in the attribute table." ) ) );
}

QSet<QgsActionScope> QgsActionScopeRegistry::actionScopes() const
{
  return mActionScopes;
}

void QgsActionScopeRegistry::registerActionScope( const QgsActionScope& actionScope )
{
  mActionScopes.insert( actionScope );

  emit actionScopesChanged();
}

void QgsActionScopeRegistry::unregisterActionScope( const QgsActionScope& actionScope )
{
  mActionScopes.remove( actionScope );

  emit actionScopesChanged();
}

QgsActionScope QgsActionScopeRegistry::actionScope( const QString& id )
{
  Q_FOREACH ( const QgsActionScope& actionScope, mActionScopes )
  {
    if ( actionScope.id() == id )
    {
      return actionScope;
    }
  }

  return QgsActionScope();
}
