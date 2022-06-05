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

#ifndef PAL_LAYER_H_
#define PAL_LAYER_H_

#define SIP_NO_FILE


#include "qgis_core.h"
#include "pal.h" // for LineArrangementFlags enum
#include "qgsgeos.h"
#include "qgsgenericspatialindex.h"
#include "qgslabelobstaclesettings.h"
#include <QMutex>
#include <QLinkedList>
#include <QHash>
#include <fstream>

class QgsLabelFeature;

namespace pal
{

  class FeaturePart;

  class Pal;
  class LabelInfo;

  /**
   * \ingroup core
   * \brief A set of features which influence the labeling process
   * \class pal::Layer
   * \note not available in Python bindings
   */
  class CORE_EXPORT Layer
  {
      friend class Pal;
      friend class FeaturePart;

      friend class Problem;

      friend class LabelPosition;

    public:

      /**
       * \brief Create a new layer
       *
       * \param provider Associated provider
       * \param name Name of the layer (for stats, debugging - does not need to be unique)
       * \param arrangement Arrangement mode : how to place candidates
       * \param defaultPriority layer's prioriry (0 is the best, 1 the worst)
       * \param active is the layer is active (currently displayed)
       * \param toLabel the layer will be labeled whether toLablel is TRUE
       * \param pal pointer to the pal object
       */
      Layer( QgsAbstractLabelProvider *provider, const QString &name, Qgis::LabelPlacement arrangement, double defaultPriority, bool active, bool toLabel, Pal *pal );

      virtual ~Layer();

      /**
       * Returns the number of features in layer.
       */
      int featureCount() { return mHashtable.size(); }

      /**
       * Returns the maximum number of point label candidates to generate for features
       * in this layer.
       */
      std::size_t maximumPointLabelCandidates() const
      {
        // when an extreme number of features exist in the layer, we limit the number of candidates
        // to avoid the engine processing endlessly...
        const int size = mHashtable.size();
        if ( size > 1000 )
          return static_cast< std::size_t >( mPal->globalCandidatesLimitPoint() > 0 ? std::min( mPal->globalCandidatesLimitPoint(), 4 ) : 4 );
        else if ( size > 500 )
          return static_cast< std::size_t >( mPal->globalCandidatesLimitPoint() > 0 ? std::min( mPal->globalCandidatesLimitPoint(), 6 ) : 6 );
        else if ( size > 200 )
          return static_cast< std::size_t >( mPal->globalCandidatesLimitPoint() > 0 ? std::min( mPal->globalCandidatesLimitPoint(), 8 ) : 8 );
        else if ( size > 100 )
          return static_cast< std::size_t >( mPal->globalCandidatesLimitPoint() > 0 ? std::min( mPal->globalCandidatesLimitPoint(), 12 ) : 12 );
        else
          return static_cast< std::size_t >( std::max( mPal->globalCandidatesLimitPoint(), 0 ) );
      }

      /**
       * Returns the maximum number of line label candidates to generate for features
       * in this layer.
       */
      std::size_t maximumLineLabelCandidates() const
      {
        // when an extreme number of features exist in the layer, we limit the number of candidates
        // to avoid the engine processing endlessly...
        const int size = mHashtable.size();
        if ( size > 1000 )
          return static_cast< std::size_t >( mPal->globalCandidatesLimitLine() > 0 ? std::min( mPal->globalCandidatesLimitLine(), 5 ) : 5 );
        else if ( size > 500 )
          return static_cast< std::size_t >( mPal->globalCandidatesLimitLine() > 0 ? std::min( mPal->globalCandidatesLimitLine(), 10 ) : 10 );
        else if ( size > 200 )
          return static_cast< std::size_t >( mPal->globalCandidatesLimitLine() > 0 ? std::min( mPal->globalCandidatesLimitLine(), 20 ) : 20 );
        else if ( size > 100 )
          return static_cast< std::size_t >( mPal->globalCandidatesLimitLine() > 0 ? std::min( mPal->globalCandidatesLimitLine(), 40 ) : 40 );
        else
          return static_cast< std::size_t >( std::max( mPal->globalCandidatesLimitLine(), 0 ) );
      }

      /**
       * Returns the maximum number of polygon label candidates to generate for features
       * in this layer.
       */
      std::size_t maximumPolygonLabelCandidates() const
      {
        // when an extreme number of features exist in the layer, we limit the number of candidates
        // to avoid the engine processing endlessly...
        const int size = mHashtable.size();
        if ( size > 1000 )
          return static_cast< std::size_t >( mPal->globalCandidatesLimitPolygon() > 0 ? std::min( mPal->globalCandidatesLimitPolygon(), 5 ) : 5 );
        else if ( size > 500 )
          return static_cast< std::size_t >( mPal->globalCandidatesLimitPolygon() > 0 ? std::min( mPal->globalCandidatesLimitPolygon(), 15 ) : 15 );
        else if ( size > 200 )
          return static_cast< std::size_t >( mPal->globalCandidatesLimitPolygon() > 0 ? std::min( mPal->globalCandidatesLimitPolygon(), 20 ) : 20 );
        else if ( size > 100 )
          return static_cast< std::size_t >( mPal->globalCandidatesLimitPolygon() > 0 ? std::min( mPal->globalCandidatesLimitPolygon(), 25 ) : 25 );
        else
          return static_cast< std::size_t >( std::max( mPal->globalCandidatesLimitPolygon(), 0 ) );
      }

      //! Returns pointer to the associated provider
      QgsAbstractLabelProvider *provider() const { return mProvider; }

      /**
       * Returns the layer's name.
       */
      QString name() const { return mName; }

      /**
       * Returns the layer's arrangement policy.
       * \see setArrangement
       */
      Qgis::LabelPlacement arrangement() const { return mArrangement; }

      /**
       * Returns TRUE if the layer has curved labels
       */
      bool isCurved() const { return mArrangement == Qgis::LabelPlacement::Curved || mArrangement == Qgis::LabelPlacement::PerimeterCurved; }

      /**
       * Sets the layer's arrangement policy.
       * \param arrangement arrangement policy
       * \see arrangement
       */
      void setArrangement( Qgis::LabelPlacement arrangement ) { mArrangement = arrangement; }

      /**
       * \brief Sets whether the layer is currently active.
       *
       * Active means "is currently displayed or used as obstacles". When a layer is
       * deactivated then feature of this layer will not be used for either
       * labeling or as obstacles.
       *
       * \param active set to TRUE to make the layer active, or FALSE to deactivate the layer
       * \see active
       */
      void setActive( bool active ) { mActive = active; }

      /**
       * Returns whether the layer is currently active.
       * \see setActive
       */
      bool active() const { return mActive; }

      /**
       * Sets whether the layer will be labeled.
       * \note Layers are labelled if and only if labelLayer and active are TRUE
       * \param toLabel set to FALSE disable labeling this layer
       * \see labelLayer
       * \see setActive
       */
      void setLabelLayer( bool toLabel ) { mLabelLayer = toLabel; }

      /**
       * Returns whether the layer will be labeled or not.
       * \see setLabelLayer
       */
      bool labelLayer() const { return mLabelLayer; }

      /**
       * Returns the obstacle type, which controls how features within the layer
       * act as obstacles for labels.
       * \see setObstacleType
       */
      QgsLabelObstacleSettings::ObstacleType obstacleType() const { return mObstacleType; }

      /**
       * Sets the obstacle type, which controls how features within the layer
       * act as obstacles for labels.
       * \param obstacleType new obstacle type
       * \see obstacleType
       */
      void setObstacleType( QgsLabelObstacleSettings::ObstacleType obstacleType ) { mObstacleType = obstacleType; }

      /**
       * Sets the layer's priority.
       * \param priority layer priority, between 0 and 1. 0 corresponds to highest priority,
       * 1 to lowest priority.
       * \see priority
       */
      void setPriority( double priority );

      /**
       * Returns the layer's priority, between 0 and 1. 0 corresponds to highest priority,
       * 1 to lowest priority.
       * \see setPriority
       */
      double priority() const { return mDefaultPriority; }

      /**
       * Sets whether connected lines should be merged before labeling
       * \param merge set to TRUE to merge connected lines
       * \see mergeConnectedLines
       */
      void setMergeConnectedLines( bool merge ) { mMergeLines = merge; }

      /**
       * Returns whether connected lines will be merged before labeling.
       * \see setMergeConnectedLines
       */
      bool mergeConnectedLines() const { return mMergeLines; }

      /**
       * Sets how upside down labels will be handled within the layer.
       * \param ud upside down label handling mode
       * \see upsidedownLabels
       */
      void setUpsidedownLabels( Qgis::UpsideDownLabelHandling ud ) { mUpsidedownLabels = ud; }

      /**
       * Returns how upside down labels are handled within the layer.
       * \see setUpsidedownLabels
       */
      Qgis::UpsideDownLabelHandling upsidedownLabels() const { return mUpsidedownLabels; }

      /**
       * Sets whether labels placed at the centroid of features within the layer
       * are forced to be placed inside the feature's geometry.
       * \param forceInside set to TRUE to force centroid labels to be within the
       * feature. If set to FALSE then the centroid may fall outside the feature.
       * \see centroidInside
       */
      void setCentroidInside( bool forceInside ) { mCentroidInside = forceInside; }

      /**
       * Returns whether labels placed at the centroid of features within the layer
       * are forced to be placed inside the feature's geometry.
       * \see setCentroidInside
       */
      bool centroidInside() const { return mCentroidInside; }

      /**
       * Register a feature in the layer.
       *
       * Does not take ownership of the label feature (it is owned by its provider).
       *
       * \throws PalException::FeatureExists
       *
       * \returns TRUE on success (i.e. valid geometry)
       */
      bool registerFeature( QgsLabelFeature *label );

      //! Join connected features with the same label text
      void joinConnectedFeatures();

      /**
       * Returns the connected feature ID for a label feature ID, which is unique for all features
       * which have been joined as a result of joinConnectedFeatures()
       * \returns connected feature ID, or -1 if feature was not joined
       */
      int connectedFeatureId( QgsFeatureId featureId ) const;

      //! Chop layer features at the repeat distance
      void chopFeaturesAtRepeatDistance();

    protected:
      QgsAbstractLabelProvider *mProvider; // not owned
      QString mName;

      //! List of feature parts
      std::deque< std::unique_ptr< FeaturePart > > mFeatureParts;

      //! List of obstacle parts
      QList<FeaturePart *> mObstacleParts;

      std::vector< geos::unique_ptr > mGeosObstacleGeometries;

      Pal *mPal = nullptr;

      double mDefaultPriority;

      QgsLabelObstacleSettings::ObstacleType mObstacleType = QgsLabelObstacleSettings::PolygonBoundary;
      bool mActive;
      bool mLabelLayer;
      bool mCentroidInside = false;

      //! Optional flags used for some placement methods
      Qgis::LabelPlacement mArrangement;

      bool mMergeLines = false;

      Qgis::UpsideDownLabelHandling mUpsidedownLabels = Qgis::UpsideDownLabelHandling::FlipUpsideDownLabels;

      //! Lookup table of label features (owned by the label feature provider that created them)
      QHash< QgsFeatureId, QgsLabelFeature *> mHashtable;

      QHash< QString, QVector<FeaturePart *> > mConnectedHashtable;
      QHash< QgsFeatureId, int > mConnectedFeaturesIds;

      QMutex mMutex;

      //! Add newly created feature part into r tree and to the list
      void addFeaturePart( std::unique_ptr< FeaturePart > fpart, const QString &labelText = QString() );

      //! Add newly created obstacle part into r tree and to the list
      void addObstaclePart( FeaturePart *fpart );

  };

} // end namespace pal


#endif
