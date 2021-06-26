/***************************************************************************
                          qgslayoutundocommand.cpp
                          ------------------------
    begin                : July 2017
    copyright            : (C) 2020 by Wang Peng
    email                : nyall dot dawson at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutundocommand.h"


QgsAbstractLayoutUndoCommand::QgsAbstractLayoutUndoCommand( const QString &text, int id, QUndoCommand *parent )
  : QUndoCommand( text, parent )
  , mId( id )
{}

void QgsAbstractLayoutUndoCommand::undo()
{
  QUndoCommand::undo();
  restoreState( mBeforeState );
}

void QgsAbstractLayoutUndoCommand::redo()
{
  if ( mFirstRun )
  {
    mFirstRun = false;
    return;
  }
  QUndoCommand::redo();
  restoreState( mAfterState );
}

void QgsAbstractLayoutUndoCommand::saveBeforeState()
{
  saveState( mBeforeState );
}

void QgsAbstractLayoutUndoCommand::saveAfterState()
{
  saveState( mAfterState );
}

bool QgsAbstractLayoutUndoCommand::containsChange() const
{
  return !( mBeforeState.isNull() || mAfterState.isNull() || mBeforeState.toString() == mAfterState.toString() );
}

void QgsAbstractLayoutUndoCommand::setAfterState( const QDomDocument &stateDoc )
{
  mAfterState = stateDoc;
}
