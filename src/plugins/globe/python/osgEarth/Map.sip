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
%TypeHeaderCode
#include <osgEarth/Map>
%End

    /**
     * Map is the main data model that the MapNode will render. It is a
     * container for all Layer objects (that contain the actual data) and
     * the rendering options.
     */
    class Map // : osg::Referenced
    {
    public:
        /**
         * Constructs a new, empty map.
         */
//        Map( const MapOptions& options =MapOptions() );

//    public:
//        /**
//         * Gets the options governing this map.
//         */
//        const MapOptions& getMapOptions() const { return _mapOptions; }
//
//        /**
//         * Gets the options with which this map was initially created.
//         */
//        const MapOptions& getInitialMapOptions() const { return _initMapOptions; }
//
//        /**
//         * Gets the map's master profile. This value may not be available until
//         * after autoCalculateProfile has been called.
//         */
//        const Profile* getProfile() const;
//
        /**
         * Gets the SRS of the map's profile (convenience)
         */
        const osgEarth::SpatialReference* getSRS() const;

        /**
         * Copies references of the map image layers into the output list.
         * This method is thread safe. It returns the map revision that was
         * in effect when the data was copied.
         */
//        osgEarth::Revision getImageLayers( osgEarth::ImageLayerVector& out_layers ) const;

        /**
         * Gets the number of image layers in the map.
         */
        int getNumImageLayers() const;

        /**
         * Gets an image layer by name.
         */
        osgEarth::ImageLayer* getImageLayerByName( const std::string& name ) const;

        /**
         * Gets an image layer by its unique ID.
         */
        osgEarth::ImageLayer* getImageLayerByUID( osgEarth::UID layerUID ) const;
//
//        /**
//         * Gets an image layer at the specified index.
//         */
//        ImageLayer* getImageLayerAt( int index ) const;
//
//        /**
//         * Copies references of the elevation layers into the output list.
//         * This method is thread safe. It returns the map revision that was
//         * in effect when the data was copied.
//         */
//        Revision getElevationLayers( ElevationLayerVector& out_layers ) const;
//
//        /**
//         * Gets the number of elevation layers in the map.
//         */
//        int getNumElevationLayers() const;
//
//        /**
//         * Gets an elevation layer by name.
//         */
//        ElevationLayer* getElevationLayerByName( const std::string& name ) const;
//
//        /**
//         * Gets an elevation layer by its unique ID.
//         */
//        ElevationLayer* getElevationLayerByUID( UID layerUID ) const;
//
//        /**
//         * Gets an elevation layer at the specified index.
//         */
//        ElevationLayer* getElevationLayerAt( int index ) const;
//
        /**
         * Copies references of the model layers into the output list.
         * This method is thread safe. It returns the map revision that was
         * in effect when the data was copied.
         */
//        osgEarth::Revision getModelLayers( osgEarth::ModelLayerVector& out_layers /Out/ ) const;


        /**
         * Gets the number of model layers in the map.
         */
        int getNumModelLayers() const;

        /**
         * Gets a model layer by name.
         */
        osgEarth::ModelLayer* getModelLayerByName( const std::string& name ) const;

        /**
         * Gets a model layer by its unique ID.
         */
        osgEarth::ModelLayer* getModelLayerByUID( osgEarth::UID layerUID ) const;

        /**
         * Gets the model layer at the specified index.
         */
        osgEarth::ModelLayer* getModelLayerAt( int index ) const;
//
//        /**
//         * Copies references of the mask layers into the output list.
//         * This method is thread safe. It returns the map revision that was
//         * in effect when the data was copied.
//         */
//        int getTerrainMaskLayers( MaskLayerVector& out_list ) const;
//
//        /**
//         * Adds a map layer callback to this map. This will be notified whenever layers are
//         * added, removed, or re-ordered.
//         */
//        void addMapCallback( MapCallback* callback ) const;
//
//        /**
//         * Removes a callback previously added with addMapCallback.
//         */
//        void removeMapCallback( MapCallback* callback );
//
//        /**
//         * Begin a batch-update operation. Call this if you intend to add multiple
//         * layers at once; then call endUpdate() to complete the operation.
//         * Between calls to beginUpdate() and endUpdate(), Map will not invoke
//         * any callbacks you added wtih addMapCallback.
//         */
//        void beginUpdate();
//
//        /**
//         * Complete a batch update operation that started with a call to
//         * beginUpdate(). Fires all callbacks for operations that occurred
//         * since the call to beginUpdate().
//         */
//        void endUpdate();
//
//        /**
//         * Adds an image layer to the map.
//         */
//        void addImageLayer( ImageLayer* layer );
//
//        /**
//        * Adds an image layer to the map at the specified index.
//        */
//        void insertImageLayer( ImageLayer* layer, unsigned int index );
//
//        /**
//         * Removes an image layer from the map.
//         */
//        void removeImageLayer( ImageLayer* layer );
//
//        /**
//         * Moves (re-orders) an image layer to another index position in its list.
//         */
//        void moveImageLayer( ImageLayer* layer, unsigned int newIndex );
//
//        /**
//         * Adds an elevation layer to the map.
//         */
//        void addElevationLayer( ElevationLayer* layer );
//
//        /**
//         * Removes an elevation layer from the map.
//         */
//        void removeElevationLayer( ElevationLayer* layer );
//
//        /**
//         * Moves (re-orders) an elevation layer to another index position in its list.
//         */
//        void moveElevationLayer( ElevationLayer* layer, unsigned int newIndex );
//
//        /**
//         * Adds a new model layer to the map.
//         */
//        void addModelLayer( osgEarth::ModelLayer* layer );
//
//        /**
//         * Adds a new model layer to the map at the specified index.
//         */
//        void insertModelLayer( ModelLayer* layer, unsigned int index );
//
//        /**
//         * Removes a model layer from the map.
//         */
//        void removeModelLayer( ModelLayer* layer );
//
//        /**
//         * Moves (re-orders) an image layer to another index position in its list.
//         */
//        void moveModelLayer( ModelLayer* layer, unsigned int newIndex );
//
//        /**
//         * Adds a new layer to use as a terrain mask.
//         */
//        void addTerrainMaskLayer( MaskLayer* layer );
//
//        /**
//         * Removed a terrain mask layer that was set with setTerrainMaskLayer().
//         */
//        void removeTerrainMaskLayer( MaskLayer* layer );
//
//        /**
//         * Clear all layers from this map.
//         */
//        void clear();
//
//        /**
//         * Replaces the layers in this Map with layers from the specified Map
//         * (except for terrain mask layers)
//         */
//        void setLayersFromMap( const Map* map );
//
//    public:
//        /**
//         * Gets the user-provided options structure stored in this map.
//         */
//        const osgDB::ReaderWriter::Options* getGlobalOptions() const;
//        void setGlobalOptions( const osgDB::ReaderWriter::Options* options );
//
//        /**
//         * Sets the readable name of this map.
//         */
//        void setName( const std::string& name );
//
//        /**
//         * Gets the readable name of this map.
//         */
//        const std::string& getName() const { return _name; }
//
//        /**
//         * Creates a heightfield for the region covered by the given TileKey, falling back on
//         * lower resolutions if necessary.
//         *
//         * NOTE: By default, this method will return a heightfield with HAE (height above ellipsoid)
//         * values, even if the TileKey profile has an MSL vertical datum. That's because this
//         * method is usually called to produce a renderable height field. You can override this
//         * behavior by passing in convertToHAE=false.
//         *
//         * @param key
//         *      Tile key defining the region (and ideal LOD) for which to return a heightfield
//         * @param fallbackIfNecessary
//         *      If the map can't generate a true heightfield for the key, fall back on lower
//         *      levels of detail until it can make one.
//         * @param out_hf
//         *      Resulting heightfield is written here.
//         * @param out_isFallback
//         *      Output flag telling the caller whether the method had to "fall back" on a
//         *      lower level of detail.
//         * @param convertToHAE
//         *      Whether to return height-above-ellipsoid values in the heightfield, even if the
//         *      input tile key's SRS specifies a vertical datum (which would normally result in
//         *      a heightfield expressed relative to MSL). This is typical for when you are building
//         *      the actual 3D terrain.
//         * @param samplePolicy
//         *      How to intepolate heightfield samples.
//         * @param progress
//         *      (optional) progress callback.
//         */
//        bool getHeightField(
//            const TileKey&                  key,
//            bool                            fallbackIfNeessary,
//            osg::ref_ptr<osg::HeightField>& out_hf,
//            bool*                           out_isFallback =0L,
//            bool                            convertToHAE   =true,
//            ElevationSamplePolicy           samplePolicy   =SAMPLE_FIRST_VALID,
//            ProgressCallback*               progress       =0L)  const;
//
//        /**
//         * Sets the Cache for this Map. Set to NULL for no cache.
//         */
//        void setCache( Cache* cache );
//
//        /**
//         * Gets the Cache for this Map
//         */
//        Cache* getCache() const;
//
//        /**
//         * Gets the revision # of the map. The revision # changes every time
//         * you add, remove, or move layers. You can use this to track changes
//         * in the map model (as a alternative to installing a MapCallback).
//         */
//        Revision getDataModelRevision() const;
//
//        /**
//         * Convenience function that returns TRUE if the map cs type is
//         * geocentric.
//         */
//        bool isGeocentric() const;
//
//        /**
//         * Synronizes a map frame to the current revision of the map's data model.
//         * Returns true if new Map model data was available and a sync occurred;
//         * returns false if nothing changed.
//         */
//        bool sync( class MapFrame& frame ) const;
//
//        enum ModelParts {
//            IMAGE_LAYERS     = 1 << 0,
//            ELEVATION_LAYERS = 1 << 1,
//            TERRAIN_LAYERS   = IMAGE_LAYERS | ELEVATION_LAYERS,
//            MODEL_LAYERS     = 1 << 2,
//            MASK_LAYERS      = 1 << 3,
//            MASKED_TERRAIN_LAYERS = TERRAIN_LAYERS | MASK_LAYERS,
//            ENTIRE_MODEL     = 0xff
//        };
//
//        /**
//         * Gets the database options associated with this map.
//         */
//        const osgDB::Options* getDBOptions() const { return _dbOptions.get(); }
//
//        const Profile* getProfileNoVDatum() const { return _profileNoVDatum.get(); }
//
      protected:

        virtual ~Map();

      private:
        Map( const osgEarth::Map& rh );
    };
};
