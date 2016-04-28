/***************************************************************************
    qgsmaptooloffsetpointsymbol.h
    -----------------------------
    begin                : April 2016
    copyright            : (C) 2016 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLOFFSETPOINTSYMBOL_H
#define QGSMAPTOOLOFFSETPOINTSYMBOL_H

#include "qgsmaptoolpointsymbol.h"
#include "qgssymbolv2.h"

class QgsMarkerSymbolV2;
class QgsPointMarkerItem;

/** \ingroup app
 * \class QgsMapToolOffsetPointSymbol
 * \brief A class that allows interactive manipulation of the offset field(s) for point layers.
 */

class APP_EXPORT QgsMapToolOffsetPointSymbol: public QgsMapToolPointSymbol
{
    Q_OBJECT

  public:
    QgsMapToolOffsetPointSymbol( QgsMapCanvas* canvas );
    ~QgsMapToolOffsetPointSymbol();

    void canvasPressEvent( QgsMapMouseEvent* e ) override;
    void canvasMoveEvent( QgsMapMouseEvent* e ) override;
    void canvasReleaseEvent( QgsMapMouseEvent* e ) override;

    /** Returns true if the symbols of a map layer can be offset. This means the layer
     *  is a vector layer, has type point or multipoint and has at least one offset attribute in the renderer.
    */
    static bool layerIsOffsetable( QgsMapLayer* ml );

  protected:

    virtual void canvasPressOnFeature( QgsMapMouseEvent* e, const QgsFeature& feature, const QgsPoint& snappedPoint ) override;
    virtual bool checkSymbolCompatibility( QgsMarkerSymbolV2* markerSymbol, QgsRenderContext& context ) override;
    virtual void noCompatibleSymbols() override;

  private:

    //! True when user is dragging to offset a point
    bool mOffsetting;

    //! Item that previews the offset during mouse move
    QgsPointMarkerItem* mOffsetItem;

    //! Clone of first found marker symbol for feature with offset attribute set
    QScopedPointer< QgsMarkerSymbolV2 > mMarkerSymbol;

    //! Feature which was clicked on
    QgsFeature mClickedFeature;

    //! Point in map units where click originated
    QgsPoint mClickedPoint;

    //! Stores the symbol rotation so that offset can be adjusted to account for rotation
    double mSymbolRotation;

    //! Create item with the point symbol for a specific feature. This will be used to show the offset to the user.
    void createPreviewItem( QgsMarkerSymbolV2 *markerSymbol );

    //! Calculates the new values for offset attributes, respecting the symbol's offset units
    //! @note start and end point are in map units
    QMap< int, QVariant > calculateNewOffsetAttributes( const QgsPoint& startPoint, const QgsPoint& endPoint ) const;

    /** Updates the preview item to reflect a new offset.
     * @note start and end points are in map units
     */
    void updateOffsetPreviewItem( const QgsPoint& startPoint, const QgsPoint& endPoint );

    //! Calculates the required offset from the start to end points, in the specified unit
    //! @note start and end points are in map units
    QPointF calculateOffset( const QgsPoint& startPoint, const QgsPoint& endPoint, QgsSymbolV2::OutputUnit unit ) const;

    //! Adjusts the calculated offset to account for the symbol's rotation
    QPointF rotatedOffset( QPointF offset, double angle ) const;
};

#endif // QGSMAPTOOLOFFSETPOINTSYMBOL_H
