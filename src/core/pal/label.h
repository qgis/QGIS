/*
 *   libpal - Automated Placement of Labels Library
 *
 *   Copyright (C) 2008 Maxence Laurent, MIS-TIC, HEIG-VD
 *                      University of Applied Sciences, Western Switzerland
 *                      http://www.hes-so.ch
 *
 *   Contact:
 *      maxence.laurent <at> heig-vd <dot> ch
 *    or
 *      eric.taillard <at> heig-vd <dot> ch
 *
 * This file is part of libpal.
 *
 * libpal is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libpal is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libpal.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef _LABEL_H
#define _LABEL_H

namespace pal {

    class LabelPosition;
    class PalGeometry;

    /**
     * \brief Represent a label to be displayed
     */
    class Label {

        friend class LabelPosition;
    private:
        double x[4];
        double y[4];

        double a;

        char *featureId;
        char *lyrName;

        PalGeometry *userGeom;

        /**
         * \brief Create a new label
         *
         * @param x x coordinate of down-left label corner
         * @param y y coordinate of down-left label corner
         * @param alpha rotation to aplay to the text
         * @param ftid id of the corresponding feature
         * @param lyrName name of the corresponding layer
         * @param userGeom PalGeometry of the feature
         */
        Label (double x[4], double y[4], double alpha, const char *ftid, const char *lyrName, PalGeometry *userGeom);

    public:
        /**
         * \brief delete a label
         */
        ~Label();

        /**
         * \brief return the down-left x coordinate
         * @return x coordinate
         */
        double getOrigX();

        /**
         * \brief return the down-left y coordinate
         * @return y coordinate
         */
        double getOrigY();

        /**
         * \brief get a specific x coordinate
         * @param i 0 => down-left, 1=>down-right, 2=>up-right 3=> up-left
         * @return the i'th x coordinate
         */
        double getX (size_t i);

        /**
         * \brief get a specific y coordinate
         * @param i 0 => down-left, 1=>down-right, 2=>up-right 3=> up-left
         * @return the i'th y coordinate
         */
        double getY (size_t i);

        /**
         * \brief return the label orientation
         * @return alpha in rad, couterclockwise
         */
        double getRotation();

        /**
         * \brief return the name of the layer wich contains the feature
         * @return the layer's name
         */
        const char *getLayerName();

        /**
         * \brief return the feature's unique id
         * @return the feature's id
         */
        const char *getFeatureId();

        /**
         * \brief return user geometry (pal::Layer::registerFeature())
         * @return pointer to the user geometry
         */
        PalGeometry * getGeometry();
    };

} // end namespace pal

#endif
