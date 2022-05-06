/***************************************************************************
                          qgselevationprofiletoolmeasure.h
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
#ifndef QGSELEVATIONPROFILETOOLMEASURE_H
#define QGSELEVATIONPROFILETOOLMEASURE_H

#include "qgsplottool.h"
#include "qgsprofilepoint.h"

#include <QPointer>

class QgsElevationProfileCanvas;
class QGraphicsLineItem;

class QgsElevationProfileToolMeasure : public QgsPlotTool
{
    Q_OBJECT

  public:
    QgsElevationProfileToolMeasure( QgsElevationProfileCanvas *canvas );

    ~QgsElevationProfileToolMeasure() override;

    void deactivate() override;
    void plotMoveEvent( QgsPlotMouseEvent *event ) override;
    void plotPressEvent( QgsPlotMouseEvent *event ) override;
    void plotReleaseEvent( QgsPlotMouseEvent *event ) override;

  signals:

    void distanceChanged( double distance );
    void cleared();

  private:

    QgsElevationProfileCanvas *mElevationCanvas = nullptr;

    QGraphicsLineItem *mRubberBand = nullptr;

    QgsProfilePoint mStartPoint;
    bool mMeasureInProgress = false;

};

#endif // QGSELEVATIONPROFILETOOLMEASURE_H
