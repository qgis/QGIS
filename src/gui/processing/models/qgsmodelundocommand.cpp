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


QgsModelUndoCommand::QgsModelUndoCommand( QgsProcessingModelAlgorithm *model, const QString &text, int id, QUndoCommand *parent )
  : QUndoCommand( text, parent )
  , mModel( model )
  , mId( id )
{
  mBeforeState = model->toVariant();
}

void QgsModelUndoCommand::saveAfterState()
{
  mAfterState = mModel->toVariant();
}

int QgsModelUndoCommand::id() const
{
  return mId;
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
  if ( other->id() == 0 || other->id() != mId )
    return false;

  if ( const QgsModelUndoCommand *c = dynamic_cast<const QgsModelUndoCommand *>( other ) )
  {
    mAfterState = c->mAfterState;
    return true;
  }
  else
  {
    return false;
  }
}

///@endcond
