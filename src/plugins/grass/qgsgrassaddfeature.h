/***************************************************************************
                            qgsgrassaddfeature.h
                             -------------------
    begin                : September, 2015
    copyright            : (C) 2015 by Radim Blazek
    email                : radim.blazek@gmail.com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGRASSADDFEATURE_H
#define QGSGRASSADDFEATURE_H

#include "qgsmaptooladdfeature.h"

class QgsGrassAddFeature : public QgsMapToolAddFeature
{
    Q_OBJECT
  public:
    QgsGrassAddFeature( QgsMapCanvas* canvas, CaptureMode mode = CaptureNone );
    virtual ~QgsGrassAddFeature() override;
};

#endif // QGSGRASSADDFEATURE_H
