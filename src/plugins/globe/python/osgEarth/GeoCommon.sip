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
     * Types of interpolation between two geodetic locations.
     */
    enum GeoInterpolation
    {
        GEOINTERP_GREAT_CIRCLE,
        GEOINTERP_RHUMB_LINE
    };

    /**
     * Elevation interpolation methods.
     */
    enum ElevationInterpolation
    {
        INTERP_AVERAGE,
        INTERP_NEAREST,
        INTERP_BILINEAR,
        INTERP_TRIANGULATE
    };

    /**
     * Elevation stack sampling policy
     */
    enum ElevationSamplePolicy
    {
        SAMPLE_FIRST_VALID,
        SAMPLE_HIGHEST,
        SAMPLE_LOWEST,
        SAMPLE_AVERAGE
    };

    /**
     * Indicates how to interpret a Z coordinate.
     */
    enum AltitudeMode
    {
        ALTMODE_ABSOLUTE,  // Z value is the absolute height above MSL/HAE.
        ALTMODE_RELATIVE   // Z value is the height above the terrain elevation.
    };
