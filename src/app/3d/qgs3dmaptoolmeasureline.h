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
#include "qgsvectorlayer.h"
#include "qgsvector3d.h"
#include "qgslinestring.h"
#include "qgsfeature.h"
#include "qgsline3dsymbol.h"
#include "qgspoint.h"
#include "qgsvectorlayer3drenderer.h"
//#include "qgs3dmeasuredialog.h"

#include <memory>

namespace Qt3DRender
{
  class QPickEvent;
}

class Qgs3DMapToolMeasureLinePickHandler;
class Qgs3DMeasureDialog;


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
    void removeLastPoint();

    //! Returns reference to array of the points
    QVector<QgsPoint> points() const;

    //! Catching key press event
    void keyPressEvent( QKeyEvent *e );

    // Inherited from Qgs3DMapTool
    void mousePressEvent( QMouseEvent *event ) override { Q_UNUSED( event )}
    void mouseReleaseEvent( QMouseEvent *event ) override { Q_UNUSED( event )}
    void mouseMoveEvent( QMouseEvent *event ) override {Q_UNUSED( event )}

    void activate() override;
    void deactivate() override;

  private slots:
    void onTerrainPicked( Qt3DRender::QPickEvent *event );
    void onTerrainEntityChanged();

  private:
    std::unique_ptr<Qgs3DMapToolMeasureLinePickHandler> mPickHandler;

    friend class Qgs3DMapToolMeasureLinePickHandler;

    QgsVectorLayer *mMeasurementLayer = nullptr;
    QgsFeature *mMeasurementFeature = nullptr;
    QgsLineString *mMeasurementLine = nullptr;
    QgsLine3DSymbol *mLineSymbol = nullptr;
    QgsVectorLayer3DRenderer *mLineSymbolRenderer = nullptr;


    //! Store points
    QVector<QgsPoint> mPoints;

    //! Indicates whether we've just done a right mouse click
    bool mDone = true;

    //! Set the line layer renderer
    void setMeasurementLayerRenderer( QgsVectorLayer *layer );

    //! Dialog
    Qgs3DMeasureDialog *mDialog = nullptr;
};

#endif // QGS3DMAPTOOLMEASURELINE_H
