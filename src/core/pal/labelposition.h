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

#ifndef LABELPOSITION_H
#define LABELPOSITION_H

#define SIP_NO_FILE


#include "qgis_core.h"
#include "pointset.h"
#include "palrtree.h"
#include <fstream>

namespace pal
{

  class FeaturePart;
  class Pal;
  class Label;


  /**
   * \ingroup core
   * \brief LabelPosition is a candidate feature label position
   * \class pal::LabelPosition
   * \note not available in Python bindings
   */
  class CORE_EXPORT LabelPosition : public PointSet
  {
      friend class CostCalculator;
      friend class PolygonCostCalculator;

    public:

      /**
       * \brief Position of label candidate relative to feature.
       */
      enum Quadrant
      {
        QuadrantAboveLeft,
        QuadrantAbove,
        QuadrantAboveRight,
        QuadrantLeft,
        QuadrantOver,
        QuadrantRight,
        QuadrantBelowLeft,
        QuadrantBelow,
        QuadrantBelowRight
      };

      /**
       * \brief create a new LabelPosition
       *
       * \param id id of this labelposition
       * \param x1 down-left x coordinate
       * \param y1 down-left y coordinate
       * \param w label width
       * \param h label height
       * \param alpha rotation in rad
       * \param cost geographic cost
       * \param feature labelpos owners
       * \param isReversed label is reversed
       * \param quadrant relative position of label to feature
       */
      LabelPosition( int id, double x1, double y1,
                     double w, double h,
                     double alpha, double cost,
                     FeaturePart *feature, bool isReversed = false, Quadrant quadrant = QuadrantOver );

      //! Copy constructor
      LabelPosition( const LabelPosition &other );

      ~LabelPosition() override;

      /**
       * Returns TRUE if the label position intersects a \a geometry.
       *
       * \note This method considers the label's outer bounds (see QgsLabelFeature::outerBounds())
       */
      bool intersects( const GEOSPreparedGeometry *geometry );

      /**
       * Returns TRUE if the label position is within a \a geometry.
       *
       * \note This method considers the label's outer bounds (see QgsLabelFeature::outerBounds())
       */
      bool within( const GEOSPreparedGeometry *geometry );

      /**
       * Check whether or not this overlap with another labelPosition.
       *
       * \note This method considers the label's outer bounds (see QgsLabelFeature::outerBounds())
       *
       * \param ls other labelposition
       * \returns TRUE or FALSE
       */
      bool isInConflict( const LabelPosition *ls ) const;

      /**
       * Returns bounding box - amin: xmin,ymin - amax: xmax,ymax
       *
       * \note This method considers the label's outer bounds (see QgsLabelFeature::outerBounds())
       */
      void getBoundingBox( double amin[2], double amax[2] ) const;

      /**
       * Returns TRUE if the outer bounding box of this pointset intersects the outer bounding box
       * of another label position.
       */
      bool outerBoundingBoxIntersects( const LabelPosition *other ) const;

      /**
       * Gets distance from this label to a point. If point lies inside, returns negative number.
       *
       * If \a useOuterBounds is TRUE then the distance will be calculated to the outer bounds
       * of the label (see QgsLabelFeature::outerBounds()), otherwise it will be calculated
       * to the label's actual rectangle.
       */
      double getDistanceToPoint( double xp, double yp, bool useOuterBounds ) const;

      /**
       * Returns TRUE if this label crosses the specified line.
       *
       * \note This method considers the label's outer bounds (see QgsLabelFeature::outerBounds())
       */
      bool crossesLine( PointSet *line ) const;

      /**
       * Returns TRUE if this label crosses the boundary of the specified polygon.
       *
       * \note This method considers the label's outer bounds (see QgsLabelFeature::outerBounds())
       */
      bool crossesBoundary( PointSet *polygon ) const;

      /**
       * Returns cost of position intersection with polygon (testing area of intersection and center).
       * Cost ranges between 0 and 12, with extra cost if center of label position is covered.
       */
      int polygonIntersectionCost( PointSet *polygon ) const;

      /**
       * Returns TRUE if any intersection between polygon and position exists.
       *
       * \note This method considers the label's outer bounds (see QgsLabelFeature::outerBounds())
      */
      bool intersectsWithPolygon( PointSet *polygon ) const;

      /**
       * Returns the id
       */
      int getId() const;

      /**
       * Returns the feature corresponding to this labelposition
       */
      FeaturePart *getFeaturePart() const;

      int getNumOverlaps() const { return nbOverlap; }
      void resetNumOverlaps() { nbOverlap = 0; } // called from problem.cpp, pal.cpp

      /**
       * Increases the number of overlaps recorded against this position by 1.
       */
      void incrementNumOverlaps() { nbOverlap++; }

      /**
       * Decreases the number of overlaps recorded against this position by 1.
       */
      void decrementNumOverlaps() { nbOverlap++; }

      int getProblemFeatureId() const { return probFeat; }

      /**
       * Set problem feature ID and assigned label candidate ID.
       * called from pal.cpp during extraction.
      */
      void setProblemIds( int probFid, int lpId )
      {
        probFeat = probFid;
        id = lpId;
        if ( mNextPart ) mNextPart->setProblemIds( probFid, lpId );
      }

      /**
       * Returns the candidate label position's geographical cost.
       * \see setCost
       */
      double cost() const { return mCost; }

      /**
       * Sets the candidate label position's geographical cost.
       * \param newCost new cost for position
       * \see cost
      */
      void setCost( double newCost ) { mCost = newCost; }

      /**
       * Sets whether the position is marked as conflicting with an obstacle feature.
       * \param conflicts set to TRUE to mark candidate as being in conflict
       * \note This method applies to all label parts for the candidate position.
       * \see conflictsWithObstacle
       */
      void setConflictsWithObstacle( bool conflicts );

      /**
       * Returns whether the position is marked as conflicting with an obstacle feature.
       * \see setConflictsWithObstacle
       */
      bool conflictsWithObstacle() const { return mHasObstacleConflict; }

      /**
       * Sets whether the position is marked as having a hard conflict with an obstacle feature.
       * A hard conflict means that the placement should (usually) not be considered, because the candidate
       * conflicts with a obstacle of sufficient weight.
       * \see hasHardObstacleConflict()
       */
      void setHasHardObstacleConflict( bool conflicts );

      /**
       * Returns whether the position is marked as having a hard conflict with an obstacle feature.
       * A hard conflict means that the placement should (usually) not be considered, because the candidate
       * conflicts with a obstacle of sufficient weight.
       * \see setHasHardObstacleConflict()
       */
      bool hasHardObstacleConflict() const { return mHasHardConflict; }

      //! Make sure the cost is less than 1
      void validateCost();

      /**
       * Returns the down-left x coordinate.
       * \see getY()
       */
      double getX( int i = 0 ) const;

      /**
       * Returns the down-left y coordinate.
       * \see getX()
       */
      double getY( int i = 0 ) const;

      double getWidth() const { return w; }
      double getHeight() const { return h; }

      /**
       * Returns the angle to rotate text (in rad).
       */
      double getAlpha() const;
      bool getReversed() const { return reversed; }
      bool getUpsideDown() const { return upsideDown; }

      Quadrant getQuadrant() const { return quadrant; }

      /**
       * Returns the next part of this label position (i.e. the next character for a curved label).
       *
       * \see setNextPart()
       */
      LabelPosition *nextPart() const { return mNextPart.get(); }

      /**
       * Sets the \a next part of this label position (i.e. the next character for a curved label).
       *
       * \see nextPart()
       */
      void setNextPart( std::unique_ptr< LabelPosition > next ) { mNextPart = std::move( next ); }

      // -1 if not multi-part
      int getPartId() const { return partId; }
      void setPartId( int id ) { partId = id; }

      /**
       * Sets the \a count of upside down characters for this label position.
       *
       * \see upsideDownCharCount()
       */
      void setUpsideDownCharCount( int count ) { mUpsideDownCharCount = count ; }

      /**
       * Returns the number of upside down characters for this label position.
       *
       * \see setUpsideDownCharCount()
       */
      int upsideDownCharCount() const { return mUpsideDownCharCount; }

      /**
       * Removes the label position from the specified \a index.
       */
      void removeFromIndex( PalRtree<LabelPosition> &index );

      /**
       * Inserts the label position into the specified \a index.
       */
      void insertIntoIndex( PalRtree<LabelPosition> &index );

      /**
       * Returns a prepared GEOS representation of all label parts as a multipolygon.
       *
       * \since QGIS 3.20
       */
      const GEOSPreparedGeometry *preparedMultiPartGeom() const;

      /**
       * Returns the prepared outer boundary geometry.
       *
       * \since QGIS 3.30
       */
      const GEOSPreparedGeometry *preparedOuterBoundsGeom() const;

      /**
       * Returns the global ID for the candidate, which is unique for a single run of the pal
       * labelling engine.
       *
       * A return value of 0 means that the ID has not been assigned.
       *
       * \see setGlobalId()
       */
      unsigned int globalId() const { return mGlobalId; }

      /**
       * Sets the global \a id for the candidate, which is unique for a single run of the pal
       * labelling engine.
       *
       * \see globalId()
       */
      void setGlobalId( unsigned int id ) { mGlobalId = id; }

      /**
       * Returns the angle differential of all LabelPosition parts
       */
      double angleDifferential();

    protected:

      int id;

      FeaturePart *feature = nullptr;

      // bug # 1 (maxence 10/23/2008)
      int probFeat;

      int nbOverlap;

      double alpha;
      double w;
      double h;

      int partId;

      //True if label direction is the same as line / polygon ring direction.
      //Could be used by the application to draw a directional arrow ('<' or '>')
      //if the layer arrangement is P_LINE
      bool reversed;

      bool upsideDown;

      LabelPosition::Quadrant quadrant;

    private:

      unsigned int mGlobalId = 0;
      std::unique_ptr< LabelPosition > mNextPart;

      std::vector< double > mOuterBoundsX;
      std::vector< double > mOuterBoundsY;

      double mOuterBoundsXMin = std::numeric_limits<double>::max();
      double mOuterBoundsXMax = std::numeric_limits<double>::lowest();
      double mOuterBoundsYMin = std::numeric_limits<double>::max();
      double mOuterBoundsYMax = std::numeric_limits<double>::lowest();

      geos::unique_ptr mOuterBoundsGeos;
      const GEOSPreparedGeometry *mPreparedOuterBoundsGeos = nullptr;

      double mCost;
      bool mHasObstacleConflict;
      bool mHasHardConflict = false;
      int mUpsideDownCharCount;

      /**
       * Calculates the total number of parts for this label position
       */
      int partCount() const;

      /**
       * Calculates the polygon intersection cost for a single label position part
       * \returns double between 0 - 12
       */
      double polygonIntersectionCostForParts( PointSet *polygon ) const;

      /**
       * Creates a GEOS representation of all label parts as a multipolygon.
       */
      void createMultiPartGeosGeom() const;

      bool isInConflictMultiPart( const LabelPosition *lp ) const;

      void createOuterBoundsGeom();

      LabelPosition &operator=( const LabelPosition & ) = delete;
  };

} // end namespace

#endif
