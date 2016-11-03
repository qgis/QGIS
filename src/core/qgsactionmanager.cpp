/***************************************************************************
                               qgsactionmanager.cpp

 A class that stores and controls the managment and execution of actions
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
#include <qgslogger.h>
#include "qgsexpression.h"

#include <QList>
#include <QStringList>
#include <QDomElement>
#include <QSettings>
#include <QDesktopServices>
#include <QUrl>
#include <QDir>
#include <QFileInfo>


void QgsActionManager::addAction( QgsAction::ActionType type, const QString& name, const QString& action, bool capture )
{
  addAction( QgsAction( type, name, action, capture ) );
}

void QgsActionManager::addAction( QgsAction::ActionType type, const QString& name, const QString& action, const QString& icon, bool capture )
{
  addAction( QgsAction( type, name, action, icon, capture ) );
}

void QgsActionManager::addAction( const QgsAction& action )
{
  static int actionId = 0;

  mActions.insert( ++actionId, action );
}

void QgsActionManager::doAction( const QString& actionId, const QgsFeature& feature, int defaultValueIndex )
{
  QgsExpressionContext context = createExpressionContext();
  QgsExpressionContextScope* actionScope = new QgsExpressionContextScope();
  actionScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "field_index" ), defaultValueIndex, true ) );
  if ( defaultValueIndex >= 0 && defaultValueIndex < feature.fields().size() )
    actionScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "field_name" ), feature.fields().at( defaultValueIndex ), true ) );
  actionScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "field_value" ), feature.attribute( defaultValueIndex ), true ) );
  context << actionScope;
  doAction( actionId, feature, context );
}

void QgsActionManager::doAction( const QString& actionId, const QgsFeature& feat, const QgsExpressionContext& context )
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

QList<QgsAction> QgsActionManager::listActions( const QString& actionScope ) const
{
  if ( actionScope.isNull() )
    return mActions;
  else
  {
    QList<QgsAction> actions;

    Q_FOREACH ( const QgsAction& action, mActions )
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
  << QgsExpressionContextUtils::projectScope();
  if ( mLayer )
    context << QgsExpressionContextUtils::layerScope( mLayer );

  return context;
}

bool QgsActionManager::writeXml( QDomNode& layer_node, QDomDocument& doc ) const
{
  QDomElement aActions = doc.createElement( QStringLiteral( "attributeactions" ) );
  for ( QVariantMap::const_iterator defaultAction = mDefaultActions.constBegin(); defaultAction != mDefaultActions.constEnd(); ++ defaultAction )
  {
    QDomElement defaultActionElement = doc.createElement( QStringLiteral( "defaultAction" ) );
    defaultActionElement.setAttribute( QStringLiteral( "key" ), defaultAction.key() );
    defaultActionElement.setAttribute( QStringLiteral( "value" ), defaultAction.value().toString() );
    aActions.appendChild( defaultActionElement );
  }

  Q_FOREACH ( const QgsAction& action, mActions )
  {
    QDomElement actionSetting = doc.createElement( QStringLiteral( "actionsetting" ) );
    actionSetting.setAttribute( QStringLiteral( "type" ), action.type() );
    actionSetting.setAttribute( QStringLiteral( "name" ), action.name() );
    actionSetting.setAttribute( QStringLiteral( "shortTitle" ), action.shortTitle() );
    actionSetting.setAttribute( QStringLiteral( "icon" ), action.iconPath() );
    actionSetting.setAttribute( QStringLiteral( "action" ), action.command() );
    actionSetting.setAttribute( QStringLiteral( "capture" ), action.capture() );

    Q_FOREACH ( const QString& scope, action.actionScopes() )
    {
      QDomElement actionScopeElem = doc.createElement( "actionScope" );
      actionScopeElem.setAttribute( "id", scope );
      actionSetting.appendChild( actionScopeElem );
    }
    aActions.appendChild( actionSetting );
  }
  layer_node.appendChild( aActions );

  return true;
}

bool QgsActionManager::readXml( const QDomNode& layer_node )
{
  mActions.clear();

  QDomNode aaNode = layer_node.namedItem( QStringLiteral( "attributeactions" ) );

  if ( !aaNode.isNull() )
  {
    QDomNodeList actionsettings = aaNode.toElement().elementsByTagName( QStringLiteral( "actionsetting" ) );
    for ( int i = 0; i < actionsettings.size(); ++i )
    {
      QDomElement setting = actionsettings.item( i ).toElement();

      QDomNodeList actionScopeNodes = setting.elementsByTagName( "actionScope" );
      QSet<QString> actionScopes;

      if ( actionScopeNodes.isEmpty() )
      {
        actionScopes
        << QStringLiteral( "Canvas" )
        << QStringLiteral( "FieldSpecific" )
        << QStringLiteral( "AttributeTableRow" )
        << QStringLiteral( "FeatureForm" );
      }
      else
      {
        for ( int j = 0; j < actionScopeNodes.length(); ++j )
        {
          QDomElement actionScopeElem = actionScopeNodes.item( j ).toElement();
          actionScopes << actionScopeElem.attribute( "id" );
        }
      }

      mActions.append(
        QgsAction( static_cast< QgsAction::ActionType >( setting.attributeNode( QStringLiteral( "type" ) ).value().toInt() ),
                   setting.attributeNode( QStringLiteral( "name" ) ).value(),
                   setting.attributeNode( QStringLiteral( "action" ) ).value(),
                   setting.attributeNode( QStringLiteral( "icon" ) ).value(),
                   setting.attributeNode( QStringLiteral( "capture" ) ).value().toInt() != 0,
                   setting.attributeNode( QStringLiteral( "shortTitle" ) ).value(),
                   actionScopes
                 )
      );
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

QgsAction QgsActionManager::action( const QString& id )
{
  Q_FOREACH ( const QgsAction& action, mActions )
  {
    if ( action.id() == id )
      return action;
  }

  return QgsAction();
}

void QgsActionManager::setDefaultAction( const QString& actionScope, const QString& actionId )
{
  mDefaultActions[ actionScope ] = actionId;
}

QgsAction QgsActionManager::defaultAction( const QString& actionScope )
{
  return action( mDefaultActions.value( actionScope ).toString() );
}
