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

#include "moc_qgsactionscoperegistry.cpp"

QgsActionScopeRegistry::QgsActionScopeRegistry( QObject *parent )
  : QObject( parent )
{
  // Register some default action scopes:

  QgsExpressionContextScope canvasScope;
  canvasScope.addVariable( QgsExpressionContextScope::StaticVariable( u"click_x"_s, 25, true ) );
  canvasScope.addVariable( QgsExpressionContextScope::StaticVariable( u"click_y"_s, 30, true ) );
  mActionScopes.insert( QgsActionScope( u"Canvas"_s, tr( "Canvas" ), tr( "Available for the action map tool on the canvas." ), canvasScope ) );

  QgsExpressionContextScope fieldScope;
  fieldScope.addVariable( QgsExpressionContextScope::StaticVariable( u"field_index"_s, 0, true ) );
  fieldScope.addVariable( QgsExpressionContextScope::StaticVariable( u"field_name"_s, "[field_name]", true ) );
  fieldScope.addVariable( QgsExpressionContextScope::StaticVariable( u"field_value"_s, "[field_value]", true ) );

  mActionScopes.insert( QgsActionScope( u"Field"_s, tr( "Field" ), tr( "Available for individual fields. For example in the attribute table." ), fieldScope ) );
  mActionScopes.insert( QgsActionScope( u"Feature"_s, tr( "Feature" ), tr( "Available for individual features. For example on feature forms or per row in the attribute table." ) ) );
  mActionScopes.insert( QgsActionScope( u"Layer"_s, tr( "Layer" ), tr( "Available as layer global action. For example on top of the attribute table." ) ) );
  mActionScopes.insert( QgsActionScope( u"Form"_s, tr( "Form" ), tr( "Available only when connected to a form action button in a drag and drop attribute form." ) ) );
}

QSet<QgsActionScope> QgsActionScopeRegistry::actionScopes() const
{
  return mActionScopes;
}

void QgsActionScopeRegistry::registerActionScope( const QgsActionScope &actionScope )
{
  mActionScopes.insert( actionScope );

  emit actionScopesChanged();
}

void QgsActionScopeRegistry::unregisterActionScope( const QgsActionScope &actionScope )
{
  mActionScopes.remove( actionScope );

  emit actionScopesChanged();
}

QgsActionScope QgsActionScopeRegistry::actionScope( const QString &id )
{
  const auto constMActionScopes = mActionScopes;
  for ( const QgsActionScope &actionScope : constMActionScopes )
  {
    if ( actionScope.id() == id )
    {
      return actionScope;
    }
  }

  return QgsActionScope();
}
