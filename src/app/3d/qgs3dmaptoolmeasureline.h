/***************************************************************************
  qgs3dmaptoolmeasureline.h
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

#ifndef QGS3DMAPTOOLMEASURELINE_H
#define QGS3DMAPTOOLMEASURELINE_H

#include "qgs3dmaptool.h"
#include "qgspoint.h"

#include <memory>


class Qgs3DMeasureDialog;
class QgsRubberBand3D;


class Qgs3DMapToolMeasureLine : public Qgs3DMapTool
{
    Q_OBJECT

  public:
    Qgs3DMapToolMeasureLine( Qgs3DMapCanvas *canvas );
    ~Qgs3DMapToolMeasureLine() override;

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
    bool mIsAlreadyActivated = false;

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
};

#endif // QGS3DMAPTOOLMEASURELINE_H
