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

namespace osgEarth {
  namespace Features {
    /**
     * Options for a feature index
     */
    class FeatureSourceIndexOptions
    {
%TypeHeaderCode
#include <osgEarthFeatures/FeatureSourceIndexNode>
%End
    public:
        FeatureSourceIndexOptions( const osgEarth::Config& conf =osgEarth::Config() );

        /** Wheter to embed the actual Feature objects in the index (instead of
         *  just the FeatureID). This is useful for feature sources that cannot
         *  be queried by ID (e.g., streaming data like TFS) */
//        optional<bool>& embedFeatures() { return _embedFeatures; }
//        const optional<bool>& embedFeatures() const { return _embedFeatures; }

    public:
        osgEarth::Config getConfig() const;

    private:
//        optional<bool> _embedFeatures;
    };


    /**
     * Interface for feature indexing.
     */
    class FeatureSourceIndex
    {
%TypeHeaderCode
#include <osgEarthFeatures/FeatureSourceIndexNode>
%End
    public: // tagging functions
//        virtual void tagPrimitiveSets( osg::Drawable* drawable, Feature* feature ) const =0;
        virtual void tagNode( osg::Node* node, osgEarth::Features::Feature* feature ) const =0;

        virtual ~FeatureSourceIndex();
    private:
        FeatureSourceIndex();
    };

    /**
     * Maintains an index that maps FeatureID's from a FeatureSource to
     * PrimitiveSets within the subgraph's geometry.
     */
    class FeatureSourceIndexNode : osg::Group, osgEarth::Features::FeatureSourceIndex
    {
    public:
        /**
         * Constructs a new index node.
         */
//        FeatureSourceIndexNode(
//            osgEarth::Features::FeatureSource*                   featureSource,
//            const FeatureSourceIndexOptions& options );

        virtual ~FeatureSourceIndexNode();


    public: // FeatureSourceIndex

        /**
         * Tags all the primitive sets in a Drawable with the specified FeatureID.
         */
//        void tagPrimitiveSets( osg::Drawable* drawable, Feature* feature ) const;

        /**
         * Tags a node with the specified FeatureID.
         */
        void tagNode( osg::Node* node, osgEarth::Features::Feature* feature ) const;


    public:
        /**
         * The feature source tied to this node
         */
//        osgEarth::Features::FeatureSource* getFeatureSource();

        /**
         * Traverses this node's subgraph and rebuilds the feature index based on
         * any tagged drawables found. (See tagPrimitiveSets for tagging drawables).
         */
        void reindex();

        /**
         * Given a primitive set, returns the feature ID corresponding to that set.
         *
         * @param pset Primitive set to query
         * @param output Holds the result of the query, if returning true
         * @return true if successful
         */
//        bool getFID(osg::PrimitiveSet* pset, FeatureID& output) const;

        /**
         * Gets the Feature ID corresponding to a drawable and a prim index. This is
         * useful to call using the results of an intersection test.
         *
         * @param drawable       Drawable for which to lookup the feature ID
         * @param primitiveIndex Index of the primitive to look up
         * @param output         Holds the result of the query, if returning true
         * @return true if successful
         */
//        bool getFID(osg::Drawable* drawable, int primitiveIndex, FeatureID& output) const;

        /**
         * Given a FeatureID, returns the collection of drawable/primitiveset combinations
         * corresponding to that feature.
         *
         * @param fid Feature ID to look up
         * @return Corresponding collection of primitive sets (empty if the query fails)
         */
//        FeatureDrawSet& getDrawSet( const FeatureID& fid );

        /**
         * Given a FeatureID, returns the cached feature.
         *
         * @param fid     Feature ID to look up
         * @param output  cached feature
         * @return true if successful
         */
        bool getFeature(const osgEarth::Features::FeatureID& fid, const osgEarth::Features::Feature*& output) const;

    public:
        virtual const char* className() const;
        virtual const char* libraryName() const;

    private:
        FeatureSourceIndexNode();
    };

  };
}; // namespace osgEarth::Features
