/***************************************************************************
                              qgstininterpolator.cpp
                              ----------------------
  begin                : March 10, 2008
  copyright            : (C) 2008 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstininterpolator.h"
#include "DualEdgeTriangulation.h"
#include "LinTriangleInterpolator.h"
#include "Point3D.h"
//#include "qgssinglesymbolrenderer.h"
#include "qgsvectorlayer.h"

QgsTINInterpolator::QgsTINInterpolator( const QList<QgsVectorLayer*>& inputData ): QgsInterpolator( inputData ), mTriangulation( 0 ), mTriangleInterpolator( 0 ), mIsInitialized( false )
{
}

QgsTINInterpolator::~QgsTINInterpolator()
{
  delete mTriangulation;
  delete mTriangleInterpolator;
}

int QgsTINInterpolator::interpolatePoint( double x, double y, double& result )
{
  if ( !mIsInitialized )
  {
    initialize();
  }

  if ( !mTriangleInterpolator )
  {
    return 1;
  }

  Point3D r;
  if ( !mTriangleInterpolator->calcPoint( x, y, &r ) )
  {
    return 2;
  }
  result = r.getZ();
  return 0;
}

void QgsTINInterpolator::initialize()
{
  if ( !mDataIsCached )
  {
    cacheBaseData();
  }

  //create DualEdgeTriangulation

  DualEdgeTriangulation* theDualEdgeTriangulation = new DualEdgeTriangulation( mCachedBaseData.size(), 0 );
  mTriangulation = theDualEdgeTriangulation;

  //add all the vertices to the triangulation
  QVector<vertexData>::const_iterator vertex_it = mCachedBaseData.constBegin();
  for ( ; vertex_it != mCachedBaseData.constEnd(); ++vertex_it )
  {
    Point3D* thePoint = new Point3D( vertex_it->x, vertex_it->y, vertex_it->z );
    mTriangulation->addPoint( thePoint );
  }

  mTriangleInterpolator = new LinTriangleInterpolator( theDualEdgeTriangulation );

  mIsInitialized = true;
}
