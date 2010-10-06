/***************************************************************************
    globe.h

    Globe Plugin
    a QGIS plugin
     --------------------------------------
    Date                 : 08-Jul-2010
    Copyright            : (C) 2010 by Sourcepole
    Email                : info at sourcepole.ch
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS_GLOBE_PLUGIN_H
#define QGS_GLOBE_PLUGIN_H

#include "../qgisplugin.h"
#include "qgsosgviewer.h"
#include "qgsosgearthtilesource.h"
#include <QObject>
#include <QDockWidget>
#include <osgEarth/MapNode>
#include <osgEarth/MapLayer>
#include <osgEarthUtil/EarthManipulator>

class QAction;
class QToolBar;
class QgisInterface;

class GlobePlugin : public QObject, public QgisPlugin
{
  Q_OBJECT

  public:
    GlobePlugin( QgisInterface* theQgisInterface );
    virtual ~GlobePlugin();

  public slots:
    //! init the gui
    virtual void initGui();
    //! Show the dialog box
    void run();
    //! unload the plugin
    void unload();
    //! show the help document
    void help();

    //!  Called when the main canvas is about to be rendered.
    void renderStarting();
    //!  Called when the main canvas has rendered.
    void renderComplete( QPainter * );
    //! Emitted when a new set of layers has been received
    void layersChanged();
    //! Called when the extents of the map change
    void extentsChanged();

  private:
    int mPluginType;
    //! Pointer to the QGIS interface object
    QgisInterface *mQGisIface;
    //!pointer to the qaction for this plugin
    QAction * mQActionPointer;
    //! OSG Viewer
    QgsOsgViewer viewer;
    //! Dock widget for viewer
    QDockWidget mQDockWidget;
    //! Map node
    osgEarth::MapNode* mMapNode;
    //! QGIS maplayer
    osgEarth::MapLayer* mQgisMapLayer;
    //! Tile source
    osgEarth::Drivers::QgsOsgEarthTileSource* mTileSource;
};


class FlyToExtentHandler : public osgGA::GUIEventHandler 
{
  public:
    FlyToExtentHandler( osgEarthUtil::EarthManipulator* manip, QgisInterface *qGisIface ) : _manip(manip), mQGisIface(qGisIface) { }

    bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa );

  private:
    osg::observer_ptr<osgEarthUtil::EarthManipulator> _manip;

    //! Pointer to the QGIS interface object
    QgisInterface *mQGisIface;
};


#endif // QGS_GLOBE_PLUGIN_H
