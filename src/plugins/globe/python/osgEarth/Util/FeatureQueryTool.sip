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
  namespace Util
  {

    /**
     * Tool that let you query the map for Features generated from a FeatureSource.
     *
     * By default, an unmodified left-click will activate a query. You can replace
     * this test by calling setInputPredicate().
     */
    class FeatureQueryTool : osgGA::GUIEventHandler //, MapNodeObserver
    {
%TypeHeaderCode
#include <osgEarthUtil/FeatureQueryTool>
%End
    public:
        struct Callback  // : osg::Referenced
        {
            struct EventArgs
            {
//                const osgGA::GUIEventAdapter*  _ea;
//                osgGA::GUIActionAdapter*       _aa;
                osg::Vec3d                     _worldPoint;
            };

            // called when a valid feature is found under the mouse coords
            virtual void onHit( osgEarth::Features::FeatureSourceIndexNode* index, osgEarth::Features::FeatureID fid, const EventArgs& args );

            // called when no feature is found under the mouse coords
            virtual void onMiss( const EventArgs& args );
        };

        /**
         * Interface for a custom input test.
         */
        class InputPredicate// : osg::Referenced
        {
            public:
            // return true to active a query under the mouse cursor.
//            virtual bool accept( const osgGA::GUIEventAdapter& ea ) =0;
            private:
              InputPredicate(); // TODO remove once accept can be uncommented
        };

    public:
        /**
         * Constructs a new feature query tool.
         *
         * @param mapNode
         *      Map node containing feature data to query
         * @param callbackToAdd
         *      (optional) convenience; calls addCallback with this parameter
         */
        FeatureQueryTool(
            osgEarth::MapNode*  mapNode,
            Callback* callbackToAdd =0 );

        /** Dtor */
        virtual ~FeatureQueryTool();

        /**
         * Adds a feature query callback.
         */
        void addCallback( Callback* callback );

        /**
         * Removes a feature query callback.
         */

        /**
         * Sets a custom input test. By default, the action is a left-click.
         */
        void setInputPredicate( InputPredicate* value );


    public: // GUIEventHandler

//        virtual bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa );


    public: // MapNodeObserver

        virtual void setMapNode( osgEarth::MapNode* mapNode );

        virtual osgEarth::MapNode* getMapNode();

        typedef std::vector< osg::observer_ptr<Callback> > Callbacks;
    };

    //--------------------------------------------------------------------

    /**
     * Sample callback that will highlight a feature upon a query hit.
     */
    class FeatureHighlightCallback : osgEarth::Util::FeatureQueryTool::Callback
    {
    public:
        virtual void onHit( osgEarth::Features::FeatureSourceIndexNode* index, osgEarth::Features::FeatureID fid, const EventArgs& args );

        virtual void onMiss( const EventArgs& args );

    protected:
        void clear();

        /** Dtor */
        virtual ~FeatureHighlightCallback();
    };


    //--------------------------------------------------------------------

    /**
     * Sample callback that will display feature attributes upon a query hit.
     */
    class FeatureReadoutCallback : osgEarth::Util::FeatureQueryTool::Callback
    {
    public:
//        FeatureReadoutCallback( Controls::Container* container );

    public:
        virtual void onHit( osgEarth::Features::FeatureSourceIndexNode* index, osgEarth::Features::FeatureID fid, const EventArgs& args );

        virtual void onMiss( const EventArgs& args );

    protected:
        void clear();

        /** Dtor */
        virtual ~FeatureReadoutCallback();

    private:
        FeatureReadoutCallback();
    };
  };
}; // namespace osgEarthUtil
