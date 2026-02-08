/***************************************************************************
    qgsmaptoolshapecircleabstract.h  -  map tool for adding circle
    ---------------------
    begin                : July 2017
    copyright            : (C) 2017
    email                : lituus at free dot fr
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLSHAPECIRCLEABSTRACT_H
#define QGSMAPTOOLSHAPECIRCLEABSTRACT_H

#include "qgis_app.h"
#include "qgscircle.h"
#include "qgsmaptoolshapeabstract.h"
#include "qgsmaptoolshaperegistry.h"
#include "qgspointlocator.h"

struct EdgesOnlyFilter : public QgsPointLocator::MatchFilter
{
    bool acceptMatch( const QgsPointLocator::Match &m ) override { return m.hasEdge(); }
};


class APP_EXPORT QgsMapToolShapeCircleAbstract : public QgsMapToolShapeAbstract
{
    Q_OBJECT

  public:
    QgsMapToolShapeCircleAbstract( const QString &id, QgsMapToolCapture *parentTool )
      : QgsMapToolShapeAbstract( id, parentTool ) {}

    ~QgsMapToolShapeCircleAbstract() override = default;

    void clean() override;

  protected:
    void addCircleToParentTool();

    //! Circle
    QgsCircle mCircle;
};

#endif // QGSMAPTOOLSHAPECIRCLEABSTRACT_H
