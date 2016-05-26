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
     * Initialization options for an image layer.
     */
    class ImageLayerOptions // : osgEarth::TerrainLayerOptions
    {
    public:
        /** Constructs new image layer options. */
        ImageLayerOptions( const osgEarth::ConfigOptions& options =osgEarth::ConfigOptions() );

        /**
         * Constructs new image layer options, giving a name to the layer and specifying
         * driver options to use.
         */
//        ImageLayerOptions( const std::string& name, const osgEarth::TileSourceOptions& driverOpt =osgEarth::TileSourceOptions() );

        /** Dtor */
        virtual ~ImageLayerOptions();


    public: // properties

        /**
         * The initial opacity of this layer
         */
//        optional<float>& opacity();
//        const optional<float>& opacity() const;

        /**
         * The initial minimum camera range at which this layer is visible.
         */
//        optional<float>& minVisibleRange();
//        const optional<float>& minVisibleRange() const;

        /**
         * The initial maximum camera range at which this layer is visible.
         */
//        optional<float>& maxVisibleRange();
//        const optional<float>& maxVisibleRange() const;

        /**
         * Gets or sets the nodata image for this MapLayer
         */
//        optional<URI>& noDataImageFilename();
//        const optional<URI>& noDataImageFilename();

        /**
         * Gets the transparent color of this TileSource
         */
//        optional<osg::Vec4ub>& transparentColor();
//        const optional<osg::Vec4ub>& transparentColor() const;

        /**
         * Whether LOD blending is enabled for this layer
         */
//        optional<bool>& lodBlending();
//        const optional<bool>& lodBlending() const;

        /**
         * Filters attached to this layer.
         */
//        ColorFilterChain& colorFilters();
//        const ColorFilterChain& colorFilters() const;

    public:

        virtual osgEarth::Config getConfig() const;
        virtual osgEarth::Config getConfig( bool isolate ) const;
        virtual void mergeConfig( const osgEarth::Config& conf );
    };

    //--------------------------------------------------------------------

    /**
     * Callback for receiving notification of property changes on an ImageLayer.
     */
    struct ImageLayerCallback // : osgEarth::TerrainLayerCallback
    {
%TypeHeaderCode
#include<osgEarth/ImageLayer>
%End
        virtual void onOpacityChanged( osgEarth::ImageLayer* layer );
        virtual void onVisibleRangeChanged( osgEarth::ImageLayer* layer );
        virtual void onColorFiltersChanged( osgEarth::ImageLayer* layer );
        virtual ~ImageLayerCallback();
    };

//    typedef void (ImageLayerCallback::*ImageLayerCallbackMethodPtr)(ImageLayer* layer);

    typedef std::list< osg::ref_ptr<ImageLayerCallback> > ImageLayerCallbackList;

    //--------------------------------------------------------------------

    /**
     * Internal utility class for post-processing image tiles that come from
     * a TileSource
     */
//    class ImageLayerTileProcessor
//    {
//%TypeHeaderCode
//#include<osgEarth/ImageLayer>
//%End
//    public:
//        ImageLayerTileProcessor( const osgEarth::ImageLayerOptions& options =osgEarth::ImageLayerOptions() );
//
//        /** dtor */
//        virtual ~ImageLayerTileProcessor();
//
//        void init( const osgEarth::ImageLayerOptions& options, const osgDB::Options* dbOptions, bool layerInTargetProfile );
//
//        void process( osg::ref_ptr<osg::Image>& image ) const;
//    };

    //--------------------------------------------------------------------

    /**
     * A map terrain layer containing bitmap image data.
     */
    class ImageLayer // : TerrainLayer
    {
%TypeHeaderCode
#include<osgEarth/ImageLayer>
%End
    public:
        /**
         * Constructs a new image layer.
         */
        ImageLayer( const osgEarth::ImageLayerOptions& options );

        /**
         * Constructs a new image layer with the given name and driver options.
         */
//        ImageLayer( const std::string& name, const TileSourceOptions& driverOptions );

        /**
         * Constructs a new image layer with a custom TileSource.
         */
//        ImageLayer( const osgEarth::ImageLayerOptions& options, TileSource* tileSource );

        /** Dtor */
        virtual ~ImageLayer();

    public:
        /**
         * Access to the initialization options used to create this image layer
         */
//        const osgEarth::ImageLayerOptions& getImageLayerOptions() const;
//        virtual const TerrainLayerOptions& getTerrainLayerRuntimeOptions() const;

        /** Adds a property notification callback to this layer */
        void addCallback( osgEarth::ImageLayerCallback* cb );

        /** Removes a property notification callback from this layer */
        void removeCallback( osgEarth::ImageLayerCallback* cb );

        /** Override: see TerrainLayer */
//        virtual void setTargetProfileHint( const osgEarth::Profile* profile );


        /**
         * Add a color filter (to the end of the chain)
         */
//        void addColorFilter( ColorFilter* filter );

        /**
         * Remove a color filter
         */
//        void removeColorFilter( ColorFilter* filter );

        /**
         * Access the image filter chain
         */
//        const ColorFilterChain& getColorFilters() const;


    public: // runtime properties

        /**
         * Sets the opacity of this image layer.
         * @param opacity Opacity [0..1] -> [transparent..opaque]
         */
        void setOpacity( float opacity );

        float getOpacity() const;

        void disableLODBlending();
        bool isLODBlendingEnabled() const;

        float getMinVisibleRange() const;
        void setMinVisibleRange( float minVisibleRange );

        float getMaxVisibleRange() const;
        void setMaxVisibleRange( float maxVisibleRange );


    public: // methods

        /**
         * Creates a GeoImage from this layer corresponding to the provided key. The
         * image is in the profile of the key and will be reprojected, mosaiced and
         * cropped automatically.
         */
//        virtual GeoImage createImage( const TileKey& key, ProgressCallback* progress = 0, bool forceFallback =false);

        /**
         * Creates an image that is in the image layer's native profile.
         */
//        GeoImage createImageInNativeProfile(const TileKey& key, ProgressCallback* progress, bool forceFallback, bool& out_isFallback);

    public: // TerrainLayer override

//        CacheBin* getCacheBin( const osgEarth::Profile* profile );


    protected:

        // Creates an image that's in the same profile as the provided key.
//        GeoImage createImageInKeyProfile(const TileKey& key, ProgressCallback* progress, bool forceFallback, bool& out_isFallback);

        // Fetches an image from the underlying TileSource whose data matches that of the
        // key extent.
//        GeoImage createImageFromTileSource(const TileKey& key, ProgressCallback* progress, bool forceFallback, bool& out_isFallback);

        // Fetches multiple images from the TileSource; mosaics/reprojects/crops as necessary, and
        // returns a single tile. This is called by createImageFromTileSource() if the key profile
        // doesn't match the layer profile.
//        GeoImage assembleImageFromTileSource(const TileKey& key, ProgressCallback* progress, bool& out_isFallback);


        virtual void initTileSource();

    protected:
//        osgEarth::ImageLayerOptions              _runtimeOptions;
//        osg::ref_ptr<TileSource::ImageOperation> _preCacheOp;
//        osg::ref_ptr<osg::Image>                 _emptyImage;
//        ImageLayerCallbackList                   _callbacks;

//        virtual void fireCallback( TerrainLayerCallbackMethodPtr method );
//        virtual void fireCallback( ImageLayerCallbackMethodPtr method );

        void init();
        void initPreCacheOp();
    private:
        ImageLayer( const osgEarth::ImageLayer& );
    };

    typedef std::vector< osg::ref_ptr<osgEarth::ImageLayer> > ImageLayerVector;

}; // namespace osgEarth
