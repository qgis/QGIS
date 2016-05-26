/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/


namespace osgViewer {

//struct DepthPartitionSettings : public osg::Referenced
//{
//    enum DepthMode
//    {
//        FIXED_RANGE,
//        BOUNDING_VOLUME
//    };
//
//    DepthPartitionSettings(DepthMode mode=BOUNDING_VOLUME);
//
//    virtual bool getDepthRange(osg::View& view, unsigned int partition, double& zNear, double& zFar);
//
//    DepthMode _mode;
//    double _zNear;
//    double _zMid;
//    double _zFar;
//};

/** View holds a single view on a scene, this view may be composed of one or more slave cameras.*/
class View : osg::View //, public osgGA::GUIActionAdapter
{
%TypeHeaderCode
#include <osgViewer/View>
%End
    public:

        View();

//        View(const osgViewer::View& view, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);
//
//        META_Object(osgViewer,View);
//
//        /** Provide a mechanism for getting the osg::View associated from the GUIActionAdapter.
//          * One would use this to case view to osgViewer::View(er) if supported by the subclass.*/
//        virtual osg::View* asView() { return this; }
//
//        /** Provide a mechanism for getting the viewer object from this osgViewer::View.
//          * In the case of a osgViewer::Viewer the ViewerBase will effectively point to this object as Viewer subclasses from View.
//          * In the case of a osgViewer::CompsoiteViewer the ViewerBase will point to the CompositeViewer that owns this View. */
//        ViewerBase* getViewerBase() { return _viewerBase.get(); }
//
//        /** Take all the settings, Camera and Slaves from the passed in view, leaving it empty. */
//        virtual void take(osg::View& rhs);
//
//        virtual void setStartTick(osg::Timer_t tick);
//        osg::Timer_t getStartTick() const { return _startTick; }
//
//        Scene* getScene() { return _scene.get(); }
//        const Scene* getScene() const { return _scene.get(); }
//
//        /** Set the scene graph that the View will use.*/
//        virtual void setSceneData(osg::Node* node);
//
//        /** Get the View's scene graph.*/
//        osg::Node* getSceneData() { return _scene.valid() ? _scene->getSceneData() : 0; }
//
//        /** Get the const View's scene graph.*/
//        const osg::Node* getSceneData() const { return _scene.valid() ? _scene->getSceneData() : 0; }
//
//
//        /** Set the View's database pager.*/
//        void setDatabasePager(osgDB::DatabasePager* dp);
//
//        /** Get the View's database pager.*/
//        osgDB::DatabasePager* getDatabasePager();
//
//        /** Get the const View's database pager.*/
//        const osgDB::DatabasePager* getDatabasePager() const;
//
//
//        /** Set the View's image pager.*/
//        void setImagePager(osgDB::ImagePager* ip);
//
//        /** Get the View's image pager.*/
//        osgDB::ImagePager* getImagePager();
//
//        /** Get the const View's image pager.*/
//        const osgDB::ImagePager* getImagePager() const;
//
//
//        /* Set the EventQueue that View uses to integrate external non window related events.*/
//        void setEventQueue(osgGA::EventQueue* eventQueue) { _eventQueue = eventQueue; }
//
//        /* Get the View's EventQueue.*/
//        osgGA::EventQueue* getEventQueue() { return _eventQueue.get(); }
//
//        /* Get the const View's EventQueue.*/
//        const osgGA::EventQueue* getEventQueue() const { return _eventQueue.get(); }
//
//        /** Set the CameraManipulator that moves the View's master Camera position in response to events.
//          * The parameter resetPosition determines whether manipulator is set to its home position.*/
//        void setCameraManipulator(osgGA::CameraManipulator* manipulator, bool resetPosition = true);
//
//        /** Get the View's CameraManipulator.*/
//        osgGA::CameraManipulator* getCameraManipulator() { return _cameraManipulator.get(); }
//
//        /** Get the const View's CameraManipulator.*/
//        const osgGA::CameraManipulator* getCameraManipulator() const { return _cameraManipulator.get(); }
//
//        /** Set the view to the CameraManipulator's home position, if non is attached home() is does nothing.
//          * Note, to set the home position use getCamaraManipulator()->setHomePosition(...). */
//        void home();
//
//
//        typedef std::list< osg::ref_ptr<osgGA::GUIEventHandler> > EventHandlers;
//
        /** Add an EventHandler that adds handling of events to the View.*/
        void addEventHandler( osgGA::GUIEventHandler* eventHandler );

//        /** Remove an EventHandler from View.*/
//        void removeEventHandler(osgGA::GUIEventHandler* eventHandler);
//
//        /** Get the View's list of EventHandlers.*/
//        EventHandlers& getEventHandlers() { return _eventHandlers; }
//
//        /** Get the const View's list of EventHandlers.*/
//        const EventHandlers& getEventHandlers() const { return _eventHandlers; }
//
//
//        /** Set the NodePath to any active CoordinateSystemNode present in the Scene.
//          * The CoordinateSystemNode path is used to help applications and CamaraManipualtors handle geocentric coordinates systems,
//          * such as known which way is the local up at any position on the a whole earth. */
//        void setCoordinateSystemNodePath(const osg::NodePath& nodePath);
//
//        /** Get the NodePath to any active CoordinateSystemNode present in the Scene.*/
//        osg::NodePath getCoordinateSystemNodePath() const;
//
//        /** Compute the NodePath to any active CoordinateSystemNode present in the Scene.*/
//        void computeActiveCoordinateSystemNodePath();
//
//
//        /** Set the DisplaySettings object associated with this view.*/
//        void setDisplaySettings(osg::DisplaySettings* ds) { _displaySettings = ds; }
//
//        /** Set the DisplaySettings object associated with this view.*/
//        osg::DisplaySettings* getDisplaySettings() { return _displaySettings.get(); }
//
//        /** Set the DisplaySettings object associated with this view.*/
//        const osg::DisplaySettings* getDisplaySettings() const { return _displaySettings.get(); }
//
//        /** Set the FusionDistanceMode and Value. Note, is used only when working in stereo.*/
//        void setFusionDistance(osgUtil::SceneView::FusionDistanceMode mode,float value=1.0f)
//        {
//            _fusionDistanceMode = mode;
//            _fusionDistanceValue = value;
//        }
//
//        /** Get the FusionDistanceMode.*/
//        osgUtil::SceneView::FusionDistanceMode getFusionDistanceMode() const { return _fusionDistanceMode; }
//
//        /** Get the FusionDistanceValue. Note, only used for USE_FUSION_DISTANCE_VALUE & PROPORTIONAL_TO_SCREEN_DISTANCE modes.*/
//        float getFusionDistanceValue() const { return _fusionDistanceValue; }
//
//
//        /** Convenience method for creating slave Cameras and associated GraphicsWindows across all screens.*/
//        void setUpViewAcrossAllScreens();
//
//        /** Convenience method for a single camera on a single window.*/
//        void setUpViewInWindow(int x, int y, int width, int height, unsigned int screenNum=0);
//
//        /** Convenience method for a single camera associated with a single full screen GraphicsWindow.*/
//        void setUpViewOnSingleScreen(unsigned int screenNum=0);
//
//
//        /** Convenience method for spherical display using 6 slave cameras rendering the 6 sides of a cube map, and 7th camera doing distortion correction to present on a spherical display.*/
//        void setUpViewFor3DSphericalDisplay(double radius=1.0, double collar=0.45, unsigned int screenNum=0, osg::Image* intensityMap=0, const osg::Matrixd& projectorMatrix = osg::Matrixd());
//
//        /** Convenience method for spherical display by rendering main scene to as panoramic 2:1 texture and then doing distortion correction to present onto a spherical display.*/
//        void setUpViewForPanoramicSphericalDisplay(double radius=1.0, double collar=0.45, unsigned int screenNum=0, osg::Image* intensityMap=0, const osg::Matrixd& projectorMatrix = osg::Matrixd());
//
//        /** Convenience method for autostereoscopic Philips WoWvx display.*/
//        void setUpViewForWoWVxDisplay(unsigned int screenNum, unsigned char wow_content, unsigned char wow_factor, unsigned char wow_offset, float wow_disparity_Zd, float wow_disparity_vz, float wow_disparity_M, float wow_disparity_C);
//
//
//        /** Convenience method for setting up multiple slave cameras that depth partition the specified camera.*/
//        bool setUpDepthPartitionForCamera(osg::Camera* cameraToPartition, DepthPartitionSettings* dps=0);
//
//        /** Convenience method for setting up multiple slave cameras that depth partition each of the view's active cameras.*/
//        bool setUpDepthPartition(DepthPartitionSettings* dsp=0);
//
//
//        /** Return true if this view contains a specified camera.*/
//        bool containsCamera(const osg::Camera* camera) const;
//
//        /** Get the camera which contains the pointer position x,y specified master cameras window/eye coords.
//          * Also passes back the local window coords for the graphics context associated with the camera passed back. */
//        const osg::Camera* getCameraContainingPosition(float x, float y, float& local_x, float& local_y) const;
//
//        /** Compute intersections between a ray through the specified master cameras window/eye coords and a specified node.
//          * Note, when a master cameras has slaves and no viewport itself its coordinate frame will be in clip space i.e. -1,-1 to 1,1,
//          * while if its has a viewport the coordintates will be relative to its viewport dimensions.
//          * Mouse events handled by the view will automatically be attached into the master camera window/clip coords so can be passed
//          * directly on to the computeIntersections method. */
//        bool computeIntersections(float x,float y, osgUtil::LineSegmentIntersector::Intersections& intersections,osg::Node::NodeMask traversalMask = 0xffffffff);
//
//        /** Compute intersections between a ray through the specified master cameras window/eye coords and a specified nodePath's subgraph. */
//        bool computeIntersections(float x,float y, const osg::NodePath& nodePath, osgUtil::LineSegmentIntersector::Intersections& intersections,osg::Node::NodeMask traversalMask = 0xffffffff);
//
//
//        virtual void requestRedraw();
//        virtual void requestContinuousUpdate(bool needed=true);
//        virtual void requestWarpPointer(float x,float y);
//
//    public:
//
//        void assignSceneDataToCameras();
//        void init();
//
//    protected:
//
//        friend class CompositeViewer;
//
//        virtual osg::GraphicsOperation* createRenderer(osg::Camera* camera);
//
//        osg::observer_ptr<ViewerBase>          _viewerBase;
//
//
//        osg::Timer_t                            _startTick;
//
//        osg::ref_ptr<osgViewer::Scene>          _scene;
//        osg::ref_ptr<osgGA::EventQueue>         _eventQueue;
//        osg::ref_ptr<osgGA::CameraManipulator>  _cameraManipulator;
//        EventHandlers                           _eventHandlers;
//
//        osg::ObserverNodePath                   _coordinateSystemNodePath;
//
//        osg::ref_ptr<osg::DisplaySettings>      _displaySettings;
//        osgUtil::SceneView::FusionDistanceMode  _fusionDistanceMode;
//        float                                   _fusionDistanceValue;
    private:
        virtual ~View();

};

};
