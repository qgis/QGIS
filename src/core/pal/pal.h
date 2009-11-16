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

#ifndef _PAL_H
#define _PAL_H


#include <list>
#include <iostream>

#ifdef _MSC_VER
#include <time.h>
#endif

// TODO ${MAJOR} ${MINOR} etc instead of 0.2

/**
 * \mainpage Pal Libray
 *
 * \section intro_sec Introduction
 *
 * Pal is a labelling library released under the GPLv3 license
 *
 */

namespace pal
{

  template <class Type> class LinkedList;

  class Layer;
  class LabelPosition;
  class PalStat;
  class Problem;
  class PointSet;
  class SimpleMutex;

  /** Units for label sizes and distlabel */
  enum _Units
  {
    PIXEL = 0, /**< pixel [px]*/
    METER, /**< meter [m]*/
    FOOT, /**< foot [ft]*/
    DEGREE /**< degree [°] */
  };

  /** Typedef for _Units enumeration */
  typedef enum _Units Units;

  /** Search method to use */
  enum _searchMethod
  {
    CHAIN = 0, /**< is the worst but fastest method */
    POPMUSIC_TABU_CHAIN = 1, /**< is the best but slowest */
    POPMUSIC_TABU = 2, /**< is a little bit better than CHAIN but slower*/
    POPMUSIC_CHAIN = 3, /**< is slower and best than TABU, worse and faster than TABU_CHAIN */
    FALP = 4 /** only initial solution */
  };

  /** Typedef for _Units enumeration */
  typedef enum _searchMethod SearchMethod;

  /** The way to arrange labels against spatial entities
   *
   * \image html arrangement.png "Arrangement modes" width=7cm
   * */
  enum _arrangement
  {
    P_POINT = 0, /**< arranges candidates around a point (centroid for polygon)*/
    P_POINT_OVER, /** arranges candidates over a point (centroid for polygon)*/
    P_LINE, /**< Only for lines and polygons, arranges candidates over the line or the polygon perimeter */
    P_CURVED, /** Only for lines, labels along the line */
    P_HORIZ, /**< Only for polygon, arranges candidates horizontaly */
    P_FREE /**< Only for polygon, arranges candidates with respect of polygon orientation */
  };

  /** typedef for _arrangement enumeration */
  typedef enum _arrangement Arrangement;

  /** enumeration line arrangement flags. Flags can be combined. */
  enum LineArrangementFlags
  {
    FLAG_ON_LINE     = 1,
    FLAG_ABOVE_LINE  = 2,
    FLAG_BELOW_LINE  = 4,
    FLAG_MAP_ORIENTATION = 8
  };

  /**
   *  \brief Pal main class.
   *
   *  A pal object will contains layers and global informations such as which search method
   *  will be used, the map resolution (dpi) ....
   *
   *  \author Maxence Laurent <maxence _dot_ laurent _at_ heig-vd _dot_ ch>
   */
  class CORE_EXPORT Pal
  {
      friend class Problem;
      friend class FeaturePart;
      friend class Layer;
    private:
      std::list<Layer*> * layers;

      SimpleMutex *lyrsMutex;

      // TODO remove after tests !!!
      clock_t tmpTime;

      Units map_unit;

      /**
       * \brief maximum # candidates for a point
       */
      int point_p;

      /**
       * \brief maximum # candidates for a line
       */
      int line_p;

      /**
       * \brief maximum # candidates for a polygon
       */
      int poly_p;

      SearchMethod searchMethod;

      /*
       * POPMUSIC Tuning
       */
      int popmusic_r;

      int tabuMaxIt;
      int tabuMinIt;

      int dpi;

      int ejChainDeg;
      int tenure;
      double candListSize;

      /**
       * \brief Problem factory
       * Extract features to label and generates candidates for them,
       * respects to a bounding box and a map scale
       *
       * @param nbLayers  number of layers to extract
       * @param layersName layers name to be extracted
       * @param layersFactor layer's factor (priority between layers, 0 is the best, 1 the worst)
       * @param lambda_min xMin bounding-box
       * @param phi_min yMin bounding-box
       * @param lambda_max xMax bounding-box
       * @param phi_max yMax bounding-box
       * @param scale the scale (1:scale)
       * @param svgmap stream to wrtie the svg map (need _EXPORT_MAP_ #defined to work)
       */
      Problem* extract( int nbLayers, char **layersName, double *layersFactor,
                        double lambda_min, double phi_min,
                        double lambda_max, double phi_max,
                        double scale, std::ofstream *svgmap );


      /**
       * \brief Choose the size of popmusic subpart's
       * @param r subpart size
       */
      void setPopmusicR( int r );



      /**
       * \brief minimum # of iteration for search method POPMUSIC_TABU, POPMUSIC_CHAIN and POPMUSIC_TABU_CHAIN
       * @param min_it Sub part optimization min # of iteration
       */
      void setMinIt( int min_it );

      /**
       * \brief maximum \# of iteration for search method POPMUSIC_TABU, POPMUSIC_CHAIN and POPMUSIC_TABU_CHAIN
       * @param max_it Sub part optimization max # of iteration
       */
      void setMaxIt( int max_it );

      /**
       * \brief For tabu search : how many iteration a feature will be tabu
       * @param tenure consiser a feature as tabu for tenure iteration after updating feature in solution
       */
      void setTenure( int tenure );

      /**
       * \brief For *CHAIN, select the max size of a transformation chain
       * @param degree maximum soze of a transformation chain
       */
      void setEjChainDeg( int degree );

      /**
       * \brief How many candidates will be tested by a tabu iteration
       * @param fact the ration (0..1) of candidates to test
       */
      void setCandListSize( double fact );


      /**
       * \brief Get the minimum # of iteration doing in POPMUSIC_TABU, POPMUSIC_CHAIN and POPMUSIC_TABU_CHAIN
       * @return minimum # of iteration
       */
      int getMinIt();
      /**
       * \brief Get the maximum # of iteration doing in POPMUSIC_TABU, POPMUSIC_CHAIN and POPMUSIC_TABU_CHAIN
       * @return maximum # of iteration
       */
      int getMaxIt();


    public:

      /**
       * \brief Create an new pal instance
       */
      Pal();

      /**
       * \brief delete an instance
       */
      ~Pal();

      /**
       * \brief add a new layer
       *
       * @param lyrName layer's name
       * @param min_scale bellow this scale: no labelling (-1 to disable)
       * @param max_scale above this scale: no labelling (-1 to disable)
       * @param arrangement Howto place candidates
       * @param label_unit Unit for labels sizes
       * @param defaultPriority layer's prioriry (0 is the best, 1 the worst)
       * @param obstacle 'true' will discourage other label to be placed above features of this layer
       * @param active is the layer is active (currently displayed)
       * @param toLabel the layer will be labeled only if toLablel is true
       *
       * @throws PalException::LayerExists
       *
       * @todo add symbolUnit
       */
      Layer * addLayer( const char *lyrName, double min_scale, double max_scale, Arrangement arrangement, Units label_unit, double defaultPriority, bool obstacle, bool active, bool toLabel );

      /**
       * \brief Look for a layer
       *
       * @param lyrName name of layer to search
       *
       * @throws PalException::UnkownLayer
       *
       * @return a pointer on layer or NULL if layer not exist
       */
      Layer *getLayer( const char *lyrName );

      /**
       * \brief get all layers
       *
       * @return a list of all layers
       */
      std::list<Layer*> *getLayers();

      /**
       * \brief remove a layer
       *
       * @param layer layer to remove
       */
      void removeLayer( Layer *layer );

      /**
       * \brief the labeling machine
       * Will extract all active layers
       *
       * @param scale map scale is 1:scale
       * @param bbox map extent
       * @param stats A PalStat object (can be NULL)
       * @param displayAll if true, all feature will be labelled evan though overlaps occurs
       *
       * @return A list of label to display on map
       */
      std::list<LabelPosition*> *labeller( double scale, double bbox[4], PalStat **stats, bool displayAll );


      /**
       * \brief the labeling machine
       * Active layers are specifiend through layersName array
       * @todo add obstacles and tolabel arrays
       *
       * @param nbLayers # layers
       * @param layersName names of layers to label
       * @param layersFactor layers priorities array
       * @param scale map scale is  '1:scale'
       * @param bbox map extent
       * @param stat will be filled with labelling process statistics, can be NULL
       * @param displayAll if true, all feature will be labelled evan though overlaps occurs
       *
       * @todo UnknownLayer will be ignored ? should throw exception or not ???
       *
       * @return A list of label to display on map
       */
      std::list<LabelPosition*> *labeller( int nbLayers,
                                   char **layersName,
                                   double *layersFactor,
                                   double scale, double bbox[4],
                                   PalStat **stat,
                                   bool displayAll );


      Problem* extractProblem(double scale, double bbox[4]);

      std::list<LabelPosition*>* solveProblem(Problem* prob, bool displayAll);

      /**
       * \brief Set map resolution
       *
       * @param dpi map resolution (dot per inch)
       */
      void setDpi( int dpi );

      /**
       * \brief get map resolution
       *
       * @return map resolution (dot per inch)
       */
      int getDpi();



      /**
       * \brief set # candidates to generate for points features
       * Higher the value is, longer Pal::labeller will spend time
       *
       * @param point_p # candidates for a point
       */
      void setPointP( int point_p );

      /**
       * \brief set maximum # candidates to generate for lines features
       * Higher the value is, longer Pal::labeller will spend time
       *
       * @param line_p maximum # candidates for a line
       */
      void setLineP( int line_p );

      /**
       * \brief set maximum # candidates to generate for polygon features
       * Higher the value is, longer Pal::labeller will spend time
       *
       * @param poly_p maximum # candidate for a polygon
       */
      void setPolyP( int poly_p );

      /**
       *  \brief get # candidates to generate for point features
       */
      int getPointP();

      /**
       *  \brief get maximum  # candidates to generate for line features
       */
      int getLineP();

      /**
       *  \brief get maximum # candidates to generate for polygon features
       */
      int getPolyP();

      /**
       * \brief get current map unit
       */
      Units getMapUnit();

      /**
       * \brief set map unit
       */
      void setMapUnit( Units map_unit );

      /**
       * \brief Select the search method to use.
       *
       * For interactive mapping using CHAIN is a good
       * idea because it is the fastest. Other methods, ordered by speedness, are POPMUSIC_TABU,
       * POPMUSIC_CHAIN and POPMUSIC_TABU_CHAIN, defined in pal::_searchMethod enumeration
       * @param method the method to use
       */
      void setSearch( SearchMethod method );

      /**
       * \brief get the search method in use
       *
       * @return the search method
       */
      SearchMethod getSearch();
  };
} // end namespace pal
#endif
