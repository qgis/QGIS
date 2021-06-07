/***************************************************************************
                          qgslayoutitemgroupundocommand.cpp
                          ---------------------------
    begin                : 2016-06-09
    copyright            : (C) 2016 by Sandro Santilli
    email                : strk at kbt dot io
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutitemgroupundocommand.h"
#include "qgslayoutitemgroup.h"
#include "qgslayout.h"
#include "qgsproject.h"

///@cond PRIVATE
QgsLayoutItemGroupUndoCommand::QgsLayoutItemGroupUndoCommand( State s, QgsLayoutItemGroup *group, QgsLayout *layout, const QString &text, QUndoCommand *parent )
  : QUndoCommand( text, parent )
  , mGroupUuid( group->uuid() )
  , mLayout( layout )
  , mState( s )
{
  const QList< QgsLayoutItem * > items = group->items();
  for ( QgsLayoutItem *i : items )
  {
    mItemUuids.insert( i->uuid() );
  }
}

void QgsLayoutItemGroupUndoCommand::redo()
{
  if ( mFirstRun )
  {
    mFirstRun = false;
    return;
  }
  switchState();
}

void QgsLayoutItemGroupUndoCommand::undo()
{
  if ( mFirstRun )
  {
    mFirstRun = false;
    return;
  }
  switchState();
}

void QgsLayoutItemGroupUndoCommand::switchState()
{
  if ( mState == Grouped )
  {
    // ungroup
    QgsLayoutItemGroup *group = dynamic_cast< QgsLayoutItemGroup * >( mLayout->itemByUuid( mGroupUuid ) );
    Q_ASSERT_X( group, "QgsLayoutItemGroupUndoCommand::switchState", "Could not find group" );
    group->removeItems();
    mLayout->removeLayoutItemPrivate( group );
    mState = Ungrouped;
  }
  else //Ungrouped
  {
    // find group by uuid...
    QgsLayoutItemGroup *group = dynamic_cast< QgsLayoutItemGroup * >( mLayout->itemByUuid( mGroupUuid ) );
    if ( !group )
    {
      group = new QgsLayoutItemGroup( mLayout );
      mLayout->addLayoutItemPrivate( group );
    }

    for ( const QString &childUuid : qgis::as_const( mItemUuids ) )
    {
      QgsLayoutItem *childItem = mLayout->itemByUuid( childUuid );
      group->addItem( childItem );
    }

    mState = Grouped;
  }
  mLayout->project()->setDirty( true );
}
///@endcond
