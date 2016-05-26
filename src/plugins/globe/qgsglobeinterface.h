/***************************************************************************
    qgsglobeinterface.h
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

#ifndef QGSGLOBEINTERFACE_H
#define QGSGLOBEINTERFACE_H

#include <QObject>
#include "qgsplugininterface.h"

class GlobePlugin;
class QDateTime;
namespace osgViewer { class Viewer; }
namespace osgEarth { class MapNode; }

class GLOBE_EXPORT QgsGlobeInterface : public QgsPluginInterface
{
  public:
    QgsGlobeInterface( GlobePlugin* const globe, QObject* parent = 0 );

    osgViewer::Viewer* osgViewer();

    osgEarth::MapNode* mapNode();

    void syncExtent();

    void enableFrustumHighlight( bool status );

    void enableFeatureIdentification( bool status );

  private:
    GlobePlugin* mGlobe;
};

#endif // QGSGLOBEINTERFACE_H
