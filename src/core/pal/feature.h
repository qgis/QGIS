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

#ifndef FEATURE_H
#define FEATURE_H

#define SIP_NO_FILE


#include "qgis_core.h"
#include "pointset.h"
#include "labelposition.h" // for LabelPosition enum
#include "qgslabelfeature.h"
#include "rtree.hpp"
#include <iostream>
#include <fstream>
#include <cmath>
#include <QString>

/**
 * \ingroup core
 * \class pal::LabelInfo
 * \note not available in Python bindings
 */

namespace pal
{
  //! Optional additional info about label (for curved labels)
  class CORE_EXPORT LabelInfo
  {
    public:
      typedef struct
      {
        double width;
      } CharacterInfo;

      LabelInfo( int num, double height, double maxinangle = 20.0, double maxoutangle = -20.0 )
      {
        max_char_angle_inside = maxinangle;
        // outside angle should be negative
        max_char_angle_outside = maxoutangle > 0 ? -maxoutangle : maxoutangle;
        label_height = height;
        char_num = num;
        char_info = new CharacterInfo[num];
      }
      ~LabelInfo() { delete [] char_info; }

      //! LabelInfo cannot be copied
      LabelInfo( const LabelInfo &rh ) = delete;
      //! LabelInfo cannot be copied
      LabelInfo &operator=( const LabelInfo &rh ) = delete;

      double max_char_angle_inside;
      double max_char_angle_outside;
      double label_height;
      int char_num;
      CharacterInfo *char_info = nullptr;

  };

  class LabelPosition;
  class FeaturePart;

  /**
   * \ingroup core
   * \brief Main class to handle feature
   * \class pal::FeaturePart
   * \note not available in Python bindings
   */
  class CORE_EXPORT FeaturePart : public PointSet
  {

    public:

      /**
       * Creates a new generic feature.
        * \param lf a pointer for a feature which contains the spatial entites
        * \param geom a pointer to a GEOS geometry
        */
      FeaturePart( QgsLabelFeature *lf, const GEOSGeometry *geom );

      FeaturePart( const FeaturePart &other );

      /**
       * Delete the feature
       */
      virtual ~FeaturePart();

      /**
       * Returns the parent feature.
       */
      QgsLabelFeature *feature() { return mLF; }

      /**
       * Returns the layer that feature belongs to.
       */
      Layer *layer();

      /**
       * Returns the unique ID of the feature.
       */
      QgsFeatureId featureId() const;

      /**
       * Generic method to generate label candidates for the feature.
       * \param lPos pointer to an array of candidates, will be filled by generated candidates
       * \param bboxMin min values of the map extent
       * \param bboxMax max values of the map extent
       * \param mapShape generate candidates for this spatial entity
       * \param candidates index for candidates
       * \returns the number of candidates generated in lPos
       */
      int createCandidates( QList<LabelPosition *> &lPos, double bboxMin[2], double bboxMax[2], PointSet *mapShape, RTree<LabelPosition *, double, 2, double> *candidates );

      /**
       * Generate candidates for point feature, located around a specified point.
       * \param x x coordinate of the point
       * \param y y coordinate of the point
       * \param lPos pointer to an array of candidates, will be filled by generated candidates
       * \param angle orientation of the label
       * \returns the number of generated candidates
       */
      int createCandidatesAroundPoint( double x, double y, QList<LabelPosition *> &lPos, double angle );

      /**
       * Generate one candidate over or offset the specified point.
       * \param x x coordinate of the point
       * \param y y coordinate of the point
       * \param lPos pointer to an array of candidates, will be filled by generated candidate
       * \param angle orientation of the label
       * \returns the number of generated candidates (always 1)
       */
      int createCandidatesOverPoint( double x, double y, QList<LabelPosition *> &lPos, double angle );

      /**
       * Generates candidates following a prioritized list of predefined positions around a point.
       * \param x x coordinate of the point
       * \param y y coordinate of the point
       * \param lPos pointer to an array of candidates, will be filled by generated candidate
       * \param angle orientation of the label
       * \returns the number of generated candidates
       */
      int createCandidatesAtOrderedPositionsOverPoint( double x, double y, QList<LabelPosition *> &lPos, double angle );

      /**
       * Generate candidates for line feature.
       * \param lPos pointer to an array of candidates, will be filled by generated candidates
       * \param mapShape a pointer to the line
       * \returns the number of generated candidates
       */
      int createCandidatesAlongLine( QList<LabelPosition *> &lPos, PointSet *mapShape );

      /**
       * Generate candidates for line feature, by trying to place candidates towards the middle of the longest
       * straightish segments of the line. Segments closer to horizontal are preferred over vertical segments.
       * \param lPos pointer to an array of candidates, will be filled by generated candidates
       * \param mapShape a pointer to the line
       * \returns the number of generated candidates
       */
      int createCandidatesAlongLineNearStraightSegments( QList<LabelPosition *> &lPos, PointSet *mapShape );

      /**
       * Generate candidates for line feature, by trying to place candidates as close as possible to the line's midpoint.
       * Candidates can "cut corners" if it helps them place near this mid point.
       * \param lPos pointer to an array of candidates, will be filled by generated candidates
       * \param mapShape a pointer to the line
       * \param initialCost initial cost for candidates generated using this method. If set, cost can be increased
       * by a preset amount.
       * \returns the number of generated candidates
       */
      int createCandidatesAlongLineNearMidpoint( QList<LabelPosition *> &lPos, PointSet *mapShape, double initialCost = 0.0 );

      /**
       * Returns the label position for a curved label at a specific offset along a path.
       * \param path_positions line path to place label on
       * \param path_distances array of distances to each segment on path
       * \param orientation can be 0 for automatic calculation of orientation, or -1/+1 for a specific label orientation
       * \param index
       * \param distance distance to offset label along curve by
       * \param reversed if true label is reversed from lefttoright to righttoleft
       * \param flip if true label is placed on the other side of the line
       * \returns calculated label position
       */
      LabelPosition *curvedPlacementAtOffset( PointSet *path_positions, double *path_distances,
                                              int &orientation, int index, double distance, bool &reversed, bool &flip );

      /**
       * Generate curved candidates for line features.
       * \param lPos pointer to an array of candidates, will be filled by generated candidates
       * \param mapShape a pointer to the line
       * \returns the number of generated candidates
       */
      int createCurvedCandidatesAlongLine( QList<LabelPosition *> &lPos, PointSet *mapShape );

      /**
       * Generate candidates for polygon features.
       * \param lPos pointer to an array of candidates, will be filled by generated candidates
       * \param mapShape a pointer to the polygon
       * \returns the number of generated candidates
       */
      int createCandidatesForPolygon( QList<LabelPosition *> &lPos, PointSet *mapShape );

      /**
       * Tests whether this feature part belongs to the same QgsLabelFeature as another
       * feature part.
       * \param part part to compare to
       * \returns true if both parts belong to same QgsLabelFeature
       */
      bool hasSameLabelFeatureAs( FeaturePart *part ) const;

#if 0

      /**
       * \brief Print feature information
       * Print feature unique id, geometry type, points, and holes on screen
       */
      void print();
#endif

      double getLabelWidth() const { return mLF->size().width(); }
      double getLabelHeight() const { return mLF->size().height(); }
      double getLabelDistance() const { return mLF->distLabel(); }

      //! Returns true if the feature's label has a fixed rotation
      bool hasFixedRotation() const { return mLF->hasFixedAngle(); }

      //! Returns the fixed angle for the feature's label
      double fixedAngle() const { return mLF->fixedAngle(); }

      //! Returns true if the feature's label has a fixed position
      bool hasFixedPosition() const { return mLF->hasFixedPosition(); }

      /**
       * Returns true if the feature's label should always been shown,
       * even when it collides with other labels
       */
      bool alwaysShow() const { return mLF->alwaysShow(); }

      //! Returns true if the feature should act as an obstacle to labels
      bool isObstacle() const { return mLF->isObstacle(); }

      /**
       * Returns the feature's obstacle factor, which represents the penalty
       * incurred for a label to overlap the feature
       */
      double obstacleFactor() const { return mLF->obstacleFactor(); }

      //! Returns the distance between repeating labels for this feature
      double repeatDistance() const { return mLF->repeatDistance(); }

      //! Get number of holes (inner rings) - they are considered as obstacles
      int getNumSelfObstacles() const { return mHoles.count(); }
      //! Get hole (inner ring) - considered as obstacle
      FeaturePart *getSelfObstacle( int i ) { return mHoles.at( i ); }

      //! Check whether this part is connected with some other part
      bool isConnected( FeaturePart *p2 );

      /**
       * Merge other (connected) part with this one and save the result in this part (other is unchanged).
       * Return true on success, false if the feature wasn't modified */
      bool mergeWithFeaturePart( FeaturePart *other );

      void addSizePenalty( int nbp, QList<LabelPosition *> &lPos, double bbx[4], double bby[4] );

      /**
       * Calculates the priority for the feature. This will be the feature's priority if set,
       * otherwise the layer's default priority.
       * \see Feature::setPriority
       * \see Feature::priority
       */
      double calculatePriority() const;

      //! Returns true if feature's label must be displayed upright
      bool showUprightLabels() const;

      //! Returns true if the next char position is found. The referenced parameters are updated.
      bool nextCharPosition( double charWidth, double segment_length, PointSet *path_positions, int &index, double &distance,
                             double &start_x, double &start_y, double &end_x, double &end_y ) const;

    protected:

      QgsLabelFeature *mLF = nullptr;
      QList<FeaturePart *> mHoles;

      //! \brief read coordinates from a GEOS geom
      void extractCoords( const GEOSGeometry *geom );

    private:

      LabelPosition::Quadrant quadrantFromOffset() const;
  };

} // end namespace pal

#endif
