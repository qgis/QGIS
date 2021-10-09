/***************************************************************************
    qgsannotationitemeditoperation.cpp
    ----------------
    copyright            : (C) 2021 by Nyall Dawson
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

#include "qgsannotationitemeditoperation.h"

//
// QgsAbstractAnnotationItemEditOperation
//
QgsAbstractAnnotationItemEditOperation::QgsAbstractAnnotationItemEditOperation( const QString &itemId )
  : mItemId( itemId )
{

}

QgsAbstractAnnotationItemEditOperation::~QgsAbstractAnnotationItemEditOperation() = default;


//
// QgsAnnotationItemEditOperationMoveNode
//
QgsAnnotationItemEditOperationMoveNode::QgsAnnotationItemEditOperationMoveNode( const QString &itemId, QgsVertexId nodeId, const QgsPoint &before, const QgsPoint &after )
  : QgsAbstractAnnotationItemEditOperation( itemId )
  , mNodeId( nodeId )
  , mBefore( before )
  , mAfter( after )
{

}

QgsAbstractAnnotationItemEditOperation::Type QgsAnnotationItemEditOperationMoveNode::type() const
{
  return Type::MoveNode;
}


//
// QgsAnnotationItemEditOperationDeleteNode
//

QgsAnnotationItemEditOperationDeleteNode::QgsAnnotationItemEditOperationDeleteNode( const QString &itemId, QgsVertexId nodeId, const QgsPoint &before )
  : QgsAbstractAnnotationItemEditOperation( itemId )
  , mNodeId( nodeId )
  , mBefore( before )
{

}

QgsAbstractAnnotationItemEditOperation::Type QgsAnnotationItemEditOperationDeleteNode::type() const
{
  return Type::DeleteNode;
}

//
// QgsAnnotationItemEditOperationTranslateItem
//

QgsAnnotationItemEditOperationTranslateItem::QgsAnnotationItemEditOperationTranslateItem( const QString &itemId, double translateX, double translateY )
  : QgsAbstractAnnotationItemEditOperation( itemId )
  , mTranslateX( translateX )
  , mTranslateY( translateY )
{

}

QgsAbstractAnnotationItemEditOperation::Type QgsAnnotationItemEditOperationTranslateItem::type() const
{
  return Type::TranslateItem;
}


//
// QgsAnnotationItemEditOperationAddNode
//

QgsAnnotationItemEditOperationAddNode::QgsAnnotationItemEditOperationAddNode( const QString &itemId, const QgsPoint &point )
  : QgsAbstractAnnotationItemEditOperation( itemId )
  , mPoint( point )
{

}

QgsAbstractAnnotationItemEditOperation::Type QgsAnnotationItemEditOperationAddNode::type() const
{
  return Type::AddNode;
}
