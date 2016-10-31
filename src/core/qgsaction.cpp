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

void QgsAction::run( QgsVectorLayer* layer, const QgsFeature& feature, const QgsExpressionContext& expressionContext ) const
{
  QgsExpressionContext actionContext( expressionContext );

  actionContext << QgsExpressionContextUtils::layerScope( layer );
  actionContext.setFeature( feature );

  run( actionContext );
}

void QgsAction::run( const QgsExpressionContext& expressionContext ) const
{
  if ( !isValid() )
  {
    QgsDebugMsg( "Invalid action cannot be run" );
    return;
  }

  QString expandedAction = QgsExpression::replaceExpressionText( mCommand, &expressionContext );

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

void QgsAction::setActionScopes( const QSet<QString>& actionScopes )
{
  mActionScopes = actionScopes;
}
