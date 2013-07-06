/***************************************************************************
    globe.h

    Globe Plugin
    a QGIS plugin
     --------------------------------------
    Date                 : 08-Jul-2010
    Copyright            : (C) 2010 by Sourcepole
    Email                : info at sourcepole.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS_GLOBE_PLUGIN_H
#define QGS_GLOBE_PLUGIN_H

#include "qgsconfig.h"
#include "qgisplugin.h"
#include "qgsosgearthtilesource.h"
#include "globe_plugin_dialog.h"
#include <QObject>
#include <osgViewer/Viewer>
#include <osgEarth/MapNode>
#include <osgEarth/ImageLayer>
#include <osgEarthUtil/EarthManipulator>
#ifndef HAVE_OSGEARTHQT //use backported controls if osgEarth <= 2.1
#define USE_BACKPORTED_CONTROLS
#endif
#ifdef USE_BACKPORTED_CONTROLS
#include "osgEarthUtil/Controls"
using namespace osgEarth::Util::Controls21;
#else
#include <osgEarthUtil/Controls>
using namespace osgEarth::Util::Controls;
#endif
#ifdef HAVE_OSGEARTH_ELEVATION_QUERY
#include <osgEarth/ElevationQuery>
#include <osgEarthUtil/ObjectLocator>
#else
#include <osgEarthUtil/ElevationManager>
#include <osgEarthUtil/ObjectPlacer>
#endif

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
    //!  Reset globe
    void reset();
    //! unload the plugin
    void unload();
    //! show the help document
    void help();

    //! Emitted when a new set of image layers has been received
    void imageLayersChanged();
    //! Emitted when a new set of elevation layers has been received
    void elevationLayersChanged();
    //! Called when the extents of the map change
    void extentsChanged();
    //! Sync globe extent to mapCanavas
    void syncExtent();

    //! called when a project has been read successfully
    void projectReady();
    //! called when a new project has been created successfully
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
    osgViewer::Viewer* mOsgViewer;
    //! QT viewer widget
    QWidget* mViewerWidget;
    //! Settings Dialog
    QgsGlobePluginDialog *mSettingsDialog;
    //! OSG root node
    osg::Group* mRootNode;
    //! Map node
    osgEarth::MapNode* mMapNode;
    //! QGIS maplayer
    osgEarth::ImageLayer* mQgisMapLayer;
    //! Tile source
    osgEarth::Drivers::QgsOsgEarthTileSource* mTileSource;
    //! Control Canvas
    ControlCanvas* mControlCanvas;
#ifdef HAVE_OSGEARTH_ELEVATION_QUERY
    //! Elevation manager
    osgEarth::ElevationQuery* mElevationManager;
    //! Object placer
    osgEarth::Util::ObjectLocator* mObjectPlacer;
#else
    //! Elevation manager
    osgEarth::Util::ElevationManager* mElevationManager;
    //! Object placer
    osgEarth::Util::ObjectPlacer* mObjectPlacer;
#endif
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
#ifdef HAVE_OSGEARTH_ELEVATION_QUERY
#else
class QueryCoordinatesHandler : public osgGA::GUIEventHandler
{
  public:
    QueryCoordinatesHandler( GlobePlugin* globe, osgEarth::Util::ElevationManager* elevMan,
                             const osgEarth::SpatialReference* mapSRS )
        :  mGlobe( globe ), _mapSRS( mapSRS ), _elevMan( elevMan ) { }

    bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa );

    virtual osg::Vec3d getCoords( float x, float y, osgViewer::View* view, bool getElevation = false );

  private:
    GlobePlugin* mGlobe;
    osg::ref_ptr<const SpatialReference> _mapSRS;
    osg::ref_ptr<osgEarth::Util::ElevationManager> _elevMan;
};
#endif


class KeyboardControlHandler : public osgGA::GUIEventHandler
{
  public:
    KeyboardControlHandler( osgEarth::Util::EarthManipulator* manip ) : _manip( manip ) { }

    bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa );

  private:
    osg::observer_ptr<osgEarth::Util::EarthManipulator> _manip;
};


namespace osgEarth
{
  namespace Util
  {
#ifdef USE_BACKPORTED_CONTROLS
    namespace Controls21
#else
    namespace Controls
#endif
    {
      class NavigationControlHandler : public ControlEventHandler
      {
        public:
          virtual void onMouseDown( class Control* control, int mouseButtonMask ) { Q_UNUSED( control ); Q_UNUSED( mouseButtonMask ); }
          virtual void onClick( class Control* control, int mouseButtonMask, const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa ) { Q_UNUSED( control ); Q_UNUSED( mouseButtonMask ); Q_UNUSED( ea ); Q_UNUSED( aa ); }
          virtual void onClick( class Control* control, int mouseButtonMask ) { Q_UNUSED( control ); Q_UNUSED( mouseButtonMask ); }
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
