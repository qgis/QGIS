/***************************************************************************
  qgs3dmaptoolmeasure.h
  --------------------------------------
  Date                 : Jun 2019
  Copyright            : (C) 2019 by Ismail Sunni
  Email                : imajimatika at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS3DMAPTOOLMEASURE_H
#define QGS3DMAPTOOLMEASURE_H

#include <memory>

#include "qgs3dmaptool.h"
#include "qgspoint.h"

class Qgs3DMeasureDialog;
class QgsRubberBand3D;


class Qgs3DMapToolMeasure : public Qgs3DMapTool
{
    Q_OBJECT

  public:
    Qgs3DMapToolMeasure( Qgs3DMapCanvas *canvas, bool measureArea );
    ~Qgs3DMapToolMeasure() override;

    //! returns whether measuring distance or area
    bool measureArea() const { return mMeasureArea; }

    //! When we have added our last point, and not following
    bool done() const { return mDone; }

    //! Reset and start new
    void restart();

    //! Add new point
    void addPoint( const QgsPoint &point );

    //! Removes the last point
    void undo();

    //! Returns reference to array of the points
    QVector<QgsPoint> points() const;

    //! Update values from settings
    void updateSettings();

    void activate() override;
    void deactivate() override;

    QCursor cursor() const override;

  private slots:
    void handleClick( const QPoint &screenPos );
    void mousePressEvent( QMouseEvent *event ) override;
    void mouseReleaseEvent( QMouseEvent *event ) override;
    void mouseMoveEvent( QMouseEvent *event ) override;
    void keyPressEvent( QKeyEvent *event ) override;

  private:
    //! Store points
    QVector<QgsPoint> mPoints;

    //! Indicates whether we've just done a right mouse click
    bool mDone = true;

    //! Dialog
    std::unique_ptr<Qgs3DMeasureDialog> mDialog;

    std::unique_ptr<QgsRubberBand3D> mRubberBand;

    //! Check if mouse was moved between mousePress and mouseRelease
    bool mMouseHasMoved = false;
    QPoint mMouseClickPos;

    //! Z value for computing map coordinates on mouse move
    float zMean = std::numeric_limits<float>::quiet_NaN();

    //! Indicates whether we're measuring distances or areas
    bool mMeasureArea = false;
};

#endif // QGS3DMAPTOOLMEASURE_H
