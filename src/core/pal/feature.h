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

#include "qgsgeometry.h"
#include "pointset.h"
#include "util.h"
#include "labelposition.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <QString>

#include "qgslabelfeature.h"

/** \ingroup core
 * \class pal::LabelInfo
 * \note not available in Python bindings
 */

namespace pal
{
  /** Optional additional info about label (for curved labels) */
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

      double max_char_angle_inside;
      double max_char_angle_outside;
      double label_height;
      int char_num;
      CharacterInfo* char_info;
    private:

      LabelInfo( const LabelInfo& rh );
      LabelInfo& operator=( const LabelInfo& rh );
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

      /** Creates a new generic feature.
        * @param lf a pointer for a feature which contains the spatial entites
        * @param geom a pointer to a GEOS geometry
        */
      FeaturePart( QgsLabelFeature* lf, const GEOSGeometry* geom );

      FeaturePart( const FeaturePart& other );

      /** Delete the feature
       */
      virtual ~FeaturePart();

      /** Returns the parent feature.
       */
      QgsLabelFeature* feature() { return mLF; }

      /** Returns the layer that feature belongs to.
       */
      Layer* layer();

      /** Returns the unique ID of the feature.
       */
      QgsFeatureId featureId() const;

      /** Generic method to generate label candidates for the feature.
       * \param lPos pointer to an array of candidates, will be filled by generated candidates
       * \param bboxMin min values of the map extent
       * \param bboxMax max values of the map extent
       * \param mapShape generate candidates for this spatial entity
       * \param candidates index for candidates
       * \return the number of candidates generated in lPos
       */
      int createCandidates( QList<LabelPosition *> &lPos, double bboxMin[2], double bboxMax[2], PointSet *mapShape, RTree<LabelPosition*, double, 2, double>* candidates );

      /** Generate candidates for point feature, located around a specified point.
       * @param x x coordinate of the point
       * @param y y coordinate of the point
       * @param lPos pointer to an array of candidates, will be filled by generated candidates
       * @param angle orientation of the label
       * @param mapShape optional geometry of source polygon
       * @returns the number of generated candidates
       */
      int createCandidatesAroundPoint( double x, double y, QList<LabelPosition *> &lPos, double angle, PointSet *mapShape = nullptr );

      /** Generate one candidate over or offset the specified point.
       * @param x x coordinate of the point
       * @param y y coordinate of the point
       * @param lPos pointer to an array of candidates, will be filled by generated candidate
       * @param angle orientation of the label
       * @param mapShape optional geometry of source polygon
       * @returns the number of generated candidates (always 1)
       */
      int createCandidatesOverPoint( double x, double y, QList<LabelPosition *> &lPos, double angle, PointSet *mapShape = nullptr );

      /** Generates candidates following a prioritised list of predefined positions around a point.
       * @param x x coordinate of the point
       * @param y y coordinate of the point
       * @param lPos pointer to an array of candidates, will be filled by generated candidate
       * @param angle orientation of the label
       * @returns the number of generated candidates
       */
      int createCandidatesAtOrderedPositionsOverPoint( double x, double y, QList<LabelPosition *> &lPos, double angle );

      /** Generate candidates for line feature.
       * @param lPos pointer to an array of candidates, will be filled by generated candidates
       * @param mapShape a pointer to the line
       * @returns the number of generated candidates
       */
      int createCandidatesAlongLine( QList<LabelPosition *> &lPos, PointSet *mapShape );

      LabelPosition* curvedPlacementAtOffset( PointSet* path_positions, double* path_distances,
                                              int orientation, int index, double distance );

      /** Generate curved candidates for line features.
       * @param lPos pointer to an array of candidates, will be filled by generated candidates
       * @param mapShape a pointer to the line
       * @returns the number of generated candidates
       */
      int createCurvedCandidatesAlongLine( QList<LabelPosition *> &lPos, PointSet* mapShape );

      /** Generate candidates for polygon features.
       * \param lPos pointer to an array of candidates, will be filled by generated candidates
       * \param mapShape a pointer to the polygon
       * \return the number of generated candidates
       */
      int createCandidatesForPolygon( QList<LabelPosition *> &lPos, PointSet *mapShape );

      /** Tests whether this feature part belongs to the same QgsLabelFeature as another
       * feature part.
       * @param part part to compare to
       * @returns true if both parts belong to same QgsLabelFeature
       */
      bool hasSameLabelFeatureAs( FeaturePart* part ) const;

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

      bool getFixedRotation() { return mLF->hasFixedAngle(); }
      double getLabelAngle() { return mLF->fixedAngle(); }
      bool getFixedPosition() { return mLF->hasFixedPosition(); }
      bool getAlwaysShow() { return mLF->alwaysShow(); }
      bool isObstacle() { return mLF->isObstacle(); }
      double obstacleFactor() { return mLF->obstacleFactor(); }
      double repeatDistance() { return mLF->repeatDistance(); }

      //! Get number of holes (inner rings) - they are considered as obstacles
      int getNumSelfObstacles() const { return mHoles.count(); }
      //! Get hole (inner ring) - considered as obstacle
      FeaturePart* getSelfObstacle( int i ) { return mHoles.at( i ); }

      /** Check whether this part is connected with some other part */
      bool isConnected( FeaturePart* p2 );

      /** Merge other (connected) part with this one and save the result in this part (other is unchanged).
       * Return true on success, false if the feature wasn't modified */
      bool mergeWithFeaturePart( FeaturePart* other );

      void addSizePenalty( int nbp, QList<LabelPosition *> &lPos, double bbx[4], double bby[4] );

      /** Calculates the priority for the feature. This will be the feature's priority if set,
       * otherwise the layer's default priority.
       * @see Feature::setPriority
       * @see Feature::priority
       */
      double calculatePriority() const;


    protected:

      QgsLabelFeature* mLF;
      QList<FeaturePart*> mHoles;

      /** \brief read coordinates from a GEOS geom */
      void extractCoords( const GEOSGeometry* geom );

    private:

      LabelPosition::Quadrant quadrantFromOffset() const;
  };

} // end namespace pal

#endif
