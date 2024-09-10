/***************************************************************************
    qgslabelingenginerule.cpp
    ---------------------
  Date                 : August 2024
  Copyright            : (C) 2024 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslabelingenginerule.h"


//
// QgsLabelingEngineContext
//

QgsLabelingEngineContext::QgsLabelingEngineContext( QgsRenderContext &renderContext )
  : mRenderContext( renderContext )
{

}

QgsGeometry QgsLabelingEngineContext::mapBoundaryGeometry() const
{
  return mMapBoundaryGeometry;
}

void QgsLabelingEngineContext::setMapBoundaryGeometry( const QgsGeometry &geometry )
{
  mMapBoundaryGeometry = geometry;
}

QgsRectangle QgsLabelingEngineContext::extent() const
{
  return mExtent;
}

void QgsLabelingEngineContext::setExtent( const QgsRectangle &extent )
{
  mExtent = extent;
}

//
// QgsAbstractLabelingEngineRule
//

QgsAbstractLabelingEngineRule::~QgsAbstractLabelingEngineRule() = default;

void QgsAbstractLabelingEngineRule::resolveReferences( const QgsProject * )
{

}

bool QgsAbstractLabelingEngineRule::candidatesAreConflicting( const pal::LabelPosition *, const pal::LabelPosition * ) const
{
  return false;
}

QgsRectangle QgsAbstractLabelingEngineRule::modifyCandidateConflictSearchBoundingBox( const QgsRectangle &candidateBounds ) const
{
  return candidateBounds;
}

bool QgsAbstractLabelingEngineRule::candidateIsIllegal( const pal::LabelPosition *, QgsLabelingEngineContext & ) const
{
  return false;
}

void QgsAbstractLabelingEngineRule::alterCandidateCost( pal::LabelPosition *, QgsLabelingEngineContext & ) const
{

}

void QgsAbstractLabelingEngineRule::copyCommonProperties( QgsAbstractLabelingEngineRule *other ) const
{
  other->mName = mName;
  other->mIsActive = mIsActive;
}

bool QgsAbstractLabelingEngineRule::isAvailable() const
{
  return true;
}

QString QgsAbstractLabelingEngineRule::description() const
{
  return mName.isEmpty() ? displayType() : mName;
}

bool QgsAbstractLabelingEngineRule::active() const
{
  return mIsActive;
}

void QgsAbstractLabelingEngineRule::setActive( bool active )
{
  mIsActive = active;
}
