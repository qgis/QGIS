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
#include <QObject>
#include <osgEarth/MapNode>
#include <osgEarth/ImageLayer>
#include <osgEarthUtil/EarthManipulator>
#include <osgEarthUtil/Controls>
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

    //! Emitted when a new set of layers has been received
    void layersChanged();
    //! Called when the extents of the map change
    void extentsChanged();
    //! Sync globe extent to mapCanavas
    void syncExtent();

    //! called when a project has been read succesfully
    void projectReady();
    //! called when a new project has been created succesfully
    void blankProjectReady();
    //! called when the globe window is closed
    void setGlobeNotRunning();
    //! set the globe coordinates of a user right-click on the globe
    void setSelectedCoordinates( osg::Vec3d coords );
    //! get a coordinates vector
    osg::Vec3d getSelectedCoordinates();
    //! prints the ccordinates in a QMessageBox
    void showSelectedCoordinates();
    //! emits signal with current mouse coordinates
    void showCurrentCoordinates( double lon, double lat );
    //! get longitude of user right click
    double getSelectedLon();
    //! get latitude of user right click
    double getSelectedLat();
    //! get elevation of user right click
    double getSelectedElevation();


    //! Place an OSG model on the globe
    void placeNode( osg::Node* node, double lat, double lon, double alt = 0.0 );

    //! Recursive copy folder
    static void copyFolder( QString sourceFolder, QString destFolder );

  private:
    //!  Set HTTP proxy settings
    void setupProxy();
    //!  Setup map
    void setupMap();
    //!  Setup map controls
    void setupControls();

  private://! Checks if the globe is open
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
    QDockWidgetGlobe mQDockWidget;
    //! Settings Dialog
    QgsGlobePluginDialog mSettingsDialog;
    //! OSG root node
    osg::Group* mRootNode;
    //! Map node
    osgEarth::MapNode* mMapNode;
    //! QGIS maplayer
    osgEarth::ImageLayer* mQgisMapLayer;
    //! Tile source
    osgEarth::Drivers::QgsOsgEarthTileSource* mTileSource;
    //! Control Canvas
    osgEarth::Util::Controls::ControlCanvas* mControlCanvas;
    //! Elevation manager
    osgEarth::Util::ElevationManager* mElevationManager;
    //! Object placer
    osgEarth::Util::ObjectPlacer* mObjectPlacer;
    //! tracks if the globe is open
    bool mIsGlobeRunning;
    //! coordinates of the right-clicked point on the globe
    double mSelectedLat, mSelectedLon, mSelectedElevation;

  signals:
    //! emits current mouse position
    void xyCoordinates( const QgsPoint & p );
    //! emits position of right click on globe
    void newCoordinatesSelected( const QgsPoint & p );
};

class FlyToExtentHandler : public osgGA::GUIEventHandler
{
  public:
    FlyToExtentHandler( GlobePlugin* globe ) : mGlobe( globe ) { }

    bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa );

  private:
    GlobePlugin* mGlobe;
};

// An event handler that will print out the coordinates at the clicked point
class QueryCoordinatesHandler : public osgGA::GUIEventHandler
{
  public:
    QueryCoordinatesHandler( GlobePlugin* globe, osgEarth::Util::ElevationManager* elevMan,
                             const osgEarth::SpatialReference* mapSRS )
        :  mGlobe( globe ), _elevMan( elevMan ), _mapSRS( mapSRS ), _mouseDown( false ) { }

    bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa );

    virtual osg::Vec3d getCoords( float x, float y, osgViewer::View* view, bool getElevation = false );

  private:
    GlobePlugin* mGlobe;
    osg::ref_ptr<const SpatialReference> _mapSRS;
    osg::ref_ptr<osgEarth::Util::ElevationManager> _elevMan;
    bool _mouseDown;
};


class KeyboardControlHandler : public osgGA::GUIEventHandler
{
  public:
    KeyboardControlHandler( osgEarth::Util::EarthManipulator* manip, QgisInterface *qGisIface ) : _manip( manip ), mQGisIface( qGisIface ) { }

    bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa );

  private:
    osg::observer_ptr<osgEarth::Util::EarthManipulator> _manip;

    //! Pointer to the QGIS interface object
    QgisInterface *mQGisIface;
};


namespace osgEarth
{
  namespace Util
  {
    namespace Controls
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
}

#endif // QGS_GLOBE_PLUGIN_H
