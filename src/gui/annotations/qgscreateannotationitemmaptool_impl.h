/***************************************************************************
                             qgscreateannotationitemmaptool_impl.h
                             ------------------------
    Date                 : September 2021
    Copyright            : (C) 2021 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCREATEANNOTATIONITEMMAPTOOLIMPL_H
#define QGSCREATEANNOTATIONITEMMAPTOOLIMPL_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgscreateannotationitemmaptool.h"
#include "qgsmaptoolcapture.h"

#define SIP_NO_FILE

///@cond PRIVATE

class QgsMapToolCaptureAnnotationItem: public QgsMapToolCapture, public QgsCreateAnnotationItemMapToolInterface
{
    Q_OBJECT
  public:
    QgsMapToolCaptureAnnotationItem( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget, CaptureMode mode );
    QgsCreateAnnotationItemMapToolHandler *handler() override;
    QgsMapTool *mapTool() override;
    QgsMapLayer *layer() const override;
    QgsMapToolCapture::Capabilities capabilities() const override;
    bool supportsTechnique( Qgis::CaptureTechnique technique ) const override;

  protected:

    QgsCreateAnnotationItemMapToolHandler *mHandler = nullptr;

};

class QgsCreatePointTextItemMapTool: public QgsMapToolAdvancedDigitizing, public QgsCreateAnnotationItemMapToolInterface
{
    Q_OBJECT

  public:

    QgsCreatePointTextItemMapTool( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget );
    ~QgsCreatePointTextItemMapTool() override;

    void cadCanvasPressEvent( QgsMapMouseEvent *event ) override;
    QgsCreateAnnotationItemMapToolHandler *handler() override;
    QgsMapTool *mapTool() override;

  private:

    QgsCreateAnnotationItemMapToolHandler *mHandler = nullptr;

};


class QgsCreateMarkerItemMapTool: public QgsMapToolCaptureAnnotationItem
{
    Q_OBJECT

  public:

    QgsCreateMarkerItemMapTool( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget );

    void cadCanvasReleaseEvent( QgsMapMouseEvent *event ) override;

};

class QgsCreateLineItemMapTool: public QgsMapToolCaptureAnnotationItem
{
    Q_OBJECT

  public:

    QgsCreateLineItemMapTool( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget );

  private slots:
    void lineCaptured( const QgsCurve *line ) override;
};

class QgsCreatePolygonItemMapTool: public QgsMapToolCaptureAnnotationItem
{
    Q_OBJECT

  public:

    QgsCreatePolygonItemMapTool( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget );

  private slots:
    void polygonCaptured( const QgsCurvePolygon *polygon ) override;
};

///@endcond PRIVATE

#endif // QGSCREATEANNOTATIONITEMMAPTOOLIMPL_H
