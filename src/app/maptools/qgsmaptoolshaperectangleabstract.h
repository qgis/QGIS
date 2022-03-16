/***************************************************************************
    qgsmaptoolshaperectangleabstract.h  -  map tool for adding rectangle
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

#ifndef QGSMAPTOOLSHAPERECTANGLEABSTRACT_H
#define QGSMAPTOOLSHAPERECTANGLEABSTRACT_H

#include "qgsmaptoolshapecircleabstract.h"
#include "qgspolygon.h"
#include "qgsquadrilateral.h"
#include "qgis_app.h"

class APP_EXPORT QgsMapToolShapeRectangleAbstract: public QgsMapToolShapeAbstract
{
    Q_OBJECT

  public:
    QgsMapToolShapeRectangleAbstract( const QString &id, QgsMapToolCapture *parentTool )
      : QgsMapToolShapeAbstract( id, parentTool )
    {}

    void clean() override;

  protected:
    void addRectangleToParentTool();

    //! Rectangle
    QgsQuadrilateral mRectangle;
};

#endif // QGSMAPTOOLSHAPERECTANGLEABSTRACT_H
