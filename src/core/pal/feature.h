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

#ifndef _FEATURE_H
#define _FEATURE_H

#include <iostream>
#include <fstream>
#include <cmath>

#include <geos_c.h>

#include <pal/palgeometry.h>

#include "pointset.h"
#include "util.h"


namespace pal
{
  /** optional additional info about label (for curved labels) */
  class LabelInfo
  {
  public:
    typedef struct
    {
        ushort chr;
        double width;
    } CharacterInfo;

    LabelInfo(int num, double height)
    {
      max_char_angle_delta = 20;
      label_height = height;
      char_num = num;
      char_info = new CharacterInfo[num];
    }
    ~LabelInfo() { delete [] char_info; }

    double max_char_angle_delta;
    double label_height;
    int char_num;
    CharacterInfo* char_info;
  };

  class LabelPosition;

  /**
   * \brief Main class to handle feature
   */
  class Feature : public PointSet
  {

    protected:
      //int id;   /* feature no id into layer */
      double label_x;
      double label_y;
      LabelInfo* labelInfo; // optional

      int nbSelfObs;
      PointSet **selfObs;

      char *uid;
      Layer *layer;

      double distlabel;

      GEOSGeometry *the_geom;
      int currentAccess;

      int nPart;
      int part;

      PalGeometry *userGeom;

      SimpleMutex *accessMutex;

public:
      /**
       * \brief generate candidates for point feature
       * Generate candidates for point features
       * \param x x coordinates of the point
       * \param y y coordinates of the point
       * \param scale map scale is 1:scale
       * \param lPos pointer to an array of candidates, will be filled by generated candidates
       * \return the number of generated cadidates
       */
      int setPositionForPoint( double x, double y, double scale, LabelPosition ***lPos, double delta_width );

      /**
       * generate one candidate over specified point
       */
      int setPositionOverPoint( double x, double y, double scale, LabelPosition ***lPos, double delta_width );

      /**
       * \brief generate candidates for line feature
       * Generate candidates for line features
       * \param scale map scale is 1:scale
       * \param lPos pointer to an array of candidates, will be filled by generated candidates
       * \param mapShape a pointer to the line
       * \return the number of generated cadidates
       */
      int setPositionForLine( double scale, LabelPosition ***lPos, PointSet *mapShape, double delta_width );

      LabelPosition* curvedPlacementAtOffset( PointSet* path_positions, double* path_distances,
                                                      int orientation, int index, double distance );

      /**
       * Generate curved candidates for line features
       */
      int setPositionForLineCurved( LabelPosition ***lPos, PointSet* mapShape );

      /**
       * \brief generate candidates for point feature
       * Generate candidates for point features
       * \param scale map scale is 1:scale
       * \param lPos pointer to an array of candidates, will be filled by generated candidates
       * \param mapShape a pointer to the polygon
       * \return the number of generated cadidates
       */
      int setPositionForPolygon( double scale, LabelPosition ***lPos, PointSet *mapShape, double delta_width );


      /**
       * \brief Feature against problem bbox
       * \param bbox[0] problem x min
       * \param bbox[1] problem x max
       * \param bbox[2] problem y min
       * \param bbox[3] problem y max
       * return A set of feature which are in the bbox or null if the feature is in the bbox
       */
      //LinkedList<Feature*> *splitFeature( double bbox[4]);


      /**
        * \brief create a new generic feature
        *
        * \param feat a pointer for a Feat which contains the spatial entites
        * \param layer feature is in this layer
        * \param part which part of the collection is this feature for ?
        * \param nPart how many feats have same uid (MULTI..., Collection)
        */
      Feature( Feat *feat, Layer *layer, int part, int nPart, PalGeometry *userGeom );


      /**
      * \brief Used to load pre-computed feature
      * \param file the file open by  Pal::Pal(const char *pal_file)
      * \param layer feature is in this layer
      */
      Feature( std::ifstream *file, Layer *layer );

      /**
       * \brief Delete the feature
       */
      virtual ~Feature();

      /**
       * \brief return the feature id
       * \return the feature id
       */
      //int getId();

      /**
       * \brief return the layer that feature belongs to
       * \return the layer of the feature
       */
      Layer * getLayer();

      /**
       * \brief save the feature into file
       * Called by Pal::save()
       * \param file the file to write
       */
      //void save(std::ofstream *file);

      /**
       * \brief generic method to generate candidates
       * This method will call either setPositionFromPoint(), setPositionFromLine or setPositionFromPolygon
       * \param scale the map scale is 1:scale
       * \param lPos pointer to candidates array in which candidates will be put
       * \param bbox_min min values of the map extent
       * \param bbox_max max values of the map extent
       * \param mapShape generate candidates for this spatial entites
       * \param candidates index for candidates
       * \param svgmap svg map file
       * \return the number of candidates in *lPos
       */
      int setPosition( double scale, LabelPosition ***lPos, double bbox_min[2], double bbox_max[2], PointSet *mapShape, RTree<LabelPosition*, double, 2, double>*candidates
#ifdef _EXPORT_MAP_
                       , std::ofstream &svgmap
#endif
                     );

      /**
       * \brief get the unique id of the feature
       * \return the feature unique identifier
       */
      const char *getUID();


      /**
       * \brief Print feature informations
       * Print feature unique id, geometry type, points, and holes on screen
       */
      void print();

      //void toSVGPath(std::ostream &out, double scale, int xmin, int ymax, bool exportInfo);
      /**
       * \brief Draw the feature and its candidates in a svg file
       * The svg file will be uid-scale-text.svg. For DEBUG Purpose
       * \param text a text to append on filename
       * \param scale the scale of the drawing
       * \param nbp number of candidats in lPos
       * \param lPos array of candidates
       */
      //void toSvg(char *text, double scale, int nbp, LabelPosition **lPos);

      /**
          * \brief Draw the feature and the convexe polygon bounding box
          * The svg file will be uid-scale-text.svg. For DEBUG purpose
          * \param text a text to append on filename
          * \param scale the scale of the drawing
          * \param nbp number of candidats in lPos
          * \param lPos array of candidates
          */
      //void toSvg(char *text, double scale, int nb_bb, CHullBox **bbox);


      void deleteCoord();

      void fetchCoordinates();
      void releaseCoordinates();



      PalGeometry* getUserGeometry() { return userGeom; }

      void setLabelSize(double x, double y) { label_x = x; label_y = y; }
      double getLabelWidth() const { return label_x; }
      double getLabelHeight() const { return label_y; }

      int getNumParts() const { return nPart; }

      void setLabelDistance(double dist) { distlabel = dist; }
      double getLabelDistance() const { return distlabel; }

      int getNumSelfObstacles() const { return nbSelfObs; }
      PointSet* getSelfObstacle(int i) { return selfObs[i]; }

      void setLabelInfo(LabelInfo* info) { labelInfo = info; }


  };

} // end namespace pal

#endif
