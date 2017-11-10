/***************************************************************************
  qgsaction.cpp - QgsAction

 ---------------------
 begin                : 18.4.2016
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

#include "qgsaction.h"

#include <QDesktopServices>
#include <QFileInfo>
#include <QUrl>

#include "qgspythonrunner.h"
#include "qgsrunprocess.h"
#include "qgsexpressioncontext.h"
#include "qgsvectorlayer.h"
#include "qgslogger.h"

bool QgsAction::runable() const
{
  return mType == Generic ||
         mType == GenericPython ||
         mType == OpenUrl ||
#if defined(Q_OS_WIN)
         mType == Windows
#elif defined(Q_OS_MAC)
         mType == Mac
#else
         mType == Unix
#endif
         ;
}

void QgsAction::run( QgsVectorLayer *layer, const QgsFeature &feature, const QgsExpressionContext &expressionContext ) const
{
  QgsExpressionContext actionContext( expressionContext );

  actionContext << QgsExpressionContextUtils::layerScope( layer );
  actionContext.setFeature( feature );

  run( actionContext );
}

void QgsAction::run( const QgsExpressionContext &expressionContext ) const
{
  if ( !isValid() )
  {
    QgsDebugMsg( "Invalid action cannot be run" );
    return;
  }

  QgsExpressionContextScope *scope = new QgsExpressionContextScope( mExpressionContextScope );
  QgsExpressionContext context( expressionContext );
  context << scope;

  QString expandedAction = QgsExpression::replaceExpressionText( mCommand, &context );

  if ( mType == QgsAction::OpenUrl )
  {
    QFileInfo finfo( expandedAction );
    if ( finfo.exists() && finfo.isFile() )
      QDesktopServices::openUrl( QUrl::fromLocalFile( expandedAction ) );
    else
      QDesktopServices::openUrl( QUrl( expandedAction, QUrl::TolerantMode ) );
  }
  else if ( mType == QgsAction::GenericPython )
  {
    // TODO: capture output from QgsPythonRunner (like QgsRunProcess does)
    QgsPythonRunner::run( expandedAction );
  }
  else
  {
    // The QgsRunProcess instance created by this static function
    // deletes itself when no longer needed.
    QgsRunProcess::create( expandedAction, mCaptureOutput );
  }
}

QSet<QString> QgsAction::actionScopes() const
{
  return mActionScopes;
}

void QgsAction::setActionScopes( const QSet<QString> &actionScopes )
{
  mActionScopes = actionScopes;
}

void QgsAction::readXml( const QDomNode &actionNode )
{
  QDomElement actionElement = actionNode.toElement();
  QDomNodeList actionScopeNodes = actionElement.elementsByTagName( QStringLiteral( "actionScope" ) );

  if ( actionScopeNodes.isEmpty() )
  {
    mActionScopes
        << QStringLiteral( "Canvas" )
        << QStringLiteral( "Field" )
        << QStringLiteral( "Feature" );
  }
  else
  {
    for ( int j = 0; j < actionScopeNodes.length(); ++j )
    {
      QDomElement actionScopeElem = actionScopeNodes.item( j ).toElement();
      mActionScopes << actionScopeElem.attribute( QStringLiteral( "id" ) );
    }
  }

  mType = static_cast< QgsAction::ActionType >( actionElement.attributeNode( QStringLiteral( "type" ) ).value().toInt() );
  mDescription = actionElement.attributeNode( QStringLiteral( "name" ) ).value();
  mCommand = actionElement.attributeNode( QStringLiteral( "action" ) ).value();
  mIcon = actionElement.attributeNode( QStringLiteral( "icon" ) ).value();
  mCaptureOutput = actionElement.attributeNode( QStringLiteral( "capture" ) ).value().toInt() != 0;
  mShortTitle = actionElement.attributeNode( QStringLiteral( "shortTitle" ) ).value();
  mNotificationMessage = actionElement.attributeNode( QStringLiteral( "notificationMessage" ) ).value();
  mId = QUuid( actionElement.attributeNode( QStringLiteral( "id" ) ).value() );
  if ( mId.isNull() )
    mId = QUuid::createUuid();
}

void QgsAction::writeXml( QDomNode &actionsNode ) const
{
  QDomElement actionSetting = actionsNode.ownerDocument().createElement( QStringLiteral( "actionsetting" ) );
  actionSetting.setAttribute( QStringLiteral( "type" ), mType );
  actionSetting.setAttribute( QStringLiteral( "name" ), mDescription );
  actionSetting.setAttribute( QStringLiteral( "shortTitle" ), mShortTitle );
  actionSetting.setAttribute( QStringLiteral( "icon" ), mIcon );
  actionSetting.setAttribute( QStringLiteral( "action" ), mCommand );
  actionSetting.setAttribute( QStringLiteral( "capture" ), mCaptureOutput );
  actionSetting.setAttribute( QStringLiteral( "notificationMessage" ), mNotificationMessage );
  actionSetting.setAttribute( QStringLiteral( "id" ), mId.toString() );

  Q_FOREACH ( const QString &scope, mActionScopes )
  {
    QDomElement actionScopeElem = actionsNode.ownerDocument().createElement( QStringLiteral( "actionScope" ) );
    actionScopeElem.setAttribute( QStringLiteral( "id" ), scope );
    actionSetting.appendChild( actionScopeElem );
  }

  actionsNode.appendChild( actionSetting );
}

void QgsAction::setExpressionContextScope( const QgsExpressionContextScope &scope )
{
  mExpressionContextScope = scope;
}

QgsExpressionContextScope QgsAction::expressionContextScope() const
{
  return mExpressionContextScope;
};
