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
#include "qgsrunprocess.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgslogger.h"
#include "qgsexpression.h"
#include "qgsdataprovider.h"
#include "qgsexpressioncontextutils.h"
#include "qgsaction.h"

#include <QList>
#include <QStringList>
#include <QDomElement>
#include <QSettings>
#include <QDesktopServices>
#include <QUrl>
#include <QFileInfo>
#include <QRegularExpression>

QgsActionManager::QgsActionManager( QgsVectorLayer *layer )
  : mLayer( layer )
{}

QUuid QgsActionManager::addAction( Qgis::AttributeActionType type, const QString &name, const QString &command, bool capture )
{
  QgsAction action( type, name, command, capture );
  addAction( action );
  return action.id();
}

QUuid QgsActionManager::addAction( Qgis::AttributeActionType type, const QString &name, const QString &command, const QString &icon, bool capture )
{
  QgsAction action( type, name, command, icon, capture );
  addAction( action );
  return action.id();
}

void QgsActionManager::addAction( const QgsAction &action )
{
  QgsDebugMsgLevel( "add action " + action.name(), 3 );
  mActions.append( action );
  if ( mLayer && mLayer->dataProvider() && !action.notificationMessage().isEmpty() )
  {
    mLayer->dataProvider()->setListening( true );
    if ( !mOnNotifyConnected )
    {
      QgsDebugMsgLevel( QStringLiteral( "connecting to notify" ), 3 );
      connect( mLayer->dataProvider(), &QgsDataProvider::notify, this, &QgsActionManager::onNotifyRunActions );
      mOnNotifyConnected = true;
    }
  }
}

void QgsActionManager::onNotifyRunActions( const QString &message )
{
  for ( const QgsAction &act : std::as_const( mActions ) )
  {
    if ( !act.notificationMessage().isEmpty() && QRegularExpression( act.notificationMessage() ).match( message ).hasMatch() )
    {
      if ( !act.isValid() || !act.runable() )
        continue;

      QgsExpressionContext context = createExpressionContext();

      Q_ASSERT( mLayer ); // if there is no layer, then where is the notification coming from ?
      context << QgsExpressionContextUtils::layerScope( mLayer );
      context << QgsExpressionContextUtils::notificationScope( message );

      QString expandedAction = QgsExpression::replaceExpressionText( act.command(), &context );
      if ( expandedAction.isEmpty() )
        continue;
      runAction( QgsAction( act.type(), act.name(), expandedAction, act.capture() ) );
    }
  }
}

void QgsActionManager::removeAction( QUuid actionId )
{
  int i = 0;
  for ( const QgsAction &action : std::as_const( mActions ) )
  {
    if ( action.id() == actionId )
    {
      mActions.removeAt( i );
      break;
    }
    ++i;
  }

  if ( mOnNotifyConnected )
  {
    bool hasActionOnNotify = false;
    for ( const QgsAction &action : std::as_const( mActions ) )
      hasActionOnNotify |= !action.notificationMessage().isEmpty();
    if ( !hasActionOnNotify && mLayer && mLayer->dataProvider() )
    {
      // note that there is no way of knowing if the provider is listening only because
      // this class has hasked it to, so we do not reset the provider listening state here
      disconnect( mLayer->dataProvider(), &QgsDataProvider::notify, this, &QgsActionManager::onNotifyRunActions );
      mOnNotifyConnected = false;
    }
  }
}

void QgsActionManager::doAction( QUuid actionId, const QgsFeature &feature, int defaultValueIndex, const QgsExpressionContextScope &scope )
{
  QgsExpressionContext context = createExpressionContext();
  QgsExpressionContextScope *actionScope = new QgsExpressionContextScope( scope );
  actionScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "field_index" ), defaultValueIndex, true ) );
  if ( defaultValueIndex >= 0 && defaultValueIndex < feature.fields().size() )
    actionScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "field_name" ), feature.fields().at( defaultValueIndex ).name(), true ) );
  actionScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "field_value" ), feature.attribute( defaultValueIndex ), true ) );
  context << actionScope;
  doAction( actionId, feature, context );
}

void QgsActionManager::doAction( QUuid actionId, const QgsFeature &feat, const QgsExpressionContext &context )
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
  if ( mOnNotifyConnected && mLayer && mLayer->dataProvider() )
  {
    // note that there is no way of knowing if the provider is listening only because
    // this class has hasked it to, so we do not reset the provider listening state here
    disconnect( mLayer->dataProvider(), &QgsDataProvider::notify, this, &QgsActionManager::onNotifyRunActions );
    mOnNotifyConnected = false;
  }
}

QList<QgsAction> QgsActionManager::actions( const QString &actionScope ) const
{
  if ( actionScope.isNull() )
    return mActions;
  else
  {
    QList<QgsAction> actions;

    for ( const QgsAction &action : std::as_const( mActions ) )
    {
      if ( action.actionScopes().contains( actionScope ) )
        actions.append( action );
    }

    return actions;
  }
}

void QgsActionManager::runAction( const QgsAction &action )
{
  switch ( action.type() )
  {
    case Qgis::AttributeActionType::OpenUrl:
    {
      QFileInfo finfo( action.command() );
      if ( finfo.exists() && finfo.isFile() )
        QDesktopServices::openUrl( QUrl::fromLocalFile( action.command() ) );
      else
        QDesktopServices::openUrl( QUrl( action.command(), QUrl::TolerantMode ) );
      break;
    }
    case Qgis::AttributeActionType::GenericPython:
    case Qgis::AttributeActionType::SubmitUrlEncoded:
    case Qgis::AttributeActionType::SubmitUrlMultipart:
    {
      action.run( QgsExpressionContext() );
      break;
    }
    case Qgis::AttributeActionType::Generic:
    case Qgis::AttributeActionType::Mac:
    case Qgis::AttributeActionType::Unix:
    case Qgis::AttributeActionType::Windows:
    {
      // The QgsRunProcess instance created by this static function
      // deletes itself when no longer needed.
      QgsRunProcess::create( action.command(), action.capture() );
      break;
    }
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

  for ( const QgsAction &action : std::as_const( mActions ) )
  {
    action.writeXml( aActions );
  }
  layer_node.appendChild( aActions );

  return true;
}

bool QgsActionManager::readXml( const QDomNode &layer_node )
{
  clearActions();

  QDomNode aaNode = layer_node.namedItem( QStringLiteral( "attributeactions" ) );

  if ( !aaNode.isNull() )
  {
    QDomNodeList actionsettings = aaNode.toElement().elementsByTagName( QStringLiteral( "actionsetting" ) );
    for ( int i = 0; i < actionsettings.size(); ++i )
    {
      QgsAction action;
      action.readXml( actionsettings.item( i ) );
      addAction( action );
    }

    QDomNodeList defaultActionNodes = aaNode.toElement().elementsByTagName( QStringLiteral( "defaultAction" ) );

    for ( int i = 0; i < defaultActionNodes.size(); ++i )
    {
      QDomElement defaultValueElem = defaultActionNodes.at( i ).toElement();
      mDefaultActions.insert( defaultValueElem.attribute( QStringLiteral( "key" ) ), QUuid( defaultValueElem.attribute( QStringLiteral( "value" ) ) ) );
    }
  }
  return true;
}

QgsAction QgsActionManager::action( QUuid id ) const
{
  for ( const QgsAction &action : std::as_const( mActions ) )
  {
    if ( action.id() == id )
      return action;
  }

  return QgsAction();
}

QgsAction QgsActionManager::action( const QString &id ) const
{
  for ( const QgsAction &action : std::as_const( mActions ) )
  {
    if ( action.id().toString() == id )
      return action;
  }

  return QgsAction();
}

void QgsActionManager::setDefaultAction( const QString &actionScope, QUuid actionId )
{
  mDefaultActions[ actionScope ] = actionId;
}

QgsAction QgsActionManager::defaultAction( const QString &actionScope )
{
  return action( mDefaultActions.value( actionScope ) );
}
