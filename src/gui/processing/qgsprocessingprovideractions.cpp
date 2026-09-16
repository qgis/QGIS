/***************************************************************************
                             qgsprocessingprovideractions.cpp
                             ----------------------------------
    Date                 : September 2026
    Copyright            : (C) 2026 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingprovideractions.h"

#include "qgsapplication.h"
#include "qgsdockwidget.h"

#include <QIcon>

//
// QgsProcessingActionContext
//

QWidget *QgsProcessingActionContext::parentWidget() const
{
  return mParentWidget;
}

void QgsProcessingActionContext::setParentWidget( QWidget *newParentWidget )
{
  mParentWidget = newParentWidget;
}

//
// QgsProcessingToolboxAction
//

QgsProcessingToolboxAction::QgsProcessingToolboxAction( const QString &name, const QString &group )
  : mName( name )
  , mGroup( group )
{}

QgsProcessingToolboxAction::~QgsProcessingToolboxAction() = default;

QIcon QgsProcessingToolboxAction::icon() const
{
  // either this or getIcon() may be called, so transparently fall back
  // to the other method which may be implemented, but avoid endless loops
  if ( mUseDeprecatedLoopBreak )
    return QgsApplication::getThemeIcon( "/processingAlgorithm.svg" );

  mUseDeprecatedLoopBreak = true;
  Q_NOWARN_DEPRECATED_PUSH
  const QIcon res = getIcon();
  mUseDeprecatedLoopBreak = false;
  Q_NOWARN_DEPRECATED_POP
  return res;
}

QIcon QgsProcessingToolboxAction::getIcon() const
{
  // either this or getIcon() may be called, so transparently fall back
  // to the other method which may be implemented, but avoid endless loops
  if ( mUseDeprecatedLoopBreak )
    return QgsApplication::getThemeIcon( "/processingAlgorithm.svg" );

  mUseDeprecatedLoopBreak = true;
  const QIcon res = icon();
  mUseDeprecatedLoopBreak = false;
  return res;
}

void QgsProcessingToolboxAction::setData( QgsDockWidget *widget )
{
  mToolboxDockWidget = widget;
}

void QgsProcessingToolboxAction::trigger( const QgsProcessingActionContext & )
{
  if ( mUseDeprecatedLoopBreak )
    return;

  mUseDeprecatedLoopBreak = true;
  Q_NOWARN_DEPRECATED_PUSH
  execute();
  Q_NOWARN_DEPRECATED_POP
  mUseDeprecatedLoopBreak = false;
}

void QgsProcessingToolboxAction::execute()
{
  if ( mUseDeprecatedLoopBreak )
    return;

  mUseDeprecatedLoopBreak = true;
  trigger( QgsProcessingActionContext() );
  mUseDeprecatedLoopBreak = false;
}

QgsDockWidget *QgsProcessingToolboxAction::data() const
{
  return mToolboxDockWidget;
}
