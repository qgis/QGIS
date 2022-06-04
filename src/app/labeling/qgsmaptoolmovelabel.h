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
#include "qgis_app.h"

//! A map tool for dragging label positions
class APP_EXPORT QgsMapToolMoveLabel: public QgsMapToolLabel
{
    Q_OBJECT

  public:
    QgsMapToolMoveLabel( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDock );
    ~QgsMapToolMoveLabel();

    void deleteRubberBands() override;

    void cadCanvasMoveEvent( QgsMapMouseEvent *e ) override;
    void cadCanvasPressEvent( QgsMapMouseEvent *e ) override;
    void cadCanvasReleaseEvent( QgsMapMouseEvent *e ) override;
    void canvasReleaseEvent( QgsMapMouseEvent *e ) override;
    void keyPressEvent( QKeyEvent *e ) override;
    void keyReleaseEvent( QKeyEvent *e ) override;

  protected:

    bool canModifyCallout( const QgsCalloutPosition &position, bool isOrigin, int &xCol, int &yCol ) override;

    bool mCurrentCalloutMoveOrigin = false;

    QgsRubberBand *mCalloutMoveRubberBand = nullptr;

    //! Start point of the move in map coordinates
    QgsPointXY mStartPointMapCoords;

    double mClickOffsetX = 0;
    double mClickOffsetY = 0;

  private:
    bool currentCalloutDataDefinedPosition( double &x, bool &xSuccess, double &y, bool &ySuccess, int &xCol, int &yCol );

    QgsPointXY snapCalloutPointToCommonAngle( const QgsPointXY &mapPoint, bool showStatusMessage ) const;

    bool mAnchorDetached = false;
    double mLabelTearFromLineThreshold = 0;

};

#endif // QGSMAPTOOLMOVELABEL_H
