/***************************************************************************
    qgsbeziermarker.h  -  Visualization for Poly-Bézier curve digitizing
    ---------------------
    begin                : December 2025
    copyright            : (C) 2025 by Loïc Bartoletti
                           Adapted from BezierEditing plugin work by Takayuki Mizutani
    email                : loic dot bartoletti at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSBEZIERMARKER_H
#define QGSBEZIERMARKER_H

#include <memory>
#include <vector>

#include "qgis_gui.h"
#include "qgsbezierdata.h"
#include "qobjectuniqueptr.h"

#include <QObject>

#define SIP_NO_FILE

class QgsMapCanvas;
class QgsRubberBand;
class QgsVertexMarker;


///@cond PRIVATE

/**
 * \brief Visualization class for Poly-Bézier curve during digitizing.
 *
 * This class manages the visual representation of anchors, handles,
 * handle lines, and the curve itself on the map canvas.
 *
 * \since QGIS 4.0
 */
class GUI_EXPORT QgsBezierMarker : public QObject
{
    Q_OBJECT

  public:
    /**
     * Configuration constants for marker appearance.
     */
    struct MarkerConfig
    {
        static constexpr int AnchorMarkerSize = 10;
        static constexpr int HandleMarkerSize = 8;
        static constexpr int MarkerPenWidth = 2;
        static constexpr int HandleLineWidth = 1;
        static constexpr int FillAlpha = 100;
        static constexpr int HandleLineAlpha = 150;
        static constexpr int SelectionAlpha = 150;
    };

    /**
     * Constructor.
     * \param canvas the map canvas for rendering
     * \param parent parent object
     */
    explicit QgsBezierMarker( QgsMapCanvas *canvas, QObject *parent = nullptr );

    ~QgsBezierMarker() override;

    /**
     * Updates the visualization from the given Bézier data.
     * \param data the Bézier curve data
     */
    void updateFromData( const QgsBezierData &data );

    /**
     * Updates the curve visualization only (optimization for mouse move).
     * \param data the Bézier curve data
     */
    void updateCurve( const QgsBezierData &data );

    /**
     * Sets visibility of all markers.
     * \param visible whether markers should be visible
     */
    void setVisible( bool visible );

    /**
     * Sets visibility of handles.
     * \param visible whether handles should be visible
     */
    void setHandlesVisible( bool visible );

    //! Clears all markers
    void clear();

    /**
     * Highlights an anchor.
     * \param idx anchor index (-1 for none)
     */
    void setHighlightedAnchor( int idx );

    /**
     * Highlights a handle.
     * \param idx handle index (-1 for none)
     */
    void setHighlightedHandle( int idx );

  private:
    QgsMapCanvas *mCanvas = nullptr;

    std::vector<QgsVertexMarker *> mAnchorMarkers;
    std::vector<QgsVertexMarker *> mHandleMarkers;
    std::vector<QObjectUniquePtr<QgsRubberBand>> mHandleLines;
    QObjectUniquePtr<QgsRubberBand> mCurveRubberBand;

    int mHighlightedAnchor = -1;
    int mHighlightedHandle = -1;

    bool mVisible = true;
    bool mHandlesVisible = true;

    //! Creates a new anchor marker
    QgsVertexMarker *createAnchorMarker();

    //! Creates a new handle marker
    QgsVertexMarker *createHandleMarker();

    //! Creates a new handle line rubber band
    QObjectUniquePtr<QgsRubberBand> createHandleLine();

    //! Updates anchor markers
    void updateAnchorMarkers( const QgsBezierData &data );

    //! Updates handle markers
    void updateHandleMarkers( const QgsBezierData &data );

    //! Updates handle lines
    void updateHandleLines( const QgsBezierData &data );
};

///@endcond PRIVATE

#endif // QGSBEZIERMARKER_H
