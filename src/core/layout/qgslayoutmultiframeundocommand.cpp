/***************************************************************************
                          qgslayoutmultiframeundocommand.cpp
                          ----------------------
    begin                : October 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutmultiframeundocommand.h"
#include "qgslayoutmultiframe.h"
#include "qgsreadwritecontext.h"
#include "qgslayout.h"
#include "qgsproject.h"
#include "qgsfeedback.h"

///@cond PRIVATE
QgsLayoutMultiFrameUndoCommand::QgsLayoutMultiFrameUndoCommand( QgsLayoutMultiFrame *frame, const QString &text, int id, QUndoCommand *parent )
  : QgsAbstractLayoutUndoCommand( text, id, parent )
  , mFrameUuid( frame->uuid() )
  , mLayout( frame->layout() )
  , mItemType( frame->type() )
{

}

bool QgsLayoutMultiFrameUndoCommand::mergeWith( const QUndoCommand *command )
{
  if ( command->id() == 0 )
    return false;

  const QgsLayoutMultiFrameUndoCommand *c = dynamic_cast<const QgsLayoutMultiFrameUndoCommand *>( command );
  if ( !c )
  {
    return false;
  }
  if ( c->multiFrameUuid() != multiFrameUuid() )
    return false;

  setAfterState( c->afterState() );
  return true;
}

void QgsLayoutMultiFrameUndoCommand::saveState( QDomDocument &stateDoc ) const
{
  stateDoc.clear();
  QDomElement documentElement = stateDoc.createElement( QStringLiteral( "ItemState" ) );

  QgsLayoutMultiFrame *item = mLayout->multiFrameByUuid( mFrameUuid );
  Q_ASSERT_X( item, "QgsLayoutMultiFrameUndoCommand::saveState", "could not retrieve item for saving state" );

  item->writeXml( documentElement, stateDoc, QgsReadWriteContext(), true );
  stateDoc.appendChild( documentElement );
}

void QgsLayoutMultiFrameUndoCommand::restoreState( QDomDocument &stateDoc )
{
  // find item by uuid...
  QgsLayoutMultiFrame *item = mLayout->multiFrameByUuid( mFrameUuid );
  if ( !item )
  {
    // uh oh - it's been deleted! we need to create a new instance
    item = recreateItem( mItemType, mLayout );
  }

  item->readXml( stateDoc.documentElement().firstChild().toElement(), stateDoc, QgsReadWriteContext(), true );
  item->finalizeRestoreFromXml();
  mLayout->project()->setDirty( true );
}

QgsLayoutMultiFrame *QgsLayoutMultiFrameUndoCommand::recreateItem( int itemType, QgsLayout *layout )
{
  QgsLayoutMultiFrame *item = QgsApplication::layoutItemRegistry()->createMultiFrame( itemType, layout );
  mLayout->addMultiFrame( item );
  return item;
}

QString QgsLayoutMultiFrameUndoCommand::multiFrameUuid() const
{
  return mFrameUuid;
}

QgsLayout *QgsLayoutMultiFrameUndoCommand::layout() const
{
  return mLayout;
}


//
// QgsLayoutMultiFrameDeleteUndoCommand
//

QgsLayoutMultiFrameDeleteUndoCommand::QgsLayoutMultiFrameDeleteUndoCommand( QgsLayoutMultiFrame *item, const QString &text, int id, QUndoCommand *parent )
  : QgsLayoutMultiFrameUndoCommand( item, text, id, parent )
{
  saveBeforeState();
}

bool QgsLayoutMultiFrameDeleteUndoCommand::mergeWith( const QUndoCommand * )
{
  return false;
}

void QgsLayoutMultiFrameDeleteUndoCommand::redo()
{
  if ( mFirstRun )
  {
    mFirstRun = false;
    return;
  }

  QgsLayoutMultiFrame *item = layout()->multiFrameByUuid( multiFrameUuid() );
  //Q_ASSERT_X( item, "QgsLayoutMultiFrameDeleteUndoCommand::redo", "could not find item to re-delete!" );

  if ( item )
  {
    layout()->removeMultiFrame( item );
    delete item;
  }
}

QgsLayoutMultiFrameAddItemCommand::QgsLayoutMultiFrameAddItemCommand( QgsLayoutMultiFrame *frame, const QString &text, int id, QUndoCommand *parent )
  : QgsLayoutMultiFrameUndoCommand( frame, text, id, parent )
{
  saveAfterState();
}

bool QgsLayoutMultiFrameAddItemCommand::containsChange() const
{
  return true;
}

bool QgsLayoutMultiFrameAddItemCommand::mergeWith( const QUndoCommand * )
{
  return false;
}

void QgsLayoutMultiFrameAddItemCommand::undo()
{
  if ( mFirstRun )
  {
    mFirstRun = false;
    return;
  }

  QgsLayoutMultiFrame *item = layout()->multiFrameByUuid( multiFrameUuid() );
  if ( item )
  {
    layout()->removeMultiFrame( item );
    delete item;
  }
}


///@endcond
