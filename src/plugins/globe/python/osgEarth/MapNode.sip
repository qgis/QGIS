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
#include <osgEarth/TraversalData>
#include <osgEarth/MapNode>
%End

    /**
     * OSG Node that forms the root of an osgEarth map. This node is a "view" component
     * that renders data from a "Map" data model.
     */
    class MapNode : osg::Group
    {
    public:
        /**
         * Attempts to load a MapNOde from a ".earth" file in the arguments list
         */
//        static MapNode* load( class osg::ArgumentParser& arguments );

    public:
        /**
         * Creates an empty map node.
         */
        MapNode();

//        /**
//         * Creates an empty map node.
//         *
//         * @param options
//         *      Tile creation and rendering properties
//         */
//        MapNode( const MapNodeOptions& options );
//
//        /**
//         * Creates a new map node.
//         *
//         * @param map
//         *      Map data that this map node will render.
//         */
//        MapNode( Map* map );
//
//        /**
//         * Creates a new map node.
//         *
//         * @param map
//         *      Map data that this map node will render.
//         * @param options
//         *      Tile creation and rendering properties
//         */
//        MapNode( Map* map, const MapNodeOptions& options );
//
//    public:
//
//        virtual const char* libraryName() const { return "osgEarth"; }
//        virtual const char* className() const { return "MapNode"; }
//
        /**
         * Gets the Map that this MapNode is rendering.
         */
        osgEarth::Map* getMap();

        /**
         * Gets the spatial reference system of the underlying map.
         * Convenience function.
         */
        const osgEarth::SpatialReference* getMapSRS() const;

        /**
         * Gets an interface for querying the in-memory terrain scene graph directly.
         */
        osgEarth::Terrain* getTerrain();

        /**
         * Finds the topmost Map node in the specified scene graph, or returns NULL if
         * no Map node exists in the graph.
         *
         * @param graph
         *      Node graph in which to search for a MapNode
         * @param travMask
         *      Traversal mask to apply while searching
         */
        static osgEarth::MapNode* findMapNode( osg::Node* graph, unsigned travMask =~0 );
        static osgEarth::MapNode* get( osg::Node* graph, unsigned travMask =~0 );

        /**
         * Returns true if the realized terrain model is geocentric, false if
         * it is flat/projected.
         */
        bool isGeocentric() const;

        /**
         * Accesses the group node that contains all the ModelLayers.
         */
        osg::Group* getModelLayerGroup() const;
//
//        /**
//         * Accesses the root node for a specific ModelLayer.
//         */
//        osg::Node* getModelLayerNode( ModelLayer* layer ) const;
//
//        /**
//         * Adds a node that decorates the terrain groups
//         */
//        void addTerrainDecorator( osg::Group* decorator );
//
//        /**
//         * Removes a node previously added via addTerrainDecorator.
//         */
//        void removeTerrainDecorator( osg::Group* decorator );
//
//        /**
//         * Gets the overlay decorator in this mapnode. Usually you do not need to
//         * access this directly. Instead install a DrapeableNode in the scene graph.
//         */
//        OverlayDecorator* getOverlayDecorator() { return _overlayDecorator; }
//
//        /**
//         * Gets the engine properties associated with this node. The engine
//         * properties dictate how the map engine will create scene graph elements.
//         */
//        const MapNodeOptions& getMapNodeOptions() const;
//
//        /**
//         * Gets the underlying terrain engine that renders the terrain surface of the map.
//         */
//        TerrainEngineNode* getTerrainEngine() const;
//
//        /**
//         * Gets the Config object serializing external data. External data is information
//         * that osgEarth itself does not control, but that an app can include in the
//         * MapNode for the purposes of including it in a .earth file.
//         */
//        Config& externalConfig() { return _externalConf; }
//        const Config& externalConfig() const { return _externalConf; }
//
//        /**
//         * Sets a custom texture compositor technique on the underlying terrain engine.
//         * This method is here b/c just calling getTerrainEngine()->getTextureCompositor()
//         * ->setTechnique() has problems (with init order of the terrain engine). Someday
//         * this needs to be cleaned up.
//         */
//        void setCompositorTechnique( class TextureCompositorTechnique* tech );
//
//    public: // special purpose
//
//        /**
//         * Constructs a mapnode, optionally specifying whether th intialize the
//         * data sources in the map. Typically you would only use this CTOR if you are
//         * strictly using the MapNode for serialization.
//         */
//        MapNode( Map* map, const MapNodeOptions& options, bool initMap );
//
//
//    public: //override osg::Node
//
//        virtual osg::BoundingSphere computeBound() const;
//
//        virtual void traverse( class osg::NodeVisitor& nv );
//
//    public: // MapCallback proxy
//
//        void onModelLayerAdded( ModelLayer*, unsigned int );
//        void onModelLayerRemoved( ModelLayer* );
//		void onModelLayerMoved( ModelLayer* layer, unsigned int oldIndex, unsigned int newIndex );
//
//    public:
//        void onModelLayerOverlayChanged( ModelLayer* layer );
//
//    public:
//        struct TileRangeData : public osg::Referenced {
//            TileRangeData(double minRange, double maxRange) : _minRange( minRange ), _maxRange( maxRange ) { }
//            double _minRange;
//            double _maxRange;
//        };
//

      private:
        // Copying not allowed
        MapNode( const osgEarth::MapNode& rh );
        ~MapNode();
    };

}; // namespace osgEarth
