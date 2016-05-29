/***************************************************************************
    qgsglobeinterface.cpp
     --------------------------------------
    Date                 : 22.8.2013
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

#include "qgsglobeinterface.h"
#include "globe_plugin.h"

QgsGlobeInterface::QgsGlobeInterface( GlobePlugin* const globe, QObject* parent )
    : QgsPluginInterface( parent )
    , mGlobe( globe )
{
}

osgViewer::Viewer* QgsGlobeInterface::osgViewer()
{
  return mGlobe->osgViewer();
}

osgEarth::MapNode* QgsGlobeInterface::mapNode()
{
  return mGlobe->mapNode();
}

void QgsGlobeInterface::syncExtent()
{
  mGlobe->syncExtent();
}

void QgsGlobeInterface::enableFrustumHighlight( bool status )
{
  mGlobe->enableFrustumHighlight( status );
}

void QgsGlobeInterface::enableFeatureIdentification( bool status )
{
  mGlobe->enableFeatureIdentification( status );
}
