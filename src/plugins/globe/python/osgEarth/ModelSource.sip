/* -*-c++-*- */
/* osgEarth - Dynamic map generation toolkit for OpenSceneGraph
 * Copyright 2008-2013 Pelican Mapping
 * http://osgearth.org
 *
 * osgEarth is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

namespace osgEarth
{
    /**
     * Configuration options for a models source.
     */
    class ModelSourceOptions// : public DriverConfigOptions
    {
%TypeHeaderCode
#include <osgEarth/ModelSource>
%End
    public:
        ModelSourceOptions( const osgEarth::ConfigOptions& options =osgEarth::ConfigOptions() );

        /** Dtor */
        virtual ~ModelSourceOptions();

    public: // properties

//        /** minimum camera range at which the data is visible */
//        optional<float>& minRange() { return _minRange; }
//        const optional<float>& minRange() const { return _minRange; }
//
//        /** maximum camera range at which the data is visible */
//        optional<float>& maxRange() { return _maxRange; }
//        const optional<float>& maxRange() const { return _maxRange; }
//
//        /** render bin order */
//        optional<int>& renderOrder() { return _renderOrder; }
//        const optional<int>& renderOrder() const { return _renderOrder; }
//
//        /** whether to read the depth buffer when rendering */
//        optional<bool>& depthTestEnabled() { return _depthTestEnabled; }
//        const optional<bool>& depthTestEnabled() const { return _depthTestEnabled; }

    public:
        virtual osgEarth::Config getConfig() const;

    protected:
        virtual void mergeConfig( const osgEarth::Config& conf );
    };


    /**
     * A ModelSource is a plugin object that generates OSG nodes.
     */
    class ModelSource : osg::Object, osgEarth::Revisioned
    {
    public:
//        ModelSource( const osgEarth::ModelSourceOptions& options =osgEarth::ModelSourceOptions() );

        /** Dtor */
        virtual ~ModelSource();

//        osg::Node* createNode(
//            const osgEarth::Map*  map,
//            const osgDB::Options* dbOptions,
//            ProgressCallback*     progress );

        /**
         * Subclass implements this method to create a scene graph within the
         * context of the provided Map.
         */
//        virtual osg::Node* createNodeImplementation(
//            const osgEarth::Map*            map,
//            const osgDB::Options* dbOptions,
//            ProgressCallback*     progress ) =0;
//
//        /**
//         * Add a post processing opeation - this will be called on any node
//         * that enters the scene graph by the model source.
//         */
//        void addPostProcessor( NodeOperation* cb );
//
//        /**
//         * Remove a post processing operation
//         */
//        void removePostProcessor( NodeOperation* cb );
//
//        /**
//         * The vector of post processor callback operations
//         */
//        const NodeOperationVector& postProcessors() const { return _postProcessors; }


//    public: // META_Object specialization
//        // these are neseccary to we can load ModelSource implementations as plugins
//        virtual osg::Object* cloneType() const { return 0; } // cloneType() not appropriate
//        virtual osg::Object* clone(const osg::CopyOp&) const { return 0; } // clone() not appropriate
//        virtual bool isSameKindAs(const osg::Object* obj) const { return dynamic_cast<const ModelSource*>(obj)!=NULL; }
        virtual const char* className() const;
        virtual const char* libraryName() const;

    public:

        /** Perform any one-time initialization */
//        virtual void initialize( const osgDB::Options* dbOptions );

        /** Get the options used to create this model source */
        const osgEarth::ModelSourceOptions& getOptions() const;

    protected:
        /**
         * Fire the callbacks. The implementation class should call this whenever it adds
         * a new node.
         */
//        void firePostProcessors( osg::Node* node );
//
   private:
        ModelSource(); // TODO(MK): Remove this and make this class abstract instead
        ModelSource& operator=( const osgEarth::ModelSource& );
        ModelSource( const osgEarth::ModelSource& rh );
    };

    //--------------------------------------------------------------------

    class ModelSourceDriver // : osgDB::ReaderWriter
    {
    protected:
//        const osgEarth::ModelSourceOptions& getModelSourceOptions( const osgDB::ReaderWriter::Options* rwOpt ) const;
    private:
      ModelSourceDriver& operator=( const osgEarth::ModelSourceDriver& );
    };

    //--------------------------------------------------------------------

    /**
     * Creates TileSource instances and chains them together to create
     * tile source "pipelines" for data access and processing.
     */
    class ModelSourceFactory
    {
    public:
        static osgEarth::ModelSource* create( const osgEarth::ModelSourceOptions& options );
    };
};
