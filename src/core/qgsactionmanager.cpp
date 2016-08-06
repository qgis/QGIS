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
  mActions << QgsAction( type, name, action, capture );
}

void QgsActionManager::addAction( QgsAction::ActionType type, const QString& name, const QString& action, const QString& icon, bool capture )
{
  mActions << QgsAction( type, name, action, icon, capture );
}

void QgsActionManager::addAction( const QgsAction& action )
{
  mActions << action;
}

void QgsActionManager::removeAction( int index )
{
  if ( index >= 0 && index < mActions.size() )
  {
    mActions.removeAt( index );
  }
}

void QgsActionManager::doAction( int index, const QgsFeature& feat, int defaultValueIndex )
{
  QgsExpressionContext context = createExpressionContext();
  QgsExpressionContextScope* actionScope = new QgsExpressionContextScope();
  actionScope->setVariable( "current_field", feat.attribute( defaultValueIndex ) );
  context << actionScope;
  doAction( index, feat, context );
}

void QgsActionManager::doAction( int index, const QgsFeature& feat, const QgsExpressionContext& context )
{
  if ( index < 0 || index >= size() )
    return;

  const QgsAction &action = at( index );
  if ( !action.runable() )
    return;

  QgsExpressionContext actionContext( context );

  if ( mLayer )
    actionContext << QgsExpressionContextUtils::layerScope( mLayer );
  actionContext.setFeature( feat );

  QString expandedAction = QgsExpression::replaceExpressionText( action.action(), &actionContext );
  if ( expandedAction.isEmpty() )
    return;

  QgsAction newAction( action.type(), action.name(), expandedAction, action.capture() );
  runAction( newAction );
}

void QgsActionManager::clearActions()
{
  mActions.clear();
}

QList<QgsAction> QgsActionManager::listActions() const
{
  return mActions;
}

void QgsActionManager::runAction( const QgsAction &action, void ( *executePython )( const QString & ) )
{
  if ( action.type() == QgsAction::OpenUrl )
  {
    QFileInfo finfo( action.action() );
    if ( finfo.exists() && finfo.isFile() )
      QDesktopServices::openUrl( QUrl::fromLocalFile( action.action() ) );
    else
      QDesktopServices::openUrl( QUrl( action.action(), QUrl::TolerantMode ) );
  }
  else if ( action.type() == QgsAction::GenericPython )
  {
    if ( executePython )
    {
      // deprecated
      executePython( action.action() );
    }
    else if ( smPythonExecute )
    {
      // deprecated
      smPythonExecute( action.action() );
    }
    else
    {
      // TODO: capture output from QgsPythonRunner (like QgsRunProcess does)
      QgsPythonRunner::run( action.action() );
    }
  }
  else
  {
    // The QgsRunProcess instance created by this static function
    // deletes itself when no longer needed.
    QgsRunProcess::create( action.action(), action.capture() );
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
  QDomElement aActions = doc.createElement( "attributeactions" );
  aActions.setAttribute( "default", mDefaultAction );

  Q_FOREACH ( const QgsAction& action, mActions )
  {
    QDomElement actionSetting = doc.createElement( "actionsetting" );
    actionSetting.setAttribute( "type", action.type() );
    actionSetting.setAttribute( "name", action.name() );
    actionSetting.setAttribute( "shortTitle", action.shortTitle() );
    actionSetting.setAttribute( "icon", action.iconPath() );
    actionSetting.setAttribute( "action", action.action() );
    actionSetting.setAttribute( "capture", action.capture() );
    actionSetting.setAttribute( "showInAttributeTable", action.showInAttributeTable() );
    aActions.appendChild( actionSetting );
  }
  layer_node.appendChild( aActions );

  return true;
}

bool QgsActionManager::readXml( const QDomNode& layer_node )
{
  mActions.clear();

  QDomNode aaNode = layer_node.namedItem( "attributeactions" );

  if ( !aaNode.isNull() )
  {
    mDefaultAction = aaNode.toElement().attribute( "default", 0 ).toInt();

    QDomNodeList actionsettings = aaNode.childNodes();
    for ( int i = 0; i < actionsettings.size(); ++i )
    {
      QDomElement setting = actionsettings.item( i ).toElement();
      mActions.append(
        QgsAction( static_cast< QgsAction::ActionType >( setting.attributeNode( "type" ).value().toInt() ),
                   setting.attributeNode( "name" ).value(),
                   setting.attributeNode( "action" ).value(),
                   setting.attributeNode( "icon" ).value(),
                   setting.attributeNode( "capture" ).value().toInt() != 0,
                   !setting.attributes().contains( "showInAttributeTable" ) || setting.attributeNode( "showInAttributeTable" ).value().toInt() != 0,
                   setting.attributeNode( "shortTitle" ).value()
                 )
      );
    }
  }
  return true;
}

void ( *QgsActionManager::smPythonExecute )( const QString & ) = nullptr;

void QgsActionManager::setPythonExecute( void ( *runPython )( const QString & ) )
{
  smPythonExecute = runPython;
}
