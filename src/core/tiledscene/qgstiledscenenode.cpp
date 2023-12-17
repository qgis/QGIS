/***************************************************************************
                         qgstiledscenenode.cpp
                         --------------------
    begin                : June 2023
    copyright            : (C) 2023 by Nyall Dawson
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

#include "qgstiledscenenode.h"
#include "qgstiledscenetile.h"

QgsTiledSceneNode::QgsTiledSceneNode( QgsTiledSceneTile *tile )
  : mTile( tile )
{

}

QgsTiledSceneNode::~QgsTiledSceneNode()
{
  qDeleteAll( mChildren );
  mChildren.clear();
}

QgsTiledSceneTile *QgsTiledSceneNode::tile()
{
  return mTile.get();
}

void QgsTiledSceneNode::addChild( QgsTiledSceneNode *child )
{
  child->mParent = this;
  mChildren.append( child );
}
