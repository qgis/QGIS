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

#include <qgisplugin.h>
#include <QObject>
#include <osg/ref_ptr>
#include <osgEarth/Version>

#include "qgsglobeplugindialog.h"
#include "qgsrectangle.h"

class QAction;
class QDateTime;
class QDockWidget;
class QgsAnnotationItem;
class QgsGlobeAnnotation;
class QgsGlobeLayerPropertiesFactory;
class QgsGlobePluginDialog;
class QgsGlobeWidget;
class QgsMapLayer;
class QgsPoint;
class QgsRectangle;
class QgsGlobeFrustumHighlightCallback;
class QgsGlobeFeatureIdentifyCallback;
class QgsGlobeTileSource;
class QgsGlobeVectorLayerConfig;

namespace osg
{
  class Group;
  class Vec3d;
}
namespace osgViewer { class Viewer; }

namespace osgEarth
{
  class GeoPoint;
  class GeoExtent;
  class ImageLayer;
  class MapNode;
  namespace Annotation { class PlaceNode; }
  namespace QtGui { class ViewerWidget; }
  namespace Util
  {
    class FeatureHighlightCallback;
    class FeatureQueryTool;
    class SkyNode;
    class VerticalScale;
    namespace Controls
    {
      class Control;
      class ControlEventHandler;
      class LabelControl;
    }
  }
}


class GLOBE_EXPORT GlobePlugin : public QObject, public QgisPlugin
{
    Q_OBJECT

  public:
    GlobePlugin( QgisInterface* theQgisInterface );
    ~GlobePlugin();

    //! init the gui
    virtual void initGui() override;
    //! unload the plugin
    void unload() override;

    //! Enable or disable frustum highlight
    void enableFrustumHighlight( bool statu );
    //! Enable or disable feature identification
    void enableFeatureIdentification( bool status );

    //! set the globe coordinates of a user right-click on the globe
    void setSelectedCoordinates( const osg::Vec3d& coords );
    //! get a coordinates vector
    osg::Vec3d getSelectedCoordinates();
    //! emits signal with current mouse coordinates
    void showCurrentCoordinates( const osgEarth::GeoPoint &geoPoint );
    //! get longitude of user right click
    double getSelectedLon() const { return mSelectedLon; }
    //! get latitude of user right click
    double getSelectedLat() const { return mSelectedLat; }
    //! get elevation of user right click
    double getSelectedElevation() { return mSelectedElevation; }

    //! Get the OSG viewer
    osgViewer::Viewer* osgViewer() { return mOsgViewer; }
    //! Get OSG map node
    osgEarth::MapNode* mapNode() { return mMapNode; }

    QgisInterface* qgisIface() const { return mQGisIface; }

  public slots:
    void run();
    void updateLayers();
    void showSettings();
    void syncExtent();

  private:
    QgisInterface *mQGisIface;

    QAction* mActionToggleGlobe;
    osgEarth::QtGui::ViewerWidget* mViewerWidget;
    QgsGlobeWidget* mDockWidget;
    QgsGlobePluginDialog* mSettingsDialog;
    QString mBaseLayerUrl;
    QList<QgsGlobePluginDialog::LayerDataSource> mImagerySources;
    QList<QgsGlobePluginDialog::LayerDataSource> mElevationSources;
    double mSelectedLat, mSelectedLon, mSelectedElevation;

    osg::ref_ptr<osgViewer::Viewer> mOsgViewer;
    osg::ref_ptr<osgEarth::MapNode> mMapNode;
    osg::ref_ptr<osg::Group> mRootNode;
    osg::ref_ptr<osgEarth::Util::SkyNode> mSkyNode;
    osg::ref_ptr<osgEarth::ImageLayer> mBaseLayer;
    osg::ref_ptr<osgEarth::ImageLayer> mQgisMapLayer;
    osg::ref_ptr<QgsGlobeTileSource> mTileSource;
    QMap<QString, QgsRectangle> mLayerExtents;
    osg::ref_ptr<osgEarth::Util::VerticalScale> mVerticalScale;

    //! Creates additional pages in the layer properties for adjusting 3D properties
    QgsGlobeLayerPropertiesFactory* mLayerPropertiesFactory;
    osg::ref_ptr<QgsGlobeFrustumHighlightCallback> mFrustumHighlightCallback;
    osg::ref_ptr<QgsGlobeFeatureIdentifyCallback> mFeatureQueryToolIdentifyCb;
    // TODO: How to port highlight to 2.7.0?
#if OSGEARTH_VERSION_LESS_THAN(2, 7, 0)
    osg::ref_ptr<osgEarth::Util::FeatureHighlightCallback> mFeatureQueryToolHighlightCb;
#endif
    osg::ref_ptr<osgEarth::Util::FeatureQueryTool> mFeatureQueryTool;
    osg::ref_ptr<osgEarth::Util::Controls::LabelControl> mStatsLabel;

    void setupProxy();
    void addControl( osgEarth::Util::Controls::Control* control, int x, int y, int w, int h, osgEarth::Util::Controls::ControlEventHandler* handler );
    void addImageControl( const std::string &imgPath, int x, int y, osgEarth::Util::Controls::ControlEventHandler* handler = 0 );
    void addModelLayer( QgsVectorLayer* mapLayer , QgsGlobeVectorLayerConfig *layerConfig );
    void setupControls();
    void applyProjectSettings();
    QgsRectangle getQGISLayerExtent() const;

  private slots:
    void setGlobeEnabled( bool enabled );
    void reset();
    void projectRead();
    void applySettings();
    void layerChanged( QgsMapLayer* mapLayer = 0 );
    void rebuildQGISLayer();
    void refreshQGISMapLayer( const QgsRectangle &dirtyRect );
    void updateTileStats( int queued, int tot );

  signals:
    //! emits current mouse position
    void xyCoordinates( const QgsPoint & p );
    //! emits position of right click on globe
    void newCoordinatesSelected( const QgsPoint & p );
};

#endif // QGS_GLOBE_PLUGIN_H
