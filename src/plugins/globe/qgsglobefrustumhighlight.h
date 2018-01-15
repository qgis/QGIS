/***************************************************************************
    qgsglobefrustumhighlight.h
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

#ifndef QGSGLOBEFRUSTUMHIGHLIGHT_H
#define QGSGLOBEFRUSTUMHIGHLIGHT_H

#include <osg/Callback>

class QgsRubberBand;
namespace osg { class View; }
namespace osgEarth
{
  class Terrain;
  class SpatialReference;
}

struct QgsGlobeFrustumHighlightCallback : public osg::Callback
{
  public:
    QgsGlobeFrustumHighlightCallback( osg::View *view, osgEarth::Terrain *terrain, QgsMapCanvas *mapCanvas, QColor color );
    ~QgsGlobeFrustumHighlightCallback();

    bool run( osg::Object *object, osg::Object *data ) override;

  private:
    osg::View *mView = nullptr;
    osgEarth::Terrain *mTerrain = nullptr;
    QgsRubberBand *mRubberBand = nullptr;
    osgEarth::SpatialReference *mSrs = nullptr;
};

#endif // QGSGLOBEFRUSTUMHIGHLIGHT_H
