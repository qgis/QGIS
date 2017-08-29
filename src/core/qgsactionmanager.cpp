/***************************************************************************
                               qgsactionmanager.cpp

 A class that stores and controls the management and execution of actions
 associated. Actions are defined to be external programs that are run
 with user-specified inputs that can depend on the value of layer
 attributes.

                             -------------------
    begin                : Oct 24 2004
    copyright            : (C) 2004 by Gavin Macaulay
    email                : gavin at macaulay dot co dot nz

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsactionmanager.h"
#include "qgspythonrunner.h"
#include "qgsrunprocess.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgslogger.h"
#include "qgsexpression.h"

#include <QList>
#include <QStringList>
#include <QDomElement>
#include <QSettings>
#include <QDesktopServices>
#include <QUrl>
#include <QDir>
#include <QFileInfo>


QUuid QgsActionManager::addAction( QgsAction::ActionType type, const QString &name, const QString &command, bool capture )
{
  QgsAction action( type, name, command, capture );
  addAction( action );
  return action.id();
}

QUuid QgsActionManager::addAction( QgsAction::ActionType type, const QString &name, const QString &command, const QString &icon, bool capture )
{
  QgsAction action( type, name, command, icon, capture );
  addAction( action );
  return action.id();
}

void QgsActionManager::addAction( const QgsAction &action )
{
  mActions.append( action );
}

void QgsActionManager::removeAction( const QUuid &actionId )
{
  int i = 0;
  Q_FOREACH ( const QgsAction &action, mActions )
  {
    if ( action.id() == actionId )
    {
      mActions.removeAt( i );
      return;
    }
    ++i;
  }
}

void QgsActionManager::doAction( const QUuid &actionId, const QgsFeature &feature, int defaultValueIndex )
{
  QgsExpressionContext context = createExpressionContext();
  QgsExpressionContextScope *actionScope = new QgsExpressionContextScope();
  actionScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "field_index" ), defaultValueIndex, true ) );
  if ( defaultValueIndex >= 0 && defaultValueIndex < feature.fields().size() )
    actionScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "field_name" ), feature.fields().at( defaultValueIndex ).name(), true ) );
  actionScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "field_value" ), feature.attribute( defaultValueIndex ), true ) );
  context << actionScope;
  doAction( actionId, feature, context );
}

void QgsActionManager::doAction( const QUuid &actionId, const QgsFeature &feat, const QgsExpressionContext &context )
{
  QgsAction act = action( actionId );

  if ( !act.isValid() || !act.runable() )
    return;

  QgsExpressionContext actionContext( context );

  if ( mLayer )
    actionContext << QgsExpressionContextUtils::layerScope( mLayer );
  actionContext.setFeature( feat );

  QString expandedAction = QgsExpression::replaceExpressionText( act.command(), &actionContext );
  if ( expandedAction.isEmpty() )
    return;

  QgsAction newAction( act.type(), act.name(), expandedAction, act.capture() );
  runAction( newAction );
}

void QgsActionManager::clearActions()
{
  mActions.clear();
}

QList<QgsAction> QgsActionManager::actions( const QString &actionScope ) const
{
  if ( actionScope.isNull() )
    return mActions;
  else
  {
    QList<QgsAction> actions;

    Q_FOREACH ( const QgsAction &action, mActions )
    {
      if ( action.actionScopes().contains( actionScope ) )
        actions.append( action );
    }

    return actions;
  }
}

void QgsActionManager::runAction( const QgsAction &action )
{
  if ( action.type() == QgsAction::OpenUrl )
  {
    QFileInfo finfo( action.command() );
    if ( finfo.exists() && finfo.isFile() )
      QDesktopServices::openUrl( QUrl::fromLocalFile( action.command() ) );
    else
      QDesktopServices::openUrl( QUrl( action.command(), QUrl::TolerantMode ) );
  }
  else if ( action.type() == QgsAction::GenericPython )
  {
    // TODO: capture output from QgsPythonRunner (like QgsRunProcess does)
    QgsPythonRunner::run( action.command() );
  }
  else
  {
    // The QgsRunProcess instance created by this static function
    // deletes itself when no longer needed.
    QgsRunProcess::create( action.command(), action.capture() );
  }
}

QgsExpressionContext QgsActionManager::createExpressionContext() const
{
  QgsExpressionContext context;
  context << QgsExpressionContextUtils::globalScope()
          << QgsExpressionContextUtils::projectScope( QgsProject::instance() );
  if ( mLayer )
    context << QgsExpressionContextUtils::layerScope( mLayer );

  return context;
}

bool QgsActionManager::writeXml( QDomNode &layer_node ) const
{
  QDomElement aActions = layer_node.ownerDocument().createElement( QStringLiteral( "attributeactions" ) );
  for ( QMap<QString, QUuid>::const_iterator defaultAction = mDefaultActions.constBegin(); defaultAction != mDefaultActions.constEnd(); ++ defaultAction )
  {
    QDomElement defaultActionElement = layer_node.ownerDocument().createElement( QStringLiteral( "defaultAction" ) );
    defaultActionElement.setAttribute( QStringLiteral( "key" ), defaultAction.key() );
    defaultActionElement.setAttribute( QStringLiteral( "value" ), defaultAction.value().toString() );
    aActions.appendChild( defaultActionElement );
  }

  Q_FOREACH ( const QgsAction &action, mActions )
  {
    action.writeXml( aActions );
  }
  layer_node.appendChild( aActions );

  return true;
}

bool QgsActionManager::readXml( const QDomNode &layer_node )
{
  mActions.clear();

  QDomNode aaNode = layer_node.namedItem( QStringLiteral( "attributeactions" ) );

  if ( !aaNode.isNull() )
  {
    QDomNodeList actionsettings = aaNode.toElement().elementsByTagName( QStringLiteral( "actionsetting" ) );
    for ( int i = 0; i < actionsettings.size(); ++i )
    {
      QgsAction action;
      action.readXml( actionsettings.item( i ) );
      mActions.append( action );
    }

    QDomNodeList defaultActionNodes = aaNode.toElement().elementsByTagName( "defaultAction" );

    for ( int i = 0; i < defaultActionNodes.size(); ++i )
    {
      QDomElement defaultValueElem = defaultActionNodes.at( i ).toElement();
      mDefaultActions.insert( defaultValueElem.attribute( "key" ), defaultValueElem.attribute( "value" ) );
    }
  }
  return true;
}

QgsAction QgsActionManager::action( const QUuid &id )
{
  Q_FOREACH ( const QgsAction &action, mActions )
  {
    if ( action.id() == id )
      return action;
  }

  return QgsAction();
}

void QgsActionManager::setDefaultAction( const QString &actionScope, const QUuid &actionId )
{
  mDefaultActions[ actionScope ] = actionId;
}

QgsAction QgsActionManager::defaultAction( const QString &actionScope )
{
  return action( mDefaultActions.value( actionScope ) );
}
