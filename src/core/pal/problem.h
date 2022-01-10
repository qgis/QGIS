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

#ifndef PAL_PROBLEM_H
#define PAL_PROBLEM_H

#define SIP_NO_FILE


#include "qgis_core.h"
#include <list>
#include <QList>
#include "palrtree.h"
#include "qgsrendercontext.h"
#include <memory>
#include <vector>

namespace pal
{

  class LabelPosition;
  class Label;
  class PriorityQueue;

  /**
   * \class pal::Sol
   * \ingroup core
   * \brief Chain solution parameters.
   * \note not available in Python bindings
   */

  struct Chain
  {
    int degree;
    double delta;
    int *feat = nullptr;
    int *label = nullptr;
  };

  /**
   * \ingroup core
   * \brief Representation of a labeling problem
   * \class pal::Problem
   * \note not available in Python bindings
   */
  class CORE_EXPORT Problem
  {
      friend class Pal;

    public:

      /**
       * Constructor for Problem.
       *
       * The \a extent argument specifies the bounds of the incoming coordinates.
       */
      Problem( const QgsRectangle &extent );

      //Problem(char *lorena_file, bool displayAll);

      ~Problem();

      //! Problem cannot be copied
      Problem( const Problem &other ) = delete;
      //! Problem cannot be copied
      Problem &operator=( const Problem &other ) = delete;

      /**
       * Adds a candidate label position to the problem.
       * \param position label candidate position. Ownership is transferred to Problem.
       * \since QGIS 2.12
       */
      void addCandidatePosition( std::unique_ptr< LabelPosition > position ) { mLabelPositions.emplace_back( std::move( position ) ); }

      /**
       * Returns the total number of features considered during the labeling problem.
       */
      std::size_t featureCount() const { return mFeatureCount; }

      /**
       * Returns the number of candidates generated for the \a feature at the specified index.
       */
      int featureCandidateCount( int feature ) const { return mFeatNbLp[feature]; }

      /**
       * Returns the candidate corresponding to the specified \a feature and \a candidate index.
       */
      LabelPosition *featureCandidate( int feature, int candidate ) const { return mLabelPositions[ mFeatStartId[feature] + candidate ].get(); }

      void reduce();

      /**
       * \brief Test with very-large scale neighborhood
       */
      void chainSearch( QgsRenderContext &context );

      /**
       * Solves the labeling problem, selecting the best candidate locations for all labels and returns a list of these
       * calculated label positions.
       *
       * If \a returnInactive is TRUE, then the best positions for ALL labels will be returned, regardless of whether these
       * labels overlap other labels.
       *
       * If the optional \a unlabeled list is specified, it will be filled with a list of all feature labels which could
       * not be placed in the returned solution (e.g. due to overlaps or other constraints).
       *
       * Ownership of the returned labels is not transferred - it resides with the pal object.
       */
      QList<LabelPosition *> getSolution( bool returnInactive, QList<LabelPosition *> *unlabeled = nullptr );

      /* useful only for postscript post-conversion*/
      //void toFile(char *label_file);

      void init_sol_falp();

      /**
       * Returns a reference to the list of label positions which correspond to
       * features with no candidates.
       *
       * Ownership of positions added to this list is transferred to the problem.
       */
      std::vector< std::unique_ptr< LabelPosition > > *positionsWithNoCandidates()
      {
        return &mPositionsWithNoCandidates;
      }

      /**
       * Returns the index containing all label candidates.
       */
      PalRtree< LabelPosition > &allCandidatesIndex() { return mAllCandidatesIndex; }

    private:

      /**
       * Returns TRUE if a labelling candidate \a lp1 conflicts with \a lp2.
       */
      bool candidatesAreConflicting( const LabelPosition *lp1, const LabelPosition *lp2 ) const;

      /**
       * Total number of layers containing labels
       */
      int mLayerCount = 0;

      /**
       * Names of the labelled layers
       */
      QStringList labelledLayersName;

      /**
       * Total number of active candidates (remaining after reduce())
       */
      int mTotalCandidates = 0;

      /**
       * Number of candidates (all, including)
       */
      int mAllNblp = 0;

      /**
       * Total number of features to label.
       */
      std::size_t mFeatureCount = 0;

      /**
       * if TRUE, special value -1 is prohibited
       */
      bool mDisplayAll = false;

      /**
       * Map extent (xmin, ymin, xmax, ymax)
       */
      double mMapExtentBounds[4] = {0, 0, 0, 0};

      std::vector< std::unique_ptr< LabelPosition > > mLabelPositions;

      PalRtree<LabelPosition> mAllCandidatesIndex;
      PalRtree<LabelPosition> mActiveCandidatesIndex;

      std::vector< std::unique_ptr< LabelPosition > > mPositionsWithNoCandidates;

      std::vector< int > mFeatStartId;
      std::vector< int > mFeatNbLp;
      std::vector< double > mInactiveCost;

      class Sol
      {
        public:

          //! Placeholder list for active labels. Will contain label id for active labels, or -1 for empty positions in list
          std::vector< int > activeLabelIds;

          void init( std::size_t featureCount )
          {
            activeLabelIds.resize( featureCount, -1 );
          }
      };

      Sol mSol;
      double mNbOverlap = 0.0;

      Chain *chain( int seed );

      Pal *pal = nullptr;

      void solution_cost();
      void ignoreLabel( const LabelPosition *lp, pal::PriorityQueue &list, PalRtree<LabelPosition> &candidatesIndex );
  };

} // namespace

#endif
