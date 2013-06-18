/***************************************************************************
                          qgsaddremovemultiframecommand.cpp
                          ---------------------------------
    begin                : 2012-07-31
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

#include "qgsaddremovemultiframecommand.h"
#include "qgscomposermultiframe.h"
#include "qgscomposition.h"
#include "qgsproject.h"


QgsAddRemoveMultiFrameCommand::QgsAddRemoveMultiFrameCommand( State s, QgsComposerMultiFrame* multiFrame, QgsComposition* c, const QString& text, QUndoCommand* parent ):
    QUndoCommand( text, parent ), mMultiFrame( multiFrame ), mComposition( c ), mState( s ), mFirstRun( true )
{
}

QgsAddRemoveMultiFrameCommand::QgsAddRemoveMultiFrameCommand(): mMultiFrame( 0 ), mComposition( 0 )
{
}

QgsAddRemoveMultiFrameCommand::~QgsAddRemoveMultiFrameCommand()
{
  if ( mState == Removed )
  {
    delete mMultiFrame;
  }
}

void QgsAddRemoveMultiFrameCommand::redo()
{
  if ( checkFirstRun() )
  {
    return;
  }
  switchState();
}

void QgsAddRemoveMultiFrameCommand::undo()
{
  if ( checkFirstRun() )
  {
    return;
  }
  switchState();
}

void QgsAddRemoveMultiFrameCommand::switchState()
{
  if ( mComposition )
  {
    if ( mState == Added )
    {
      mComposition->removeMultiFrame( mMultiFrame );
      mState = Removed;
    }
    else
    {
      mComposition->addMultiFrame( mMultiFrame );
      mState = Added;
    }
    QgsProject::instance()->dirty( true );
  }
}

bool QgsAddRemoveMultiFrameCommand::checkFirstRun()
{
  if ( mFirstRun )
  {
    mFirstRun = false;
    return true;
  }
  else
  {
    return false;
  }
}
