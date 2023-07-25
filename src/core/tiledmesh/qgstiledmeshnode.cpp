/***************************************************************************
                         qgstiledmeshnode.cpp
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

#include "qgstiledmeshnode.h"
#include "qgstiledmeshboundingvolume.h"

QgsTiledMeshNode::QgsTiledMeshNode()
{

}

QgsTiledMeshNode::~QgsTiledMeshNode() = default;

QgsTiledMeshNode::QgsTiledMeshNode( const QgsTiledMeshNode &other )
  : mIsValid( other.mIsValid )
  , mParent( nullptr )
  , mRefinementProcess( other.mRefinementProcess )
  , mTransform( other.mTransform )
  , mContentUri( other.mContentUri )
  , mGeometricError( other.mGeometricError )
{
  mBoundingVolume.reset( other.mBoundingVolume ? other.mBoundingVolume->clone() : nullptr );
}

QgsTiledMeshNode &QgsTiledMeshNode::operator=( const QgsTiledMeshNode &other )
{
  mIsValid = other.mIsValid;
  mParent = other.mParent;
  mRefinementProcess = other.mRefinementProcess;
  mTransform = other.mTransform;
  mContentUri = other.mContentUri;
  mGeometricError = other.mGeometricError;
  for ( const QgsTiledMeshNode *node : other.mChildren )
  {
    QgsTiledMeshNode *newNode = new QgsTiledMeshNode( *node );
    newNode->mParent = this;
    mChildren.append( newNode );
  }
  mBoundingVolume.reset( other.mBoundingVolume ? other.mBoundingVolume->clone() : nullptr );
  return *this;
}

void QgsTiledMeshNode::setRefinementProcess( Qgis::TileRefinementProcess process )
{
  mRefinementProcess = process;
  mIsValid = true;
}

void QgsTiledMeshNode::setBoundingVolume( QgsAbstractTiledMeshNodeBoundingVolume *volume )
{
  mBoundingVolume.reset( volume );
  mIsValid = true;
}

const QgsAbstractTiledMeshNodeBoundingVolume *QgsTiledMeshNode::boundingVolume() const
{
  return mBoundingVolume.get();
}

void QgsTiledMeshNode::setTransform( const QgsMatrix4x4 &transform )
{
  mTransform = transform;
  if ( mBoundingVolume )
    mBoundingVolume->setTransform( transform );
  mIsValid = true;
}

QString QgsTiledMeshNode::contentUri() const
{
  return mContentUri;
}

void QgsTiledMeshNode::setContentUri( const QString &uri )
{
  mContentUri = uri;
  mIsValid = true;
}

void QgsTiledMeshNode::setGeometricError( double error )
{
  mGeometricError = error;
  mIsValid = true;
}
