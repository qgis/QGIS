/***************************************************************************
    qgsglobefeatureidentify.h
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

#ifndef QGSGLOBEFEATUREIDENTIFY_H
#define QGSGLOBEFEATUREIDENTIFY_H

#include <osgEarth/Version>
#include <osgEarthUtil/FeatureQueryTool>

class QgsMapCanvas;
class QgsRubberBand;

#if OSGEARTH_VERSION_LESS_THAN(2, 7, 0)
class QgsGlobeFeatureIdentifyCallback : public osgEarth::Util::FeatureQueryTool::Callback
#else
class QgsGlobeFeatureIdentifyCallback : public osgEarth::Picker::Callback
#endif
{
  public:
    QgsGlobeFeatureIdentifyCallback( QgsMapCanvas* mapCanvas );
    ~QgsGlobeFeatureIdentifyCallback();

#if OSGEARTH_VERSION_LESS_THAN(2, 7, 0)
    void onHit( osgEarth::Features::FeatureSourceIndexNode* index, osgEarth::Features::FeatureID fid, const EventArgs& args ) override;
    void onMiss( const EventArgs &args ) override;
#else
    void onHit( osgEarth::ObjectID id ) override;
    void onMiss() override;
#endif

  private:
    QgsMapCanvas* mCanvas;
    QgsRubberBand* mRubberBand;
};

#endif // QGSGLOBEFEATUREIDENTIFY_H
