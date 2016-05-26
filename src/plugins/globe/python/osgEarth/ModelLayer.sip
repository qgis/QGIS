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
     * Configuration options for a ModelLayer.
     */
    class ModelLayerOptions : osgEarth::ConfigOptions
    {
%TypeHeaderCode
#include <osgEarth/ModelLayer>
%End
    public:
        ModelLayerOptions( const osgEarth::ConfigOptions& options =osgEarth::ConfigOptions() );

        ModelLayerOptions( const std::string& name, const osgEarth::ModelSourceOptions& driverOptions =ModelSourceOptions() );

        /** Dtor */
        virtual ~ModelLayerOptions();

        /**
         * The readable name of the layer.
         */
//        optional<std::string>& name();
//        const optional<std::string>& name() const;

        /**
         * Options for the underlying model source driver.
         */
//        optional<ModelSourceOptions>& driver();
        const optional<osgEarth::ModelSourceOptions>& driver() const;

        /**
         * Whether to enable OpenGL lighting on the model node.
         */
//        optional<bool>& lightingEnabled();
//        const optional<bool>& lightingEnabled() const;

        /**
         * Whether this layer is active
         */
//        optional<bool>& enabled();
//        const optional<bool>& enabled() const;

        /**
         * Whether this layer is visible
         */
//        optional<bool>& visible();
//        const optional<bool>& visible() const;

        /**
         * Whether to drape the model geometry over the terrain as a projected overlay.
         * Defaults to false
         */
//        optional<bool>& overlay();
//        const optional<bool>& overlay() const;

    public:
        virtual osgEarth::Config getConfig() const;
        virtual void mergeConfig( const osgEarth::Config& conf );
    };

    /**
    * Callback for receiving notification of property changes on a ModelLayer.
    */
    struct ModelLayerCallback // : osg::Referenced
    {
%TypeHeaderCode
#include <osgEarth/ModelLayer>
%End
        virtual void onVisibleChanged( osgEarth::ModelLayer* layer );
        virtual void onOverlayChanged( osgEarth::ModelLayer* layer );
        virtual ~ModelLayerCallback();
    };

//    typedef void
//    (ModelLayerCallback::*ModelLayerCallbackMethodPtr)(osgEarth::ModelLayer* layer);

    typedef std::list< osg::ref_ptr<ModelLayerCallback> > ModelLayerCallbackList;


    class ModelLayer : public osgEarth::Layer
    {
%TypeHeaderCode
#include <osgEarth/ModelLayer>
%End
    public:
        /**
         * Constructs a new model layer.
         */
        ModelLayer( const osgEarth::ModelLayerOptions& options );

        /**
         * Constructs a new model layer with a user-provided driver options.
         */
//        ModelLayer( const std::string& name, const ModelSourceOptions& options );

        /**
         * Constructs a new model layer with a user-provided model source.
         */
//        ModelLayer(const osgEarth::ModelLayerOptions& options, osgEarth::ModelSource* source );

        /**
         * Constructs a new model layer with a user provided name and an existing node
         */
//        ModelLayer(const std::string& name, osg::Node* node);

        /** Dtor */
        virtual ~ModelLayer();

    public:
        /**
         * Gets the name of this model layer
         */
//        const std::string& getName() const;

        /**
         * Gets the initialization options for this layer.
         */
        const osgEarth::ModelLayerOptions& getModelLayerOptions() const;

        /**
         * Access the underlying model source.
         */
        osgEarth::ModelSource* getModelSource() const;

    public:

        /**
         * Perform one-time initialize of the model layer.
         */
//        void initialize( const osgDB::Options* options );

        /**
         * Creates the scene graph representing this model layer for the given Map.
         */
//        osg::Node* createSceneGraph(
//            const Map*            map,
//            const osgDB::Options* dbOptions,
//            ProgressCallback*     progress );


    public: // properties

        /** Whether this layer is rendered. */
        bool getVisible() const;
        void setVisible(bool value);

        /** Whether this layer is used at all. */
        bool getEnabled() const;

        /** Whether this layer is drawn as normal geometry or as a draped overlay. */
        bool getOverlay() const;
        void setOverlay( bool overlay );

        /** Whether to apply lighting to the model layer's root node */
        void setLightingEnabled( bool value );
        bool isLightingEnabled() const;

    public:

        /** Adds a property notification callback to this layer */
        void addCallback( osgEarth::ModelLayerCallback* cb );

        /** Removes a property notification callback from this layer */
        void removeCallback( osgEarth::ModelLayerCallback* cb );
    private:
      // Copying not allowed
      ModelLayer( const osgEarth::ModelLayer& rh );

    };

    typedef std::vector< osg::ref_ptr<ModelLayer> > ModelLayerVector;
};
