/***************************************************************************
                          qgsmaptoolprofilecurvefromfeature.h
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
#ifndef QGSMAPTOOLPROFILECURVEFROMFEATURE_H
#define QGSMAPTOOLPROFILECURVEFROMFEATURE_H

#include "qgsmaptool.h"

class QgsGeometry;

class QgsMapToolProfileCurveFromFeature : public QgsMapTool
{
    Q_OBJECT

  public:
    QgsMapToolProfileCurveFromFeature( QgsMapCanvas *canvas );
    Flags flags() const override;
    void canvasPressEvent( QgsMapMouseEvent *e ) override;

  signals:

    void curveCaptured( const QgsGeometry &curve );
};

#endif // QGSMAPTOOLPROFILECURVEFROMFEATURE_H
