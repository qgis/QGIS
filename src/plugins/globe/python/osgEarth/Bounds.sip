/* -*-c++-*-
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

    /**
     * An "anonymous" bounding extent (i.e., no geo reference information)
     */
    class Bounds
    {
    public:
%TypeHeaderCode
#include <osgEarth/Bounds>
using namespace osgEarth;
using namespace osg;
%End
        Bounds();
        Bounds(double xmin, double ymin, double xmax, double ymax );

        /** Dtor */
        virtual ~Bounds();

        double width() const;
        double height() const;
        double depth() const;
        bool contains(double x, double y ) const;
        bool contains(const Bounds& rhs) const;
        Bounds unionWith(const Bounds& rhs) const;
        Bounds intersectionWith(const Bounds& rhs) const;
        void expandBy( double x, double y );
        void expandBy( double x, double y, double z );
        void expandBy( const Bounds& rhs );
        Vec2d center2d() const;
        double radius2d() const;
        double area2d() const;
        bool isValid() const;
        bool isEmpty() const;
    };
