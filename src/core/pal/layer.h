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

#ifndef _LAYER_H_
#define _LAYER_H_

#include "pal.h"
#include "palgeometry.h"
#include <QMutex>
#include <QLinkedList>
#include <QHash>
#include <fstream>

namespace pal
{

  template<class DATATYPE, class ELEMTYPE, int NUMDIMS, class ELEMTYPEREAL, int TMAXNODES, int TMINNODES> class RTree;

  class Feature;
  class FeaturePart;
  class Pal;
  class LabelInfo;

  /**
   * \brief A layer of spacial entites
   *
   * a layer is a bog of feature with some data which influence the labelling process
   *
   *  \author Maxence Laurent (maxence _dot_ laurent _at_ heig-vd _dot_ ch)
   */
  class CORE_EXPORT Layer
  {
      friend class Pal;
      friend class FeaturePart;

      friend class Problem;

      friend class LabelPosition;
      friend bool extractFeatCallback( FeaturePart *ft_ptr, void *ctx );

    public:
      enum LabelMode { LabelPerFeature, LabelPerFeaturePart };
      enum UpsideDownLabels
      {
        Upright, // upside-down labels (90 <= angle < 270) are shown upright
        ShowDefined, // show upside down when rotation is layer- or data-defined
        ShowAll // show upside down for all labels, including dynamic ones
      };

      virtual ~Layer();

      bool displayAll() const { return mDisplayAll; }

      /** Returns the number of features in layer.
       */
      int featureCount() { return features->size(); }

      /** Returns the layer's name.
       */
      QString name() const { return mName; }

      /** Returns the layer's arrangement policy.
       * @see setArrangement
       */
      Arrangement arrangement() const { return mArrangement; }

      /** Sets the layer's arrangement policy.
       * @param arrangement arrangement policy
       * @see arrangement
       */
      void setArrangement( Arrangement arrangement ) { mArrangement = arrangement; }

      /** Returns the layer's arrangement flags.
       * @see setArrangementFlags
       */
      LineArrangementFlags arrangementFlags() const { return mArrangementFlags; }

      /** Sets the layer's arrangement flags.
       * @param flags arrangement flags
       * @see arrangementFlags
       */
      void setArrangementFlags( LineArrangementFlags flags ) { mArrangementFlags = flags; }

      /**
       * \brief Sets whether the layer is currently active.
       *
       * Active means "is currently displayed or used as obstacles". When a layer is
       * deactivated then feature of this layer will not be used for either
       * labelling or as obstacles.
       *
       * @param active set to true to make the layer active, or false to deactivate the layer
       * @see active
       */
      void setActive( bool active ) { mActive = active; }

      /** Returns whether the layer is currently active.
       * @see setActive
       */
      bool active() const { return mActive; }

      /** Sets whether the layer will be labeled.
       * @note Layers are labelled if and only if labelLayer and active are true
       * @param toLabel set to false disable labeling this layer
       * @see labelLayer
       * @see setActive
       */
      void setLabelLayer( bool toLabel ) { mLabelLayer = toLabel; }

      /** Returns whether the layer will be labeled or not.
       * @see setLabelLayer
       */
      bool labelLayer() const { return mLabelLayer; }

      /** Sets whether features within the layer will act as obstacles for labels.
       * @note this property is only effective if the layer is active
       * @param obstacle set to true if features will act as obstacles
       * @see setActive
       * @see obstacle
       */
      void setObstacle( bool obstacle ) { mObstacle = obstacle; }

      /** Returns whether features within the layer act as obstacles for labels.
       * @see setObstacle
       */
      bool obstacle() const { return mObstacle; }

      /** Returns the obstacle type, which controls how features within the layer
       * act as obstacles for labels.
       * @see setObstacleType
       */
      ObstacleType obstacleType() const { return mObstacleType; }

      /** Sets the obstacle type, which controls how features within the layer
       * act as obstacles for labels.
       * @param obstacleType new obstacle type
       * @see obstacleType
       */
      void setObstacleType( ObstacleType obstacleType ) { mObstacleType = obstacleType; }

      /** Sets the layer's priority.
       * @param priority layer priority, between 0 and 1. 0 corresponds to highest priority,
       * 1 to lowest priority.
       * @see priority
       */
      void setPriority( double priority );

      /** Returns the layer's priority, between 0 and 1. 0 corresponds to highest priority,
       * 1 to lowest priority.
       * @see setPriority
       */
      double priority() const { return mDefaultPriority; }

      /** Sets the layer's labeling mode.
       * @param mode label mode
       * @see labelMode
       */
      void setLabelMode( LabelMode mode ) { mMode = mode; }

      /** Returns the layer's labeling mode.
       * @see setLabelMode
       */
      LabelMode labelMode() const { return mMode; }

      /** Sets whether connected lines should be merged before labeling
       * @param merge set to true to merge connected lines
       * @see mergeConnectedLines
       */
      void setMergeConnectedLines( bool merge ) { mMergeLines = merge; }

      /** Returns whether connected lines will be merged before labeling.
       * @see setMergeConnectedLines
       */
      bool mergeConnectedLines() const { return mMergeLines; }

      /** Sets how upside down labels will be handled within the layer.
       * @param ud upside down label handling mode
       * @see upsideDownLabels
       */
      void setUpsidedownLabels( UpsideDownLabels ud ) { mUpsidedownLabels = ud; }

      /** Returns how upside down labels are handled within the layer.
       * @see setUpsidedownLabels
       */
      UpsideDownLabels upsidedownLabels() const { return mUpsidedownLabels; }

      /** Sets whether labels placed at the centroid of features within the layer
       * are forced to be placed inside the feature's geometry.
       * @param forceInside set to true to force centroid labels to be within the
       * feature. If set to false then the centroid may fall outside the feature.
       * @see centroidInside
       */
      void setCentroidInside( bool forceInside ) { mCentroidInside = forceInside; }

      /** Returns whether labels placed at the centroid of features within the layer
       * are forced to be placed inside the feature's geometry.
       * @see setCentroidInside
       */
      bool centroidInside() const { return mCentroidInside; }

      /** Sets whether labels which do not fit completely within a polygon feature
       * are discarded.
       * @param fitInPolygon set to true to discard labels which do not fit within
       * polygon features. Set to false to allow labels which partially fall outside
       * the polygon.
       * @see fitInPolygonOnly
       */
      void setFitInPolygonOnly( bool fitInPolygon ) { mFitInPolygon = fitInPolygon; }

      /** Returns whether labels which do not fit completely within a polygon feature
       * are discarded.
       * @see setFitInPolygonOnly
       */
      bool fitInPolygonOnly() const { return mFitInPolygon; }

      /** Register a feature in the layer.
       * @param geom_id unique identifier
       * @param userGeom user's geometry that implements the PalGeometry interface
       * @param label_x label width
       * @param label_y label height
       * @param labelText label text
       * @param labelPosX x position of the label (in case of fixed label position)
       * @param labelPosY y position of the label (in case of fixed label position)
       * @param fixedPos true if a single fixed position for this label is needed
       * @param angle fixed angle (in radians) to rotate the label
       * @param fixedAngle whether to use provided fixed angle
       * @param xQuadOffset move label to quadrant: left, don't move, right (-1, 0, 1)
       * @param yQuadOffset move label to quadrant: down, don't move, up (-1, 0, 1)
       * @param xOffset map unit (+/-) to x-offset the label
       * @param yOffset map unit (+/-) to y-offset the label
       * @param alwaysShow whether to skip priority and always show the label (causes overlapping)
       * @param repeatDistance distance for repeating the label
       *
       * @throws PalException::FeatureExists
       *
       * @return true on success (i.e. valid geometry)
       */
      bool registerFeature( const QString &geom_id, PalGeometry *userGeom, double label_x = -1, double label_y = -1,
                            const QString& labelText = QString(), double labelPosX = 0.0, double labelPosY = 0.0,
                            bool fixedPos = false, double angle = 0.0, bool fixedAngle = false,
                            int xQuadOffset = 0, int yQuadOffset = 0, double xOffset = 0.0, double yOffset = 0.0,
                            bool alwaysShow = false, double repeatDistance = 0 );

      /** Return pointer to feature or NULL if doesn't exist */
      Feature* getFeature( const QString &geom_id );

      /** Join connected features with the same label text */
      void joinConnectedFeatures();

      /** Chop layer features at the repeat distance **/
      void chopFeaturesAtRepeatDistance();

    protected:
      QString mName; /* unique */

      /** List of feature parts */
      QLinkedList<FeaturePart*> *featureParts;

      /** List of features - for deletion */
      QLinkedList<Feature*> *features;

      Pal *pal;

      double mDefaultPriority;

      bool mObstacle;
      ObstacleType mObstacleType;
      bool mActive;
      bool mLabelLayer;
      bool mDisplayAll;
      bool mCentroidInside;
      bool mFitInPolygon;

      /** Optional flags used for some placement methods */
      Arrangement mArrangement;
      LineArrangementFlags mArrangementFlags;
      LabelMode mMode;
      bool mMergeLines;

      UpsideDownLabels mUpsidedownLabels;

      // indexes (spatial and id)
      RTree<FeaturePart*, double, 2, double, 8, 4> *rtree;
      QHash< QString, Feature*> *hashtable;

      QHash< QString, QLinkedList<FeaturePart*>* >* connectedHashtable;
      QLinkedList< QString >* connectedTexts;

      QMutex mMutex;

      /**
       * \brief Create a new layer
       *
       * @param lyrName layer's name
       * @param arrangement Arrangement mode : how to place candidates
       * @param defaultPriority layer's prioriry (0 is the best, 1 the worst)
       * @param obstacle 'true' will discourage other label to be placed above features of this layer
       * @param active is the layer is active (currently displayed)
       * @param toLabel the layer will be labeled whether toLablel is true
       * @param pal pointer to the pal object
       * @param displayAll if true, all features will be labelled even though overlaps occur
       *
       */
      Layer( const QString& lyrName, Arrangement arrangement, double defaultPriority, bool obstacle, bool active, bool toLabel, Pal *pal, bool displayAll = false );

      /** Add newly created feature part into r tree and to the list */
      void addFeaturePart( FeaturePart* fpart, const QString &labelText = QString() );
  };

} // end namespace pal

#endif
