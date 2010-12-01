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
#include "globe_plugin_dialog.h"
#include "Controls"
#include <QObject>
#include <QDockWidget>
#include <osgEarth/MapNode>
#include <osgEarth/MapLayer>
#include <osgEarthUtil/EarthManipulator>
#include <osgEarthUtil/ElevationManager>
#include <osgEarthUtil/ObjectPlacer>

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
    //! Show the settings dialog box
    void settings();
    //! unload the plugin
    void unload();
    //! show the help document
    void help();

    //!  Called when the main canvas is about to be rendered
    void renderStarting();
    //!  Called when the main canvas has rendered.
    void renderComplete( QPainter * );
    //! Emitted when a new set of layers has been received
    void layersChanged();
    //! Called when the extents of the map change
    void extentsChanged();
    //! Sync globe extent to mapCanavas
    void syncExtent();

    //! Place an OSG model on the globe
    void placeNode( osg::Node* node, double lat, double lon, double alt = 0.0 );

    //! Recursive copy folder
    static void copyFolder(QString sourceFolder, QString destFolder);

  private:
    //!  Set HTTP proxy settings
    void setupProxy();
    //!  Setup map
    void setupMap();
    //!  Setup map controls
    void setupControls();

  private:
    int mPluginType;
    //! Pointer to the QGIS interface object
    QgisInterface *mQGisIface;
    //!pointer to the qaction for this plugin
    QAction * mQActionPointer;
    //!pointer to the qaction for this plugin
    QAction * mQActionSettingsPointer;
    //! OSG Viewer
    QgsOsgViewer viewer;
    //! Dock widget for viewer
    QDockWidget mQDockWidget;
    //! Settings Dialog
    QgsGlobePluginDialog mSettingsDialog;
    //! OSG root node
    osg::Group* mRootNode;
    //! Map node
    osgEarth::MapNode* mMapNode;
    //! QGIS maplayer
    osgEarth::MapLayer* mQgisMapLayer;
    //! Tile source
    osgEarth::Drivers::QgsOsgEarthTileSource* mTileSource;
    //! Control Canvas
    osgEarthUtil::Controls2::ControlCanvas* mControlCanvas;
    //! Elevation manager
    osgEarthUtil::ElevationManager* mElevationManager;
    //! Object placer
    osgEarthUtil::ObjectPlacer* mObjectPlacer;
};

class FlyToExtentHandler : public osgGA::GUIEventHandler
{
  public:
    FlyToExtentHandler( GlobePlugin* globe ) : mGlobe ( globe ) { }

    bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa );

  private:
  GlobePlugin* mGlobe;
};


class KeyboardControlHandler : public osgGA::GUIEventHandler
{
  public:
    KeyboardControlHandler( osgEarthUtil::EarthManipulator* manip, QgisInterface *qGisIface ) : _manip(manip), mQGisIface(qGisIface) { }

    bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa );

  private:
    osg::observer_ptr<osgEarthUtil::EarthManipulator> _manip;

    //! Pointer to the QGIS interface object
    QgisInterface *mQGisIface;
};

namespace osgEarthUtil
{
  namespace Controls2
  {
    class NavigationControlHandler : public ControlEventHandler
    {
      public:
        virtual void onMouseDown( class Control* control, int mouseButtonMask ) { }
        virtual void onClick( class Control* control, int mouseButtonMask, const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa ) { }
    };

    class NavigationControl : public ImageControl
    {
      public:
        NavigationControl( osg::Image* image = 0L ) : ImageControl( image ),  _mouse_down_event( NULL ) {}

      protected:
        virtual bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, ControlContext& cx );

      private:
        osg::ref_ptr<const osgGA::GUIEventAdapter> _mouse_down_event;
    };
  }
}

#endif // QGS_GLOBE_PLUGIN_H
