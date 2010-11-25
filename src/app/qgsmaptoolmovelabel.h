/***************************************************************************
                          qgsmaptoolmovelabel.h
                          --------------------
    begin                : 2010-11-03
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLMOVELABEL_H
#define QGSMAPTOOLMOVELABEL_H

#include "qgsmaptoollabel.h"

/**A map tool for dragging label positions*/
class QgsMapToolMoveLabel: public QgsMapToolLabel
{
    Q_OBJECT

  public:
    QgsMapToolMoveLabel( QgsMapCanvas* canvas );
    ~QgsMapToolMoveLabel();

    virtual void canvasPressEvent( QMouseEvent * e );

    virtual void canvasMoveEvent( QMouseEvent * e );

    virtual void canvasReleaseEvent( QMouseEvent * e );

  protected:
    /**Get data defined position of a feature
      @param layerId layer identification string
      @param x out: data defined x-coordinate
      @param xSuccess out: false if attribute value is NULL
      @param y out: data defined y-coordinate
      @param ySuccess out: false if attribute value is NULL
      @param xCol out: index of the x position column
      @param yCol out: index of the y position column
      @return false if layer does not have data defined label position enabled*/
    bool dataDefinedPosition( QgsVectorLayer* vlayer, int featureId, double& x, bool& xSuccess, double& y, bool& ySuccess, int& xCol, int& yCol ) const;

    /**Returns true if layer move can be applied to a layer
      @param xCol out: index of the attribute for data defined x coordinate
      @param yCol out: index of the attribute for data defined y coordinate
      @return true if labels of layer can be moved*/
    bool layerIsMoveable( const QgsMapLayer* ml, int& xCol, int& yCol ) const;

    /**Start point of the move in map coordinates*/
    QgsPoint mStartPointMapCoords;

    double mClickOffsetX;
    double mClickOffsetY;
};

#endif // QGSMAPTOOLMOVELABEL_H
