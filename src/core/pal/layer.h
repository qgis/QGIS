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

#ifndef _LAYER_H_
#define _LAYER_H_

#include <fstream>

#include <pal/pal.h>
#include <pal/palgeometry.h>


namespace pal
{

  template <class Type> class LinkedList;
  template <class Type> class Cell;
  template <typename Data> class HashTable;

  template<class DATATYPE, class ELEMTYPE, int NUMDIMS, class ELEMTYPEREAL, int TMAXNODES, int TMINNODES> class RTree;

  class Feature;
  class FeaturePart;
  class Pal;
  class SimpleMutex;
  class LabelInfo;

  /**
   * \brief A layer of spacial entites
   *
   * a layer is a bog of feature with some data which influence the labelling process
   *
   *  \author Maxence Laurent <maxence _dot_ laurent _at_ heig-vd _dot_ ch>
   */
  class Layer
  {
      friend class Pal;
      friend class FeaturePart;

      friend class Problem;

      friend class LabelPosition;
      friend bool extractFeatCallback( FeaturePart *ft_ptr, void *ctx );
      friend void toSVGPath( int nbPoints, double *x, double *y, int dpi, Layer *layer, int type, char *uid, std::ostream &out, double scale, int xmin, int ymax, bool exportInfo, char *color );

    public:
      enum LabelMode { LabelPerFeature, LabelPerFeaturePart };

    protected:
      char *name; /* unique */

      /** list of feature parts */
      LinkedList<FeaturePart*> *featureParts;

      /** list of features - for deletion */
      LinkedList<Feature*> *features;

      Pal *pal;

      double defaultPriority;

      bool obstacle;
      bool active;
      bool toLabel;

      Units label_unit;

      double min_scale;
      double max_scale;

      Arrangement arrangement;

      LabelMode mode;
      bool mergeLines;

      /** optional flags used for some placement methods */
      unsigned long arrangementFlags;

      // indexes (spatial and id)
      RTree<FeaturePart*, double, 2, double, 8, 4> *rtree;
      HashTable<Feature*> *hashtable;

      HashTable< LinkedList<FeaturePart*>* > * connectedHashtable;
      LinkedList< char* >* connectedTexts;

      SimpleMutex *modMutex;

      /**
       * \brief Create a new layer
       *
       * @param lyrName layer's name
       * @param min_scale bellow this scale: no labeling
       * @param max_scale above this scale: no labeling
       * @param arrangement Arrangement mode : how to place candidates
       * @param label_unit Unit for labels sizes
       * @param defaultPriority layer's prioriry (0 is the best, 1 the worst)
       * @param obstacle 'true' will discourage other label to be placed above features of this layer
       * @param active is the layer is active (currently displayed)
       * @param toLabel the layer will be labeled whether toLablel is true
       * @param pal pointer to the pal object
       *
       */
      Layer( const char *lyrName, double min_scale, double max_scale, Arrangement arrangement, Units label_unit, double defaultPriority, bool obstacle, bool active, bool toLabel, Pal *pal );

      /**
       * \brief Delete the layer
       */
      virtual ~Layer();

      /**
       * \brief check if the scal is in the scale range min_scale -> max_scale
       * @param scale the scale to check
       */
      bool isScaleValid( double scale );

      /** add newly creted feature part into r tree and to the list */
      void addFeaturePart( FeaturePart* fpart, const char* labelText = NULL );

    public:
      /**
       * \brief get the number of features into layer
       */
      int getNbFeatures();

      /**
       * \brief get layer's name
       */
      const char * getName();


      /**
       *  \brief get arrangement policy
       */
      Arrangement getArrangement();

      /**
       * \brief set arrangement policy
       *
       * @param arrangement arrangement policy
       */
      void setArrangement( Arrangement arrangement );

      unsigned long getArrangementFlags() const { return arrangementFlags; }
      void setArrangementFlags( unsigned long flags ) { arrangementFlags = flags; }

      /**
       * \brief get units for label size
       */
      Units getLabelUnit();

      /**
       * \brief set unit for label size
       *
       */
      void setLabelUnit( Units label_unit );

      /**
       * \brief activate or desactivate the layer
       *
       * active means "is currently display". When active is false
       * feature of this layer will never be used (neither for
       * labelling nor as obstacles)
       *
       * @param active turn the layer active (true) or inactive (false)
       */
      void setActive( bool active );

      /**
       * \brief return the layer's activity status
       */
      bool isActive();


      /**
       * \brief tell pal whether the layer has to be labelled.
       *
       * The layer will be labelled if and only if toLabel and isActive were set to true
       *
       * @param toLabel set to false disable lbelling this layer
       */
      void setToLabel( bool toLabel );


      /**
       * \brief return if the layer will be labelled or not
       */
      bool isToLabel();


      /**
       * \brief mark layer's features as obstacles
       *
       * Avoid putting labels over obstalces.
       * isActive must also be true to consider feature as obstacles,
       * otherwise they will be ignored
       */
      void setObstacle( bool obstacle );

      /**
       * \brief return the obstacle status
       */
      bool isObstacle();

      /**
       * \brief set the minimum valid scale, below this scale the layer will not be labelled
       *
       * Use -1 to disable
       */
      void setMinScale( double min_scale );

      /**
       * \brief return the minimum valid scale
       */
      double getMinScale();


      /**
       * \brief set the maximum valid scale, upon this scale the layer will not be labelled
       *
       * use -1 to disable
       */
      void setMaxScale( double max_scale );


      /**
       * \brief return the maximum valid scale
       */
      double getMaxScale();


      /**
       * \ brief set the layer priority
       *
       * The best priority is 0, the worst is 1
       * Should be links with a slider in a nice gui
       */
      void setPriority( double priority );


      /**
       * return the layer's priority
       */
      double getPriority();

      void setLabelMode( LabelMode m ) { mode = m; }
      LabelMode getLabelMode() const { return mode; }

      void setMergeConnectedLines(bool m) { mergeLines = m; }
      bool getMergeConnectedLines() const { return mergeLines; }

      /**
       * \brief register a feature in the layer
       *
       * @param geom_id unique identifier
       * @param label_x label width
       * @param label_y label height
       * @param userGeom user's geometry that implements the PalGeometry interface
       *
       * @throws PalException::FeatureExists
       *
       * @return true on success (i.e. valid geometry)
       */
      bool registerFeature( const char *geom_id, PalGeometry *userGeom, double label_x = -1, double label_y = -1, const char* labelText = NULL );

      /** return pointer to feature or NULL if doesn't exist */
      Feature* getFeature( const char* geom_id );

      /** join connected features with the same label text */
      void joinConnectedFeatures();

  };

} // end namespace pal

#endif
