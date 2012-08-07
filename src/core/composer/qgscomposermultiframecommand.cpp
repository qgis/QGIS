/***************************************************************************
                          qgscomposermultiframecommand.cpp
                          --------------------------------
    begin                : 2012-08-02
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscomposermultiframecommand.h"
#include "qgscomposermultiframe.h"

QgsComposerMultiFrameCommand::QgsComposerMultiFrameCommand( QgsComposerMultiFrame* multiFrame, const QString& text, QUndoCommand* parent ):
    QUndoCommand( text, parent ), mMultiFrame( multiFrame ), mFirstRun( false )
{
}

QgsComposerMultiFrameCommand::~QgsComposerMultiFrameCommand()
{
}

void QgsComposerMultiFrameCommand::undo()
{
  restoreState( mPreviousState );
}

void QgsComposerMultiFrameCommand::redo()
{
  if ( !checkFirstRun() )
  {
    return;
  }
  restoreState( mAfterState );
}

void QgsComposerMultiFrameCommand::savePreviousState()
{
  if ( mMultiFrame )
  {
    saveState( mPreviousState );
  }
}

void QgsComposerMultiFrameCommand::saveAfterState()
{
  if ( mMultiFrame )
  {
    saveState( mAfterState );
  }
}

void QgsComposerMultiFrameCommand::saveState( QDomDocument& stateDoc )
{
  if ( mMultiFrame )
  {
    stateDoc.clear();
    QDomElement documentElement = stateDoc.createElement( "ComposerMultiFrameState" );
    mMultiFrame->writeXML( documentElement, stateDoc );
    stateDoc.appendChild( documentElement );
  }
}

void QgsComposerMultiFrameCommand::restoreState( QDomDocument& stateDoc )
{
  if ( mMultiFrame )
  {
    mMultiFrame->readXML( stateDoc.documentElement().firstChild().toElement(), stateDoc );
  }
}

bool QgsComposerMultiFrameCommand::checkFirstRun()
{
  if ( !mFirstRun )
  {
    false;
  }
  mFirstRun = false;
  return true;
}

bool QgsComposerMultiFrameCommand::containsChange() const
{
  return !( mPreviousState.isNull() || mAfterState.isNull() || mPreviousState.toString() == mAfterState.toString() );
}
