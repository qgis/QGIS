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

namespace osg
{

/** View - maintains a master camera view and a list of slave cameras that are relative to this master camera.
  * Note, if no slave cameras are attached to the view then the master camera does both the control and implementation of the rendering of the scene,
  * but if slave cameras are present then the master controls the view onto the scene, while the slaves implement the rendering of the scene.
*/
class View : osg::Object
{
%TypeHeaderCode
#include <osg/View>
%End
    public :

        View();

//        View(const osg::View& view, const osg::CopyOp& copyop=CopyOp::SHALLOW_COPY);
//
//        META_Object(osg,View);
//
//        /** Take all the settings, Camera and Slaves from the passed in view, leaving it empty. */
//        virtual void take(View& rhs);
//
//
//        /** Set the Stats object used for collect various frame related timing and scene graph stats.*/
//        void setStats(osg::Stats* stats) { _stats = stats; }
//
//        /** Get the Viewers Stats object.*/
//        osg::Stats* getStats() { return _stats.get(); }
//
//        /** Get the Viewers Stats object.*/
//        const osg::Stats* getStats() const { return _stats.get(); }
//
//
//        /** Options for controlling the global lighting used for the view.*/
//        enum LightingMode
//        {
//            NO_LIGHT,
//            HEADLIGHT,
//            SKY_LIGHT
//        };
//
//        /** Set the global lighting to use for this view.
//          * Defaults to headlight. */
//        void setLightingMode(LightingMode lightingMode);
//
//        /** Get the global lighting used for this view.*/
//        LightingMode getLightingMode() const { return _lightingMode; }
//
//        /** Get the global light.*/
//        void setLight(osg::Light* light) { _light = light; }
//
//        /** Get the global lighting if assigned.*/
//        osg::Light* getLight() { return _light.get(); }
//
//        /** Get the const global lighting if assigned.*/
//        const osg::Light* getLight() const { return _light.get(); }
//
//        /** Set the master camera of the view. */
//        void setCamera(osg::Camera* camera);
//
//        /** Get the master camera of the view. */
//        osg::Camera* getCamera() { return _camera.get(); }
//
//        /** Get the const master camera of the view. */
//        const osg::Camera* getCamera() const { return _camera.get(); }
//
//        /** Set the frame stamp of the view. */
//        void setFrameStamp(osg::FrameStamp* fs) { _frameStamp = fs; }
//
//        /** Get the frame stamp of the view. */
//        osg::FrameStamp* getFrameStamp() { return _frameStamp.get(); }
//
//        /** Get the frame stamp of the view. */
//        const osg::FrameStamp* getFrameStamp() const { return _frameStamp.get(); }
//
//
//        /** Slave allows one to up a camera that follows the master with a local offset to the project and view matrices.*/
//        struct OSG_EXPORT Slave
//        {
//            Slave(bool useMastersSceneData=true):
//                _useMastersSceneData(useMastersSceneData) {}
//
//            Slave(osg::Camera* camera, const osg::Matrixd& projectionOffset, const osg::Matrixd& viewOffset, bool useMastersSceneData=true):
//                _camera(camera),
//                _projectionOffset(projectionOffset),
//                _viewOffset(viewOffset),
//                _useMastersSceneData(useMastersSceneData) {}
//
//            Slave(const Slave& rhs) :
//                _camera(rhs._camera),
//                _projectionOffset(rhs._projectionOffset),
//                _viewOffset(rhs._viewOffset),
//                _useMastersSceneData(rhs._useMastersSceneData),
//                _updateSlaveCallback(rhs._updateSlaveCallback) {}
//
//            virtual ~Slave() {}
//
//            Slave& operator = (const Slave& rhs)
//            {
//                _camera = rhs._camera;
//                _projectionOffset = rhs._projectionOffset;
//                _viewOffset = rhs._viewOffset;
//                _useMastersSceneData = rhs._useMastersSceneData;
//                _updateSlaveCallback = rhs._updateSlaveCallback;
//                return *this;
//            }
//
//            struct UpdateSlaveCallback : public virtual Referenced
//            {
//                virtual void updateSlave(osg::View& view, osg::View::Slave& slave) = 0;
//            };
//
//            void updateSlave(View& view)
//            {
//                if (_updateSlaveCallback.valid()) _updateSlaveCallback->updateSlave(view, *this);
//                else updateSlaveImplementation(view);
//            }
//
//            virtual void updateSlaveImplementation(View& view);
//
//            osg::ref_ptr<osg::Camera>           _camera;
//            osg::Matrixd                        _projectionOffset;
//            osg::Matrixd                        _viewOffset;
//            bool                                _useMastersSceneData;
//            osg::ref_ptr<UpdateSlaveCallback>   _updateSlaveCallback;
//        };
//
//        bool addSlave(osg::Camera* camera, bool useMastersSceneData=true) { return addSlave(camera, osg::Matrix::identity(), osg::Matrix::identity(), useMastersSceneData); }
//
//        bool addSlave(osg::Camera* camera, const osg::Matrix& projectionOffset, const osg::Matrix& viewOffset, bool useMastersSceneData=true);
//
//        bool removeSlave(unsigned int pos);
//
//        unsigned int getNumSlaves() const { return static_cast<unsigned int>(_slaves.size()); }
//
//        Slave& getSlave(unsigned int pos) { return _slaves[pos]; }
//        const Slave& getSlave(unsigned int pos) const { return _slaves[pos]; }
//
//        unsigned int findSlaveIndexForCamera(osg::Camera* camera) const;
//
//        Slave * findSlaveForCamera(osg::Camera* camera);
//
//        void updateSlaves();
//
    protected :
//        virtual osg::GraphicsOperation* createRenderer(osg::Camera*) { return 0; }
//
//        osg::ref_ptr<osg::Stats>        _stats;
//
//        LightingMode                    _lightingMode;
//        osg::ref_ptr<osg::Light>        _light;
//
//        osg::ref_ptr<osg::Camera>       _camera;
//
//        typedef std::vector<Slave>      Slaves;
//        Slaves                          _slaves;
//
//        osg::ref_ptr<osg::FrameStamp>   _frameStamp;

private:
    virtual ~View();
};

};
