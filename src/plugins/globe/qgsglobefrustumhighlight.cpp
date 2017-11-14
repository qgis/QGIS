/***************************************************************************
    qgsglobefrustumhighlight.cpp
     --------------------------------------
    Date                 : 27.10.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qgsrubberband.h>
#include <qgsmapcanvas.h>
#include <osg/View>
#include <osgEarth/SpatialReference>
#include <osgEarth/Terrain>

#include "qgsglobefrustumhighlight.h"

QgsGlobeFrustumHighlightCallback::QgsGlobeFrustumHighlightCallback( osg::View *view, osgEarth::Terrain *terrain, QgsMapCanvas *mapCanvas, QColor color )
  : osg::Callback()
  , mView( view )
  , mTerrain( terrain )
  , mRubberBand( new QgsRubberBand( mapCanvas, QgsWkbTypes::PolygonGeometry ) )
  , mSrs( osgEarth::SpatialReference::create( mapCanvas->mapSettings().destinationCrs().toWkt().toStdString() ) )
{
  mRubberBand->setColor( color );
}

QgsGlobeFrustumHighlightCallback::~QgsGlobeFrustumHighlightCallback()
{
  delete mRubberBand;
}

bool QgsGlobeFrustumHighlightCallback::run( osg::Object *object, osg::Object *data )
{
  osg::Node *node = dynamic_cast<osg::Node *>( object );
  osg::NodeVisitor *nv = dynamic_cast<osg::NodeVisitor *>( data );
  if ( node && nv )
  {
    const osg::Viewport::value_type &width = mView->getCamera()->getViewport()->width();
    const osg::Viewport::value_type &height = mView->getCamera()->getViewport()->height();

    osg::Vec3d corners[4];

    mTerrain->getWorldCoordsUnderMouse( mView, 0,         0,          corners[0] );
    mTerrain->getWorldCoordsUnderMouse( mView, 0,         height - 1, corners[1] );
    mTerrain->getWorldCoordsUnderMouse( mView, width - 1, height - 1, corners[2] );
    mTerrain->getWorldCoordsUnderMouse( mView, width - 1, 0,          corners[3] );

    mRubberBand->reset( QgsWkbTypes::PolygonGeometry );
    for ( int i = 0; i < 4; i++ )
    {
      osg::Vec3d localCoords;
      mSrs->transformFromWorld( corners[i], localCoords );
      mRubberBand->addPoint( QgsPointXY( localCoords.x(), localCoords.y() ) );
    }
    return true;
  }
  else
  {
    return traverse( object, data );
  }
}
