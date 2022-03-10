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
#include "qgstextrendererutils.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <QString>

/**
 * \ingroup core
 * \brief pal labeling engine
 * \class pal::LabelInfo
 * \note not available in Python bindings
 */

namespace pal
{
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

      //! Path offset variances used in curved placement.
      enum PathOffset
      {
        NoOffset,
        PositiveOffset,
        NegativeOffset
      };

      /**
       * Creates a new generic feature.
        * \param lf a pointer for a feature which contains the spatial entites
        * \param geom a pointer to a GEOS geometry
        */
      FeaturePart( QgsLabelFeature *lf, const GEOSGeometry *geom );

      FeaturePart( const FeaturePart &other );

      /**
       * Deletes the feature.
       */
      ~FeaturePart() override;

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
       * Returns the maximum number of point candidates to generate for this feature.
       */
      std::size_t maximumPointCandidates() const;

      /**
       * Returns the maximum number of line candidates to generate for this feature.
       */
      std::size_t maximumLineCandidates() const;

      /**
       * Returns the maximum number of polygon candidates to generate for this feature.
       */
      std::size_t maximumPolygonCandidates() const;

      /**
       * Generates a list of candidate positions for labels for this feature.
       */
      std::vector<std::unique_ptr<LabelPosition> > createCandidates( Pal *pal );

      /**
       * Generate candidates for point feature, located around a specified point.
       * \param x x coordinate of the point
       * \param y y coordinate of the point
       * \param lPos pointer to an array of candidates, will be filled by generated candidates
       * \param angle orientation of the label
       * \returns the number of generated candidates
       */
      std::size_t createCandidatesAroundPoint( double x, double y, std::vector<std::unique_ptr<LabelPosition> > &lPos, double angle );

      /**
       * Generate one candidate over or offset the specified point.
       * \param x x coordinate of the point
       * \param y y coordinate of the point
       * \param lPos pointer to an array of candidates, will be filled by generated candidate
       * \param angle orientation of the label
       * \returns the number of generated candidates (always 1)
       */
      std::size_t createCandidatesOverPoint( double x, double y, std::vector<std::unique_ptr<LabelPosition> > &lPos, double angle );

      /**
       * Generate one candidate centered over the specified point.
       * \param x x coordinate of the point
       * \param y y coordinate of the point
       * \param lPos pointer to an array of candidates, will be filled by generated candidate
       * \param angle orientation of the label
       * \returns the number of generated candidates (always 1)
       */
      std::size_t createCandidateCenteredOverPoint( double x, double y, std::vector<std::unique_ptr<LabelPosition> > &lPos, double angle );

      /**
       * Creates a single candidate using the "point on sruface" algorithm.
       *
       * \note Unlike the other create candidates methods, this method
       * bypasses the usual candidate filtering steps and ALWAYS returns a single candidate.
       */
      std::unique_ptr< LabelPosition > createCandidatePointOnSurface( PointSet *mapShape );

      /**
       * Generates candidates following a prioritized list of predefined positions around a point.
       * \param x x coordinate of the point
       * \param y y coordinate of the point
       * \param lPos pointer to an array of candidates, will be filled by generated candidate
       * \param angle orientation of the label
       * \returns the number of generated candidates
       */
      std::size_t createCandidatesAtOrderedPositionsOverPoint( double x, double y, std::vector<std::unique_ptr<LabelPosition> > &lPos, double angle );

      /**
       * Generate candidates for line feature.
       * \param lPos pointer to an array of candidates, will be filled by generated candidates
       * \param mapShape a pointer to the line
       * \param allowOverrun set to TRUE to allow labels to overrun features
       * \param pal point to pal settings object, for cancellation support
       * \returns the number of generated candidates
       */
      std::size_t createCandidatesAlongLine( std::vector<std::unique_ptr<LabelPosition> > &lPos, PointSet *mapShape, bool allowOverrun, Pal *pal );

      /**
       * Generate horizontal candidates for line feature.
       * \param lPos pointer to an array of candidates, will be filled by generated candidates
       * \param mapShape a pointer to the line
       * \param pal point to pal settings object, for cancellation support
       * \returns the number of generated candidates
       */
      std::size_t createHorizontalCandidatesAlongLine( std::vector<std::unique_ptr<LabelPosition> > &lPos, PointSet *mapShape, Pal *pal );

      /**
       * Generate candidates for line feature, by trying to place candidates towards the middle of the longest
       * straightish segments of the line. Segments closer to horizontal are preferred over vertical segments.
       * \param lPos pointer to an array of candidates, will be filled by generated candidates
       * \param mapShape a pointer to the line
       * \param pal point to pal settings object, for cancellation support
       * \returns the number of generated candidates
       */
      std::size_t createCandidatesAlongLineNearStraightSegments( std::vector<std::unique_ptr<LabelPosition> > &lPos, PointSet *mapShape, Pal *pal );

      /**
       * Generate candidates for line feature, by trying to place candidates as close as possible to the line's midpoint.
       * Candidates can "cut corners" if it helps them place near this mid point.
       * \param lPos pointer to an array of candidates, will be filled by generated candidates
       * \param mapShape a pointer to the line
       * \param initialCost initial cost for candidates generated using this method. If set, cost can be increased
       * by a preset amount.
       * \param pal point to pal settings object, for cancellation support
       * \returns the number of generated candidates
       */
      std::size_t createCandidatesAlongLineNearMidpoint( std::vector<std::unique_ptr<LabelPosition> > &lPos, PointSet *mapShape, double initialCost = 0.0, Pal *pal = nullptr );

      /**
       * Returns the label position for a curved label at a specific offset along a path.
       * \param mapShape line path to place label on
       * \param pathDistances array of distances to each segment on path
       * \param direction either RespectPainterOrientation or FollowLineDirection
       * \param distance distance to offset label along curve by
       * \param labeledLineSegmentIsRightToLeft if TRUE label is reversed from lefttoright to righttoleft
       * \param applyAngleConstraints TRUE if label feature character angle constraints should be applied
       * \param uprightOnly TRUE if only upright label placement should be generated
       * \returns calculated label position
       */
      std::unique_ptr< LabelPosition > curvedPlacementAtOffset( PointSet *mapShape, const std::vector<double> &pathDistances,
          QgsTextRendererUtils::LabelLineDirection direction, double distance, bool &labeledLineSegmentIsRightToLeft, bool applyAngleConstraints, bool uprightOnly );

      /**
       * Generate curved candidates for line features.
       * \param lPos pointer to an array of candidates, will be filled by generated candidates
       * \param mapShape a pointer to the line
       * \param allowOverrun set to TRUE to allow labels to overrun features
       * \param pal point to pal settings object, for cancellation support
       * \returns the number of generated candidates
       */
      std::size_t createCurvedCandidatesAlongLine( std::vector<std::unique_ptr<LabelPosition> > &lPos, PointSet *mapShape, bool allowOverrun, Pal *pal );

      /**
       * Generate candidates for polygon features.
       * \param lPos pointer to an array of candidates, will be filled by generated candidates
       * \param mapShape a pointer to the polygon
       * \param pal point to pal settings object, for cancellation support
       * \returns the number of generated candidates
       */
      std::size_t createCandidatesForPolygon( std::vector<std::unique_ptr<LabelPosition> > &lPos, PointSet *mapShape, Pal *pal );

      /**
       * Generate candidates outside of polygon features.
       * \param lPos pointer to an array of candidates, will be filled by generated candidates
       * \param pal point to pal settings object, for cancellation support
       * \returns the number of generated candidates
       */
      std::size_t createCandidatesOutsidePolygon( std::vector<std::unique_ptr<LabelPosition> > &lPos, Pal *pal );

      /**
       * Tests whether this feature part belongs to the same QgsLabelFeature as another
       * feature part.
       * \param part part to compare to
       * \returns TRUE if both parts belong to same QgsLabelFeature
       */
      bool hasSameLabelFeatureAs( FeaturePart *part ) const;

      /**
       * Returns the width of the label, optionally taking an \a angle into account.
       * \returns the width of the label
       */
      double getLabelWidth( double angle = 0.0 ) const { return mLF->size( angle ).width(); }

      /**
       * Returns the height of the label, optionally taking an \a angle into account.
       * \returns the hieght of the label
       */
      double getLabelHeight( double angle = 0.0 ) const { return mLF->size( angle ).height(); }

      /**
       * Returns the distance from the anchor point to the label
       * \returns the distance to the label
       */
      double getLabelDistance() const { return mLF->distLabel(); }

      //! Returns TRUE if the feature's label has a fixed rotation
      bool hasFixedRotation() const { return mLF->hasFixedAngle(); }

      //! Returns the fixed angle for the feature's label
      double fixedAngle() const { return mLF->fixedAngle(); }

      //! Returns TRUE if the feature's label has a fixed position
      bool hasFixedPosition() const { return mLF->hasFixedPosition(); }

      /**
       * Returns TRUE if the feature's label should always been shown,
       * even when it collides with other labels
       */
      bool alwaysShow() const { return mLF->alwaysShow(); }

      /**
       * Returns the feature's obstacle settings.
       */
      const QgsLabelObstacleSettings &obstacleSettings() const { return mLF->obstacleSettings(); }

      //! Returns the distance between repeating labels for this feature
      double repeatDistance() const { return mLF->repeatDistance(); }

      //! Gets number of holes (inner rings) - they are considered as obstacles
      int getNumSelfObstacles() const { return mHoles.count(); }
      //! Gets hole (inner ring) - considered as obstacle
      FeaturePart *getSelfObstacle( int i ) { return mHoles.at( i ); }

      //! Check whether this part is connected with some other part
      bool isConnected( FeaturePart *p2 );

      /**
       * Merge other (connected) part with this one and save the result in this part (other is unchanged).
       * Returns TRUE on success, FALSE if the feature wasn't modified.
      */
      bool mergeWithFeaturePart( FeaturePart *other );

      /**
       * Increases the cost of the label candidates for this feature, based on the size of the feature.
       *
       * E.g. small lines or polygons get higher cost so that larger features are more likely to be labeled.
       */
      void addSizePenalty( std::vector<std::unique_ptr<LabelPosition> > &lPos, double bbx[4], double bby[4] );

      /**
       * Calculates the priority for the feature. This will be the feature's priority if set,
       * otherwise the layer's default priority.
       */
      double calculatePriority() const;

      //! Returns TRUE if feature's label must be displayed upright
      bool onlyShowUprightLabels() const;

      /**
       * Returns the total number of repeating labels associated with this label.
       * \see setTotalRepeats()
       */
      int totalRepeats() const;

      /**
       * Returns the total number of repeating labels associated with this label.
       * \see totalRepeats()
       */
      void setTotalRepeats( int repeats );

    protected:

      QgsLabelFeature *mLF = nullptr;
      QList<FeaturePart *> mHoles;

      //! \brief read coordinates from a GEOS geom
      void extractCoords( const GEOSGeometry *geom );

    private:

      LabelPosition::Quadrant quadrantFromOffset() const;

      int mTotalRepeats = 0;

      mutable std::size_t mCachedMaxLineCandidates = 0;
      mutable std::size_t mCachedMaxPolygonCandidates = 0;

      FeaturePart &operator= ( const FeaturePart & ) = delete;
  };

} // end namespace pal

#endif
