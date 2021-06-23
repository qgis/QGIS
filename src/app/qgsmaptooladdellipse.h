/***************************************************************************
    qgsmaptooladdellipse.h  -  map tool for adding ellipse
    ---------------------
    begin                : July 2017
    copyright            : (C) 2017
    email                : lbartoletti at tuxfamily dot org
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLADDELLIPSE_H
#define QGSMAPTOOLADDELLIPSE_H

#include "qgsmaptooladdabstract.h"
#include "qgsellipse.h"
#include "qgssettingsregistrycore.h"
#include "qgis_app.h"

class QgsGeometryRubberBand;
class QgsSnapIndicator;

class APP_EXPORT QgsMapToolAddEllipse: public QgsMapToolAddAbstract
{
    Q_OBJECT
  public:
    QgsMapToolAddEllipse( QgsMapToolCapture *parentTool, QgsMapCanvas *canvas, CaptureMode mode = CaptureLine );

    void deactivate() override;
    void clean() override;

  protected:
    explicit QgsMapToolAddEllipse( QgsMapCanvas *canvas ) = delete; //forbidden

    //! Ellipse
    QgsEllipse mEllipse;

    //! convenient method to return the number of segments
    unsigned int segments( ) { return QgsSettingsRegistryCore::settingsDigitizingOffsetQuadSeg.valueAsInt() * 12; }
};

#endif // QGSMAPTOOLADDELLIPSE_H
