/***************************************************************************
    qgsmaptoolmeasureangle.h
    ------------------------
    begin                : December 2009
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

#ifndef QGSMAPTOOLMEASUREANGLE_H
#define QGSMAPTOOLMEASUREANGLE_H

#include "qgsmaptool.h"
#include "qgspointxy.h"
#include "qgsdistancearea.h"
#include "qgis_app.h"

class QgsDisplayAngle;
class QgsRubberBand;
class QgsSnapIndicator;

//! Map tool to measure angle between two segments
class APP_EXPORT QgsMapToolMeasureAngle: public QgsMapTool
{
    Q_OBJECT
  public:
    QgsMapToolMeasureAngle( QgsMapCanvas *canvas );
    ~QgsMapToolMeasureAngle() override;

    Flags flags() const override { return QgsMapTool::AllowZoomRect; }

    //! Mouse move event for overriding
    void canvasMoveEvent( QgsMapMouseEvent *e ) override;

    //! Mouse release event for overriding
    void canvasReleaseEvent( QgsMapMouseEvent *e ) override;

    //! called when set as currently active map tool
    void activate() override;

    //! called when map tool is being deactivated
    void deactivate() override;

  private:
    //! Points defining the angle (three for measuring)
    QList<QgsPointXY> mAnglePoints;
    QgsRubberBand *mRubberBand = nullptr;
    QgsDisplayAngle *mResultDisplay = nullptr;

    //! Creates a new rubber band and deletes the old one
    void createRubberBand();

    //! Tool for measuring
    QgsDistanceArea mDa;

    std::unique_ptr<QgsSnapIndicator> mSnapIndicator;

  public slots:
    //! Recalculate angle if projection state changed
    void updateSettings();

  private slots:
    //! Deletes the rubber band and the dialog
    void stopMeasuring();

    //! Configures distance area objects with ellipsoid / output crs
    void configureDistanceArea();

};

#endif // QGSMAPTOOLMEASUREANGLE_H
