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

#include "qgsmaptool.h"
#include "qgsmapcanvasitem.h"
#include "qgsfeatureid.h"
#include "qobjectuniqueptr.h"
#include "qgslinesymbollayer.h"
#include "qgssymbol.h"
#include "qgsrubberband.h"

class QgsMapToolBlankSegmentRubberBand;
class QgsVectorLayer;
class QgsSymbol;
class QgsSymbolLayer;

/**
 * \ingroup gui
 * \brief Map tool base class to edit blank segments. Digitized blank segments are stored per feature
 * inside a field as a string ( \see QgsBlankSegmentUtils::parseBlankSegments for string format)
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
    // compute and return current blank segment start and end distance
    QPair<double, double> getStartEndDistance() const;
    void updateAttribute();
    void loadFeaturePoints();

    virtual QgsTemplatedLineSymbolLayerBase *createRenderedPointsSymbolLayer( const QgsTemplatedLineSymbolLayerBase *original ) = 0;

    int getClosestBlankSegmentIndex( const QPointF &point, double &distance ) const;
    QPointF getClosestPoint( const QPointF &point, double &distance, int &partIndex, int &ringIndex, int &pointIndex ) const;

    void updateStartEndRubberBand();
    void updateHoveredBlankSegment( const QPoint &pos );
    void setCurrentBlankSegment( int currentBlankSegmentIndex );

    class BlankSegment : public QgsRubberBand
    {
      public:
        BlankSegment( int partIndex, int ringIndex, int startIndex, int endIndex, QPointF startPt, QPointF endPt, QgsMapCanvas *canvas, const FeaturePoints &points );
        BlankSegment( QgsMapCanvas *canvas, const FeaturePoints &points );

        void setPoints( int partIndex, int ringIndex, int startIndex, int endIndex, QPointF startPt, QPointF endPt );
        void copyFrom( const BlankSegment &blankSegment );
        void setHighlighted( bool highlighted );

        const QPointF &getStartPoint() const;
        const QPointF &getEndPoint() const;
        int getStartIndex() const;
        int getEndIndex() const;
        int getPartIndex() const;
        int getRingIndex() const;

        QPair<double, double> getStartEndDistance( Qgis::RenderUnit unit ) const;

        int pointsCount() const;
        const QPointF &pointAt( int index ) const;

      private:
        void updatePoints();

        int mPartIndex = -1;
        int mRingIndex = -1;
        int mStartIndex = -1;
        int mEndIndex = -1;
        QPointF mStartPt;
        QPointF mEndPt;
        bool mNeedSwap = false;
        const FeaturePoints &mPoints; //! all feature rendered points
    };

    enum State
    {
      SELECT_FEATURE,
      FEATURE_SELECTED,
      BLANK_SEGMENT_SELECTED,
      BLANK_SEGMENT_MODIFICATION_STARTED,
      BLANK_SEGMENT_CREATION_STARTED
    };

    std::vector<QObjectUniquePtr<BlankSegment>> mBlankSegments;
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
    QObjectUniquePtr<BlankSegment> mEditedBlankSegment;
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
