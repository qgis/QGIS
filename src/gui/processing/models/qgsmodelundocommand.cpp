/***************************************************************************
                             qgsmodelundocommand.cpp
                             ----------------------------------
    Date                 : March 2020
    Copyright            : (C) 2020 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmodelundocommand.h"

#include "qgsprocessingmodelalgorithm.h"

///@cond NOT_STABLE


QgsModelUndoCommand::QgsModelUndoCommand( QgsProcessingModelAlgorithm *model, const QString &text, CommandOperation id, QUndoCommand *parent )
  : QUndoCommand( text, parent )
  , mModel( model )
  , mOperation( id )
{
  mBeforeState = model->toVariant();
}

QgsModelUndoCommand::QgsModelUndoCommand( QgsProcessingModelAlgorithm *model, const QString &text, const QString &idString, QUndoCommand *parent )
  : QgsModelUndoCommand( model, text, CommandOperation::Unknown, parent )
{
  mIdString = idString;
}

void QgsModelUndoCommand::saveAfterState()
{
  mAfterState = mModel->toVariant();
}

int QgsModelUndoCommand::id() const
{
  // QUndoStack::push() will only try to merge two commands if they have the same ID, and the ID is not -1.
  // so we always return the same (non -1) value here, and implement the actual merge compatibility logic in
  // mergeWith
  return 0;
}

void QgsModelUndoCommand::undo()
{
  QUndoCommand::undo();

  // some settings must live "outside" the undo stack
  const QVariantMap params = mModel->designerParameterValues();

  mModel->loadVariant( mBeforeState );

  mModel->setDesignerParameterValues( params );
}

void QgsModelUndoCommand::redo()
{
  if ( mFirstRun )
  {
    mFirstRun = false;
    return;
  }
  QUndoCommand::redo();

  // some settings must live "outside" the undo stack
  const QVariantMap params = mModel->designerParameterValues();

  mModel->loadVariant( mAfterState );

  mModel->setDesignerParameterValues( params );
}

bool QgsModelUndoCommand::mergeWith( const QUndoCommand *other )
{
  if ( const QgsModelUndoCommand *c = dynamic_cast<const QgsModelUndoCommand *>( other ) )
  {
    bool canMerge = false;
    if ( !c->idString().isEmpty() && c->idString() == mIdString )
    {
      canMerge = true;
    }
    else if ( c->operation() != CommandOperation::Unknown && c->operation() == mOperation )
    {
      canMerge = true;
    }

    if ( !canMerge )
      return false;

    mAfterState = c->mAfterState;
    return true;
  }
  else
  {
    return false;
  }
}

///@endcond
