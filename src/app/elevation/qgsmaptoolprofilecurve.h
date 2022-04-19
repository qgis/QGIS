/***************************************************************************
                          qgsmaptoolprofilecurve.h
                          ---------------
    begin                : March 2022
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
#ifndef QGSMAPTOOLPROFILECURVE_H
#define QGSMAPTOOLPROFILECURVE_H

#include "qgsmaptoolcapture.h"

#include <QPointer>

class QgsMapToolProfileCurve : public QgsMapToolCapture
{
    Q_OBJECT

  public:
    QgsMapToolProfileCurve( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget );

    ~QgsMapToolProfileCurve() override;

    QgsMapToolCapture::Capabilities capabilities() const override;
    bool supportsTechnique( Qgis::CaptureTechnique technique ) const override;
    void keyPressEvent( QKeyEvent *e ) override;
    void cadCanvasReleaseEvent( QgsMapMouseEvent *e ) override;
    QgsMapLayer *layer() const override;
  signals:

    void captureStarted();
    void captureCanceled();
    void curveCaptured( const QgsGeometry &curve );

  private:
    void lineCaptured( const QgsCurve *line ) override;

    QPointer< QgsMapTool > mPreviousTool;


};

#endif // QGSMAPTOOLPROFILECURVE_H
