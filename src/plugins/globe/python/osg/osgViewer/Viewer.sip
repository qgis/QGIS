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

/** Viewer holds a single view on to a single scene.*/
class Viewer : osgViewer::View
{
%TypeHeaderCode
  #include <osgViewer/Viewer>
%End
    public:

        Viewer();

//        Viewer(const osgViewer::Viewer& viewer, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

//        Camera* getCameraWithFocus();

        virtual ~Viewer();

//        META_Object(osgViewer,Viewer);
//
//        /** Take all the settings, Camera and Slaves from the passed in view(er), leaving it empty. */
//        virtual void take(osg::View& rhs);
//
//
//        /** Set the Stats object used for collect various frame related timing and scene graph stats.*/
//        virtual void setViewerStats(osg::Stats* stats) { setStats(stats); }
//
//        /** Get the Viewers Stats object.*/
//        virtual osg::Stats* getViewerStats() { return getStats(); }
//
//        /** Get the Viewers Stats object.*/
//        virtual const osg::Stats* getViewerStats() const { return getStats(); }
//
//
//        /** read the viewer configuration from a configuration file.*/
//        virtual bool readConfiguration(const std::string& filename);
//
//        /** Get whether at least of one of this viewers windows are realized.*/
//        virtual bool isRealized() const;
//
//        /** set up windows and associated threads.*/
//        virtual void realize();
//
//        virtual void setStartTick(osg::Timer_t tick);
//        void setReferenceTime(double time=0.0);
//
//        /** Set the sene graph data that viewer with view.*/
//        virtual void setSceneData(osg::Node* node);
//
//
//        /** Convenience method for setting up the viewer so it can be used embedded in an external managed window.
//          * Returns the GraphicsWindowEmbedded that can be used by applications to pass in events to the viewer. */
//        virtual GraphicsWindowEmbedded* setUpViewerAsEmbeddedInWindow(int x, int y, int width, int height);
//
//
//        virtual double elapsedTime();
//
//        virtual osg::FrameStamp* getViewerFrameStamp() { return getFrameStamp(); }
//
//        /** Execute a main frame loop.
//          * Equivalent to while (!viewer.done()) viewer.frame();
//          * Also calls realize() if the viewer is not already realized,
//          * and installs trackball manipulator if one is not already assigned.
//          */
//        virtual int run();
//
//        /** check to see if the new frame is required, called by run(..) when FrameScheme is set to ON_DEMAND.*/
//        virtual bool checkNeedToDoFrame();
//
//        virtual void advance(double simulationTime=USE_REFERENCE_TIME);
//
//        virtual void eventTraversal();
//
//        virtual void updateTraversal();
//
//        void setCameraWithFocus(osg::Camera* camera) { _cameraWithFocus = camera; }
//        osg::Camera* getCameraWithFocus() { return _cameraWithFocus.get(); }
//        const osg::Camera* getCameraWithFocus() const { return _cameraWithFocus.get(); }
//
//        virtual void getCameras(Cameras& cameras, bool onlyActive=true);
//
//        virtual void getContexts(Contexts& contexts, bool onlyValid=true);
//
//        virtual void getAllThreads(Threads& threads, bool onlyActive=true);
//
//        virtual void getOperationThreads(OperationThreads& threads, bool onlyActive=true);
//
//        virtual void getScenes(Scenes& scenes, bool onlyValid=true);
//
//        virtual void getViews(Views& views, bool onlyValid=true);
//
//        /** Get the keyboard and mouse usage of this viewer.*/
//        virtual void getUsage(osg::ApplicationUsage& usage) const;
//
//
//    protected:
//
//        void constructorInit();
//
//        virtual void viewerInit() { init(); }
//
//        osg::observer_ptr<osg::Camera>              _cameraWithFocus;
//
//

    private:
      Viewer& operator= ( const Viewer& );
};

};
