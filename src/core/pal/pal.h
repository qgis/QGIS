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

#ifndef PAL_H
#define PAL_H

#define SIP_NO_FILE


#include "qgis_core.h"
#include "qgsgeometry.h"
#include "qgsgeos.h"
#include "qgspallabeling.h"
#include <QList>
#include <iostream>
#include <ctime>
#include <QMutex>
#include <QStringList>

// TODO ${MAJOR} ${MINOR} etc instead of 0.2

class QgsAbstractLabelProvider;

namespace pal
{
  class Layer;
  class LabelPosition;
  class PalStat;
  class Problem;
  class PointSet;

  //! Search method to use
  enum SearchMethod
  {
    CHAIN = 0, //!< Is the worst but fastest method
    POPMUSIC_TABU_CHAIN = 1, //!< Is the best but slowest
    POPMUSIC_TABU = 2, //!< Is a little bit better than CHAIN but slower
    POPMUSIC_CHAIN = 3, //!< Is slower and best than TABU, worse and faster than TABU_CHAIN
    FALP = 4 //!< Only initial solution
  };

  //! Enumeration line arrangement flags. Flags can be combined.
  enum LineArrangementFlag
  {
    FLAG_ON_LINE     = 1,
    FLAG_ABOVE_LINE  = 2,
    FLAG_BELOW_LINE  = 4,
    FLAG_MAP_ORIENTATION = 8
  };
  Q_DECLARE_FLAGS( LineArrangementFlags, LineArrangementFlag )

  /**
   * \ingroup core
   *  \brief Main Pal labeling class
   *
   *  A pal object will contains layers and global information such as which search method
   *  will be used.
   *  \class pal::Pal
   *  \note not available in Python bindings
   */
  class CORE_EXPORT Pal
  {
      friend class Problem;
      friend class FeaturePart;
      friend class Layer;

    public:

      /**
       * \brief Create an new pal instance
       */
      Pal();

      ~Pal();

      //! Pal cannot be copied.
      Pal( const Pal &other ) = delete;
      //! Pal cannot be copied.
      Pal &operator=( const Pal &other ) = delete;

      /**
       * \brief add a new layer
       *
       * \param provider Provider associated with the layer
       * \param layerName layer's name
       * \param arrangement Howto place candidates
       * \param defaultPriority layer's prioriry (0 is the best, 1 the worst)
       * \param active is the layer is active (currently displayed)
       * \param toLabel the layer will be labeled only if toLablel is TRUE
       * \param displayAll if TRUE, all features will be labelled even though overlaps occur
       *
       * \throws PalException::LayerExists
       */
      Layer *addLayer( QgsAbstractLabelProvider *provider, const QString &layerName, QgsPalLayerSettings::Placement arrangement, double defaultPriority, bool active, bool toLabel, bool displayAll = false );

      /**
       * \brief remove a layer
       *
       * \param layer layer to remove
       */
      void removeLayer( Layer *layer );

      typedef bool ( *FnIsCanceled )( void *ctx );

      //! Register a function that returns whether this job has been canceled - PAL calls it during the computation
      void registerCancellationCallback( FnIsCanceled fnCanceled, void *context );

      //! Check whether the job has been canceled
      inline bool isCanceled() { return fnIsCanceled ? fnIsCanceled( fnIsCanceledContext ) : false; }

      /**
       * Extracts the labeling problem for the specified map \a extent - only features within this
       * extent will be considered. The \a mapBoundary argument specifies the actual geometry of the map
       * boundary, which will be used to detect whether a label is visible (or partially visible) in
       * the rendered map. This may differ from \a extent in the case of rotated or non-rectangular
       * maps.
       */
      std::unique_ptr< Problem > extractProblem( const QgsRectangle &extent, const QgsGeometry &mapBoundary );

      /**
       * Solves the labeling problem, selecting the best candidate locations for all labels and returns a list of these
       * calculated label positions.
       *
       * If \a displayAll is true, then the best positions for ALL labels will be returned, regardless of whether these
       * labels overlap other labels.
       *
       * If the optional \a unlabeled list is specified, it will be filled with a list of all feature labels which could
       * not be placed in the returned solution (e.g. due to overlaps or other constraints).
       *
       * Ownership of the returned labels is not transferred - it resides with the pal object.
       */
      QList<LabelPosition *> solveProblem( Problem *prob, bool displayAll, QList<pal::LabelPosition *> *unlabeled = nullptr );

      /**
       *\brief Set flag show partial label
       *
       * \param show flag value
       */
      void setShowPartial( bool show );

      /**
       * Returns whether partial labels should be allowed.
       */
      bool getShowPartial();

      /**
       * \brief set # candidates to generate for points features
       * Higher the value is, longer Pal::labeller will spend time
       *
       * \param point_p # candidates for a point
       */
      void setPointP( int point_p );

      /**
       * \brief set maximum # candidates to generate for lines features
       * Higher the value is, longer Pal::labeller will spend time
       *
       * \param line_p maximum # candidates for a line
       */
      void setLineP( int line_p );

      /**
       * \brief set maximum # candidates to generate for polygon features
       * Higher the value is, longer Pal::labeller will spend time
       *
       * \param poly_p maximum # candidate for a polygon
       */
      void setPolyP( int poly_p );

      /**
       * Returns the number of candidates to generate for point features.
       */
      int getPointP();

      /**
       * Returns the number of candidates to generate for line features.
       */
      int getLineP();

      /**
       * Returns the number of candidates to generate for polygon features.
       */
      int getPolyP();

    private:

      QHash< QgsAbstractLabelProvider *, Layer * > mLayers;

      QMutex mMutex;

      /**
       * \brief maximum # candidates for a point
       */
      int point_p = 16;

      /**
       * \brief maximum # candidates for a line
       */
      int line_p = 50;

      /**
       * \brief maximum # candidates for a polygon
       */
      int poly_p = 30;

      /*
       * POPMUSIC Tuning
       */
      int popmusic_r = 30;

      int tabuMaxIt = 4;
      int tabuMinIt = 2;

      int ejChainDeg = 50;
      int tenure = 10;
      double candListSize = 0.2;

      /**
       * \brief show partial labels (cut-off by the map canvas) or not
       */
      bool showPartial = true;

      //! Callback that may be called from PAL to check whether the job has not been canceled in meanwhile
      FnIsCanceled fnIsCanceled = nullptr;
      //! Application-specific context for the cancellation check function
      void *fnIsCanceledContext = nullptr;

      /**
       * Creates a Problem, by extracting labels and generating candidates from the given \a extent.
       * The \a mapBoundary geometry specifies the actual visible region of the map, and is used
       * for pruning candidates which fall outside the visible region.
       */
      std::unique_ptr< Problem > extract( const QgsRectangle &extent, const QgsGeometry &mapBoundary );

      /**
       * \brief Choose the size of popmusic subpart's
       * \param r subpart size
       */
      void setPopmusicR( int r );

      /**
       * \brief minimum # of iteration for search method POPMUSIC_TABU, POPMUSIC_CHAIN and POPMUSIC_TABU_CHAIN
       * \param min_it Sub part optimization min # of iteration
       */
      void setMinIt( int min_it );

      /**
       * \brief maximum \# of iteration for search method POPMUSIC_TABU, POPMUSIC_CHAIN and POPMUSIC_TABU_CHAIN
       * \param max_it Sub part optimization max # of iteration
       */
      void setMaxIt( int max_it );

      /**
       * \brief For tabu search : how many iteration a feature will be tabu
       * \param tenure consiser a feature as tabu for tenure iteration after updating feature in solution
       */
      void setTenure( int tenure );

      /**
       * \brief For *CHAIN, select the max size of a transformation chain
       * \param degree maximum soze of a transformation chain
       */
      void setEjChainDeg( int degree );

      /**
       * \brief How many candidates will be tested by a tabu iteration
       * \param fact the ration (0..1) of candidates to test
       */
      void setCandListSize( double fact );


      /**
       * Returns the minimum number of iterations used for POPMUSIC_TABU, POPMUSIC_CHAIN and POPMUSIC_TABU_CHAIN.
       * \see getMaxIt()
       */
      int getMinIt();

      /**
       * Returns the maximum number of iterations allowed for POPMUSIC_TABU, POPMUSIC_CHAIN and POPMUSIC_TABU_CHAIN.
       * \see getMinIt()
       */
      int getMaxIt();

  };

} // end namespace pal

Q_DECLARE_OPERATORS_FOR_FLAGS( pal::LineArrangementFlags )

#endif
