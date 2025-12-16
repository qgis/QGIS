/***************************************************************************
    qgsmaptooleditblanksegments.h
    ---------------------
    begin                : 2025/08/19
    copyright            : (C) 2025 by Julien Cabieces
    email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLEDITBLANKSEGMENTS_H
#define QGSMAPTOOLEDITBLANKSEGMENTS_H

#define SIP_NO_FILE

#include "qgsfeatureid.h"
#include "qgslinesymbollayer.h"
#include "qgsmapcanvasitem.h"
#include "qgsmaptool.h"
#include "qgsrubberband.h"
#include "qgssymbol.h"
#include "qobjectuniqueptr.h"

class QgsMapToolBlankSegmentRubberBand;
class QgsVectorLayer;
class QgsSymbol;
class QgsSymbolLayer;

/**
 * \ingroup gui
 * \brief Map tool base class to edit blank segments. Digitized blank segments are stored per feature
 * inside a field as a string ( \see QgsBlankSegmentUtils::parseBlankSegments for string format)
 *
 * This tool allows to:
 *
 * - Click in the neighborhood of the rendered line to start creating a blank segment, click again to
 *
 * finish editing the segment.
 *
 * - Select a blank segment, press Del key to remove it
 * - Drag the start/end of an already existing blank segment and move it along the line to
 *
 * reduce/enlarge it
 *
 * \see QgsMapToolEditBlankSegments
 * \since QGIS 4.0
*/
class GUI_EXPORT QgsMapToolEditBlankSegmentsBase : public QgsMapTool
{
    Q_OBJECT

  public:
    /**
     * Constructor
     * \param canvas map canvas where the edit take place
     * \param layer layer to be edited
     * \param symbolLayer symbol layer affected by the blank segments
     * \param blankSegmentFieldIndex index of the field containing the digitized blank segments
     */
    QgsMapToolEditBlankSegmentsBase( QgsMapCanvas *canvas, QgsVectorLayer *layer, QgsLineSymbolLayer *symbolLayer, int blankSegmentFieldIndex );

    /**
     * Destructor
     */
    ~QgsMapToolEditBlankSegmentsBase();

    void canvasMoveEvent( QgsMapMouseEvent *e ) override;
    void canvasPressEvent( QgsMapMouseEvent *e ) override;
    void keyPressEvent( QKeyEvent *e ) override;

    void activate() override;

  protected:
    typedef QList<QList<QPolygonF>> FeaturePoints;
    FeaturePoints mPoints;

  private:
    /**
     * compute and return current blank segment start and end distance
     */
    QPair<double, double> getStartEndDistance() const;

    /**
     * Update feature attribute based on current edited blank segments
     */
    void updateAttribute();

    /**
     * Load current feature rendered points and feature existing blank segments
     */
    void loadFeaturePoints();

    /**
     * Create symbol layer used to retrieve rendered points from symbol layer
     * The feature will be rendered using this symbol layer so we can retrieve the rendered points in
     * renderPolyline() methods
     */
    virtual QgsTemplatedLineSymbolLayerBase *createRenderedPointsSymbolLayer( const QgsTemplatedLineSymbolLayerBase *original ) = 0;

    /**
     * Returns feature blank segment index closest to \a point, -1 if none could be found (there is
     * no existing blank segments for instance). \a distance will be set to the distance between
     * \a point and the returned blank segment
     */
    int getClosestBlankSegmentIndex( const QPointF &point, double &distance ) const;

    /**
     * Returns rendered point closest to \a point
     * \param distance updated with the distance between \a point and the returned rendered point
     * \param partIndex will be set to the geometry part index of the returned point
     * \param ringIndex will be set to the geometry ring index of the returned point
     * \param pointIndex will be set to the rendered points index of the returned point
     */
    QPointF getClosestPoint( const QPointF &point, double &distance, int &partIndex, int &ringIndex, int &pointIndex ) const;

    /**
     * Update start and end rubber band (anchor points used to resize the blank segment rubber band)
     * according to current map tool state and selected blank segment
     */
    void updateStartEndRubberBand();

    /**
     * Update currently hovered (if any) blank segment according to mouse position \a pos
     */
    void updateHoveredBlankSegment( const QPoint &pos );

    /**
     * Set currently edited blank segment
     */
    void setCurrentBlankSegment( int currentBlankSegmentIndex );

    /**
     * Rubber band used to draw blank segments on edition
     */
    class BlankSegmentRubberBand;

    /**
     * Map tool state
     */
    enum State
    {
      SELECT_FEATURE,                     //!< User needs to select a feature
      FEATURE_SELECTED,                   //!< User has selected a feature
      BLANK_SEGMENT_SELECTED,             //!< User has selected a blank segment
      BLANK_SEGMENT_MODIFICATION_STARTED, //!< User has started to drag start/end of an existing blank segment
      BLANK_SEGMENT_CREATION_STARTED      //!< User has started to create a new blank segment
    };

    std::vector<QObjectUniquePtr<BlankSegmentRubberBand>> mBlankSegments;
    QgsVectorLayer *mLayer = nullptr;
    std::unique_ptr<QgsSymbol> mSymbol;
    const QString mSymbolLayerId;
    QgsTemplatedLineSymbolLayerBase *mSymbolLayer = nullptr;

    int mBlankSegmentsFieldIndex = -1;
    QgsFeatureId mCurrentFeatureId = FID_NULL;
    QgsRectangle mExtent;
    State mState = State::SELECT_FEATURE;
    int mCurrentBlankSegmentIndex = -1;
    int mHoveredBlankSegmentIndex = -1;

    // currently edited blank segment, start point is the fixed point and end point is the currently
    // modified one
    QObjectUniquePtr<BlankSegmentRubberBand> mEditedBlankSegment;
    QObjectUniquePtr<QgsRubberBand> mStartRubberBand;
    QObjectUniquePtr<QgsRubberBand> mEndRubberBand;

    friend class TestQgsMapToolEditBlankSegments;
};

/**
 * \ingroup gui
 * \brief Specializes the map tool to edit blank segments given the targeted symbol
 * layer type (QgsMarkerLineSymbolLayer and QgsHashedLineSymbolLayer).
 * \since QGIS 4.0
*/
template<class T>
class GUI_EXPORT QgsMapToolEditBlankSegments : public QgsMapToolEditBlankSegmentsBase
{
  public:
    /**
     * Constructor
     * \param canvas map canvas where the edit take place
     * \param layer layer to be edited
     * \param symbolLayer symbol layer affected by the blank segments
     * \param blankSegmentFieldIndex index of the field containing the digitized blank segments
     */
    QgsMapToolEditBlankSegments<T>( QgsMapCanvas *canvas, QgsVectorLayer *layer, QgsLineSymbolLayer *symbolLayer, int blankSegmentFieldIndex )
      : QgsMapToolEditBlankSegmentsBase( canvas, layer, symbolLayer, blankSegmentFieldIndex )
    {
    }

    QgsTemplatedLineSymbolLayerBase *createRenderedPointsSymbolLayer( const QgsTemplatedLineSymbolLayerBase *originalSl ) override
    {
      const T *sl = dynamic_cast<const T *>( originalSl );
      return sl ? new QgsRenderedPointsSymbolLayer( sl, mPoints ) : nullptr;
    }

  private:
    /**
     * Helper class to retrieve the rendered points from symbol layer. It extends the original
     * template type (either QgsMarkerLineSymbolLayer or QgsHashedLineSymbolLayer)
     */
    class QgsRenderedPointsSymbolLayer : public T
    {
      public:
        /**
         * Construct symbol layer based on the \a original symbol layer. \a points will be updated with
         * the generated rendered points
         */
        QgsRenderedPointsSymbolLayer( const T *original, FeaturePoints &points )
          : T( original->rotateSymbols(), original->interval() )
          , mPoints( points )
        {
          original->copyTemplateSymbolProperties( this );
        }

        void renderPolyline( const QPolygonF &points, QgsSymbolRenderContext &context ) override
        {
          const int iPart = context.geometryPartNum() - 1;
          if ( iPart < 0 || QgsRenderedPointsSymbolLayer::mRingIndex < 0 )
            return;

          if ( iPart >= mPoints.count() )
            mPoints.resize( iPart + 1 );

          QVector<QPolygonF> &rings = mPoints[iPart];
          if ( QgsRenderedPointsSymbolLayer::mRingIndex >= rings.count() )
            rings.resize( QgsRenderedPointsSymbolLayer::mRingIndex + 1 );

          rings[QgsRenderedPointsSymbolLayer::mRingIndex] = points;
        }

      private:
        FeaturePoints &mPoints;
    };
};


#endif
