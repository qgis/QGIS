/***************************************************************************
                          qgselevationprofiletoolidentify.h
                          ---------------
    begin                : April 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSELEVATIONPROFILETOOLIDENTIFY_H
#define QGSELEVATIONPROFILETOOLIDENTIFY_H

#include "qgsplottool.h"
#include "qgspoint.h"

class QgsPlotRectangularRubberBand;
class QgsElevationProfileCanvas;

class QgsElevationProfileToolIdentify : public QgsPlotTool
{

    Q_OBJECT

  public:

    QgsElevationProfileToolIdentify( QgsElevationProfileCanvas *canvas );
    ~QgsElevationProfileToolIdentify() override;

    Qgis::PlotToolFlags flags() const override;
    void plotPressEvent( QgsPlotMouseEvent *event ) override;
    void plotReleaseEvent( QgsPlotMouseEvent *event ) override;
    void plotMoveEvent( QgsPlotMouseEvent *event ) override;
  private:

    //! Start position for mouse press
    QPointF mMousePressStartPos;
    QgsPointXY mSnappedMousePressStartPos;

    bool mMarquee = false;

    //! Rubber band item
    std::unique_ptr< QgsPlotRectangularRubberBand > mRubberBand;

};

#endif // QGSELEVATIONPROFILETOOLIDENTIFY_H
