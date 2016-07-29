/***************************************************************************
    qgsmaptoolrotatepointsymbols.h
    ---------------------
    begin                : September 2009
    copyright            : (C) 2009 by Marco Hugentobler
    email                : marco at hugis dot net
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLROTATEPOINTSYMBOLS_H
#define QGSMAPTOOLROTATEPOINTSYMBOLS_H

#include "qgsmaptoolpointsymbol.h"

class QgsPointRotationItem;
class QgsMarkerSymbolV2;

/** \ingroup app
 * \class QgsMapToolRotatePointSymbols
 * \brief A class that allows interactive manipulation the value of the rotation field(s) for point layers.
 */

class APP_EXPORT QgsMapToolRotatePointSymbols: public QgsMapToolPointSymbol
{
    Q_OBJECT

  public:
    QgsMapToolRotatePointSymbols( QgsMapCanvas* canvas );
    ~QgsMapToolRotatePointSymbols();

    void canvasPressEvent( QgsMapMouseEvent* e ) override;
    void canvasMoveEvent( QgsMapMouseEvent* e ) override;
    void canvasReleaseEvent( QgsMapMouseEvent* e ) override;

    /** Returns true if the symbols of a maplayer can be rotated. This means the layer
      is a vector layer, has type point or multipoint and has at least one rotation attribute in the renderer*/
    static bool layerIsRotatable( QgsMapLayer* ml );

  protected:

    virtual void canvasPressOnFeature( QgsMapMouseEvent* e, const QgsFeature& feature, const QgsPoint& snappedPoint ) override;
    virtual bool checkSymbolCompatibility( QgsMarkerSymbolV2* markerSymbol, QgsRenderContext& context ) override;
    virtual void noCompatibleSymbols() override;

  private:

    /** Last azimut between mouse and edited point*/
    double mCurrentMouseAzimut;
    /** Last feature rotation*/
    double mCurrentRotationFeature;
    bool mRotating;
    QSet<int> mCurrentRotationAttributes;
    /** Item that displays rotation during mouse move*/
    QgsPointRotationItem* mRotationItem;
    /** True if ctrl was pressed during the last mouse move event*/
    bool mCtrlPressed;
    //! Clone of first found marker symbol for feature with rotation attribute set
    QScopedPointer< QgsMarkerSymbolV2 > mMarkerSymbol;

    void drawArrow( double azimut ) const;
    /** Calculates the azimut between mousePos and mSnappedPoint*/
    double calculateAzimut( QPoint mousePos );
    /** Create item with the point symbol for a specific feature. This will be used to show the rotation to the user*/
    void createPixmapItem( QgsMarkerSymbolV2 *markerSymbol );
    /** Sets the rotation of the pixmap item*/
    void setPixmapItemRotation( double rotation );
    /** Rounds value to 15 degree integer (used if ctrl pressed)*/
    static int roundTo15Degrees( double n );

};

#endif // QGSMAPTOOLROTATEPOINTSYMBOLS_H
