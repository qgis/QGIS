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
// QgsAnnotationItemEditContext
//

QgsRectangle QgsAnnotationItemEditContext::currentItemBounds() const
{
  return mCurrentItemBounds;
}

void QgsAnnotationItemEditContext::setCurrentItemBounds( const QgsRectangle &bounds )
{
  mCurrentItemBounds = bounds;
}

QgsRenderContext QgsAnnotationItemEditContext::renderContext() const
{
  return mRenderContext;
}

void QgsAnnotationItemEditContext::setRenderContext( const QgsRenderContext &context )
{
  mRenderContext = context;
}


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
QgsAnnotationItemEditOperationMoveNode::QgsAnnotationItemEditOperationMoveNode( const QString &itemId, QgsVertexId nodeId, const QgsPoint &before, const QgsPoint &after,
    double translatePixelsX, double translatePixelsY )
  : QgsAbstractAnnotationItemEditOperation( itemId )
  , mNodeId( nodeId )
  , mBefore( before )
  , mAfter( after )
  , mTranslatePixelsX( translatePixelsX )
  , mTranslatePixelsY( translatePixelsY )
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

QgsAnnotationItemEditOperationTranslateItem::QgsAnnotationItemEditOperationTranslateItem( const QString &itemId, double translateX, double translateY, double translatePixelsX, double translatePixelsY )
  : QgsAbstractAnnotationItemEditOperation( itemId )
  , mTranslateX( translateX )
  , mTranslateY( translateY )
  , mTranslatePixelsX( translatePixelsX )
  , mTranslatePixelsY( translatePixelsY )
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
